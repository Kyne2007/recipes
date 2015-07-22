#include "socket.h"

#include "inet_address.h"
#include "sockets_ops.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <strings.h>

using namespace mouse;

Socket::~Socket()
{
    sockets::close(fd_);
}

void Socket::bindAddress(const InetAddress& addr)
{
    sockets::bind(fd_, addr.addr());
}

void Socket::listen()
{
    sockets::listen(fd_);
}

int Socket::accept(InetAddress* peeraddr)
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof addr);
    int connfd = sockets::accept(fd_, &addr);
    if (connfd >= 0)
    {
        peeraddr->setAddr(addr);
    }
    return connfd;
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR,
            &optval, sizeof optval);
}

void Socket::shutdownWrite()
{
  sockets::shutdownWrite(fd_);
}


