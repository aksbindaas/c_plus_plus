#include <iostream>
#include <pthread.h>

// Problem to solve
// we have 8 players who will roll the dice and they will store dice number
//  in their index array, Now main thread will be calculate the winner 
typedef struct {
    pthread_mutex_t bm;
    pthread_cond_t bc;
    int count;
    int totalCount;


} pthread_barrier_t;

void pthread_barrier_init(pthread_barrier_t *b, int count) {
    pthread_mutex_init(&b->bm, nullptr);
    pthread_cond_init(&b->bc, nullptr);
    b->count = 0;
    b->totalCount = count;
}

void pthread_barrier_destory(pthread_barrier_t *b) {
    pthread_mutex_destroy(&b->bm);
    pthread_cond_destroy(&b->bc);
    b->count = 0;
    b->totalCount = 0;
}

void pthread_barrier_wait(pthread_barrier_t *b) {
    pthread_mutex_lock(&b->bm);
    ++(b->count);
    if(b->count == b->totalCount) {
        b->count = 0;
        pthread_cond_broadcast(&b->bc);
    } else {
        pthread_cond_wait(&b->bc, &b->bm);
    }
    pthread_mutex_unlock(&b->bm);
}


#define TOTAL_PLAYER 8
int rollOutcomes[TOTAL_PLAYER] = {0};
int status[TOTAL_PLAYER] = {0};
pthread_barrier_t b;
pthread_barrier_t c;
using namespace std;


void *rollDice(void *param) {
    int index = *(int *)param;
    rollOutcomes[index] = rand() %6 +1;
    printf("%d You have rolled wait ....\n", index);
    pthread_barrier_wait(&b);
    printf ("result is being calculated by Main thread wait---->\n");
    pthread_barrier_wait(&c);
    if(status[index] == 1) {
        printf("Index %d Won outcome %d\n",index, rollOutcomes[index]);
    } else {
        printf("Index %d Lost outcome %d\n",index, rollOutcomes[index]);
    }
    delete (int *)param;
    return nullptr;
}

int main() {
    srand(time(nullptr));
    pthread_t players[TOTAL_PLAYER];
    
    pthread_barrier_init(&b, TOTAL_PLAYER+1);
    pthread_barrier_init(&c, TOTAL_PLAYER+1);
    for (int i = 0; i< TOTAL_PLAYER ; i++) {
        int *a = new int(i);
        if(pthread_create(&players[i], nullptr, rollDice, a) != 0 ) {
            perror("Error while creating the thread");
        }
    }
    /* We want to make sure the all the player as roll the diced so we will put barrier in rollDice routing
    */
    // Calculate the max from the outcomes
    pthread_barrier_wait(&b);
    printf("Main thread Calculating the result ....\n");
    int max = 0;
    for (int i = 0; i< TOTAL_PLAYER ; i++) {
        if(rollOutcomes[i] > max) {
            max = rollOutcomes[i];
        }
    }
    // whoever got max set as winner by inititalising as 1
    for (int i= 0 ; i < TOTAL_PLAYER ; i++) {
        (rollOutcomes[i] == max)? status[i] = 1 : status[i] = 0;
    }
    printf("Calculated the result annouse the result....\n");
    pthread_barrier_wait(&c);
    pthread_exit(nullptr);
    pthread_barrier_destory(&b);
    pthread_barrier_destory(&c);
    return 0;
}