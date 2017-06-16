/**********************************************************
*
* Author：　　　ShooterIT
* Datatime: 　　2016-6-9 12:08:10
* description：增强型i/o实现，from CSAPP
*
**********************************************************/
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "rio.h"

//无缓冲的输入函数
ssize_t rio_read(int fd, void *userbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nread = -1;
    void *buf = userbuf;
    while (nleft > 0) {
        if((nread = read(fd, buf, nleft)) < 0){
            if(errno == EINTR){ //被中断，重新尝试
                nread = 0;
            }else{
                return -1;      //read 出错
            }
        }else if(nread == 0){
            break;              //EOF
        }
        nleft -= nread;
        buf += nread;
    }
    return n - nleft;
}

//无缓冲的输出函数
ssize_t rio_write(int fd, void *userbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten = -1;
    void *buf = userbuf;

    while (nleft > 0) {
        if((nwritten = write(fd, buf, nleft)) <= 0){
            if(errno == EINTR){     //被中断，重新尝试
                nwritten = 0;
            }else{
                return -1;          //write出错
            }
        }
        nleft -= nwritten;
        buf += nwritten;
    }
    return n;
}

//初始化缓存
void rio_initbuf(rio_t *rp, int fd)
{
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_ptr = rp->rio_buf;
}

//带缓冲的输入函数,支持线程安全
static ssize_t rio_read_core(rio_t *rp, void *userbuf, size_t n)
{
    int cnt;
    //如果是空，则读满
    while (rp->rio_cnt <= 0) {
        rp->rio_cnt  = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        if(rp->rio_cnt < 0){
            if(errno != EINTR){         //中断继续，否则退出
                return -1;
            }
        }else if(rp->rio_cnt == 0){
            return 0;
        }else{
            rp->rio_ptr = rp->rio_buf;  //重置buf ptr
        }
    }
    //拷贝读取内存
    cnt = n;
    if(rp->rio_cnt < n){
        cnt = rp->rio_cnt;
    }
    memcpy(userbuf, rp->rio_ptr, cnt);
    rp->rio_ptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}

//带缓冲的读一行
ssize_t rio_readlineb(rio_t *rp, void *userbuf, size_t maxlen)
{
    ssize_t nread, n;
    char ch,*buf = (char*)userbuf;

    for(nread = 1; nread < maxlen; nread++){
        if((n = rio_read_core(rp, &ch, 1)) == 1){
            *buf++ = ch;
            if(ch == '\n'){
                nread++;
                break;
            }
        }else if(n == 0){
            if(nread == 1){
                return 0;       //eof，未读到数据
            }else{
                break;          //eof，但是已经读到了数据
            }
        }else{
            return -1;          //read出错
        }
    }
    *buf = 0;
    return nread - 1;
}

//带缓冲的读
ssize_t rio_readb(rio_t *rp, void *userbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nread = -1;
    char *buf = (char*)userbuf;;

    while (nleft > 0) {
        if((nread == rio_read_core(rp, buf, nleft)) < 0){
            return -1;  //read 出错
        }else if(nread == 0){
            break;      //eof
        }
        nleft -= nread;
        buf   += nread;
    }
    return n - nleft;  //返回已经读到的字符
}

//输出错误
inline static void rio_error_exit(const char *msg)
{
    fprintf(stderr, "%s error: %s.\n", msg ,strerror(errno));
    exit(0);
}

//对错误进行处理
ssize_t Rio_read(int fd, void *userbuf, size_t n)
{
    ssize_t nread;
    if((nread = rio_read(fd, userbuf, n)) < 0){
        rio_error_exit("Rio_read");
    }
    return nread;
}
ssize_t Rio_write(int fd, void *userbuf, size_t n)
{
    ssize_t nwritten;
    if((nwritten = rio_write(fd, userbuf, n)) < 0){
        rio_error_exit("Rio_write");
    }
    return nwritten;
}
void Rio_initbuf(rio_t *rp, int fd)
{
    rio_initbuf(rp, fd);
}
ssize_t Rio_readlineb(rio_t *rp, void *userbuf, size_t maxlen)
{
    ssize_t nread;
    if((nread = rio_readlineb(rp, userbuf, maxlen)) < 0){
        rio_error_exit("Rio_readlineb");
    }
    return nread;
}
ssize_t Rio_readb(rio_t *rp, void *userbuf, size_t n)
{
    ssize_t nread;
    if((nread = rio_readb(rp, userbuf, n)) < 0){
        rio_error_exit("Rio_readb");
    }
    return nread;
}
