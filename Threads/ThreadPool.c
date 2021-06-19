#include <pthread.h>
#include<stdio.h>
#include <stdlib.h>
#include<unistd.h>
#define QUEUE_SIZE 1000


typedef void *(*routine)(void *);

typedef struct {
    int front, rear, size;
    void **array;
    int capacity;
}Queue;

Queue *queue_init(int capacity) {
    Queue *q = (Queue *)malloc(sizeof(Queue));
    q->array = malloc(sizeof(void *)*capacity);
    q->capacity = capacity;
    q->front = q->size =  0;
    q->rear = capacity -1;
    return q;
}

void queue_destory(Queue *q) {
    q->front = q->size =  0;
    q->rear = q->capacity -1;
    free(q->array);
    free(q);
}
int isEmpty(Queue *q) {
    return (q->size == 0);
}

int isFull(Queue *q) {
    return (q->size == q->capacity);
}

int enqueue(Queue * q , void *data) {
    if(isFull(q)) {
        return -1;
    }
    q->rear = (q->rear+1) % q->capacity;
    q->array[q->rear] = data;
    q->size = q->size+1;
    printf("Enqueued Job now size is %d\n",q->size);
    return 0;
}

void *dequeue(Queue *q) {
    if(isEmpty(q)) {
        return NULL;
    }
    void *data = q->array[q->front];
    q->front = (q->front + 1)% q->capacity;  
    q->size = q->size-1;
    printf("Dequeued Job now the size is %d\n", q->size);
    return data;
}

typedef struct{
    routine r;
    void *params;
} ThreadJob;

ThreadJob * thread_job_construct(routine r , void *arg) {
     ThreadJob *job = (ThreadJob *)malloc(sizeof(ThreadJob));
     job->r = r;
     job->params = arg;
     return job;
}

void thread_job_destroy(ThreadJob *job) {
    job->r = NULL;
    job->params = NULL;
    free(job);
}


typedef struct  {
  int num_threads;
  pthread_t *threads;
  Queue *queue;
  pthread_mutex_t mutexQueue;
  pthread_cond_t condQueue;
  pthread_cond_t condQueueFull;

}Thread_Pool;

void * thread_job (void *args) {
    Thread_Pool *pool = (Thread_Pool *)args;
    while (1) {
        pthread_mutex_lock(&pool->mutexQueue);
        while(isEmpty(pool->queue) == 1) {
            pthread_cond_wait(&pool->condQueue,&pool->mutexQueue);
         }
        ThreadJob *job = (ThreadJob*) dequeue(pool->queue);
        pthread_mutex_unlock(&pool->mutexQueue);
        pthread_cond_signal(&pool->condQueueFull);
        job->r(job->params);
        free(job);
    }
}

void thread_pool_addJob(Thread_Pool *pool,  routine r, void *param) {
    
    ThreadJob *job  = thread_job_construct(r, param);
    pthread_mutex_lock(&pool->mutexQueue);
    while(isFull(pool->queue) == 1) {
        pthread_cond_wait(&pool->condQueueFull,&pool->mutexQueue);
    }
    if (enqueue(pool->queue, job) == -1) {
        printf("Failed to insert into the queue\n");
    }
    pthread_mutex_unlock(&pool->mutexQueue);
    pthread_cond_signal(&pool->condQueue);
}

Thread_Pool * thread_pool_construct(int num_threads) {
    printf("Thread Pool Created with %d threads\n", num_threads);
    Thread_Pool * pool = (Thread_Pool *) malloc(sizeof(Thread_Pool));
    pthread_mutex_init (&pool->mutexQueue, NULL);
    pthread_cond_init(&pool->condQueue, NULL);
    pthread_cond_init(&pool->condQueueFull, NULL);

    pool->queue = queue_init(QUEUE_SIZE);
    pool->num_threads = num_threads;
    pool->threads = (pthread_t*) malloc(sizeof(pthread_t) * num_threads);
    for (int i = 0 ; i< num_threads; i++) {
        if(pthread_create(&pool->threads[i], NULL, &thread_job, pool) != 0) {
            perror("Thread creation error");
        }
    }
    return pool;
}

void thread_pool_destroy(Thread_Pool *pool) {
    pool->num_threads = 0;
    free(pool->threads);
    queue_destory(pool->queue);
    for (int i = 0 ; i< pool->num_threads; i++) {
        pthread_join(pool->threads[i],NULL);
    }
    pthread_mutex_destroy(&pool->mutexQueue);
    pthread_cond_destroy(&pool->condQueue);
    pthread_cond_destroy(&pool->condQueueFull);
    free(pool);
    printf("Thread Pool Destroyed\n");
}


char *operationStr[4] = {
    "sum +++++++++",
    "minus ---------",
    "multiply *********",
    "devide ////////"
};

void * random_operation(void *param) {
    int operation = *(int *)param;
    printf("Started %s operation\n", operationStr[operation]); 
    int randSleep = rand() % 4 + 1 ;
    switch(operation) {
        case 0: { sleep(randSleep); }
        break;
        case 1: {sleep(randSleep);}
        break;
        case 2: {sleep(randSleep);}
        break;
        case 3: { sleep(randSleep);}
        break;
    }
    printf("Done %s operation\n", operationStr[operation]); 
    free(param);
    return NULL;
}

void *addOperation(void *param) {
    Thread_Pool *pool = thread_pool_construct(100);
    while (1) {
         int* operation = (int *)malloc(sizeof(int));
         *operation = rand()%4;
         printf("Adding to queue for %s operation\n", operationStr[*operation]);
         thread_pool_addJob(pool, &random_operation, operation);
    }
    thread_pool_destroy(pool);
}

int main() {
    srand(time(NULL));
    pthread_t thread;
    pthread_create(&thread, NULL, &addOperation, NULL);
    pthread_join(thread, NULL);
    return 0;
}