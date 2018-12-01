/*
Programmer: Briton A. Powe          Program Homework Assignment #4
Date: 10/25/18                      Class: Operating Systems
File: oss_struct.h
------------------------------------------------------------------------
Program Description:
Header file to declare process block, oss shared memory clock, and queue
structures. The function declarations are also stated here.
*/

#ifndef OSS_CLOCK
#define OSS_CLOCK
#include <sys/types.h>
#include <stdio.h>

//Structure of message for message queue.
typedef struct processMessage {
    long mesg_type; 
    char mesg_text[100];
    int choice_index;
    int choice; 
} message;

//Structure for page data.
typedef struct pageData {
    int referenceBit;
    int dirtyBit;
    int frameIndex;
} page;

//Stucture of process control block.
typedef struct processBlock {
    pid_t pid;
    page pageTable[32];
    int numPageFaults;
    int numMemAccess;
} processBlock;

//Structure for frame segment.
typedef struct frameSegment {
    pid_t pid;
    int pageNumber;
} frameSegment;

//Structure to be used in shared memory.
typedef struct Oss_clock {
    int sec;
    int nanoSec;
    int numChildren;
    int numTotalMemAccess;
    int numTotalFaults;
} oss_clock;

//Queue structure.
typedef struct queue 
{ 
    int front, rear, size; 
    unsigned capacity; 
    processBlock* array; 
} queue; 

//Queue function declarations.
queue* createQueue(unsigned capacity);
int isFull(struct queue* queue);
int isEmpty(struct queue* queue);
void enqueue(struct queue* queue, struct processBlock item);
processBlock* dequeue(struct queue* queue);
processBlock* front(struct queue* queue);

#endif