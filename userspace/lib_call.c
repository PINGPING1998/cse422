#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
uid_t print_uid(void)
{
    return getuid();
}

int change_uid(uid_t uid){
    setuid(uid);
}

int main()
{
    int status = 0;
    uid_t newuid = 0;
    printf("The current uid is: ");
    printf("%d\n", print_uid());
    status = change_uid(newuid);
    printf("Trying to change the uid: ");
    printf("%d\n", status);
    if(status != 0){
        printf("Error: setuid failed! Reason: %s\n", strerror(errno));
    };
    
    return status;
}

