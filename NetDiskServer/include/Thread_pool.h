#ifndef _THREADPOOL_H
#define _THREADPOOL_H
#include "packdef.h"


typedef struct
{
    void * (*task)(void*);
    void * arg;
} task_t;

typedef struct
{
    int thread_max;
    int thread_min;//消费者线程开辟的最小数量，也是每次忙线程>80%,管理者线程创建的数量
    int thread_alive;
    int thread_busy;
    int thread_shutdown;
    int thread_wait;//空闲消费者线程过多时，删除的线程数量
    int queue_max;
    int queue_cur;
    int queue_front;
    int queue_rear;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
    pthread_mutex_t lock;
    task_t * queue_task;
    pthread_t *tids;
    pthread_t manager_tid;
} pool_t;

class thread_pool
{
public:
    pool_t *Pool_create(int,int,int);
    int pool_destroy(pool_t*);
    int Producer_add(pool_t*,void *(*)(void*),void*);
    static void * Custom(void*);
    static void * Manager(void*);

    static int if_thread_alive(pthread_t);
};



#endif




