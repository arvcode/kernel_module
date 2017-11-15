/*
 * ioctl definitions 
 */

#ifndef ARM_IOCTL_H
#define ARM_IOCTL_H
#include<linux/ioctl.h>

#define MAJOR_NUM 250


#define IOCTL_READ_USR_MSG  _IOR(MAJOR_NUM,0,char*)
#define IOCTL_WRITE_USR_MSG _IOW(MAJOR_NUM,1,char*)
#define IOCTL_OPEN	    _IO(MAJOR_NUM,3,char*)
#define IOCTL_READ_WRITE    _IOWR(MAJOR_NUM,4,char*)

#endif
