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
	int port;
	if(argc == 2){
		if(!strcmp("quit", argv[1])){
			specialnum=418;
		}
		port = atoi(argv[1]);
			
	}
	
	int sfd, con, i;
	struct sockaddr_in peer_addr;
	socklen_t peer_addr_size;
	FILE * read_fd;
	char buffer[1000];
	char reply[1000];
	
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1){
			printf("Error with socket\n");
			printf("Error: %s\n", strerror(errno));

	
	}
	memset(&peer_addr, 0, sizeof(peer_addr));
	peer_addr.sin_family = AF_INET;
	peer_addr.sin_port = htons(port);
	inet_aton("127.0.0.1", &peer_addr.sin_addr);
	
	//connect
	con = connect(sfd, (struct sockaddr *) &peer_addr, sizeof(peer_addr)); 
	if (con == -1){
		printf("Error with opening socket.\n");
	}
	else{
		printf("connection made\n");
	}

	read_fd = fdopen(sfd, "w");
	/*
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
	*/
	while(1){
		//fgets(buffer, sizeof(buffer), stdin);
		//int len = strlen(buffer)-1;
		//if(buffer[len] == '\n'){

		//	buffer[len] = '\0';
		//}
		int z = 0;
		for(z = 0; z < 5; z++){
			memset(buffer,0,strlen(buffer));
			sprintf(buffer, "This is message %d\n", z);
			con = send(sfd, buffer, strlen(buffer), 0);
			if(con < 0){
				printf("Send failed\n");
				fflush(stdout);
			}
		}
		if(recv(sfd, reply, 1000, 0) < 0){
			printf("receive failed \n");
			fflush(stdout);
		}

		printf("server reply: %s\n", reply);
		fflush(stdout);
		break;
		if(specialnum){
			sprintf(buffer, "%d\n", specialnum);
			fputs(buffer, stdout);
			con = send(sfd, buffer, strlen(buffer), 0);
		}
	}	
	fclose(read_fd);
	close(sfd);
	return 0;
	
}
