#ifndef MOUSE_NET_TCP_CONNECTION_H
#define MOUSE_NET_TCP_CONNECTION_H

#include "buffer.h"
#include "callbacks.h"
#include "inet_address.h"

#include <memory>
#include <string>

namespace mouse
{

class Channel;
class EventLoop;
class Socket;

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
    //nocopyable
    TcpConnection(const TcpConnection&) = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;

public:
    TcpConnection(EventLoop* loop,
                  const std::string& name,
                  int sockfd,
                  const InetAddress& local_address,
                  const InetAddress& peer_address);
    ~TcpConnection();

    EventLoop* loop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() { return local_address_; }
    const InetAddress& peerAddress() { return peer_address_; }
    bool connected() const { return state_ == kConnected; }

    //void send(const void* message, size_t len);
    // Thread safe.
    void send(const std::string& message);
    // Thread safe.
    void shutdown();

    void setConnectionCallback(const ConnectionCallback& cb)
    { connection_callback_ = cb; }

    void setMessageCallback(const MessageCallback& cb)
    { message_callback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { write_complete_callback_ = cb; }

    /// Internal use only.
    void setCloseCallback(const CloseCallback& cb)
    { close_callback_ = cb; }

  /// Internal use only.

  // called when TcpServer accepts a new connection
  void connectEstablished();   // should be called only once

  void connectDestroyed();  // should be called only once

private:
    enum StateE { kConnecting, kConnected, kDisconnecting, kDisconnected };

    void setState(StateE s) { state_ = s; }
    void handleRead(Timestamp receive_time);
    void handleWrite();
    void handleClose();
    void handleError();
    void sendInLoop(const std::string& message);
    void shutdownInLoop();

    EventLoop* loop_;
    std::string name_;
    StateE state_;
    // we don't expose those classes to client.
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    InetAddress local_address_;
    InetAddress peer_address_;

    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    CloseCallback close_callback_;

    Buffer input_buffer_;
    Buffer output_buffer_;
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

}//namespace mouse

#endif
