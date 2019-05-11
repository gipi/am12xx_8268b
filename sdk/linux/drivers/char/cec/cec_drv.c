/***************************************************************************
	cec_drv.c
 ***************************************************************************/

/*=============================================================
 * Copyright (c)      Actionsmicro Corporation, 2004 * 
 * All rights reserved.                                       *
 *============================================================*/
 
/***************************************************************************
                          cec_drv.c  -  description
                             -------------------
    begin                : Thursday February 13, 2013
    copyright            : (C) 2013 by Yang Jianying
    email                : yangjy@actions-micro.com 
 ***************************************************************************/

/**
 * @file cec_drv.c
 * CEC Low Level driver.
 *
 * @author Yang Jianying
 * @email yangjy@actions-micro.com  
 * @date Thursday February 13, 2013
 * @version 0.1
 * @ingroup pres_hdmi
 *
 */
#include "cec_drv.h"
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/smp_lock.h>
#include <linux/major.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/device.h>



#define act_writel(val,reg)  (*(volatile int *)(reg) = (val))                
#define act_readl(port)  (*(volatile unsigned int *)(port))                                                           
static void RegBitSet(int val,int reg,int msb,int lsb)                                            
{                                             
unsigned int mask = 0xFFFFFFFF;
unsigned int old  = act_readl(reg);

	mask = mask << (31 - msb) >> (31 - msb + lsb) << lsb;
	act_writel((old & ~mask) | ((val << lsb) & mask), reg);	         
} 

void cec_recv(CEC_CHAN *pChan)
{
UINT8 *rx_buf;
UINT8 rx_len;
INT32 j;

	if(pChan->channelMode) {
		if(CEC_GetRxInt(pChan)) {
//printk("rx(%d)0x%08x\n", pChan->port, REG(pChan, CEC_TxRxCR));
			CEC_ClrRxInt(pChan);
			if(!RxIsFull(pChan)) {
				/* reserved first byte for packet length */
				rx_buf = &pChan->rx_buf[pChan->rxPutIdx*CEC_FRAME_LEN+1];
				rx_len = 0;
				while(CEC_GetRxLen(pChan)) {
					rx_buf[rx_len] = REG(pChan, CEC_RxDR);
					rx_len++;
				}
				pChan->rx_buf[pChan->rxPutIdx*CEC_FRAME_LEN] = rx_len;
				/* debug */
				if(rx_len) {
					printk("cec%d rx(%d): ",pChan->port,rx_len);
					for(j=0;j<rx_len; j++)
						printk("%02x ", rx_buf[j]);
					printk("\n");
				}
				//
				/* set to next queue */
				pChan->rxPutIdx++;
				pChan->rxPutIdx &= (CEC_QUEUE_LEN-1);
			}
			//if(!TxIsEmpty(pChan))
			//	cec_send(pChan);
			//else 
			{
				/* enable Rx */
				CEC_SetRxReset(pChan);
				CEC_ClrRxReset(pChan);
				CEC_SetRxEn(pChan);
				CEC_SetRxIntEn(pChan);
			}
		}
	}
	return;
}

