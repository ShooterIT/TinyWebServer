/**********************************************************
*
* Author：　　　ShooterIT
* Datatime: 　　2016-10-20 17:19:22
* description：网路相关接口定义，from CSAPP
*
**********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include "net.h"

//输出错误
inline static void net_error_exit(const char *msg)
{
    fprintf(stderr, "%s error: %s.\n", msg ,strerror(errno));
    exit(0);
}

//Getaddrinfo/Getnameinfo的错误
inline static void gai_errorx(int code, char *msg)
{
    fprintf(stderr, "%s error: %s.\n", msg, gai_strerror(code));
    exit(0);
}

//应用程序错误
inline static void app_error(char *msg)
{
    fprintf(stderr, "%s.\n", msg);
    exit(0);
}

//gethostbyname错误
inline static void dns_error(char *msg)
{
    fprintf(stderr, "%s.\n", msg);
    exit(0);
}

//带错误处理的网络处理
int Socket(int domain, int type, int protocol)
{
    int sockfd;

    if ((sockfd = socket(domain, type, protocol)) < 0){
    	net_error_exit("Socket");
    }
    return sockfd;
}

void Setsockopt(int s, int level, int optname, const void *optval, int optlen)
{
    if (setsockopt(s, level, optname, optval, optlen) < 0){
	       net_error_exit("Setsockopt");
    }
}

void Bind(int sockfd, struct sockaddr *my_addr, int addrlen)
{
    if (bind(sockfd, my_addr, addrlen) < 0){
        net_error_exit("Bind");
    }
}

void Listen(int s, int backlog)
{
    if (listen(s,  backlog) < 0){
        net_error_exit("Listen");
    }
}

int Accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
    int connfd;

    if ((connfd = accept(s, addr, addrlen)) < 0){
        net_error_exit("Accept");
    }
    return connfd;
}

void Connect(int sockfd, struct sockaddr *serv_addr, int addrlen)
{
    if (connect(sockfd, serv_addr, addrlen) < 0){
        net_error_exit("Connect");
    }
}

//不依赖协议函数
void Getaddrinfo(const char *node, const char *service,
                 const struct addrinfo *hints, struct addrinfo **res)
{
    int en; //错误代码

    if ((en = getaddrinfo(node, service, hints, res)) != 0){
        gai_errorx(en, "Getaddrinfo");
    }
}
void Getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host,
                 size_t hostlen, char *serv, size_t servlen, int flags)
{
    int en; //错误代码

    if ((en = getnameinfo(sa, salen, host, hostlen, serv,
                          servlen, flags)) != 0) {
        gai_errorx(en, "Getnameinfo");
    }
}

void Freeaddrinfo(struct addrinfo *res)
{
    freeaddrinfo(res);
}

void Inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
    if (!inet_ntop(af, src, dst, size)){
        net_error_exit("Inet_ntop");
    }
}

void Inet_pton(int af, const char *src, void *dst)
{
    int en; //errer number

    en = inet_pton(af, src, dst);
    if (en == 0){
	    app_error("inet_pton error: invalid dotted-decimal address");
    }else if (en < 0){
        net_error_exit("Inet_pton");
    }
}

//DNS处理
struct hostent *Gethostbyname(const char *name)
{
    struct hostent *p;

    if ((p = gethostbyname(name)) == NULL){
	    dns_error("Gethostbyname error");
    }
    return p;
}

struct hostent *Gethostbyaddr(const char *addr, int len, int type)
{
    struct hostent *p;

    if ((p = gethostbyaddr(addr, len, type)) == NULL){
	    dns_error("Gethostbyaddr error");
    }
    return p;
}


//客户端建立与服务器的连接
int open_clientfd(char *hostname, char* port)
{
    int clientfd, en;
    struct addrinfo hints, *listp, *p;

    //获得潜在的服务器地址列表
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;  //流式
    hints.ai_flags = AI_NUMERICSERV;  //必须为数字格式的端口号
    hints.ai_flags |= AI_ADDRCONFIG;  //推荐配置
    if ((en = getaddrinfo(hostname, port, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo failed (%s:%s): %s\n", hostname, port, gai_strerror(en));
        return -2;  //无法获得服务器地址列表
    }

    //遍历链表找到合适连接
    for (p = listp; p; p = p->ai_next) {
        //创建一个套接字描述符
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            continue; //失败，尝试下一个
        }

        //创建套接字成功，尝试连接
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1){
            break; //成功，跳出即可
        }
        //连接失败，关闭套接字退出
        if (close(clientfd) < 0) {
            fprintf(stderr, "open_clientfd: close failed: %s\n", strerror(errno));
            return -1;//连接失败
        }
    }

    //内存清理
    freeaddrinfo(listp);

    //尝试所有服务器，均失败
    if (!p){
        return -1;
    }
    //成功连接的套接字描述符
    else{
        return clientfd;
    }
}

//服务器监听客户端，成功返回监听描述符
int open_listenfd(char* port)
{
    struct addrinfo hints, *listp, *p;
    int listenfd, en, optval=1;

    //获得潜在的服务器地址列表
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;             //流式
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; //返回的套接字可能作为监听套接字
    hints.ai_flags |= AI_NUMERICSERV;            //端口号必须为数字格式
    if ((en = getaddrinfo(NULL, port, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo failed (port %s): %s\n", port, gai_strerror(en));
        return -2;  //无法获得服务器地址列表
    }

    //遍历链表找到合适绑定
    for (p = listp; p; p = p->ai_next) {
        //创建套接字描述符
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0){
            continue;  //失败，尝试下一个
        }

        //评估是否在使用
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval , sizeof(int));

        //绑定套接字地址
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0){
            break; //成功，跳出
        }
        //失败，关闭套接字描述符
        if (close(listenfd) < 0) {
            fprintf(stderr, "open_listenfd close failed: %s\n", strerror(errno));
            return -1;
        }
    }

    //内存清理
    freeaddrinfo(listp);
    //尝试所有服务器，均失败
    if (!p) {
        return -1;
    }

    //将连接套接字转换为监听套接字
    if (listen(listenfd, LISTENQ) < 0) {
        close(listenfd);
	    return -1;
    }
    //返回监听套接字
    return listenfd;
}

//带错误处理客户端/服务器创建套接字
int Open_clientfd(char *hostname, char* port)
{
    int en;

    if ((en = open_clientfd(hostname, port)) < 0){
        net_error_exit("Open_clientfd");
    }
    return en;
}
int Open_listenfd(char* port)
{
    int en;

    if ((en = open_listenfd(port)) < 0){
        net_error_exit("Open_listenfd");
    }
    return en;
}
