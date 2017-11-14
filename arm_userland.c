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

#define DEV_NAME "/dev/keycatch"
main() {
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
	while(1){
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
		close(fd);
		sleep(1);
	}


}