#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    int numargs = argc;
    FILE * file_pointer;
    int i = 2;
    if(numargs > 2){
        file_pointer = fopen(argv[1], "w");
        for(i=2; i < argc; i++){
            fprintf(file_pointer, "%s\n", argv[i]);
        }

        fclose(file_pointer);
        return 0;
    }
    else{
        printf("Usage: %s <filename>\n", argv[0]);
        return -1;
    }

}
