/**********************************************************
*
* Author：　　　ShooterIT
* Datatime: 　　2017-11-4 17:40:18
* description：信号量维护的缓冲区
*
**********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "sembuf.h"

//错误处理
inline static void unix_error(char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}

//初始化信号量以及PV操作
void Sem_init(sem_t *sem, int pshared, unsigned int value)
{
    if (sem_init(sem, pshared, value) < 0){
    	unix_error("Sem_init error");
    }
}

void P(sem_t *sem)
{
    if (sem_wait(sem) < 0){
	   unix_error("P error");
   }
}

void V(sem_t *sem)
{
    if (sem_post(sem) < 0){
	   unix_error("V error");
   }
}

//初始化缓冲区大小
void sembuf_init(sembuf_t *sp, int n)
{
    sp->buf = (int*)malloc(n * sizeof(int));
    if(sp->buf == NULL){
        unix_error("malloc error");
    }
    sp->n = n;
    sp->front = sp->rear = 0;
    Sem_init(&sp->mutex, 0, 1); //互斥量
    Sem_init(&sp->slots, 0, n); //生产槽
    Sem_init(&sp->items, 0, 0); //消费项
}

//清理缓冲区
void sembuf_deinit(sembuf_t *sp)
{
    if(sp->buf != NULL){
        free(sp->buf);
        sp->buf = NULL;
    }
}

//插入一个可用项
void sembuf_insert(sembuf_t *sp, int item)
{
    P(&sp->slots); //申请槽
    P(&sp->mutex); //互斥锁
    sp->buf[(++sp->rear) % (sp->n)] = item; //添加消费项
    V(&sp->mutex);
    V(&sp->items); //增加消费项
}

//移除一个可用项
int sembuf_remove(sembuf_t *sp)
{
    int item;
    P(&sp->items); //申请消费项
    P(&sp->mutex); //互斥锁
    item = sp->buf[(++sp->front) % (sp->n)];//获得得消费项
    V(&sp->mutex);
    V(&sp->slots);//增加生产槽
    return item;
}
