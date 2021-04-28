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

// Global Variables


//Constants
#define MAXLINE 100
#define MAXNUM_ARGS 3
#define PROGNAME 0
#define FILENAME 1
#define PORT 2
#define READ 1
#define NOT_READ 0




struct block{
	FILE * file;
	int block_fd;
	struct block *next;
};

int main(int argc, char *argv[]){

	//check arguments
	if(argc != MAXNUM_ARGS){
		//usage message
		return WRONG_NUMBER_OF_ARGS;
	}

	
	//For File reading
	FILE * input_file;
       	FILE * output_file;
	int * fragment_files;
	char file_name[MAXLINE];
	int line_numbers = 0;
	int fragment_failed = 0;
	int input_read_check = 0;

	input_file = fopen(argv[FILENAME], "r");

	//check if input file could be opened
	if(!input_file){
		perror("Error opening file\n");
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
		printf("here after initialize\n");
		fflush(stdout);	
		if(current_block != root_block){
			frag_file = fopen(file_name, "r");
			if(!frag_file){
				printf("here in null\n");
				perror("Error opening fragment file\n");
				return FAIL_OPEN_FRAGMENT_FILE;
			}
		
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



	//For Socket Connections
	int ready, nfds, epollfd, numRead;
	struct timeval timeout;
	char buff[1024] = {0}; //leave space for input buffer
	char clientbuff[1024];
	int port = atoi(argv[PORT]);
	struct epoll_event ev, stdinev, events[3];
	
		

	int sfd = 0;
	int cfd = 0;
	struct sockaddr_in my_addr, peer_addr;
	FILE * read_fd;

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
	
	ev.events = EPOLLIN | EPOLLET;
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
		nfds = epoll_wait(epollfd, events, 5, -1);
		if(nfds == -1){
			perror("epoll_wait");
			return -1;
		}
		
		int numRead = 0;
		int i = 0;
		for(i = 0; i < nfds; i++){
			if(events[i].data.fd == STDIN_FILENO && (events[i].events & (EPOLLIN | EPOLLET))){
				read(STDIN_FILENO, buff, sizeof(buff));
				int len = strlen(buff)-1;
				if(buff[len] == '\n'){
					buff[len]='\0';
				}
				printf("%s was read from stdin\n", buff);
				fflush(stdout);
			}
		
	
			else if(events[i].data.fd == sfd && (events[i].events & EPOLLIN)){

				cfd = accept(sfd, (struct sockaddr *) &peer_addr, &peer_addr_size);
				printf("connection made\n");
				fflush(stdout);
				ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLOUT;
				ev.data.fd = cfd;
				if(epoll_ctl(epollfd, EPOLL_CTL_ADD, cfd, &ev) == -1){
					perror("epoll_ctl: cfd");
					return -1;
				}	
			}

			else{
				while(1){
					
					if((events[i].events &  EPOLLOUT)){
						printf("in cfd\n");
						fflush(stdout);
						input_read_check =0;
						current_block = current_block->next;

						memset(line, 0, MAXLINE);
						while(input_read_check != EOF){
							
							input_read_check = fscanf(current_block->file, "%[^\n]\n",&line);
							if(input_read_check == EOF){
								break;
							}
							if(input_read_check == -1){
								perror("Fail reading fragment file\n");
								return FAIL_READ_INPUTFILE;
							}
							if(write(events[i].data.fd, &line, strlen(line)) == -1){
								perror("sending data");
								return FAIL_SENDING_DATA;
							
							}

							printf("%s\n", &line);
							fflush(stdout);
							memset(line, 0, MAXLINE);
						}

						printf("All sent\n");
						fflush(stdout);

						if(write(events[i].data.fd, "complete", strlen("complete")) == -1){
								perror("sending new line");
								return FAIL_SENDING_DATA;
						}

						events[i].events = EPOLLIN;
					}

					if((events[i].events & EPOLLIN) && (events[i].data.fd != events[0].data.fd)){

						int numread;
							
						while((read(events[i].data.fd, &clientbuff, 1024) > 0)){
							int len2 = strlen(clientbuff)-1;
							printf("%s\n", clientbuff);
							fflush(stdout);
							int j = 0;
							char * str = "quit";
							while(j < 5){
								if(buff[j] == str[j]){
									j++;
									numread = 1;	
								}
								else{
									numread = 0;
									break;
								}
							}
						}
						if(numread == 1){
							break;
						}		
							
						printf("outside loop\n");
						fflush(stdout);
						close(events[i].data.fd);
						
						
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
	
	
	//clean up at the end
	close(sfd);
	close(cfd);
	current_block = root_block;
	while(current_block->next != NULL){
		struct block * prev_block = current_block;
		current_block = current_block->next;
		fclose(prev_block->file);
		free(prev_block);
	}

	return 0;


}
