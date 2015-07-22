#ifndef MOUSE_NET_SOCKETS_OPS_H
#define MOUSE_NET_SOCKETS_OPS_H

#include <arpa/inet.h>
#include <endian.h>

namespace mouse
{

namespace sockets
{

//create a non-blocking socket file description
//abort is any error
int createNonblocking();

int  connect(int sockfd, const struct sockaddr_in& addr);
void bind(int sockfd, const struct sockaddr_in& addr);
void listen(int sockfd);
int  accept(int sockfd, struct sockaddr_in* addr);
void close(int sockfd);
void shutdownWrite(int sockfd);

void toIpPort(char* buf, size_t size, const struct sockaddr_in& addr);
void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);

struct sockaddr_in getLocalAddr(int sockfd);
struct sockaddr_in getPeerAddr(int sockfd);

int getSocketError(int sockfd);
bool isSelfConnect(int sockfd);

}//namespace sockets

}//namespace mouse

#endif
