#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#define MAXLINE 80
#define OPEN_MAX 65525
#define SERV_PORT 8000

int main(int argc, char *argv[])
{
	int i, j, maxi, listenfd, connfd, sockfd;
	int n;
	ssize_t nread, efd, res;
	char buf[MAXLINE], str[INET_ADDRSTRLEN];
	socklen_t socklen;
	int listenq = 1024;
	int client[OPEN_MAX];
	struct sockaddr_in servaddr, cliaddr;
	socklen = sizeof(cliaddr);
	listen(listenfd, 20);

	/*epoll_create(1024), epoll_ctl() 修改，添加删除监控文件描述符,
	epoll_wait() 监控阻塞*/
	struct epoll_event tep, ep[OPEN_MAX];
	

	//AF_INET:ipv4   SOCK_STREAM:stream协议 0:tcp
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//任意地址 0.0.0.0 泛指本机
	servaddr.sin_port = htons(SERV_PORT);//host to 网络序  大端序
	//ip+port

	if (listenfd < 0){
		perror("socket error");
		return -1;
	}

	if (bind(listenfd, (struct sockaddr *) &servaddr, socklen) < 0){
		perror("bind error");
		return -1;
	}
	
	if (listen(listenfd, listenq) < 0){
		perror("listen error");
		return -1;
	}
	
	printf ("echo server startup, listen on port:%d\n", SERV_PORT);

	//bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	efd = epoll_create(OPEN_MAX);

	tep.events = EPOLLIN;
	tep.data.fd = listenfd;

	res = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tep);
	
	for(;;) {
		/*阻塞监听*/
		n = epoll_wait(efd, ep, OPEN_MAX, -1);//-1:永久阻塞
		for(i = 0; i < n; i++){
			if(ep[i].data.fd == listenfd) {
			printf("Welcome!\n");
			connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &socklen);
			printf("received from %s at PORT %d\n", inet_ntop(AF_INET, &cliaddr, str, sizeof(cliaddr)));
			//printf("new client:%s:%d\n",inet_ntoa(tep.sin_addr),ntohs(tep.sin_port));
			tep.events = EPOLLIN; 
			tep.data.fd = connfd;
			res = epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tep);
			} 
			else {
				sockfd = ep[i].data.fd;
				n = read(sockfd, buf, MAXLINE);
				if(n == 0){
					res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
					close(sockfd);
					printf("client[%d] close connection\n", j);
				}
				else {
					for(j = 0; j < n; j++)
						buf[j] = toupper(buf[j]);
					write(sockfd, buf, n);
				}
			}
		}
	}
}
