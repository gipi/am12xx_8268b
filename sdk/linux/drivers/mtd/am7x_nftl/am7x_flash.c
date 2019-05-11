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


#define TEST_FLASH_SPEED   (0)
#define NEW_BLOCK_MODE   (0x01)
#define ADD_UPDATE_MODE   (0x01)

#define TRANS_AM_RC(am_rc,linux_rc)  (am_rc)==TRUE ? 0 : (linux_rc)
#define NOP()                        do {} while (0)
#define NR_LFI_SECTS  (0x800000*4 / 512)
#define AM7X_MTD_DEVICES       1
#define FW_BACK_PAR_NAME  "reserve"


#if 0
#define whoami()       printf("\n\n===call:%s,%d ===\n\n", __func__,__LINE__)
#else
#define whoami()       NOP()
#endif

#if 0
#define  DEBUG_MSG(fmt,msg...)  printk(fmt,##msg)
#else
#define  DEBUG_MSG(fmt,msg...)  NOP()
#endif

#define WRITE_ENABLE   0
#define WRITE_DISABLE  1

static u32 sectors;
static u32 cylinders;
static u32 heads;
static u32 write_flash_flag=WRITE_ENABLE;
static u32 kernelcap;
INT8U *CmpBuf;
#define MAX__PARTS   16 
#define NAME_SIZE_MAX  10
#define PART_Str_SIZE   128 
struct Flash_Partiton {
    char      partName[NAME_SIZE_MAX];
    uint32_t part_off;   /* LBA of first sector in the partition */
    uint32_t part_size;  /* number of blocks in partition */
}__attribute__ ((packed));//,aligned(4)

struct Flash_Partiton  Fparts[MAX__PARTS];
#define WRITE_FLASH   0x01
//@fish 200---> 100 目的在系统中断电保护
#define WAIT_ILDE_TIME   50

struct am7x_flash_host{
        struct timer_list timer;
        struct task_struct *thread;
        INT8U WriteFlashFlag;   
        struct mutex lock;
	struct semaphore flash_sem_op; 
        
};
struct am7x_flash_host   Flash_Host;
static INT32S flash_read_sectors(INT32U offset, INT8U *buffer, INT32U sectors,INT8U *Part );
static INT32S flash_write_sectors(INT32U offset, INT8U *buffer, INT32U sectors,INT8U *Part );
static INT32S flash_getcapacity(INT8U *Part);

static INT32S  nand_adfu_read(INT32U Lba, INT8U *Buf, INT32U Length);
static INT32S  nand_adfu_write(INT32U Lba, INT8U *Buf, INT32U Length);
/*
*
*/
static int nand_updata(void)
{
    struct timeval tpstart,tpend; 
    INT32U Timetmp;
    int retry=0;

    if(Flash_Host.WriteFlashFlag == 0)
        return -1;
RETRY:
    if(down_timeout(&Flash_Host.flash_sem_op,2000) == -ETIME)//added by jjf 
    {
        retry++;
        printk("Get Flash_Host.flash_sem_op Fail :%d\n",retry);
        if(retry<3)
            goto RETRY;
        return  -ETIME;
    }

    do_gettimeofday(&tpstart); 
    am_get_multi_res(FLASH_CARD);

    FTL_FlushPageCache();
    BMM_WriteBackAllMapTbl();
    FTL_Param_Init();

    am_release_multi_res(FLASH_CARD);

    up(&Flash_Host.flash_sem_op);//added by jjf 

    
    do_gettimeofday(&tpend); 	
    Timetmp  = 1000000*(tpend.tv_sec-tpstart.tv_sec) + tpend.tv_usec-tpstart.tv_usec; 
    Timetmp /= 1000;
    printk("###time:%d ms####\n",Timetmp);
    
    return 0x01;
}

/*
*
*/
INT32U TimeCntTT =0x00;
static void Flash_poll_event(unsigned long arg) 
{
    struct timeval tpstart; 
    INT32U Timetmp;
    do_gettimeofday(&tpstart); 

    if(Flash_Host.WriteFlashFlag)
    {
        wake_up_process(Flash_Host.thread);
    }

    Timetmp=1000000*(tpstart.tv_sec) + tpstart.tv_usec;     
    //  printk("time:%d  %d \n", (Timetmp/1000-TimeCntTT),Flash_Host.WriteFlashFlag);
    TimeCntTT =Timetmp/1000;
    mod_timer(&Flash_Host.timer,jiffies+WAIT_ILDE_TIME);	
}
/*
*
*/
int Flash_Update_thread(void)
{
    while(!kthread_should_stop())
    {           
        if((Flash_Host.WriteFlashFlag))
        {
            nand_updata();    
            Flash_Host.WriteFlashFlag =0x00;
            // printk("run to :%s %d\n",__func__,__LINE__);
         }
        set_current_state(TASK_INTERRUPTIBLE);
        schedule();
    }
    return 0x00;
}
/*
*
*/
static struct semaphore flash_sem; 
extern INT32U NandDMAChannel;

