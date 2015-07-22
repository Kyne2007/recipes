#include "../net/connector.h"
#include "../net/event_loop.h"

#include <stdio.h>

mouse::EventLoop* g_loop;

void connectCallback(int sockfd)
{
  printf("connected.\n");
  g_loop->quit();
}

int main(int argc, char* argv[])
{
  mouse::EventLoop loop;
  g_loop = &loop;
  mouse::InetAddress addr("127.0.0.1", 2007);
  mouse::ConnectorPtr connector(new mouse::Connector(&loop, addr));
  connector->setNewConnectionCallback(connectCallback);
  connector->start();

  loop.startLoop();
}