void cec_send(CEC_CHAN *pChan)
{
UINT8 *buf;
UINT8 i, n;

	/*if(CEC_GetTxEn(pChan))
	{
		printk("%d tx(%d)0x%08x\n",__LINE__, pChan->port, REG(pChan, CEC_TxCR0));
		return;
	}*/	
	if(CEC_GetRxLen(pChan))
	{
		printk("%d tx(%d)0x%08x\n",__LINE__, pChan->port, REG(pChan, CEC_RxCR0));
		return;
	}

	//printk("%d tx(%d)0x%08x\n",__LINE__, pChan->port, REG(pChan, CEC_TxCR0));

	if(CEC_GetTxInt(pChan)) {
		CEC_ClrTxInt(pChan);
		pChan->txBusy = 0;
	}
	
	if(pChan->txBusy) {
		//printk("busy = %d\n", 	pChan->txBusy);
		//return;
	}
	/* if HW is not sending frame, de-queue a frame to send */
	if(!TxIsEmpty(pChan)) {
		/* disable Rx */
		//CEC_ClrRxEn(pChan);
		//CEC_SetRxReset(pChan);
		//CEC_ClrRxReset(pChan);
		
		n = pChan->tx_buf[pChan->txGetIdx*CEC_FRAME_LEN];
		buf = &pChan->tx_buf[pChan->txGetIdx*CEC_FRAME_LEN+1];
		CEC_SetTxReset(pChan);
		CEC_ClrTxReset(pChan);
		//printk("buf[0]&0xf=0x%x\n",buf[0]&0xf);
		CEC_SetDestAddr(pChan,(buf[0]&0xf));
		//printk("DestAddr =0x%x\n",CEC_GetDestAddr(pChan));
		for(i=1; i<n; i++)
			REG(pChan, CEC_TxDR) = (UINT32)(buf[i]);

		//printk("%d tx(%d)0x%08x TxDR 0x%08x\n",__LINE__, pChan->port, REG(pChan, CEC_TxCR0),REG(pChan, CEC_TxDR));

		pChan->txGetIdx++;
		pChan->txGetIdx &= (CEC_QUEUE_LEN-1);
		pChan->txBusy = 1;
		/* trigger to Tx */
		CEC_SetRxEn(pChan);
		CEC_SetTxEn(pChan);
		CEC_SetTxIntEn(pChan);
		/*printk("cec%d tx(%d): ", pChan->port, n);
		for(i=0; i<n; i++) {
			printk("%02x ", buf[i]);
		}			
		printk("\n");*/
		while(!CEC_GetTxInt(pChan));
	}
	else {
		/* enable Rx */
		CEC_SetRxReset(pChan);
		CEC_ClrRxReset(pChan);
		CEC_SetRxEn(pChan);
		CEC_SetRxIntEn(pChan);
	}
	//printk("%d tx(%d)0x%08x\n",__LINE__, pChan->port, REG(pChan, CEC_TxCR0));
	return;
}

ssize_t cec_read (struct file *file, char *buf, size_t count,
                            loff_t *offset)
{
	char *tmp;
	int rx_len;
	char *rx_buf;
	struct inode *inode = file->f_dentry->d_inode;
	unsigned int minor = 0;//MINOR(inode->i_rdev);
	CEC_CHAN *pChan;

	if(minor >= N_CEC_CHAN)
		return -ENODEV;
	
	pChan = file->private_data;
	
#ifdef DEBUG
	printk("cec_drv.o: i2c-%d reading %d bytes.\n",MINOR(inode->i_rdev),
	       count);
#endif

	if(RxIsEmpty(pChan))
		return 0;

	rx_len = pChan->rx_buf[pChan->rxGetIdx*CEC_FRAME_LEN];
	rx_buf = &pChan->rx_buf[pChan->rxGetIdx*CEC_FRAME_LEN+1];
	pChan->rxGetIdx++;
	pChan->rxGetIdx&=(CEC_QUEUE_LEN-1);

	if(copy_to_user(buf, rx_buf, rx_len))
		return -EFAULT;

	return rx_len;
}

ssize_t cec_write (struct file *file, const char *buf, size_t count,loff_t *offset)
{
	int ret;
	UINT8 *tmp;
	struct inode *inode = file->f_dentry->d_inode;
	unsigned int minor = 0;//MINOR(inode->i_rdev);
	CEC_CHAN *pChan;
	
	if(minor >= N_CEC_CHAN)
		return -ENODEV;
	if (!file)
		pChan = &cec_chan[minor];
	else
		pChan = file->private_data;

	/* copy user space data to kernel space. */
	if(TxIsFull(pChan))
		return -EFAULT;

	pChan->tx_buf[pChan->txPutIdx*CEC_FRAME_LEN] = count;
	tmp = &pChan->tx_buf[pChan->txPutIdx*CEC_FRAME_LEN+1];
	if (copy_from_user(tmp,buf,count)) {
		return -EFAULT;
	}
	pChan->txPutIdx++;
	pChan->txPutIdx &= (CEC_QUEUE_LEN-1);

	cec_send(pChan);

#ifdef DEBUG
	printk("cec_drv.o: i2c-%d writing %d bytes.\n",MINOR(inode->i_rdev),
	       count);
#endif
	return count;
}

