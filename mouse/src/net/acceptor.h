#ifndef MOUSE_NET_ACCEPTOR_H
#define MOUSE_NET_ACCEPTOR_H

#include <functional>

#include "channel.h"
#include "socket.h"

namespace mouse
{

class EventLoop;
class InetAddress;

class Acceptor {
    //nocopyable
    Acceptor(const Acceptor&) = delete;
    Acceptor& operator=(const Acceptor&) = delete;

public:
    typedef std::function<void (int sockfd, const InetAddress&)> NewConnectionCallback;

    Acceptor(EventLoop* loop, const InetAddress& listen_addr);

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { new_connection_callback_ = cb; }

    bool listenning() const { return listenning_; }
    void listen();

private:
    void handleRead();

    EventLoop* loop_;
    Socket accept_socket_;
    Channel accept_channel_;
    NewConnectionCallback new_connection_callback_;
    bool listenning_;
};

}//namespace mouse

#endif
