#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]){
    int numargs = argc;
    FILE * file_pointer;
    //char buffer[15];
    char * line = NULL;
    unsigned int count = 0;
    unsigned int i = 0;
    ssize_t read; 
    //unsigned int j = 0;
    char * p;
    if(numargs == 2){
        file_pointer = fopen(argv[1], "r");
        while ((read = getline(&line, &i, file_pointer)) != -1){
           char * delete;
          
           delete = strchr(line, ' ');
           char cpline[strlen(delete)];
           strcpy(cpline, delete);
           if(delete != NULL){
               
               strcpy(delete, cpline);
               *delete = '\0';
           }
           
           if(count < atoi(line)){
               if(count == 0){
                   count = atoi(line);
                   p = (char*) malloc(count);
                   p = cpline;
               }
               else{
                count = atoi(line);
                p = (char*)realloc(p, count);
                p = cpline; 
               }
               
           
           }

        }
        printf("%s", p);
        fclose(file_pointer);
        return 0;
    }
}
