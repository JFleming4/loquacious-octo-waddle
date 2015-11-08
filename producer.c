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
    int bytes_to_copy;
    int bytes_left;
    FILE *fp;
    char tmp_buffer[BUFSIZ];
    char ch;
    struct timespec tms;
    int64_t start_ms, end_ms;

    // Set up the shared memory and set the correct start values.
    shared_memory = attach_buffers();
    shared_buffers = (circular_buffer_st *) shared_memory;
    shared_buffers->head = 0;
    shared_buffers->tail = 0;

    // Set up the 3 semaphores
    sem_buffer = get_sem_key(SEM_BUFFER, 1);
    sem_empty = get_sem_key(SEM_EMPTY, 0);
    sem_full = get_sem_key(SEM_FULL, CIRCULAR_BUFFER_SIZE - 1);

    printf("SEM_BUFFER ID: %d\n", sem_buffer);
    printf("SEM_EMPTY ID: %d\n", sem_empty);
    printf("SEM_FULL ID: %d\n", sem_full);

    // Open the test text file for reading
    fp = fopen("file.txt", "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to Open File\n");
    }
    fseek(fp, SEEK_SET, 0);
    
    
    // Get the start time in Microseconds
    if (clock_gettime(CLOCK_REALTIME,&tms)) {
        return -1;
    }
    start_ms = tms.tv_sec * 1000000 + tms.tv_nsec/1000;
    if (tms.tv_nsec % 1000 >= 500) {
        ++start_ms;
    }
    
    while(1) {
        // Read in some characters and transfer them to our circular buffers
        bytes_read = fread(tmp_buffer, sizeof(char), BUFSIZ, fp);
        for (bytes_left = bytes_read; bytes_left > 0; bytes_left -= bytes_to_copy) {
            bytes_to_copy = (bytes_left < TEXT_SIZE) ? bytes_left : TEXT_SIZE;
            
            semaphore_p(sem_full);
            semaphore_p(sem_buffer);
            /*
             * Critical Section
             */
            // Copy bytes to a buffer in the shared memory
            for (int i = 0; i < bytes_to_copy; i++) { 
                ch = tmp_buffer[i + (bytes_read - bytes_left)];
                shared_buffers->buffers[shared_buffers->tail].text[i] = ch;
            }
            // Update the length and tail of our Circular Buffer
            shared_buffers->buffers[shared_buffers->tail].length = bytes_to_copy;
            shared_buffers->tail = (shared_buffers->tail + 1) % CIRCULAR_BUFFER_SIZE;
//             printf("Tail: %d\tHead: %d\n", shared_buffers->tail, shared_buffers->head);
            /*
             * End Critical Section
             */
            semaphore_v(sem_buffer);
            semaphore_v(sem_empty);
        }

        total_bytes += bytes_read;

        // If you read 0 bytes set the next buffer to 0 to end consumer
        if (bytes_read == 0) {
            semaphore_p(sem_full);
            semaphore_p(sem_buffer);
            shared_buffers->buffers[shared_buffers->tail].length = 0;
            shared_buffers->tail = (shared_buffers->tail + 1) % CIRCULAR_BUFFER_SIZE;
            semaphore_v(sem_buffer);
            semaphore_v(sem_empty);
            break;
        }
    }

    // Close the file and detach the shared memory
    fclose(fp);
    detach_buffers(shared_memory);
    
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
    sleep(1);
}
