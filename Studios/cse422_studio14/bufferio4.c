#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
int main(int argc, char *argv[]){
    int numargs = argc;
    FILE * file_pointer;
    int i = 2;
    if(numargs > 2){
        file_pointer = fopen(argv[1], "a");
        int str_length;
        for(i=2; i < argc; i++){
            str_length += strlen(argv[i]);
        }
        char str[str_length];
        for(i=2; i < argc; i++){
            strcat(str, argv[i]);
            strcat(str, " ");
            //fprintf(file_pointer, "\r%s ", argv[i]);
        }
        fprintf(file_pointer, "%s\n", str);
        fclose(file_pointer);
        return 0;
    }
    else{
        printf("Usage: %s <filename>\n", argv[0]);
        return -1;
    }

}