int cec_ioctl (struct inode *inode, struct file *file, unsigned int cmd,unsigned long arg)
{
unsigned int minor = 0;//MINOR(inode->i_rdev);
CEC_CHAN *pChan;
UINT32 val;

	if(minor >= N_CEC_CHAN)
		return -ENODEV;
	
	pChan = file->private_data;
#if 1
	switch(cmd) {	
	case CEC_GET_DEST_ADDR:
		put_user(CEC_GetDestAddr(pChan), (INT8 *)arg);
		break;
	case CEC_SET_DEST_ADDR:
		get_user(val, (INT32 *)arg);
		CEC_SetDestAddr(pChan, val);
		break;
	case CEC_GET_SRC_ADDR:	
		put_user(CEC_GetLogicalAddress(pChan), (INT8 *)arg);
		break;
	case CEC_SET_SRC_ADDR:	
		get_user(val, (INT32 *)arg);
		CEC_SetLogicalAddress(pChan, val);
		break;
	case CEC_GET_DAC_CLOCK:	
		put_user(CEC_GetDACClock(pChan), (INT8 *)arg);
		break;
	case CEC_SET_DAC_CLOCK:	
		get_user(val, (INT32 *)arg);
		CEC_SetDACClock(pChan, val);
		break;
	case CEC_GET_CLOCK_DIVISOR:	
		put_user(CEC_GetClockDivisor(pChan), (INT8 *)arg);
		break;
	case CEC_SET_CLOCK_DIVISOR:	
		get_user(val, (INT32 *)arg);
		CEC_SetClockDivisor(pChan, val);
		break;
		
	case CEC_GET_TX_MAX_RETRANSMIT:	
		put_user(CEC_GetTxMaxRetransmit(pChan), (INT8 *)arg);
		break;
	case CEC_SET_TX_MAX_RETRANSMIT:	
		get_user(val, (INT32 *)arg);
		CEC_SetTxMaxRetransmit(pChan, val);
		break;


	case CEC_GET_RX_START_LOW_MIN:	
		put_user(CEC_GetRxStartLowMin(pChan), (INT8 *)arg);
		break;
	case CEC_SET_RX_START_LOW_MIN:	
		get_user(val, (INT32 *)arg);
		CEC_SetRxStartLowMin(pChan, val);
		break;

	case CEC_GET_RX_START_PERIOD_MAX:	
		put_user(CEC_GetRxStartPeriodMax(pChan), (INT8 *)arg);
		break;
	case CEC_SET_RX_START_PERIOD_MAX:	
		get_user(val, (INT32 *)arg);
		CEC_SetRxStartPeriodMax(pChan, val);
		break;

	case CEC_GET_RX_DATA_SAMPLE:
		put_user(CEC_GetRxDataSample(pChan), (INT8 *)arg);
		break;
	case CEC_SET_RX_DATA_SAMPLE:
		get_user(val, (INT32 *)arg);
		CEC_SetRxDataSample(pChan, val);
		break;

	case CEC_GET_RX_DATA_PERIOD_MIN:
		put_user(CEC_GetRxDataPeriodMin(pChan), (INT8 *)arg);
		break;
	case CEC_SET_RX_DATA_PERIOD_MIN:
		get_user(val, (INT32 *)arg);
		CEC_SetRxDataPeriodMin(pChan, val);
		break;

	case CEC_GET_TX_START_LOW_MIN:	
		put_user(CEC_GetTxStartLowMin(pChan), (INT8 *)arg);
		break;
	case CEC_SET_TX_START_LOW_MIN:	
		get_user(val, (INT32 *)arg);
		CEC_SetTxStartLowMin(pChan, val);
		break;

	case CEC_GET_TX_START_PERIOD_MAX:	
		put_user(CEC_GetTxStartPeriodMax(pChan), (INT8 *)arg);
		break;
	case CEC_SET_TX_START_PERIOD_MAX:	
		get_user(val, (INT32 *)arg);
		CEC_SetTxStartPeriodMax(pChan, val);
		break;

	case CEC_GET_TX_DATA_SAMPLE:
		put_user(CEC_GetTxDataSample(pChan), (INT8 *)arg);
		break;
	case CEC_SET_TX_DATA_SAMPLE:
		get_user(val, (INT32 *)arg);
		CEC_SetTxDataSample(pChan, val);
		break;

	case CEC_GET_TX_DATA_PERIOD_MIN:
		put_user(CEC_GetTxDataPeriodMin(pChan), (INT8 *)arg);
		break;
	case CEC_SET_TX_DATA_PERIOD_MIN:
		get_user(val, (INT32 *)arg);
		CEC_SetTxDataPeriodMin(pChan, val);
		break;

	case CEC_GET_TX_DATA_HIGH:
		put_user(CEC_GetTxDataHigh(pChan), (INT8 *)arg);
		break;
	case CEC_SET_TX_DATA_HIGH:
		get_user(val, (INT32 *)arg);
		CEC_SetTxDataHigh(pChan, val);
		break;
		
	case CEC_GET_PHYSICAL_ADDR:		
		put_user(physical_address, (unsigned int *)arg);
		break;
	default:
		break;
	}
#endif
	return 0;
}

