/**********************************************************
*
* Author：　　　ShooterIT
* Datatime: 　　2016-12-26 09:08:133
* description：TinyWeb的函数定义
*
**********************************************************/
#ifndef  __TINY_WEB_H__
#define  __TINY_WEB_H__

#include "rio.h"

#ifdef __cplusplus
extern "C" {
#endif

//执行一个请求
void doit(int fd);
//忽略请求报文
void read_requesthdrs(rio_t *rp);
//解析uri
int parse_uri(char *uri, char *filename, char *cgiargs);
//获取文件类型
void get_filetype(char *filename, char *filetype);
//静态内容处理
void server_static(int fd, char *filename, int filesize);
//动态内容处理
void server_dynamic(int fd, char *filename, char *cgiargs);
//客户端请求错误
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);


#ifdef __cplusplus
}
#endif

#endif
