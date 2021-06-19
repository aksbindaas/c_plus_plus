#include "threadPool.h"

#include<iostream>
#include <pthread.h>
ThreadJob::ThreadJob(routine r, void * params) {
    this->r = r;
    this->args = params;
}

void ThreadJob::exec() {
    r(this->args);
}

void * ThreadPool::run(void *param) {
    ThreadPool *pool = static_cast<ThreadPool *>(param);
    while(1) {
        pthread_mutex_lock(&pool->mtxQ);
        while(pool->q.empty()) {
            pthread_cond_wait(&pool->condEmpty, &pool->mtxQ);
        }

        ThreadJob * job = pool->q.front(); pool->q.pop();
        pthread_mutex_unlock(&pool->mtxQ);
        //std::cout<<"Executing the job\n";
        job->exec();
        delete job;
    }
}

void ThreadPool::doWork(routine r, void *args) {
    //std::cout<<"Enqueing the task\n";
    pthread_mutex_lock(&mtxQ);
    q.push(new ThreadJob(r, args));
    pthread_mutex_unlock(&mtxQ);
    pthread_cond_signal(&condEmpty);
}

ThreadPool::ThreadPool(int num) {
    numThreads = num;
    workers = new pthread_t[num];
    for (int i = 0 ; i < num; i++) {
        if(pthread_create(&workers[i], nullptr, &ThreadPool::run, this) != 0) {
            //std::cout<<"Thread creation error";
        }
    }
}

ThreadPool::~ThreadPool() {
    for (int i = 0 ; i< numThreads; i++) {
        pthread_join(workers[i], nullptr);
    }
    for (int i = 0 ; i< numThreads; i++) {
        pthread_cond_signal(&condEmpty);
    }
    pthread_mutex_lock(&mtxQ);
    
    pthread_mutex_unlock(&mtxQ);
    delete [] workers;   
}
