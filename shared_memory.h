#ifndef __SHARED_MEMORY__
#define __SHARED_MEMORY__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <openssl/md5.h>

typedef struct message{

    uint8_t msg[101];
    char checksum[MD5_DIGEST_LENGTH+1];
    bool retransmission_enc1; //If true Encode1 process retransmitted the message
    bool retransmission_enc2; //If true Encode2 process retransmitted the message
}message;

//Create shared memory
int shared_create(key_t key, size_t size); //Creating shared memory 

//Return a pointer to the shared memory segment
message *shared_attach(int shared_id);

//Detach the shared memory
int shared_detach(message *shm_ptr);

//Remove shared memory
int shared_remove(int shared_id);

#endif