int cec_open (struct inode *inode, struct file *file)
{
	unsigned int minor = 0;//MINOR(inode->i_rdev);
	CEC_CHAN *pChan;
	
	if(minor >= N_CEC_CHAN)
		return -ENODEV;

	if(!file->private_data)
		file->private_data = &cec_chan[minor];
	else
		return -EEXIST;
	return 0;
}

int cec_release (struct inode *inode, struct file *file)
{
	unsigned int minor = 0;//MINOR(inode->i_rdev);
	CEC_CHAN *pChan;
	if(minor >= N_CEC_CHAN)
		return -ENODEV;
	
	pChan = file->private_data;
	file->private_data=NULL;
	
#ifdef DEBUG
	printk("cec_drv.o: Closed: cec%d\n", minor);
#endif
	return 0;
}

int cec_get_physical_address(unsigned int* phys_addr)
{
	unsigned char edid[256], block_length, checksum = 0;
	const unsigned char edid_marker[] = {0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00};
	int i, r, block_start;
	struct file *fp=NULL;
	mm_segment_t fs; 

	*phys_addr = 0xFFFF;	/* undetermined */

	fs = get_fs(); 
	set_fs(KERNEL_DS);
	//for EDID device ID 
	fp = filp_open(EDID_INFO_PATH, O_RDONLY, 0644);
	if (IS_ERR(fp)) { 
	   		printk("open /mnt/vram/edidinfo.bin error\n"); 
		return -EINVAL;
	}
	r = vfs_read(fp,edid,256, &fp->f_pos);
	filp_close(fp,NULL);
	set_fs(fs);
	/*printk("@@@@@@@@@@@@@@@@@@r=%d\n",r);
	if (r==256)
	{
		for (i=0;i<256;i++)
		{
			if (i%8==0)
				printk("\n");
			printk("0x%x ",edid[i]);
		}
	}
	else
	{
		printk("read /mnt/vram/edidinfo.bin error\n");
		return -EINVAL;
	}*/

	/* Confirm that we have an HDMI device */
	if (edid[0x7e] == 0) {
		printk("display device does not appear to be HDMI");
		return -EINVAL;
	}

	/* Check that we have an E-EDID extension */
	if ((edid[0x7e] == 0) || (edid[0x80] != 0x02) || (edid[0x81] != 0x03)) {
		printk("E-EDID marker not found");
		return -EINVAL;
	}

	for (block_start=0x84; block_start<256; ) {
		block_length = (edid[block_start] & 0x1f) + 1;
		switch((edid[block_start]>>5)&7) {
		case 3:	/* HDMI VSDB */
			if ( (edid[block_start+1] == 0x03) && (edid[block_start+2] == 0x0c)
				&& (edid[block_start+3] == 0x00) ) {
				/* HDMI IEEE Registration Identifier */
				*phys_addr = (edid[block_start+4]<<8) + edid[block_start+5];
				printk("found physical address %04X\n", *phys_addr);
				return 0;
			}
		default:
			block_start += block_length;
		}
	}

	return -EINVAL;
}

