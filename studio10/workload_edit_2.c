/*    Simulated workload using OpenMP
 *
 * This program will create some number of seconds of work on each processor
 * on the system.
 *
 * This program requires the use of the OpenMP compiler flag, and that 
 * optimizations are turned off, to build correctly. E.g.: 
 * gcc -fopenmp workload.c -o workload
 */

#define _GNU_SOURCE

#include <stdio.h> // for printf()
#include <sched.h> // for sched_getcpu()
#include <stdbool.h> //for booleans
#include <sys/syscall.h>
#include <unistd.h>
#include <linux/futex.h>
#include <sys/time.h>
#include <limits.h>

// 500 million iterations should take several seconds to run
#define ITERS 500000000
#define UNLOCKED 1
#define LOCKED 0

int lock_state = UNLOCKED;

void critical_section( void ){
	int index = 0;
	while(index < ITERS){ index++; }
}

//Lock function
int lock(volatile int * x){
	int ret_val;
	ret_val = __atomic_sub_fetch(&lock_state, 1, __ATOMIC_ACQ_REL);
	if(ret_val >= 0){
		return 0;
	}
	else{
		syscall(SYS_futex, &lock_state, FUTEX_WAIT, ret_val, NULL);
		lock(&lock_state);
	}


}

//Unlock function
int unlock(volatile int * y){
	int ret_val;
	
	ret_val = __atomic_add_fetch(&lock_state, 1, __ATOMIC_ACQ_REL);
	if(ret_val == 1){
		return 0;
	}
	else{
		__atomic_store_n(&lock_state, 1, __ATOMIC_RELEASE);
		syscall(SYS_futex, &lock_state, FUTEX_WAKE, INT_MAX);
	}
}


int main (int argc, char* argv[]){

	// Create a team of threads on each processor
	#pragma omp parallel
	{
		// Each thread executes this code block independently
		//call to lock
		int status;
		lock(&lock_state);
		critical_section();
		unlock(&lock_state);

		printf("CPU %d finished!\n", sched_getcpu());
	}

	return 0;
}
