#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
	int limit = 20;
	char fifo_name[] = "/tmp/myfifo";
	FILE * f;
	int i;
	f = fopen(fifo_name, "w");
	for(i=0; i < limit; i+=2){
		fprintf(f, "%d\n", i);
	}

}

