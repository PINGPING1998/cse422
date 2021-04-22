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

// 500 million iterations should take several seconds to run
#define ITERS 500000000
#define UNLOCKED 0
#define LOCKED 1

int lock_state = UNLOCKED;

void critical_section( void ){
	int index = 0;
	while(index < ITERS){ index++; }
}

//Lock function
void lock(volatile int * x){
	int expected = UNLOCKED;
	int desired = LOCKED;
	bool status = false;
	status = __atomic_compare_exchange( &lock_state, &expected, &desired, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);	
	if(!status){
		printf("Error occured in Lock\n");
	}
	while(!status){
		expected = UNLOCKED;
		status = __atomic_compare_exchange( &lock_state, &expected, &desired, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);	
	}
}

//Unlock function
void unlock(volatile int * y){
	int expected = LOCKED;
	int desired = UNLOCKED;
	bool status = true;
	status = __atomic_compare_exchange( &lock_state, &expected, &desired, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
	if(!status){
		printf("Error occured in Unlock");
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