#if DMA_IRQ_MODE ==0x01
INT32U nand_StartDma(INT32U nDmaNum)
{
	am_set_dma_irq(nDmaNum,1,0);
	am_dma_start(nDmaNum,0);	
}
/*
*
*/
void Nand_ReadConfig(INT32U nDmaNum, void* Mem_Addr, INT32U ByteCount)
{
	INT32U mode;
	dma_cache_wback_inv((INT32U)((INT32U)Mem_Addr & ~MASK_COST_Read), ByteCount); 
	am_dma_cmd(nDmaNum,DMA_CMD_RESET);
	am_set_dma_addr(nDmaNum,(NAND_IOMAP_PHY_BASE + NAND_DATA),(INT32U)Mem_Addr&0x1fffffff); 	
	if ((INT32U)Mem_Addr % 4 == 0)
	{
		mode = 0x008401a4;
	}
	else
	{
		mode = 0x008001a4;
	}
	am_dma_config(nDmaNum,mode,ByteCount);    

}
/*
*
*/
INT32S _dma_start(INT32U dmanr, INT32U cmd)
{
	am_dma_start(dmanr,cmd);	
	return 0x00;
}
/*
*
*/
irqreturn_t Isr_DMA_Flashdriver(int isr,void *id)
{
	//printk("Start of ### %s  %d ###\n",__func__,__LINE__);
    if(am_get_dma_irq(NandDMAChannel,1,0)==1)
    {
        //printk("endif of ### %s  %d ###\n",__func__,__LINE__);
        am_clear_dma_irq(NandDMAChannel,1,0);    
        if(Flash_DMA_Mode ==FLASH_DMA_IRQ)
            up(&flash_sem);	
    }  
    return IRQ_HANDLED;
}
/*
*
*/
INT32U Flash_Dmatimeout(void)
{
	INT32U Ret;
	int reg2;
	Ret =0x00;	
	if(Flash_DMA_Mode ==FLASH_DMA_IRQ)
	{	
		
		reg2 = down_timeout(&flash_sem,100);
		if(reg2!=0)
		{
			printk("### %s,%d Time out ###\n",__func__,__LINE__);
			Print_msg();
			Ret =0x01;
		}
		//printk("Start of ### %s  %d ###\n",__func__,__LINE__);
	}
	return Ret;
}
#endif
/*
*
*/
INT8U  flash_irq_init(void)
{

	INT32U ret;
#if EIP_IRQ_MODE ==0x01	
	ret = request_irq(IRQ_NAND, Isr_Flashdriver, 0,  "nand Flash", "Flash");
#endif
	sema_init(&flash_sem,0);
	return 0x00;
}
/*
*
*/
void flash_irq_enable(void)
{
	if(Flash_DMA_Mode !=FLAH_EIP_IRQ)
		return ;
	act_writel(act_readl(INTC_MSK) | (1<<IRQ_NAND), INTC_MSK);
	///act_writel(act_readl(NAND_STATUS) | (1<<0), NAND_STATUS);	
	//NAND_REG_WRITE(NAND_STATUS,NAND_REG_READ(NAND_STATUS)|(1<<0));
	NAND_REG_WRITE(NAND_CTL,NAND_REG_READ(NAND_CTL)|(1<<20));
	IRQ_CLEAR_PENDING(NAND_REG_READ(NAND_STATUS)|(3<<0),__AM7X_HWREG(0xB00a0000+0x04));
		
	//printf("NAND_STATUS:%x\n",NAND_REG_READ(NAND_STATUS) );
	//printf("NAND_CTL:%x\n",NAND_REG_READ(NAND_CTL) );
	//printf("INTC_MSK:0x%x\n",act_readl(INTC_MSK));
}
/*
*
*/
void flash_irq_disable(void)
{
	act_writel(act_readl(INTC_MSK)&~(1<<IRQ_NAND), INTC_MSK);
	IRQ_CLEAR_PENDING(NAND_REG_READ(NAND_STATUS)|(3<<0),__AM7X_HWREG(0xB00a0000+0x04));
	NAND_REG_WRITE(NAND_CTL,NAND_REG_READ(NAND_CTL)&(~(1<<20)));
}
/*
*
*/
void flash_clr_status()
{
	IRQ_CLEAR_PENDING(NAND_REG_READ(NAND_STATUS)|(0x3<<0),__AM7X_HWREG(0xB00a0000+0x04));
		
}


#if EIP_IRQ_MODE ==0x01
/*
*
*/
irqreturn_t Isr_Flashdriver(int isr,void *id)
{
 //   printk("####NAND_STATUS:%x#########\n",NAND_REG_READ(NAND_STATUS) );
    INT32U PageReg;
	if(NAND_REG_READ(NAND_STATUS)&0x01)
	{		
		//Print_msg();
		IRQ_CLEAR_PENDING(NAND_REG_READ(NAND_STATUS)|(1<<0),__AM7X_HWREG(0xB00a0000+0x04));
		PageReg = NAND_REG_READ(NAND_PAGE_CTL);
		if(Flash_DMA_Mode ==FLAH_EIP_IRQ &&(PageReg&0x03) ==0x00) 
		{
			up(&flash_sem);
			///printk("####2 status:0x%x\n",NAND_REG_READ(NAND_STATUS) );
			//printk("x2\n");
		}
		
	}        
        return IRQ_HANDLED;
}
/*
*
*/
INT32U Flash_timeout(void)
{
	INT32U Ret;
	int reg2;
	Ret =0x00;
	
	if(Flash_DMA_Mode ==FLAH_EIP_IRQ)
	{
		//printk("Start of ### %s ###\n",__func__);
		reg2 = down_timeout(&flash_sem,100);
		if(reg2!=0)
		{
			printk("### %s,%d Time out ###\n",__func__,__LINE__);
			Print_msg();
			Ret =0x01;
		}
		//printk("end of ### %s ###\n",__func__);
		//printk("reg:%x\n",reg2);
		//Print_msg();
		//printk("### %s ###\n",__func__);
	}
	return Ret;
}
#endif
/*
*
*/
static u32 nand_get_cap(void)
{

    u32 physical_whole_cap, logical_whole_cap;
    physical_whole_cap = NandStorageInfo.ChipCnt * NandStorageInfo.DieCntPerChip
                    * NandStorageInfo.TotalBlkNumPerDie * NandStorageInfo.PageNumPerPhyBlk
                    * NandStorageInfo.SectorNumPerPage;
                    
    logical_whole_cap = (physical_whole_cap / 1024) * LogicOrganisePara.DataBlkNumPerZone;	
    logical_whole_cap -= kernelcap;
    printk("==nand_get_cap:%x,%dMB==\n",logical_whole_cap,logical_whole_cap/2048);
    return logical_whole_cap;

}
/*
*
*/
static inline unsigned long translate_block(unsigned long block)
{
	return block + kernelcap;
}
/*
*
*/
INT32U bIrqFlag;
void Flash_Disable_Irq(void)
{
	//local_irq_save(bIrqFlag);
}
void Flash_Enable_Irq(void)
{
	//local_irq_restore(bIrqFlag);
}


