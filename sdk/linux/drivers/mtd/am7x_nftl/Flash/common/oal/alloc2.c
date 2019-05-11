/*
*********************************************************************************************************
*                                                USDK130
*                                          ucOS + MIPS,Nand Module
*
*                               (c) Copyright 2007, Actions Co,Ld.
*                                          All Right Reserved
* File    : alloc.c
* By      : nand flash group
* Version : V0.1
*********************************************************************************************************
*/
#include "flash_driver_config.h"
#include "nand_flash_driver_type.h"

#if   __KERNEL__
#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/mm.h>
//#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/highmem.h>
#include <dma.h>

/*
*********************************************************************************************************
*                       COPY MEMORY TO ANOTHE ADDRESS
*
* Description: copy data from a buffer to another buffer
*
* Aguments   : desti_buffer     the pointer to the destination buffer;
*              source_buffer    the pointer to the source buffer;
*              buffer_size      the size of the memory need be copy;
*
* Returns    : none
*********************************************************************************************************
*/
void kmemcpy(INT8S* desti_buffer, INT8S* source_buffer, INT32S buffer_size)
{
//printk("desti_buffer: 0x%x, source_buffer: 0x%x, size: %d\n", desti_buffer, source_buffer, buffer_size);
#if 0
	am_memcpy_dma(desti_buffer,source_buffer,buffer_size);
	return;
#else

    INT32S i;
    INT8S *desti, *source;

    desti = desti_buffer;
    source = source_buffer;

    for(i=0; i<buffer_size; i++)
        *desti++ = *source++;

    return;
#endif	
}


/*
*********************************************************************************************************
*                       SET MEMORY DATA
*
* Description: set memory data.
*
* Aguments   : Buf      the pointer to the buffer need be set value;
*              Value    the value to be set to the buffer;
*              Length   the size of buffer to be set;
*
* Returns    : none
*********************************************************************************************************
*/
void kmemset(void* Buf, INT8U Value, INT32U Length)
{
    INT32S i;
    INT8U *tmpPointer = Buf;

    for (i = 0; i < Length; i++)
    {
       tmpPointer[i] = Value;
    }

    return ;
}
#endif

