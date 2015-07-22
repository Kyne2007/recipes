#include "inet_address.h"
#include "sockets_ops.h"

#include <strings.h>
#include <netinet/in.h>

using namespace mouse;

InetAddress::InetAddress(uint16_t port)
{
    bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_.sin_port = htons(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port)
{
    bzero(&addr_, sizeof addr_);
    sockets::fromIpPort(ip.c_str(), port, &addr_);
}

std::string InetAddress::toIpPort() const
{
    char buf[32];
    sockets::toIpPort(buf, sizeof buf, addr_);
    return buf;
}

