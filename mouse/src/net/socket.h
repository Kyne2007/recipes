#ifndef MOUSE_NET_SOCKET_H
#define MOUES_NET_SOCKET_H

namespace mouse
{

class InetAddress;

class Socket
{
    //nocopyable
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

public:
    explicit Socket(int sockfd)
        : fd_(sockfd)
    {
    }

    ~Socket();

    void bindAddress(const InetAddress& localaddr);
    void listen();

    int accept(InetAddress* peeraddr);

    void setReuseAddr(bool on);

    void shutdownWrite();

    int fd() const { return fd_; }

private:
    const int fd_;
};

} //namespace Mouse

#endif
