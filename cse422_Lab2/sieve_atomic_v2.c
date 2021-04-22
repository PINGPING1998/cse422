/*
* sieve.c - module that runs the Sieve of Eratosthenes algorithm 
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <stdbool.h>
#include <linux/slab.h> //for kmalloc and kfree
#include <linux/kthread.h> //for kthread
#include <linux/atomic.h> //for atomic variables
#include <linux/time.h> //for time stamps

// ERROR MESSAGES
#define INVALID_PARAMETERS -3
#define ALLOCATION_ERROR -4

// module parameter for number of threads
static unsigned long num_threads = 1;
module_param(num_threads, ulong, 0644);

// module parameter for upper bound of range
static unsigned long upper_bound = 10;
module_param(upper_bound, ulong, 0644);

/* --------- global parameters -----------*/

static int * crossedcount; //pointer to counter variables array
volatile int currPrimeNumberIndex = 0;

/***ATOMIC CHANGE***/
atomic_t * num_array;

//struct task_struct * threads[1];
static struct task_struct * * threads;

/*--------- time stamp variables ---------*/
time_t init_time = 0;
time_t first_barrier_time = 0;
time_t second_barrier_time = 0;

/*------------- finished variables ------------------*/
#define FINISHED 1
#define UNFINISHED 0
atomic_t finished;
atomic_t barrier_1;
atomic_t barrier_2;

/* ---------- spin lock variables and functions --------------*/
#define LOCKED 1
#define UNLOCKED 0
volatile int lock_state = UNLOCKED;
volatile int lock_state_2 = UNLOCKED;

