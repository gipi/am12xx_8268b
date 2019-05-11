/*
 * linux/driver/mmc/host/actions_mci.c - Actions-micro MMC driver
 *
 * Copyright (C) 2014
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/mmc/host.h>
#include <linux/scatterlist.h>

#include <asm/cacheflush.h>

#include <actions_io.h>
#include <actions_regs.h>
#include <actions_mci.h>
//#include <am_platform_device.h>
#include <irq.h>
#include <dma.h>

#define DRIVER_NAME	"sd-am7xxx"


#define DBG_PRT(cond, fmt, args...)		\
        do {                                    \
                if (cond)                       \
                        printk(fmt, ##args);    \
        } while (0)


#define	RESET_CONTROLLER				\
{                                       \
    int loop;                           \
    for(loop=0;loop<10;loop++)          \
        act_writel(0xe0000000, CRC_CTL);\
}


#define SD_DEBUG	0
#define SD_DEBUG_R	0
#define SD_DEBUG_W	0
#define SD_ENABLE_IRQ	1
#define SD_START_ADD  0

enum{
	SD_IN_INIT = 0,
	SD_IN_IDLE = 1,
	SD_IN_READY = 2,
	SD_IN_IDENT = 3,
	SD_IN_STBY = 4,
	SD_IN_TRANS = 5,
	SD_IN_READ = 6,
	SD_IN_WRITE = 7,
	SD_IN_FREE = 8,
	SD_IN_BUSY = 9,
	SD_IN_CHECK = 10,
};




enum{
    SD_CTL_ERR,
    SD_CTL_IDLE,
};

struct am_mci{
	struct mmc_host	*mmc;
	struct resource	*res;
	void __iomem	*reg_base;
	u32 		SD_mode;
	u32		dma_cn;
	int		irq;
    u32 irq_en;
    u32 irq_pend;
	unsigned int	dma_sg_len;
	unsigned int	dma_dir;
      u32         SD_CTL_STATE;
	u32       sd_clock;
	u32       sd_parameter;
	struct semaphore mmc_sem;
     dma_addr_t adma_phy;
     u8 * adma_virt;
};

//Function define
static int am_SD_send_msg(struct am_mci *host, u8 *data, int len, u32 wait_time, u32 cmd_data);
static int am_SD_rece_msg(struct am_mci *host, u8 *data, int len, u32 wait_time);
static int am_SD_send_para_cmd(struct am_mci *host, u8 SD_para, u8 option);
static int am_SD_send_clock_cmd(struct am_mci *host, u8 ctl_clock);
static int am_SD_send_cmd(struct am_mci *host, u8 cmd_idx, u32 cmd_arg, u8 rsp_type, u8 *rsp_buf);
static void actmci_set_sdio_irq(struct mmc_host *host, int enable);
void actmci_set_irq(struct am_mci *host, unsigned int flag);
void set_sdio_int(struct am_mci *host,int enable);


//Function
static void card_stop(struct am_mci *host)
{
	unsigned char cmd[2];
	int result = SD_ERROR;
	cmd[0] = (unsigned char) (SD_CARD_STOP & 0xff);
	cmd[1] = (unsigned char) ((SD_CARD_STOP >> 8) & 0xff);

	result = am_SD_send_msg(host, cmd, sizeof(cmd), SD_DFT_WAIT_T, SD_CTL_CMD);
	if(result != SD_GOOD)
	{
		printk("ERROR STOP CARD\n");
	}
    else
    {
        host->SD_CTL_STATE = SD_CTL_IDLE;
    }
}

static void reset_ctller(struct am_mci *host)
{
	int loop;
	for(loop=0;loop<10;loop++)
	{
		writel(0xe0000000, host->reg_base + SD_CTL_REG);
		udelay(100);
	}
    card_stop(host);
}


void GPIO25_cpl(int cnt)
{
    // GPIO26
    u32 gpio_cpl;
    u32 i;
    gpio_cpl = act_readl(0xb01c0040);
    gpio_cpl = gpio_cpl&(~(3<<22));
    act_writel(gpio_cpl,0xb01c0040);

    gpio_cpl = act_readl(0xb01c0000);
    gpio_cpl = gpio_cpl|(1<<25);
    act_writel(gpio_cpl,0xb01c0000);

    gpio_cpl = act_readl(0xb01c0008);
    for(i=0;i<cnt;i++)
    {
        gpio_cpl = gpio_cpl^(1<<25);
        act_writel(gpio_cpl,0xb01c0008);    
    }
    
}



void GPIO26_cpl(int cnt)
{
    // GPIO26
    u32 gpio_cpl;
    u32 i;
    gpio_cpl = act_readl(0xb01c0040);
    gpio_cpl = gpio_cpl&(~(3<<24));
    act_writel(gpio_cpl,0xb01c0040);

    gpio_cpl = act_readl(0xb01c0000);
    gpio_cpl = gpio_cpl|(1<<26);
    act_writel(gpio_cpl,0xb01c0000);

    gpio_cpl = act_readl(0xb01c0008);
    for(i=0;i<cnt;i++)
    {
        gpio_cpl = gpio_cpl^(1<<26);
        act_writel(gpio_cpl,0xb01c0008);    
    }
    
}








void GPIO25_high(void)
{
    // GPIO26
    u32 gpio_cpl;

    gpio_cpl = act_readl(0xb01c0040);
    gpio_cpl = gpio_cpl&(~(3<<22));
    act_writel(gpio_cpl,0xb01c0040);

    gpio_cpl = act_readl(0xb01c0000);
    gpio_cpl = gpio_cpl|(1<<25);
    act_writel(gpio_cpl,0xb01c0000);

    gpio_cpl = act_readl(0xb01c0008);
    gpio_cpl = gpio_cpl | (1<<25);
    act_writel(gpio_cpl,0xb01c0008);  
    
    
}





void GPIO25_low(void)
{
    // GPIO26
    u32 gpio_cpl;
   
    gpio_cpl = act_readl(0xb01c0040);
    gpio_cpl = gpio_cpl&(~(3<<22));
    act_writel(gpio_cpl,0xb01c0040);

    gpio_cpl = act_readl(0xb01c0000);
    gpio_cpl = gpio_cpl|(1<<25);
    act_writel(gpio_cpl,0xb01c0000);

    gpio_cpl = act_readl(0xb01c0008);
    gpio_cpl = gpio_cpl&(~(1<<25));
    act_writel(gpio_cpl,0xb01c0008);   
  
    
}





void GPIO77_cpl(int cnt)
{
    // GPIO26
    u32 gpio_cpl;
    u32 i;
    gpio_cpl = act_readl(0xb01c0048);
    gpio_cpl = gpio_cpl&(~(3<<2));
    act_writel(gpio_cpl,0xb01c0048);

    gpio_cpl = act_readl(0xb01c0018);
    gpio_cpl = gpio_cpl|(1<<13);
    act_writel(gpio_cpl,0xb01c0018);

    gpio_cpl = act_readl(0xb01c0020);
    for(i=0;i<cnt;i++)
    {
        gpio_cpl = gpio_cpl^(1<<13);
        act_writel(gpio_cpl,0xb01c0020);    
    }
    
}


void GPIO80_high(void)
{
    u32 gpio_cpl;
    
    gpio_cpl = act_readl(0xb01c0048);
    gpio_cpl = gpio_cpl&(~(3<<29));
    act_writel(gpio_cpl,0xb01c0048);

    gpio_cpl = act_readl(0xb01c0018);
    gpio_cpl = gpio_cpl|(1<<16);
    act_writel(gpio_cpl,0xb01c0018);

        
    gpio_cpl = act_readl(0xb01c0020);
    gpio_cpl |= (1<<16);
    act_writel(gpio_cpl,0xb01c0020);
   
}
void GPIO77_high(void)
{
    u32 gpio_cpl;
    
    gpio_cpl = act_readl(0xb01c0048);
    gpio_cpl = gpio_cpl&(~(3<<2));
    act_writel(gpio_cpl,0xb01c0048);

    gpio_cpl = act_readl(0xb01c0018);
    gpio_cpl = gpio_cpl|(1<<13);
    act_writel(gpio_cpl,0xb01c0018);

        
    gpio_cpl = act_readl(0xb01c0020);
    gpio_cpl |= (1<<13);
    act_writel(gpio_cpl,0xb01c0020);
   
}


void GPIO80_low(void)
{
    u32 gpio_cpl;
    
    gpio_cpl = act_readl(0xb01c0048);
    gpio_cpl = gpio_cpl&(~(3<<29));
    act_writel(gpio_cpl,0xb01c0048);

    gpio_cpl = act_readl(0xb01c0018);
    gpio_cpl = gpio_cpl|(1<<16);
    act_writel(gpio_cpl,0xb01c0018);

        
    gpio_cpl = act_readl(0xb01c0020);
    gpio_cpl &= (~(1<<16));
    act_writel(gpio_cpl,0xb01c0020);
    
}


void GPIO77_low(void)
{
    u32 gpio_cpl;
    
    gpio_cpl = act_readl(0xb01c0048);
    gpio_cpl = gpio_cpl&(~(3<<2));
    act_writel(gpio_cpl,0xb01c0048);

    gpio_cpl = act_readl(0xb01c0018);
    gpio_cpl = gpio_cpl|(1<<13);
    act_writel(gpio_cpl,0xb01c0018);

        
    gpio_cpl = act_readl(0xb01c0020);
    gpio_cpl &= (~(1<<13));
    act_writel(gpio_cpl,0xb01c0020);
    
}
static int am_SD_send_msg_dma(struct am_mci *host, u8 *data, int len, u32 wait_time, u32 cmd_data)
{
	u32 temp_reg, i;

    u32 j;
    u32 dma_mode;
    u32 ret;

	//set control register and data len

    if(cmd_data == SD_CTL_CMD)
	{
		temp_reg = (SD_CTL_CMD | SD_CTL_WR) | (len & SD_CTL_MAX_LEN);
	}else{
		temp_reg = (SD_CTL_DATA | SD_CTL_WR) | (len & SD_CTL_MAX_LEN);
	}

    writel(temp_reg, host->reg_base + SD_CTL_REG);
    

    dma_mode = DMA_DST_BST_SINGLE | DMA_DST_FIX_ADDR | (DMA_TRIG_SD << DMA_DST_DRQ_BIT) |\
		   DMA_SRC_BST_SINGLE | DMA_SRC_INC_ADDR | (DMA_TRIG_SDRAM << DMA_SRC_DRQ_BIT);
    //Start transfer
	ret = am_dma_cmd(host->dma_cn, DMA_CMD_RESET);
    if(ret != 0)
        printk("%s %d error!\n",__func__,__LINE__);
    ret = am_set_dma_addr(host->dma_cn, (u32)data, SD_REG_BASE + SD_DATA_REG);
    if(ret != 0)
        printk("%s %d error!\n",__func__,__LINE__);
	ret = am_dma_config(host->dma_cn, dma_mode, len);
    if(ret != 0)
        printk("%s %d error!\n",__func__,__LINE__);
	ret = am_dma_start(host->dma_cn, 0);         //We only check in start. The better way is to rewrite the dma function call.
    if(ret != 0)
        printk("%s %d error!\n",__func__,__LINE__);
    
    //Wait stop
	for(j = 0; j <= 1000000; j++)
	{
		if(!am_dma_cmd(host->dma_cn, DMA_CMD_QUERY))
		{
			break;
		}else if(j == 1000000){
			printk("%s Error dma timeout %d\n", __func__, j);
            
			reset_ctller(host);
			return -ETIMEDOUT;
		}else{
			udelay(2); //10us
		}
	}

	//wait finish ,default 2S
	for(i = 0; i <= 100000*wait_time; i++)
	{
		//time out
		if(i == 100000*wait_time)
		{
			printk("%s TIMEOUT %d 0x%x\n", __func__, __LINE__,temp_reg);
            RESET_CONTROLLER;
            return -ETIMEDOUT;
		}

		//Check and delay
		temp_reg = readl(host->reg_base + SD_STATUS_REG);
		if(temp_reg & SD_CRC_READY)
		{
			break;
		}else{
			udelay(10); //10 us
		}
	}

	return SD_GOOD;
}



static int am_SD_send_msg(struct am_mci *host, u8 *data, int len, u32 wait_time, u32 cmd_data)
{
	u32 temp_reg, i;

    u32 j;

	//set control register and data len

    if(cmd_data == SD_CTL_CMD)
	{
		temp_reg = (SD_CTL_CMD | SD_CTL_WR) | (len & SD_CTL_MAX_LEN);
	}else{
		temp_reg = (SD_CTL_DATA | SD_CTL_WR) | (len & SD_CTL_MAX_LEN);
	}

    writel(temp_reg, host->reg_base + SD_CTL_REG);
    

	
	//send msg
	//printk("**************************************************\n");
	
	for(i=0; i<(len+3)/4; i++)
	{
        j=0;
        while((readl(host->reg_base + SD_STATUS_REG)&SD_TX_FULL_IRQ))
        {
            if(j++ < 10000*wait_time)
            {
                udelay(10);
            }
            else
            {
    			printk("%s send TIMEOUT %d 0x%x 0x%x\n", __func__, __LINE__,temp_reg,len);
                return -ETIMEDOUT;
            }
                
        }
        writel(*(u32 *)(data+i*4), host->reg_base + SD_DATA_REG);
		//printk("card_send_msg 171 0x%08x\n",*(u32 *)(data+i*4));
	}

	//wait finish ,default 2S
	for(i = 0; i <= 100000*wait_time; i++)
	{
		//time out
		if(i == 100000*wait_time)
		{
			printk("%s TIMEOUT %d 0x%x\n", __func__, __LINE__,temp_reg);
            RESET_CONTROLLER;
            return -ETIMEDOUT;
		}

		//Check and delay
		temp_reg = readl(host->reg_base + SD_STATUS_REG);
		if(temp_reg & SD_CRC_READY)
		{
			break;
		}else{
			udelay(10); //10 us
		}
	}

	return SD_GOOD;
}

static int am_SD_rece_msg(struct am_mci *host, u8 *data, int len, u32 wait_time)
{
	u32 temp_reg, i, rece_len = 0, count;
	//printk("%s 0x%x\n", __func__, data);

	//check if actions control have received response and get the receive len
	for(i = 0; i <= 10000*wait_time; i++)
	{
		temp_reg = readl(host->reg_base + SD_STATUS_REG);
		if(temp_reg & SD_TRN_ACTIVE)
		{
			rece_len = temp_reg >> 8;
			if(rece_len != len)
			{
				
                if(len >512)
                    break;
                else
				{
				
					printk("ERROR, receive len is error. len = %d, rece_len = %d\n", len, rece_len);
				    return -EILSEQ;
				}
			}
            
			break;
		}

		//delay and check time out
		if(i == 10000*wait_time)
		{
			printk("%s TIMEOUT %d\n", __func__, __LINE__);
			return -ETIMEDOUT;
		}else{
			udelay(10); //10 us
		}
	}

	//receive response from actions SD control
	writel((len & SD_CTL_MAX_LEN), host->reg_base + SD_CTL_REG);
	//printk("#############################################\n");
	for(i = 0, count=0 ; count < ((len+3)/4); )
	{
		temp_reg = readl(host->reg_base + SD_STATUS_REG);
		if(!(temp_reg & SD_RX_FIFO_EMPTY))
		{
			/*temp_reg = readl(host->reg_base + SD_DATA_REG); //for debug
 			printk("rece%d = 0x%x\n", count, temp_reg);
 			*(u32 *)(data+count*4) = temp_reg;*/
			*(volatile u32 *)(data+count*4) = readl(host->reg_base + SD_DATA_REG);
		       //printk("card_recv_msg 259 0x%08x\n",*(volatile u32 *)(data+count*4));
			count ++;
                   
		}
        else
        {
    		//delay and check time out
    		if(i == 10000*wait_time)
    		{
    			printk("%s TIMEOUT %d, count = %d 0x%x\n", __func__, __LINE__, count,readl(host->reg_base + SD_STATUS_REG));
    			return -ETIMEDOUT;
    		}else{
    			udelay(2);
    		}
			i++;
        }
        //udelay(2);
	}

	//check SD_STATUS bit 4 if transfer is done
	for(i = 0; i <= 10000*wait_time; i++)
	{
		temp_reg = readl(host->reg_base + SD_STATUS_REG);
		if(temp_reg & SD_TRN_END)
		{
			//DBG_PRT(SD_DEBUG, "reve done len = 0x%x 0x%x\n", len,temp_reg);
			break;
		}

		//delay and check time out
		if(i == 10000*wait_time)
		{
			printk("%s TIMEOUT %d\n", __func__, __LINE__);
			return -ETIMEDOUT;
		}else{
			udelay(10); //10 us
		}
	}

	return SD_GOOD;
}