#if NEW_BLOCK_MODE  ==0x00
/*
*
*/
static void am7x_nftl_add_mtd(struct mtd_blktrans_ops *tr, struct mtd_info *mtd)
{
	struct mtd_blktrans_dev *new_dev;
	int result;

	whoami();
	new_dev = kmalloc(sizeof(*new_dev), GFP_KERNEL);
	if (!new_dev) {
		printk(KERN_WARNING "out of memory for data structures\n");
		return;
	}
	memset(new_dev, 0, sizeof(*new_dev));
	new_dev->mtd = mtd;
	new_dev->devnum = -1;
	new_dev->tr = tr;
	new_dev->size = sectors*cylinders*heads;
	result = add_mtd_blktrans_dev(new_dev);
	if(result < 0) {
		printk(KERN_WARNING "Unable to add mtd blktrans dev, ret=%d\n", result);
		return;
	}
}
/*
*
*/
static void nftl_remove_dev(struct mtd_blktrans_dev *dev)
{
	whoami();
	del_mtd_blktrans_dev(dev);
	kfree(dev);
}
/*
*
*/
static int nftl_open(struct mtd_blktrans_dev *dev)
{
	printk("%s: %s\n", __func__, dev->tr->name);
	return 0;
}
/*
*
*/
static int nftl_flush(struct mtd_blktrans_dev *dev)
{
	whoami();
	nand_updata();
	return 0;
}
/*
*
*/
static int nftl_release(struct mtd_blktrans_dev *dev)
{
        whoami();
        return 0;	
}
static int nftl_getgeo(struct mtd_blktrans_dev *dev,  struct hd_geometry *geo)
{
	whoami();
	geo->heads = heads;
	geo->sectors = sectors;
	geo->cylinders = cylinders;
	return 0;
}
/*
*
*/
static int nftl_readblock(struct mtd_blktrans_dev *mbd, unsigned long block, char *buffer)
{
	int result;
	unsigned long lba = translate_block(block) ;

//	if(block==0) printk("read MBR\n");
//	DEBUG_MSG("  %s(%d, 0x%08lx)\n", __func__, block, (unsigned long)buffer);
	result = FTL_Read(lba, 1, buffer);
	return TRANS_AM_RC(result, -EIO);
}
/*
*
*/
static int nftl_writeblock(struct mtd_blktrans_dev *mbd, unsigned long block, char *buffer)
{
	int result;
	unsigned long lba = translate_block(block) ;

//	printk("  %s(%d, 0x%08lx)\n", __func__, block,(unsigned long)buffer);
	result = FTL_Write(lba, 1, buffer);
	return TRANS_AM_RC(result, -EIO);
}
#else


/*
*
*/
static int nftl_open(struct inode *node, struct file *fd)
{
     whoami();
    return 0;
}
/*
*
*/
static int nftl_release(struct inode *node, struct file *fd)
{
    whoami();
    return 0;	
}

