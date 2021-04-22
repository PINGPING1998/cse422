/*
* timer_module.c - Timer kernel module that schedules a repeating timer
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
int i = 0;

/*static thread variables*/
struct task_struct * task1;

/*thread_start - run one or more kernel threads when they are spawned
                by the module.
  inputs: void *
  returns: integer indicating success/failure value
*/
static int thread_start(void * unused){
    while(!kthread_should_stop()){
        printk("Loop iteration has started: %d ; Volutary Context Switches: %ld ; Involuntary Context Switches: %ld", i, current->nvcsw, current->nivcsw);
        i++;
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
    wake_up_process(task1);
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
    task1 = kthread_run(thread_start, NULL, "task1_thread");
    return 0;
}

/*
* timer_exit - the exit function, called when the module is removed. 
* The timer is canceled when the module is unloaded.
*/
static void timer_exit(void){
    printk("Timer canceling!");
    kthread_stop(task1);
    hrtimer_cancel(&timer);

}

module_init(timer_init);
module_exit(timer_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tara Renduchintala and Anderson Gonzalez");
MODULE_DESCRIPTION("A timer module");
