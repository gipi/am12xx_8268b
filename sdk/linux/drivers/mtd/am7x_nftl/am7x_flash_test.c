#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/hdreg.h>

#include <linux/kmod.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nftl.h>
#include <linux/mtd/blktrans.h>

#include "linux/kthread.h"
#include <linux/time.h>
#include <linux/fs.h>
#include <linux/syscalls.h>
#include <asm-mips/unaligned.h>
#include "../mtdcore.h"

#include "linux/blkdev.h"
#include "sys_cfg.h"
#include "nand_flash_driver_type.h"
#include "linux/usb_mediaops.h"
#include <am7x_board.h>

#include <linux/semaphore.h>
#include <linux/interrupt.h>
#include <dma.h>
#include <irq.h>
#include "nand_reg_def-1201.h"
#include <actions_io.h>
#include <actions_regs.h>
#include <sys_nand.h>


#if TEST_FLASH_SPEED 
static INT32U Flash_read_Test(INT32U  Start_LBA,INT32U len_per_read,INT32U FileSize,INT8U *Buf)
{
	INT8U     *buffer;
	INT32U iLoop=0;
	INT32U result;	
	INT32U  lba,sectors,bNum;    
	struct timeval tpstart,tpend; 
	unsigned int span, speed;

	
	lba =Start_LBA;
	sectors = FileSize/512;
	bNum =len_per_read/512;
	buffer = Buf;	
	
	for(iLoop=lba;iLoop<(sectors+lba);iLoop+=bNum)
	{	    
		do_gettimeofday(&tpstart); 
		//bPrintFlag =0x01;
		
		//result=FTL_Read(iLoop,bNum,buffer);
		nftl_read_Mulitblock(NULL,iLoop,bNum,buffer);
		//bPrintFlag =0x00;
		do_gettimeofday(&tpend); 
		span  = 1000000*(tpend.tv_sec-tpstart.tv_sec) + tpend.tv_usec-tpstart.tv_usec; 
		///span /= 1000;
		//printk("%x,%x,%d\n",iLoop,bNum,span);
		
	}
	
       return 0;
}
static int Flash_write_Test(INT32U  Start_LBA,INT32U len_per_read,INT32U FileSize,INT8U *Buf)
{
	
	INT8U     *buffer;
	INT32U iLoop=0;
	INT32U result;	
	INT32U  lba,sectors,bNum; 
	lba =Start_LBA;
	sectors = FileSize/512;
	bNum =len_per_read/512;
	buffer = Buf;	

	for(iLoop=lba;iLoop<(sectors+lba);iLoop+=bNum)
	{	       
		result=FTL_Write(iLoop,bNum,buffer);
	}

	return 0;
	
         
}

static void report_speed(struct timeval *start, struct timeval *end, size_t iob_size, size_t total)
{
	unsigned int span, speed;

	span  = 1000000*(end->tv_sec-start->tv_sec) + end->tv_usec-start->tv_usec; 
	span /= 1000;
 
//	speed = (1000 * total) / (1024 *  span);
	speed = (total / 1024 * 1000) / span;
	printk("  %5uKB,  %5uKB,%5dms,  %u KB/S\n", iob_size/1024, total/1024, span, speed);    
	//printk("E: %lu.%lu\tS: %lu.%lu\n", end->tv_sec, end->tv_usec,
	//	start->tv_sec, start->tv_usec);
}


INT32U  Flash_Speed_Test(void)
{
	INT32U read_len_per_time;
	INT32U  FileSize;
	INT32U   iLoop;

	INT32U Start_lba;  	   
	INT8U   *TmpBuf;
	struct timeval tpstart,tpend; 
	TmpBuf =(void*)MALLOC(128 * 512);
#if 1	
	FileSize =32*1024*1024;  
	Start_lba =2048*200;
	printk("===========begin to test file flash api reading==========\n"); 
	printk("**Buflen,**Time ,  **Speed**\n");
	for(iLoop = 0x01;iLoop<8;iLoop++)
	{         
		read_len_per_time  =  (1<<iLoop)*512;    
		do_gettimeofday(&tpstart); 
		//bPrintFlag =0x01;
		///bPrintFlag2 =0x01;
		Flash_read_Test(Start_lba,read_len_per_time,FileSize,TmpBuf);
		///bPrintFlag2 =0x00;
		
		//bPrintFlag =0x00;
		do_gettimeofday(&tpend); 
		Get_bPageCntVal();
		report_speed(&tpstart, &tpend, read_len_per_time, FileSize);
	}
#endif	
#if 0
///*-------------------------------------------------------------------------//
	//	FileSize =4*1024*1024;   
	printk("===========begin to test file Writing==========\n"); 
	printk("FileSize:%d KB\n",FileSize/1024);
	FileSize =8*1024*1024;  
	Start_lba =2048*200;
	printk("**Buflen,**Time ,  **Speed**\n");
	for(iLoop = 0x05;iLoop<7;iLoop++)
	{
		read_len_per_time  = (1<<iLoop)*512;
		do_gettimeofday(&tpstart); 
		bPrintFlag =0x01;
		Flash_write_Test(Start_lba,read_len_per_time,FileSize,TmpBuf);	
		bPrintFlag =0x00;
		do_gettimeofday(&tpend); 		
	} 
#endif
	printk("### end of %s,%d###\n",__func__,__LINE__);
	FREE(TmpBuf);	
	return  0x00 ;
}
#endif