/*
*
*/
static int nftl_ioctl(struct inode *inode, struct file *file,
		unsigned  cmd, unsigned long param)
{
        
    struct media_ops  *Tmpops;
    nand_parm  ops;
    Tmpops=(struct media_ops  *)param;
    
    
    switch (cmd) {
        case IOCTL_GET_MEDIAOPS:
            Tmpops->media_read = flash_read_sectors;
            Tmpops->media_write= flash_write_sectors;
            Tmpops->get_media_cap= flash_getcapacity;
            Tmpops->media_update = nand_updata;
            // Tmpops->nand_wpdectect
            break;
         case IOCTL_GET_FLASHOPS: //define <sdk/include/sys_nand.h>
            {
                printk("run to :%s %d \n",__func__,__LINE__);
              //  void __user *argp = (void __user *)param;
                
              //  copy_from_user((void*)ops,(void*)param,256);
                
              //  ops=(struct nand_ops  *)argp;
              //  ops ->flash_log_Read = nand_adfu_read2;
              //  ops ->flash_log_Write= nand_adfu_write;
                
              ///  copy_to_user((void*)param,(void*)ops,256);
                
                break;
            }
         case IOCTL_LOG_Read:
            {
                printk("##%s %d IOCTL_LOG_Read cmd:0x%x##\n",__func__,__LINE__,cmd);
                copy_from_user((void*)&ops,(void*)param,sizeof(nand_parm));
                printk("lba: %d, %d ,\n",ops.Lba,ops.Length);
                nand_adfu_read(ops.Lba,CmpBuf,ops.Length);
                printk("run to :%s %d \n",__func__,__LINE__);
                copy_to_user((void*)ops.Buf,(void*)CmpBuf,ops.Length*512);
                printk("run to :%s %d \n",__func__,__LINE__);
                break;
            }
         case IOCTL_LOG_Write:
            {
                printk("##%s %d IOCTL_LOG_Write cmd:0x%x##\n",__func__,__LINE__,cmd);
                copy_from_user((void*)&ops,(void*)param,sizeof(nand_parm));
                
                copy_from_user((void*)CmpBuf,(void*)ops.Buf,ops.Length*512);
                
                nand_adfu_write(ops.Lba,CmpBuf,ops.Length);
                
                break;
            }
         case IOCTL_PHYBoot_Read:
            {
                int iLoop;
                printk("##%s %d IOCTL_PHYBoot_Read cmd:0x%x ##\n",__func__,__LINE__,cmd);
                copy_from_user((void*)&ops,(void*)param,sizeof(nand_parm));
              ///  printf("#lba:%d,page:%d,len;%d,%p##\n",ops.Lba,ops.bPage,ops.Length,ops.Buf);
                memset((void*)CmpBuf,0x00,ops.Length*512);
                for(iLoop =0x00;iLoop<ops.Length;iLoop++)
                {
                    brec_sector_read(ops.Lba,iLoop+ops.bPage,CmpBuf);
                }                
                copy_to_user((void*)ops.Buf,(void*)CmpBuf,ops.Length*512);
                
                break;
            }
         case IOCTL_PHYBoot_Write:
            {
                int iLoop;
                printk("##%s %d IOCTL_PHYBoot_Write cmd:0x%x## \n",__func__,__LINE__,cmd);
                copy_from_user((void*)&ops,(void*)param,sizeof(nand_parm));
                copy_from_user((void*)CmpBuf,(void*)ops.Buf,ops.Length*512);
                for(iLoop =0x00;iLoop<ops.Length;iLoop++)
                {
                    brec_sector_write(ops.Lba,iLoop+ops.bPage,CmpBuf);
                }                
                
                
                break;
            }
         case IOCTL_PHY_Erase:
            {
                printk("run to :%s %d IOCTL_PHY_Erase cmd:0x%x \n",__func__,__LINE__,cmd);
                copy_from_user((void*)&ops,(void*)param,sizeof(nand_parm));
              ///  printf("#lba:%d,page:%d,len;%d,%p##\n",ops.Lba,ops.bPage,ops.Length,ops.Buf);
                
                Entry_Phy_EraseBlk(ops.Lba,ops.bPage,ops.Length,NULL); 
                break;
            }
         case IOCTL_PHY_ReadPge:
            {
                printk("run to :%s %d IOCTL_PHY_ReadPge cmd:0x%x \n",__func__,__LINE__,cmd);
                copy_from_user((void*)&ops,(void*)param,sizeof(nand_parm));
                printf("#lba:%d,page:%d,len;%d,%p##\n",ops.Lba,ops.bPage,ops.Length,ops.Buf);
                Entry_Phy_ReadPage(ops.Lba,ops.bPage,ops.Length,CmpBuf);
                copy_to_user((void*)ops.Buf,(void*)CmpBuf,ops.Length*512);                
                
                break;
            }
           case IOCTL_PHY_WritePge:
            {
                printk("run to :%s %d IOCTL_PHY_WritePge cmd:0x%x \n",__func__,__LINE__,cmd);
                copy_from_user((void*)&ops,(void*)param,sizeof(nand_parm));
                copy_from_user((void*)CmpBuf,(void*)ops.Buf,ops.Length*512);
                Entry_Phy_WritePage(ops.Lba,ops.bPage,ops.Length,CmpBuf);
                
                break;
            }
		    case IOCTL_WRITE_ENABLE:
				write_flash_flag = WRITE_ENABLE;
				printk("run to :%s %d IOCTL_WRITE_ENABLE cmd:0x%x \n",__func__,__LINE__,cmd);
				break;
			case IOCTL_WRITE_DISABLE:
				write_flash_flag = WRITE_DISABLE;
				printk("run to :%s %d IOCTL_WRITE_DISABLE cmd:0x%x \n",__func__,__LINE__,cmd);
				break;
        default :
            break;
    }
    return 0x00;
  
}
/*
*
*/
static int nftl_media_changed(struct gendisk *geo)
{
    whoami();
    return 0;
}
/*
*
*/
static int nftl_getgeo(struct block_device *dev, struct hd_geometry *geo)
{
    whoami();
    geo->heads = heads;
    geo->sectors = sectors;
    geo->cylinders = cylinders;
    return 0;
}
/*
*
*/
int Flash_GetPartNum(struct Flash_Partiton *Fparts)
{
    int iLoop,jLoop,PartNo;
    char ch,* partInfo;

    partInfo=prom_getAM7X_PARTS();
    printk(KERN_WARNING"Partinfo:%s\n",partInfo);
    if(memcmp(partInfo,"AM7X_PARTS=",11))
    {
        return 0x00;    
    }
    jLoop =0x00;
    iLoop = 11;
    ch = partInfo[NAME_SIZE_MAX];
    PartNo =0x00;
    while(iLoop<PART_Str_SIZE)
    {
        ch = partInfo[iLoop];
        ///      printk("NO:%d,%c 0x%x, ",iLoop,ch,ch);
        if(ch ==',')
        {      
            if(jLoop>=NAME_SIZE_MAX)
            {
                jLoop= NAME_SIZE_MAX-1;
            }
            Fparts[PartNo].partName[jLoop] ='\0';                       
            // printk("name:%s\n", Fparts[PartNo].partName);
            PartNo++;
            jLoop =0x00;                       
        }
        else if (ch==0x00)
        {     
            if(jLoop>=NAME_SIZE_MAX)
            {
                jLoop= NAME_SIZE_MAX-1;
            }
            Fparts[PartNo].partName[jLoop] ='\0';
           // printk("name:%s,\n", Fparts[PartNo].partName);
            break;
        }
        else{
            Fparts[PartNo].partName[jLoop++] =ch;
        }
        iLoop++;
    }
    /// printk("PartNo:%d\n",PartNo);
    return (PartNo+1);
}
EXPORT_SYMBOL(Flash_GetPartNum);

