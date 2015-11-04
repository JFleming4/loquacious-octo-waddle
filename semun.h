#include <sys/sem.h>

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
    /* union semun is defined by including <sys/sem.h> */
#else
    /* according to X/OPEN we have to define it ourselves */
    union semun {
        int val;                    /* value for SETVAL */
        struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
        unsigned short int *array;  /* array for GETALL, SETALL */
        struct seminfo *__buf;      /* buffer for IPC_INFO */
    };
#endif
    
#define SEM_BUFFER 4321
#define SEM_FULL 4132
#define SEM_EMPTY 3421

int get_sem_key(int key, int val);
int set_semvalue(int sem_id, int val);
void del_semvalue(int sem_id);
int semaphore_p(int sem_id);
int semaphore_v(int sem_id);

int get_sem_key(int key, int val) {
    int id = semget((key_t) key, 1, 0666);
    if (id == -1) {
        id = semget((key_t) key, 1, 0666 | IPC_CREAT);
        if (!set_semvalue(id, val)) {
            fprintf(stderr, "Failed to initialize semaphore: %d\n", key);
            exit(EXIT_FAILURE);
        }
    }
    return id;
}

/* The function set_semvalue initializes the semaphore using the SETVAL command in a
 semctl call. We need to do this before we can use the semaphore. */

int set_semvalue(int sem_id, int val)
{
    union semun sem_union;

    sem_union.val = val;
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) return(0);
    return(1);
}

/* The del_semvalue function has almost the same form, except the call to semctl uses
 the command IPC_RMID to remove the semaphore's ID. */

void del_semvalue(int sem_id)
{
    union semun sem_union;
    
    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
        fprintf(stderr, "Failed to delete semaphore: %d\n", sem_id);
}

/* semaphore_p changes the semaphore by -1 (waiting). */

int semaphore_p(int sem_id)
{
    struct sembuf sem_b;
    
    sem_b.sem_num = 0;
    sem_b.sem_op = -1; /* P() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_p failed: %d\n", sem_id);
        exit(EXIT_FAILURE);
    }
    return(1);
}

/* semaphore_v is similar except for setting the sem_op part of the sembuf structure to 1,
 so that the semaphore becomes available. */

int semaphore_v(int sem_id)
{
    struct sembuf sem_b;
    
    sem_b.sem_num = 0;
    sem_b.sem_op = 1; /* V() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_v failed: %d\n", sem_id);
        exit(EXIT_FAILURE);
    }
    return(1);
}