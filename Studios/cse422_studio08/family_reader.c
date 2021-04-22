// SPDX-License-Identifier: GPL-2.0
/*
 * Sample kobject implementation
 *
 * Copyright (C) 2004-2007 Greg Kroah-Hartman <greg@kroah.com>
 * Copyright (C) 2007 Novell Inc.
 */
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/pid.h>
#include <linux/sched.h>

/*
 * This module shows how to create a simple subdirectory in sysfs called
 * /sys/kernel/kobject-example  In that directory, 3 files are created:
 * "foo", "baz", and "bar".  If an integer is written to these files, it can be
 * later read out of it.
 */

static int pidint;


/*
 * More complex function where we determine which variable is being accessed by
 * looking at the attribute for the "baz" and "bar" files.
 */
static ssize_t pid_show(struct kobject *kobj, struct kobj_attribute *attr,
		      char *buf)
{
	int var;

	if (strcmp(attr->attr.name, "pidint") == 0)
		var = pidint;
	return sprintf(buf, "%d\n", var);
}

static ssize_t pid_store(struct kobject *kobj, struct kobj_attribute *attr,
		       const char *buf, size_t count)
{
	int var, ret;
    long currpid;
    
    struct pid*  pid;
    struct task_struct *task, *curr_task, *parent;
	ret = kstrtoint(buf, 10, &var);
	if (ret < 0)
		return ret;
   
    pidint = var;

    if(find_vpid(pidint) > 0){
        pid = find_vpid(pidint);
    }
    else{
        printk("Error in find_vpid");
    }

    if(pid_task(pid, PIDTYPE_PID) > 0){
        task  = pid_task(pid, PIDTYPE_PID);
    }
    else{
        printk("Error in pid_task");
    }

    currpid = 0;
    curr_task = task;
    while(currpid != 1){
        parent = curr_task->real_parent;
        printk("Parent Command: %s and  Parent ID: %ld", parent->comm, (long)parent->pid);
        currpid = (long)(parent->pid);
        printk("Task Command: %s and Task PID: %ld\n", curr_task->comm, (long)curr_task->pid);
        curr_task = parent;
    }
    if((long)parent->pid == 1){
        printk("Initial Command: %s and Initial ID: %ld", curr_task->comm, (long)(curr_task->pid));
    }

	return count;
}

static struct kobj_attribute pid_attribute =
	__ATTR(pidint, 0664, pid_show, pid_store);


/*
 * Create a group of attributes so that we can create and destroy them all
 * at once.
 */
static struct attribute *attrs[] = {
	&pid_attribute.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};

/*
 * An unnamed attribute group will put all of the attributes directly in
 * the kobject directory.  If we specify a name, a subdirectory will be
 * created for the attributes with the directory being the name of the
 * attribute group.
 */
static struct attribute_group attr_group = {
	.attrs = attrs,
};

static struct kobject *fam_reader_kobj;

static int __init fam_reader_init(void)
{
	int retval;

	/*
	 * Create a simple kobject with the name of "kobject_example",
	 * located under /sys/kernel/
	 *
	 * As this is a simple directory, no uevent will be sent to
	 * userspace.  That is why this function should not be used for
	 * any type of dynamic kobjects, where the name and number are
	 * not known ahead of time.
	 */
	fam_reader_kobj = kobject_create_and_add("fam_reader", kernel_kobj);
	if (!fam_reader_kobj)
		return -ENOMEM;

	/* Create the files associated with this kobject */
	retval = sysfs_create_group(fam_reader_kobj, &attr_group);
	if (retval)
		kobject_put(fam_reader_kobj);

	return retval;
}

static void __exit fam_reader_exit(void)
{
	kobject_put(fam_reader_kobj);
}

module_init(fam_reader_init);
module_exit(fam_reader_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Greg Kroah-Hartman <greg@kroah.com>");