/*
*
*/
int Flash_Get_PartNumber(char *name)
{
    INT8U iLoop;
    INT8U res;
    for(iLoop =0x00;iLoop<MAX__PARTS;iLoop++)
    {
        if(0x00==strcmp(Fparts[iLoop].partName,name))
        {
            res = iLoop;
            break;
        }
    }
    if(MAX__PARTS==iLoop)
    {
        res = 0xFF;
        printk(KERN_WARNING"Partition name Error! %s Your crazy \n",name);
    }
    return res;     
        
}
EXPORT_SYMBOL(Flash_Get_PartNumber);
#endif
/*
*
*/
INT32U bStartFlag=0x00;
INT32U wTotalTime =0x00;
INT32U wTotalSector=0x00;
static int nftl_read_Mulitblock(struct mtd_blktrans_dev *mbd, unsigned long block, 
                 unsigned int sector ,char *buffer)
{
    int result;
    unsigned long lba = translate_block(block) ;
//	if(WRITE_DISABLE== write_flash_flag){
//        printk("#read nand is disable,so do nothing and quit\n");
//		return 0;
//	}
    am_get_multi_res(FLASH_CARD);
   
    result = FTL_Read(lba, sector, buffer);

    /*
    if(block>=0x10576)
    {
        bStartFlag =0x01;			
        if(block ==0x10576)
        {
            wTotalTime = 0x0;
            wTotalSector =0x00;
        }
    }
    else
    {
        bStartFlag = 0x0;
    }
    */	
    am_release_multi_res(FLASH_CARD);
#if ADD_UPDATE_MODE ==0x01      
    mod_timer(&Flash_Host.timer,jiffies+WAIT_ILDE_TIME*8);	
#endif
    return TRANS_AM_RC(result, -EIO);
}
/*
*
*/
static int nftl_write_Mulitblock(struct mtd_blktrans_dev *mbd, unsigned long block, 
           unsigned int sector ,char *buffer)
{
    int result;
    unsigned long lba = translate_block(block) ;	
	if(WRITE_DISABLE== write_flash_flag){
        printk("write to nand is disable,so do nothing and quit\n");
		return 0;
	}
    Flash_Host.WriteFlashFlag =0x01;
    
    // printk("WRITE lba:%x,Sector:%d,Buf:0x%x\n",lba,sector,buffer);
    // printk("\n %s  lba:%x,Sector:%d,Buf:0x%x\n",__func__,lba,sector,buffer);
    am_get_multi_res(FLASH_CARD);
   
    result = FTL_Write(lba, sector, buffer);
    
    am_release_multi_res(FLASH_CARD);
    
#if ADD_UPDATE_MODE ==0x01      
    mod_timer(&Flash_Host.timer,jiffies+WAIT_ILDE_TIME*8);	
#endif

    return TRANS_AM_RC(result, -EIO);
        
}
/*
*
*/
static INT32S flash_read_sectors(INT32U offset, INT8U *buffer, INT32U sectors,INT8U *Part )
{

    int result;
    int partno;
    unsigned long lba;
    //   INIT_BOOT("[");	
//	if(WRITE_DISABLE== write_flash_flag){
//        printk("flash read nand is disable,so do nothing and quit\n");
//		return 1;
//	}
    //  printk("\n %s  lba:%x,Sector:%d,Buf:0x%x\n",__func__,offset,sectors,buffer);
    partno = Flash_Get_PartNumber(Part);
    lba = translate_block(offset+Fparts[partno].part_off) ;
    /*if(offset == 391336)
    {	
    printk("###%s  lba:%x,Sector:%d,Buf:0x%x\n",__func__,lba,sectors,buffer);
    }*/
    am_get_multi_res(FLASH_CARD);
    
 
    result = FTL_Read(lba, sectors, buffer);

    am_release_multi_res(FLASH_CARD);
#if ADD_UPDATE_MODE ==0x01        
    mod_timer(&Flash_Host.timer,jiffies+WAIT_ILDE_TIME*8);	
#endif
  
    return result;
       /// return TRANS_AM_RC(result, -EIO);
}
/*
*
*/
static INT32S flash_write_sectors(INT32U offset, INT8U *buffer, INT32U sectors,INT8U *Part )
{
    int result;
    int partno;
    unsigned long lba;
    //    struct timeval tpstart,tpend; 
    //   INT32U Timetmp;
    //  do_gettimeofday(&tpstart); 
	if(WRITE_DISABLE== write_flash_flag){
        printk("in sector write funtion:do nothing and quit\n");
		return 0;
	}   

    /// printk(" %s  lba:%5x, %d CNT:%4d,Buf:0x%8x\n",__func__,offset,offset/2048,sectors,buffer);
    partno = Flash_Get_PartNumber(Part);
    lba = translate_block(offset+Fparts[partno].part_off) ;
    Flash_Host.WriteFlashFlag =0x01;
    
    //    printk("###%s WRITE lba:%x,Sector:%d,Buf:0x%x\n",__func__,lba,sectors,buffer);
    //      mutex_lock(&Flash_Host.lock);
    am_get_multi_res(FLASH_CARD);
    
    
    result = FTL_Write(lba, sectors, buffer);
#if 0	
	MEMSET(CmpBuf,0xff,sectors*512);
	
	FTL_Read(lba, sectors, CmpBuf);	
	if(MEMCMP(CmpBuf,buffer,512*sectors))
	{
		printk("No1 @@@@@@%s WRITE lba:%x,Sector:%d,Buf:0x%x\n",__func__,lba,sectors,buffer);
	}
	if(MEMCMP(CmpBuf,buffer,512*sectors))
	{
		 printk("No2 @@@@@@%s WRITE lba:%x,Sector:%d,Buf:0x%x\n",__func__,lba,sectors,buffer);
		if(sectors<=8)
		{
			 Str_printf("Write Buf\n",buffer, 512*sectors);
			 Str_printf("Read Buf\n",CmpBuf, 512*sectors);
		}	
		 FTL_Read(lba, sectors, CmpBuf);
		 if(MEMCMP(CmpBuf,buffer,512*sectors))
			 printk("No3 $$$$$$$$%s WRITE lba:%x,Sector:%d,Buf:0x%x\n",__func__,lba,sectors,buffer);		 
	
	}
	else
	{
		//printk("###%s xxxxxxxWRITE lba:%x,Sector:%d,Buf:0x%x\n",__func__,lba,sectors,buffer);
	}
#endif	
    am_release_multi_res(FLASH_CARD);

#if 0
    //mutex_unlock(&Flash_Host.lock);        
    do_gettimeofday(&tpend); 	
    Timetmp  = 1000000*(tpend.tv_sec-tpstart.tv_sec) + tpend.tv_usec-tpstart.tv_usec; 
    Timetmp /= 1000;
    printk("@@:%3d ms :0x%6x, %d :%4d,%x\n",Timetmp,offset,offset/2048,sectors,buffer);
#endif    
 #if ADD_UPDATE_MODE ==0x01   
 //Timeout 400*4ms
    mod_timer(&Flash_Host.timer,jiffies+WAIT_ILDE_TIME*8);	
 #endif
        return result;
    // return TRANS_AM_RC(result, -EIO);
}
/*
*
*/
static INT32S  nand_adfu_read(INT32U Lba, INT8U *Buf, INT32U Length)
{
    int result;
    /**/
    down_timeout(&Flash_Host.flash_sem_op,2000);
    
    am_get_multi_res(FLASH_CARD);   
    result = FTL_Read(Lba, Length, Buf);    

    am_release_multi_res(FLASH_CARD);

    up(&Flash_Host.flash_sem_op);
    
    return result;
       
}
/*
*
*/
static INT32S  nand_adfu_write(INT32U Lba, INT8U *Buf, INT32U Length)
{
    int result;

    down_timeout(&Flash_Host.flash_sem_op,2000);
    
    am_get_multi_res(FLASH_CARD);
    
    result = FTL_Write(Lba, Length, Buf);

    am_release_multi_res(FLASH_CARD);

    up(&Flash_Host.flash_sem_op);
    
    return result;

}
/*
*
*/
static INT32S flash_getcapacity(INT8U *Part)
{
    int partno;
    int cap;
    partno = Flash_Get_PartNumber(Part);
    cap = Fparts[partno].part_size;
    return cap;
}
/*
*
*/
INT32U get_nand_flash_type(void)
{
    INT32U nand_type;
	nand_type = '8' * 0x1000000 + '4' * 0x10000 + '6' * 0x100 + 'F';
	return nand_type;
}
void nand_logical_update(void)
{
    nand_updata();    
}
/*
*
*/
struct StorageInfo * GetNandStorageInfo(void)
{
    return  &NandStorageInfo;
}

