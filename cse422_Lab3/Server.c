//
//  422lab3.c
//  
//
//  Created by Anderson Gonzalez on 4/20/21.
//

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>

//return codes for success or failure
#define success 0;
#define wrong_number_argument -1;
#define input_file_open_failed  -2
#define output_file_open_failed  -3
#define fragment_file_open_failed -4
#define input_file_empty -5
#define failed_allocated_memory -6
#define failed_file_close -7
//constants and global variables
#define MAXLINE 100
#define expected_num_args 3
#define program_name_index 0
#define file_name_index 1
#define READ 1
#define NOTREAD 0




int main (int argc, char * argv[]){
    if(argc == expected_num_args){
        FILE * input_file;
        FILE * output_file;
        int * fragment_files;
        char file_name[MAXLINE];
        int line_numbers= 0;
        int fragment_failed = 0;
        
        input_file = fopen(argv[file_name_index], "r");
        
        
        //check if input file can be opened
        if (!input_file ){
            printf("There was an error trying to open the file\n");
            //change this return value
            return input_file_open_failed;
        }
        
        
        //get number of fragmnet files
        while( fscanf(input_file,"%s",file_name) == READ){
            printf("Line: %s\n",file_name );
            line_numbers++;
        }
        //subtract to reflect ouput_file
        line_numbers--;
        
        printf("The number of lines are %d\n", line_numbers);
        
        //go back to beggining of file
        rewind(input_file);
        
        //read the first line for ouput file
        if (fscanf(input_file,"%s",file_name) != READ){
            printf("Input file is empty\n");
            return input_file_empty;
        }
        
        //check if output can be open
        output_file = fopen(file_name,"r+");
        if (!output_file){
            printf("There was an error trying to open the output file\n");
            //change this return value
            return output_file_open_failed;
        }
    
        //allocate memory for correct number of file pointers
        printf("Trying to allocate array of ints \n");
        fragment_files = (int *)malloc(sizeof(int) * line_numbers);
        
        if(!fragment_files){
            printf("Failed to allocated arrays");
            return failed_allocated_memory;
        }
        
        
        
        //check if fragment files can be opened and store in file descriptor array
        int j;
        for(j=0; j < line_numbers;j++){
            fscanf(input_file,"%s",file_name);
            int open_attempt = open(file_name, O_RDONLY);
            if(open_attempt == -1){
                fragment_failed = 1;
                printf("fail is true\n");
                break;
            }
            printf("There was a succes %d\n",open_attempt);
            fragment_files[j] = open_attempt;
        }
        
        
        
        //if a fragment file could not be open
        //closed all the opened ones and safely exit
        if(fragment_failed == 1){
            int j;
            for(j=0; j < line_numbers;j++){
                if(fragment_files[j]){
                    if(close(fragment_files[j]) < 0 ){
                        return failed_file_close;
                    }
                }
            }
            free(fragment_files);
            fclose(output_file);
            fclose(input_file);
            printf("Failed to open a fragment file\n");
            return fragment_file_open_failed;
        }
        
        //check to see
        //TODO
        
        
        //code to close file for now
        //AT END FREE MEMORY
        for(j=0; j < line_numbers;j++){
            if(close(fragment_files[j]) < 0 ){
                    return failed_file_close;
            }
        }
        free(fragment_files);
        fclose(output_file);
        fclose(input_file);
        
        
        
        return success;
    }
    else{
        printf("Return Usage Message here!");
        return wrong_number_argument;
    }
}
