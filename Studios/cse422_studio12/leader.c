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
    int i;
    int temp_arr[SIZE_SHARED_ARRAY];
    fd = shm_open(SHARED_MEMORY_REGION, O_RDWR | O_CREAT, S_IRWXU);
    //check if fd fails
    ftruncate(fd, sizeof(struct shared_data_t));
    //check if ftruncate fails
    addr = mmap(NULL, SIZE_SHARED_ARRAY, PROT_READ| PROT_WRITE, MAP_SHARED, fd, 0);
    //check if mmap fails
    region = (struct shared_data_t *) addr;
   //set non-data fields
    region->write_guard=0;
    region->read_guard=0;
    region->delete_guard=0;

    //wait on write_guard
    while(region->write_guard==0){};

    printf("Leader is writing data\n");
    srand(0);

    for(i = 0; i < SIZE_SHARED_ARRAY; i++){
        temp_arr[i] = rand();
    }

    memcpy((void *)region->data, &temp_arr, sizeof(temp_arr));
/*
    printf("Leader data array: ");
    for(i=0; i < SIZE_SHARED_ARRAY; i++){
        printf(" %d ", region->data[i], temp_arr[i]);
    }
    printf("\n");
*/
    region->read_guard=1;

    while(region->delete_guard==0){};
    printf("Leader is deleting the region.\n");
    shm_unlink(SHARED_MEMORY_REGION);
}
