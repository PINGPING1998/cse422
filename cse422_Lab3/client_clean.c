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
#include <ctype.h>


//For ordering
#include "avl_tree.h"

//Error Messages
#define SUCCESS 0;
#define WRONG_NUMBER_OF_ARGS -1
#define SELECT_FAILED -1
#define READ_FAILED -2
#define FAIL_SOCKET -4
#define FAIL_CONNECT -5


//constants
#define BUFFER_SIZE 2000
#define MAXNUM_ARGS 3
#define PORT_NUMBER 2
#define SERVER_ADRS 1
#define SENT "sent"
#define RECEIVED "done"
#define RCOMPLETED 1
#define RINCOMPLETE 0
#define WCOMPLETED 1
#define WINCOMPLETE 0

int UsageMethod(){
    printf("Client program for lab 3\n");
    printf("Usage: client_write internet_addres port\n");
    printf("\n");
    printf("Postional arguments: \n");
    printf(" internet_address  address to make connection to server program\n");
    printf("    port           port number at which client can establish a connection\n");
    return WRONG_NUMBER_OF_ARGS;
}


int main(int argc, char *argv[]){
	int port;
	char * ipadd;
    
    
    //check for correct number of arguments
    if(argc != MAXNUM_ARGS){
        //usage message
        return UsageMethod();
       
    }
    else{
        ipadd = argv[SERVER_ADRS];
		port = atoi(argv[PORT_NUMBER]);
    }		
	
	//ordering Node
	struct Node *rootNode = NULL;

	//Socket Connection Variables
	int sfd;
	int con;
	struct sockaddr_in peer_addr;
	socklen_t peer_addr_size;

	//Select Variables
	fd_set readfds;
	fd_set writefds;
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_sec = 0;



	//socket configuration
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1){
			printf("Error with socket\n");
			printf("Error: %s\n", strerror(errno));
        //return FAIL_SOCKET;
	}
	memset(&peer_addr, 0, sizeof(peer_addr));
	peer_addr.sin_family = AF_INET;
	peer_addr.sin_port = htons(port);
	inet_aton(ipadd, &peer_addr.sin_addr);
	

	//connect to server using addres and port recieved
	con = connect(sfd, (struct sockaddr *) &peer_addr, sizeof(peer_addr));
	if (con == -1){
		printf("Error with connecting to server at address:%s.\n",argv[SERVER_ADRS]);
        printf("and port:%s\n",argv[PORT_NUMBER]);
		fflush(stdout);
        return FAIL_CONNECT;
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
	int rcomplete = 0;
	int wcomplete = 0;

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
				
				if(strncmp(reply, SENT, strlen(SENT)) == 0){
					rcomplete = RCOMPLETED;
					printf("complete: %d\n", rcomplete);
					//Order(rootNode);
					fflush(stdout);
				}
				
				else{
					token = strtok(reply, "\n");
					
					while(token != NULL){

						line = strchr(token, ' ');
						printf("line: %s\n", line);
						int isDigit = 1;
						if(line != NULL){
							int difference = strlen(token) - strlen(line);
							linenum = (char *)malloc(sizeof(char) * (difference + 1));
							if(difference == 0){
								isDigit = 0;
							}

							for(i = 0; i < difference; i++){
								printf("token: %s\n", token);
								if(isdigit(token[i]) == 0){
									isDigit = 0;
									printf("not a digit\n");
									fflush(stdout);
									break;
								}
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
							if(isDigit == 1){
							rootNode = insert(rootNode, linenumint, newline);

							memset(newline, 0, strlen(newline));
							}

						} //LINE != NULL END
						if(line == NULL){
							if(strncmp(token, SENT, strlen(SENT)) == 0){
									rcomplete = RCOMPLETED;
									printf("complete: %d\n", rcomplete);
							}
							Order(rootNode);
							
						} //LINE == NULL END
						printf("nulling token\n");
						fflush(stdout);
						token = strtok(NULL, "\n");
						//isDigit = 1;

				} //WhILE LOOP END (token != NULL)	
					printf("nulling reply\n");
					fflush(stdout);
					memset(reply, 0, BUFFER_SIZE);
					
				
			} //ELSE STATEMENT END
		
		} //if read status > 0 end
	}
			if(rcomplete == RCOMPLETED){
				FD_ZERO(&writefds);
				FD_SET(sfd, &writefds);
			}
	
			if(FD_ISSET(sfd, &writefds)){
				int sendstatus;
				if(rcomplete != RCOMPLETED){
					continue;
				}
				if(rcomplete == RCOMPLETED){
					if(wcomplete == WINCOMPLETE){
						WriteOrSend(rootNode, 1, sfd, NULL);
						sendstatus = write(sfd, RECEIVED, strlen(RECEIVED));
						if(sendstatus < 0){
							perror("error sending\n");
							exit(-2);
						}
					}
					wcomplete = WCOMPLETED;
					break;
				}
			}
			
	
		
		printf("Out of while loop\n");
		fflush(stdout);
		if(rcomplete == RCOMPLETED){
			break;
		}

	}
	printf("after close\n");
	fflush(stdout);
	return 0;
	
}
