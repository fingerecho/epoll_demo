/**********************client***********************/
#include "epollClient.h"

int cp[N_THREAD];	//client-producer-pipe描述符

void* producer(void *arg)
{
	int id = *((int*)arg);
	char buf[DATA_SIZE] = {0};
	while(1){		
		sleep(rand() % 3 + 1);	
		int data = rand() % 1000;
		sprintf(buf, "%d", data);
		write(cp[id], buf, sizeof(buf));
		printf("id:%d data:%d\n", id, data);
		memset(buf, 0, sizeof(buf));
	}
}
 
int isEmpty(Queue* q)
{
	return q->rear == q->front;
} 
int isFull(Queue* q)
{
	return (q->rear + 1) % BUFFER_SIZE == q->front;
}
void initQueue(Queue* q)
{
	memset(q, 0, sizeof(Queue));
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
 

int epollclient(int argc, char *argv[])
{
	Queue buffer;
	initQueue(&buffer);
 
	//创建并打开client-producer-pipe
	for(int i = 0; i < N_THREAD; ++i){
		char path[128] = {0};
		sprintf(path, "%s%d", PRODUCER_CLIENT, i);
		mkfifo(path, 0666);
		cp[i] = open(path, O_RDWR);
	}
	
	//创建并打开client-server-pipe
	mkfifo(CLIENT_SERVER, 0666);
	int cs = open(CLIENT_SERVER, O_WRONLY);
 
	//创建生产者子线程
	pthread_t tid[N_THREAD];
	int id[N_THREAD];	//线程标识
	for(int i = 0; i < N_THREAD; ++i){
		id[i] = i;
		pthread_create(&tid[i], NULL, producer, id + i);
	}
 
	//创建epoll	
	int epfd = epoll_create(N_THREAD + 1);
	struct epoll_event event[N_THREAD + 1];
	for(int i = 0; i < N_THREAD; ++i){	
		event[i].data.fd = cp[i];
		event[i].events = EPOLLIN;	
		epoll_ctl(epfd, EPOLL_CTL_ADD, cp[i], event + i);
	}
	event[N_THREAD].data.fd = cs;
	event[N_THREAD].events = EPOLLOUT;	
	epoll_ctl(epfd, EPOLL_CTL_ADD, cs, event + N_THREAD);
 
	//监听epoll
	struct epoll_event wait_event[N_THREAD + 1];
	char buf[DATA_SIZE] = {0};
	while(1){
		int n = epoll_wait(epfd, wait_event, N_THREAD + 1, -1);
		for(int i = 0; i < n; ++i){
			if(wait_event[i].data.fd == cs){
				if(!isEmpty(&buffer)){
					int data;
					pop(&buffer, &data);
					memset(buf, 0, sizeof(buf));
					sprintf(buf, "%d", data);
					write(cs, buf, sizeof(buf));
				}
			}
			else{	
				memset(buf, 0, sizeof(buf));
				read(wait_event[i].data.fd, buf, sizeof(buf));
				if(!isFull(&buffer)){
					int data;
					sscanf(buf, "%d", &data);
					push(&buffer, data);
				}
			}
		}
	}
	for(int i = 0; i < N_THREAD; ++i){
		pthread_join(tid[i], NULL);
	}
	for(int i = 0; i < N_THREAD; ++i){
		close(cp[i]);
	}
	close(cs);
	return 0;
}