static int fast_am_SD_rece_msg(struct am_mci *host, u8 *data, int len, u32 wait_time)
{
	u32 temp_reg, i, rece_len = 0, count;
	//printk("%s 0x%x\n", __func__, data);

	//check if actions control have received response and get the receive len
	for(i = 0; i <= 10000*wait_time; i++)
	{
		temp_reg = readl(host->reg_base + SD_STATUS_REG);
		if(temp_reg & SD_TRN_ACTIVE)
		{
			rece_len = temp_reg >> 8;
			if(rece_len != len)
			{
				
                if(len >512)
                    break;
                else
				{
				
					printk("ERROR, receive len is error. len = %d, rece_len = %d\n", len, rece_len);
				    return -EILSEQ;
				}
			}
            
			break;
		}

		//delay and check time out
		if(i == 100000*wait_time)
		{
			printk("%s TIMEOUT %d\n", __func__, __LINE__);
			return -ETIMEDOUT;
		}else{
			udelay(10); //10 us
		}
	}

	//receive response from actions SD control
	writel((len & SD_CTL_MAX_LEN), host->reg_base + SD_CTL_REG);
	//printk("#############################################\n");
	for(i = 0, count=0 ; count < ((len+3)/4);)
	{
		temp_reg = readl(host->reg_base + SD_STATUS_REG);
		if(!(temp_reg & SD_RX_FIFO_EMPTY))
		{
			/*temp_reg = readl(host->reg_base + SD_DATA_REG); //for debug
 			printk("rece%d = 0x%x\n", count, temp_reg);
 			*(u32 *)(data+count*4) = temp_reg;*/
			*(volatile u32 *)(data+count*4) = readl(host->reg_base + SD_DATA_REG);
		       //printk("card_recv_msg 259 0x%08x\n",*(volatile u32 *)(data+count*4));
			count ++;
                   
		}
        else
        {
    		//delay and check time out
    		if(i == 100*wait_time)
    		{
    			printk("%s TIMEOUT %d, count = %d 0x%x\n", __func__, __LINE__, count,readl(host->reg_base + SD_STATUS_REG));
    			return -ETIMEDOUT;
    		}else{
    			//udelay(2);
    		}
             i++;
        }
        udelay(2);
	}

	//check SD_STATUS bit 4 if transfer is done
	for(i = 0; i <= 100*wait_time; i++)
	{
		temp_reg = readl(host->reg_base + SD_STATUS_REG);
		if(temp_reg & SD_TRN_END)
		{
			//DBG_PRT(SD_DEBUG, "reve done len = 0x%x 0x%x\n", len,temp_reg);
			break;
		}

		//delay and check time out
		if(i == 100*wait_time)
		{
			printk("%s TIMEOUT %d\n", __func__, __LINE__);
			return -ETIMEDOUT;
		}else{
			udelay(10); //10 us
		}
	}

	return SD_GOOD;
}















static int Check_Reg(struct am_mci *host, unsigned char Reg_idx)
{
	unsigned char cmd[4], rece_buf[10];
	int result = -EILSEQ;

	cmd[0] = (unsigned char) (SD_REG_CHK & 0xff);
	cmd[1] = (unsigned char) ((SD_REG_CHK >> 8) & 0xff);
	cmd[2] = Reg_idx;
	cmd[3] = 0;  //Reserved

	result = am_SD_send_msg(host, cmd, sizeof(cmd), SD_DFT_WAIT_T, SD_CTL_CMD);
	if(result != SD_GOOD)
	{
		printk("%s REG check Send ERROR = %d\n", __func__, result);
		reset_ctller(host);
		return result;
	}

	result = am_SD_rece_msg(host, rece_buf, 2, SD_DFT_WAIT_T); //wait 2s
	if(result != SD_GOOD)
	{
		printk("%s Receive ERROR = %d\n", __func__, result);
		reset_ctller(host);
		return result;
	}

	DBG_PRT(SD_DEBUG|SD_DEBUG_R|SD_DEBUG_W, "REGCHECK = 0x%x 0x%x 0x%x\n", rece_buf[0], rece_buf[1],act_readl(CRC_STATUS));

	return result;
}

