#include "tcp_server.h"

#include "acceptor.h"
#include "event_loop.h"
#include "event_loop_thread_pool.h"
#include "sockets_ops.h"

#include <glog/logging.h>

#include <functional>
using namespace std::placeholders;
#include <stdio.h>
#include <assert.h>


using namespace mouse;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listen_addr)
    : loop_(CHECK_NOTNULL(loop)),
      name_(listen_addr.toIpPort()),
      acceptor_(new Acceptor(loop, listen_addr)),
      thread_pool_(new EventLoopThreadPool(loop)),
      started_(false),
      next_conn_id_(1)
{
    acceptor_->setNewConnectionCallback(
            std::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer()
{
}

void TcpServer::setThreadsNum(int threads_num)
{
    assert(0 <= threads_num);
    thread_pool_->setThreadsNum(threads_num);
}

void TcpServer::start()
{
    if (!started_)
    {
        started_ = true;
    }

    if (!acceptor_->listenning())
    {
        loop_->runInLoop(
                std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peer_addr)
{
    loop_->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof buf, "#%d", next_conn_id_);
    ++next_conn_id_;
    std::string conn_name = name_ + buf;

    LOG(INFO) << "TcpServer::NewConnection [" << name_
        << "] - new connection [" << conn_name
        << "] from " << peer_addr.toIpPort();
    InetAddress local_addr(sockets::getLocalAddr(sockfd));
    // FIXME poll with zero timeout to double confirm the new connection
    EventLoop* io_loop = thread_pool_->getNextLoop();
    TcpConnectionPtr conn(
            new TcpConnection(io_loop, conn_name, sockfd, local_addr, peer_addr));
    connections_[conn_name] = conn;
    conn->setConnectionCallback(connection_callback_);
    conn->setMessageCallback(message_callback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, _1));
    io_loop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    loop_->assertInLoopThread();
    LOG(INFO) << "TcpServer::removeConnectionInLoop [" << name_
        << "] - connection " << conn->name();
    size_t n = connections_.erase(conn->name());
    assert(n == 1); (void)n;
    EventLoop* io_loop = conn->loop();
    io_loop->queueInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn));
}

