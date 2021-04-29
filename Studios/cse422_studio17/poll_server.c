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
#include <poll.h>



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
	int ready, nfds, fd, numRead;
	int readsize;
	int check;
	struct timeval timeout;
	char buff[1024] = {0}; //leave space for input buffer
	char clientbuff[1024];
	int port = atoi(argv[1]);
	struct pollfd fds[3];
	
	nfds = 1;
	fds[0].fd = STDIN_FILENO;
	fds[0].events = POLLIN;


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
	

	memset(fds, 0, sizeof(fds));
	fds[1].fd = sfd;
	fds[1].events = POLLIN;
       	fds[0].fd = STDIN_FILENO;
	fds[0].events = POLLIN;
	int num = 0;
	while(poll(fds, nfds, 30000) != -1){
		//printf("poll: %d\n", poll(fds, nfds, 30000));
		fflush(stdout);
		int i = 0;
		for(i = 0; i < nfds; i++){
		if(i == 0){
			if(fds[i].revents & POLLIN){
				//fgets(buff, sizeof(buff), stdin);
			//int len = strlen(buff)-1;
			//if(buff[len] == '\n'){
			//	buff[len] = '\0';
			//}
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
		}
		if(i == 1){
			if(fds[1].revents & POLLIN){

				cfd = accept(sfd, (struct sockaddr *) &peer_addr, &peer_addr_size);
				fds[2].fd = cfd;
				fds[2].events = POLLIN | POLLRDHUP;
				nfds++;
			}
		}

		if(i==2){
			if(fds[2].revents & (POLLIN | POLLRDHUP)){
				if(fds[2].revents == POLLIN){
					while((readsize = read(cfd, &clientbuff, 1024)) > 0){

						int len2 = strlen(clientbuff)-1;
						/*
						if(clientbuff[len2] == '\n'){
							//printf("%s\n", clientbuff);
							//fflush(stdout);
							clientbuff[len2] = '\0';
						}
						*/
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
	}
	close(sfd);
	close(cfd);
	return 0;


}
