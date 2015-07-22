#include "echo.h"

#include <glog/logging.h>
#include "../net/event_loop.h"

int main(int argc, char *argv[])
{
    google::InitGoogleLogging(argv[0]);
    google::LogToStderr();
    google::InstallFailureSignalHandler();
    LOG(INFO) << "pid = " << getpid();
    mouse::EventLoop loop;
    mouse::InetAddress listenAddr(2007);
    EchoServer server(&loop, listenAddr);
    server.start();
    loop.startLoop();
}

