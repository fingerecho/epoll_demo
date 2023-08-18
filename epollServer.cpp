/**********************server***********************/
#include "epollServer.h"

int fd[N_THREAD];            //server-consumer-pipe描述符

void initQueue(Queue* q)
{
	memset(q, 0, sizeof(Queue));
}
int isEmpty(Queue* q)
{
	return q->rear == q->front;
}
int isFull(Queue* q)
{
	return (q->rear + 1) % BUFFER_SIZE == q->front;
}
int push(Queue* q, int data)
{
	if(isFull(q))
		return 0;
	q->elem[q->rear] = data;
	q->rear = (q->rear + 1) % BUFFER_SIZE;
	return 1;
}
int pop(Queue* q, int* data)
{
	if(isEmpty(q))
		return 0;
	*data = q->elem[q->front];
	q->front = (q->front + 1) % BUFFER_SIZE;
	return 1;
}
  
//创建消费者任务,其消费者通过id区分
void* consumer(void *arg)
{
	int id = *((int*)arg);	
	char buf[DATA_SIZE] = {0};
	while(1){
		memset(buf, 0, sizeof(buf));		
		read(fd[id], buf, sizeof(buf));
		sleep(rand() % 3 + 1);
		int data;
		sscanf(buf, "%d", &data);
		printf("id:%d data:%d\n", id, data);				
	}
    return nullptr;
}
 
int epollserver()
{	
    //初始化环形队列
	Queue buffer;
	initQueue(&buffer);
	
	//创建并打开server-consumer-pipe
	for(int i = 0; i < N_THREAD; ++i){
		char path[128] = {0};
		sprintf(path, "%s%d", SERVER_CONSUMER, i);
		mkfifo(path, 0666);
		fd[i] = open(path, O_RDWR);
	}
	//打开client-server-pipe(由client创建)
	int cs = open(CLIENT_SERVER, O_RDONLY);
 
	//创建N个消费者子线程
	pthread_t tid[N_THREAD];
	int id[N_THREAD];	//线程标识
	for(int i = 0; i < N_THREAD; ++i){
		id[i] = i;
		pthread_create(&tid[i], NULL, consumer, id + i);
	}
	
	//创建epoll实例
	int epfd = epoll_create(N_THREAD + 1);
	struct epoll_event event[N_THREAD + 1];
	for(int i = 0; i < N_THREAD; ++i){	
		event[i].data.fd = fd[i];
		event[i].events = EPOLLOUT;	              //监听写事件
		epoll_ctl(epfd, EPOLL_CTL_ADD, fd[i], event + i);
	}
	event[N_THREAD].data.fd = cs;
	event[N_THREAD].events = EPOLLIN;	          //监听读事件
	epoll_ctl(epfd, EPOLL_CTL_ADD, cs, event + N_THREAD);
 
	//监听epoll，等待事件可读可写的事件返回
	struct epoll_event wait_event[N_THREAD + 1];
	while(1){
		int n = epoll_wait(epfd, wait_event, N_THREAD + 1, -1);
		char buf[DATA_SIZE] = {0};
		for(int i = 0; i < n; ++i){
			if(wait_event[i].data.fd == cs){
				memset(buf, 0, sizeof(buf));
				read(cs, buf, sizeof(buf));
				if(!isFull(&buffer)){
					int data;
					sscanf(buf, "%d", &data);
					push(&buffer, data);
				}
			}
			else{
				if(!isEmpty(&buffer)){	
					int data;
					pop(&buffer, &data);
					memset(buf, 0, sizeof(buf));
					sprintf(buf, "%d", data);
					write(wait_event[i].data.fd, buf, sizeof(buf));
				}
			}
		}
		
	}
    //等待线程退出
	for(int i = 0; i < N_THREAD; ++i){
		pthread_join(tid[i], NULL);
	}
    //关闭文件句柄
	for(int i = 0; i < N_THREAD; ++i){
		close(fd[i]);
	}
	return 0;
}
 
