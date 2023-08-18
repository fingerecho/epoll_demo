#ifndef EPOLLSERVER_H
#define EPOLLSERVER_H


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <sys/syscall.h>
 
#define SERVER_CONSUMER "./server_consumer"  
#define CLIENT_SERVER   "./client_server"
#define N_THREAD 3	     //线程数
#define DATA_SIZE 5	     //数据大小
#define BUFFER_SIZE 20	 //缓冲区大小

//定义循环队列缓冲区
typedef struct Queue{
    int rear;
    int front;
    int elem[BUFFER_SIZE];
}Queue;
void initQueue(Queue* q);
int isEmpty(Queue* q);
int isFull(Queue* q);
int push(Queue* q, int data);
int pop(Queue* q, int* data);

//创建消费者任务,其消费者通过id区分
void* consumer(void *arg);
int epollserver(); 

#endif
