#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]){
    int numargs = argc;
    int  file_pointer;
    int i = 2;
    if(numargs > 2){
        file_pointer = open(argv[1], O_RDWR | O_CREAT);
        for(i=2; i < argc; i++){
            
           write(file_pointer, argv[i], strlen(argv[i]));
           write(file_pointer, "\n", strlen("\n"));
            
        }

        close(file_pointer);
        return 0;
    }
    else{
        printf("Usage: %s <filename>\n", argv[0]);
        return -1;
    }

}
