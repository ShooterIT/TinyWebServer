/**********************************************************
*
* Author：　　　ShooterIT
* Datatime: 　　2016-6-9 12:08:10
* description：增强型i/o定义, from CSAPP
*
**********************************************************/

#ifndef __RIO_H__
#define __RIO_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIO_BUFSIZE 8192

typedef struct
{
    int rio_fd;                 //文件描述符
    int rio_cnt;                //buf中未读字节数
    char *rio_ptr;              //buf中未读指针
    char rio_buf[RIO_BUFSIZE];  //内部buf
}rio_t;

//robust i/o
//无缓冲的输入输出函数
ssize_t rio_read(int fd, void *userbuf, size_t n);
ssize_t rio_write(int fd, void *userbuf, size_t n);

//带缓冲的输入函数
void    rio_initbuf(rio_t *rp, int fd);
ssize_t rio_readlineb(rio_t *rp, void *userbuf, size_t maxlen);
ssize_t rio_readb(rio_t *rp, void *userbuf, size_t n);

//带错误处理的rubust i/o
ssize_t Rio_read(int fd, void *userbuf, size_t n);
ssize_t Rio_write(int fd, void *userbuf, size_t n);
void    Rio_initbuf(rio_t *rp, int fd);
ssize_t Rio_readlineb(rio_t *rp, void *userbuf, size_t maxlen);
ssize_t Rio_readb(rio_t *rp, void *userbuf, size_t n);

#ifdef __cplusplus
}
#endif

#endif
