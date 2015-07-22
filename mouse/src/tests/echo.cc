#include "echo.h"

#include <glog/logging.h>

#include <functional>
using namespace std::placeholders;
#include <string>

EchoServer::EchoServer(mouse::EventLoop* loop,
                       const mouse::InetAddress& listenAddr)
  : server_(loop, listenAddr)
{
  server_.setConnectionCallback(
      std::bind(&EchoServer::onConnection, this, _1));
  server_.setMessageCallback(
      std::bind(&EchoServer::onMessage, this, _1, _2, _3));
}

void EchoServer::start()
{
  server_.start();
}

void EchoServer::onConnection(const mouse::TcpConnectionPtr& conn)
{
  LOG(INFO) << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
}

void EchoServer::onMessage(const mouse::TcpConnectionPtr& conn,
                           mouse::Buffer* buf,
                           mouse::Timestamp time)
{
  std::string msg(buf->retrieveAsString());
  LOG(INFO) << conn->name() << " echo " << msg.size() << " bytes, "
           << "data received at " << time.toString();
  conn->send(msg);
}

