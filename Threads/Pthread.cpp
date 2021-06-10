#include <pthread.h>
#include <iostream>
using namespace std;

// intent to get sum of this array via two threads
int number [10] = {1,2,3,4,5,6,7,8,9,10};
void *routine (void *params) {
    // basically params will have start index of the array
     int index = *(int *)params;
     int sum = 0;
     for (int i = 0 ; i<  5; i++) {
            sum += number[index+ i]; 
     }
     cout<<"Local Sum "<<sum<<endl;
     // overriding sum value to passed argument to reuse the existing resource
     // and will return it
    *(int *)params = sum;
     return params;
}

int main() {
    const int size  = 2;
    pthread_t t1[size];
    for (int i=0; i < size ; i++) {
        int *a = new int(i);
        *a = i * 5;
        if(pthread_create(&t1[i],nullptr, &routine, a) != 0) {
            cout<<"Error creating thread\n";
        }
    }

    int total_sum = 0;
    for (int i=0; i < size ; i++) {
        int *tid;        
        if(pthread_join(t1[i], (void **)&tid) != 0) {
                cout<<"Error in joining thread\n";
        }
        total_sum += *tid;
        delete tid;
    }
    
    cout<<"Total Sum  "<<total_sum<<endl;
    return 0;
}