static int am_SD_send_para_cmd(struct am_mci *host, u8 SD_para, u8 option)
{
	unsigned char cmd[4];
	int result = -EILSEQ;

	cmd[0] = (unsigned char) (SD_SET_PARA & 0xff);
	cmd[1] = (unsigned char) ((SD_SET_PARA >> 8) & 0xff);
	cmd[2] = SD_para;
	cmd[3] = option;
	//cmd[4] = 0x00;
	//cmd[5] = 0x80;

	result = am_SD_send_msg(host, cmd, sizeof(cmd), SD_DFT_WAIT_T, SD_CTL_CMD);
	if(result != SD_GOOD)
	{
		printk("%s ERROR = %d\n", __func__, result);
		reset_ctller(host);
		return result;
	}

	DBG_PRT(SD_DEBUG, "%s 0x%x OK\n", __func__,act_readl(CRC_STATUS));
	//reset_ctller(host);
	return result;
}


static int am_SDIO_ENABLE_INT(struct am_mci *host, u8 SD_para, u8 Reserved)
{
	unsigned char cmd[4];
	int result = -EILSEQ;

	cmd[0] = (unsigned char) (SDIO_ENABLE_INT & 0xff);
	cmd[1] = (unsigned char) ((SDIO_ENABLE_INT >> 8) & 0xff);
	cmd[2] = SD_para;
	cmd[3] = 0;
	
	

	result = am_SD_send_msg(host, cmd, sizeof(cmd), SD_DFT_WAIT_T, SD_CTL_CMD);
	if(result != SD_GOOD)
	{
		printk("%s ERROR = %d\n", __func__, result);
		reset_ctller(host);
		return result;
	}

	DBG_PRT(SD_DEBUG, "%s 0x%x OK\n", __func__,act_readl(CRC_STATUS));
	
	return result;
}



static int am_SD_send_clock_cmd(struct am_mci *host, u8 ctl_clock)
{
	unsigned char cmd[2];
	int result = -EILSEQ;

	cmd[0] = (unsigned char) (SD_SWITCH_CTL_CLK & 0xff);
	cmd[1] = ctl_clock;

	result = am_SD_send_msg(host, cmd, sizeof(cmd), SD_DFT_WAIT_T, SD_CTL_CMD);
	if(result != SD_GOOD)
	{
		printk("%s ERROR = %d\n", __func__, result);
		reset_ctller(host);
		return result;
	}

	//DBG_PRT(SD_DEBUG, "%s 0x%x OK\n", __func__,act_readl(CRC_STATUS));
    //  reset_ctller(host);
	return result;
}



static int am_SD_CLR_INT_CMD(struct am_mci *host)
{
	unsigned char cmd[2];
	int result = -EILSEQ;

	cmd[0] = (unsigned char) (SDIO_CLR_INT_REG & 0xff);
	cmd[1] = (unsigned char) ((SDIO_CLR_INT_REG >> 8) & 0xFF);

	result = am_SD_send_msg(host, cmd, sizeof(cmd), SD_DFT_WAIT_T, SD_CTL_CMD);
	if(result != SD_GOOD)
	{
		printk("%s ERROR = %d\n", __func__, result);
		reset_ctller(host);
		return result;
	}
    return result;
}





static int am_SD_send_cmd(struct am_mci *host, u8 cmd_idx, u32 cmd_arg, u8 rsp_type, u8 *rsp_buf)
{
	int result = -EILSEQ, rece_len = 0;
	unsigned char cmd[8];
	unsigned char rece_buf[20];



	


	cmd[0] = (unsigned char)(SD_SEND_CMD & 0xFF);
	cmd[1] = (unsigned char)((SD_SEND_CMD >> 8) & 0xFF);
	cmd[2] = cmd_idx | SD_CMD_HDR;
	cmd[3] = (unsigned char)((cmd_arg >> 24) & 0xFF);
	cmd[4] = (unsigned char)((cmd_arg >> 16) & 0xFF);
	cmd[5] = (unsigned char)((cmd_arg >> 8) & 0xFF);
	cmd[6] = (unsigned char)(cmd_arg & 0xFF);
	cmd[7] = rsp_type;

    

	//Send msg to control
	result = am_SD_send_msg(host, cmd, sizeof(cmd), SD_DFT_WAIT_T, SD_CTL_CMD);
	if(result != SD_GOOD)
	{
		printk("%s Send ERROR = %d 0x%x 0x%x\n", __func__, result,cmd_idx,cmd_arg);
		reset_ctller(host);
		return result;
	}

	//Receive msg from control
	if((rsp_type & SD_RSP_MASK) != SD_RSP_MODE_R0)
	{
		//get response data
		switch(rsp_type & SD_RSP_MASK)
		{
		case SD_RSP_MODE_R1:
		case SD_RSP_MODE_R1b:
		case SD_RSP_MODE_R3:
			rece_len = 6;
			break;
		case SD_RSP_MODE_R2:
			rece_len = 17 - 1;
			break;
		default:
			printk("ERROR: Unknown response type 0x%x\n",rsp_type);
		}

		result = am_SD_rece_msg(host, rece_buf, rece_len, SD_DFT_WAIT_T);
		if(result != SD_GOOD)
		{
			printk("%s Receive ERROR = %d 0x%x 0x%x\n", __func__, result,cmd_idx,cmd_arg);
			reset_ctller(host);
			return result;
		}

		//Parse the rece_buf
		if(rece_len == 6){
			//If the response is 6 bytes, response data has 6 bytes and 0, 1, 2, 3, 5 byte is useful but 5th byte is dummy byte
			//lst = cmd_idx or check bits, it don't need to copy into rsp_buf
			rsp_buf[0] = rece_buf[5];
			rsp_buf[1] = rece_buf[3];
			rsp_buf[2] = rece_buf[2];
			rsp_buf[3] = rece_buf[1];

            
		}else if(rece_len == 16){
			//If the response is 17 bytes the response data has 16 bytes.
			//rsp_buf should be 127bit, 126bit ... 0bit.
			//Our controller only receive the first 16bytes, it mean bit7~bit0 won't receive and the 0 bytes is 0b00111111 (0x3f
			rsp_buf[0] = rece_buf[4];
			rsp_buf[1] = rece_buf[3];
			rsp_buf[2] = rece_buf[2];
			rsp_buf[3] = rece_buf[1];
			rsp_buf[4] = rece_buf[8];
			rsp_buf[5] = rece_buf[7];
			rsp_buf[6] = rece_buf[6];
			rsp_buf[7] = rece_buf[5];
			rsp_buf[8] = rece_buf[12];
			rsp_buf[9] = rece_buf[11];
			rsp_buf[10] = rece_buf[10];
			rsp_buf[11] = rece_buf[9];
			rsp_buf[12] = rece_buf[16]; //it is dummy and not correct.
			rsp_buf[13] = rece_buf[15];
			rsp_buf[14] = rece_buf[14];
			rsp_buf[15] = rece_buf[13];
            
		}

	}
    else
    {
      //  reset_ctller(host);        
    }

	//Check the SD controller status
	//result = Check_Reg(host, 1);

	return result;
}

void pin_conf(void)
{

#if CONFIG_AM_CHIP_ID == 1213
#if 0

	//MFP set CMD pin
    temp_reg = readl((void __iomem *)GPIO_MFCTL2);
	temp_reg = (temp_reg & ~(7 << 26)) | (2 << 26);
	writel(temp_reg, (void __iomem *)GPIO_MFCTL2);
	//MFP set CLK and Data pin
	temp_reg = readl((void __iomem *)GPIO_MFCTL3);
	temp_reg = (temp_reg & ~(7 << 2) & ~(7 << 7)) | (2 << 2) | (2 << 7);
	writel(temp_reg, (void __iomem *)GPIO_MFCTL3);
    #endif
#elif CONFIG_AM_CHIP_ID == 1215
	//MFP set CMD CLK pin
	temp_reg = readl((void __iomem *)GPIO_MFCTL1);
	temp_reg = (temp_reg & ~(7 << 26)) | (2 << 26);
	//MFP set Data pin
	temp_reg = readl((void __iomem *)GPIO_MFCTL2);
	temp_reg = (temp_reg & ~(7 << 1) & ~(7 << 4)) | (2 << 1) | (2 << 4);
#endif
}

static void am_sd_init_clk_pin(void)
{
    unsigned int mfp; 

    /*config mfp*/
    mfp=act_readl(GPIO_MFCTL2);
    act_writel((mfp&SD_MF2_MSK_8250)|SD_MF2_VAL_8250,GPIO_MFCTL2);
    /*config clk*/
    act_writel(act_readl(CMU_DEVCLKEN)| (1<<3), CMU_DEVCLKEN);	
    act_writel(((1<<2)|(3<<4)|(4<<8)|(5<<12)|(7<<16)),CMU_CARDCLKSEL);
    act_writel(0x17,CMU_CARDCLK);


   
}
int __mmc_io_rw_direct(struct am_mci *host, int write, unsigned fn,
	unsigned addr, u8 in, u8* out)
{
    u32			opcode=0;
	u32			arg=0;
    u32         rsp_type=0;
    u8          rsp_buf[4];
    int err;
    opcode = 52;
	arg = write ? 0x80000000 : 0x00000000;
	arg |= fn << 28;
	arg |= (write && out) ? 0x08000000 : 0x00000000;
	arg |= addr << 9;
	arg |= in;
	rsp_type = 0x5;
    host->SD_mode =  SD_IN_CHECK;
	err = am_SD_send_cmd(host,opcode,arg,rsp_type,rsp_buf);
    host->SD_mode =  SD_IN_FREE;
    if(err)
        return -EINVAL;
    if(out)
        *out=rsp_buf[0];
    return 0;
}

