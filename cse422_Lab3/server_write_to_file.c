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
#include <sys/poll.h>

//For ordering
#include "avl_tree.h"

//Return Codes
#define SUCCESS 0;
#define WRONG_NUMBER_OF_ARGS -1
#define FAIL_OPEN_INPUT_FILE -2
#define FAIL_OPEN_OUTPUT_FILE -3
#define FAIL_OPEN_FRAGMENT_FILE -4
#define INPUT_FILE_EMPTY -5
#define FAIL_MEMORY_ALLOCATION -6
#define FAIL_CLOSE_FILE -7
#define FAIL_READ_INPUTFILE -8
#define FAIL_SENDING_DATA -9
#define FAIL_POLLING -10
#define FAIL_ACCEPT_SOCKET -11
#define FAIL_READING_CLIENT -12
#define FAIL_SOCKET -13
#define FAIL_BIND -14
#define FAIL_LISTEN -15
// Global Variables


//Constants
#define MAXLINE 100
#define MAXNUM_ARGS 3
#define PROGNAME 0
#define FILENAME 1
#define PORT 2
#define READ 1
#define NOT_READ 0

void UsageMethod(){
    printf("Server program for lab 3\n");
    printf("Usage: server_write file_name port\n");
    printf("\n");
    printf("Postional arguments: \n");
    printf("   file_name    name of file resinding in server working directory\n");
    printf("   port         port number at which serve will listen and accept connections\n");
    fflush(stdout);
}



struct block{
	FILE * file;
	int block_fd;
	struct block *next;
};

