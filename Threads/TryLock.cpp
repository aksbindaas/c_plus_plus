#include <pthread.h>
#include <iostream>
#include <unistd.h>

using namespace std;
#define CHEFS 10

// when we have multiple resources and threads wants to occupies one the resources
// then try catch will be useful, it is non blocking
pthread_mutex_t mutexStoves[4];
int stoves[4] = {100, 100, 100, 100};

void *routine(void *params) {

    for(int i=0 ; i < 4; i++) {
        if(pthread_mutex_trylock(&mutexStoves[i]) == 0) {
            int fuelNeeded = rand()%40;
            if(stoves[i] - fuelNeeded < 0  )  {
                cout<<"Not Enough Fuel in stove "<<i<<"\n";
            } else {
                stoves[i] -= fuelNeeded;
                usleep(500000);
                cout<<"Gas Remaining in stove "<<i<<"\n";
            }
            pthread_mutex_unlock(&mutexStoves[i]);
            break;
        } 
         else {
             if(i ==3) {
                cout<<"Waiting ....\n";
                usleep(300000);
                i = 0;
             }
             
         }
    }
    return nullptr;
}

int main() {
    pthread_t chefs[CHEFS];
    srand(time(nullptr));
    for (int i= 0; i< 4; i++) {
        pthread_mutex_init(&mutexStoves[i], nullptr);
    }

    for (int i =0 ; i< CHEFS ; i++) {
        if(pthread_create(&chefs[i], nullptr, &routine, nullptr) != 0) {
            cout<<"Thread creation error"<<endl;
        }   
    }

    for (int i= 0; i< CHEFS; i++) {
        if(pthread_join(chefs[i], nullptr) != 0) {
            cout<<"Thread joined failed"<<endl;
        }
    }

    for (int i= 0; i< 4; i++){
        pthread_mutex_destroy(&mutexStoves[i]);
    }
    return 0;
}