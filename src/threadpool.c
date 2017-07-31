/**********************************************************
*
* Author：　　　from network
* Datatime: 　　2016-11-8 17:12:20
* description： 线程池相关实现
*
**********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include "threadpool.h"


//应用程序错误
inline static void app_error(char *msg)
{
    fprintf(stderr, "%s.\n", msg);
    //exit(0);
}

/*
 * pthreadpool_init - 初始化线程池
 * @thread_num - 线程池开启的线程个数
 * @queue_max_num - 队列的最大job个数
 *  返回 - 成功返回线程池地址 失败返回NULL
 */
threadpool_t *threadpool_init(int thread_num, int queue_max_num)
{
    //申请内存池结构空间
    threadpool_t *pool = NULL;
    pool = (threadpool_t*)malloc(sizeof(threadpool_t));
    do{
        if (NULL == pool){
            app_error("failed to malloc threadpool");
            break;
        }
        pool->thread_num = thread_num;
        pool->queue_max_num = queue_max_num;
        pool->queue_cur_num = 0;
        pool->head = NULL;
        pool->tail = NULL;
        //初始化互斥锁
        if (pthread_mutex_init(&(pool->mutex), NULL)){
            app_error("pthread_mutex_init");
            break;
        }
        //初始化条件锁
        if (pthread_cond_init(&(pool->queue_empty), NULL)){
            app_error("pthread_cond_init");
            break;
        }
        if (pthread_cond_init(&(pool->queue_not_empty), NULL)){
            app_error("pthread_cond_init");
            break;
        }
        if (pthread_cond_init(&(pool->queue_not_full), NULL)){
            app_error("pthread_cond_init");
            break;
        }
        //线程描述符存储空间
        pool->pthreads = (pthread_t *)malloc(sizeof(pthread_t) * thread_num);
        if (NULL == pool->pthreads){
            app_error("malloc error");
            break;
        }
        pool->queue_close = 0;
        pool->pool_close = 0;
        int i;
        //创建线程
        for (i = 0; i < pool->thread_num; ++i){
            if (pthread_create(&(pool->pthreads[i]), NULL,
                threadpool_function, (void *)pool) < 0){
                app_error("pthread_create");
            }
        }
        return pool;
    } while (0);
    return NULL;
}

/*
 * threadpool_add_job - 想线程池中添加任务
 * @pool - 线程池地址
 * @callback_function - 回调函数
 * @arg - 回调函数参数
 * 返回 - 成功返回0 失败返回-1
 */
 int threadpool_add_job(threadpool_t *pool, callback_func_t p_callback_func, void *arg)
 {
     if (pool == NULL || p_callback_func == NULL){
         return -1;
     }

     pthread_mutex_lock(&(pool->mutex));
     // 队列满的时候就等待
     while ((pool->queue_cur_num == pool->queue_max_num) &&
        !(pool->pool_close || pool->queue_close)){
         // 等待threadpool_function发送queue_not_full信号
         pthread_cond_wait(&(pool->queue_not_full), &(pool->mutex));
     }
     // 队列关闭或者线程池关闭就退出
     if (pool->queue_close || pool->pool_close){
         pthread_mutex_unlock(&(pool->mutex));
         return -1;
     }
     //开辟空间
     job_t *pjob = (job_t *)malloc(sizeof(job_t));
     if (NULL == pjob){ //申请空间失败
         pthread_mutex_unlock(&(pool->mutex));
         return -1;
     }
     //回调函数
     pjob->callback_func = p_callback_func;
     pjob->arg = arg;
     pjob->next = NULL;
     if (pool->head == NULL){
         pool->head = pool->tail = pjob;
         // 队列空的时候，有任务来了，就通知线程池中的线程：队列非空
         pthread_cond_broadcast(&(pool->queue_not_empty));
     }
     else{
         pool->tail->next = pjob;
         pool->tail = pjob; // 把任务插入到队列的尾部
     }
     pool->queue_cur_num++;
     pthread_mutex_unlock(&(pool->mutex));

     return 0;
 }

/*
 * threadpool_destory - 销毁线程池
 * @pool - 线程池地址
 * 返回 - 永远返回0
 */
int threadpool_destory(threadpool_t *pool)
{
    if (pool == NULL){
        return 0;
    }
    pthread_mutex_lock(&(pool->mutex));
    // 线程池已经退出了，就直接返回
    if (pool->queue_close && pool->pool_close){
        pthread_mutex_unlock(&(pool->mutex));
        return 0;
    }
    pool->queue_close = 1; // 关闭任务队列，不接受新的任务了
    // 等待队列为空
    while (pool->queue_cur_num != 0){
        pthread_cond_wait(&(pool->queue_empty), &(pool->mutex));
    }
    pool->pool_close = 1; // 线程池关闭
    pthread_mutex_unlock(&(pool->mutex));

    // 唤醒线程池中正在阻塞的线程
    pthread_cond_broadcast(&(pool->queue_not_empty));
    // 唤醒添加任务的threadpool_add_job函数
    pthread_cond_broadcast(&(pool->queue_not_full));

    int i;
    for (i = 0; i < pool->thread_num; ++i){
        // 等待线程池的所有线程执行完毕
        pthread_join(pool->pthreads[i], NULL);
    }
     // 清理相关锁
    pthread_mutex_destroy(&(pool->mutex));
    pthread_cond_destroy(&(pool->queue_empty));
    pthread_cond_destroy(&(pool->queue_not_empty));
    pthread_cond_destroy(&(pool->queue_not_full));
    // 清理线程id
    free(pool->pthreads);
    //清理任务队列
    job_t *pjob;
    while (pool->head != NULL){
        pjob = pool->head;
        pool->head = pjob->next;
        free(pjob);
    }
    // 清理线程池
    free(pool);
    return 0;
}


/*
 * threadpool_function - 线程池中线程函数
 * @arg - 线程池地址
 */
void *threadpool_function(void *arg)
{
    threadpool_t *pool = (threadpool_t *)arg;
    job_t *pjob = NULL;
    while (1){
        pthread_mutex_lock(&(pool->mutex));
        // 队列为空，就等待队列非空
        while ((pool->queue_cur_num == 0) && !pool->pool_close) {
            // 等待threadpool_add_job函数发送queue_not_empty信号
            pthread_cond_wait(&(pool->queue_not_empty), &(pool->mutex));
        }
        if (pool->pool_close){ // 线程池关闭，线程就退出
            pthread_mutex_unlock(&(pool->mutex));
            pthread_exit(NULL);
        }
        // 减少一个任务
        pool->queue_cur_num--;
        pjob = pool->head;
        if (pool->queue_cur_num == 0){
            pool->head = pool->tail = NULL;
        }
        else{
            pool->head = pjob->next;
        }
        //队列中没有任务
        if (pool->queue_cur_num == 0){
            // 通知destory函数可以销毁线程池了
            pthread_cond_signal(&(pool->queue_empty));
        }
        else if (pool->queue_cur_num <= pool->queue_max_num - 1){
            // 向threadpool_add_job发送queue_not_full信号
            pthread_cond_broadcast(&(pool->queue_not_full));
        }
        pthread_mutex_unlock(&(pool->mutex));
        (*(pjob->callback_func))(pjob->arg); //线程真正要做的工作，调用回调函数
        free(pjob);
        pjob = NULL;
    }
}
