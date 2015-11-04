#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <signal.h>
#include "shared_mem.h"
#include "semun.h"

int main(void) {
    void *shared_memory = (void *)0;
    circular_buffer_st *shared_buffers;
    int sem_buffer, sem_empty, sem_full;
    long total_bytes = 0;
    int bytes_read = 0;
    FILE *fp;
    char ch;
    char tmp_buffer[TEXT_SIZE];
    
    shared_memory = attach_buffers();
    shared_buffers = (circular_buffer_st *) shared_memory;
    
    sem_buffer = get_sem_key(SEM_BUFFER, 1);
    sem_empty = get_sem_key(SEM_EMPTY, 0);
    sem_full = get_sem_key(SEM_FULL, CIRCULAR_BUFFER_SIZE - 1);

    printf("SEM_BUFFER ID: %d\n", sem_buffer);
    printf("SEM_EMPTY ID: %d\n", sem_empty);
    printf("SEM_FULL ID: %d\n", sem_full);
    
    remove("other.txt");
    fp = fopen("other.txt", "a");
    
    if (fp == NULL) {
        fprintf(stderr, "Failed to Open File\n");
    }
    while(1) {
        semaphore_p(sem_empty);
        semaphore_p(sem_buffer);
        /* 
        * Critical Section
        */
        strcpy(tmp_buffer, shared_buffers->buffers[shared_buffers->head].text);
        bytes_read = shared_buffers->buffers[shared_buffers->head].length;
        shared_buffers->head = (shared_buffers->head + 1) % CIRCULAR_BUFFER_SIZE;
//         printf("Head: %d\t BR: %d\n", shared_buffers->head,bytes_read);
//         printf("Head: %d\tTail: %d\n", shared_buffers->head, shared_buffers->tail);
        
        semaphore_v(sem_buffer);
        semaphore_v(sem_full);
        
        total_bytes += fwrite(tmp_buffer, sizeof(char), bytes_read, fp);
        
        if(bytes_read == 0)
            break;
    }
    
    fclose(fp);
    del_semvalue(sem_buffer);
    del_semvalue(sem_empty);
    del_semvalue(sem_full);
    detach_buffers(shared_memory);
    delete_buffers();
    
    printf("Total Bytes Wrote: %ld\n", total_bytes);
}