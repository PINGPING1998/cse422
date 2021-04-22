#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
	printf("Before forking! I am the parent and my pid is: %d\n", getpid());
	int childPID = fork();
	if(childPID != 0){
		printf("After forking, the child's PID is: %d\n", childPID);
	}

	if(childPID == 0){
		printf("I am a child and my PID is: %d and my parent's PID is: %d.\n", getpid(), getppid());
	}
}