int main(int argc, char *argv[]){
	
	//check arguments
	if(argc != MAXNUM_ARGS){
		//usage message
        UsageMethod();
		return WRONG_NUMBER_OF_ARGS;
	}
	
	int port = atoi(argv[PORT]);

	struct Node * root = NULL;
	
	//For File reading
	FILE * input_file;
       	FILE * output_file;
	int  num_fragment_files = 0;
	char file_name[MAXLINE];
	int line_numbers = 0;
	int fragment_failed = 0;
	int input_read_check = 0;

	input_file = fopen(argv[FILENAME], "r");

	//check if input file could be opened
	if(!input_file){
		perror("Error opening input file\n");
		return FAIL_OPEN_INPUT_FILE;
	}

	//Create first node
	struct block * root_block;
	root_block = (struct block *)malloc(sizeof(struct block));
	struct block * current_block = root_block;

	//get number of fragment files
	while(input_read_check != EOF){
		//scan line till null terminator
		input_read_check = fscanf(input_file, "%[^\n]\n", &file_name);
		
		if(input_read_check == EOF){
			break;
		}
		
		if(input_read_check == -1){
			perror("Fail reading input file\n");
			return FAIL_READ_INPUTFILE;
		}
		
		printf("line %d: %s\n", line_numbers, &file_name);
		fflush(stdout);

		//open file provided by input line if not root bloack
		FILE * frag_file;
		
		fflush(stdout);	
		if(current_block == root_block){
			output_file = fopen(file_name, "a");
			root_block->file = output_file;
		}
		if(current_block != root_block){
			frag_file = fopen(file_name, "r");
			if(!frag_file){
				perror("Error opening fragment file\n");
				return FAIL_OPEN_FRAGMENT_FILE;
			}
			num_fragment_files++;
			
		
		}



		struct block *new_block = (struct block *)malloc(sizeof(struct block));
		new_block->file = frag_file;
		new_block->next = NULL;
		current_block->next = new_block;
		current_block = current_block->next;
		line_numbers++;

	}

	//after all fragment files are assigned to a node
	current_block = root_block->next;
	char line[MAXLINE];
	input_read_check = 0;
	/*
	while(current_block->next != NULL){
		input_read_check =0;
		current_block = current_block->next;
		while(input_read_check != EOF){
			input_read_check = fscanf(current_block->file, "%[^\n]\n", &line);
			if(input_read_check == EOF){
				break;
			}
			if(input_read_check == -1){
				perror("Fail reading fragment file\n");
				return FAIL_READ_INPUTFILE;
			}

			printf("fragment line: %s\n", line);
			fflush(stdout);
		}
		
	}
	*/



		
	//For socket connectons
	int sfd = 0;
	int cfd = 0;
	struct sockaddr_in my_addr, peer_addr;
	FILE * read_fd;
    char server_address[2*MAXLINE];

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd == 1){
		printf("Error in socket: %s\n", strerror(errno));
		fflush(stdout);
        //return FAIL_SOCKET;
	
	}
	memset(&my_addr, 0, sizeof(struct sockaddr_in));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(sfd, (struct sockaddr *) &my_addr, sizeof(my_addr)) == -1){
		printf("Error with bind \n");
		printf("Error: %s\n", strerror(errno));
		fflush(stdout);
        //return FAIL_BIND;
	}
    //convert server address to printable string
    if(inet_ntop(AF_INET, &my_addr,server_address,sizeof(server_address))){
        printf("Connect to port:%d server address: %s\n",port,server_address);
        fflush(stdout);
    }
    
	if(listen(sfd, 50) == -1){
		printf("Error with Listen\n");
		printf("Error: %s\n", strerror(errno));
		fflush(stdout);
        //return FAIL_LISTEN;
	}
	socklen_t peer_addr_size = sizeof(peer_addr);
	int option = 1;
	//setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));
	//Variables for Polling
	int nfds = 0; 
	int fdindex;
	struct timeval timeout;
	char buff[1024] = {0}; //leave space for input buffer
	struct pollfd pollfds[50];


	//For line manipulation
	char * content;
	char * token;
	char * linenum;
	int linenumint;
	char * newline;

	//Status variables
	int pollstatus;
	int readstatus;	
	//Add listening socket 
	pollfds[nfds].fd = sfd;
	pollfds[nfds].events = POLLIN;
	nfds++;	
	int num_sockets_open = 0;
	int num_sockets_return = 0;
     
	while(1){
		pollstatus = poll(pollfds, nfds, -1);
	
		//Polling Error Check
		if(pollstatus < 0){
			perror("error in polling\n");
			return FAIL_POLLING;
		}
		
		//No data but don't break
		if(pollstatus == 0){
			continue;
		}

		//printf("num fragment files: %d\n", num_fragment_files);
		//fflush(stdout);

		if(pollstatus > 0){
			for(fdindex = 0; fdindex < nfds; fdindex++){

				//listening socket
				if((pollfds[fdindex].revents & POLLIN) && (fdindex == 0)) {
					printf("num_sockets_open: %d\n", num_sockets_open);
					fflush(stdout);
					if(num_sockets_open == num_fragment_files){
						close(pollfds[0].fd);
						pollfds[0].events = 0;
						break;
						
					}

					cfd = accept(sfd, (struct sockaddr *) &peer_addr, &peer_addr_size);
					if(cfd  < 0){
						perror("Error making socket");
						return FAIL_ACCEPT_SOCKET;
					}

					printf("connection made\n");
					fflush(stdout);

					//add new socket to list
					pollfds[nfds].fd = cfd;
					pollfds[nfds].events = POLLIN | POLLOUT; //listen both ways
					nfds++;
					num_sockets_open++;
					break;
				}
			

				//client socket
				if((pollfds[fdindex].revents & POLLOUT) && (fdindex > 0)) {		
			
						input_read_check =0;
						current_block = current_block->next;

						memset(line, 0, MAXLINE);
							
						while(fgets(line, sizeof(line), current_block->file)){
							if(input_read_check == EOF){
								break;
							}
							if(input_read_check == -1){
								perror("Fail reading fragment file\n");
								return FAIL_READ_INPUTFILE;
							}
							if(write(pollfds[fdindex].fd, &line, strlen(line)) == -1){
								perror("sending data");
								return FAIL_SENDING_DATA;
							
							}

							printf("%s\n", &line);
							fflush(stdout);
							memset(line, 0, MAXLINE);
							
						}

					

						if(write(pollfds[fdindex].fd, "complete", strlen("complete")) == -1){
								perror("sending new line");
								return FAIL_SENDING_DATA;
						}
						
						printf("All sent\n");
						fflush(stdout);
						//stop writing to the client
						pollfds[fdindex].events = POLLIN;
						break;
						
					}

				if((pollfds[fdindex].revents & POLLIN) && fdindex > 0){
					int i = 0;
					readstatus = read(pollfds[fdindex].fd, buff, 1024);
					if(readstatus < 0){
						perror("Error in read from client\n");
						return FAIL_READING_CLIENT;
					}
					else if(readstatus == 0){
						continue;
					}
					
					//printf("Here in POLLIN\n");
					token = strtok(buff, "\n");
					while(token != NULL){
						content = strchr(token, ' ');


						
						if(content == NULL){
							
							if(strncmp(token, "complete", strlen("complete")) == 0){
								printf("in complete\n");
								Order(root);
								num_sockets_return++;

								//if(num_sockets_return == num_fragment_files){
									break;
								//}
								//pollfds[fdindex].events = 0;
								//break;
							}
						}

						if(content != NULL){
							int difference = strlen(token) - strlen(content);
							linenum = (char *)malloc(sizeof(char) * (difference + 1));
							for(i=0; i < difference; i++){
								linenum[i] = token[i];
							}

							linenum[difference] = '\0';
							linenumint = atoi(linenum);
							int lengthLine = strlen(content);
							if(lengthLine == 1){
								newline = (char *)malloc(sizeof(char) * 2);
								newline[1] = '\0';
								newline[0] = '\n';
							}
							if(lengthLine > 1){
								
								newline = (char *)malloc(sizeof(char) * (lengthLine + 1));
								for(i=0; i < (lengthLine -1); i++){
									newline[i] = content[i+1];
								}

								newline[lengthLine-1] = '\n';
								newline[lengthLine] = '\0';
							}
							printf("%d: %s\n",linenumint, newline);
							fflush(stdout);	
							root = insert(root, linenumint, newline);
							memset(newline, 0, strlen(newline));
						}

					
						token = strtok(NULL, "\n");
					}
					memset(buff, 0, 1024);
					
				}
					if(num_sockets_return == num_fragment_files){
						break;
					}						
				}
				if(num_sockets_return == num_fragment_files){
					break;
				}
			}
		}
	printf("Write or Send\n");
	fflush(stdout);
	WriteOrSend(root, 0, 0, output_file);
	
	
	
	//clean up at the end
	close(sfd);
	close(cfd);
	current_block = root_block;
	/*
	while(current_block->next != NULL){

		struct block * prev_block = current_block;
		current_block = current_block->next;
		fclose(prev_block->file);
		free(prev_block);
	}
	*/
	return 0;


}
