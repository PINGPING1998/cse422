#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char * argv[]){
	int cpu = atoi(argv[1]);
	printf("cpu: %d", cpu);
	fflush(stdout);
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(cpu, &cpuset);
	pid_t pid = getpid();
	int get_affinity = sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset);
	if(get_affinity != 0){
		printf("error with sched_getaffinity");
	}

			while(1){
	}



}

