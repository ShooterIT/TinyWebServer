/**********************************************************
*
* Author：　　　ShooterIT
* Datatime: 　　2016-6-5 12:08:10
* description：常用工具函数的实现，from CSAPP
*
**********************************************************/

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "utils.h"

//unix输出错误
void error_exit(const char *msg)
{
    fprintf(stderr, "%s error: %s.\n", msg ,strerror(errno));
    exit(0);
}
//应用程序错误
void app_error(char *msg)
{
    fprintf(stderr, "%s.\n", msg);
    exit(0);
}

//异步信号安全输出,调用函数必须均为异步信号安全函数
void Sio_put(const char *str)
{
    if(write(STDOUT_FILENO, str, strlen(str)) < 0){
        _exit(1);
    }
}

//i/o基本操作
int Open(const char *pathname, int flags, mode_t mode)
{
    int rc;

    if ((rc = open(pathname, flags, mode))  < 0){
	       error_exit("Open");
    }
    return rc;
}

ssize_t Read(int fd, void *buf, size_t count)
{
    ssize_t rc;

    if ((rc = read(fd, buf, count)) < 0){
         error_exit("Read");
    }
    return rc;
}

ssize_t Write(int fd, const void *buf, size_t count)
{
    ssize_t rc;

    if ((rc = write(fd, buf, count)) < 0){
        error_exit("Write");
    }
    return rc;
}

off_t Lseek(int fildes, off_t offset, int whence)
{
    off_t rc;

    if ((rc = lseek(fildes, offset, whence)) < 0){
        error_exit("Lseek");
    }
    return rc;
}

void Close(int fd)
{
    if ((close(fd)) < 0){
        error_exit("Close");
    }
}

int Select(int  n, fd_set *readfds, fd_set *writefds,
	   fd_set *exceptfds, struct timeval *timeout)
{
    int rc;

    if ((rc = select(n, readfds, writefds, exceptfds, timeout)) < 0){
        error_exit("Select");
    }
    return rc;
}

//标准库i/o
void Fclose(FILE *fp)
{
    if (fclose(fp) != 0){
        error_exit("Fclose");
    }
}

FILE *Fdopen(int fd, const char *type)
{
    FILE *fp;

    if ((fp = fdopen(fd, type)) == NULL){
        error_exit("Fdopen");
    }
    return fp;
}

char *Fgets(char *ptr, int n, FILE *stream)
{
    char *rptr;

    if (((rptr = fgets(ptr, n, stream)) == NULL) && ferror(stream)){
        app_error("Fgets");
    }

    return rptr;
}

FILE *Fopen(const char *filename, const char *mode)
{
    FILE *fp;

    if ((fp = fopen(filename, mode)) == NULL){
        error_exit("Fopen");
    }

    return fp;
}

void Fputs(const char *ptr, FILE *stream)
{
    if (fputs(ptr, stream) == EOF){
        error_exit("Fputs");
    }
}

size_t Fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t n;

    if (((n = fread(ptr, size, nmemb, stream)) < nmemb) && ferror(stream)){
        error_exit("Fread");
    }

    return n;
}

void Fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if (fwrite(ptr, size, nmemb, stream) < nmemb){
        error_exit("Fwrite");
    }
}


//带错误处理的Fork
pid_t Fork()
{
    pid_t pid;
    if((pid = fork()) < 0){
        error_exit("Fork");
    }
    return pid;
}
//带错误处理的wait
pid_t Wait(int *status)
{
    pid_t pid;

    if ((pid  = wait(status)) < 0){
        error_exit("Wait");
    }
    return pid;
}
//带错误exec簇
void Execve(const char * filename, char * const argv[], char * const envp[])
{
    if(execve(filename, argv, envp) < 0){
        error_exit("Execve");
    }
}
void Execvp(const char * path, char * const argv[])
{
    if(execvp(path, argv) < 0){
        error_exit("Execvp");
    }
}
//i/o重定向
int Dup2(int oldfd, int newfd)
{
    int fd;
    if((fd = dup2(oldfd, newfd)) < 0){
        error_exit("Dup2");
    }
    return fd;
}

//内存映射
void *Mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
    void *ptr;

    if ((ptr = mmap(addr, len, prot, flags, fd, offset)) == ((void *) -1)){
	    error_exit("Mmap");
    }
    return ptr;
}

void Munmap(void *start, size_t length)
{
    if (munmap(start, length) < 0){
	    error_exit("Munmap");
    }
}