//Lock Function
void lock(volatile int * x){
    int expected = UNLOCKED;
    int desired = LOCKED;
    bool status = false;
    status = __atomic_compare_exchange( &lock_state, &expected, &desired, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
    if(!status){
        printk("Waiting for Lock or Error occured\n");
    }
    while(!status){
        expected = UNLOCKED;
        status = __atomic_compare_exchange(&lock_state, &expected, &desired, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
    }
}

//Unlock Function
void unlock(volatile int * y){
    int expected = LOCKED;
    int desired = UNLOCKED;
    bool status = true;
    status = __atomic_compare_exchange( & lock_state, &expected, &desired, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
    if(!status){
        printk("Error occured in Unlock!\n");
    }
}



/*-------------------barrier function-----------------------*/
static void barrier_fn_first(void){
    struct timespec firstbarriertime;
    atomic_add(1, &barrier_1);
    printk("Threads at barrier: %d\n", atomic_read(&barrier_1));
    while(atomic_read(&barrier_1) != num_threads){
        //empty loop
    }

    //get the timestamp of the first barrier
    getnstimeofday(&firstbarriertime);
    first_barrier_time = timespec_to_ns(&firstbarriertime);
    printk("Barrier synchronizaiton completed");
    return;
}

static void barrier_fn_second(void){
    struct timespec secondbarriertime;
    atomic_add(1, &barrier_2);
    printk("Threads at barrier: %d\n", atomic_read(&barrier_2));
    while(atomic_read(&barrier_2) != num_threads){
        //empty loop
        printk("Second barrier hold\n");
    }

    //get the timestamp of the second barrier
    getnstimeofday(&secondbarriertime);
    second_barrier_time = timespec_to_ns(&secondbarriertime);
    printk("Barrier synchronizaiton completed");
    return;
}

/*------------- prime computation funciton ------------------*/
static void prime_fn(int * count){
    int keep_spinning = 1;
    int curr;
    int i;
    int value;
    while(keep_spinning){
        lock(&lock_state); //lock the prime function
        //printk("Locked prime function\n");
        curr = currPrimeNumberIndex;

        /****ATOMIC CHANGE****/
        value = atomic_read(&num_array[curr]);

        currPrimeNumberIndex++;

        // check for current GLOBAL position to be less than length of array
        if(currPrimeNumberIndex == upper_bound -1){
            break;
        }

        /*****ATOMIC CHANGE****/
        while(atomic_read(&num_array[currPrimeNumberIndex]) == 0 && currPrimeNumberIndex < upper_bound -1){
            currPrimeNumberIndex++;
        }

        unlock(&lock_state);

        //printk("unlocked prime function \n");
        // check for LOCAL position to be less than length of array
        if(curr > upper_bound - 2){
            break;
        } 

        //debug purposes
        //printk("current: %d\n", curr);
        
        
        //Exit while loop when see first non-zero number
        for(i = curr + value; i < upper_bound-1; i=i+value){
            //increment by i+ value because it is a mulitple of value
                
            /***ATOMIC CHANGE -- no longer need lock***/ 
                if(value == 0) {
                    break;
                }
                
                
                atomic_set(&num_array[i], 0);
                (*count)++; //increment number for crossed number
                
        }
    }

    return;
}

/*----------------------------------*/
/*
* thread_function - function that all threads will run that performs barrier synchronization, marks non-prime number in array
*               calls barrier function, updates atomic variable to indicate all threads have finished processing
*/
static int thread_function(void * count){
    int * count_int = (int * ) count;
    printk("Started thread function\n");
    barrier_fn_first();
    prime_fn(count_int);
    barrier_fn_second();
    atomic_set(&finished, FINISHED);
    return 0;
}

/*
* sieve_init - the init function that is called when the module is added
*
*/
static int sieve_init(void)
{
    int i,k, j;
    
    //get time stamp of initialization
    struct timespec inittime;
    getnstimeofday(&inittime);
    init_time = timespec_to_ns(&inittime);

    //set atomic variables
    atomic_set(&finished, FINISHED);
    atomic_set(&barrier_1, UNFINISHED);
    atomic_set(&barrier_2, UNFINISHED);

    printk("initializing module\n");

    num_array = 0;
    crossedcount = 0;
    

    //check paramters are good
    if(num_threads < 1 || upper_bound < 2){
        printk("Invalid Module Parameters \n");
        upper_bound = 0;
        num_threads = 0;
        return INVALID_PARAMETERS;
     }
    
    //allocate number array
    num_array = kmalloc(sizeof(int)*(upper_bound-1), GFP_KERNEL);
    
    //check for successful allocation - if NULL then error
    if(num_array == NULL){
        printk("Error in allocating num_array\n");
        //reset all values
        num_threads = 0;
        upper_bound = 0;
        num_array = 0;
        return ALLOCATION_ERROR;
    }
    
    printk("num array allocation complete\n");

    //allocate counter array
    crossedcount = kmalloc(sizeof(int)*(num_threads), GFP_KERNEL);
    
    //check for successful allocation - if NULL then error
    if(crossedcount == NULL){
        printk("Error in allocating crossedcount array\n");
        crossedcount = 0;
        upper_bound = 0;
        num_threads = 0;
        kfree(num_array);
        num_array = 0;
        return ALLOCATION_ERROR;
    }

    printk("crossed count allocation complete\n");

    //initalize array 
    for(i= 0; i < upper_bound - 1; i++){
        /***ATOMIC CHANGE****/
        atomic_set(&num_array[i],  i+2);
    }

    //initialize counter array
    for(k = 0; k < num_threads; k++){
        crossedcount[k] = 0;
    }

    //set finished atomic variable
    atomic_set(&finished, UNFINISHED);

    
    
    //initialize threads
    printk("Beginning to initialize thredas\n");
    threads = kmalloc(sizeof(struct task_struct *)*num_threads, GFP_KERNEL);
    for(j = 0; j < num_threads; j++){
        threads[j] = kthread_create(thread_function,(void *) &crossedcount[j], "thread");
        wake_up_process(threads[j]);
    }

    
    return 0;
}




/*
* sieve_exit - module exit
*
*/
static void sieve_exit(void){
    int j=0; //for threads
    int i = 0; //for prime

    unsigned long total_init_time = 0;
    unsigned long total_proc_time = 0;

    int prime_count = 0;
    int total_elements = 0;
    int non_prime_count = 0;
    int total_crosses = 0;

    //check to see if finished
    if(atomic_read(&finished) == UNFINISHED){
        printk("Unfinished but stopping...\n");
        for(j=0; j < num_threads; j++){
            kthread_stop(threads[j]);
        }
        printk("stopped threads\n");
        return;
    }

    printk("Prime numbers: \n");
    for(i = 0; i < upper_bound -1; i++){
        if(atomic_read(&num_array[i]) != 0){
            prime_count++;
            if(prime_count % 9 != 0){
                printk(KERN_CONT "%d ", atomic_read(&num_array[i]));
            }
            else{
                printk(KERN_CONT "%d ", atomic_read(&num_array[i]));
            }
            
        }
        total_elements++;
    }

    for(i=0; i < num_threads; i++){
        total_crosses += (crossedcount[i]);
    }

    printk("\n");
    non_prime_count = total_elements - prime_count;

    printk("Total Primes found: %d", prime_count);
    printk("Total non Primes: %d", non_prime_count);
    printk("Count for unecessarily crossed out numbers: %d", (total_crosses - non_prime_count));
    printk("Number of threads: %lu", num_threads);
    printk("Upper bounds: %lu", upper_bound);
    
    
    total_init_time = first_barrier_time - init_time;
    total_proc_time = second_barrier_time - first_barrier_time;

    printk("Time spent setting up module: %lu\n", total_init_time);
    printk("Tme spent processing primes: %lu\n", total_proc_time);

    kfree(num_array);
    kfree(crossedcount);
    printk("Module unloaded\n");
    return;

}


module_init(sieve_init);
module_exit(sieve_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tara Renduchintala");
MODULE_DESCRIPTION("Lab 2");
