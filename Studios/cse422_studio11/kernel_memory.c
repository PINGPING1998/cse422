#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/time.h>
#include <linux/kthread.h>
#include <linux/mm.h>

#include <asm/uaccess.h>

#define ARR_SIZE 8
typedef struct datatype_t {
    unsigned int array[ARR_SIZE];
} datatype;

static uint nr_structs = 2000;
module_param(nr_structs, uint, 0644); 

static struct task_struct * kthread = NULL;

static unsigned int nr_pages;
static unsigned int order;
static unsigned int nr_structs_per_page;
static struct page * pages;
static struct datatype_t * dt;
static unsigned long pageFrameNumber=0;
static unsigned long physicalMemoryAddress=0;

static unsigned int
my_get_order(unsigned int value)
{
        unsigned int shifts = 0;

            if (!value)
                        return 0;

                if (!(value & (value - 1)))
                            value--;

                    while (value > 0) {
                                value >>= 1;
                                        shifts++;
                                            }

                        return shifts;
}

static int
thread_fn(void * data)
{
        int dataTypeSize = sizeof(datatype);
        long pageSize = PAGE_SIZE;

        int i, j, k;
        //void * virtualMemoryAddress;

        nr_structs_per_page = pageSize/dataTypeSize; //number of data types per page
        nr_pages  = nr_structs/nr_structs_per_page;
        if(nr_structs%nr_structs_per_page > 0){
            nr_pages+=1;
        }
        order=my_get_order(nr_pages);
        pages = alloc_pages(GFP_KERNEL, order);
        if(pages == NULL){
            printk("There was an error in alloc_pages");
            return -1;
        }
   
        pageFrameNumber = page_to_pfn(pages);
        physicalMemoryAddress = PFN_PHYS(pageFrameNumber);
        //virtualMemoryAddress = __va(physicalMemoryAddress);


        for (i = 0; i < nr_pages; i++){
            unsigned long cur_page = physicalMemoryAddress + i*pageSize;
            for(j=0 ; j < nr_structs_per_page; j++){
                unsigned long cur_struct = cur_page + j*dataTypeSize;
                dt = (struct datatype_t *)__va(cur_struct);

                
                for(k = 0; k < ARR_SIZE; k++){
                    dt -> array[k] = i*nr_structs_per_page*ARR_SIZE + j*ARR_SIZE + k;
                    if(k == 0 && j== 0){
                       // printk("value: %d", this_struct->array[k]);
                    }
                }
            }
        }
        printk("Hello from thread %s. nr_structs=%u\n", current->comm, nr_structs);
        printk("Kernel Page Size:  %ld", PAGE_SIZE);
        printk("Datatype struct size: %d", sizeof(datatype)); 
        printk("Number of structs that fit in one page: %d", nr_structs_per_page);
        printk("Number of pages: %d", nr_pages);
        printk("The order: %d", order);
            while (!kthread_should_stop()) {
                        schedule();
                            }

                return 0;
}

static int kernel_memory_init(void)
{
        printk(KERN_INFO "Loaded kernel_memory module\n");

            kthread = kthread_create(thread_fn, NULL, "k_memory");
                if (IS_ERR(kthread)) {
                            printk(KERN_ERR "Failed to create kernel thread\n");
                                    return PTR_ERR(kthread);
                                        }
                    
                    wake_up_process(kthread);

                        return 0;
}

static void kernel_memory_exit(void)
{
        int i, j, k;
        int success = 0;
        int dataTypeSize = sizeof(datatype);
        long pageSize = PAGE_SIZE;
        kthread_stop(kthread);
            printk(KERN_INFO "Unloaded kernel_memory module\n");

        for (i = 0; i < nr_pages; i++){
            unsigned long cur_page = physicalMemoryAddress + i*pageSize;
            for(j=0 ; j < nr_structs_per_page; j++){
                unsigned long cur_struct = cur_page + j*dataTypeSize;
                dt = (struct datatype_t *)__va(cur_struct);

 
                for(k = 0; k < ARR_SIZE; k++){
                    if(dt->array[k]  != i*nr_structs_per_page*ARR_SIZE + j*ARR_SIZE + k){
                        printk("Value does not match\n");
                         success = 1;
                    }
                }
            }
        }

        __free_pages(pages, order);
        if(success != 1){
            printk("Successful!!");
        }
        
}

module_init(kernel_memory_init);
module_exit(kernel_memory_exit);

MODULE_LICENSE ("GPL");
