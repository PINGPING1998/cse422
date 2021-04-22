#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
int main(){
	int status = 1;
	char * myfifo = "/tmp/myfifo";
//	status = mkfifo(myfifo, S_IRUSR | S_IWUSR);
	if(status == -1){
		printf("Error with making fifo");
		return -1;
	}
	FILE * fp;
	FILE * fpw;
	if((fp = fopen(myfifo, "r")) == NULL){
		printf("Error in opening fifo");
		return -1;
	}
	if((fpw = fopen(myfifo, "w")) == NULL){
		printf("Error in opening write fifo");
	}
	//fprintf(fp, "%s", "This is a test message\n");
	int c;
	//int doublec;
	while(fscanf(fp, "%d", &c)){
		//doublec = 2*c;
		printf("integer: %d, double: %d\n", c, 2*c);
	}

	return 0;
	//fclose(fp);
	//printf("Ended");
	//fclose(fpw);
	//unlink(myfifo);
}
