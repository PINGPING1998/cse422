/******************************************************************************
* 
* receiver.c
*
* A signal receiver program for the Signals studio
* in CSE 422S at Washington University in St. Louis
* 
* Written by Marion Sudvarg
* Last modified January 18, 2021
* 
******************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>



int main (int argc, char* argv[]){
    struct rlimit softlim;
    union sigval value;
    value.sival_int = 0;
    value.sival_ptr = 0;
    getrlimit(RLIMIT_SIGPENDING, &softlim);
    int pid = atoi(argv[1]);
    int num_sigs = atoi(argv[2]);
    if((long long) softlim.rlim_cur < num_sigs){
		    printf("Entered value is past limit");
		    return -1;
    }
    int i;
    for(i = 0; i < num_sigs; i++){
        sigqueue(pid, SIGRTMIN, value);
    }   
   value.sival_int = 1; 
    sigqueue(pid, SIGRTMIN, value);

    return 0;
}