#ifdef CONFIG_MMC_ACTIONS_DMA
static int am_SD_autowrite2_dma(struct am_mci *host, struct mmc_request *req)
{
	int result = -EILSEQ, temp_reg, dma_mode = 0, i, j;
	unsigned char cmd[12], status[2];
	unsigned int blksize = req->data->blksz; 
	unsigned int blk = req->data->blocks;
	u8 cmd_idx = req->cmd->opcode;
	u32 cmd_arg = req->cmd->arg;

	//Merge sg
	DBG_PRT(SD_DEBUG_W, "Write: dma_sg_len = %d DMA_LENGTH = 0x%x PDMA_ADD = 0x%x VDMA_ADD = 0x%x cmd_arg = 0x%x\n", \
		host->dma_sg_len, sg_dma_len(req->data->sg), sg_dma_address(req->data->sg), (unsigned int)sg_virt(req->data->sg), cmd_arg+SD_START_ADD);
	host->dma_dir = DMA_TO_DEVICE;
	host->dma_sg_len = dma_map_sg(mmc_dev(host->mmc), req->data->sg, req->data->sg_len, host->dma_dir);
	DBG_PRT(SD_DEBUG_W, "Write: dma_sg_len = %d DMA_LENGTH = 0x%x PDMA_ADD = 0x%x VDMA_ADD = 0x%x cmd_arg = 0x%x\n", \
		host->dma_sg_len, sg_dma_len(req->data->sg), sg_dma_address(req->data->sg), (unsigned int)sg_virt(req->data->sg), cmd_arg+SD_START_ADD);

	if(host->dma_sg_len > 1)
	{
		printk("###\nSG LEN is more than 1, our chip doesn't support\n###\n");
		return result;
	}

	cmd_arg = cmd_arg+SD_START_ADD;

	cmd[0] = (unsigned char)(SD_AUTOWRITE2 & 0xff);
	cmd[1] = (unsigned char)((SD_AUTOWRITE2 >> 8) & 0xff);
	cmd[2] = cmd_idx | SD_CMD_HDR;
	cmd[3] = (unsigned char)((cmd_arg >> 24) & 0xFF);
	cmd[4] = (unsigned char)((cmd_arg >> 16) & 0xFF);
	cmd[5] = (unsigned char)((cmd_arg >> 8) & 0xFF);
	cmd[6] = (unsigned char)(cmd_arg & 0xFF);
	cmd[7] = SD_RSP_MODE_R1 | CHECK_CRC16;
	cmd[8] = (unsigned char)(blksize & 0xFF);
	cmd[9] = (unsigned char)(blksize >> 8);
	cmd[10] = (unsigned char)(blk & 0xFF);
	cmd[11] = (unsigned char)(blk >> 8);

	//Send msg to control
	req->cmd->error = am_SD_send_msg(host, cmd, sizeof(cmd), SD_DFT_WAIT_T, SD_CTL_CMD);
	if(req->cmd->error != SD_GOOD)
	{
		printk("%s Send ERROR = %d\n", __func__, result);
		reset_ctller(host);
		return result;
	}

	DBG_PRT(SD_DEBUG_W, "Write data with DMA. TL = 0x%x, src_buf = 0x%x\n", sg_dma_len(req->data->sg), sg_dma_address(req->data->sg));

	//Write data
	for(i=0;i<blk;i++)
	{
		//Set SD to write data
		temp_reg = (SD_CTL_DATA | SD_CTL_WR) | (blksize & SD_CTL_MAX_LEN);
		writel(temp_reg, host->reg_base + SD_CTL_REG);

		/*//Using 4
		writel(SD_DRQ_4_MOD ,host->reg_base + SD_DRQ_SET_REG);
		dma_mode = DMA_DST_BST_SINGLE | DMA_DST_FIX_ADDR | (DMA_TRIG_SD << DMA_DST_DRQ_BIT) |\
			   DMA_SRC_BST_INCR4 | DMA_SRC_INC_ADDR | (DMA_TRIG_SDRAM << DMA_SRC_DRQ_BIT);
		*/
		//We will use burst 1 because our internal bus is more fast than SD conrtroller
		writel(SD_DRQ_1_MOD ,host->reg_base + SD_DRQ_SET_REG);
		dma_mode = DMA_DST_BST_SINGLE | DMA_DST_FIX_ADDR | (DMA_TRIG_SD << DMA_DST_DRQ_BIT) |\
			   DMA_SRC_BST_SINGLE | DMA_SRC_INC_ADDR | (DMA_TRIG_SDRAM << DMA_SRC_DRQ_BIT);

		//Start transfer
		am_dma_cmd(host->dma_cn, DMA_CMD_RESET);
		am_set_dma_addr(host->dma_cn, sg_dma_address(req->data->sg) + blksize*i, SD_REG_BASE + SD_DATA_REG);
		am_dma_config(host->dma_cn, dma_mode, blksize);
		result = am_dma_start(host->dma_cn, 0);         //We only check in start. The better way is to rewrite the dma function call.

		//Wait stop
		for(j = 0; j <= 1000000; j++)
		{
			if(!am_dma_cmd(host->dma_cn, DMA_CMD_QUERY))
			{
				break;
			}else if(j == 1000000){
				printk("%s Error dma timeout %d\n", __func__, i);
				reset_ctller(host);
				return -ETIMEDOUT;
			}else{
				udelay(2); //10us
			}
		}

		//check transfer done.
		while((readl(host->reg_base + SD_STATUS_REG) & 0x01) == 0x0)
		{
			DBG_PRT(SD_DEBUG_W, "%s SD_STATUS = 0x%x %d\n", __func__, act_readl(SD_STATUS), i);
		}
	}

	//Check the status
	result = am_SD_rece_msg(host, status, 2, 1);
	DBG_PRT(SD_DEBUG_W, "Check register status for writing data, status = 0x%x, 0x%x\n", status[0], status[1]);
	if((result != SD_GOOD) || (status[0] & 0x04))
	{
		printk("%s check status error, status = 0x%x, 0x%x\n", __func__, status[0], status[1]);
		reset_ctller(host);
		return result;
	}

	return result;
}

static int am_SD_autoread2_dma(struct am_mci *host, struct mmc_request *req)
{
	int result = -EILSEQ, temp_reg, dma_mode = 0, i, j;
	unsigned char cmd[12];
	unsigned int blksize = req->data->blksz; 
	unsigned int blk = req->data->blocks;
	u8 cmd_idx = req->cmd->opcode;
	u32 cmd_arg = req->cmd->arg;

	//Merge sg
	DBG_PRT(SD_DEBUG_R, "Read: dma_sg_len = %d DMA_LENGTH = 0x%x PDMA_ADD = 0x%x VDMA_ADD = 0x%x cmd_arg = 0x%x\n", \
		host->dma_sg_len, sg_dma_len(req->data->sg), sg_dma_address(req->data->sg), (unsigned int)sg_virt(req->data->sg), cmd_arg+SD_START_ADD);
	host->dma_dir = DMA_FROM_DEVICE;
	host->dma_sg_len = dma_map_sg(mmc_dev(host->mmc), req->data->sg, req->data->sg_len, host->dma_dir);
	DBG_PRT(SD_DEBUG_R, "Read: dma_sg_len = %d DMA_LENGTH = 0x%x PDMA_ADD = 0x%x VDMA_ADD = 0x%x cmd_arg = 0x%x\n", \
		host->dma_sg_len, sg_dma_len(req->data->sg), sg_dma_address(req->data->sg), (unsigned int)sg_virt(req->data->sg), cmd_arg+SD_START_ADD);
	
	if(host->dma_sg_len > 1)
	{
		printk("###\nSG LEN is more than 1, our chip doesn't support\n###\n");
		return result;
	}

	cmd_arg = cmd_arg+SD_START_ADD;

	cmd[0] = (unsigned char)(SD_AUTOREAD2 & 0xff);
	cmd[1] = (unsigned char)((SD_AUTOREAD2 >> 8) & 0xff);
	cmd[2] = cmd_idx | SD_CMD_HDR;
	cmd[3] = (unsigned char)((cmd_arg >> 24) & 0xFF);
	cmd[4] = (unsigned char)((cmd_arg >> 16) & 0xFF);
	cmd[5] = (unsigned char)((cmd_arg >> 8) & 0xFF);
	cmd[6] = (unsigned char)(cmd_arg & 0xFF);
	cmd[7] = SD_RSP_MODE_R1 | CHECK_CRC16;
	cmd[8] = (unsigned char)(blksize & 0xFF);
	cmd[9] = (unsigned char)(blksize >> 8);
	cmd[10] = (unsigned char)(blk & 0xFF);
	cmd[11] = (unsigned char)(blk >> 8);

	//Send msg to control
	req->cmd->error = am_SD_send_msg(host, cmd, sizeof(cmd), SD_DFT_WAIT_T, SD_CTL_CMD);
	if(req->cmd->error != SD_GOOD)
	{
		printk("%s Send ERROR = %d\n", __func__, result);
		reset_ctller(host);
		return result;
	}

	DBG_PRT(SD_DEBUG_R, "Read data with DMA. TL = 0x%x, dst_buf = 0x%x\n", sg_dma_len(req->data->sg), sg_dma_address(req->data->sg));

	//Block Read
	for(i=0;i<blk;i++)
	{
		temp_reg = (SD_CTL_DATA | SD_CTL_RD) | (blksize & SD_CTL_MAX_LEN);
		writel(temp_reg, host->reg_base + SD_CTL_REG);

		/*//Using 4
		writel(SD_DRQ_4_MOD ,host->reg_base + SD_DRQ_SET_REG);
		dma_mode = DMA_SRC_BST_INCR4 | DMA_DST_INC_ADDR | (DMA_TRIG_SDRAM << DMA_DST_DRQ_BIT) |\
			   DMA_SRC_BST_SINGLE | DMA_SRC_FIX_ADDR | (DMA_TRIG_SD << DMA_SRC_DRQ_BIT);
		*/
		//We will use burst 1 because our internal bus is more fast than SD conrtroller
		writel(SD_DRQ_1_MOD ,host->reg_base + SD_DRQ_SET_REG);
		dma_mode = DMA_DST_BST_SINGLE | DMA_DST_INC_ADDR | (DMA_TRIG_SDRAM << DMA_DST_DRQ_BIT) |\
			   DMA_SRC_BST_SINGLE | DMA_SRC_FIX_ADDR | (DMA_TRIG_SD << DMA_SRC_DRQ_BIT);

		//Start transfer
		am_dma_cmd(host->dma_cn, DMA_CMD_RESET);
		am_set_dma_addr(host->dma_cn, SD_REG_BASE + SD_DATA_REG, sg_dma_address(req->data->sg) + blksize*i );
		am_dma_config(host->dma_cn, dma_mode, blksize);
		result = am_dma_start(host->dma_cn, 0);		//We only check in start. The better way is to rewrite the dma function call.

		//Wait stop
		for(j = 0; j <= 1000000; j++)
		{
			if(!am_dma_cmd(host->dma_cn, DMA_CMD_QUERY))
			{
				break;
			}else if(j == 1000000){
				printk("%s Error dma timeout %d\n", __func__, i);
				reset_ctller(host);
				return -ETIMEDOUT;
			}else{
				udelay(2); //10us
			}
		}
	}

	return result;
}
#else
static int am_SD_autowrite2(struct am_mci *host, struct mmc_request *req)
{
	int result = -EILSEQ;
	unsigned char cmd[12], status[2];
	unsigned int blksize = req->data->blksz; 
	unsigned int blk = req->data->blocks;
	u8 *src_buf = sg_virt(req->data->sg); 
	u8 cmd_idx = req->cmd->opcode;
	u32 cmd_arg = req->cmd->arg;

	//In cpu mode, we don't use dma_map_sg/dma_unmap_sg. If we use dma_map_sg/dma_unmap_sg, the data will be error.
	host->dma_dir = DMA_TO_DEVICE;
	host->dma_sg_len = req->data->sg_len;
	DBG_PRT(SD_DEBUG_W, "Write: dma_sg_len = %d DMA_LENGTH = 0x%x PDMA_ADD = 0x%x VDMA_ADD = 0x%x cmd_arg = 0x%x\n ", \
		host->dma_sg_len, sg_dma_len(req->data->sg), sg_dma_address(req->data->sg), (unsigned int)sg_virt(req->data->sg), cmd_arg+SD_START_ADD);

	if(host->dma_sg_len > 1)
	{
		printk("###\nSG LEN is more than 1, our chip doesn't support\n###\n");
		return result;
	}


    if(blksize*blk >512 && (blksize*blk&511))
        printk("am_SD_autowrite2:error Not support this block! %d\n",blksize*blk);
    

	cmd_arg = cmd_arg+SD_START_ADD;

	cmd[0] = (unsigned char)(SD_AUTOWRITE2 & 0xff);
	cmd[1] = (unsigned char)((SD_AUTOWRITE2 >> 8) & 0xff);
	cmd[2] = cmd_idx | SD_CMD_HDR;
	cmd[3] = (unsigned char)((cmd_arg >> 24) & 0xFF);
	cmd[4] = (unsigned char)((cmd_arg >> 16) & 0xFF);
	cmd[5] = (unsigned char)((cmd_arg >> 8) & 0xFF);
	cmd[6] = (unsigned char)(cmd_arg & 0xFF);
	cmd[7] = SD_RSP_MODE_R1 | CHECK_CRC16;
	cmd[8] = (unsigned char)(blksize & 0xFF);
	cmd[9] = (unsigned char)(blksize >> 8);
	cmd[10] = (unsigned char)(blk & 0xFF);
	cmd[11] = (unsigned char)(blk >> 8);


#if 0
    if(blksize*blk>512)
        printk("am_SD_autowrite2:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
            cmd[2],cmd[3],cmd[4],cmd[5],cmd[6],blksize,blk);
#endif   

    //Send msg to control
	req->cmd->error = am_SD_send_msg(host, cmd, sizeof(cmd), SD_DFT_WAIT_T, SD_CTL_CMD);
	if(req->cmd->error != SD_GOOD)
	{
		printk("%s Send ERROR = %d\n", __func__, result);
		reset_ctller(host);
		return result;
	}

	//Write data
	result = am_SD_send_msg(host, src_buf, blksize*blk, blk*2, SD_CTL_DATA);
	if(result != SD_GOOD)
	{
		printk("%s Send ERROR = %d\n", __func__, result);
		reset_ctller(host);
		return result;
	}

	//Check the status
	result = fast_am_SD_rece_msg(host, status, 2, 1);
    
	DBG_PRT(SD_DEBUG_W, "Check register status for writing data, status = 0x%x, 0x%x\n", status[0], status[1]);
	if(result != SD_GOOD)
	{
		//printk("%s check status error, status = 0x%x, 0x%x\n", __func__, status[0], status[1]);
		reset_ctller(host);
        //__mmc_io_rw_direct(host,1,0,6,1,NULL);
		return result;
	}

	return result;
}


