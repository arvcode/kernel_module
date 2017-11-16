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
#include<asm/uaccess.h> /* for put_user & get_user */
#include<linux/proc_fs.h>
#include<linux/sched.h>
#include<linux/tty.h>

#include"arm_ioctl.h"
/* /dev/  entry*/
#define DEVNAME "keycatch"
#define BUF_LEN 80
/* /proc/ entry */
#define PROC_NAME "keycatch"
#define TIMER_DELAY HZ/2
/* static variables sharing the same memory space in kernel */
static char* test="test";
module_param(test,charp,S_IRUGO);
MODULE_PARM_DESC(test, "test code");

static int major_number,open_dev,open_proc;

static char msg[BUF_LEN];
static char *msg_ptr;

DECLARE_WAIT_QUEUE_HEAD(wait_q);



/* /dev/ file operations */
static int arm_module_open(struct inode*, struct file*);
static int arm_module_release(struct inode*, struct file*);
static ssize_t arm_module_read(struct file*,  char *, size_t, loff_t *);
static ssize_t arm_module_write(struct file*, const char *, size_t, loff_t *);
static long arm_module_ioctl(struct file*, unsigned int, unsigned long);

/* /proc/ operations */
/* valid in 2.6 kernel 
*int procfile_read(char *buffer, char ** buffer_loc, off_t offset, int buff_len, int *eof, void *data);
*/
/* 3.14 kernel needs to use file_operations */
int procfile_read (struct file*, char *, size_t, loff_t *);
int procfile_write (struct file*, const char *, size_t, loff_t *);
int procfile_open(struct inode* , struct file* );
int procfile_release(struct inode*, struct file*);

/*
 * Note struct proc_dir_entry has changed in 3.14 kernel.
 */
struct proc_dir_entry *proc_filp; 

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

struct file_operations procfops ={
	.read=procfile_read,
	.owner=THIS_MODULE,
	.write=procfile_write,
	.open=procfile_open,
	.release=procfile_release
};


static void print_tty(char *str){
	struct tty_struct *p_tty;
	
	p_tty=current->signal->tty;
	
	if (p_tty !=NULL) {
			/* tty_struct changed in 3.14 kernel 
			 * 2.6 kernel p_tty->driver->write 
			 * /include/linux/tty_driver.h
			 */
			((p_tty->driver->ops)->write)(p_tty,str,strlen(str));
	}
	((p_tty->driver->ops)->write) (p_tty, "\015\012", 2);
	
	
}

struct timer_list timer_s;

static void timer_s_func(unsigned long ptr) {
	printk(KERN_INFO"INSIDE TIMER \n");
	timer_s.expires=jiffies+TIMER_DELAY;
	add_timer(&timer_s);
	
}


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
	
	
	print_tty("USING TTY PRINT \n");
	
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
	
	init_timer(&timer_s);
	timer_s.function=timer_s_func;
	timer_s.expires=jiffies+TIMER_DELAY;
	add_timer(&timer_s);
	
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
	del_timer(&timer_s);
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
	printk(KERN_INFO"module is closed \n");
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
	char buff[1024];
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
		printk(KERN_INFO"copy_from_user error \n");
		return -EFAULT;
	}
	printk(KERN_INFO"FROM USERSPACE %s",buff);
	return len;
}

int procfile_open(struct inode* inode, struct file* filp) {
	
	if (filp->f_flags & O_NONBLOCK && open_proc) {
		printk(KERN_INFO"PROC FILE OPEN -> NON BLOCKING IS SET \n");
		return -EAGAIN;		
	}
	/* increase once */
	try_module_get(THIS_MODULE);
	while(open_proc) {
		/* wait for condition , open_proc=0 */
		wait_event_interruptible(wait_q, !open_proc);		
	}
	open_proc=1;
	return 0;

}

int procfile_release(struct inode* inode, struct file* filp) {
	
	open_proc=0;
	/* wake up all waiting processess on queue */
	wake_up(&wait_q);
	module_put(THIS_MODULE);
	return 0;
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

8. wait_event_interruptible(); -> TASK_INTERRUPTIBLE
9. extern void* sys_call_table[];
	sys_call_table[__NR_open]-> for open
	asmlinkage int (*original_call)(const char *,int, int); -> open System call
	
10. struct inode_operations {
	struct dentry * (*lookup) (struct inode *,struct dentry *, unsigned int);
	void * (*follow_link) (struct dentry *, struct nameidata *);
	
	**permission is changed in 3.14 kernel. 2.6 kernel -> int (*permission) (struct inode *, int, unsigned int);
	
	int (*permission) (struct inode *, int);
	struct posix_acl * (*get_acl)(struct inode *, int);

	int (*readlink) (struct dentry *, char __user *,int);
	void (*put_link) (struct dentry *, struct nameidata *, void *);

	int (*create) (struct inode *,struct dentry *, umode_t, bool);
	int (*link) (struct dentry *,struct inode *,struct dentry *);
	int (*unlink) (struct inode *,struct dentry *);
	int (*symlink) (struct inode *,struct dentry *,const char *);
	int (*mkdir) (struct inode *,struct dentry *,umode_t);
	int (*rmdir) (struct inode *,struct dentry *);
	int (*mknod) (struct inode *,struct dentry *,umode_t,dev_t);
	int (*rename) (struct inode *, struct dentry *,
			struct inode *, struct dentry *);
	int (*setattr) (struct dentry *, struct iattr *);
	int (*getattr) (struct vfsmount *mnt, struct dentry *, struct kstat *);
	int (*setxattr) (struct dentry *, const char *,const void *,size_t,int);
	ssize_t (*getxattr) (struct dentry *, const char *, void *, size_t);
	ssize_t (*listxattr) (struct dentry *, char *, size_t);
	int (*removexattr) (struct dentry *, const char *);
	int (*fiemap)(struct inode *, struct fiemap_extent_info *, u64 start,
		      u64 len);
	int (*update_time)(struct inode *, struct timespec *, int);
	int (*atomic_open)(struct inode *, struct dentry *,
			   struct file *, unsigned open_flag,
			   umode_t create_mode, int *opened);
	int (*tmpfile) (struct inode *, struct dentry *, umode_t);
	int (*set_acl)(struct inode *, struct posix_acl *, int);
} ____cacheline_aligned;

*/

