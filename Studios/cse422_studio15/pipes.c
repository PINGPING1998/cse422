#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>

int main(){	
			int pipefd[2];
			pid_t cpid;
			FILE * readfs;
			FILE * writefs;

			if(pipe(pipefd) == -1){
						perror("pipe");
						return -1;
			}

			cpid = fork();
			if(cpid == -1){
						perror("fork");
						return -1;
			}

			if (cpid == 0) { //Child process
						close(pipefd[0]); //close the read end
						writefs = fdopen(pipefd[1], "w");
						fprintf(writefs, "This is a test message \n");
						fprintf(writefs, "hello, world!\n");
						fprintf(writefs, "goodbye, world!\n");
						fclose(writefs);
						return 0;
			}

			else { //Parent process
						close(pipefd[1]); //close read end
						int c;
						readfs = fdopen(pipefd[0], "r");
						while((c = fgetc(readfs)) != EOF){
										putchar(c);
						}
						fclose(readfs);
						return 0;
			}

}
