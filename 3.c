#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>

sem_t sem;

void* write_thread(void* arg){
    while(1){
        if (sem_trywait(&sem) == 0){
            int* local_data = arg;
            printf("--------I got message for you--------\n");
            for (int i = 0; i < 10; i++)
                printf("%d : %d\n", i, local_data[i]);
            printf("-------------------------------------\n");
            sem_post(&sem); 
        }
    }
    return NULL;
}   

void* read_thread(void* arg){
    while(1){
        if (sem_trywait(&sem) == 0){
            int* local_data = arg;
            int i = 0, a = 0;
            if(scanf("%d%d", &i, &a))
                local_data[i] = a;
            sem_post(&sem);
        }
    }
    return NULL ;
}

int main() {
    int data[10];
    for (int i = 0; i < 10; i++)
        data[i] = i + 1;
    
    pthread_t t1, t2;
    sem_init(&sem, 0, 1);

    pthread_create(&t1, NULL, read_thread, &data);
    pthread_create(&t2, NULL, write_thread, &data);

    pthread_exit(NULL);
}