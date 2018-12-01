/*
Programmer: Briton A. Powe          Program Homework Assignment #4
Date: 10/25/18                      Class: Operating Systems
File: queue.c
------------------------------------------------------------------------
Program Description:
These are the function definition for queue operations declared in the
header file called oss_struct.h. The main file, oss.c uses queues to
organize and schedule proccess.
*/

#include <stdio.h> 
#include <stdlib.h> 
#include <limits.h> 
#include "oss_structs.h"

// Function to create a queue of given capacity.  
queue* createQueue(unsigned capacity) 
{ 
    struct queue* newQueue = (queue*) malloc(sizeof(queue)); 
    newQueue->capacity = capacity; 
    newQueue->front = newQueue->size = 0;  
    newQueue->rear = capacity - 1;
    newQueue->array = (processBlock*) malloc(newQueue->capacity * sizeof(struct processBlock)); 
    return newQueue; 
} 

//Queue is full when size becomes equal to the capacity.  
int isFull(queue* currentQueue) 
{  return (currentQueue->size == currentQueue->capacity);  } 

//Queue is empty when size is 0. 
int isEmpty(queue* currentQueue) 
{  return (currentQueue->size == 0); } 

//Function to add an item to the queue.   
void enqueue(queue* currentQueue, processBlock item){ 
    if (isFull(currentQueue)) 
        return; 
    currentQueue->rear = (currentQueue->rear + 1)%currentQueue->capacity; 
    currentQueue->array[currentQueue->rear] = item; 
    currentQueue->size = currentQueue->size + 1; 
} 

//Function to remove an item from queue.   
processBlock* dequeue(queue* currentQueue) 
{ 
    processBlock* empty = NULL;
    if (isEmpty(currentQueue)) 
        return empty; 
    processBlock* item = &currentQueue->array[currentQueue->front]; 
    currentQueue->front = (currentQueue->front + 1)%currentQueue->capacity; 
    currentQueue->size = currentQueue->size - 1; 
    return item; 
} 

//Function to get front of queue.
processBlock* front(queue* currentQueue) 
{ 
    processBlock* empty = NULL;
    if (isEmpty(currentQueue)) 
        return empty; 
    return &currentQueue->array[currentQueue->front]; 
} 
