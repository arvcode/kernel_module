/*
 *  /arm_module.c
 *
 *  Copyright (C) 2017 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h> /* needed for printk*/
#include<linux/fs.h>
#include<asm/uaccess.h>

/* /dev/keycatch */
#define DEVNAME "keycatch"
#define BUF_LEN 80

/* static variables sharing the same memory space in kernel */
static char* test="test";
module_param(test,charp,S_IRUGO);
MODULE_PARM_DESC(test, "test code");

static int major_number,open_dev;
static char msg[BUF_LEN];
static char *msg_ptr;

/* file operations */
static int arm_module_open(struct inode*, struct file*);
static int arm_module_release(struct inode*, struct file*);
static ssize_t arm_module_read(struct file*,  char *, size_t, loff_t *);
static ssize_t arm_module_write(struct file*, const char *, size_t, loff_t *);


static struct file_operations fops={
	.read=arm_module_read,
	.write=arm_module_write,
	.open=arm_module_open,
	.release=arm_module_release

};

/* insmod */

static int __init arm_module_init(void) {
	printk(KERN_INFO"module init \n");
	major_number=register_chrdev(0,DEVNAME,&fops);
	if (major_number<0) {
		printk(KERN_INFO"Unable to register keycatcher \n");
		return major_number;
	}	
	try_module_get(THIS_MODULE);
	module_put(THIS_MODULE); /* decremented /proc/modules */
	printk(KERN_INFO"Major number assigned is %d, mknod /dev/%s c %d 0",major_number,DEVNAME,major_number);

	return 0;
}

/* rmmod */
static void __exit arm_module_exit(void) {
	unregister_chrdev(major_number, DEVNAME);
 	printk(KERN_INFO"module exit void\n");

}

/* File operations */

static int arm_module_open(struct inode* inode, struct file* file) {
	
	if (open_dev) {
		return -EBUSY;
	}
	open_dev++;
	sprintf(msg,"Open keycatcher %d \n", open_dev);
	msg_ptr=msg;
	try_module_get(THIS_MODULE); /* increase /proc/modules */
	return 0;
}

static int arm_module_release(struct inode* inode, struct file* file) {
	open_dev--;
	module_put(THIS_MODULE);
	return 0;	
}

static int arm_module_read(struct file * filp, char *buffer, size_t len, loff_t * offset) {
	int byte_read=0;
	
	if (msg_ptr==NULL) {
		return 0;
	}
	while (*msg_ptr !='\0' && len !=0) {
		put_user( *msg_ptr++,buffer++);
		len--;
		byte_read++;
	}
	return byte_read;
}	

static int arm_module_write(struct file *filp, const char *buffer, size_t len, loff_t * offset){
	printk(KERN_INFO"write to \n");
	return -EINVAL;
}


module_init(arm_module_init);
module_exit(arm_module_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("KEY CATCHER");
MODULE_VERSION("1.0");
MODULE_LICENSE("NO GPL");
MODULE_AUTHOR("LINUX BOT");



/*
 * Function reference  
int register_chrdev(unsigned int major, const char * name,const struct file_operations* fops);
module_param_array(name, type, pointer to count, permission);
module_param_string();

struct file_operations fops = {
	read: arm_read,
	write: arm_write,
	open: arm_open,
	release: arm_release
};

struct file_operations fops ={
	.read=arm_read,
	.write=arm_write,
	.open=arm_open,
	.relase=arm_release
};


*/