static int am_SD_autowrite2_dma_hw(struct am_mci *host, struct mmc_request *req)
{
	int result = -EILSEQ;
	unsigned char  status[2];
	unsigned int blksize = req->data->blksz; 
	unsigned int blk = req->data->blocks;
	
    
	u8 cmd_idx = req->cmd->opcode;
	u32 cmd_arg = req->cmd->arg;
    unsigned char *cmd=host->adma_virt;


    host->dma_dir = DMA_TO_DEVICE;
    host->dma_sg_len = dma_map_sg(mmc_dev(host->mmc), req->data->sg, req->data->sg_len, host->dma_dir);
    if(host->dma_sg_len>1)
        printk("%s %d not support 0x%x\n",__func__,__LINE__,host->dma_sg_len);

    if(blksize*blk >512 && (blksize*blk&511))
        printk("am_SD_autowrite2:error Not support this block! %d\n",blksize*blk);
    

	cmd_arg = cmd_arg+SD_START_ADD;

	cmd[0] = (unsigned char)(SD_AUTOWRITE2 & 0xff);
	cmd[1] = (unsigned char)((SD_AUTOWRITE2 >> 8) & 0xff);
	cmd[2] = cmd_idx | SD_CMD_HDR;
	cmd[3] = (unsigned char)((cmd_arg >> 24) & 0xFF);
	cmd[4] = (unsigned char)((cmd_arg >> 16) & 0xFF);
	cmd[5] = (unsigned char)((cmd_arg >> 8) & 0xFF);
	cmd[6] = (unsigned char)(cmd_arg & 0xFF);
	cmd[7] = SD_RSP_MODE_R1 | CHECK_CRC16;
	cmd[8] = (unsigned char)(blksize & 0xFF);
	cmd[9] = (unsigned char)(blksize >> 8);
	cmd[10] = (unsigned char)(blk & 0xFF);
	cmd[11] = (unsigned char)(blk >> 8);


    req->cmd->error = am_SD_send_msg_dma(host, (u8 *)host->adma_phy,12, SD_DFT_WAIT_T, SD_CTL_CMD);
    if(req->cmd->error != SD_GOOD)
	{
		printk("%s Send ERROR = %d\n", __func__, result);
		reset_ctller(host);
		return result;
	}

    
    
    result = am_SD_send_msg_dma(host, (u8 *)sg_dma_address(&(req->data->sg[0])), sg_dma_len(&(req->data->sg[0])), blk*2, SD_CTL_DATA);
    if(result != SD_GOOD)
	{
		printk("%s Send ERROR = %d\n", __func__, result);
		reset_ctller(host);
		return result;
	}

    

	//Check the status
	result = fast_am_SD_rece_msg(host, status, 2, 1);
    
	DBG_PRT(SD_DEBUG_W, "Check register status for writing data, status = 0x%x, 0x%x\n", status[0], status[1]);
	if(result != SD_GOOD)
	{
        reset_ctller(host);
        return result;
	}

	return result;
}
#if 0
static int am_SD_NormalRead(struct am_mci *host, struct mmc_request *req)
{
	int result = -EILSEQ;
	unsigned char cmd[12];
	unsigned int blksize = req->data->blksz; 
	unsigned int blk = req->data->blocks;
	u8 *dst_buf = sg_virt(req->data->sg); 
	u8 cmd_idx = req->cmd->opcode;
	u32 cmd_arg = req->cmd->arg;
    u32 i;
    u32 * p;
    u32 base;
    u32 byteCnt;
    u32 new_arg;
    u32 size;

	//In cpu mode, we don't use dma_map_sg/dma_unmap_sg. If we use dma_map_sg/dma_unmap_sg, the data will be error.
	host->dma_dir = DMA_FROM_DEVICE;
	host->dma_sg_len = req->data->sg_len;
	DBG_PRT(SD_DEBUG_R, "Read: dma_sg_len = %d DMA_LENGTH = 0x%x PDMA_ADD = 0x%x VDMA_ADD = 0x%x cmd_arg = 0x%x\n", \
		host->dma_sg_len, sg_dma_len(req->data->sg), sg_dma_address(req->data->sg), (unsigned int)sg_virt(req->data->sg), cmd_arg+SD_START_ADD);
	
	if(host->dma_sg_len > 1)
	{
		printk("###\nSG LEN is more than 1, our chip doesn't support\n###\n");
		return result;
	}

    
    blksize = blksize*blk;
    size = blksize;
    blk = 1;

    base = (cmd_arg>>9)&0x0001ffff;
    while(size>0)
    {
        if(size>512)
            byteCnt = 512;
        else
            byteCnt = size;
        size -= byteCnt;

        new_arg = (cmd_arg&0xf0000000)|(cmd_arg&0x04000000)|(base<<9)|(byteCnt&0x1ff);
        cmd[0] = (unsigned char)(SD_NORMALREAD & 0xff);
        cmd[1] = (unsigned char)((SD_NORMALREAD >> 8) & 0xff);
        cmd[2] = cmd_idx | SD_CMD_HDR;
        cmd[3] = (unsigned char)((new_arg >> 24) & 0xFF);
        cmd[4] = (unsigned char)((new_arg >> 16) & 0xFF);
        cmd[5] = (unsigned char)((new_arg >> 8) & 0xFF);
        cmd[6] = (unsigned char)(new_arg & 0xFF);
        cmd[7] = SD_RSP_MODE_R1 | CHECK_CRC16;//|DIVIDE_SCHEME1;
        cmd[8] = (unsigned char)(byteCnt & 0xFF);
        cmd[9] = (unsigned char)(byteCnt >> 8);
        cmd[10] = (unsigned char)(1 & 0xFF);
        cmd[11] = (unsigned char)(1 >> 8);   

        if(cmd_arg&0x04000000)
        {
            base += byteCnt;
        }

        if(cmd_arg != new_arg)
            printk("cmd_arg:0x%x,new arg:0x%x\n",cmd_arg,new_arg);
    
        //Send msg to control
    	req->cmd->error = am_SD_send_msg(host, cmd, sizeof(cmd), SD_DFT_WAIT_T, SD_CTL_CMD);
    	if(req->cmd->error != SD_GOOD)
    	{
    		printk("%s Send ERROR = %d\n", __func__, result);
    		reset_ctller(host);
    		return result;
    	}

    	//read data
    	result = am_SD_rece_msg(host, (u8 *)dst_buf, byteCnt, blk*2);
    	if(result != SD_GOOD)
    	{
    		printk("%s Receive ERROR = %d\n", __func__, result);
    		reset_ctller(host);
    		return result;
    	}
        dst_buf +=byteCnt;


   

        //Check the SD controller status
        //result = Check_Reg(host, 1);
    }
	return result;
}

