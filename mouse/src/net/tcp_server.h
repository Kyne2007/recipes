#ifndef MOUSE_NET_TCP_SERVER_H
#define MOUSE_NET_TCP_SERVER_H

#include "callbacks.h"
#include "tcp_connection.h"

#include <map>
#include <memory>

namespace mouse
{

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer
{
    //nocopyable
    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

public:
    TcpServer(EventLoop* loop, const InetAddress& listen_addr);
    ~TcpServer();

    void setThreadsNum(int threads_num);

    void start();

    void setConnectionCallback(const ConnectionCallback& cb)
    { connection_callback_ = cb; }

    void setMessageCallback(const MessageCallback& cb)
    { message_callback_ = cb; }

    /// Set write complete callback.
    /// Not thread safe.
    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { write_complete_callback_ = cb; }


private:
    void newConnection(int sockfd, const InetAddress& peer_addr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

    EventLoop* loop_;  // the acceptor loop
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_; // avoid revealing Acceptor
    std::unique_ptr<EventLoopThreadPool> thread_pool_; // avoid revealing Acceptor
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    bool started_;
    int next_conn_id_;  // always in loop thread
    ConnectionMap connections_;
};

}//namespace mouse

#endif
