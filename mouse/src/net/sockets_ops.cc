#include "sockets_ops.h"

#include <glog/logging.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace mouse;

namespace
{

typedef struct sockaddr SA;

const SA* sockaddr_cast(const struct sockaddr_in* addr)
{
    return reinterpret_cast<const SA*>(addr);
}

SA* sockaddr_cast(struct sockaddr_in* addr)
{
    return reinterpret_cast<SA*>(addr);
}

void setNonBlockAndCloseOnExec(int sockfd)
{
  // non-block
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    ::fcntl(sockfd, F_SETFL, flags);

    // close-on-exec
    flags = ::fcntl(sockfd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    ::fcntl(sockfd, F_SETFD, flags);
}

}//namespace

int sockets::createNonblocking()
{
    int sockfd = ::socket(AF_INET,
            SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
            IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOG(FATAL) << "sockets::createNonblocking";
    }

    return sockfd;
}

int sockets::connect(int sockfd, const struct sockaddr_in& addr)
{
    return ::connect(sockfd, sockaddr_cast(&addr), sizeof addr);
}

void sockets::bind(int sockfd, const struct sockaddr_in& addr)
{
    int ret = ::bind(sockfd, sockaddr_cast(&addr), sizeof addr);
    if (ret < 0)
    {
        LOG(FATAL) << "sockets::bind";
    }
}

void sockets::listen(int sockfd)
{
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0)
    {
        LOG(FATAL) << "sockets::listen";
    }
}

int sockets::accept(int sockfd, struct sockaddr_in* addr)
{
    socklen_t addrlen = sizeof *addr;

    int connfd = ::accept4(sockfd, sockaddr_cast(addr),
            &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);

    if (connfd < 0)
    {
        int saved_errno = errno;
        LOG(ERROR) << "Socket::accept";
        switch (saved_errno)
        {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
                errno = saved_errno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                LOG(FATAL) << "unexpected error of ::accept " << saved_errno;
                break;
            default:
                LOG(FATAL) << "unknown error of ::accept " << saved_errno;
                break;
        }
    }

    return connfd;
}

void sockets::close(int sockfd)
{
    if (::close(sockfd) < 0)
    {
        LOG(ERROR) << "sockets::close";
    }
}

void sockets::shutdownWrite(int sockfd)
{
  if (::shutdown(sockfd, SHUT_WR) < 0)
  {
    LOG(ERROR) << "sockets::shutdownWrite";
  }
}


void sockets::toIpPort(char* buf, size_t size, const struct sockaddr_in& addr)
{
    char ip[INET_ADDRSTRLEN] = "INVALID";
    ::inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof ip);
    uint16_t port = ntohs(addr.sin_port);
    snprintf(buf, size, "%s:%u", ip, port);
}

void sockets::fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
    {
        LOG(ERROR) << "sockets::fromHostPort";
    }
}

struct sockaddr_in sockets::getLocalAddr(int sockfd)
{
    struct sockaddr_in local_addr;
    bzero(&local_addr, sizeof local_addr);
    socklen_t addrlen = sizeof(local_addr);
    if (::getsockname(sockfd, sockaddr_cast(&local_addr), &addrlen) < 0)
    {
        LOG(ERROR) << "sockets::getLocalAddr";
    }
    return local_addr;
}

struct sockaddr_in sockets::getPeerAddr(int sockfd)
{
    struct sockaddr_in peer_addr;
    bzero(&peer_addr, sizeof peer_addr);
    socklen_t addrlen = sizeof(peer_addr);
    if (::getpeername(sockfd, sockaddr_cast(&peer_addr), &addrlen) < 0)
    {
        LOG(ERROR) << "sockets::getPeerAddr";
    }
    return peer_addr;
}

int sockets::getSocketError(int sockfd)
{
    int optval;
    socklen_t optlen = sizeof optval;

    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        return errno;
    }
    else
    {
        return optval;
    }
}

bool sockets::isSelfConnect(int sockfd)
{
    struct sockaddr_in local_addr = getLocalAddr(sockfd);
    struct sockaddr_in peer_addr = getPeerAddr(sockfd);
    return local_addr.sin_port == peer_addr.sin_port
        && local_addr.sin_addr.s_addr == peer_addr.sin_addr.s_addr;
}