#else
static int am_SD_NormalRead(struct am_mci *host, struct mmc_request *req)
{
	int result = -EILSEQ;
	unsigned char cmd[12];
	unsigned int blksize = req->data->blksz; 
	unsigned int blk = req->data->blocks;
	u8 *dst_buf = sg_virt(req->data->sg); 
	u8 cmd_idx = req->cmd->opcode;
	u32 cmd_arg = req->cmd->arg;

	//In cpu mode, we don't use dma_map_sg/dma_unmap_sg. If we use dma_map_sg/dma_unmap_sg, the data will be error.
	host->dma_dir = DMA_FROM_DEVICE;
	host->dma_sg_len = req->data->sg_len;
	DBG_PRT(SD_DEBUG_R, "Read: dma_sg_len = %d DMA_LENGTH = 0x%x PDMA_ADD = 0x%x VDMA_ADD = 0x%x cmd_arg = 0x%x\n", \
		host->dma_sg_len, sg_dma_len(req->data->sg), sg_dma_address(req->data->sg), (unsigned int)sg_virt(req->data->sg), cmd_arg+SD_START_ADD);
	
	if(host->dma_sg_len > 1)
	{
		printk("###\nSG LEN is more than 1, our chip doesn't support\n###\n");
		return result;
	}

    if(blksize*blk >512 && (blksize*blk&511))
        printk("am_SD_NormalRead:error Not support this block! %d\n",blksize*blk);
   
    


	cmd_arg = cmd_arg+SD_START_ADD;

	cmd[0] = (unsigned char)(SD_NORMALREAD & 0xff);
	cmd[1] = (unsigned char)((SD_NORMALREAD >> 8) & 0xff);
	cmd[2] = cmd_idx | SD_CMD_HDR;
	cmd[3] = (unsigned char)((cmd_arg >> 24) & 0xFF);
	cmd[4] = (unsigned char)((cmd_arg >> 16) & 0xFF);
	cmd[5] = (unsigned char)((cmd_arg >> 8) & 0xFF);
	cmd[6] = (unsigned char)(cmd_arg & 0xFF);
	cmd[7] = SD_RSP_MODE_R1 | CHECK_CRC16;//|DIVIDE_SCHEME1;
    cmd[8] = (unsigned char)(blksize & 0xFF);
	cmd[9] = (unsigned char)(blksize >> 8);
	cmd[10] = (unsigned char)(blk & 0xFF);
	cmd[11] = (unsigned char)(blk >> 8);


    
 //   printk("am_SD_autoread2 %d The send cmd is :0x%x 0x%x 0x%x 0x%x !\n",__LINE__,cmd[0],cmd[1],cmd[2],cmd[3]);
 //   printk("am_SD_autoread2 %d The send cmd is :0x%x 0x%x 0x%x 0x%x !\n",__LINE__,cmd[4],cmd[5],cmd[6],cmd[7]);
 //   printk("am_SD_autoread2 %d The send cmd is :0x%x 0x%x 0x%x 0x%x !\n",__LINE__,cmd[8],cmd[9],cmd[10],cmd[11]);

	//Send msg to control
	req->cmd->error = am_SD_send_msg(host, cmd, sizeof(cmd), SD_DFT_WAIT_T, SD_CTL_CMD);
	if(req->cmd->error != SD_GOOD)
	{
		printk("%s Send ERROR = %d\n", __func__, result);
		reset_ctller(host);
		return result;
	}

	//read data
	result = am_SD_rece_msg(host, (u8 *)dst_buf, blksize*blk, blk*2);
	if(result != SD_GOOD)
	{
		printk("%s Receive ERROR = %d\n", __func__, result);
		reset_ctller(host);
		return result;
	}

        //Check the SD controller status
        //result = Check_Reg(host, 1);

	return result;
}

#endif





static int am_SD_autoread2(struct am_mci *host, struct mmc_request *req)
{
	int result = -EILSEQ;
	unsigned char cmd[12];
	unsigned int blksize = req->data->blksz; 
	unsigned int blk = req->data->blocks;
	u8 *dst_buf = sg_virt(req->data->sg); 
	u8 cmd_idx = req->cmd->opcode;
	u32 cmd_arg = req->cmd->arg;

	//In cpu mode, we don't use dma_map_sg/dma_unmap_sg. If we use dma_map_sg/dma_unmap_sg, the data will be error.
	host->dma_dir = DMA_FROM_DEVICE;
	host->dma_sg_len = req->data->sg_len;
	DBG_PRT(SD_DEBUG_R, "Read: dma_sg_len = %d DMA_LENGTH = 0x%x PDMA_ADD = 0x%x VDMA_ADD = 0x%x cmd_arg = 0x%x\n", \
		host->dma_sg_len, sg_dma_len(req->data->sg), sg_dma_address(req->data->sg), (unsigned int)sg_virt(req->data->sg), cmd_arg+SD_START_ADD);
	
	if(host->dma_sg_len > 1)
	{
		printk("###\nSG LEN is more than 1, our chip doesn't support\n###\n");
		return result;
	}

	cmd_arg = cmd_arg+SD_START_ADD;

	cmd[0] = (unsigned char)(SD_AUTOREAD2 & 0xff);
	cmd[1] = (unsigned char)((SD_AUTOREAD2 >> 8) & 0xff);
	cmd[2] = cmd_idx | SD_CMD_HDR;
	cmd[3] = (unsigned char)((cmd_arg >> 24) & 0xFF);
	cmd[4] = (unsigned char)((cmd_arg >> 16) & 0xFF);
	cmd[5] = (unsigned char)((cmd_arg >> 8) & 0xFF);
	cmd[6] = (unsigned char)(cmd_arg & 0xFF);
	cmd[7] = SD_RSP_MODE_R1 | CHECK_CRC16;//|DIVIDE_SCHEME1;
	cmd[8] = (unsigned char)(blksize & 0xFF);
	cmd[9] = (unsigned char)(blksize >> 8);
	cmd[10] = (unsigned char)(blk & 0xFF);
	cmd[11] = (unsigned char)(blk >> 8);


 //   printk("am_SD_autoread2 %d The send cmd is :0x%x 0x%x 0x%x 0x%x !\n",__LINE__,cmd[0],cmd[1],cmd[2],cmd[3]);
 //   printk("am_SD_autoread2 %d The send cmd is :0x%x 0x%x 0x%x 0x%x !\n",__LINE__,cmd[4],cmd[5],cmd[6],cmd[7]);
 //   printk("am_SD_autoread2 %d The send cmd is :0x%x 0x%x 0x%x 0x%x !\n",__LINE__,cmd[8],cmd[9],cmd[10],cmd[11]);

	//Send msg to control
	req->cmd->error = am_SD_send_msg(host, cmd, sizeof(cmd), SD_DFT_WAIT_T, SD_CTL_CMD);
	if(req->cmd->error != SD_GOOD)
	{
		printk("%s Send ERROR = %d\n", __func__, result);
		reset_ctller(host);
		return result;
	}

	//read data
	result = am_SD_rece_msg(host, (u8 *)dst_buf, blksize*blk, blk*2);
	if(result != SD_GOOD)
	{
		printk("%s Receive ERROR = %d\n", __func__, result);
		reset_ctller(host);
		return result;
	}

        //Check the SD controller status
        //result = Check_Reg(host, 1);

	return result;
}
#endif
static int am_SD_autoread3(struct am_mci *host, struct mmc_request *req)
{
	int result = -EILSEQ;
	unsigned char cmd[6];
	unsigned int blksize = req->data->blksz; 
	unsigned int blk = req->data->blocks;
	u8 *dst_buf = sg_virt(req->data->sg);

	//In cpu mode, we don't use dma_map_sg/dma_unmap_sg. If we use dma_map_sg/dma_unmap_sg, the data will be error.
	host->dma_dir = DMA_FROM_DEVICE;
	host->dma_sg_len = req->data->sg_len;
	DBG_PRT(SD_DEBUG_R, "Read 3: dma_sg_len =%d DMA_LENGTH = 0x%x PDMA_ADD = 0x%x VDMA_ADD = 0x%x\n", \
		host->dma_sg_len, sg_dma_len(req->data->sg), sg_dma_address(req->data->sg), (unsigned int)sg_virt(req->data->sg));

	cmd[0] = (unsigned char)(SD_AUTOREAD3 & 0xFF);
	cmd[1] = (unsigned char)((SD_AUTOREAD3 >> 8) & 0xFF);
	cmd[2] = (unsigned char)(blksize & 0xFF);
	cmd[3] = (unsigned char)(blksize >> 8);
	cmd[4] = (unsigned char)(blk & 0xFF);
	cmd[5] = (unsigned char)(blk >> 8);

	//Send read cmd to controller
	result = am_SD_send_msg(host, cmd, sizeof(cmd), SD_DFT_WAIT_T, SD_CTL_CMD);
	if(result != SD_GOOD)
	{
		printk("%s Send ERROR = %d\n", __func__, result);
		reset_ctller(host);
		return result;
	}

	//read data
	result = am_SD_rece_msg(host, (u8 *)dst_buf, blksize*blk, blk*2);
	if(result != SD_GOOD)
	{
		printk("%s Receive ERROR = %d\n", __func__, result);
		reset_ctller(host);
		return result;
	}

	return result;
}

static irqreturn_t am_mmc_irq(int irq, void *dev_id)
{
	struct am_mci *host = dev_id;
    writel(4, host->reg_base + SD_IRQ_CLR_REG);
    mmc_signal_sdio_irq(host->mmc);         
    return IRQ_HANDLED;
}


