/**********************************************************
*
* Author：　　　ShooterIT
* Datatime: 　　2016-12-26 09:08:133
* description：TinyWeb的实现 from csapp
*
**********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "net.h"
#include "utils.h"
#include "rio.h"
#include "sembuf.h"

#define NTHREAD 100
#define SBSIZE  200

//基于信号量的缓冲区
sembuf_t sb;

//http服务
void *http_thread(void *argv)
{
     //获得进程id并分离线程
    Pthread_detach(Pthread_self());
    while (1) {
        int connfd = sembuf_remove(&sb); //获得一个可用消费项
        //回声服务
        doit(connfd);
        //关闭连接
        Close(connfd);
        printf("Close connection\n");
    }

    return NULL;
}

//主流程
int main(int argc, char *argv[])
{
    int i,listenfd,connfd;
    char hostname[MAXLINE],port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    //用法: xxx <port>
    if(argc != 2){
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    clientlen = sizeof(clientaddr);

    //创建一个监听套接字
    listenfd = Open_listenfd(argv[1]);

    //初始化缓冲区
    sembuf_init(&sb, SBSIZE);
    //创建线程
    for(i = 0; i < NTHREAD; i++){
        Pthread_create(&tid, NULL, http_thread, NULL);
    }

    while(1){
        //创建一个与客户端的连接套接字
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        //获得客户端的信息
        Getnameinfo((SA*)&clientaddr, clientlen, hostname, MAXLINE,
            port, MAXLINE,0);
        printf("Accepted connection from (%s,%s)\n", hostname, port);
        sembuf_insert(&sb, connfd);
    }
}

//执行一个请求
void doit(int fd)
{
    int is_static;     //是否是静态内容
    struct stat fstat; //文件元数据
    //请求行缓存
    char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE];
    char filename[MAXLINE],cgiargs[MAXLINE];
    rio_t rio;

    //读取请求行
    Rio_initbuf(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    //输出请求行
    printf("Request header:\n%s\n", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    //只处理GET请求
    if(strcasecmp("GET",method)){
        clienterror(fd, method, "501", "Not implemented",
            "TinyWeb does not implement this memthod.");
        return ;
    }
    //忽略请求报文
    read_requesthdrs(&rio);

    //解析uri
    is_static = parse_uri(uri, filename, cgiargs);
    //判断文件是否存在
    if(stat(filename, &fstat) < 0){
        clienterror(fd, filename, "404", "Not found",
            "TinyWeb couldn't find this file.");
        return ;
    }

    //静态文件
    if(is_static){
        //普通文件且当前用户具有读的权限
        if(!(S_ISREG(fstat.st_mode)) || !(S_IRUSR & fstat.st_mode)){
            clienterror(fd, filename, "403", "Forbidden",
                "TinyWeb couldn't read the file.");
            return ;
        }
        //静态内容服务
        server_static(fd, filename, fstat.st_size);
    }
    //动态文件
    else {
        //普通文件且当前用户具有执行的权限
        if(!(S_ISREG(fstat.st_mode)) || !(S_IXUSR & fstat.st_mode)){
            clienterror(fd, filename, "403", "Forbidden",
                "TinyWeb couldn't run the CGI program.");
            return ;
        }
        //动态内容服务
        server_dynamic(fd, filename, cgiargs);
    }
}


//解析uri
int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;

    //静态内容
    if(!strstr(uri, "cgi-bin")){//非cgi-bin目录下都认为是静态内容
        strcpy(cgiargs,""); //清楚参数
        //构建文件名
        strcpy(filename, ".");
        strcat(filename, uri);
        //请求为目录,则认为是当前目录下的index.html文件
        if(uri[strlen(uri) - 1] == '/'){
            strcat(filename, "index.html");
        }
        return 1;
    }
    //动态内容
    else{
        ptr = strchr(uri, '?');
        //有参数
        if(ptr != NULL){
            strcpy(cgiargs, ptr+1); //拷贝参数
            *ptr = '\0'; //分隔文件名
        }else{
            strcpy(cgiargs,""); //无参数
        }
        //拷贝文件名
        strcpy(filename, ".");
        strcat(filename, uri);
        return 0;
    }
}

//获取文件类型
void get_filetype(char *filename, char *filetype)
{
    if(strstr(filename, ".html")){
        strcpy(filetype,"text/html");
    }
    if(strstr(filename, ".gif")){
        strcpy(filetype,"image/gif");
    }
    if(strstr(filename, ".png")){
        strcpy(filetype,"image/png");
    }
    if(strstr(filename, ".ico")){
        strcpy(filetype,"image/ico");
    }
    if(strstr(filename, ".jpg")){
        strcpy(filetype,"image/jpeg");
    }
    else{
        strcpy(filetype,"text/plain");
    }
}

//静态内容处理
void server_static(int fd, char *filename, int filesize)
{
    int n = 0, srcfd; //filename文件描述符
    char *srcp, filetype[MAXLINE], buf[MAXLINE];
    char *pbuf = buf;

    //发送响应行和响应报文
    get_filetype(filename, filetype);
    n = sprintf(pbuf+=n, "HTTP/1.0 200 OK\r\n");
    n = sprintf(pbuf+=n, "Server: TinyWeb Server\r\n");
    n = sprintf(pbuf+=n, "Connection: close\r\n");
    n = sprintf(pbuf+=n, "Content-length: %d\r\n", filesize);
    n = sprintf(pbuf+=n, "Content-type: %s\r\n\r\n", filetype);//第二个\r\n是报文结束标志
    Rio_write(fd, buf, strlen(buf));
    printf("Response header:\n%s", buf);

    //发送响应实体
    srcfd = Open(filename, O_RDONLY, 0);
    //将文件直接映射到内存中
    srcp = Mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);
    Rio_write(fd, srcp, filesize);
    //释放内存映射
    Munmap(srcp,filesize);
}

//动态内容处理
void server_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = {NULL};

    //构造响应行
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_write(fd, buf, strlen(buf));
    //响应报文
    sprintf(buf, "Server: TinyWeb Server\r\n");
    Rio_write(fd, buf, strlen(buf));
    //剩余部分应有可执行程序完成

    if(Fork() == 0){
        //子进程
        setenv("QUERY_STRING", cgiargs, 1);
        Dup2(fd,STDOUT_FILENO); //将子进程的输出重定向到连接描述符中
        Execve(filename, emptylist, environ); //运行CGI程序
    }

    //父进程
    Wait(NULL);
}

//忽略请求报文
void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];

    //为读到请求报文结束，一直读
    do{
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s",buf);
    }while (strcmp(buf, "\r\n"));

}

//客户端请求错误
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXLINE];
	char *pbody = body;
	int n = 0;

    //创建http响应主体
	n = sprintf(pbody+=n, "<html><title>TinyWeb Error</title>");
	n = sprintf(pbody+=n, "<body bgcolor=\"FFFFFF\">\r\n");
	n = sprintf(pbody+=n, "%s: %s\r\n", errnum, shortmsg);
    n = sprintf(pbody+=n, "<p>%s: %s\r\n", longmsg, cause);
    n = sprintf(pbody+=n, "<hr><em>Tiny Web Server</em>\r\n");

    //http响应头
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_write(fd, buf, strlen(buf));

    //http响应报文
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_write(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n", (int)strlen(body));
    Rio_write(fd, buf, strlen(buf));
    //响应报文结束
    Rio_write(fd, "\r\n", strlen("\r\n"));

    //发送响应主体
    Rio_write(fd, body, strlen(body));
}
