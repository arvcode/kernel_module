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
#include<linux/proc_fs.h>
#include"arm_ioctl.h"
/* /dev/  entry*/
#define DEVNAME "keycatch"
#define BUF_LEN 80

/* /proc/ entry */
#define PROC_NAME "keycatch"

/* static variables sharing the same memory space in kernel */
static char* test="test";
module_param(test,charp,S_IRUGO);
MODULE_PARM_DESC(test, "test code");

static int major_number,open_dev;
static char msg[BUF_LEN];
static char *msg_ptr;

/* /dev/ file operations */
static int arm_module_open(struct inode*, struct file*);
static int arm_module_release(struct inode*, struct file*);
static ssize_t arm_module_read(struct file*,  char *, size_t, loff_t *);
static ssize_t arm_module_write(struct file*, const char *, size_t, loff_t *);
static long arm_module_ioctl(struct file*, unsigned int, unsigned long);

static struct file_operations fops={
	.read=arm_module_read,
	.write=arm_module_write,
	.open=arm_module_open,
	/* 2.6 kernel ioctl
	 * 3.14 kernel unlocked_ioctl
	 */
	.unlocked_ioctl=arm_module_ioctl,
	.release=arm_module_release,
	
};

/* /proc/ operations */

/*
 * Note struct proc_dir_entry has changed in 3.14 kernel.
 */
struct proc_dir_entry *proc_filp; 

/* valid in 2.6 kernel 
*int procfile_read(char *buffer, char ** buffer_loc, off_t offset, int buff_len, int *eof, void *data);
*/
/* 3.14 kernel needs to use file_operations */
int procfile_read (struct file*, char *, size_t, loff_t *);
int procfile_write (struct file*, const char *, size_t, loff_t *);

struct file_operations procfops ={
	.read=procfile_read,
	.owner=THIS_MODULE,
	.write=procfile_write
};

/* insmod  create a character driver*
 * register a file in /proc 
 */

static int __init arm_module_init(void) {
	
	/* /dev/ entry */
	
	printk(KERN_INFO"module init \n");
	major_number=register_chrdev(0,DEVNAME,&fops);
	if (major_number<0) {
		printk(KERN_INFO"Unable to register keycatcher \n");
		return major_number;
	}	
	try_module_get(THIS_MODULE);
	module_put(THIS_MODULE); /* decremented /proc/modules */
	printk(KERN_INFO"Major number assigned is %d, mknod /dev/%s c %d 0",major_number,DEVNAME,major_number);
	
	/* /proc/ entry */

	 /* 3.14 kernel does not have create_proc_entry.
	  * proc_filp=create_proc_entry(PROC_NAME, 0644, NULL); -> valid in 2.6 kernel.
	  * Use proc_create include/linux/proc_fs.h
	  * static inline struct proc_dir_entry *proc_create(const char *name, umode_t mode,
												struct proc_dir_entry *parent,
												const struct file_operations *proc_fops)
	  */
	proc_filp=proc_create(PROC_NAME,0644,NULL,&procfops);
	
	if (proc_filp==NULL) {
		remove_proc_entry(PROC_NAME, NULL);
		return -ENOMEM;		
	}

	printk(KERN_INFO"/proc entry created \n");
	
	return 0;
}

/* rmmod */
static void __exit arm_module_exit(void) {
	unregister_chrdev(major_number, DEVNAME);
 	printk(KERN_INFO"module exit void\n");
	/* Unsupported in 3.14 kernel &proc_root.
	 * remove_proc_entry(PROC_NAME, &proc_root);
	 */
	 remove_proc_entry(PROC_NAME, NULL);
	printk(KERN_INFO" removed /proc entry \n");
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
	printk(KERN_INFO"ARM module write \n");
	return -EINVAL;
}

