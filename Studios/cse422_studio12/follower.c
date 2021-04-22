#include "shared_memory.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(){
    struct shared_data_t * region;
    char *addr;
    int fd;
    //int i;
    int temp_arr[SIZE_SHARED_ARRAY];
    fd = shm_open(SHARED_MEMORY_REGION, O_RDWR , S_IRWXU);
    //check if fd fails
    addr = mmap(NULL, SIZE_SHARED_ARRAY, PROT_READ| PROT_WRITE, MAP_SHARED, fd, 0);
    //check if mmap fails
    region = (struct shared_data_t *) addr;
    
    printf("Follower is created\n");
    region->write_guard=1;
    while(region->read_guard==0){};

    printf("Data is being read\n");
    memcpy(&temp_arr, (void *)region->data, sizeof(region->data));

    //printf("Follower temp_arr: ");
    //for(i=0; i < SIZE_SHARED_ARRAY; i++){
    //    printf(" %d ", temp_arr[i]);
    //}
    printf("\n");
    printf("Follower is unlinking \n");
    region->delete_guard=1;
    
}
