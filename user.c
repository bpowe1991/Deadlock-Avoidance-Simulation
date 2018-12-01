/*
Programmer: Briton A. Powe          Program Homework Assignment #5
Date: 11/14/18                      Class: Operating Systems
File: user.c
------------------------------------------------------------------------
Program Description:
This is the file for child processes for oss. User process run in a loop,
generating a choice to request, release or terminate. It reads from a message
queue to determine if it is allowed to allocate, release, or end.
If request is blocked, a user will not change a choice until it is granted.
Once it recieves a value for terminating, it will exit the loop and end.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <sys/msg.h>  
#include <signal.h>
#include "oss_structs.h"
#include <sys/stat.h>

struct Oss_clock *clockptr;

void sigQuitHandler(int);

int main(int argc, char *argv[]){ 
    signal(SIGQUIT, sigQuitHandler);
    int shmid, choice, sendmsgid, rcmsgid;
    key_t shmkey = 3670404;
    key_t recievekey = 3670405;
    key_t sendkey = 3670406;
    message recieveMsg;
    message sendMsg;

    srand(time(0));
    
    //Finding shared memory segment.
    if ((shmid = shmget(shmkey, sizeof(struct Oss_clock), 0666|IPC_CREAT)) < 0) {
        perror(strcat(argv[0],": Error: Failed shmget find"));
        exit(-1);
    }

    //Attaching to memory segment.
    if ((clockptr = shmat(shmid, NULL, 0)) == (void *) -1) {
        perror(strcat(argv[0],": Error: Failed shmat attach"));
        exit(-1);
    }

    //Setting up send message queue.
    if ((sendmsgid = msgget(sendkey, 0666 | IPC_CREAT)) < 0) {
        perror(strcat(argv[0],": Error: Failed message queue creation/attach"));
        exit(1);
    }
    
    //Setting up recieve message queue.
    if ((rcmsgid = msgget(recievekey, 0666 | IPC_CREAT)) < 0) {
        perror(strcat(argv[0],": Error: Failed message queue creation/attach"));
        exit(1);
    }


    
    //Detaching from memory segment.
    if (shmdt(clockptr) == -1) {
      perror(strcat(argv[0],": Error: Failed shmdt detach"));
      clockptr = NULL;
      exit(-1);
   }

    return 0;
}

//Handler for quit signal.
void sigQuitHandler(int sig) {
   abort();
}