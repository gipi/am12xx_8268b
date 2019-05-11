/*
*********************************************************************************************************
*                                       NAND FLASH DRIVER MODULE
*                               (c) Copyright 2007, Actions Co,Ld.
*                                          All Right Reserved
* File    : flash_driver_oal.h
* By      : nand flash group
* Version : V0.1
* Date    : 2007-10-16 9:33
*********************************************************************************************************
*/
#ifndef __OAL_H__
#define __OAL_H__


#define FREE(x)                         free(x)
#define MALLOC(x)                       malloc(x)
#if __KERNEL__
#define MEMSET(x,y,z)                   kmemset(x,y,z)
#define MEMCPY(x,y,z)                   kmemcpy(x,y,z)
#define PRINT(x...)                     printk(x)
#else
#define MEMSET(x,y,z)                   memset(x,y,z)
#define MEMCPY(x,y,z)                   memcpy(x,y,z)
#define PRINT(x...)                     printf(x)
#endif
#define MEMCMP(s1, s2, count)           memcmp(s1, s2, count)

//@fish modify 2009-03-02
#ifndef NULL
#define NULL                   ((void *)0)
#endif
#if __KERNEL__
#define printf(format,args...) printk(format,##args)
#endif

#define FLASH_DRIVER_API       0x80000398

#endif /* __OAL_H__ */

