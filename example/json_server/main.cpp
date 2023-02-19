#include <chrono>
#include <cstdio>
#include <thread>

#include "json_server_client.h"
#include "json_server_dispatcher.h"
#include "logger/stdout_logger.h"
#include "packet/byte_packet.h"
#include "pool/thread_pool.h"
#include "reactor/epoller.h"
#include "server/json_server.h"
#include "util/syscall.h"

using namespace std::literals;

static bool IsMainRunning = true;

int main(int argc, char *argv[]) {
  webkit::ServerConfig config;
  config.SetIp("127.0.0.1");
  config.SetPort(8080);
  config.SetIoThreadNum(1);

  webkit::Status s;

  if (config.IsDeamon()) {
    s = webkit::Deamon();
    if (!s.Ok()) {
      printf("server run deamon error status code %d message %s\n", s.Code(),
             s.Message());
      return -1;
    }
  }

  webkit::StdoutLogger logger;
  webkit::Logger::SetDefaultInstance(&logger);
  webkit::Logger::SetLogLevel(webkit::Logger::eError);

  webkit::EpollerFactory epoller_factory(&config);
  webkit::ReactorFactory::SetDefaultInstance(&epoller_factory);

  webkit::BytePacketFactory packet_factory;
  webkit::PacketFactory::SetDefaultInstance(&packet_factory);

  config.SetWorkerThreadNum(1);
  webkit::ThreadPoolFactory pool_factory(config.GetWorkerThreadNum(),
                                         config.GetWorkerQueueSize());
  webkit::PoolFactory::SetDefaultInstance(&pool_factory);

  JsonServerDispatcherFactory dispatcher_factory;
  webkit::DispatcherFactory::SetDefaultInstance(&dispatcher_factory);

  webkit::JsonServer server(&config);
  s = server.Init();
  if (!s.Ok()) {
    printf("server init error status code %d message %s\n", s.Code(),
           s.Message());
    return -1;
  }
  s = server.Run();
  if (!s.Ok()) {
    printf("server run error status code %d message %s\n", s.Code(),
           s.Message());
    return -1;
  }

  webkit::Signal(SIGKILL, [](int) { IsMainRunning = false; });
  std::this_thread::sleep_for(std::chrono::seconds(1));

  webkit::ClientConfig cli_config;
  cli_config.SetIp("127.0.0.1");
  cli_config.SetPort(8080);

  JsonServerClient client(&cli_config);
  std::string req = "{\"method\":\"echo\",\"data\":{\"key\":\"a\"}}";
  std::string rsp;
  s = client.Echo(req, rsp);
  printf("rsp %s\n", rsp.c_str());

  server.Stop();
  return 0;
}