int cec_allocate_logical_address(unsigned char device_type, unsigned int* physical_address)
{
	const unsigned char logical_address_table[15] = {0, 1, 1, 3, 4, 5, 3, 3, 4, 1, 3, 4, 2, 2, 0};
	int r;
	unsigned char logical_address, polling_message;
	unsigned int minor = 0;//MINOR(inode->i_rdev);
	CEC_CHAN *pChan;

	pChan = &cec_chan[minor];
	if(!pChan->channelMode)
		return -EEXIST;

	switch (device_type) {
		case 0:
		case 1:
		case 3:
		case 4:
		case 5:
			break;
		default:
			return -EINVAL;
	}

	/* Set our logical address to unregistered during allocation */
	CEC_SetLogicalAddress(pChan, 15);
	logical_address = 15;
	printk("switched to unregistered logical address\n");

	r = cec_get_physical_address(physical_address);
	if (r)
	{
		printk("get physical_address error r=%d\n",r);
		return -EINVAL;
	}
	printk("physical address: %d.%d.%d.%d\n", (*physical_address>>12)&0xF,
		(*physical_address>>8)&0xF, (*physical_address>>4)&0xF, (*physical_address>>0)&0xF);
	if (*physical_address == 0xFFFF) {
		printk(" keep unregistered\n");
		return -EINVAL;	/* keep unregistered */
	}

	/* TV as root */
	if (*physical_address == 0x0000) {
		if (device_type != 0) {
			printk("invalid device type for physical address 0.0.0.0 - must be TV\n");
			return -EINVAL;
		}
		printk("using logical address 0\n");
		CEC_SetLogicalAddress(pChan, 0);
		logical_address = 0;
		return logical_address;
	}

	for (logical_address = 1; logical_address < 15; logical_address++) {
		if (logical_address_table[logical_address] != device_type) {
			continue;
		}

		CEC_SetTxReset(pChan);
		CEC_ClrTxReset(pChan);
		CEC_SetDestAddr(pChan,logical_address);
		CEC_SetTxAddr(pChan,logical_address);
		CEC_SetTxAddrEn(pChan);
		/* trigger to Tx */
		CEC_SetRxEn(pChan);
		CEC_SetTxEn(pChan);
		CEC_SetTxIntEn(pChan);
		printk("querying logical address %d\n", logical_address);
		while(!CEC_GetTxInt(pChan));
			//printk("%d tx(%d)0x%08x\n",__LINE__, pChan->port, REG(pChan, CEC_TxCR0));
		//printk("%d tx(%d)0x%08x\n",__LINE__, pChan->port, REG(pChan, CEC_TxCR0));
		if (!CEC_GetTxEom(pChan))
		{
			/* error on polling (no ACK) => assume address is free */			
			printk("using logical address %d\n", logical_address);
			CEC_SetLogicalAddress(pChan, logical_address);
			return logical_address;			
		}	
		printk("address already in use - trying next\n");		
	}
	/* Set our logical address to unregistered during allocation */
	CEC_SetLogicalAddress(pChan, 15);
	logical_address = 15;
	printk("exhausted all possible logical addresses - keeping unregistered (15)\n");
	return logical_address;
}

