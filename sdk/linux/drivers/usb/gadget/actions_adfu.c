/*
 * zero.c -- Gadget Zero, for USB development
 *
 * Copyright (C) 2003-2008 David Brownell
 * Copyright (C) 2008 by Nokia Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


/*
 * Gadget Zero only needs two bulk endpoints, and is an example of how you
 * can write a hardware-agnostic gadget driver running inside a USB device.
 * Some hardware details are visible, but don't affect most of the driver.
 *
 * Use it with the Linux host/master side "usbtest" driver to get a basic
 * functional test of your device-side usb stack, or with "usb-skeleton".
 *
 * It supports two similar configurations.  One sinks whatever the usb host
 * writes, and in return sources zeroes.  The other loops whatever the host
 * writes back, so the host can read it.
 *
 * Many drivers will only have one configuration, letting them be much
 * simpler if they also don't support high speed operation (like this
 * driver does).
 *
 * Why is *this* driver using two configurations, rather than setting up
 * two interfaces with different functions?  To help verify that multiple
 * configuration infrastucture is working correctly; also, so that it can
 * work with low capability USB controllers without four bulk endpoints.
 */

/*
 * driver assumes self-powered hardware, and
 * has no way for users to trigger remote wakeup.
 */

/* #define VERBOSE_DEBUG */

#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/device.h>
#include <am7x_flash_api.h>

#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>

#include "file_storage.h"
#include <sysinfo.h>
#include <sys_buf_def.h>
#include <sys_at24c02.h>


#define DEBUG_ADFU
#ifdef  DEBUG_ADFU
#define DBG_ADFU(fmt,stuff...)   printk(KERN_INFO fmt,##stuff)
#else
#define DBG_ADFU(fmt,stuff...)   do {} while (0)
#endif

#define ADFU_DATA_TYPE_SENDOK	0x00		
#define ADFU_DATA_TYPE_MBREC		0x06
#define ADFU_DATA_TYPE_BREC		0x07
#define ADFU_DATA_TYPE_WRDATA	0x08
#define ADFU_DATA_TYPE_WRLBA		0x09
#define ADFU_DATA_TYPE_RDDATA	0x0a
#define ADFU_DATA_TYPE_RDLBA		0x0b

#define ADFU_DATA_MAC_SENDOK		0x10		
#define ADFU_DATA_MAC_WRIIS		0x11
#define ADFU_DATA_MAC_RDIIS		0x12
#define ADFU_DATA_MAC_WRSPI		0x13
#define ADFU_DATA_MAC_RDSPI		0x14


#define ADFU_BUFSIZE					PAGE_SIZE*8

/*************************HWSC RETURN*********************************************/
static ADFU_SysInfo_t	  sysinfo = {
	.SYSInfoFlag = {'S','Y','S',' ','I','N','F','O'},
	.adfu_hwscaninfo = {
		.Frametype = {'H','W'},
		.ICVersion = 0x3963,
		.SubVersion = {0,'C'},
		.BromVersion = {0x13,0,0,0},
		.BromDate = {0x20,0x06,0x05,0x20},
	},
	.adfu_fwscaninfo = {
		.FrameType =  {'F','W'},
		.VID = DRIVER_VENDOR_ID,
		.PID = DRIVER_PRODUCT_ID,
		.fwVersion = "13.01.00.0000",
		.Productor = "Actions Semiconductor",
		.DeviceName = "HS UDISK DEVICE",
		.FirmwareDate = {10,11,06,20},
	},
};

static LFIHead_t lfi_header;

extern u32 get_buffer_len(void);
extern int  sleep_thread(struct fsg_dev *fsg);

