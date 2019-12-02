#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>

#define BACKEND1 "./mimic32"
#define BACKEND2 "./mimic64"

typedef struct Backend{
	int fd[2];
	int pid;
	pthread_t tid;
	char *buffer;
	int len;
}backend;

backend be[2];
pthread_rwlock_t rw_lock;

void quit()
{
	kill(be[0].pid,SIGINT);
	kill(be[1].pid,SIGINT);
	exit(0);
}

pid_t create_backend(char *path,char *argv[],int *fd)
{
	int fd0[2],fd1[2];
	pipe(fd0);
	pipe(fd1);
	pid_t pid = fork();
	if(pid > 0){
		close(fd0[0]);
		close(fd1[1]);
		fd[0] = fd1[0];
		fd[1] = fd0[1];
	}else{
		close(fd0[1]);
		close(fd1[0]);
		dup2(fd0[0],STDIN_FILENO);
		dup2(fd1[1],STDOUT_FILENO);
		execve(path,argv,NULL);
		exit(0);
	}
	return pid;
}

void backend_recv(backend *be)
{
	int ret;
	while(1){
		pthread_rwlock_rdlock(&rw_lock);
		if(be->len != 0){
			pthread_rwlock_unlock(&rw_lock);
			continue;
		}
		pthread_rwlock_unlock(&rw_lock);
		ret = read(be->fd[0],be->buffer,0x1000);
		if(ret <= 0)
			quit(0);
		pthread_rwlock_wrlock(&rw_lock);
		be->len = ret;
		pthread_rwlock_unlock(&rw_lock);
	}
}

void recv_check(){
	while(1){
		pthread_rwlock_wrlock(&rw_lock);
		if(be[0].len != 0 && be[1].len != 0){
			int len = be[0].len < be[1].len ? be[0].len : be[1].len;
			if(memcmp(be[0].buffer,be[1].buffer,len)){
				// printf("DEBUG:gg\n");
				quit();
			}
			write(1,be[0].buffer,len);
			memcpy(be[0].buffer,be[0].buffer+len,be[0].len - len);
			memcpy(be[1].buffer,be[1].buffer+len,be[1].len - len);
			be[0].len -= len;
			be[1].len -= len;
		}
		pthread_rwlock_unlock(&rw_lock);
	}
}



int main()
{
	setvbuf(stdin,0,2,0);
	setvbuf(stdout,0,2,0);
	signal(SIGINT,quit);
	be[0].pid = create_backend(BACKEND1,NULL,be[0].fd);
	be[1].pid = create_backend(BACKEND2,NULL,be[1].fd);
	if(be[0].pid < 0 || be[1].pid < 0){
		quit();
	}
	be[0].buffer = malloc(0x1000);
	be[1].buffer = malloc(0x1000);
	be[0].len = be[1].len = 0;

	pthread_rwlock_init(&rw_lock,NULL);

	pthread_create(&be[0].tid,NULL,backend_recv,&be[0]);
	pthread_create(&be[1].tid,NULL,backend_recv,&be[1]);

	pthread_t check_tid;
	pthread_create(&check_tid,NULL,recv_check,NULL);
	char *buf = malloc(0x1000);
	while(1){
		int sz = read(0,buf,0x1000);
		if(sz <= 0)
			quit();
		int ret = write(be[0].fd[1],buf,sz);
		if(ret < 0)
			quit();
		ret = write(be[1].fd[1],buf,sz);
		if(ret < 0)
			quit();
	}
}