#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char * argv[]){
	int cpu = atoi(argv[1]);
	int priority = atoi(argv[2]);
	int min = sched_get_priority_min(SCHED_RR);
	int max = sched_get_priority_max(SCHED_RR);
	int numforks = atoi(argv[3]);
	if(priority < min || priority > max){
		printf("Usage <cpu> <priority> -- priority between %d and %d\n", min, max);
	fflush(stdout);
	return -1;
	}

	struct sched_param param;
	param.sched_priority = priority;

	//printf("cpu: %d", cpu);
	//fflush(stdout);
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(cpu, &cpuset);
	pid_t pid = getpid();
	int set_schedule = sched_setscheduler(pid, SCHED_RR, &param);
	if(set_schedule != 0){
		printf("error with set_schedule\n");
	}
	int get_affinity = sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset);
	if(get_affinity != 0){
		printf("error with sched_getaffinity");
	}

	int num = 0;
	for(num = 0; num < numforks; num++){
		fork();
	}
	int i = 0;
	int j = 0;
	for(i = 0; i < 500000000; i++){
		j += i*i;
	}



}

