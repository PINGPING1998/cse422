#define _GNU_SOURCE
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/epoll.h>



int check_quit(char input[]){
	char * str = "quit";
	int j = 0;
	int sendint = 0;
	while(j < 5){
		if(input[j] == str[j]){
			j++;
			sendint = 1;
		}
		else{
			sendint=0;
			break;
			}
		}
	return sendint;
}

int main(int argc, char *argv[]){

	fd_set readfds, clientfds;
	int ready, nfds, epollfd, numRead;
	int readsize;
	int check;
	struct timeval timeout;
	char buff[1024] = {0}; //leave space for input buffer
	char clientbuff[1024];
	int port = atoi(argv[1]);
	struct epoll_event ev, stdinev, events[3];
	


	//timeout.tv_sec = 5;
	//timeout.tv_usec = 0;
	
	int sfd = 0;
	int cfd = 0;
	struct sockaddr_in my_addr, peer_addr;
	FILE * read_fd;
	char * serverfifo = "/tmp/serverfifo";
	int status;
	status = mkfifo(serverfifo, S_IRUSR | S_IWUSR);
	if(status == -1){
		if(errno != EEXIST){
			perror("mkfifo() failed");
			return -1;
		}
	}

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd == 1){
		printf("Error in socket: %s\n", strerror(errno));
		fflush(stdout);
	
	}
	memset(&my_addr, 0, sizeof(struct sockaddr_in));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(sfd, (struct sockaddr *) &my_addr, sizeof(my_addr)) == -1){
		printf("Error with bind \n");
		printf("Error: %s\n", strerror(errno));
		fflush(stdout);
	}
	if(listen(sfd, 50) == -1){
		printf("Error with Listen\n");
		printf("Error: %s\n", strerror(errno));
		fflush(stdout);
	}
	nfds++;
	socklen_t peer_addr_size = sizeof(peer_addr);
	
	//epoll
	epollfd = epoll_create1(0);
	if(epollfd == -1){
		perror("epoll_create1");
		return -1;
	}
	
	ev.events = EPOLLIN;
	ev.data.fd = sfd;
	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, sfd, &ev) == -1){
		perror("epoll_ctl: sfd");
		return -1;
	}
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = STDIN_FILENO;
	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1){
		perror("epoll_ctl: stdin");
		return -1;
	}

	int num = 0;
	while(1){
		//printf("poll: %d\n", poll(fds, nfds, 30000));
		nfds = epoll_wait(epollfd, events, 5, -1);
		if(nfds == -1){
			perror("epoll_wait");
			return -1;
		}
		
		//printf("Data is available\n");
		//fflush(stdout);

		int i = 0;
		for(i = 0; i < nfds; i++){
			if(events[i].data.fd == STDIN_FILENO && (events[i].events & EPOLLIN | EPOLLET)){
				read(STDIN_FILENO, buff, sizeof(buff));
				int len = strlen(buff)-1;
				if(buff[len] == '\n'){
					buff[len]='\0';
				}
				if(check_quit(buff)){
					break;
				}
				printf("%s was read from stdin\n", buff);
				fflush(stdout);
			}
		
	
			else if(events[i].data.fd == sfd && (events[i].events & EPOLLIN)){

				cfd = accept(sfd, (struct sockaddr *) &peer_addr, &peer_addr_size);
				//setnonblocking(cfd);
				ev.events = EPOLLIN | EPOLLRDHUP;
				ev.data.fd = cfd;
				if(epoll_ctl(epollfd, EPOLL_CTL_ADD, cfd, &ev) == -1){
					perror("epoll_ctl: cfd");
					return -1;
				}	
			}

			else{
				if(events[i].events & (EPOLLIN | EPOLLRDHUP)){
					if(events[i].events == EPOLLIN){
						while((readsize = read(events[i].data.fd, &clientbuff, 1024) > 0)){
							int len2 = strlen(clientbuff)-1;
							if(check_quit(clientbuff)){
								break;
							}
							printf("%s\n", clientbuff);
							fflush(stdout);
						}
					}
					if(events[i].events == EPOLLRDHUP){
						printf("closing socket\n");
						fflush(stdout);
						close(events[i].data.fd);

					}
				}
			}
		}
	}
/*
		if(i==2){
			if(fds[2].revents & (POLLIN | POLLRDHUP)){
				if(fds[2].revents == POLLIN){
					while((readsize = read(cfd, &clientbuff, 1024)) > 0){

						int len2 = strlen(clientbuff)-1;
						
						if(clientbuff[len2] == '\n'){
							//printf("%s\n", clientbuff);
							//fflush(stdout);
							clientbuff[len2] = '\0';
						}
						
						if(check_quit(clientbuff)){
							break;
						}
						
						printf("%s\n", clientbuff);
						fflush(stdout);
					}
				}
				if(fds[2].revents == POLLRDHUP){
					printf("closing socket\n");
					fflush(stdout);
					close(cfd);
					nfds--;
				}
			}
		}
		}
				*/
	//}
	close(sfd);
	close(cfd);
	return 0;


}
