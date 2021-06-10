#include <pthread.h>
#include <iostream>
#include <errno.h>
#define SIZE 4
using namespace std;

long counter = 0;
pthread_mutex_t mtx;
void *routine(void *params) {

    for (int i= 0; i< 10000000; i++) {
        pthread_mutex_lock(&mtx);
        // as we are accesing global variable counter which is being accessed by multiple threads 
        // so to avoid race condition we need to have lock mutex
        counter++;
        pthread_mutex_unlock(&mtx);
    }
    return nullptr;
}

int main() {

    pthread_t threads[SIZE];

    pthread_mutex_init(&mtx, nullptr);
    for (int i = 0; i < SIZE ; i++) {
        if(pthread_create(threads + i , nullptr, &routine, nullptr) != 0) {
            cout<<"Thread creation failed"<<endl;
            return -1;
        }
    }
    for (int i = 0; i< SIZE ; i++) {
        if(pthread_join(*(threads + i), nullptr) != 0) {
            cout<<"Thread Joined Failed"<<endl;
            return -1;
        }
    }
    pthread_mutex_destroy(&mtx);
    cout<<"Mails "<<counter;
    return 0;
}