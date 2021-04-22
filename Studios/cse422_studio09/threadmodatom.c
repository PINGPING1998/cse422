/*
* threadmodatom.c - studio 9 module with atomic variable
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/smp.h>

/* Module Parameters */
#define iters 1000000
#define CPUS 4

/*static thread variables*/
struct task_struct * cpus[4];

/*variables*/
int j;

/*volatile*/
volatile int shared_data = 0;

/*atomic*/
atomic_t shared_data_atomic;

/*thread_start - run one or more kernel threads when they are spawned
                by the module.
  inputs: void *
  returns: integer indicating success/failure value
*/
static int thread_start(void * unused){
    int i;
    printk("Before loop!\n");
    for(i=0; i < iters; i++){
        atomic_add(1, &shared_data_atomic);
    }
    printk("After loop!\n");
    while(!kthread_should_stop()){
        schedule();
    }

    return 0;
}
/*
* thread_init - the init function, called when the module is loaded.
*/
static int thread_init(void){
    //set atomic variable
    atomic_set(&shared_data_atomic, 0);

    //intialize thread
    for(j=0; j<CPUS; j++){
       cpus[j] = kthread_create(thread_start, NULL, "cpu_thread");
       kthread_bind(cpus[j], j);
       wake_up_process(cpus[j]);
    }

    return 0;
}

/*
* thread_exit - the exit function, called when the module is removed. 
*/
static void thread_exit(void){
    //kthread_stop(cpu1);  
    //kthread_stop(cpu2);
    //kthread_stop(cpu3);
    //kthread_stop(cpu4);
    for(j=0; j<CPUS; j++){
       kthread_stop(cpus[j]);
    }
    printk("Shared data atomic %d", (atomic_read(&shared_data_atomic)));
}

module_init(thread_init);
module_exit(thread_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tara Renduchintala and Anderson Gonzalez");
MODULE_DESCRIPTION("Studio 9");

