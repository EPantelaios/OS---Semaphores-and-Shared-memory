#ifndef __SEMAPHORES__
#define __SEMAPHORES__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>

//For semctl() function
union semun {                 
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

/*Create semaphores(Arguments: key, number_of_sems,
                               value of each sem)*/
int sem_create(key_t key, int nsems, char *values);

//Wait semaphore
int sem_down(int semid, int sem_num); //Down the semaphore

//Signal semaphore
int sem_up(int semid, int sem_num);  //Up the semaphore

//Remove semaphore
int sem_remove(int semid);

#endif