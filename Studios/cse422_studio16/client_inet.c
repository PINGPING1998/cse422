#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define MY_SOCK_PATH "/tmp/server_sock"

int main(int argc, char *argv[]){
	int specialnum = 0;
	if(argc == 2){
		if(!strcmp("quit", argv[1])){
			specialnum=418;
		}
			
	}

	int sfd, con, i;
	struct sockaddr_in peer_addr;
	socklen_t peer_addr_size;
	FILE * read_fd;
	char buffer[16];
	
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1){
			printf("Error with socket\n");
			printf("Error: %s\n", strerror(errno));

	
	}
	memset(&peer_addr, 0, sizeof(peer_addr));
	peer_addr.sin_family = AF_INET;
	peer_addr.sin_port = htons(30001);
	inet_aton("128.252.167.161", &peer_addr.sin_addr);
	
	//connect
	con = connect(sfd, (struct sockaddr *) &peer_addr, sizeof(peer_addr)); 
	if (con == -1){
		printf("Error with opening socket.\n");
	}
	else{
		printf("connection made\n");
	}

	read_fd = fdopen(sfd, "w");
	for(i = 0; i < 5; i++){
		sprintf(buffer, "%d\n", i);
		//memset(buffer, i, sizeof(buffer));
		fputs(buffer, stdout);
		con = send(sfd, buffer, strlen(buffer), 0);
		//fprintf(read_fd, "%d\n", i);
		if(con == -1){
			printf("send failed\n");
		
		}
	}
	if(specialnum){
		sprintf(buffer, "%d\n", specialnum);
		fputs(buffer, stdout);
		con = send(sfd, buffer, strlen(buffer), 0);
	}	
	fclose(read_fd);
	close(sfd);
	return 0;
	
}
