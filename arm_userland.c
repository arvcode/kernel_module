/*
 *  /arm_userland.c
 *
 *  Copyright (C) 2017 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * 
 */

#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include"arm_ioctl.h"

#define DEV_NAME "/dev/keycatch"
#define PROC_NAME "/proc/keycatch"

int main() {
	int fd=0;
	char buff[100];
	int ret=0;
	int major_number=0;
	system("insmod arm_module.ko");
		
	major_number=250;
	sprintf(buff,"mknod %s c %d 0",DEV_NAME , major_number);
	printf("buff is %s \n",buff);
	system(buff);
	sprintf(buff,"%c",'\0');
	//while(1){
		fd=open(DEV_NAME,0);
		if (fd<0) {
			sprintf(buff,"cannot open file \n");
			write(stdout,buff,sizeof(buff));
			fflush(stdout);
			exit(0);
		}
		sprintf(buff,"opened file \n");
        write(1,buff,14);
		syscall(4, 1, buff, 14);
		
		buff[0]='\0';
		ret=read(fd,buff,sizeof(buff));
		printf("PID is %d \n *** \n",getpid());
		printf("sizeof buff is %d \n",sizeof(buff));
		printf("read ret bytes %d in buff %s \n",ret,buff);
	
		ioctl(fd,IOCTL_READ_USR_MSG, buff);
		ioctl(fd,IOCTL_WRITE_USR_MSG,buff);
		printf("IOCTL call buff is %s \n",buff);
		close(fd);
		sleep(1);
		/*
		 *uapi/asm-generic/fcntl.h
		 *#define O_RDONLY        00000000
		 *#define O_WRONLY        00000001
		 *#define O_RDWR          00000002
		 */
	 
		fd=open(PROC_NAME,O_RDWR );  /* | O_NONBLOCK*/
		if (fd<0) {
			printf("Cannot open /proc \n");	
			if (errno == EAGAIN) {
				printf("file open in NON BLOCKING mode\n");
			}
			return -1;
		}
	while(1){
		
		ret=read(fd,buff,sizeof(buff));
		if (ret<0) {
			if (errno==EAGAIN) {
					printf("non blocking read \n");
			}			
		}
		printf("PROC CONTENTS \n %s \n", buff);
		sprintf(buff,"Write to kernelspace \n ------ EOP -------\n");
		write(fd,buff, sizeof(buff));
		
		//close(fd);
		sleep(1);
		
	}


}
