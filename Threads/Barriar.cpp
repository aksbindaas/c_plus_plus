#include <iostream>
#include <pthread.h>
#include <unistd.h>

typedef struct  {
    pthread_mutex_t mb;
    pthread_cond_t cnd;
    int count;
    int tripCount;

} pthread_barrier_t;

int pthread_barrier_init(pthread_barrier_t * b, pthread_attr_t *att, int num) {
   if(pthread_mutex_init(&b->mb, nullptr) != 0 ) {
       return -1;
   }

   if(pthread_cond_init(&b->cnd, nullptr) != 0) {
       return -1;
   }

   b->count = 0;
   b->tripCount = num;
   return 0;
}

int pthread_barrier_destroy(pthread_barrier_t *b) {
    pthread_mutex_destroy(&b->mb);
    pthread_cond_destroy(&b->cnd);
    b->count = 0;
    b->tripCount = 0;
    return 0;
}

int pthread_barrier_wait(pthread_barrier_t *b) {
    pthread_mutex_lock(&b->mb);
    ++(b->count);
    if(b->count == b->tripCount) {
        b->count = 0;
        pthread_cond_broadcast(&b->cnd);
        pthread_mutex_unlock(&b->mb);
        return 1;
    } else {
        pthread_cond_wait(&b->cnd, &b->mb);
        pthread_mutex_unlock(&b->mb);
        return 0;
    }
}

using namespace std;

pthread_barrier_t  barrier;
void *routine(void *param) {
    cout<<"Waiting threads to join ......\n";
    pthread_barrier_wait(&barrier);
    cout<<"Passed the barrier\n";
    pthread_exit(nullptr);
} 

int main() {

    pthread_t t[16];
    pthread_barrier_init(&barrier,nullptr, 4);
    for (int i= 0 ; i < 16 ; i++) {
        if(pthread_create(&t[i] , nullptr, &routine, nullptr)  != 0) {
            perror("Unable to created thread");
        }
        sleep(1);
    }

    for (int i = 0; i < 16 ; i++) {
        if (pthread_join(t[i], nullptr) != 0) {
            perror("Unbale to join the thread");
        }
    }

    pthread_barrier_destroy(&barrier);

    return 0;
}