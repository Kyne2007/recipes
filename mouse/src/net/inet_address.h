#ifndef MOUSE_INET_ADDRESS_H
#define MOUSE_INET_ADDRESS_H

#include <string>

#include <netinet/in.h>

namespace mouse
{

//This is a POD interface class
class InetAddress
{
public:
    explicit InetAddress(uint16_t port);

    InetAddress(const std::string& ip, uint16_t port);

    InetAddress(const struct sockaddr_in& address)
        : addr_(address)
    {
    }

    //default copy/assignment

    std::string toIpPort() const;

    const struct sockaddr_in& addr() const { return addr_; }
    void setAddr(const struct sockaddr_in& address) { addr_ = address; }

private:
    struct sockaddr_in addr_;
};

} // namespace Mouse

#endif