static INT8U StopCheck=0x00;
INT32U Stop_Check_Recover(INT8U Flag)
{
    if(Flag ==0x01)
    {
        StopCheck =0x1;
        printk("\n\n###%s-%d CHECK_STOP_VAL:%d ###\n\n",__func__,__LINE__,StopCheck);
        return StopCheck;
        
    }
    else
    {
        ///printk("\n\n###%s-%d ###\n\n",__func__,__LINE__,StopCheck);
        return StopCheck;
    }
}
EXPORT_SYMBOL(Stop_Check_Recover);
EXPORT_SYMBOL(flash_read_sectors);
EXPORT_SYMBOL(flash_write_sectors);
EXPORT_SYMBOL(flash_getcapacity);
EXPORT_SYMBOL(nand_adfu_read);
EXPORT_SYMBOL(nand_adfu_write);
EXPORT_SYMBOL(get_nand_flash_type);
EXPORT_SYMBOL(GetNandStorageInfo);
EXPORT_SYMBOL(nand_logical_update);


#if NEW_BLOCK_MODE  ==0x00
static struct mtd_blktrans_ops nftl_tr = {
	.name		= "nftl",
	.major		= NFTL_MAJOR,
	.part_bits	= NFTL_PARTN_BITS,
	.blksize    = 512,
	.open		= nftl_open,
	.flush		= nftl_flush,
	.release	= nftl_release,

	
	.getgeo		= nftl_getgeo,
	.readsect	= nftl_readblock,
	.writesect	= nftl_writeblock,
	.read_MultiSect = nftl_read_Mulitblock,
	.write_MultiSect =nftl_write_Mulitblock,

	.add_mtd	= am7x_nftl_add_mtd,
	.remove_dev	= nftl_remove_dev,
	.owner		= THIS_MODULE,
};


