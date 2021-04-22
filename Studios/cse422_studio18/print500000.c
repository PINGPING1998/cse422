/******************************************************************************
* 
* print500000.c
*
* Prints numbers 1 to 500000 in order.
* 
* A simple program serving as a starting point
* for exploring several signal-handling concepts.
*
* Modified January 18, 2021 by Marion Sudvarg
* from a similar program used in previous semesters
* of CSE 422S at Washington University in St. Louis
* 
******************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#define size 500000

volatile int i;
volatile sig_atomic_t terminate = 0;

int called [size];

void signal_handler(int arg){
    char * message = "Caught SIGINT\n";
    int len = strlen(message);
    write(STDOUT_FILENO, message, len);
    called[i] = 1;
    //terminate = 1;
}

int main (int argc, char* argv[]){
        
        struct sigaction sa;
        sa.sa_handler = signal_handler;
        sa.sa_flags = SA_RESTART;
        sigemptyset(&sa.sa_mask);

        sigaction(SIGINT, &sa, NULL);

        for(i = 0; i < size; i++){
                printf("i: %d\n", i);
                if(terminate == 1){
                    break;
                }
        }

        for(i = 0; i < size; i++){
                if( called[i] == 1 ){
                        printf("%d was possibly interrupted\n", i);
                        }
        }

        return 0;
}
