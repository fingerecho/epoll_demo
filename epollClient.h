#ifndef EPOLLCLIENT_H
#define EPOLLCLIENT_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <sys/syscall.h>
 
#define PRODUCER_CLIENT "./producer_client"
#define CLIENT_SERVER "./client_server"
#define N_THREAD 3	//线程数
#define DATA_SIZE 5	//数据大小
#define BUFFER_SIZE 20//缓冲区大小

//定义缓冲区
typedef struct Queue{
    int rear;
    int front;
    int elem[BUFFER_SIZE];
}Queue;
 
void* producer(void *arg);
int isEmpty(Queue* q);
int isFull(Queue* q);
void initQueue(Queue* q);
int push(Queue* q, int data);
int pop(Queue* q, int* data);
int epollclient(int argc, char *argv[]);

#endif