#if 0
static void am_mmc_request(struct mmc_host *mmc, struct mmc_request *req)
{
	struct am_mci *host = mmc_priv(mmc);
	int i;
	u8 rsp_type;

	DBG_PRT(SD_DEBUG|SD_DEBUG_R|SD_DEBUG_W, "%s +++\ncmd = %d\n", __func__, req->cmd->opcode);


    printk("from core the cmd is :0x%x 0x%x\n",req->cmd->opcode,mmc_resp_type(req->cmd));

	//pin_conf(); //fuck
	//Check the CRC status

    if(host->SD_CTL_STATE == SD_CTL_IDLE)
    {
    	for(i=0;i<=10;i++)
    	{
    		if(readl(host->reg_base + SD_STATUS_REG) & SD_CRC_READY)
    		{
    			DBG_PRT(SD_DEBUG, "SD status = 0x%x, cmd = %d\n", readl(host->reg_base + SD_STATUS_REG), req->cmd->opcode);
    			break;
    		}else if(i == 10){
    			printk("ERROR: Staus is CRC BUSY(ERROR)\n");
    			reset_ctller(host);
    		}else{
    			DBG_PRT(SD_DEBUG, "WARNING: CRC BUSY and wait\n");
    			udelay(10);
    		}
    	}
    }

    switch(mmc_resp_type(req->cmd))
	{
    	case MMC_RSP_NONE:
    		rsp_type = SD_RSP_MODE_R0;
    		break;
    	case MMC_RSP_R1:
    		rsp_type = SD_RSP_MODE_R1;
    		break;
    	case MMC_RSP_R1B:
    		rsp_type = SD_RSP_MODE_R1b;
    		break;
    	case MMC_RSP_R2:
    		rsp_type = SD_RSP_MODE_R2;
    		break;
    	case MMC_RSP_R3:
    		rsp_type = SD_RSP_MODE_R4;
            break;
        default:
    		printk("unknown response = 0x%x\n", mmc_resp_type(req->cmd));
    		req->cmd->error = -EINVAL;
    		mmc_request_done(mmc, req);
    		return;
    		break;
	}

	if(req->cmd->opcode == CMD0)
	{
		host->SD_mode = SD_IN_IDLE;
	}

	//Check if the cmd needs to transfer data
	if(req->data == NULL)
	{
		if(host->SD_mode < SD_IN_STBY)
		{
			rsp_type = rsp_type | DIVIDE_SCHEME0;
		}

		//Check mode
		if((host->SD_mode == SD_IN_IDLE) && (req->cmd->opcode > CMD1)&&(req->cmd->opcode != CMD8)&&(req->cmd->opcode != CMD5)&&(req->cmd->opcode != CMD3))
		{
            printk("invalid cmd :0x%x,sd_mod:0x%x!\n",req->cmd->opcode,host->SD_mode);
            req->cmd->error = -EINVAL;
			mmc_request_done(mmc, req);
			return;
		}else if((host->SD_mode == SD_IN_IDLE) && (req->cmd->opcode == CMD1)){
			host->SD_mode = SD_IN_READY;
		}else if((host->SD_mode == SD_IN_READY) && (req->cmd->opcode == CMD2)){
			host->SD_mode = SD_IN_IDENT;
		}else if((host->SD_mode == SD_IN_IDLE) && (req->cmd->opcode == CMD3)){
			host->SD_mode = SD_IN_STBY;
		}else if((host->SD_mode == SD_IN_STBY) && (req->cmd->opcode == CMD7)){
			host->SD_mode = SD_IN_TRANS;
		}

		//Send cmd 
		req->cmd->error = am_SD_send_cmd(host, req->cmd->opcode, req->cmd->arg, rsp_type, (u8 *)req->cmd->resp);
		if(req->cmd->error != SD_GOOD)
		{
			printk("%s error = %d, cmd = %d arg = %d\n", __func__, req->cmd->error, req->cmd->opcode, req->cmd->arg);
		}
	}else{	//data transfer
		DBG_PRT(SD_DEBUG_R|SD_DEBUG_W, "%s sg_len = %d, blksz = %d, blocks = %d, flags = %x, bytes_xfered = %x\n",\
			__func__, req->data->sg_len, req->data->blksz, req->data->blocks, req->data->flags, req->data->bytes_xfered);
 
		if(req->cmd->opcode == CMD8)			//Read Ecsd
		{
			//Send cmd8 first
			req->cmd->error = am_SD_send_cmd(host, req->cmd->opcode, req->cmd->arg, rsp_type, (u8 *)req->cmd->resp);
			if(req->cmd->error != SD_GOOD)
			{
				printk("%s error = %d, cmd = %d arg = %d\n", __func__, req->cmd->error, req->cmd->opcode, req->cmd->arg);
			}

			//Read Data
			req->data->error = am_SD_autoread3(host, req);
			req->data->bytes_xfered = req->data->blksz*req->data->blocks;
		}else if(req->data->flags & MMC_DATA_READ){	//Read
			//Read Data
#ifdef CONFIG_MMC_ACTIONS_DMA
			req->data->error = am_SD_autoread2_dma(host, req); 
			dma_unmap_sg(mmc_dev(host->mmc), req->data->sg, host->dma_sg_len, host->dma_dir);
#else
			req->data->error = am_SD_autoread2(host, req);
#endif
			//flush_cache_all();
			req->data->bytes_xfered = req->data->blksz*req->data->blocks;
			//dma_unmap_sg(mmc_dev(host->mmc), req->data->sg, host->dma_sg_len, host->dma_dir);

			//Check if need to send stop
			if(req->stop)
			{
				rsp_type = SD_RSP_MODE_R1b | DIVIDE_SCHEME0;
				req->stop->error = am_SD_send_cmd(host, req->stop->opcode, req->stop->arg, rsp_type, (u8 *)req->stop->resp);
				if(req->stop->error != SD_GOOD)
				{
					printk("Stop read cmd error = %d\n", req->stop->error);
					reset_ctller(host);
				}
			}
		}else{						//Write
			//write Data
#ifdef CONFIG_MMC_ACTIONS_DMA
			req->data->error= am_SD_autowrite2_dma(host, req);
			dma_unmap_sg(mmc_dev(host->mmc), req->data->sg, host->dma_sg_len,host->dma_dir);
#else
			req->data->error= am_SD_autowrite2(host, req);
#endif
			req->data->bytes_xfered = req->data->blksz*req->data->blocks;

			//Check if need to send stop
			if(req->stop)
			{
				rsp_type = SD_RSP_MODE_R1b | DIVIDE_SCHEME0;
				req->stop->error = am_SD_send_cmd(host, req->stop->opcode, req->stop->arg, rsp_type, (u8 *)req->stop->resp);
				if(req->stop->error != SD_GOOD)
				{
					printk("Stop write cmd error = %d\n", req->stop->error);
					reset_ctller(host);
				}
			}
		}
	}

	mmc_request_done(mmc, req);

	DBG_PRT(SD_DEBUG|SD_DEBUG_R|SD_DEBUG_W, "%s ---\n", __func__);
}





#else







static void am_mmc_request(struct mmc_host *mmc, struct mmc_request *req)
{
	struct am_mci *host = mmc_priv(mmc);
	int i;
	u8 rsp_type;





 

	//DBG_PRT(SD_DEBUG|SD_DEBUG_R|SD_DEBUG_W, "%s +++\ncmd = %d\n", __func__, req->cmd->opcode);

    
   

	//pin_conf(); //fuck
	//Check the CRC status

    if(host->SD_CTL_STATE != SD_CTL_IDLE)
    {
    	for(i=0;i<=10;i++)
    	{
    		if(readl(host->reg_base + SD_STATUS_REG) & SD_CRC_READY)
    		{
    			DBG_PRT(SD_DEBUG, "SD status = 0x%x, cmd = %d\n", readl(host->reg_base + SD_STATUS_REG), req->cmd->opcode);
    			break;
    		}else if(i == 10){
    			printk("ERROR: Staus is CRC BUSY(ERROR)\n");
    			reset_ctller(host);
    		}else{
    			DBG_PRT(SD_DEBUG, "WARNING: CRC BUSY and wait\n");
    			udelay(10);
    		}
    	}
    }

    switch(mmc_resp_type(req->cmd))
	{
    	case MMC_RSP_NONE:
    		rsp_type = SD_RSP_MODE_R0;
    		break;
    	case MMC_RSP_R1:
    		rsp_type = SD_RSP_MODE_R1;
    		break;
    	case MMC_RSP_R1B:
    		rsp_type = SD_RSP_MODE_R1b;
    		break;
    	case MMC_RSP_R2:
    		rsp_type = SD_RSP_MODE_R2;
    		break;
    	case MMC_RSP_R3:
    		rsp_type = SD_RSP_MODE_R4;
            break;
        default:
    		printk("unknown response = 0x%x\n", mmc_resp_type(req->cmd));
    		req->cmd->error = -EINVAL;
    		mmc_request_done(mmc, req);
    		return;
    		break;
	}

	if(req->cmd->opcode == CMD0)
	{
		host->SD_mode = SD_IN_IDLE;
	}
	else if (req->cmd->opcode == 0xff)
	{
		rsp_type |=0x80;
		printk("not calc crc7!\n");
	}

	//Check if the cmd needs to transfer data
	

	//Check mode
#if 0 // sd mode
	if((host->SD_mode == SD_IN_IDLE) && (req->cmd->opcode > CMD1)&&(req->cmd->opcode != 55)&&(req->cmd->opcode != 8)&&(req->cmd->opcode != 41)&&(req->cmd->opcode != CMD2)&&(req->cmd->opcode != 0xff))
#else // sdio mode
	if((host->SD_mode == SD_IN_IDLE) && (req->cmd->opcode > CMD1)&&(req->cmd->opcode != 8)&&(req->cmd->opcode != 3)&&(req->cmd->opcode != 5))
#endif
	{
        printk("invalid cmd :0x%x,sd_mod:0x%x EINVA:0x%x!\n",req->cmd->opcode,host->SD_mode,EINVAL);
        req->cmd->error = -EINVAL;
		mmc_request_done(mmc, req);
		return;
	}else if((host->SD_mode == SD_IN_IDLE) && (req->cmd->opcode == CMD1)){
		host->SD_mode = SD_IN_READY;
	}else if((host->SD_mode == SD_IN_IDLE) && (req->cmd->opcode == CMD2)){
		host->SD_mode = SD_IN_IDENT;
#if 0 // sd mode
	}else if((host->SD_mode == SD_IN_IDENT) && (req->cmd->opcode == CMD3)){
#else
	}else if((host->SD_mode == SD_IN_IDLE) && (req->cmd->opcode == CMD3)){
#endif
		host->SD_mode = SD_IN_STBY;
	}else if((host->SD_mode == SD_IN_STBY) && (req->cmd->opcode == CMD7)){
		host->SD_mode = SD_IN_TRANS;
	}

    
    actmci_set_irq(host,0);
	if(req->data == NULL)
	{
	//Send cmd 
        host->SD_mode = SD_IN_BUSY;
		req->cmd->error = am_SD_send_cmd(host, req->cmd->opcode, req->cmd->arg, rsp_type, (u8 *)req->cmd->resp);
		if(req->cmd->error != SD_GOOD)
		{
			printk("%s error = %d, cmd = %d arg = %d\n", __func__, req->cmd->error, req->cmd->opcode, req->cmd->arg);
		}
	}
	else 

    //if(req->data != NULL)
    {	//data transfer
		DBG_PRT(SD_DEBUG_R|SD_DEBUG_W, "%s sg_len = %d, blksz = %d, blocks = %d, flags = %x, bytes_xfered = %x\n",\
			__func__, req->data->sg_len, req->data->blksz, req->data->blocks, req->data->flags, req->data->bytes_xfered);



        
		if(req->data->flags & MMC_DATA_READ){	//Read
			//Read Data
#ifdef CONFIG_MMC_ACTIONS_DMA
			req->data->error = am_SD_autoread2_dma(host, req); 
			dma_unmap_sg(mmc_dev(host->mmc), req->data->sg, host->dma_sg_len, host->dma_dir);
#else
			//req->data->error = am_SD_autoread2(host, req);
            host->SD_mode = SD_IN_READ;
            req->data->error = am_SD_NormalRead(host, req);
                   //req->data->error = am_SD_autoread3(host, req); 
#endif
			//flush_cache_all();
			req->data->bytes_xfered = req->data->blksz*req->data->blocks;
			//dma_unmap_sg(mmc_dev(host->mmc), req->data->sg, host->dma_sg_len, host->dma_dir);

			//Check if need to send stop
			if(req->stop)
			{
				rsp_type = SD_RSP_MODE_R1b | DIVIDE_SCHEME0;
				req->stop->error = am_SD_send_cmd(host, req->stop->opcode, req->stop->arg, rsp_type, (u8 *)req->stop->resp);
				if(req->stop->error != SD_GOOD)
				{
					printk("Stop read cmd error = %d\n", req->stop->error);
					reset_ctller(host);
				}
			}
		}else{						//Write
			//write Data
#ifdef CONFIG_MMC_ACTIONS_DMA
			req->data->error= am_SD_autowrite2_dma(host, req);
			dma_unmap_sg(mmc_dev(host->mmc), req->data->sg, host->dma_sg_len,host->dma_dir);
#else
            host->SD_mode = SD_IN_WRITE;
			//req->data->error= am_SD_autowrite2(host, req);
            
            req->data->error= am_SD_autowrite2_dma_hw(host, req);
            
            //req->data->error = am_SD_autowrite3(host, req); (host, req);
#endif
			req->data->bytes_xfered = req->data->blksz*req->data->blocks;

			//Check if need to send stop
			if(req->stop)
			{
				rsp_type = SD_RSP_MODE_R1b | DIVIDE_SCHEME0;
				req->stop->error = am_SD_send_cmd(host, req->stop->opcode, req->stop->arg, rsp_type, (u8 *)req->stop->resp);
				if(req->stop->error != SD_GOOD)
				{
					printk("Stop write cmd error = %d\n", req->stop->error);
					reset_ctller(host);
				}
			}
   
            
		}

    }

	mmc_request_done(mmc, req);
    host->SD_mode = SD_IN_FREE;
    actmci_set_irq(host,1);

    return;
}

