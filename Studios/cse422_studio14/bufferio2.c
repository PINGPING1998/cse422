#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]){
    int numargs = argc;
    FILE * file_pointer;
    int i = 2;
    if(numargs > 2){
        file_pointer = fopen(argv[1], "w");
        for(i=2; i < argc; i++){
            fputs(argv[i], file_pointer);
            fputs("\n", file_pointer);
        }

        fclose(file_pointer);
        return 0;
    }
    else{
        printf("Usage: %s <filename>\n", argv[0]);
        return -1;
    }

}
