#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>

//Error Messages
#define SELECT_FAILED -1
#define READ_FAILED -2

#define BUFFER_SIZE 2000


struct reply_node{
	int linenum;
	char * line;
	struct reply_node * next;
};

int main(int argc, char *argv[]){
	int port;
	if(argc == 2){
		port = atoi(argv[1]);
			
	}
	
	//Socket Connection Variables
	int sfd;
	int con;
	struct sockaddr_in peer_addr;
	socklen_t peer_addr_size;

	//Select Variables
	fd_set readfds;
	fd_set writefds;
	struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_sec = 0;



	//socket configuration
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1){
			printf("Error with socket\n");
			printf("Error: %s\n", strerror(errno));

	
	}
	memset(&peer_addr, 0, sizeof(peer_addr));
	peer_addr.sin_family = AF_INET;
	peer_addr.sin_port = htons(port);
	inet_aton("127.0.0.1", &peer_addr.sin_addr);
	

	//connect socket
	con = connect(sfd, (struct sockaddr *) &peer_addr, sizeof(peer_addr));
	if (con == -1){
		printf("Error with opening socket.\n");
		fflush(stdout);
	}
	else{
		printf("connection made\n");
		fflush(stdout);
	}
	
	//zero out fds
	FD_ZERO(&readfds);
	FD_SET(sfd, &readfds);
	FD_ZERO(&writefds);
	
	
	//Line variables
	char reply[BUFFER_SIZE];
	char * token;
	char * line;
	char * newline;
	char * linenum;
	int  linenumint;
	int i = 0;
	//Status Variables
	int selectstatus;
	int readstatus;
	//Complete variables
	int complete = 0;

	//Revieve data
	while(1){
			
		selectstatus = select(sfd + 1, &readfds, &writefds,  NULL, &timeout);
		
		//Error on seelct
		if(selectstatus < 0){
			perror("Error with select\n");
			return SELECT_FAILED;
		}
		
		
		//Nothing is being written but don't want to break
		if(selectstatus == 0){
			continue;
		}
		
		//Something is being written -- process the lines
		if(FD_ISSET(sfd, &readfds)){
			
			readstatus = read(sfd, reply, BUFFER_SIZE);

			//Error on read
			if(readstatus < 0){
				perror("Error on read()\n");
				return READ_FAILED;
			}

			//Read returns nothing but don't want to break
			if(readstatus == 0){
				continue;
			}

			//Read returns line to be processed
			if(readstatus > 0){

				if(strncmp(reply, "complete", strlen("complete")) == 0){
					complete = 1;
				}
				else{
					token = strtok(reply, "\n");
					while(token != NULL){

						line = strchr(token, ' ');

						if(line != NULL){
							int difference = strlen(token) - strlen(line);
							linenum = (char *)malloc(sizeof(char) * (difference + 1));

							for(i = 0; i < difference; i++){
								linenum[i] = token[i];
							}

							linenum[difference] = '\0';
							linenumint = atoi(linenum);

							printf("linenum: %d\n", linenumint);
							fflush(stdout);

							if(strlen(line) > 1){
								newline = (char *)malloc(sizeof(char) * strlen(token) + 2);
								for(i = 0; i < strlen(token); i++){
									newline[i] = token[i];
								}

								newline[strlen(token)] = '\n';
								newline[strlen(token)+1] = '\0';
							}

							if(strlen(line) == 1){
								newline = (char *)malloc(sizeof(char)*strlen(token)+2);
								for(i=0; i < strlen(token); i++){
									newline[i] = token[i];
								}
								newline[strlen(token)] = '\n';
								newline[strlen(token)+1] = '\0';
							}




							
							printf("newline: %s\n", newline);
							fflush(stdout);
							memset(newline, 0, strlen(newline));

						}
						if(line == NULL){
							if(strncmp(token, "complete", strlen("complete")) == 0){
									complete = 1;
							}
						}

						token = strtok(NULL, "\n");

				}	
				
				memset(reply, 0, BUFFER_SIZE);
					
				
			}
	}

		}
	
	}
	printf("after close\n");
	fflush(stdout);
	return 0;
	
}
