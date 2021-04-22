/*
* mt_timer_module.c - Timer kernel module that schedules a repeating timer
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/kthread.h>
#include <linux/sched.h>
/* Module Parameters */
static unsigned long log_sec = 1;
static unsigned long log_nsec = 0;
module_param(log_sec, ulong, 0);
module_param(log_nsec, ulong, 0);

/*static variables*/
static ktime_t interval;
static struct hrtimer timer;
int i = 0; //for cpu1
int j = 0; //for cpu2
int k = 0; //for cpu3
int m = 0; //for cpu4

/*static thread variables*/
struct task_struct * cpu1;
struct task_struct * cpu2;
struct task_struct * cpu3;
struct task_struct * cpu4;

/*thread_start - run one or more kernel threads when they are spawned
                by the module.
  inputs: void *
  returns: integer indicating success/failure value
*/
static int thread_start(void * unused){
    while(!kthread_should_stop()){
        if(smp_processor_id() == 0){
            printk("Loop iteration has started: %d on Processor: %d ", i, smp_processor_id()); 
            i++;
        }
        else if(smp_processor_id() == 1){
            printk("Loop iteration has started: %d on Processor: %d ", j, smp_processor_id()); 
            j++;
        }
        else if(smp_processor_id() == 2){
            printk("Loop iteration has started: %d on Processor: %d ", k, smp_processor_id()); 
            k++;
        }
        else if(smp_processor_id() == 3){
            printk("Loop iteration has started: %d on Processor: %d ", m, smp_processor_id()); 
            m++;
        }


        printk("Voluntary Context Switches: %ld ; Involuntary Context Switches: %ld ; on Processor: %d", current->nvcsw, current->nivcsw, smp_processor_id());
        set_current_state(TASK_INTERRUPTIBLE);
        schedule();
    }
    i = 0;
    printk("Thread terminating");
    return 0;
}

/*expiration - the function that reschedules the timer's next
*              expiraton one timer interval forward into the future
*   inputs: pointer to the timer (hrtimer struct)
*   returns: HRTIMER_RESTART to indicate that the timer is started
*/
static enum hrtimer_restart expiration(struct hrtimer *currtimer){
    printk("Timer restarting!");
    wake_up_process(cpu1);
    wake_up_process(cpu2);
    wake_up_process(cpu3);
    wake_up_process(cpu4);
    hrtimer_forward_now(currtimer, interval);
    return HRTIMER_RESTART;
}

/*
* timer_init - the init function, called when the module is loaded.
* The timer is initialized and started at module load time
*/
static int timer_init(void){
    interval =  ktime_set(log_sec, log_nsec);
    hrtimer_init(&timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    timer.function = expiration;
    hrtimer_start(&timer, interval, HRTIMER_MODE_REL);
    printk("Timer started!");

    //intialize thread
    cpu1 = kthread_create(thread_start, "NULL", "cpu1_thread");
    cpu2 = kthread_create(thread_start, "NULL", "cpu2_thread");
    cpu3 = kthread_create(thread_start, "NULL", "cpu3_thread");
    cpu4 = kthread_create(thread_start, "NULL", "cpu4_thread");

    //bind thread
    kthread_bind(cpu1, 0);
    kthread_bind(cpu2, 1);
    kthread_bind(cpu3, 2);
    kthread_bind(cpu4, 3);

    //wake up
    wake_up_process(cpu1);
    wake_up_process(cpu2);
    wake_up_process(cpu3);
    wake_up_process(cpu4);

    return 0;
}

/*
* timer_exit - the exit function, called when the module is removed. 
* The timer is canceled when the module is unloaded.
*/
static void timer_exit(void){
    printk("Timer canceling!");
    kthread_stop(cpu1);  
    kthread_stop(cpu2);
    kthread_stop(cpu3);
    kthread_stop(cpu4);
    hrtimer_cancel(&timer);

}

module_init(timer_init);
module_exit(timer_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tara Renduchintala and Anderson Gonzalez");
MODULE_DESCRIPTION("A timer module");
