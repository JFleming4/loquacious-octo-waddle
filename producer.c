#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <signal.h>
#include "shared_mem.h"
#include "semun.h"

void cleanup(int sig) {
//     fclose(fp);
//     detach_buffers(shared_memory);
    exit(EXIT_SUCCESS);
}

int main(void) {
    void *shared_memory = (void *)0;
    circular_buffer_st *shared_buffers;
    int sem_buffer, sem_empty, sem_full;
    long total_bytes = 0;
    int bytes_read = 0;
    FILE *fp;
    char ch;
    char tmp_buffer[TEXT_SIZE];
    struct sigaction sigs;
    
    sigs.sa_handler = cleanup;
    sigemptyset(&sigs.sa_mask);
    sigs.sa_flags = 0;
    
    sigaction(SIGTERM, &sigs, 0);
    sigaction(SIGINT, &sigs, 0);
    
    shared_memory = attach_buffers();
    shared_buffers = (circular_buffer_st *) shared_memory;
    shared_buffers->head = 0;
    shared_buffers->tail = 0;
    
    sem_buffer = get_sem_key(SEM_BUFFER, 1);
    sem_empty = get_sem_key(SEM_EMPTY, 0);
    sem_full = get_sem_key(SEM_FULL, CIRCULAR_BUFFER_SIZE - 1);
    
    printf("SEM_BUFFER ID: %d\n", sem_buffer);
    printf("SEM_EMPTY ID: %d\n", sem_empty);
    printf("SEM_FULL ID: %d\n", sem_full);
    fp = fopen("file.txt", "r");
    
    if (fp == NULL) {
        fprintf(stderr, "Failed to Open File\n");
    }
    fseek(fp, SEEK_SET, 0);
    while(1) {
        bytes_read = fread(tmp_buffer, sizeof(char), TEXT_SIZE, fp);
                
        semaphore_p(sem_full);
        semaphore_p(sem_buffer);
        /* 
        * Critical Section
        */
        strcpy(shared_buffers->buffers[shared_buffers->tail].text, tmp_buffer);
        shared_buffers->buffers[shared_buffers->tail].length = bytes_read;
        shared_buffers->tail = (shared_buffers->tail + 1) % CIRCULAR_BUFFER_SIZE;
//         printf("Tail: %d\tHead: %d\n", shared_buffers->tail, shared_buffers->head);
        
        semaphore_v(sem_buffer);
        semaphore_v(sem_empty);
        
        total_bytes += bytes_read;
       
        if (bytes_read == 0)
            break;
        
    }
    fclose(fp);
    detach_buffers(shared_memory);
    
    printf("Total Bytes Read: %ld\n", total_bytes);
}