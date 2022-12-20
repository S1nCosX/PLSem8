#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>

sem_t sem;

enum{// для выхода
	OUT = 1,
	CONTINUE = 0
};

void* write_thread(void* arg){
    while(1){
        if (sem_trywait(&sem) == 0){
            int* local_data = arg;
        	if (local_data[10] == OUT){// для выхода
            	sem_post(&sem);
        		return NULL;
			}
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
            if(scanf("%d%d", &i, &a)){
            	if (i == -1){// для выхода
            		local_data[10] = OUT;            		
		            sem_post(&sem);
		            return NULL;
				}
                local_data[i] = a;
			}
            sem_post(&sem);
        }
    }
    return NULL ;
}

int main() {
    int data[11];
    for (int i = 0; i < 10; i++)
        data[i] = i + 1;
        
    data[10] = CONTINUE; // для выхода
    
	pthread_t t1, t2;
    sem_init(&sem, 0, 1);

    pthread_create(&t1, NULL, read_thread, &data);
    pthread_create(&t2, NULL, write_thread, &data);

    pthread_exit(NULL);
}
