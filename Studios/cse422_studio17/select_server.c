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

int main(int argc, char *argv[]){

	fd_set readfds, clientfds;
	int ready, nfds, fd, numRead, j;
	int readsize;
	int check;
	struct timeval timeout;
	char buff[255] = {0}; //leave space for input buffer
	char clientbuff[1024];
	int port = atoi(argv[1]);
	FD_ZERO(&readfds);
	FD_SET(STDIN_FILENO, &readfds);
	
	nfds = STDIN_FILENO;

	//timeout.tv_sec = 5;
	//timeout.tv_usec = 0;
	
	int sfd = 0;
	int cfd = 0;
	struct sockaddr_in my_addr, peer_addr;
	FILE * read_fd;
	char * serverfifo = "/tmp/serverfifo";
	int status;
	//printf("before: %d\n", cfd);
	//fflush(stdout);
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

	socklen_t peer_addr_size = sizeof(peer_addr);
	int bigfd;
	//printf("here\n");
	//fflush(stdout);
	while(1){
		FD_SET(sfd, &readfds);
		int connected = 0; //whether or not connection is active
	//	printf("sfd: %d\n", sfd);
	//	fflush(stdout);
	
	//	printf("cfd after accept: %d\n", cfd);
	//	fflush(stdout);
		/*
		if(cfd > 0){
			FD_SET(cfd, &readfds);
			connected = 1;
			printf("connected\n");
			fflush(stdout);
		}

		if(connected){	
			printf("adding to set\n");
			fflush(stdout);
			FD_SET(sfd, &readfds);
		}
		*/
		if(sfd	> STDIN_FILENO){
			bigfd = sfd;
		}
		else{
		//	printf("no cfd\n");
		//	fflush(stdout);
			bigfd = STDIN_FILENO;
		}	
		//printf("file descriptor: %d\n", bigfd);
		//fflush(stdout);
		ready = select(bigfd + 1, &readfds, NULL, NULL, &timeout);

		if(ready == -1){
			perror("select()");
		}

	if(FD_ISSET(STDIN_FILENO, &readfds)){
			//printf("buffer input\n");
			//fflush(stdout);
			fgets(buff, sizeof(buff), stdin);
			int len = strlen(buff)-1;
			if(buff[len] == '\n'){
				buff[len] = '\0';
			
			}
			char * str = "quit";
			while(j<5){
				if(buff[j] == str[j]){
					j++;
					numRead=1;
				}	
				else{	
					numRead=0;
					break;
				}
				
			}
			if(numRead){
				break;
			}
			printf("%s was read from stdin\n", buff);
			fflush(stdout);
		}

		if(FD_ISSET(sfd, &readfds)){
			//printf("client input\n");
			//fflush(stdout);
			char write_buf[1024];
			char message[] = "hostname: server\n";
			strncpy(write_buf, message, sizeof(message));
			cfd = accept(sfd, (struct sockaddr *) &peer_addr, &peer_addr_size);
			if(cfd == -1){
				printf("no connection\n");
				fflush(stdout);
			}
			//printf("message to send: %s", message);
			//fflush(stdout);
			/*
			while(fscanf(read_fd, "%s", &buff) > 0){
				printf("segfault check\n");
				fflush(stdout);

				printf("Client sends: %s\n", buff);
				fflush(stdout);
			}
			*/
			while((readsize = recv(cfd, &clientbuff, 1024, 0 )) > 0)
			{	

				
				printf("client message: %s\n", clientbuff);
				fflush(stdout);
				check = write(cfd, &message, strlen(message));
			
			}
				
				if (check != -1){

					close(cfd);
					cfd = 0;
					fflush(stdout);
				}	
				else{
					printf("error on write");
					fflush(stdout);
				}	

		}

	
		}
	close(sfd);
	close(cfd);
	return 0;


}
