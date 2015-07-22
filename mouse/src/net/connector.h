#ifndef MOUSE_NET_CONNECTOR_H
#define MOUSE_NET_CONNECTOR_H

#include "inet_address.h"
#include "timer_id.h"

#include <functional>
#include <memory>

namespace mouse
{

class Channel;
class EventLoop;

class Connector
{
public:
    typedef std::function<void (int sockfd)> NewConnectionCallback;

    Connector(EventLoop* loop, const InetAddress& server_addr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { newConnectionCallback_ = cb; }

    void start();  // can be called in any thread
    void restart();  // must be called in loop thread
    void stop();  // can be called in any thread

    const InetAddress& serverAddress() const { return server_addr_; }

private:
    enum States { kDisconnected, kConnecting, kConnected };
    static const int kMaxRetryDelayMs = 30 * 1000;
    static const int kInitRetryDelayMs = 500;

    void setState(States s) { state_ = s; }
    void startInLoop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    int removeAndResetChannel();
    void resetChannel();

    EventLoop* loop_;
    InetAddress server_addr_;
    bool connect_; // atomic
    States state_;  // FIXME: use atomic variable
    std::unique_ptr<Channel> channel_;
    NewConnectionCallback newConnectionCallback_;
    int retry_delay_;
    TimerId timer_id_;
};

typedef std::shared_ptr<Connector> ConnectorPtr;

}//namespace mouse

#endif

