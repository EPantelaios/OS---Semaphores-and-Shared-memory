#include "shared_memory.h"

#define IPC_RESULT_ERROR (-1)

int shared_create(key_t key, size_t size){

    if(key<0){

      printf("Error in key!\n");
      return IPC_RESULT_ERROR;
    }
    return shmget(key, size, IPC_CREAT | 0666);
}

message *shared_attach(int shared_id){

    return shmat(shared_id, (const void *)0, 0);
}

int shared_detach(message *shm_ptr){

    return shmdt(shm_ptr);
}

int shared_remove(int shared_id){
    
    return shmctl(shared_id, IPC_RMID, 0);
}