#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>
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
    struct timespec tms;
    int64_t start_ms, end_ms;

    // Set up the shared memory and set the correct start values.
    shared_memory = attach_buffers();
    shared_buffers = (circular_buffer_st *) shared_memory;
    
    // Set up the 3 semaphores
    sem_buffer = get_sem_key(SEM_BUFFER, 1);
    sem_empty = get_sem_key(SEM_EMPTY, 0);
    sem_full = get_sem_key(SEM_FULL, CIRCULAR_BUFFER_SIZE - 1);

    printf("SEM_BUFFER ID: %d\n", sem_buffer);
    printf("SEM_EMPTY ID: %d\n", sem_empty);
    printf("SEM_FULL ID: %d\n", sem_full);

    // Clear the destination file and open for appending
    remove("other.txt");
    fp = fopen("other.txt", "a");
    if (fp == NULL) {
        fprintf(stderr, "Failed to Open File\n");
    }
    
    
    // Get the start time in Microseconds
    if (clock_gettime(CLOCK_REALTIME,&tms)) {
        return -1;
    }
    start_ms = tms.tv_sec * 1000000 + tms.tv_nsec/1000;
    if (tms.tv_nsec % 1000 >= 500) {
        ++start_ms;
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
        /*
         * End Critical Section
         */
        semaphore_v(sem_buffer);
        semaphore_v(sem_full);

        if(bytes_read == 0)
            break;
        
        total_bytes += fwrite(tmp_buffer, sizeof(char), bytes_read, fp);
    }

    // Clean up the shared memory, close the file and delete the semaphores
    fclose(fp);
    del_semvalue(sem_buffer);
    del_semvalue(sem_empty);
    del_semvalue(sem_full);
    detach_buffers(shared_memory);
    delete_buffers();

    // Get the end time in Microseconds
    if (clock_gettime(CLOCK_REALTIME,&tms)) {
        return -1;
    }
    end_ms = tms.tv_sec * 1000000 + tms.tv_nsec/1000;
    if (tms.tv_nsec % 1000 >= 500) {
        ++end_ms;
    }
    
    printf("Total Bytes Read: %ld\n", total_bytes);
    printf("Microseconds: %"PRId64"\n", end_ms - start_ms);
}
