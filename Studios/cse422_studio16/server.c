#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MY_SOCK_PATH "/tmp/server_sock"

int main(){
	int sfd, cfd;
	struct sockaddr_un my_addr, peer_addr;
	socklen_t peer_addr_size;
	FILE * read_fd;
	char * serverfifo = "/tmp/serverfifo";
	int status;
	
	status =  mkfifo(serverfifo, S_IRUSR | S_IWUSR);
	if(status == -1){
		if(errno != EEXIST){
			perror("mkfifio() failed");
			printf("Error with making fifo");
			return -1;
		}
	}

	sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sfd == -1){
			printf("Error with socket\n");
			printf("Error: %s\n", strerror(errno));

	}
	memset(&my_addr, 0, sizeof(my_addr));
	my_addr.sun_family = AF_UNIX;
	strcpy(my_addr.sun_path, MY_SOCK_PATH);

	if(bind(sfd,(struct sockaddr *) &my_addr, sizeof(my_addr)) == -1){
			printf("Error with bind\n");
			printf("Error: %s\n", strerror(errno));
	}
	if(listen(sfd, 50) == -1){
			printf("Error with Listen\n");

			printf("Error: %s\n", strerror(errno));

	}
	
	peer_addr_size = sizeof(peer_addr);
	int term = 0;
	while(term == 0){
		cfd = accept(sfd, (struct sockaddr *) &peer_addr, &peer_addr_size);
		if (cfd == -1){
			printf("Error with Accept\n");

			printf("Error: %s\n", strerror(errno));
	
		}
		else{
				printf("New connection made: %d\n", cfd);
		}

		int i = 0;
		read_fd = fdopen(cfd, "r");
		if(read_fd == NULL){
			printf("Error in opening fifo");
			return -1;
		}
		int c;
	
		int j = 0;
		while(fscanf(read_fd, "%d", &c) > 0){
			if(c == 418){
				term = c;
				break;
			}
			printf("integer: %d\n", c);
		}
	
	}
	close(sfd);
	close(cfd);
		
	unlink(MY_SOCK_PATH);
	return 0;
	
}