static int cec_hw_init (unsigned int cec_device_type)
{
	CEC_CHAN *pChan;
	unsigned char logical_address, polling_message;
	UINT8 *buf;
	UINT16 DACClockDiv = 246;
	UINT16 CecTimerDiv = 20;
	const unsigned char BusClkCoreClkDiv[7] = {4, 4, 5, 6, 3, 7, 8};
	pChan = &cec_chan[0];
	if(pChan->channelMode)
		return -EEXIST;
	pChan->regs = CEC_BASE_ADDR;
	pChan->channelMode = 1;	/* active */

	
	RegBitSet(1,GPIO_MFCTL0,17,17); 
	/* Enable CEC */
	REG(pChan, CEC_CR0) = CEC_CR0_ENABLE;

	/* set Rx and Tx ready */
   	/* Send Tx and Rx Reset pulse */
	REG(pChan, CEC_TxCR0) = CEC_TXCR0_RESET;
	REG(pChan, CEC_RxCR0) = CEC_RXCR0_RESET;
	REG(pChan, CEC_TxCR0) = 0;
	REG(pChan, CEC_RxCR0) = 0;
	REG(pChan, CEC_TxCR0) = CEC_TXCR0_INT_EN | CEC_TXCR0_ENABLE;
	REG(pChan, CEC_RxCR0) = CEC_RXCR0_INT_EN | CEC_RXCR0_ENABLE;
   
	// Control Register
#if CONFIG_AM_CHIP_MINOR == 8268
	DACClockDiv = (GET_BITS((*(volatile UINT32 *)(0xb0010000)),6,1)*24*2/BusClkCoreClkDiv[GET_BITS((*(volatile UINT32 *)(0xb001000c)),30,28)])*10/8;
#else
	DACClockDiv = (GET_BITS((*(volatile UINT32 *)(0xb0010000)),8,2)*6*2/BusClkCoreClkDiv[GET_BITS((*(volatile UINT32 *)(0xb001000c)),30,28)])*10/8;
#endif

	if (DACClockDiv > 0xff)
#if CONFIG_AM_CHIP_MINOR == 8268
	{
		//8268 BusClk=348 348/210/41=0.0404M
		DACClockDiv = 210;
		CecTimerDiv = 41;
	}
#else
		DACClockDiv = 0xff;
#endif
	CEC_SetDACClock(pChan, DACClockDiv);  //BusClock 492/2.5 = 196.8 /0.8 = 246
	CEC_SetDACTestData(pChan, 0);
	//CEC_SetLogicalAddress(pChan, CEC1_LogicADDR);*/
	CEC_SetClockDivisor(pChan, CecTimerDiv);

	// ReTry
    CEC_SetTxMaxRetransmit(pChan, 2);
        
	// CEC Rx Timing 
	CEC_SetRxStartLowMin(pChan, 140);
	CEC_SetRxStartPeriodMax(pChan, 188);
	CEC_SetRxDataSample(pChan, 42);
	CEC_SetRxDataPeriodMin(pChan, 82);

	// CEC Tx Timing
	CEC_SetTxStartLowMin(pChan, 148);
	CEC_SetTxStartPeriodMax(pChan, 32);
	CEC_SetTxDataSample(pChan, 24);
	CEC_SetTxDataPeriodMin(pChan, 36);
	CEC_SetTxDataHigh(pChan, 36);
	
	/* Initialize Queue index */
	pChan->txPutIdx = 0;
	pChan->txGetIdx = 0;
	pChan->rxPutIdx = 0;
	pChan->rxGetIdx = 0;
	
	logical_address = cec_allocate_logical_address(cec_device_type,&physical_address);
	printk("physical_address=0x%x logical_address=%d\n",physical_address,logical_address);
	if (logical_address < 0)
	{
		printk("logical_address error %d\n",logical_address);
	}	

	pChan->tx_buf[pChan->txPutIdx*CEC_FRAME_LEN] = 5;
	buf = &pChan->tx_buf[pChan->txPutIdx*CEC_FRAME_LEN+1];
	buf[0] = BROADCAST;
	buf[1] = CEC_OP_REPORT_PHYSICAL_ADDRESS;
	buf[2] = physical_address >> 8;
	buf[3] = physical_address & 0xFF;
	buf[4] = device_type;
	pChan->txPutIdx++;
	pChan->txPutIdx &= (CEC_QUEUE_LEN-1);
	cec_send(pChan);
	
#ifdef DEBUG
	printk("cec_drv.o: opened cec%d\n",minor);
#endif
	return 0;
}

