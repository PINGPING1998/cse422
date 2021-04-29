#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]){

	fd_set readfds;
	int ready, nfds, fd, numRead, j;
	struct timeval timeout;
	char buff[255] = {0}; //leave space for input buffer
	
	FD_ZERO(&readfds);
	FD_SET(STDIN_FILENO, &readfds);
	nfds = STDIN_FILENO;

	//timeout.tv_sec = 5;
	//timeout.tv_usec = 0;

	while(1){
		ready = select(nfds, &readfds, NULL, NULL, &timeout);
		if(ready == -1){
			perror("select()");
		}
	
		else if(FD_ISSET(nfds, &readfds)){
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
		}
		else{
			printf("No data within 5 seconds.\n");
		}
	}
	return 0;


}
