#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>


int main(int argc, char *argv[]){
    int generations = atoi( argv[1]);
    int generation = 0;
    int forkid = 0;
    printf("Parent's PID is: %d\n", getpid());
    while(generation < generations){
        forkid = fork();
        int childid;
        if(forkid == 0){

            printf("Process belongs to generation: %d\n and the pid is: %d\n", generation, getpid());
            childid = wait(0);
            generation++;
        }
        else{
            //forkid = fork();
            childid = wait(0);
            if(childid == -1){
                perror("Error: ");
            }
            //printf("The child's id was: %d and the parent was: %d\n", childid, getpid());
            generation++;
        }
    }
}
