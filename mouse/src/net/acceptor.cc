#include "acceptor.h"

#include "event_loop.h"
#include "inet_address.h"
#include "sockets_ops.h"

#include <functional>
#include <glog/logging.h>

using namespace mouse;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr)
  : loop_(loop),
    accept_socket_(sockets::createNonblocking()),
    accept_channel_(loop, accept_socket_.fd()),
    listenning_(false)
{
    accept_socket_.setReuseAddr(true);
    accept_socket_.bindAddress(listenAddr);
    accept_channel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

void Acceptor::listen()
{
    loop_->assertInLoopThread();
    listenning_ = true;
    accept_socket_.listen();
    accept_channel_.enableReading();
}

void Acceptor::handleRead()
{
    loop_->assertInLoopThread();
    InetAddress peer_addr(0);

    int connfd = accept_socket_.accept(&peer_addr);
    if (connfd >= 0)
    {
        if (new_connection_callback_)
        {
            new_connection_callback_(connfd, peer_addr);
        }
        else
        {
            sockets::close(connfd);
        }
    }
}

