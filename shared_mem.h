#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/shm.h>

#define TEXT_SIZE 128
#define CIRCULAR_BUFFER_SIZE 100

typedef struct{
    int length;
    char text[TEXT_SIZE];
}shared_use_st;

typedef struct{
    int tail;
    int head;
    shared_use_st buffers[CIRCULAR_BUFFER_SIZE];
}circular_buffer_st;

void * attach_buffers(void) {
    void *shared_memory = (void *)0;
    int shmid;

    shmid = shmget((key_t)1234, sizeof(circular_buffer_st), 0666 | IPC_CREAT);

    if( shmid == -1) {
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }

     shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (void *)-1) {
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE);
    }
    printf("Memory attached at %lX\n", (long)shared_memory);

    return shared_memory;
}

void detach_buffers(void *address) {
    printf("Memory detached from %lX\n", (long)address);
    if (shmdt(address) == -1) {
        fprintf(stderr, "shmdt failed: %d\n",errno);
        exit(EXIT_FAILURE);
    }
}

void delete_buffers(void){
    int id = shmget((key_t)1234, sizeof(circular_buffer_st), 0666);
    if (shmctl(id, IPC_RMID, 0) == -1) {
        fprintf(stderr, "shmctl(IPC_RMID) failed\n");
        exit(EXIT_FAILURE);
    }
}
