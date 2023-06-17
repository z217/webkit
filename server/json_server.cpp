#include "json_server.h"

#include <fcntl.h>

#include <chrono>

#include "dispatcher/json_dispatcher.h"
#include "socket/tcp_socket.h"
#include "webkit/logger.h"
#include "webkit/packet.h"

using namespace std::literals;

namespace webkit {
JsonServer::JsonServer(const ServerConfig *config)
    : config_(config),
      worker_pool_(nullptr),
      event_queue_(nullptr),
      is_running_(false) {}

JsonServer::~JsonServer() {
  delete event_queue_;
  delete event_free_queue_;
}

Status JsonServer::Init() {
  worker_pool_ = PoolFactory::GetDefaultInstance()->Build();
  event_queue_ =
      new CircularQueue<std::shared_ptr<Event>>(config_->GetMaxConnection());
  event_free_queue_ =
      new CircularQueue<std::shared_ptr<Event>>(config_->GetMaxConnection());
  return Status::OK();
}

Status JsonServer::Run() {
  is_running_ = true;
  worker_pool_->Run();

  std::vector<std::shared_ptr<Reactor>> reactor_sp_vec;
  uint32_t io_thread_num = config_->GetIoThreadNum();
  for (uint32_t i = 0; i < io_thread_num; i++) {
    std::shared_ptr<Reactor> reactor_sp =
        ReactorFactory::GetDefaultInstance()->Build();
    reactor_sp_vec.push_back(reactor_sp);
    io_thread_vec_.emplace_back([=] { RunIo(i, reactor_sp); });
  }
  accept_thread_ = std::thread([=] { RunAccept(reactor_sp_vec); });

  return Status::OK();
}

void JsonServer::Stop() {
  is_running_ = false;
  accept_thread_.join();
  for (std::thread &t : io_thread_vec_) t.join();
  io_thread_vec_.clear();
  worker_pool_->Stop();
}

void JsonServer::RunIo(uint32_t thread_id,
                       std::shared_ptr<Reactor> reactor_sp) {
  Status s;

  while (is_running_) {
    std::vector<Event *> event_vec;
    s = reactor_sp->Wait(event_vec);
    if (s.Code() == StatusCode::eRetry) continue;
    if (!s.Ok()) {
      WEBKIT_LOGERROR(
          "server io thread %u reactor wait error status code %d message %s",
          thread_id, s.Code(), s.Message());
      return;
    }

    WEBKIT_LOGDEBUG("thread %u epoll wait return %zu", thread_id,
                    event_vec.size());
    for (Event *event : event_vec) {
      if (event->IsReadyToRecv()) {
        s = worker_pool_->Submit([&] {
          Status s;

          for (int i = 0; i < 3; i++) {
            s = event->Recv();
            if (s.Code() != StatusCode::eRetry) break;
          }
          if (!s.Ok()) {
            WEBKIT_LOGERROR("event recv error status code %d message %s",
                            s.Code(), s.Message());
            FreeEvent(event->shared_from_this());
            return;
          }

          std::shared_ptr<Dispatcher> dispatcher_sp =
              DispatcherFactory::GetDefaultInstance()->Build();
          s = dispatcher_sp->Dispatch(event->GetPacket().get());
          if (s.Code() == StatusCode::eRetry) {
            WEBKIT_LOGDEBUG("dispatcher packet need more data");
            return;
          }
          if (!s.Ok()) {
            WEBKIT_LOGERROR("dispacher error status code %d message %s",
                            s.Code(), s.Message());
            FreeEvent(event->shared_from_this());
            return;
          }

          event->ClearEvent();
          event->SetReadyToSend();
          s = event->ModInReactor();
          if (!s.Ok()) {
            WEBKIT_LOGERROR("reactor modify error status code %d message %s",
                            s.Code(), s.Message());
            FreeEvent(event->shared_from_this());
            return;
          }
        });
      } else if (event->IsReadyToSend()) {
        s = worker_pool_->Submit([&] {
          Status s;

          for (int i = 0; i < 3; i++) {
            s = event->Send();
            if (s.Code() != StatusCode::eRetry) break;
          }
          if (!s.Ok()) {
            WEBKIT_LOGERROR("event send error status code %d message %s",
                            s.Code(), s.Message());
          }

          FreeEvent(event->shared_from_this());
        });
      } else {
        WEBKIT_LOGERROR("event return error event");
        FreeEvent(event->shared_from_this());
      }
    }
  }
}

void JsonServer::RunAccept(
    std::vector<std::shared_ptr<Reactor>> reactor_sp_vec) {
  Status s;

  TcpSocket socket;
  s = socket.Listen(config_->GetIp(), config_->GetPort());
  if (!s.Ok()) {
    WEBKIT_LOGFATAL("socket listen error status code %d message %s", s.Code(),
                    s.Message());
    return;
  }

  size_t cur_reactor_idx = 0;
  while (is_running_) {
    auto cli_socket_sp = std::make_shared<TcpSocket>();
    s = socket.Accept(cli_socket_sp.get());
    if (!s.Ok()) {
      if (s.Code() != StatusCode::eRetry) {
        WEBKIT_LOGERROR("socket accept error status code %d message %s",
                        s.Code(), s.Message());
      }
      continue;
    }
    s = cli_socket_sp->SetNonBlock();
    if (!s.Ok()) {
      WEBKIT_LOGERROR(
          "client socket %s:%d set non block error status code %d message %s",
          cli_socket_sp->GetIp(), cli_socket_sp->GetPort(), s.Code(),
          s.Message());
      continue;
    }
    s = cli_socket_sp->SetTimeout(config_->GetSockTimeoutSec());
    if (!s.Ok()) {
      WEBKIT_LOGERROR("socket set timeout error status code %d message %s",
                      s.Code(), s.Message());
      continue;
    }

    if (event_queue_->IsFull()) {
      WEBKIT_LOGFATAL(
          "server current connection reaches max limit %u, new connect is "
          "dropped",
          config_->GetMaxConnection());
      continue;
    }

    std::shared_ptr<Reactor> reactor_sp = reactor_sp_vec[cur_reactor_idx];
    cur_reactor_idx = (cur_reactor_idx + 1) % reactor_sp_vec.size();

    std::shared_ptr<Event> event_sp;
    s = event_free_queue_->Pop(event_sp);
    if (!s.Ok()) {
      auto packet_sp = PacketFactory::GetDefaultInstance()->Build();
      event_sp = reactor_sp->CreateEvent(cli_socket_sp, packet_sp);
      s = event_queue_->Push(event_sp);
      if (!s.Ok()) {
        WEBKIT_LOGERROR(
            "server event queue new connection push error status code %d "
            "message %s",
            s.Code(), s.Message());
        continue;
      }
    } else {
      event_sp->SetSocket(cli_socket_sp);
      event_sp->ClearEvent();
    }

    WEBKIT_LOGDEBUG("accept client %s:%d fd %d", cli_socket_sp->GetIp(),
                    cli_socket_sp->GetPort(), cli_socket_sp->GetFd());
    event_sp->SetReadyToRecv();
    s = event_sp->AddToReactor();
    if (!s.Ok()) {
      WEBKIT_LOGERROR("event add to reactor error status code %d message %s",
                      s.Code(), s.Message());
      FreeEvent(event_sp);
      continue;
    }
  }

  socket.Close();
}

Status JsonServer::FreeEvent(std::shared_ptr<Event> event_sp) {
  if (event_sp == nullptr) return Status::OK();
  Status s;
  s = event_sp->DelFromReactor();
  if (!s.Ok()) {
    WEBKIT_LOGFATAL("event delete from reactor error status code %d message %s",
                    s.Code(), s.Message());
    return s;
  }

  s = event_sp->GetSocket()->Close();
  if (!s.Ok() && s.Code() != StatusCode::eSocketDisonnected) {
    WEBKIT_LOGFATAL("event socket close error status code %d message %s",
                    s.Code(), s.Message());
    return s;
  }

  s = event_free_queue_->Push(event_sp);
  if (!s.Ok()) {
    WEBKIT_LOGFATAL("event free queue push error status code %d message %s",
                    s.Code(), s.Message());
    return s;
  }

  return Status::OK();
}
}  // namespace webkit
