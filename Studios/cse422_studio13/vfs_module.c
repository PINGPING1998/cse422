/*
* vfs_module.c
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/fs_struct.h>
#include <linux/fdtable.h>
#include <linux/nsproxy.h>
#include <linux/path.h>
#include <linux/mount.h>
#include <linux/dcache.h>
#include <linux/pid.h>
#include <linux/fs.h>

/* Module Parameters */
static uint pid = 0;
module_param(pid, uint, 0);

struct task_struct *  task1;
struct fs_struct * task1_fs;
struct files_struct * task1_files;
struct nsproxy*  task1_ns;
struct path task1_pwd;
struct path task1_root;


/*thread_start - run one or more kernel threads when they are spawned
                by the module.
  inputs: void *
  returns: integer indicating success/failure value
*/
static int thread_start(void * unused){
    struct vfsmount * task1pwd_mount;
    struct dentry * task1pwd_dentry;

    struct vfsmount * task1root_mount;
    struct dentry * task1root_dentry;


    //for list traversal
    //struct list_head *  task1_subdir;
    //struct d_child * child;
    struct dentry *  currdentry;


    printk("fs: %p files: %p nsproxy: %p\n", task1_fs, task1_files, task1_ns);


    task1_fs = current->fs;
    task1_files=current->files;
    task1_ns=current->nsproxy;

    task1_pwd=task1_fs->pwd;
    task1_root=task1_fs->root;

    task1pwd_mount = task1_pwd.mnt;
    task1pwd_dentry = task1_pwd.dentry;

    task1root_mount = task1_root.mnt;
    task1root_dentry = task1_root.dentry;

    printk("PWD dentry: %p ROOT dentry: %p \n", task1pwd_dentry, task1root_dentry);
    if(task1pwd_dentry != task1root_dentry){
        printk("PWD: %s ROOT: %s \n", task1pwd_dentry->d_iname, task1root_dentry->d_iname);

    }
    else{
        printk("Name for PWD AND ROOT: %s \n", task1pwd_dentry->d_iname);
    }

 
    list_for_each_entry(currdentry, &task1root_dentry->d_subdirs , d_child){
        if(!list_empty(&currdentry->d_subdirs)){

            printk("Nonempty Directory: %s\n", currdentry->d_iname);
        }
        //printk("File: %s\n", currdentry->d_iname);
    }




    
    while(!kthread_should_stop()){
        schedule();
    }
    return 0;
}

static int thread_pid(void * unused){
    struct pid * process;
    struct task_struct *  task;
    struct files_struct * process_files;
    struct file * process_file;
    struct file_operations * process_operations;
    loff_t offset = 0;
    const char * message = "This is for number 6";
    int lengthmessage = strlen(message) + 1;

    process = find_get_pid(pid);
    if(process == NULL){
        return -1;
    }

    task = pid_task(process, PIDTYPE_PID);
    if(task == NULL){
        return -1;
    }

    process_files = task->files;
    process_file = fcheck_files(process_files, 1);
    process_operations = process_file->f_op;
    process_operations->write(process_file, message, lengthmessage, &offset);

    while(!kthread_should_stop()){
        schedule();
    }
    return 0;
}


/*
* module_init - the init function, called when the module is loaded.
*/
static int vfs_init(void){
    if(pid != 0){
        task1 = kthread_run(thread_pid, NULL, "task1_pidthread");
    }
    else{
        task1 = kthread_run(thread_start, NULL, "task1_thread");
    }
    return 0;
}

/*
* thread_exit - the exit function, called when the module is removed. 
*/
static void vfs_exit(void){
    printk("Thread unloading");
    kthread_stop(task1);

}

module_init(vfs_init);
module_exit(vfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tara Renduchintala and Anderson Gonzalez");
MODULE_DESCRIPTION("A vfs module");
