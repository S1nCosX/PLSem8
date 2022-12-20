/* fork-example-1.c */

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void* create_shared_memory(size_t size) {
  return mmap(NULL,
              size,
              PROT_READ | PROT_WRITE,
              MAP_SHARED | MAP_ANONYMOUS,
              -1, 0);
}


int main() {
    int* shmem = (int*) create_shared_memory(10 * sizeof(int));


    for (int i = 0; i < 10; i++)
        shmem[i] = i + 1;

    pid_t pid = fork();

    if (pid == 0) { // child process 
        int i = 0, a = 0;
        if(scanf("%d%d", &i, &a))
            shmem[i] = a;
    } else {
        while(!wait(NULL));
        for (int i = 0; i < 10; i++)
            printf("%d : %d\n", i, shmem[i]);
    }
}