#endif

















static void am_mmc_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct am_mci *host = mmc_priv(mmc);
	int SD_para = 0;
	int result;

	DBG_PRT(SD_DEBUG, "%s +++\n", __func__);

	//check clock and set clock
	DBG_PRT(SD_DEBUG, "mmc set clock = %d\n", ios->clock);
	if(ios->clock !=0)
	{
		//Switch ctl clock to meet the mmc(card) clock
		if(ios->clock != host->sd_clock)
		{
			if(ios->clock <= 26000000)
			{
				result = am_SD_send_clock_cmd(host, SD_CTL_CLOCK_30);
			}else{
				result = am_SD_send_clock_cmd(host, SD_CTL_CLOCK_96);
			}

			if(result != SD_GOOD)
			{
				printk("%s set clock ERROR\n", __func__);
			}
			host->sd_clock = ios->clock;
			host->sd_parameter = 0;
		}
	}



	mdelay(10);
	

    //check bus width
	switch(ios->bus_width)
	{
	case MMC_BUS_WIDTH_8:
		DBG_PRT(SD_DEBUG, "mmc set bus width = 8(%d)\n", ios->bus_width);
		SD_para |= SD_8BIT_MODE;
		break;
	case MMC_BUS_WIDTH_4:
		DBG_PRT(SD_DEBUG, "mmc set bus width = 4(%d)\n", ios->bus_width);
		SD_para |= SD_4BIT_MODE;	
		break;
	case MMC_BUS_WIDTH_1:
		DBG_PRT(SD_DEBUG, "mmc set bus width = 1(%d)\n", ios->bus_width);
		SD_para |= SD_1BIT_MODE;
		break;
	defualt:
		printk("%s ERROR, set unkown bus width %d", __func__, ios->bus_width);
		break;
	}



	if(ios->timing&MMC_TIMING_SD_HS)
	{// switch to high speed
		SD_para = (SD_para&(~0xf))|5;
		printk("%s %d switch to high speed!\n",__func__,__LINE__);
	}

	

	//set bus width
	if(host->sd_parameter != SD_para)
	{
	
		result = am_SD_send_para_cmd(host, SD_para, WAIT_BUSY);
		if(result != SD_GOOD)
		{
			printk("%s set parameter ERROR\n", __func__);
		}
		host->sd_parameter = SD_para;

	}
	DBG_PRT(SD_DEBUG, "%s ---\n", __func__);
}

void actmci_set_irq(struct am_mci *host, unsigned int flag)
{
    

    if(flag)
        writel(SDIO_IRQ_EN, CRC_IRQ_EN);
    else
        writel(0, CRC_IRQ_EN);
	
}

static void actmci_set_sdio_irq(struct mmc_host *mmc, int enable)
{
	struct am_mci *host = mmc_priv(mmc);


	if (enable)
    {   
        set_sdio_int(host,1);
    }   
	else
    {
         set_sdio_int(host,0);
    }   
}



void set_sdio_int(struct am_mci *host,int enable)
{
	if (enable)
    {   
        am_SD_CLR_INT_CMD(host);
        writel(4, host->reg_base + SD_IRQ_CLR_REG);
		am_SDIO_ENABLE_INT(host,1,0);
        actmci_set_irq(host, 1);
        
    }   
	else
    {
    	am_SDIO_ENABLE_INT(host,0,0);
        actmci_set_irq(host, 0);
    }  
}



static const struct mmc_host_ops am_mmc_ops = {
	.request	        = am_mmc_request,
	.set_ios	        = am_mmc_set_ios,
	.enable_sdio_irq	= actmci_set_sdio_irq,
};

static int __init am_mci_probe(struct platform_device *pdev)
{
	struct mmc_host *mmc;
	struct am_mci *host = NULL;
	struct resource *res = NULL;
	int irq = -1, ret;

	//Get SD systemresource
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(res == NULL)
	{
		printk("%s IORESOURE_MEM is NULL", __func__);
	}

	irq = platform_get_irq(pdev, 0);
	if(irq < 0)
	{
		printk("%s doesn't get irq = %d number\n", __func__, irq);
	}

	//Check register base if it have been requested
#if 1
    
    res = request_mem_region(res->start, res->end - res->start + 1, pdev->name);
	if(res == NULL)
	{
		printk("%s request mem region error\n", __func__);
		return -EBUSY;
	}

#endif
	//Allocate SD host and initial data
	mmc = mmc_alloc_host(sizeof(struct am_mci), &pdev->dev);
	if(mmc == NULL)
	{
		return -ENOMEM;
	}
	mmc->ops = &am_mmc_ops;
	mmc->f_min = 10000000;
	mmc->f_max = 52000000;
	mmc->max_phys_segs = 1;
	mmc->max_hw_segs = 1;
	mmc->max_blk_size = 512;
	mmc->max_blk_count = 128;
	mmc->max_req_size = mmc->max_blk_size * mmc->max_blk_count;	//64K
	mmc->max_seg_size = mmc->max_req_size;
	mmc->ocr_avail = MMC_VDD_165_195 | MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_29_30 | \
			MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_34_35 | MMC_VDD_35_36;
	mmc->caps = MMC_CAP_MMC_HIGHSPEED | MMC_CAP_4_BIT_DATA |MMC_CAP_SDIO_IRQ;

	//Initial host data struct
	host = mmc_priv(mmc);
	host->mmc = mmc;

#if 1
    host->res = res;
#endif
    host->sd_clock = 0;
    host->sd_parameter = 0;
    host->SD_CTL_STATE = SD_CTL_IDLE;
    host->irq = irq;
	host->reg_base = ioremap(res->start, 0x1000);
	host->SD_mode = SD_IN_INIT;
	host->dma_cn = am_request_dma(DMA_CHAN_TYPE_SPECIAL, "SD", (void *)0, 0, (void *)0);
    writel(SD_DRQ_1_MOD ,host->reg_base + SD_DRQ_SET_REG);
    printk("request dma is 0x%x,dma drq:0x%x\n",host->dma_cn,readl(host->reg_base + SD_DRQ_SET_REG));
    DBG_PRT(SD_DEBUG, "##%s irq = %d VREG_BASE = 0x%x PREG_BASE = 0x%x DMA_CN = %d\n",\
		__func__, host->irq, (unsigned int)host->reg_base, res->start, host->dma_cn);
    
	host->adma_virt= dma_alloc_coherent(mmc_dev(host->mmc), PAGE_SIZE,
					&host->adma_phy, GFP_KERNEL);
    
    host->irq_pend = 0;
    host->irq_en   = 0;

	//Initial clock and pin 
	am_sd_init_clk_pin();
#if SD_ENABLE_IRQ
	//Reqister IRQ and enable IRQ
	
    set_sdio_int(host,0);
	ret = request_irq(host->irq, am_mmc_irq, 0, DRIVER_NAME, host);
    printk("SDIO IRQ en! 0x%x 0x%x\n",(u32)mmc,(u32)host);
#endif
	platform_set_drvdata(pdev, mmc);

	//Register driver
	ret = mmc_add_host(mmc);
	if(ret != 0)
	{
		printk("%s register mmc driver fail %x\n", __func__, ret);
	}else{
		printk("%s regsiter mmc driver success\n", __func__);
	}

	return 0;
}

static int __init am_mci_remove(struct platform_device *pdev)
{
    struct mmc_host *mmc=platform_get_drvdata(pdev);
    struct am_mci *host = mmc_priv(mmc);
    
    if(host->adma_virt)
        dma_free_coherent(mmc_dev(mmc), PAGE_SIZE,
				  host->adma_virt, host->adma_phy);
    return 0;
}

static struct platform_driver am_mci_driver = {
	.probe		= am_mci_probe,
	.remove		= am_mci_remove,
	.driver		= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init am_mci_init(void)
{
	return platform_driver_register(&am_mci_driver);
}

static void __exit am_mci_exit(void)
{
	platform_driver_unregister(&am_mci_driver);
}

module_init(am_mci_init);
module_exit(am_mci_exit);

MODULE_DESCRIPTION("ACTIONS-MICRO Multimedia Card Interface Driver");
MODULE_LICENSE("GPL");
