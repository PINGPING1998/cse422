#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>

uid_t print_uid(void)
{
    return syscall(__NR_noargs);
}

int change_uid(uid_t uid){
    return syscall(__NR_onearg, uid);
}

int main()
{
    int status = 0;
    uid_t newuid = 0;
    printf("Running with syscall with no args: ");
    print_uid();
    printf("%d\n", print_uid());
    status = change_uid(newuid);
    printf("Running syscall with args: ");
    change_uid(newuid);
    printf("%d\n", status);
    if(status != 0){
        printf("Error: setuid failed! Reason: %s\n", strerror(errno));
    };
    
    return status;
}