static struct file_operations cec_fops ={
	read:		cec_read,
	write:		cec_write,
	ioctl:		cec_ioctl,
	open:		cec_open,
	release:	cec_release,
};

static int __devexit am7x_cec_remove(struct platform_device *pdev)
{
	CEC_CHAN *pChan;

	pChan = &cec_chan[0];
	/* set Rx and Tx disable */
	REG(pChan, CEC_TxCR0) &= ~(CEC_TXCR0_ENABLE | CEC_TXCR0_INT_EN);
	REG(pChan, CEC_RxCR0) &= ~(CEC_RXCR0_ENABLE | CEC_RXCR0_INT_EN);
	pChan->channelMode = 0;
	return 0;
}

static int am7x_cec_major = 0;
struct cdev *am7x_cec_cdev;
struct class *am7x_cec_class;
dev_t am7x_cec_dev;

static int __devinit am7x_cec_probe(struct platform_device *pdev)
{
	int result=-1;
	int count=1;
	printk("cec probe\n");
/*
 *	Basic hardware initialize
*/	
	cec_hw_init(device_type);
/*
 *	Register the  cec char device to system	
*/
	if(am7x_cec_major)
	{
		am7x_cec_dev = MKDEV(am7x_cec_dev, 0);
		result = register_chrdev_region(am7x_cec_dev, count, "am7x-cec");
	}
	else
	{
		result = alloc_chrdev_region(&am7x_cec_dev, 0, count, "am7x-cec");
		am7x_cec_major = MAJOR(am7x_cec_dev);
	}

	am7x_cec_cdev = cdev_alloc();
	cdev_init(am7x_cec_cdev,&cec_fops);
	am7x_cec_cdev->owner = THIS_MODULE;
	
	result = cdev_add(am7x_cec_cdev, am7x_cec_dev, count);
	if(result < 0)
	{
		printk("can't add [%s]dev", __func__);
		unregister_chrdev_region(am7x_cec_dev, count);
		cdev_del(am7x_cec_cdev);
		return result;
	}

	am7x_cec_class = class_create(THIS_MODULE, "am7x_cec_class");
	if(IS_ERR(am7x_cec_class)) {
		unregister_chrdev_region(am7x_cec_dev, count);
		cdev_del(am7x_cec_cdev);
		printk("am7x_cec Err: failed in creating class.\n");
		return -1; 
	}

	device_create(am7x_cec_class, NULL, am7x_cec_dev, NULL, "am7x-cec");
	printk( "am7x-cec: module inserted. Major = %d\n", am7x_cec_major);
	return 0;
}

static struct platform_driver am7x_cec_driver = {
	.probe = am7x_cec_probe,
	.remove = am7x_cec_remove,
	//.suspend = am7x_lcm_suspend, /* optional but recommended */
	//.resume = am7x_lcm_resume,   /* optional but recommended */
	.driver = {
		.name = "cec-am7x",  //same name as in platform
	},
};


static int __init am7x_cec_init(void)
{
	int i;
	int count=1;
	for(i=0; i<N_CEC_CHAN; i++) {
	CEC_CHAN *pChan = &cec_chan[i];
		pChan->channelMode = 0;
	}
	printk("am7x_cec_init\n");
	return platform_driver_register(&am7x_cec_driver);
}

static void __exit am7x_cec_exit(void)
{
	platform_driver_unregister(&am7x_cec_driver);
}

module_init(am7x_cec_init);
module_exit(am7x_cec_exit);
MODULE_LICENSE("GPL");


