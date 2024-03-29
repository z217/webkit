#include <signal.h>

#include <chrono>
#include <cstdio>
#include <thread>

#include "channel/simple_adapter.h"
#include "json_server_client.h"
#include "json_server_dispatcher.h"
#include "logger/chain_logger.h"
#include "logger/comm_logger.h"
#include "logger/stdout_logger.h"
#include "packet/byte_packet.h"
#include "pool/thread_pool.h"
#include "reactor/epoller.h"
#include "server/thread_server.h"
#include "util/syscall.h"

static bool IsMainRunning = true;

static void RegisterSignalHandler() {
  auto stop_func = [](int) { IsMainRunning = false; };
  webkit::Syscall::Signal(SIGKILL, stop_func);
  webkit::Syscall::Signal(SIGUSR1, stop_func);
  webkit::Syscall::Signal(SIGINT, stop_func);
}

int main(int argc, char *argv[]) {
  webkit::ServerConfig config;
  config.SetIp("127.0.0.1");
  config.SetPort(8080);
  config.SetIoThreadNum(1);

  webkit::Status s;

  if (config.IsDeamon()) {
    s = webkit::Syscall::Deamon();
    if (!s.Ok()) {
      printf("server run deamon error status code %d message %s\n", s.Code(),
             s.Message().c_str());
      return -1;
    }
  }
  RegisterSignalHandler();

  webkit::StdoutLogger stdout_logger;
  auto comm_logger_up = webkit::CommLogger::Open("json_server.log");
  webkit::ChainLogger chain_logger(&stdout_logger, comm_logger_up.get());
  webkit::Logger::SetDefaultInstance(&chain_logger);
  webkit::Logger::SetLogLevel(webkit::Logger::eDebug);

  webkit::SimpleAdapterFactory adapter_factory;
  webkit::ProtocolAdapterFactory::SetDefaultInstance(&adapter_factory);

  webkit::EpollerFactory epoller_factory(&config);
  webkit::ReactorFactory::SetDefaultInstance(&epoller_factory);

  webkit::BytePacketFactory packet_factory(&config);
  webkit::PacketFactory::SetDefaultInstance(&packet_factory);

  config.SetWorkerThreadNum(1);
  webkit::ThreadPoolFactory pool_factory(config.GetWorkerThreadNum(),
                                         config.GetWorkerQueueSize());
  webkit::PoolFactory::SetDefaultInstance(&pool_factory);

  JsonServerDispatcherFactory dispatcher_factory;
  webkit::DispatcherFactory::SetDefaultInstance(&dispatcher_factory);

  webkit::ThreadServer server(&config);
  s = server.Init();
  if (!s.Ok()) {
    printf("server init error status code %d message %s\n", s.Code(),
           s.Message().c_str());
    return -1;
  }
  s = server.Run();
  if (!s.Ok()) {
    printf("server run error status code %d message %s\n", s.Code(),
           s.Message().c_str());
    return -1;
  }

  WEBKIT_LOGINFO("json server is running");

  using namespace std::literals;
  std::this_thread::sleep_for(1s);

  webkit::ClientConfig cli_config;
  cli_config.AddHost(
      webkit::ClientConfig::Host{.ip = "127.0.0.1", .port = 8080});

  JsonServerClient client(&cli_config);
  std::string req = "{\"data\":\"Hello, world!\"}";
  std::string rsp;
  s = client.Echo(req, rsp);
  printf("rsp %s\n", rsp.c_str());

  server.Stop();
  WEBKIT_LOGINFO("json server stopped");
  return 0;
}