#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <pthread.h>
#include <functional>
#include <queue>

using routine = std::function<void *(void *)>;
class ThreadJob {
private: 
    routine r;
    void * args;
public:
    ThreadJob(routine r, void * params);
    void exec();
};

class ThreadPool {
    int numThreads;
    pthread_t *workers;
    std::queue<ThreadJob *>q;
    pthread_mutex_t mtxQ = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t condEmpty = PTHREAD_COND_INITIALIZER;
public:
    static void *run(void *param);
    ThreadPool(int num);
    void doWork(routine r, void *args);
    ~ThreadPool();
};



#endif // __THREADPOOL_H__