static long arm_module_ioctl(struct file* filp, unsigned int ioctl_num, unsigned long param) {
	
	switch(ioctl_num) {
		case IOCTL_READ_USR_MSG:
			/* length is not calculated */
			arm_module_write(filp,(char*)param,0,NULL);
		break;
		case IOCTL_WRITE_USR_MSG:
			arm_module_read(filp,(char*)param,BUF_LEN,0);
		break;
		default:
		break;
	}
	return 0;
	
}

/*/proc/ operations */
/* cat /proc/ doesn't print anything 
 * whereas read from userspace program is ok via read.
 * need to fix it.
 */
int procfile_read (struct file* filp , char * buffer, size_t len, loff_t * offset) {
	
	int ret=0;
	char buff[50];
	char *bufp=buff;
	sprintf(buff,"/proc/keycatcher-> your /dev/keycatch major number is %d \n",major_number);
		
	while(*bufp !='\0' && len !=0) {
		put_user( *bufp++, buffer++);		
		len--;
		ret++;
	}	
	/*copy_to_user(); -multi character */
	return 0;
}					

int procfile_write (struct file* filp, const char *buffer , size_t len, loff_t *off) {
	char buff[1024];
	printk(KERN_INFO"Write to proc ! \n");
	/*copy_from_user();- multi character */
	if (copy_from_user(buff,buffer,len)) {
		return -EFAULT;
	}
	printk(KERN_INFO"FROM USERSPACE %s",buff);
	return len;
}


module_init(arm_module_init);
module_exit(arm_module_exit);

/* Info for modinfo 
 * #modinfo arm_module.ko
 */
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("KEY CATCHER");
MODULE_VERSION("1.0");
MODULE_LICENSE("NO GPL");
MODULE_AUTHOR("LINUX BOT");



/* 
 * -EOF- 
 */
 
 
/*
 * Function and struct reference 
 *
 
1.int register_chrdev(unsigned int major, const char * name,const struct file_operations* fops);
2.module_param_array(name, type, pointer to count, permission);
3.module_param_string();

4. struct file_operations fops = {
	read: arm_read,
	write: arm_write,
	open: arm_open,
	release: arm_release
};
		or
5. struct file_operations fops ={
	.read=arm_read,
	.write=arm_write,
	.open=arm_open,
	.relase=arm_release
};

6. proc_register_dynamic();

7. 3.14 kernel file_operations
struct file_operations {
	struct module *owner;
	loff_t (*llseek) (struct file *, loff_t, int);
	ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
	ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
	ssize_t (*aio_read) (struct kiocb *, const struct iovec *, unsigned long, loff_t);
	ssize_t (*aio_write) (struct kiocb *, const struct iovec *, unsigned long, loff_t);
	int (*iterate) (struct file *, struct dir_context *);
	unsigned int (*poll) (struct file *, struct poll_table_struct *);
	long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
	long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
	int (*mmap) (struct file *, struct vm_area_struct *);
	int (*open) (struct inode *, struct file *);
	int (*flush) (struct file *, fl_owner_t id);
	int (*release) (struct inode *, struct file *);
	int (*fsync) (struct file *, loff_t, loff_t, int datasync);
	int (*aio_fsync) (struct kiocb *, int datasync);
	int (*fasync) (int, struct file *, int);
	int (*lock) (struct file *, int, struct file_lock *);
	ssize_t (*sendpage) (struct file *, struct page *, int, size_t, loff_t *, int);
	unsigned long (*get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
	int (*check_flags)(int);
	int (*flock) (struct file *, int, struct file_lock *);
	ssize_t (*splice_write)(struct pipe_inode_info *, struct file *, loff_t *, size_t, unsigned int);
	ssize_t (*splice_read)(struct file *, loff_t *, struct pipe_inode_info *, size_t, unsigned int);
	int (*setlease)(struct file *, long, struct file_lock **);
	long (*fallocate)(struct file *file, int mode, loff_t offset,
			  loff_t len);
	int (*show_fdinfo)(struct seq_file *m, struct file *f);
};




*/

