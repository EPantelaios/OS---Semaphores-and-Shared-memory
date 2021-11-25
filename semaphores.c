#include "semaphores.h"

int sem_create(key_t key, int nsems, char *values){

    int semid;
    union semun arg;

    if ((key < 0) || (nsems <= 0)){

        return -1;
    }

    semid = semget(key, nsems, 0666 | IPC_CREAT);
    //Check for errors
    if (semid < 0){
        
        return -1;
    }

    int cnt=0, i=0, current_sem=0;
    //Check and appropriately handle the values of the semaphores 
    while(true){

        char tmp_char[20];
        i=0;

        //Separate values with comma
        while(values[cnt]!=','){
            
            tmp_char[i] = values[cnt];
            
            if(values[cnt+1]==',' || values[cnt+1]=='\0')
                tmp_char[i+1]='\0';
                
            cnt++;
            i++;

            if(values[cnt]=='\0'){

                break;
            }
        }

        arg.val=atoi(tmp_char);
        
        int result = semctl(semid, current_sem, SETVAL, arg);
        if (result < 0){
            printf("semctl() function error.\n\n");
            return -2;
        }

        if(values[cnt]=='\0'){

            break;
        }

        cnt++;
        current_sem++;
    }

    if(nsems==current_sem+1){

        return semid;
    }
    else{

        printf("Error! Some semaphores might not have been initialized.\n");
        return -3;
    }
}


int sem_down(int semid,int sem_num){
    struct sembuf sb;

    if((semid < 0) || (sem_num < 0)){

        return -1;
    }

    sb.sem_num = sem_num;
    sb.sem_op = -1;
    sb.sem_flg = 0;

    return semop(semid,&sb,1);
}


int sem_up(int semid, int sem_num){
    struct sembuf sb;

    if((semid < 0) || (sem_num < 0)){

        return -1;
    }

    sb.sem_num = sem_num;
    sb.sem_op = 1;
    sb.sem_flg = 0;

    return semop(semid,&sb,1);
}


int sem_remove(int semid){

    if(semid < 0){

        return -1;
    }
    return semctl(semid,0,IPC_RMID);
}