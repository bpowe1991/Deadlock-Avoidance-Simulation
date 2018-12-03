/*
Programmer: Briton A. Powe          Program Homework Assignment #5
Date: 11/14/18                      Class: Operating Systems
File: oss.c
------------------------------------------------------------------------
Program Description:
Simulates oss resource allocation and deadlock avoidance. Randomly forks off 
children, reads resource request from a message queue, and calculates if system
is in a safe state. Creates a clock structure in shared memory to track time.
All events performed are recorded in a log file called log.txt by default or in 
a file designated by the -l option. To set the timer to termante the program, 
it uses a -t option.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <signal.h>
#include <sys/msg.h> 
#include "oss_structs.h"
#include <sys/stat.h>
#include <stdbool.h>

int flag = 0;
pid_t parent;
int processCount = 0, writtenTo, writeCounter;  

int is_pos_int(char test_string[]);
void alarmHandler();
void interruptHandler();

int main(int argc, char *argv[]){

    signal(SIGINT,interruptHandler);
    signal(SIGALRM, alarmHandler);
    signal(SIGQUIT, SIG_IGN);

    struct Oss_clock *clockptr;
    char filename[20] = "log.txt";
    int opt, t = 2, n = 18, shmid, status = 0, sendmsgid, rcmsgid, clockIncrement,
        scheduleTimeSec = 0, scheduleTimeNSec = 0, currentframeIndex = 0;
    double percentage = 0.0;
    key_t shmkey = 3670404;
    key_t recievekey = 3670405;
    key_t sendkey = 3670406;
    message sendMsg;
    message recieveMsg;
	pid_t childpid = 0, wpid;
    FILE *logPtr;
    parent = getpid();
    frameSegment frames[256];
    int currentFrameIndex;
    srand(time(0));

    
    
    //Setting blocked queue and in system process list for oss.
    queue* processes = createQueue(n);

    //Parsing options.
    while((opt = getopt(argc, argv, "n:t:l:vhp")) != -1){
		switch(opt){
            //Option to enter n.
            case 'n':
				if(is_pos_int(optarg) == 1){
					fprintf(stderr, "%s: Error: Entered illegal input for option -t\n",
							argv[0]);
					exit(-1);
				}
				else {
                    n = atoi(optarg);
                    if (n <= 0 || n > 18) {
                        fprintf(stderr, "%s: Error: Entered illegal input for option -t", argv[0]);
                        exit(-1);
                    }
				}
				break;
            //Option to enter t.
            case 't':
				if(is_pos_int(optarg) == 1){
					fprintf(stderr, "%s: Error: Entered illegal input for option -t\n",
							argv[0]);
					exit(-1);
				}
				else {
                    t = atoi(optarg);
                    if (t <= 0) {
                        fprintf(stderr, "%s: Error: Entered illegal input for option -t", argv[0]);
                        exit(-1);
                    }
				}
				break;
            
            //Option to enter l.
            case 'l':
				sprintf(filename, "%s", optarg);
                if (strcmp(filename, "") == 0){
                    fprintf(stderr, "%s: Error: Entered illegal input for option -l:"\
                                        " invalid filename\n", argv[0]);
                    exit(-1);
                }
				break;
            //Help option.
            case 'h':
                fprintf(stderr, "\nThis program simulates memory management with the clock\n"\
                                "agorithm. The parent increments a clock in shared memory and\n"\
                                "schedules children to run based around the clock.\n"\
                                "Child process make memory request and the parent manages them.\n"\
                                "OPTIONS:\n\n"\
                                "-n Set the number of concurrent child processes that will run."\
                                "(i.e. -n 4 allows 4 processes to run at the same time).\n"\
                                "-t Set the number of seconds the program will run."\
                                "(i.e. -t 4 allows the program to run for 4 sec).\n"\
                                "-l set the name of the log file (default: log.txt).\n"\
                                "(i.e. -l logFile.txt sets the log file name to logFile.txt).\n"\
                                "-v to set verbose to true (will print allocation table).\n"\
                                "-h Bring up this help message.\n"\
                                "-p Bring up a test error message.\n\n");
                exit(0);
                break;
            
            //Option to print error message using perror.
            case 'p':
                perror(strcat(argv[0], ": Error: This is a test Error message"));
                exit(-1);
                break;
            case '?':
                fprintf(stderr, "%s: Error: Unrecognized option \'-%c\'\n", argv[0], optopt);
                exit(-1);
                break;
			default:
				fprintf(stderr, "%s: Error: Unrecognized option\n",
					    argv[0]);
				exit(-1);
		}
	}

    //Checking if t has a valid integer value.
    if (t <= 0){
        perror(strcat(argv[0], ": Error: Illegal parameter for -t option"));
        exit(-1);
    }
   
   //Creating or opening log file.
   if((logPtr = fopen(filename,"w+")) == NULL)
   {
      fprintf(stderr, "%s: Error: Failed to open/create log file\n",
					    argv[0]);
      exit(-1);             
   }

    alarm(t);
    
    //Creating shared memory segment.
    if ((shmid = shmget(shmkey, sizeof(struct Oss_clock), 0666|IPC_CREAT)) < 0) {
        perror(strcat(argv[0],": Error: Failed shmget allocation"));
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

    //Setting up system frames.
    int a;
    for (a = 0; a < 256; a++){
        frames[a].pid = -1;
    }

    while (flag != 1) {
        //Setting scheduling time.           
        if (scheduleTimeNSec == 0 && scheduleTimeSec == 0){
            scheduleTimeNSec = clockptr->nanoSec + ((rand() % (500000000 - 1000000 + 1)) + 1000000);
            if (scheduleTimeNSec > ((int)1e9)) {
                scheduleTimeSec += (scheduleTimeNSec / ((int)1e9));
                scheduleTimeNSec = (scheduleTimeNSec % ((int)1e9));
            }
        }

        //Incrementing clock.
        clockIncrement = (rand() % (15000000 - 5000000 + 1)) + 5000000;
        clockptr->nanoSec += clockIncrement;
        if (clockptr->nanoSec > ((int)1e9)) {
            clockptr->sec += (clockptr->nanoSec/((int)1e9));
            clockptr->nanoSec = (clockptr->nanoSec%((int)1e9));
        }
        
        //Checking if new process is ready to be created.
        if ((clockptr->sec > scheduleTimeSec) || 
            (clockptr->sec == scheduleTimeSec && clockptr->nanoSec >= scheduleTimeNSec)){
            scheduleTimeNSec = 0;
            scheduleTimeSec = 0;

            if (processCount < 18) {
                //Create new process
                processBlock newProcess;
                
                //Forking child.
                if ((childpid = fork()) < 0) {
                    perror(strcat(argv[0],": Error: Failed to create child"));
                }
                else if (childpid == 0) {
                    char *args[]={"./user", NULL};
                    if ((execvp(args[0], args)) == -1) {
                        perror(strcat(argv[0],": Error: Failed to execvp child program\n"));
                        exit(-1);
                    }
                }

                //Adding process to oss processes queue.
                newProcess.pid = (long) childpid;
                
                //Setting pageTable to default.
                int x;
                for (x = 0; x < 32; x++) {
                    newProcess.pageTable[x].frameIndex = -1;
                }

                enqueue(processes, newProcess);
                clockptr->numChildren++;
                processCount++;
                
                //Adjusting for overhead.
                clockptr->nanoSec += (rand() % (50000 - 10000 + 1)) + 10000;
                if (clockptr->nanoSec > ((int)1e9)) {
                    clockptr->sec += (clockptr->nanoSec / ((int)1e9));
                    clockptr->nanoSec = (clockptr->nanoSec % ((int)1e9));
                }

                //Writing to log file.
                if (writtenTo < 10000){
                    fprintf(logPtr, "OSS : %ld : Adding to system at %d.%d\n", 
                            (long) childpid, clockptr->sec, clockptr->nanoSec);
                    writtenTo++;
                    writeCounter++;
                }
            }
        }
        
        //Checking messages in recieve message queue.
        if (msgrcv(rcmsgid, &recieveMsg, sizeof(sendMsg), 0, IPC_NOWAIT) != -1){
            if (recieveMsg.choice < 3){
                fprintf(logPtr, "OSS : %s\n", recieveMsg.mesg_text);
                writtenTo++;
                writeCounter++;
                int pageNumber = recieveMsg.address/1000;
            }
            else if (recieveMsg.choice == 3){

            }
        }
    }

    //Detaching from memory segment.
    if (shmdt(clockptr) == -1) {
        perror(strcat(argv[0],": Error: Failed shmdt detach"));
        clockptr = NULL;
        exit(-1);
    }

    //Removing memory segment.
    if (shmctl(shmid, IPC_RMID, 0) == -1) {
        perror(strcat(argv[0],": Error: Failed shmctl delete"));
        exit(-1);
    }

    //Removing message queues.
    if ((msgctl(sendmsgid, IPC_RMID, NULL)) < 0) {
      perror(strcat(argv[0],": Error: Failed send message queue removal"));
      exit(1);
    }

    if ((msgctl(rcmsgid, IPC_RMID, NULL)) < 0) {
      perror(strcat(argv[0],": Error: Failed recieve message queue removal"));
      exit(1);
    }

    //Closing log file stream.
    fclose(logPtr);
        
    return 0;
}

//Function to check whether string is a positive integer
int is_pos_int(char test_string[]){
	int is_num = 0, i;
	for(i = 0; i < strlen(test_string); i++)
	{
		if(!isdigit(test_string[i]))
			is_num = 1;
	}

	return is_num;
} 