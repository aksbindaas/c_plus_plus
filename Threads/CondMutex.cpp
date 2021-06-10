#include <iostream>
#include <unistd.h>
#include <pthread.h>

using namespace std;
#define NUM_THREAD 

pthread_mutex_t mutextDoc;
pthread_cond_t condDoc;
int document_written = 0;
void *writer(void *param) {

    for (int i = 0 ; i< 5; i++) {
        pthread_mutex_lock(&mutextDoc);
        document_written +=80;
        cout<<"\nI have written 60 lines "<<"Available now "<<document_written;;
        pthread_mutex_unlock(&mutextDoc);
        //pthread_cond_signal(&condDoc);
        pthread_cond_broadcast(&condDoc);
        // pthread_cond_signal(&condDoc); in this case only one thread will be awaken which is not 
        // optimise way for say we are 80 lines written and only 1 threading is reading it instead of 2 
        //so pthread_cond_broadcast(&condDoc) will be use full if we have more than 1 thread
        sleep(1);
    }
    pthread_exit(nullptr);
}

void *reader (void *param) {
    pthread_mutex_lock(&mutextDoc);

    // So what will happend if we comment out 
    while(document_written < 40) {
        cout<<"\nNot enough to read waiting...!";
        pthread_cond_wait(&condDoc, &mutextDoc);
        // pthread mutex unlock 
        // wait for signal  
        // pthread lock
    }
    document_written -= 40;
    cout<<"\nI have read 40 lines "<<"Remaining "<<document_written;
    pthread_mutex_unlock(&mutextDoc);
    return nullptr;
}
int main() {

    pthread_t tWriter; 
    pthread_t tReader[5];
    
    pthread_mutex_init(&mutextDoc, nullptr);
    pthread_cond_init(&condDoc, nullptr);
    for (int i = 0; i< 5; i++) {
        if(pthread_create(&tReader[i], nullptr, &reader, nullptr) != 0) {
            perror("Unable to create reader thread");
        }
    }

    if(pthread_create(&tWriter, nullptr, &writer, nullptr) != 0) {
        perror("Unable to create writer thread");
    }

    for (int i = 0; i< 5; i++) {
        if (pthread_join(tReader[i], nullptr) != 0) {
            perror("Unable to join reader thread");
        }
    }

    if (pthread_join(tWriter, nullptr) != 0) {
        perror("Unable to join writer thread");
    }
    
    pthread_cond_destroy(&condDoc);
    pthread_mutex_destroy(&mutextDoc);
    return 0;
}