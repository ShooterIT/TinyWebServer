/**********************************************************
*
* Author：　　　ShooterIT
* Datatime: 　　2017-11-4 17:30:21
* description：信号量维护的缓冲区
*
**********************************************************/

#ifndef __SEMBUF_H__
#define __SEMBUF_H__

#include <semaphore.h>

#ifdef __cplusplus
extern "C" {
#endif

//使用信号量维护的缓冲区结构定义
typedef struct
{
    int *buf;   //维护的缓冲区
    int n;      //大小
    int front;  //头
    int rear;   //尾
    sem_t mutex;//互斥锁
    sem_t slots;//可用生产槽
    sem_t items;//可用消费项
}sembuf_t;

//初始化信号量以及PV操作
void Sem_init(sem_t *sem, int pshared, unsigned int value);
void P(sem_t *sem);
void V(sem_t *sem);

//初始化缓冲区大小
void sembuf_init(sembuf_t *sp, int n);

//清理缓冲区
void sembuf_deinit(sembuf_t *sp);

//插入一个可用项
void sembuf_insert(sembuf_t *sp, int item);

//移除一个可用项
int sembuf_remove(sembuf_t *sp);

#ifdef __cplusplus
}
#endif
#endif
