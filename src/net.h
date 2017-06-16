/**********************************************************
*
* Author：　　　ShooterIT
* Datatime: 　　2016-5-10 17:12:20
* description：网路相关接口定义，from CSAPP
*
**********************************************************/

#ifndef __NET_H__
#define __NET_H__


#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LISTENQ  1024

//套接字地址格式
typedef struct sockaddr SA;

// 绑定错误或者DNS错误
extern int h_errno;

//带错误处理的网络处理
int Socket(int domain, int type, int protocol);
void Setsockopt(int s, int level, int optname, const void *optval, int optlen);
void Bind(int sockfd, struct sockaddr *my_addr, int addrlen);
void Listen(int s, int backlog);
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);
void Connect(int sockfd, struct sockaddr *serv_addr, int addrlen);

//不依赖协议函数
void Getaddrinfo(const char *node, const char *service,
                 const struct addrinfo *hints, struct addrinfo **res);
void Getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host,
                 size_t hostlen, char *serv, size_t servlen, int flags);
void Freeaddrinfo(struct addrinfo *res);
void Inet_ntop(int af, const void *src, char *dst, socklen_t size);
void Inet_pton(int af, const char *src, void *dst);

//DNS处理
struct hostent *Gethostbyname(const char *name);
struct hostent *Gethostbyaddr(const char *addr, int len, int type);

//客户端/服务器创建套接字
int open_clientfd(char *hostname, char* port);
int open_listenfd(char* port);

//带错误处理客户端/服务器创建套接字
int Open_clientfd(char *hostname, char* port);
int Open_listenfd(char* port);

#ifdef __cplusplus
}
#endif

#endif