/*======================================================*/
static inline int memcmp_l(const void * cs,const void * ct,unsigned int length)
{
		const u32 *su1, *su2;
		u32 count = length/4;
		int res = 0;
		for( su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
			if ((res = *su1 - *su2) != 0)
				break;
		return res;
}

static void fill_sysinfo(struct fsg_dev * fsg)
{
	int val = 0;
	int brec_block_size;	
	char tmp_buffer[512];
	sysinfo.adfu_hwscaninfo.BootDiskType = get_nand_flash_type();/*GET_NAND_FLASH_TYPE()*/;
	
	DBG_ADFU("%s:%d\n",__func__,__LINE__);
	switch(sysinfo.adfu_hwscaninfo.BootDiskType){
    	case NAND284:
    	case NAND644:
    	case NAND648:
    	       brec_block_size = (NAND_BREC_SIZE/512);
      	 	break;
      	case NAND649:
      	       brec_block_size = 64;
		break;
    	default:
		brec_block_size = (NAND_BREC_SIZE/512)	;
       		break;
    	}

	fsg->brec_block_size =   brec_block_size ;
	val = brec_sector_read(4,brec_block_size-1,fsg->adfu_buf);
	if(val)
		DBG_ADFU("brec sector read failed\n");

	DBG_ADFU("%s:%d(brec_block_size=%d)\n",__func__,__LINE__,brec_block_size);

	//get brec info
	sysinfo.adfu_fwscaninfo.Brecsign = get_le16(& fsg->adfu_buf[508]) /*((tmp[509]& 0xff) << 8 ) + (tmp[508]& 0xff)*/;
	sysinfo.adfu_fwscaninfo.BrecCheckSum = get_le16(& fsg->adfu_buf[510]) /*((tmp[511] & 0xff) << 8) + (tmp[510]& 0xff)*/;
	DBG_ADFU("brecsign is %x, brce checksum is %x\n",sysinfo.adfu_fwscaninfo.Brecsign,sysinfo.adfu_fwscaninfo.BrecCheckSum);

#if  1
	//get capability info
	DBG_ADFU("sizeof(lfi_header):%d\n",sizeof(lfi_header));
	nand_adfu_read(0,(void *)tmp_buffer,1);
	//print_hex_dump(KERN_ALERT , "raw data: ", DUMP_PREFIX_OFFSET,
	//		   16, 1,tmp_buffer, 512, 1);
	memcpy((void *)&lfi_header,tmp_buffer,512);

	DBG_ADFU("+++lfi_head:%s,head_check_sum:%x\n",lfi_header.Version,lfi_header.Headchecksum);	
	memcpy(&sysinfo.adfu_fwscaninfo.CapInfo,&lfi_header.CapInfo,sizeof(CapInfo_t));
	memcpy(sysinfo.adfu_fwscaninfo.DeviceName,lfi_header.DeviceName,sizeof(sysinfo.adfu_fwscaninfo.DeviceName));
	memcpy(sysinfo.adfu_fwscaninfo.Productor,lfi_header.Productor,sizeof(sysinfo.adfu_fwscaninfo.Productor));
	memcpy(&sysinfo.adfu_fwscaninfo.fwVersion,&lfi_header.Version,13);

	DBG_ADFU("+++lfi_capinfo:%x,adfu_fwscaninfo:%x\n",
				lfi_header.CapInfo.Logicalcap,
				sysinfo.adfu_fwscaninfo.CapInfo.Logicalcap);
#endif
}


static int verify_brec_checksum(struct fsg_dev *fsg,int brec_blocknum,u16 checksum)
{
	int i,k;
	int brec_block_size;	
	u16	brec_sum= 0;
	
	DBG_ADFU("checksum received is 0x%x\n",checksum);
	
	brec_block_size = fsg->brec_block_size;
	for(i = 0; i < (int)(brec_block_size-1); i++){
		brec_sector_read(brec_blocknum+4, i, fsg->adfu_buf);
		for(k = 0; k < 256; k++){
			brec_sum= brec_sum + get_le16(&fsg->adfu_buf[k*2]);
		}
	}

	brec_sector_read(brec_blocknum+4, i, fsg->adfu_buf);
	if(get_le16(&fsg->adfu_buf[508]) !=0x55AA){
		DBG_ADFU("verify 0x55AA failed\n");
		return 1;		
	}

	for(k = 0; k < 255; k++)
		brec_sum= brec_sum + get_le16(&fsg->adfu_buf[k*2]);	
	
	if (brec_sum != checksum ){
		DBG_ADFU("%s:write checksum=%04x, read checksum=%04x\n", \
			__FUNCTION__,checksum,brec_sum);
		return 1;
	}							
	
	return 0;
}


static int mbrc_write(int blocknum, int whole_sectors,void *buffer)
{
    int i;	
		
    DBG_ADFU("### %s,blk:%d,total:%d ###\n",
		__FUNCTION__,
		blocknum,
		whole_sectors);	
	
    for(i=0; i < whole_sectors ; i++)
        brec_sector_write(blocknum, i, buffer+512*i);

    DBG_ADFU("+++++===%s,blk:%d,total:%d ###\n",
		__FUNCTION__,
		blocknum,
		whole_sectors);	
    return 0;
}


static int mbrc_read_compare(int blocknum, int whole_sectors, void *pmbrec, void *buffer)
{		
	int i, ret = 0;
	int * tmp;
	DBG_ADFU("+++++++blocknum:%d,whole_sectors:%d\n",blocknum,whole_sectors);
	
	for(i=0; i<whole_sectors; i++){
		tmp = (int *)(pmbrec + i*512);

		//DBG_ADFU("nand data of block:%d,  sector:%d\n",blocknum,i);
		ret = brec_sector_read(blocknum, i, buffer);
		if(ret)
			break;
			
		ret = memcmp_l((int *)buffer,(int *)tmp, 512);
		if(ret){
			//DBG_ADFU("++++mbrc read compare not equal,sectors_num:%d\n",i);		
			//print_hex_dump(KERN_ALERT, "mbrec data: ", DUMP_PREFIX_OFFSET,
			//	16, 1, buffer, 16, 1);	
			
			//print_hex_dump(KERN_ALERT, "pc data: ", DUMP_PREFIX_OFFSET,
			//		16, 1, tmp, 16, 1);
			break;
		}
	}
	
	return ret;	//equal
}


//0xcc
int do_adfu_get_devid(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
	u8    *buf = (u8 *)bh->buf;
	char temp[10];
	
	DBG_ADFU("%s:%d\n",__FUNCTION__,__LINE__);
	
	memset(temp,0,sizeof(temp));
	if(fsg->lun == 0){
		sprintf(temp,"%04x:%04x",DRIVER_VENDOR_ID,
			DRIVER_PRODUCT_ID);
	}
	
	memcpy(buf,temp,fsg->data_size_from_cmnd);
	return fsg->data_size_from_cmnd;
}


//0xcb
int do_adfu_test_dev_ready(struct fsg_dev *fsg, struct fsg_buffhd *bh) 
{
	char *buf = (char *) bh->buf;
	memset(buf,0,fsg->data_size_from_cmnd);

	DBG_ADFU("%s:%d\n",__FUNCTION__,__LINE__);

	if(!fsg->adfu_buf){
		fsg->adfu_buf = kzalloc(ADFU_BUFSIZE,GFP_KERNEL);
		if(!fsg->adfu_buf){
			DBG_ADFU("%s:%d alloc buffer failed\n",__FUNCTION__,__LINE__);
			goto not_ready;
		}
	}
	
	buf[0] = 0xff;
	buf[1] = 0x00;
	fsg->adfu_sense = SS_NO_SENSE;
not_ready:	
	return fsg->data_size_from_cmnd;
}



//0xaf
int do_adfu_get_sysinfo(struct fsg_dev *fsg, struct fsg_buffhd *bh) 
{
	int length = sizeof(sysinfo);
	
	DBG_ADFU("desire_size:%d,actual_size:%d\n",
		fsg->data_size_from_cmnd,
		length);

	fill_sysinfo(fsg);
	memcpy(bh->buf,&sysinfo,length/*fsg->data_size_from_cmd*/);
	return length /*fsg->data_size_from_cmd*/;
}



/***********************************************************/
//0xb0...0x06
static int do_adfu_update_mbrec(struct fsg_dev  *fsg,struct fsg_buffhd *bh)
{
	int i,j,multi_en;
	const char *multi_boot_flag = "MultiSectorBoot";
	unsigned int  *pmbrec;
	unsigned int sector_num,whole_sector_num,mbrec_cnt;
	int pagecnt_per_blk;
	int result;
	int amount;
	
	fsg->adfu_sense = SS_WRITE_ERROR;
	amount = fsg->data_size_from_cmnd;
	
	BUG_ON(amount > get_buffer_len());	
	
	if(bh->state != BUF_STATE_EMPTY){
		result = sleep_thread(fsg);
		if(result)
			return result;	
	}

	bh->outreq->length = bh->bulk_out_intended_length = amount;

	DBG_ADFU("%s:%d amount:%d, bulk_out_length:%d,buf:%x\n",__FUNCTION__,
		__LINE__,amount,bh->outreq->length,(u32)bh->buf);

	start_transfer(fsg,fsg->bulk_out,bh->outreq,&bh->outreq_busy, &bh->state);		

	/* Wait for something to happen */	
	while(bh->state != BUF_STATE_FULL){
		result = sleep_thread(fsg);
		if (result)
			return result;
	}
	
	if(bh->outreq->status != 0){
		DBG_ADFU("get data failed\n");
		return -EIO;
	}
	
	DBG_ADFU("%s:%d,actual:%d,buf:%x\n",__FUNCTION__,
		__LINE__,bh->outreq->actual,(u32)bh->outreq->buf);

    //@fish add 2011-05-06 
    Stop_Check_Recover(CHECK_STOP_VAL);

	if(!strcmp(multi_boot_flag, bh->buf)){
		sector_num = *(volatile unsigned int*)(bh->buf + 0x28);//must before createTbl()
		whole_sector_num = sector_num++;
		multi_en = 1;
	}else{
		sector_num = 1;
		multi_en = 0;
		whole_sector_num = 1;
	}

	if(multi_en){	
			//@fish add 2009-11-18 临时接口定义，
			/*if(SNOR080 ==GET_NAND_FLASH_TYPE()
			||SNOR090 ==GET_NAND_FLASH_TYPE()){
			page_cnt_per_phy_blk =0x00;//GetPageCntPerPhyBlk();
			mbrecCnt =0x01;
			pMBRC = (volatile unsigned int *)(bh->buf);//PAGE_CNT_PER_PHY_BLK
			*(pMBRC+0x1fc/4) -= (*(pMBRC+0x60/4));
			*(pMBRC+0x60/4) = page_cnt_per_phy_blk;      
			*(pMBRC+0x1fc/4) += (*(pMBRC+0x60/4));	
		}
		else*/
		{
			//page_cnt_per_phy_blk = GetNandStorageInfo()->PageNumPerPhyBlk;
			pagecnt_per_blk = GetNandStorageInfo()->PageNumPerPhyBlk;
			mbrec_cnt =0x04;
			pmbrec =(unsigned int*)bh->buf;//PAGE_CNT_PER_PHY_BLK
			*(pmbrec+0x1fc/4) -= *(pmbrec+0x60/4);
			*(pmbrec+0x60/4)  = pagecnt_per_blk;      
			*(pmbrec+0x1fc/4) += *(pmbrec+0x60/4);	
		}
	}
	else{
		mbrec_cnt=0x01;
	}

	DBG_ADFU("###whole_sector_num = %d,mbrec_cnt = %d,pages_perblk:%d###\n",
				whole_sector_num,mbrec_cnt,pagecnt_per_blk);

	for(i=0; i<mbrec_cnt; i++){	
		/*we suppose at least one write is correct in the four tries*/
		result = mbrc_read_compare(i,whole_sector_num, bh->buf, fsg->adfu_buf);
		if(!result)
			continue;

		for(j=0;j<2;j++){
			mbrc_write(i, whole_sector_num,bh->buf);
			result = mbrc_read_compare(i,whole_sector_num, bh->buf, fsg->adfu_buf);
			if(!result)
				break;
		}
	}

	if(!result)
		fsg->adfu_sense = SS_NO_SENSE;
	else
		DBG_ADFU("update mbrec failed\n");

	bh->state = BUF_STATE_EMPTY;
	fsg->residue -=amount;
	fsg->usb_amount_left -= amount;

	DBG_ADFU("%s:%d\n",__FUNCTION__,__LINE__);
	return -EIO;
}


//0xb0...0x07
static int do_adfu_update_brec(struct fsg_dev *fsg)
{
  struct fsg_buffhd *bh;
  u32   amount,amount_left_to_req, amount_left_to_write;
  int     get_some_more,i;
  u8     *brec_buf;
  u16   brec_checksum = 0;
  int 	   brec_sector_nums = 0;
  int     brec_block_size;
  int 	   brec_block_num = 0; 
  int 	   rc = 0;

  //DBG_ADFU("%s:%d\n", __FUNCTION__, __LINE__);
 
  amount_left_to_req = amount_left_to_write = fsg->data_size_from_cmnd;
  get_some_more = 1;
  
  while (amount_left_to_write > 0) {  
	bh = fsg->next_buffhd_to_fill;
	if (bh->state == BUF_STATE_EMPTY && get_some_more ) {
		amount = get_buffer_len();
		amount = min(amount_left_to_req,amount);
		if (amount == 0) {
			get_some_more = 0;
			DBG_ADFU("%s odd status\n",__FUNCTION__);
			continue;
		}

		/* Get the next buffer */
		fsg->usb_amount_left -= amount ;
		amount_left_to_req -= amount;

		if (amount_left_to_req == 0)
			get_some_more = 0;

		/* amount is always divisible by 512, hence by
		* the bulk-out maxpacket size */
		bh->outreq->length = bh->bulk_out_intended_length
			= amount;
		
		start_transfer(fsg, fsg->bulk_out, 
			bh->outreq,&bh->outreq_busy, &bh->state);

		fsg->next_buffhd_to_fill = bh->next;
		continue;
	}

	bh = fsg->next_buffhd_to_drain;
	if (bh->state == BUF_STATE_EMPTY 
			&& !get_some_more)
		return -EIO;      // We stopped early
	if (bh->state == BUF_STATE_FULL) {
		fsg->next_buffhd_to_drain = bh->next;
		bh->state = BUF_STATE_EMPTY;
		
		/* Did something go wrong with the transfer? */
		if (bh->outreq->status != 0) {
			DBG_ADFU("ADFU: communication failure\n");
			fsg->adfu_sense = SS_COMMUNICATION_FAILURE;
			return -EIO;
		}
		
		amount = bh->outreq->actual;
		brec_buf = bh->buf;
		brec_block_size = fsg->brec_block_size;
		
		/* Perform the write */
		for (i = 0; i < (int)(amount>>9); i++) {
			brec_sector_write(brec_block_num+4,brec_sector_nums,brec_buf);
			if(brec_sector_nums == (brec_block_size -1))
				break;
			brec_sector_nums++;
			brec_buf += 512;
		}
		
		amount_left_to_write -= amount;
		fsg->residue -= bh->outreq->actual ;

		if(brec_sector_nums == (brec_block_size - 1)){
			brec_checksum = get_le16(brec_buf+510);
			if(verify_brec_checksum(fsg,brec_block_num,brec_checksum)){
				DBG_ADFU("verify brec %d failed\n",brec_block_num);
				fsg->adfu_sense = SS_COMMUNICATION_FAILURE;
				return -EIO;
			}
			
			brec_sector_nums=0;
			brec_block_num++;	
		}

		/* Did the host decide to stop early? */
		if (bh->outreq->actual != bh->outreq->length) {
			fsg->short_packet_received = 1;
			return -EIO;
		}
		continue;
	}

	/* Wait for something to happen */
	rc = sleep_thread(fsg);
	if (rc)
		return rc;
  }
  return -EIO;
}


//0xb0 0x00
static int do_adfu_update_ok(struct fsg_dev *fsg)
{
	if(fsg->adfu_buf ){
		kfree(fsg->adfu_buf);
		fsg->adfu_buf = NULL;
	}
	/*update flash read/write*/
	nand_logical_update();

	spin_lock_irq(&fsg->lock);
	fsg->adfu_update    = 1;
	fsg->state = FSG_STATE_TERMINATED;
	spin_unlock_irq(&fsg->lock);
	/*watch_dog reset*/
	return 0;
}

/************************************************************/
//0xb0...0x09
static int do_adfu_get_wrlba(struct fsg_dev *fsg,struct fsg_buffhd *bh)
{
	DBG_ADFU("%s:%d\n",__FUNCTION__,__LINE__);
	fsg->usb_amount_left -= fsg->data_size_from_cmnd;
	bh->outreq->length = 
		bh->bulk_out_intended_length = fsg->data_size_from_cmnd;

	start_transfer(fsg, fsg->bulk_out, bh->outreq,
	  &bh->outreq_busy, &bh->state);
	
	//wait for the coming of data
	while(bh->state != BUF_STATE_FULL);
	
	fsg->lfi_sector_num = *(u32 *)bh->buf;
	bh->state = BUF_STATE_EMPTY;
	
	fsg->residue  -=  fsg->data_size_from_cmnd;
	DBG_ADFU("lfi_sector_num=0x%x\n",fsg->lfi_sector_num );
	
	return -EIO;
}

/************************************************************/
//0xb0...0x11
#if EEPROM_TYPE==1
static int do_adfu_mac_wriis(struct fsg_dev *fsg,struct fsg_buffhd *bh)
{
	unsigned char *macdata;
	mac_address_t mac_val;
	DBG_ADFU("%s:%d\n",__FUNCTION__,__LINE__);
	fsg->usb_amount_left -= fsg->data_size_from_cmnd;
	bh->outreq->length = 
		bh->bulk_out_intended_length = fsg->data_size_from_cmnd;

	start_transfer(fsg, fsg->bulk_out, bh->outreq,
	  &bh->outreq_busy, &bh->state);
	
	//wait for the coming of data
	while(bh->state != BUF_STATE_FULL);
	
	//call write mac to iis EEPROM ,mac data is (char *)bh->buf;
	//IIS_EEPROM_Write((char *)bh->buf,size);
	macdata=(unsigned char *)bh->buf;
	mac_val.mac_addr_0 = macdata[0];
	mac_val.mac_addr_1 = macdata[1];
	mac_val.mac_addr_2 = macdata[2];
	mac_val.mac_addr_3 = macdata[3];
	mac_val.mac_addr_4 = macdata[4];
	mac_val.mac_addr_5 = macdata[5];
	i2c_write_mac_addr(&mac_val);	
	bh->state = BUF_STATE_EMPTY;
	
	fsg->residue  -=  fsg->data_size_from_cmnd;
	DBG_ADFU("write mac to iis eeprom:%02x:%02x:%2x:%02x:%02x:%02x\n",
		macdata[0],macdata[1],macdata[2],macdata[3],macdata[4],macdata[5]);
	
	return -EIO;
}
/************************************************************/
//0xb0...0x12
static int do_adfu_mac_rdiis(struct fsg_dev *fsg,struct fsg_buffhd *bh)
{
	u8    *buf = (u8 *)bh->buf;
	unsigned char temp[8];
	mac_address_t mac_val;
	DBG_ADFU("%s:%d\n",__FUNCTION__,__LINE__);
	
	memset(temp,0,sizeof(temp));

	//read iis EEPROM
	//IIS_EEPROM_Read(temp,size);
/*
	temp[0]=0x2e; 
	temp[1]=0x35;
	temp[2]=0x78;
	temp[3]=0x64;
	temp[4]=0x2c;
	temp[5]=0x9d;
*/
	if(0 == i2c_read_mac_addr(&mac_val)){
	temp[0]=mac_val.mac_addr_0; 
	temp[1]=mac_val.mac_addr_1;
	temp[2]=mac_val.mac_addr_2;
	temp[3]=mac_val.mac_addr_3;
	temp[4]=mac_val.mac_addr_4;
	temp[5]=mac_val.mac_addr_5;
	}	
	
	memcpy(buf,temp,fsg->data_size_from_cmnd);

//	DBG_ADFU("read mac from iis eeprom:%s\n",(char *)bh->buf );

	return fsg->data_size_from_cmnd;
}
#endif
/************************************************************/
//0xb0...0x13
#if EEPROM_TYPE==2

static int do_adfu_mac_wrspi(struct fsg_dev *fsg,struct fsg_buffhd *bh)
{
	unsigned char temp[8];
	mac_address_t mac_val;
	unsigned char *macdata;
	DBG_ADFU("%s:%d\n",__FUNCTION__,__LINE__);
	fsg->usb_amount_left -= fsg->data_size_from_cmnd;
	bh->outreq->length = 
		bh->bulk_out_intended_length = fsg->data_size_from_cmnd;

	start_transfer(fsg, fsg->bulk_out, bh->outreq,
	  &bh->outreq_busy, &bh->state);
	
	//wait for the coming of data
	while(bh->state != BUF_STATE_FULL);
	macdata=(unsigned char *)bh->buf;
	mac_val.mac_addr_0 = macdata[0];
	mac_val.mac_addr_1 = macdata[1];
	mac_val.mac_addr_2 = macdata[2];
	mac_val.mac_addr_3 = macdata[3];
	mac_val.mac_addr_4 = macdata[4];
	mac_val.mac_addr_5 = macdata[5];
	spi_write_mac_addr(&mac_val);
	//call write mac to spi EEPROM ,mac data is (char *)bh->buf;
	//SPI_EEPROM_Write((char *)bh->buf,size);
	macdata=(unsigned char *)bh->buf;
	
	bh->state = BUF_STATE_EMPTY;
	
	fsg->residue  -=  fsg->data_size_from_cmnd;
	DBG_ADFU("write mac to spi eeprom:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
		macdata[0],macdata[1],macdata[2],macdata[3],macdata[4],macdata[5]);
	
	return -EIO;
}
/************************************************************/
//0xb0...0x14
static int do_adfu_mac_rdspi(struct fsg_dev *fsg,struct fsg_buffhd *bh)
{
	u8    *buf = (u8 *)bh->buf;
//	unsigned char temp[32];
	unsigned char temp[8];
	mac_address_t mac_val;
	DBG_ADFU("%s:%d\n",__FUNCTION__,__LINE__);
	
	memset(temp,0,sizeof(temp));
/*
	temp[0]=0x2e; 
	temp[1]=0x35;
	temp[2]=0x78;
	temp[3]=0x64;
	temp[4]=0x2c;
	temp[5]=0x9d;
	*/
	if(0 == spi_read_mac_addr(&mac_val)){
		temp[0]=mac_val.mac_addr_0; 
		temp[1]=mac_val.mac_addr_1;
		temp[2]=mac_val.mac_addr_2;
		temp[3]=mac_val.mac_addr_3;
		temp[4]=mac_val.mac_addr_4;
		temp[5]=mac_val.mac_addr_5;
	}		
	memcpy(buf,temp,fsg->data_size_from_cmnd);

//	DBG_ADFU("read mac from spi eeprom:%s\n",(char *)bh->buf );

	return fsg->data_size_from_cmnd;
}
#endif
/************************************************************/
//0xb0...0x10
#if EEPROM_TYPE !=0
static int do_adfu_mac_ok(struct fsg_dev *fsg)
{
	DBG_ADFU("%s:%d\n",__FUNCTION__,__LINE__);

	if(fsg->adfu_buf ){
		kfree(fsg->adfu_buf);
		fsg->adfu_buf = NULL;
	}
/*
	spin_lock_irq(&fsg->lock);
	fsg->adfu_update    = 1;
	fsg->state = FSG_STATE_TERMINATED;
	spin_unlock_irq(&fsg->lock);
	*watch_dog reset
*/
	return 0;
}
#endif

static void compare_cap_info(const CapInfo_t *new_capinfo, const CapInfo_t *old_capinfo)
{
	u32  new_cap_sum,old_cap_sum;
	new_cap_sum = new_capinfo->Scodecap + new_capinfo->Scodebakcap + new_capinfo->Appinfocap;
	old_cap_sum = old_capinfo->Scodecap + old_capinfo->Scodebakcap + old_capinfo->Appinfocap;

	DBG_ADFU("Scodecap 0x%x\n",new_capinfo->Scodecap);
	DBG_ADFU("Scodecap 0x%x\n",new_capinfo->Scodebakcap);
	DBG_ADFU("Scodecap 0x%x\n",new_capinfo->Appinfocap);
	DBG_ADFU("Scodecap 0x%x\n",new_capinfo->Liccap);
	DBG_ADFU("Scodecap 0x%x\n",new_capinfo->Micap);
	DBG_ADFU("Scodecap 0x%x\n",new_capinfo->Dspcap);
}


//0xb0...0x08
#ifdef USE_METHOD_ONE
static int do_adfu_write_fwdata(struct fsg_dev *fsg)
{
	struct fsg_buffhd *bh;
	u32    amount, amount_left_to_write;
	u8      *lfi_buf;
	u32 	  num_sectors;
	int      ret =-1;

//	DBG_ADFU("%s:%d\n", __FUNCTION__, __LINE__);
	
	amount_left_to_write = fsg->data_size_from_cmnd ;
	while (amount_left_to_write > 0) {		
		bh = fsg->next_buffhd_to_fill;	
		while(bh->state != BUF_STATE_EMPTY) {
			ret = sleep_thread(fsg);
			if(ret)
				return -EIO;
		}
		
		amount = get_buffer_len();
		amount = min(amount_left_to_write, amount);
		
		#if 0
		amount_left_to_req -= amount;	
		if (amount_left_to_req == 0){
			get_some_more = 0;
		}
		#endif
	
		bh->outreq->length = 
			bh->bulk_out_intended_length = amount ;
		
		start_transfer(fsg, fsg->bulk_out, bh->outreq,
			&bh->outreq_busy, &bh->state);
	
		while (bh->state != BUF_STATE_FULL){
			ret = sleep_thread(fsg);
			if(ret)
				return -EIO;
		}	
		
		/* Did something go wrong with the transfer? */
		if (bh->outreq->status != 0) {
			DBG_ADFU("ADFU: communication failure\n");
			fsg->adfu_sense = SS_COMMUNICATION_FAILURE;
			return -EIO;
		}

		/* Did the host decide to stop early? */
		if (bh->outreq->actual != bh->outreq->length) {
			fsg->short_packet_received = 1;
			DBG_ADFU("ADFU: receive short packet\n");
			return -EIO;
		}

		amount = bh->outreq->actual;
		lfi_buf = bh->outreq->buf;	
		num_sectors = amount>>9;
		
//		DBG_ADFU("lfi_sector_num 0x%x,num_sectors:%d\n",fsg->lfi_sector_num,num_sectors);
		if(fsg->lfi_sector_num == 0)
			compare_cap_info(&((LFIHead_t*)lfi_buf)->CapInfo,&lfi_header.CapInfo);

		if(get_be32(lfi_buf) == 0x55534243){
			DBG_ADFU("++++++unbelievable ,carzy world\n");
		       print_hex_dump(KERN_ALERT, "write data: ", DUMP_PREFIX_OFFSET,
					16, 1, lfi_buf, 31, 1);
		}
		
		ret = nand_adfu_write(fsg->lfi_sector_num,lfi_buf,num_sectors);
		if (ret) {
			DBG_ADFU("######WRITE ERR###########\n");
			return -EIO;
		}
	
		ret = nand_adfu_read(fsg->lfi_sector_num,fsg->adfu_buf,num_sectors);
		if(ret){
			DBG_ADFU("######READ ERR###########\n");
			return -EIO;
		}

		ret = memcmp_l((void *)lfi_buf,(void *)fsg->adfu_buf, amount);
		if(ret){
			DBG_ADFU("######COMPARE ERR###########\n");
			return -EIO;
		}

		/* Get the next buffer */
		fsg->usb_amount_left -= amount;
		fsg->lfi_sector_num += num_sectors;
		amount_left_to_write -= amount;
		fsg->residue -= amount;
		
		bh->state = BUF_STATE_EMPTY;
		fsg->next_buffhd_to_fill= bh->next;
		continue;
	}		
	return -EIO;
}
#else

void dump_read_data(const unsigned char *pdata,unsigned int len){
	unsigned int tmp=0;
	while(len--){
		if(tmp%16 == 0){
			printk("\n%02x",*(pdata+tmp));
		}else{
			printk(" %02x",*(pdata+tmp));	
		}
		tmp++;
	}
	printk("\n");
}

static int do_adfu_write_fwdata(struct fsg_dev *fsg)
{
	struct fsg_buffhd *bh;
	  u32	  amount,amount_left_to_req, amount_left_to_write;
	  u32 	  amount_sectors;
	  int	  get_some_more;
	  u8	  *lfi_buf;
	  int ret = -1;
//	DBG_ADFU("%s adfu write:lba=0x%x\n",__func__,fsg->lfi_read_sector_num);	  
	  amount_left_to_req = amount_left_to_write =
	  		fsg->data_size_from_cmnd; 
	  get_some_more = 1;
	
	  while (amount_left_to_write > 0) {  
		bh = fsg->next_buffhd_to_fill;
		if (bh->state == BUF_STATE_EMPTY && get_some_more ) {
			amount = get_buffer_len();
			amount = min(amount_left_to_req, amount);

			if (amount == 0) {
				get_some_more = 0;
				DBG_ADFU("ADFU, odd status\n");
				continue;
			}

			/* Get the next buffer */
			fsg->usb_amount_left -= amount ;
			amount_left_to_req -= amount;
			
			if (amount_left_to_req == 0){
				get_some_more = 0;
			}

			/* amount is always divisible by 512, hence by
			* the bulk-out maxpacket size */
			bh->outreq->length = 
				bh->bulk_out_intended_length = amount ;	

			start_transfer(fsg, fsg->bulk_out, bh->outreq,
					&bh->outreq_busy, &bh->state);
			
			fsg->next_buffhd_to_fill = bh->next;
			continue;
	       }
		
		bh = fsg->next_buffhd_to_drain;
		if (bh->state == BUF_STATE_EMPTY && !get_some_more){
		 	return -EIO;		// We stopped early
		}
		
		if (bh->state == BUF_STATE_FULL) {
			fsg->next_buffhd_to_drain = bh->next;
			bh->state = BUF_STATE_EMPTY;
			/* Did something go wrong with the transfer? */
			if (bh->outreq->status != 0) {
				DBG_ADFU("ADFU: communication failure\n");
				fsg->adfu_sense = SS_COMMUNICATION_FAILURE;
				return -EIO;
			}

			amount = bh->outreq->actual;
			amount_sectors = amount>>9;
			lfi_buf = bh->buf;

			if(fsg->lfi_sector_num == 0){
				compare_cap_info(&((LFIHead_t*)lfi_buf)->CapInfo,&lfi_header.CapInfo);
			}
			
			//DBG_ADFU("%s:write %d k\n", __FUNCTION__, amount_sectors>>1);
			//ret=NAND_SECTOR_WRITE(lfi_sector_num,lfi_buf,0,0);
			ret = nand_adfu_write(fsg->lfi_sector_num,lfi_buf,amount_sectors);
			if (ret!=0){
				DBG_ADFU("######WRITE ERR###########\n");
				return ret;
			}
			fsg->lfi_sector_num+=amount_sectors;
			amount_left_to_write -= amount;
			fsg->residue -= bh->outreq->actual ;

			/* Did the host decide to stop early? */
			if (bh->outreq->actual != bh->outreq->length){
				fsg->short_packet_received = 1;
				return -EIO;
			}
			continue;
		}
	
		/* Wait for something to happen */
		ret = sleep_thread(fsg);
		if(ret)
			return -EIO;
	  }
	  return -EIO;

}
#endif


//0xb0...0x0a
static int do_adfu_read_fwdata(struct fsg_dev *fsg)
{
	struct fsg_buffhd	*bh;
	int			rc;
	u32			amount_left;
	unsigned int	amount;/*sectors*/
	unsigned int 	amount_sectors;	
	int ret;

//	DBG_ADFU("%s:%d lba=0x%x\n",__FUNCTION__,__LINE__,fsg->lfi_read_sector_num);
	amount_left = fsg->data_size_from_cmnd;/*convert to sector*/
	if (amount_left == 0) {
		DBG_ADFU("%s:%d zero req\n",__FUNCTION__, __LINE__);
		return -EIO;		// No default reply
	}

	while(amount_left > 0) {	
		amount = get_buffer_len();
		amount = min(amount_left, amount);
		
		/* Wait for the next buffer to become available */
		bh = fsg->next_buffhd_to_fill;
		while (bh->state != BUF_STATE_EMPTY) {
			rc = sleep_thread(fsg);
			if (rc)
				return rc;
		}
		
		/* If we were asked to read past the end of file,
		 * end with an empty buffer. */
		if (amount == 0) {
			bh->inreq->length = 0;
			break;
		}
		
		/* Perform the read */
		amount_sectors = amount >>9;
	//	DBG_ADFU("lfi_sector_num 0x%x,sectors_num:%d\n",fsg->lfi_read_sector_num,amount_sectors);

		memset(bh->buf,0,get_buffer_len());	
		ret = nand_adfu_read(fsg->lfi_read_sector_num,bh->buf,amount_sectors);
		if((0x674000+0x00)==fsg->lfi_read_sector_num){
			DBG_ADFU("nand read lba=0x%x",fsg->lfi_read_sector_num);
			dump_read_data(bh->buf,512);	
		}
		if (ret!=0) {
			DBG_ADFU("%s adfu read failed:lba=%d\n",
				__func__,
				fsg->lfi_read_sector_num);
			return -EIO;
	  	}
		
	       fsg->lfi_read_sector_num += amount_sectors;
		bh->inreq->length = amount ;
		bh->state = BUF_STATE_FULL;

		/* Send this buffer and go read some more */
		bh->inreq->zero = 0;
		start_transfer(fsg, fsg->bulk_in,
				bh->inreq,
				&bh->inreq_busy, 
				&bh->state);

		fsg->residue -= amount;
		amount_left   -= amount;
		
		fsg->next_buffhd_to_fill = bh->next;
	}
	
	fsg->data_size = 0;
	return -EIO;		// No default reply
}


//0xb0...0x0b
static int do_adfu_get_rdlba(struct fsg_dev *fsg,struct fsg_buffhd *bh)
{
	int result;
	
//	DBG_ADFU("%s:%d lba=0x%x\n",__FUNCTION__,__LINE__,fsg->lfi_read_sector_num);
	fsg->usb_amount_left -= fsg->data_size_from_cmnd;
	bh->outreq->length = bh->bulk_out_intended_length = fsg->data_size_from_cmnd;

	start_transfer(fsg, fsg->bulk_out, bh->outreq,
		&bh->outreq_busy, &bh->state);
	
	while(bh->state != BUF_STATE_FULL){
		result = sleep_thread(fsg);
		if (result)
			return result;
	}
	
	fsg->residue -= fsg->data_size_from_cmnd;	
	fsg->lfi_read_sector_num =*(u32 *)bh->buf;
	bh->state = BUF_STATE_EMPTY;

	DBG_ADFU("lfi_read_sector_num=0x%x\n",fsg->lfi_read_sector_num);
	return -EIO;
}


int do_adfu_send_data(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
	int sub_cmd = get_le16(&fsg->cmnd[9]);
	int result;
	
	//DBG_ADFU("%s:sub_cmd%x\n",__FUNCTION__,sub_cmd);
	fsg->data_size_from_cmnd = 0;
	switch(sub_cmd){
	case ADFU_DATA_TYPE_SENDOK:
		result = do_adfu_update_ok(fsg);
		break;
		
	case ADFU_DATA_TYPE_MBREC:
		fsg->data_size_from_cmnd = get_le16(&fsg->cmnd[7])<< 9;
		if((result = check_adfu_command(fsg,
				DATA_DIR_FROM_HOST,
				"ADFU_UPDATA_MBREC")) !=0)
			return result;
		
		result = do_adfu_update_mbrec(fsg,bh);
		break;
		
	case ADFU_DATA_TYPE_BREC:
		fsg->data_size_from_cmnd = get_le16(&fsg->cmnd[7])<< 9;
		if((result = check_adfu_command(fsg,
				DATA_DIR_FROM_HOST,
				"ADFU_UPDATA_BREC")) !=0)
			break;
		result = do_adfu_update_brec(fsg);
		break;
		
	case ADFU_DATA_TYPE_WRDATA:
		fsg->data_size_from_cmnd = get_le16(&fsg->cmnd[7])<< 9;
		if((result = check_adfu_command(fsg,
					DATA_DIR_FROM_HOST,
					"ADFU_UPDATA_WDATA")) !=0)
				break;

		result = do_adfu_write_fwdata(fsg);
		break;
		
	case ADFU_DATA_TYPE_WRLBA:
		fsg->data_size_from_cmnd = get_le16(&fsg->cmnd[7]);
		if((result = check_adfu_command(fsg,
				DATA_DIR_FROM_HOST,
				"ADFU_UPDATA_WLBA")) !=0)
			break;
		result = do_adfu_get_wrlba(fsg,bh);
		break;
		
	case ADFU_DATA_TYPE_RDDATA:
		fsg->data_size_from_cmnd = get_le16(&fsg->cmnd[7])<< 9;
		if((result = check_adfu_command(fsg,
				DATA_DIR_TO_HOST,
				"ADFU_UPDATA_RDFW")) !=0){
			DBG_ADFU("CHECK RD DATA failed\n");
			break;
		}		
		result = do_adfu_read_fwdata(fsg);
		break;
		
	case ADFU_DATA_TYPE_RDLBA:
		fsg->data_size_from_cmnd = get_le16(&fsg->cmnd[7]);
		if((result = check_adfu_command(fsg,
				DATA_DIR_FROM_HOST,
				"ADFU_UPDATA_RDLBA")) !=0)
			break;
		result = do_adfu_get_rdlba(fsg,bh);
		break;

	case ADFU_DATA_MAC_SENDOK:
#if EEPROM_TYPE!=0
		result = do_adfu_mac_ok(fsg);
#endif
		break;


	case ADFU_DATA_MAC_WRIIS:
		fsg->data_size_from_cmnd = get_le16(&fsg->cmnd[7]);
		if((result = check_adfu_command(fsg,
				DATA_DIR_FROM_HOST,
				"ADFU_UPDATA_WRIIS")) !=0)
			break;
#if EEPROM_TYPE==1
		result = do_adfu_mac_wriis(fsg,bh);
#endif
		break;

	case ADFU_DATA_MAC_RDIIS:
		fsg->data_size_from_cmnd = get_le16(&fsg->cmnd[7]);
		if((result = check_adfu_command(fsg,
				DATA_DIR_TO_HOST,
				"ADFU_UPDATA_RDIIS")) !=0)
			break;
#if EEPROM_TYPE==1
		result = do_adfu_mac_rdiis(fsg,bh);
#endif
		break;
	case ADFU_DATA_MAC_WRSPI:
		fsg->data_size_from_cmnd = get_le16(&fsg->cmnd[7]);
		if((result = check_adfu_command(fsg,
				DATA_DIR_FROM_HOST,
				"ADFU_UPDATA_WRSPI")) !=0)
			break;
#if EEPROM_TYPE==2
		result = do_adfu_mac_wrspi(fsg,bh);
#endif
		break;
	case ADFU_DATA_MAC_RDSPI:
		fsg->data_size_from_cmnd = get_le16(&fsg->cmnd[7]);
		if((result = check_adfu_command(fsg,
				DATA_DIR_TO_HOST,
				"ADFU_UPDATA_RDSPI")) !=0)
			break;
#if EEPROM_TYPE==2
		result = do_adfu_mac_rdspi(fsg,bh);
#endif
		break;
	
	default:
		result = -EIO;
		break;
	}	
	return result;
}
