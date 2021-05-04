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
#define FAIL_CLOSING_SOCKET -16
// Global Variables


//Constants
#define MAXLINE 100
#define MAXNUM_ARGS 3
#define PROGNAME 0
#define FILENAME 1
#define PORT 2
#define READ 1
#define NOT_READ 0
#define MAXBUFF 1024
#define MAXCON 50
#define SENT "sent"
#define RECEIVED "done"

int UsageMethod(){
    printf("Server program for lab 3\n");
    printf("Usage: ./server <file name> <port number>\n");
    printf("\n");
    printf("Postional arguments: \n");
    printf("   file name    name of file resinding in server working directory\n");
    printf("   port number        port number at which serve will listen and accept connections\n");
    return WRONG_NUMBER_OF_ARGS;
}



struct block{
	FILE * file;
	int block_fd;
	struct block *next;
};

void freeBlocks(struct block* head){
	struct block * tmp;

	while(head != NULL){
		tmp = head;
		head = head->next;
		free(tmp);
	}
}



int main(int argc, char *argv[]){
	
	//check arguments
	if(argc != MAXNUM_ARGS){
		//usage message
        	return UsageMethod();
	}
	
    //save port number
    	if(atoi(argv[PORT]) == 0){
			printf("Invalid Port Number\n");
			return UsageMethod();
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

    //check if input file can be opened
	input_file = fopen(argv[FILENAME], "r");
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
			perror("Failed reading input file\n");

			return FAIL_READ_INPUTFILE;
		}
		
		printf("line %d: %s\n", line_numbers, &file_name);
		fflush(stdout);

		//open file provided by input line if not root block
		FILE * frag_file;
		
        
        	//check if output file can be opened for reading/writing
		if(current_block == root_block){
			output_file = fopen(file_name, "w+");
		}

		if(!output_file){
                	perror("Failed to open output file provide in first line of inputfile\n");
                	return FAIL_OPEN_OUTPUT_FILE;
            	}
		else{
			root_block->file = output_file;
		}

        	//check if fragment file can be opend for reading
		if(current_block != root_block){
			frag_file = fopen(file_name, "r");
			if(!frag_file){
				perror("Error opening fragment file\n");

				//free alraedy allocated blocks
				freeBlocks(root_block);
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

	//after all fragment files are assigned to a node point current block to input file
	current_block = root_block->next;
	char line[MAXLINE];
	input_read_check = 0;



/*********************SOCKET PROGRAMMING*************************/

	//variables for socket connectons
	int sfd = 0;
	int cfd = 0;
	struct sockaddr_in my_addr, peer_addr;
	FILE * read_fd;
    	char * server_address_string; //for address string
    
    
    	//create new socket
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd == -1){
		printf("Error in socket: %s\n", strerror(errno));
		fflush(stdout);
        	return FAIL_SOCKET;
	
	}
	memset(&my_addr, 0, sizeof(struct sockaddr_in));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = INADDR_ANY;

    	//So port can be reused after a run
	int var = 1;
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &var, sizeof(var));
	
	//bind internet domain socket at specified port
	if(bind(sfd, (struct sockaddr *) &my_addr, sizeof(my_addr)) == -1){
		printf("Error with bind \n");
		printf("Error: %s\n", strerror(errno));
		fflush(stdout);

		//free structs
		freeBlocks(root_block);

        	return FAIL_BIND;
	}
    
    	//convert server address to printable string
    	server_address_string = inet_ntoa(my_addr.sin_addr);
    	if(!server_address_string){
        	printf("Did not convert IP to string format");
        	fflush(stdout);
    	}
   	 else{
        	printf("Connect to port:%d server address: %s\n",port,server_address_string);
         	fflush(stdout);
    	}
    
    //listen for incoming connections
	if(listen(sfd, MAXCON) == -1){
		printf("Error with Listen\n");
		printf("Error: %s\n", strerror(errno));
		fflush(stdout);
        	
		//free structs
		freeBlocks(root_block);

		return FAIL_LISTEN;
	}
	socklen_t peer_addr_size = sizeof(peer_addr);
	int option = 1;
	
/*********************POLLING PROGRAMMING*********************************/	
	
	//Variables for Polling
	int nfds = 0; 
	int fdindex;
	struct timeval timeout;
	char buff[MAXBUFF] = {0}; //leave space for input buffer
	struct pollfd pollfds[MAXCON];


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
						close(sfd);
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
								close(sfd);
								close(pollfds[fdindex].fd);
								return FAIL_READ_INPUTFILE;
							}
							if(write(pollfds[fdindex].fd, &line, strlen(line)) == -1){
								perror("sending data");
								return FAIL_SENDING_DATA;
							
							}

							memset(line, 0, MAXLINE);
							
						}

					

						if(write(pollfds[fdindex].fd, SENT, strlen(SENT)) == -1){
								perror("sending new line");
								close(pollfds[fdindex].fd);
								close(sfd);
								return FAIL_SENDING_DATA;
						}
						
						printf("All Data Sent\n");
						fflush(stdout);

						//only set server to listen
						pollfds[fdindex].events = POLLIN;

						//close input file
						
						if(fclose(current_block->file) < 0){
							perror("Error in closing file");
							close(pollfds[fdindex].fd);
							return FAIL_CLOSE_FILE;
						}
						
						break;
						
					}

				if((pollfds[fdindex].revents & POLLIN) && fdindex > 0){
					int i = 0;
					
				
					
					readstatus = read(pollfds[fdindex].fd, buff, MAXBUFF);
					if(readstatus < 0){
						perror("Error in read from client\n");
						close(pollfds[fdindex].fd);
						close(sfd);
						return FAIL_READING_CLIENT;
					}
					else if(readstatus == 0){
						continue;
					}
					
					//Parse input from client
					token = strtok(buff, "\n"); //find the new line
					while(token != NULL){
						content = strchr(token, ' '); //Find the space


						
						if(content == NULL){
							
							if(strncmp(token, RECEIVED, strlen(RECEIVED)) == 0){
								Order(root); //order the nodes
								num_sockets_return++;

								//close the socket
								if(close(pollfds[fdindex].fd) < 0){
									perror("Error closing client socket\n");
									close(sfd);
									return FAIL_CLOSING_SOCKET;
								}

								pollfds[fdindex].events = 0;
							}
						}

						if(content != NULL){
							int difference = strlen(token) - strlen(content); //find the start of content
							linenum = (char *)malloc(sizeof(char) * (difference + 1)); //get line number
							for(i=0; i < difference; i++){
								linenum[i] = token[i];
							}

							linenum[difference] = '\0'; //null terminator to end linenum string
							linenumint = atoi(linenum); //convert line num to integer
							int lengthLine = strlen(content); //create a line the length of content

							//If content is empty line
							if(lengthLine == 1){
								newline = (char *)malloc(sizeof(char) * 2); 
								newline[1] = '\0';
								newline[0] = '\n';
							}

							//If content is not empty
							if(lengthLine > 1){
								
								newline = (char *)malloc(sizeof(char) * (lengthLine + 1));
								for(i=0; i < (lengthLine -1); i++){
									newline[i] = content[i+1];
								}

								newline[lengthLine-1] = '\n';
								newline[lengthLine] = '\0';
							}


							root = insert(root, linenumint, newline); //order the root
							memset(newline, 0, strlen(newline)); //reset the contents of line
						} //END OF (CONTENT != NULL)

					
						token = strtok(NULL, "\n"); //reset the token to null to end while loop
					} //END OF (TOKEN != NULL

					memset(buff, 0, MAXBUFF); //reset the buffer
					
				} //END OF CLIENT POLL
				if(num_sockets_return == num_fragment_files){
						break;
				}						
			}
			if(num_sockets_return == num_fragment_files){
				break;
			}
		}
	}
	printf("Writing to File\n");
	fflush(stdout);

	//Call to AVL_tree.h to write to output file
	WriteOrSend(root, 0, 0, output_file);
	
/**********************CLEAN UP****************************/

	//Close Output File
	if(fclose(output_file) < 0){
		perror("Error closing output file\n");
		return FAIL_CLOSE_FILE;
	}

	//Free blocks
	freeBlocks(root_block);
	freeTree(root);

	//Fail closing socket
	if(close(sfd) < 0){
		perror("Error closing Server socket\n");
		return FAIL_CLOSING_SOCKET;
	}

	return 0;


}
