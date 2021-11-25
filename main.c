#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/timeb.h>
#include "semaphores.h"
#include "shared_memory.h"

//Constants for better handling of semaphores
#define READY 0
#define READY_P1_P2 0
#define READY_P2_P1 1
#define MUTEX 0


int main(int argc, char *argv[]){

    //Ιnitialization of variables to be used
    pid_t encode1, channel, encode2, p2;
    message *shm1_block_ptr, *shm2_block_ptr, *shm3_block_ptr, *shm4_block_ptr;
    char buf[100];
    float possibility=0;
    const char failed_message[29] = ">> Failed to send message.\n";
    key_t key[4];

    //set up random function
    srand(time(NULL)); 

    if(argc<2){
        //Default possibility if there are no parameters
        possibility=5;
    }
    else
        possibility = atof(argv[1]);

    printf("possibility = %.2f %% \n", possibility);
    

    //Generate keys for the shared memories
    //Values between 10.000 and 109.999
    for(int i=0;i<4;i++){
        key[i] = rand()%100000 + 10000;	
    }


    //Create and attach 4 shared memories size of struct message
    int shm1_id=shared_create(key[0], sizeof(message));
    if(shm1_id<0){
        printf("Create Shared Memory Error\n");
        exit(1);
    }
    shm1_block_ptr=shared_attach(shm1_id);
    if(shm1_block_ptr==(message *)(-1)){
        printf("Attach Shared Memory Error\n");
        exit(1);
    }

    int shm2_id=shared_create(key[1], sizeof(message));
    if(shm2_id<0){
        printf("Create Shared Memory Error\n");
        exit(1);
    }
    shm2_block_ptr=shared_attach(shm2_id);
    if(shm2_block_ptr==(message *)(-1)){
        printf("Attach Shared Memory Error\n");
        exit(1);
    }

    int shm3_id=shared_create(key[2], sizeof(message));
    if(shm3_id<0){
        printf("Create Shared Memory Error\n");
        exit(1);
    }
    shm3_block_ptr=shared_attach(shm3_id);
    if(shm3_block_ptr==(message *)(-1)){
        printf("Attach Shared Memory Error\n");
        exit(1);
    }

    int shm4_id=shared_create(key[3], sizeof(message));
    if(shm4_id<0){
        printf("Create Shared Memory Error\n");
        exit(1);
    }
    shm4_block_ptr=shared_attach(shm4_id);
    if(shm4_block_ptr==(message *)(-1)){
        printf("Attach Shared Memory Error\n");
        exit(1);
    }

    //Generate keyshared memories semaphores
    //Values between 200.000 and 1.199.999
    for(int i=0;i<9;i++){
        key[i] = rand()%1000000 + 200000;	
    }

    //Creating and initializing the semaphores 
    //if nsems=1 ==> arg{READY}
    //if nsems=2 ==> args{READY_P1_P2, READY_P2_P1}
    int sem_p1 = sem_create(key[0], 1, "1");           
    if (sem_p1 < 0){
        printf("Create Semaphore Error\n");
        exit(0);
    }

    int sem_encode1 = sem_create(key[1], 2, "0,0");           
    if (sem_encode1 < 0){
        printf("Create Semaphore Error\n");
        exit(0);
    }

    int sem_encode2 = sem_create(key[2], 2, "0,0");           
    if (sem_encode2 < 0){
        printf("Create Semaphore Error\n");
        exit(0);
    }

    int sem_channel = sem_create(key[3], 2, "0,0");           
    if (sem_channel < 0){
        printf("Create Semaphore Error\n");
        exit(0);
    }

    int sem_p2 = sem_create(key[4], 1, "0");           
    if (sem_p2 < 0){
        printf("Create Semaphore Error\n");
        exit(0);
    }

    //Semaphores to be used as mutexes.
    int mutex_shm1 = sem_create(key[5], 1, "1");           
    if (mutex_shm1 < 0){
        printf("Create Semaphore Error\n");
        exit(0);
    }

    int mutex_shm2 = sem_create(key[6], 1, "1");           
    if (mutex_shm2 < 0){
        printf("Create Semaphore Error\n");
        exit(0);
    }

    int mutex_shm3 = sem_create(key[7], 1, "1");           
    if (mutex_shm3 < 0){
        printf("Create Semaphore Error\n");
        exit(0);
    }

    int mutex_shm4 = sem_create(key[8], 1, "1");           
    if (mutex_shm4 < 0){
        printf("Create Semaphore Error\n");
        exit(0);
    }

    
    //Parent process (P1) create process Encode1
    encode1 = fork();
    if(encode1<0){
        printf("Error in fork() encode1 child\n");
        return 1;
    }

    if(encode1==0){  //Process Encode1 - first child of parent P1
    
        //Ιnitialization of the variables to be used
        message msg_encode1;  
        unsigned char hash[MD5_DIGEST_LENGTH]; //16 bytes
        int cnt=0, i=0;

        while(true){

            //Wait signal from P1 to start
            sem_down(sem_encode1, READY_P1_P2);
            sem_down(mutex_shm1, MUTEX);

            memcpy(msg_encode1.msg, shm1_block_ptr->msg, strlen((char *)shm1_block_ptr->msg)+1);
            
            sem_up(mutex_shm1, MUTEX);

            //Then write at shared memory between processes Encode1 and Channel.
            sem_down(mutex_shm2, MUTEX);

            //Initialization of variable hash
            for(i=0;i<MD5_DIGEST_LENGTH;i++){
                hash[i]='\0';
            }
            //Create checksum 
            MD5(shm1_block_ptr->msg, strlen((char *)shm1_block_ptr->msg)+1, hash);

            //Copy message and checksum at shared memory between Encode2 and Channel.
            memcpy(shm2_block_ptr->msg, msg_encode1.msg, strlen((char *)msg_encode1.msg)+1);
            memcpy(shm2_block_ptr->checksum, hash, strlen((char *)hash)+1);
            shm2_block_ptr->retransmission_enc1=false;
            shm2_block_ptr->retransmission_enc2=false;

            sem_up(mutex_shm2, MUTEX);
            sem_up(sem_channel, READY_P1_P2);

            if(!memcmp(msg_encode1.msg, "TERM", 4)){
                break;
            }

            //Break when the message has transmitted successful and no need for retransmission
            while(true){

                bool flag_exit_from_loop=false;

                //Wait signal from Channel to continue
                sem_down(sem_encode1, READY_P2_P1);
                sem_down(mutex_shm2, MUTEX);

                //Initialization of variable hash
                for(i=0;i<MD5_DIGEST_LENGTH;i++){
                    hash[i]='\0';
                }
                //Create checksum
                MD5(shm2_block_ptr->msg, strlen((char *)shm2_block_ptr->msg)+1, hash);
               

                //Distinguish 3 cases when the process receives a signal from the process Channel.
                //1) The process retransmitted the message.
                //2) The process request for retransmission from the sender of the message (sender is Encode2).
                //3) Succesful case. Transfer the message at process P1 to display the message at the output.

                //Retransmitted the message
                if(shm2_block_ptr->retransmission_enc1==true){
                    
                    memcpy(shm2_block_ptr->msg, msg_encode1.msg, strlen((char *)msg_encode1.msg)+1);
                    MD5(shm1_block_ptr->msg, strlen((char *)shm1_block_ptr->msg)+1, hash);
                    memcpy(shm2_block_ptr->checksum, hash, strlen((char *)hash)+1);
                    shm2_block_ptr->retransmission_enc1=false;
                    shm2_block_ptr->retransmission_enc2=false;

                    sem_up(mutex_shm2, MUTEX);
                    sem_up(sem_channel, READY_P1_P2);
                        
                }
                //Request for retransmission
                else if(memcmp(shm2_block_ptr->checksum, hash, strlen((char *)hash)+1)){

                    shm2_block_ptr->retransmission_enc2=true;
                    cnt++;

                    printf("%s", failed_message);
                    printf(">> 'Encode1': Request for retransmission. %d Attempt...\n\n", cnt);

                    sem_up(mutex_shm2, MUTEX);
                    sem_up(sem_channel, READY_P1_P2);
                }
                else{   //Transfer message at process P1.
                    
                    memcpy(msg_encode1.msg, shm2_block_ptr->msg, strlen((char *)shm2_block_ptr->msg)+1);
                    sem_up(mutex_shm2, MUTEX);

                    //Then write at shared memory between processes Encode1 and P1.
                    sem_down(mutex_shm1, MUTEX);

                    memcpy(shm1_block_ptr->msg, msg_encode1.msg, strlen((char *)msg_encode1.msg)+1);
                    shm2_block_ptr->retransmission_enc1=false;
                    shm2_block_ptr->retransmission_enc2=false;

                    flag_exit_from_loop=true;
                    cnt=0;

                    sem_up(mutex_shm1, MUTEX);
                    //Signal to continue Process P1
                    sem_up(sem_p1, READY);
                }

                //Exit from the internal loop
                if(flag_exit_from_loop==true){
                    break;
                }
            }


            if(!memcmp(msg_encode1.msg, "TERM", 4)){
                break;
            }        
        }

        exit(0);
    }
    else{
        
        //Parent process (P1) create second process Channel
        channel = fork();
        if(channel<0){
            printf("Error in fork() channel child\n");
            return 1;
        }

        if (channel == 0) { //Process Channel - second child of parent P1
            
            //Channel process create first child (Encode2)
            encode2 = fork();
            if(encode2<0){
                printf("Error in fork() encode2 child\n");
                return 1;
            }


            if(encode2==0){ //Process Encode2 - first child of Channel process

                //Ιnitialization of the variables to be used
                message msg_encode2;
                unsigned char hash[MD5_DIGEST_LENGTH];  //MD5_DIGEST_LENGTH
                int cnt=0, i=0;
                
                while(true){

                    //Break when the message has transmitted successful and no need for retransmission
                    while(true){

                        bool flag_exit_from_loop=false;

                        //Wait signal from Channel to start
                        sem_down(sem_encode2, READY_P1_P2);
                        sem_down(mutex_shm3, MUTEX);
                        
                        //Initialization of variable hash
                        for(i=0;i<MD5_DIGEST_LENGTH;i++){
                            hash[i]='\0';
                        }
                        MD5(shm3_block_ptr->msg, strlen((char *)shm3_block_ptr->msg)+1, hash); 
                
                        //Distinguish 3 cases when the process receives a signal from the process Channel.
                        //1) The process retransmitted the message.
                        //2) The process request for retransmission from the sender of the message (sender is Encode1).
                        //3) Succesful case. Transfer the message at process P2 to display the message at the output.

                        //Retransmitted the message
                        if(shm3_block_ptr->retransmission_enc2==true){
                            
                            memcpy(shm3_block_ptr->msg, msg_encode2.msg, strlen((char *)msg_encode2.msg)+1);
                            MD5(shm4_block_ptr->msg, strlen((char *)shm4_block_ptr->msg)+1, hash);
                            memcpy(shm3_block_ptr->checksum, hash, strlen((char *)hash)+1);
                            shm3_block_ptr->retransmission_enc1=false;
                            shm3_block_ptr->retransmission_enc2=false;
                            
                            sem_up(mutex_shm3, MUTEX);
                            sem_up(sem_channel, READY_P2_P1);         
                        }
                        //Request for retransmission
                        else if(memcmp(shm3_block_ptr->checksum, hash, strlen((char *)hash)+1)){                        
                            
                            shm3_block_ptr->retransmission_enc1=true;
                            cnt++;
                            
                            printf("%s", failed_message);
                            printf(">> 'Encode2': Request for retransmission. %d Attempt...\n\n", cnt);
                            
                            sem_up(mutex_shm3, MUTEX);
                            sem_up(sem_channel, READY_P2_P1);
                        }
                        else{   //Transfer message at process P2.

                            memcpy(msg_encode2.msg, shm3_block_ptr->msg, strlen((char *)shm2_block_ptr->msg)+1);
                            sem_up(mutex_shm3, MUTEX);
        
                            //Then write at shared memory between processes Channel and Encode2.
                            sem_down(mutex_shm4, MUTEX);

                            memcpy(shm4_block_ptr->msg, msg_encode2.msg, strlen((char *)msg_encode2.msg)+1);
                            shm3_block_ptr->retransmission_enc1=false;
                            shm3_block_ptr->retransmission_enc2=false;

                            flag_exit_from_loop=true;
                            cnt=0;

                            sem_up(mutex_shm4, MUTEX);
                            //Signal to continue Process P2
                            sem_up(sem_p2, READY);
                        }

                        if(flag_exit_from_loop==true){
                            break;
                        }
                    }


                    if(!memcmp(msg_encode2.msg, "TERM", 4)){
                        break;
                    }


                    //Wait signal from Channel to continue
                    sem_down(sem_encode2, READY_P2_P1);  
                    sem_down(mutex_shm4, MUTEX);
    
                    memcpy(msg_encode2.msg, shm4_block_ptr->msg, strlen((char *)shm4_block_ptr->msg)+1);                    

                    sem_up(mutex_shm4, MUTEX);

                    //Then write at shared memory between processes Channel and Encode2.
                    sem_down(mutex_shm3, MUTEX);

                    //Initialization of variable hash
                    for(i=0;i<MD5_DIGEST_LENGTH;i++){
                        hash[i]='\0';
                    }
                    MD5(shm4_block_ptr->msg, strlen((char *)shm4_block_ptr->msg)+1, hash);

                    memcpy(shm3_block_ptr->msg, msg_encode2.msg, strlen((char *)msg_encode2.msg)+1);
                    memcpy(shm3_block_ptr->checksum, hash, strlen((char *)hash)+1);
                    shm3_block_ptr->retransmission_enc1=false;
                    shm3_block_ptr->retransmission_enc2=false;

                    sem_up(mutex_shm3, MUTEX);
                    sem_up(sem_channel, READY_P2_P1);

                    if(!memcmp(msg_encode2.msg, "TERM", 4)){
                        break;
                    }        
                }

                exit(0);
            }
            else{
                
                p2 = fork();
                if(p2<0){
                    printf("Error in fork() p2 child\n");
                    return 1;
                }
                if(p2==0){ //Process P2 - second child of Channel process

                    char buf2[100];
                    message msg_p2;

                    while(true){

                        sem_down(sem_p2, READY);
                        sem_down(mutex_shm4, MUTEX);

                        memcpy(msg_p2.msg, shm4_block_ptr->msg, strlen((char *)shm4_block_ptr->msg)+1);
                        printf("\nProcess P2 print: %s\n\n", msg_p2.msg);
                        printf("-----------------------------------------------------------------\n\n");
                        
                        if(!memcmp(msg_p2.msg, "TERM", 4)){
                            printf("Τhe program has been shut down.\nExit...\n\n");
                            break;
                        }  

                        //When ready wait for input from user and
                        //then write at shared memory between processes P2 and Encode2
                        printf("\nProcess P2 wait for input: ");
                        
                        //wait input from user
                        scanf("%[^\n]%*c", buf2);

                        //If length of input string > 100 terminate the program
                        if(strlen(buf2)>100){
                            printf("\nError! The maximum length of string is 100. Exit...\n");
                            memcpy(buf2, "TERM", 5);
                        }

                        memcpy(msg_p2.msg, buf2, strlen(buf2)+1);
                        memcpy(shm4_block_ptr->msg, msg_p2.msg, strlen((char *)msg_p2.msg)+1); 

                        sem_up(mutex_shm4, MUTEX);
                        //Signal Encode2 process
                        sem_up(sem_encode2, READY_P2_P1);  

                        if(!memcmp(msg_p2.msg, "TERM", 4)){
                            break;
                        }  
                    }

                    exit(0);

                }
                else{   //Process Channel - senond child of parent P1 

                    char buf_channel[100];
                    message msg_channel;
                    float random_number=0.0;
                    int i=0;

                    while(true){
                        
                        //Wait signal from Encode1 to continue
                        sem_down(sem_channel, READY_P1_P2);
                        sem_down(mutex_shm2, MUTEX);

                        memcpy(msg_channel.msg, shm2_block_ptr->msg, strlen((char *)shm2_block_ptr->msg)+1);
                        memcpy(buf_channel, msg_channel.msg, strlen((char *)msg_channel.msg)+1);

                        //Change characters based on possibility
                        i=0;
                        while(msg_channel.msg[i] != '\0'){     /* Stop looping when we reach the null-character. */

                            //Not change characters if the signal is "TERM" or there is a request for message retransmission
                            if(!memcmp(msg_channel.msg,"TERM", 4) || shm2_block_ptr->retransmission_enc1==true || shm2_block_ptr->retransmission_enc2==true){
                                break;
                            }
                            random_number= (float)rand()/(float)(RAND_MAX/100);
                            
                            if(random_number<=possibility){
                                
                                msg_channel.msg[i]=msg_channel.msg[i]+1;
                            }
                            i++;
                        }
                        
                        if(memcmp(buf_channel, msg_channel.msg, strlen((char *)msg_channel.msg)+1)){

                            printf("\n>> Channel process has changed the original message. Direction: (P1-->P2)\n");
                        }                   
                        sem_up(mutex_shm2, MUTEX);


                        //Then write at shared memory between processes Channel and Encode2.
                        sem_down(mutex_shm3, MUTEX);

                        memcpy(shm3_block_ptr->msg, msg_channel.msg, strlen((char *)msg_channel.msg)+1);
                        memcpy(shm3_block_ptr->checksum, shm2_block_ptr->checksum, strlen(shm2_block_ptr->checksum)+1);
                        shm3_block_ptr->retransmission_enc1 = shm2_block_ptr->retransmission_enc1;
                        shm3_block_ptr->retransmission_enc2 = shm2_block_ptr->retransmission_enc2;

                        sem_up(mutex_shm3, MUTEX);
                        //Signal Encode2 process
                        sem_up(sem_encode2, READY_P1_P2);

                        if(!memcmp(msg_channel.msg, "TERM", 4)){
                            break;
                        }


                        //Wait signal from Encode2 to continue
                        sem_down(sem_channel, READY_P2_P1); 
                        sem_down(mutex_shm3, MUTEX);

                        memcpy(msg_channel.msg, shm3_block_ptr->msg, strlen((char *)shm3_block_ptr->msg)+1);
                        memcpy(buf_channel, msg_channel.msg, strlen((char *)msg_channel.msg)+1);

                        //Change characters based on possibility
                        i=0;
                        while(msg_channel.msg[i] != '\0'){      /* Stop looping when we reach the null-character. */

                            //Not change characters if the signal is "TERM" or there is a request for message retransmission
                            if(!memcmp(msg_channel.msg,"TERM", 4) || shm3_block_ptr->retransmission_enc1==true || shm3_block_ptr->retransmission_enc2==true){
                                break;
                            }

                            random_number= (float)rand()/(float)(RAND_MAX/100);
                            
                            if(random_number<=possibility){
                                
                                msg_channel.msg[i]=msg_channel.msg[i]+1;
                            }
                            i++;
                        }

                        if(memcmp(buf_channel, msg_channel.msg, strlen((char *)msg_channel.msg)+1)){

                            printf("\n>> Channel process has changed the original message. Direction: (P2-->P1)\n");
                        }
                        
                        sem_up(mutex_shm3, MUTEX);

                        //Then write at shared memory between processes Encode1 and Channel.
                        sem_down(mutex_shm2, MUTEX);

                        memcpy(shm2_block_ptr->msg, msg_channel.msg, strlen((char *)msg_channel.msg)+1);
                        memcpy(shm2_block_ptr->checksum, shm3_block_ptr->checksum, strlen(shm3_block_ptr->checksum)+1);
                        shm2_block_ptr->retransmission_enc1 = shm3_block_ptr->retransmission_enc1;
                        shm2_block_ptr->retransmission_enc2 = shm3_block_ptr->retransmission_enc2;

                        sem_up(mutex_shm2, MUTEX);
                        //Signal Encode1
                        sem_up(sem_encode1, READY_P2_P1);

                        if(!memcmp(msg_channel.msg, "TERM", 4)){
                            break;
                        }
                    }


                    //Exit...
                    int status;
                    pid_t child_pid[2];
                    //Channel process wait for its children Encode2 and P2.
                    for(int i=0;i<2;i++){
                        child_pid[i] = wait(&status);
                    }
                    printf("Return children of 'Channel' parent process with PID = %d and %d\n\n", child_pid[0], child_pid[1]);

                    exit(0);  
                }
            }

        }else{  //Process P1 - Parent 

            message msg_p1;
        
            while(true){

                sem_down(sem_p1, READY);
                sem_down(mutex_shm1, MUTEX);
                
                printf("\nProcess P1 wait for input: ");
                //wait input from user
                scanf("%[^\n]%*c", buf);

                //If length of input string > 100 terminate the program
                if(strlen(buf)>100){
                    printf("\nError! The maximum length of string is 100. Exit...\n");
                    memcpy(buf, "TERM", 5);
                }
            
                memcpy(msg_p1.msg, buf, strlen(buf)+1);
                memcpy(shm1_block_ptr->msg, msg_p1.msg, strlen((char *)msg_p1.msg)+1);

                sem_up(mutex_shm1, MUTEX);
                sem_up(sem_encode1, READY_P1_P2);

                if(!memcmp(msg_p1.msg, "TERM", 4)){
                    break;
                }

                //Wait signal from Encode1 to continue
                sem_down(sem_p1, READY);
                sem_down(mutex_shm1, MUTEX);
    
                memcpy(msg_p1.msg, shm1_block_ptr->msg, strlen((char *)shm1_block_ptr->msg)+1);
                printf("\nProcess P1 print: %s\n\n", msg_p1.msg);
                printf("-----------------------------------------------------------------\n\n");

                sem_up(mutex_shm1, MUTEX);
                sem_up(sem_p1, READY);      

                if(!memcmp(msg_p1.msg, "TERM", 4)){
                    printf("Τhe program has been shut down.\nExit...\n\n");
                    break;
                }
            }
        }
    }

    
    if(encode1!=0){  //P1 process wait for its children Encode1 and Channel.

        int status;
        pid_t child_pid[2];

        for(int i=0;i<2;i++){
            child_pid[i] = wait(&status);
        }
        printf("Return children of 'P1' parent process with PID = %d and %d\n\n", child_pid[0], child_pid[1]);
    }

    //Remove all semaphores
    sem_remove(sem_p1);
    sem_remove(sem_encode1);
    sem_remove(sem_channel);
    sem_remove(sem_encode2);
    sem_remove(sem_p2);
    sem_remove(mutex_shm1);
    sem_remove(mutex_shm2);
    sem_remove(mutex_shm3);
    sem_remove(mutex_shm4);


    //Detach Shared Memories
    if(shared_detach(shm1_block_ptr)==-1){         	         
        printf("Error Detach Shared Memory\n");
        exit(2);
    }
    if(shared_detach(shm2_block_ptr)==-1){
        printf("Error Detach Shared Memory\n");
        exit(2);
    }
    if(shared_detach(shm3_block_ptr)==-1){                  
        printf("Error Detach Shared Memory\n");
        exit(2);
    }

    if(shared_detach(shm4_block_ptr)==-1){                  
        printf("Error Detach Shared Memory\n");
        exit(2);
    }

    //Remove Shared Memories
    if(shared_remove(shm1_id)==-1){
        printf("Error Remove Shared Memory\n");
        exit(3);
    }

    if(shared_remove(shm2_id)==-1){                  
        printf("Error Remove Shared Memory\n");
        exit(3);
    }
    if(shared_remove(shm3_id)==-1){
        printf("Error Remove Shared Memory\n");
        exit(3);
    }
    if(shared_remove(shm4_id)==-1){
        printf("Error Remove Shared Memory\n");
        exit(3);
    }
}