struct mtd_info am7x_mtd_table[AM7X_MTD_DEVICES] = {
	{
		.type      = MTD_NANDFLASH,
		.flags     = MTD_WRITEABLE,
		.size      = 0x1000000,
		.writesize = 512,
		.erasesize = 4096,
		.oobsize   = 0,
		.oobavail  = 0,
		.name      = "nftl",
		.index     = -1
	}
};
#else
/*
*
*/
#define Flash_BUFFER_READ_MODE  0x00
int nftl_make_request(struct request_queue *q, struct bio *bio)
{
	struct bio_vec *bvec;
	int i;
	int nftl_lba = bio->bi_sector;
	int ret = 0;
         int nSector;
#if Flash_BUFFER_READ_MODE  ==0x01       
	char *Buf,*Buf2;
	int  Tmplba,TmpSect,CurSect,sequenceFlag;
	int iLoop,TotalSector;
#endif  
	/*	
	struct timeval tpstart,tpend; 
	INT32U span,speed,biCnt;	
	do_gettimeofday(&tpstart); 	
	biCnt = bio_sectors(bio);
	*/
//	if(bStartFlag ==0x01)
 //  printk("\n###%s,lba:%x,Cnt:0x%x,bi_vcnt:0x%x\n",__func__,nftl_lba,bio_sectors(bio),(bio)->bi_vcnt);
	down_timeout(&Flash_Host.flash_sem_op,2000);
	 
#if Flash_BUFFER_READ_MODE  ==0x01       

	sequenceFlag =0x01;
	TmpSect = nftl_lba;
	TotalSector =0x00;
	Tmplba =0x00;
	Buf =NULL;

	for(iLoop =0x00;iLoop<(bio)->bi_vcnt;iLoop++)
	{  
		Buf2 =kmap(bio_iovec_idx((bio), (iLoop))->bv_page) + bio_iovec_idx((bio), (iLoop))->bv_offset;
		CurSect = bio_iovec_idx(bio, iLoop)->bv_len >> 9;
		TotalSector +=CurSect;
		//	if(bStartFlag ==0x01)		
		//      printk("Buf2:%x,%x\n",Buf2,CurSect);
		if(iLoop==0x00)
		{
			Tmplba = TmpSect;
			Buf= Buf2;
		}
		else
		{

			if((Buf+  CurSect*512 !=   Buf2  ) || (Tmplba+CurSect !=TmpSect))
			{
				sequenceFlag =0x00;                           
			}
			else
			{
				Tmplba = TmpSect;
				Buf= Buf2;                      
			}
		}
		TmpSect = TmpSect+CurSect;                        
	}
        
	if(sequenceFlag && TotalSector>=16)
	{
		iLoop = 0x00;                    
		Buf2 = (unsigned long)kmap(bio_iovec_idx((bio), (iLoop))->bv_page) + bio_iovec_idx((bio), (iLoop))->bv_offset;
		//printk("Buf:0x%x,total:0x%x,lba:0x%x\n",Buf2,TotalSector,nftl_lba);
		switch (bio_rw(bio))
		{
			case READ:
			case READA:	                          
				ret =nftl_read_Mulitblock(NULL,nftl_lba,TotalSector, Buf2 );			
				break;
			case WRITE:                     
				ret =nftl_write_Mulitblock(NULL,nftl_lba,TotalSector, Buf2);
				break;
			default:
				break;
		}
		// __bio_kunmap_atomic(Buf2, KM_USER0);     
		bio_endio(bio, ret);
		up(&Flash_Host.flash_sem_op);
		return 0;
	}
#endif    
	   
    bio_for_each_segment(bvec, bio, i) {
        nSector = (bvec->bv_len >> 9);
        char* buffer =(unsigned long)kmap(bvec->bv_page) + bvec->bv_offset;   
        //char* buffer = __bio_kmap_atomic(bio, i,KM_USER0);
        switch (bio_rw(bio)) {
            case READ:
            case READA:	
                ///if(bStartFlag ==0x01)	
                ///printk("@@@%x,%x\n",(INT32U)buffer,bio_cur_sectors(bio));
                ret =nftl_read_Mulitblock(NULL,nftl_lba,nSector, buffer );
                break;
            case WRITE:
                ret =nftl_write_Mulitblock(NULL,nftl_lba,nSector, buffer);
                break;
            default:
                printk(KERN_ERR "nftal blk"
                        ": unknown value of bio_rw: %lu\n",  bio_rw(bio));
                bio_endio(bio, -EIO);
                up(&Flash_Host.flash_sem_op);				
            return 0;
        }
        nftl_lba += nSector;
        //__bio_kunmap_atomic(bio, KM_USER0);
    }
    bio_endio(bio, 0);
    up(&Flash_Host.flash_sem_op);
/*	
	do_gettimeofday(&tpend); 
	span  = 1000000*(tpend.tv_sec-tpstart.tv_sec) + tpend.tv_usec-tpstart.tv_usec; 
	if(bStartFlag ==0x01)
	{
		wTotalTime +=span;
		wTotalSector+= biCnt;
		/////printk("###%s,lba:0x%x,%x,cnt:0x%x\n",__func__,nftl_lba,bio_sectors(bio),(bio)->bi_vcnt);
		//printk("%x,%x\n",wTotalTime,wTotalSector);
		if(wTotalSector ==2048*10)
		{
			wTotalTime =wTotalTime/1000;
			speed = (wTotalSector / 2 * 1000) / wTotalTime;
			printk("@@    %5uKB,%5dms,  %u KB/S\n", wTotalSector/2, wTotalTime, speed); 
			wTotalSector =0x00;
			wTotalTime =0x00;
		}
	}
*/	
        return ret;
}

struct nftl_block_driver {
	struct request_queue *nftl_blkdev_queue;
	struct gendisk *nftl_blk_disk;
};

static struct nftl_block_driver nftl_driver;


/*
*
*/   
struct block_device_operations nftl_block_ops = {        
    .open = nftl_open,
    .release = nftl_release,
    .ioctl = nftl_ioctl,	
    .unlocked_ioctl=NULL,
    .media_changed = nftl_media_changed,
    .revalidate_disk=NULL,
    .getgeo =nftl_getgeo,
    .owner = THIS_MODULE       
};

