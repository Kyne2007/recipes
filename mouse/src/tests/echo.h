#ifndef MOUSE_TEST_ECHO_H
#define MOUSE_TEST_ECHO_H

#include "../net/tcp_server.h"

// RFC 862
class EchoServer
{
public:
    EchoServer(mouse::EventLoop* loop,
            const mouse::InetAddress& listenAddr);

    void start();  // calls server_.start();

private:
    void onConnection(const mouse::TcpConnectionPtr& conn);

    void onMessage(const mouse::TcpConnectionPtr& conn,
            mouse::Buffer* buf,
            mouse::Timestamp time);

    mouse::TcpServer server_;
};

#endif  // MUDUO_EXAMPLES_SIMPLE_ECHO_ECHO_H