#endif
/*
static void test_kmalloc()
{
	void *buf;
	unsigned int i, size;
	for(i=0; i<8; i++) {
		size = 4096 * i;
		buf = kmalloc(size, GFP_NOIO);
		if(buf != NULL) {
			printk("A. kmalloc(%u) okay\n", size);
			kfree(buf);
		}
	}
	for(i=0; i<8; i++) {
		size = 256 * i;
		buf = kmalloc(size, GFP_DMA );
		if(buf != NULL) {
			printk("B. kmalloc(%u) okay\n", size);
			kfree(buf);
		}
	}
}
*/
void dump_read_data(const INT8U *pdata,INT16U len){
	INT16U tmp=0;
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
void Str_printf( const INT8U*  pad, const INT8U* pData, INT16U inLen);
/*
*
*/
/* Probe MTD devices on AM7XXX evaluation board */
static int __init probe_am7x_mtd_devices(void)
{
    int result;
    u32 logical_whole_cap;
    u8* TmpBuf;
    int iLoop,partCnt;

    nand_flash_inithw();
    result = INIT_CreateBlkTbls();	
    if(result == FALSE) {
        printk("INIT_CreateBlkTbls errorccc\n");
        return -ENODEV;
    }
	TmpBuf=(u8*)kmalloc(512, GFP_KERNEL);
    FTL_Init();	
	FTL_Read(0x00, 0x01, TmpBuf);
	kernelcap = *(u32 *)(TmpBuf+0x90);
	if(0x00 == kernelcap)
		kernelcap = 0x10000;
	printk("kernel capacity is:0x%x sectors\n",kernelcap);
#if NEW_BLOCK_MODE  ==0x01      
    partCnt=Flash_GetPartNum(Fparts);        
    
    FTL_Read(translate_block(0x00), 0x01, TmpBuf);
//	dump_read_data(TmpBuf,512);
    for(iLoop =0x00;iLoop<partCnt;iLoop++)
    {              
        Fparts[iLoop].part_off =get_unaligned((INT32U*)(TmpBuf+256+16*(0+iLoop)+6));        
        Fparts[iLoop].part_size =get_unaligned((INT32U*)(TmpBuf+256+16*(0+iLoop)+10));
        printk("NO:%d,%s offset:%x,Cap:%x %dMB\n",iLoop,Fparts[iLoop].partName,Fparts[iLoop].part_off,
                Fparts[iLoop].part_size,  Fparts[iLoop].part_size/2048);
    } 
#endif        
    /* Calculate geometry */
    cylinders = 1024;
    heads = 16;
    logical_whole_cap = nand_get_cap();
    
    sectors = 32;
    heads = 128;
    cylinders = logical_whole_cap / (sectors * heads);
    
    printk(KERN_WARNING "NFTL: whole cap 0x%0x\n", logical_whole_cap);
    printk(KERN_WARNING "NFTL: using C:%d H:%d S:%d "
    		"(== 0x%lx sects)\n",
    		cylinders, heads , sectors, 
    		(long)cylinders * (long)heads * (long)sectors );
        
#if NEW_BLOCK_MODE  ==0x00
        mtd_table[0]       = &am7x_mtd_table[0];
        mtd_table[0]->size = sectors*cylinders*heads;
#endif
      //  timer.
     /// Flash_Host.timer
#if ADD_UPDATE_MODE ==0x01 
    init_timer(&Flash_Host.timer);
    Flash_Host.timer.expires = jiffies + WAIT_ILDE_TIME;   
    Flash_Host.timer.function = Flash_poll_event;
    // Flash_Host.timer.data = (unsigned long) Flash_Host;
    Flash_Host.WriteFlashFlag =0x00;
	write_flash_flag=WRITE_ENABLE;
    add_timer(&Flash_Host.timer);

    Flash_Host.thread = kthread_create(Flash_Update_thread,NULL,"%s","Flash_Update_threadthread");
#endif


    am_init_multi_res(FLASH_CARD,1);

    mutex_init(&Flash_Host.lock);

    sema_init(&Flash_Host.flash_sem_op ,1);	

#if TEST_FLASH_SPEED
    Flash_Speed_Test();
#endif
	kfree(TmpBuf);
        return 0;
}
/*
*
*/
static int __init init_am7x_nftl(void)
{
    int i, result;
    int ret;
    result= i=0x00;
    whoami();
    printk("\n%s: CHIPINFO:  0x%08x\n", __func__, CFG_CHIPINFO_VAL);
    //	test_kmalloc();

    probe_am7x_mtd_devices();
#if NEW_BLOCK_MODE  ==0x01
    nftl_driver.nftl_blkdev_queue = blk_alloc_queue(GFP_KERNEL);
    if(!nftl_driver.nftl_blkdev_queue){
        ret = ENOMEM;
        printk(KERN_WARNING "NFTL: blk alloc queue fail\n");
        goto nftla_initOut;
    }

    blk_queue_make_request(nftl_driver.nftl_blkdev_queue, nftl_make_request);

    nftl_driver.nftl_blk_disk = alloc_disk(32);
    if(!nftl_driver.nftl_blk_disk){                
        ret = ENOMEM;   
        blk_cleanup_queue(nftl_driver.nftl_blkdev_queue);
    }        
        
    strcpy(nftl_driver.nftl_blk_disk->disk_name, "nand_block");

    CmpBuf =(void*)MALLOC(128 * 512);
    printk("\n### CmpBuf:0x%p###\n",CmpBuf);

    nftl_driver.nftl_blk_disk->major = NFTL_MAJOR;
    nftl_driver.nftl_blk_disk->first_minor = 0;
    nftl_driver.nftl_blk_disk->fops = &nftl_block_ops;
    nftl_driver.nftl_blk_disk->queue = nftl_driver.nftl_blkdev_queue;
    set_capacity(nftl_driver.nftl_blk_disk,nand_get_cap());


    add_disk(nftl_driver.nftl_blk_disk);
nftla_initOut:
    printk("***install ntfl block init OK***\n");
    return 0x00;
        

#else
    result = register_mtd_blktrans(&nftl_tr);
    if(result < 0) {
            printk("NFTL: register_mtd_blktrans failed, ret=%d\n", result);
            return result;
    }

    for(i=0; i<AM7X_MTD_DEVICES; i++) {
        if( am7x_mtd_table[i].type == MTD_ABSENT )
                continue;
        printk(" MTD[%d]: size 0x%08x, writesize %d, erasesize %d, name [%s]\n",i,
                am7x_mtd_table[i].size, am7x_mtd_table[i].writesize, am7x_mtd_table[i].erasesize,
                 am7x_mtd_table[i].name);
    }
    printk("NR_LFI_SECTS: %d\n", kernelcap);
    return 0;
    
#endif        
        
}
/*
*
*/
static void __exit exit_am7x_nftl(void)
{
    whoami();     
#if NEW_BLOCK_MODE ==0x00       
    deregister_mtd_blktrans(&nftl_tr);
#else

    del_gendisk(nftl_driver.nftl_blk_disk);
    printk("*** run to 2***\n");
    put_disk(nftl_driver.nftl_blk_disk);
    printk("*** run to 3***\n");
    blk_cleanup_queue(nftl_driver.nftl_blkdev_queue);
    printk("*** run to 4***\n");

    printk("*** run to last***\n");
#endif
}

module_init(init_am7x_nftl);
module_exit(exit_am7x_nftl);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR("Onlyfish xu");
MODULE_DESCRIPTION("AM7X flash block Translation Layer");


