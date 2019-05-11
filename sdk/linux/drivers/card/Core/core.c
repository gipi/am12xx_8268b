#include "core.h"

#include "linux/kernel.h"
#include "linux/module.h"
/*am7x platform register operations*/
#include "actions_regs.h"
#include "actions_io.h"
#include "linux/slab.h"
#include <dma.h>
#include "Card_drv_params.h"
#include "Card_Reader.h"
#include "am7x-freq.h"

///#include "am7x-freq.h"



 #define CARD_DMA_SIZE 0x10000

 INT8U CRC_DMA_Mode=Loop_Mode;
static int sd_dma_num;
static int cf_dma_num;
static INT8U  InsertCardType;
 
#if SDCARD_TEST_IRQ  ==0x01
#include <linux/semaphore.h>
#include <linux/interrupt.h>



static struct semaphore sd_sem; 
static struct semaphore cf_sem;
static irqreturn_t isr_sd(int isr,void *id)
{

	if(am_get_dma_irq(sd_dma_num,1,0)==1)
	{
		am_clear_dma_irq(sd_dma_num,1,0);

		if(CRC_DMA_Mode ==DMA_IRQ_Mode)
		{
			up(&sd_sem);	
		}
	}
	return IRQ_HANDLED;
}
static irqreturn_t isr_cf(int isr,void *id)
{

	if(am_get_dma_irq(cf_dma_num,1,0)==1)
	{
		am_clear_dma_irq(cf_dma_num,1,0);

		if(CRC_DMA_Mode ==DMA_IRQ_Mode)
		{
			up(&cf_sem);	
		}
	}
	return IRQ_HANDLED;
}
#endif 
void Set_DMA_Mode(char Mode)
{
	CRC_DMA_Mode   =  Mode;
	
#if SDCARD_TEST_IRQ  ==0x01	
////	DetBUG("run to :%s,%d####\n",__func__,__LINE__);
	if(CRC_DMA_Mode ==DMA_IRQ_Mode  && InsertCardType==Card_Type_CF )
	{
		am_clear_dma_irq(cf_dma_num,1,0);
	///	am_clear_dma_irq(sd_dma_num,1,1);/////??????????
		am_set_dma_irq(cf_dma_num,1,0);
	}
	else if( CRC_DMA_Mode ==DMA_IRQ_Mode)
	{
		am_clear_dma_irq(sd_dma_num,1,0);
	///	am_clear_dma_irq(sd_dma_num,1,1);/////??????????
		am_set_dma_irq(sd_dma_num,1,0);
	}
#endif		
}
EXPORT_SYMBOL_GPL(Set_DMA_Mode);

void Set_CardType(INT8U CardFlag)
{
	InsertCardType =CardFlag;
}
EXPORT_SYMBOL_GPL(Set_CardType);



void Short_DelayUS(INT32U DelayTim)
{
//	volatile INT32U del=DelayTim*25;
#if 0
	volatile INT32U del=DelayTim*50;
	volatile INT32U  resi=0;
	while(del >0 )
	{
		del--;
	         resi= resi+2;
	}
	return;
#else
	unsigned long  c0_count;	
	const unsigned long now = read_c0_count();
	const unsigned long C_Clk = am7x_get_cclk()/_1M;
	
	_Get_TimeOut2(0);
	start_cp0_count();
	c0_count= (C_Clk/4)*DelayTim*2;
	//c0_count=c0_count_to_msec(DelayTim);
	///printk("c0_count:%d %d %d\n",c0_count,am7x_get_cclk(),DelayTim);
	while( read_c0_count() - now < c0_count ) {}	
//	_Get_TimeOut2(2);
	///printk("%d,c0_count:%d %d %d\n",_Get_TimeOut2(1),c0_count,am7x_get_cclk(),DelayTim);
	
	
#endif
}
EXPORT_SYMBOL_GPL(Short_DelayUS);


void print_Dma_reg(INT32S Dmanr)
{
  //   INT32U volatile *DMA_add;
	INT32U iLoop;
	INT32U Addr;

	DetBUG("Mut4: %x\n",act_readl(GPIO_MFCTL4));
	//DetBUG("Mut5: %x\n",act_readl(GPIO_MFCTL5));
	//DetBUG("Mut8: %x\n",act_readl(GPIO_MFCTL8));
	DetBUG("CMU_CARDCLKSEL: %x\n",act_readl(CMU_CARDCLKSEL));
	DetBUG("CMU_CARDCLK: %x\n",act_readl(CMU_CARDCLK));
	DetBUG("CMU_DEVCLKEN: %x\n",act_readl(CMU_DEVCLKEN));
  

			
	for(iLoop =0x00;iLoop<6;iLoop++)
	{
		Addr = 0xb0060000 + iLoop*4;
		printk("add:%x,val:%x,\n",Addr,act_readl(Addr));
	}
	for(iLoop =0x00;iLoop<8;iLoop++)
	{
		Addr = 0xb0060000 + 0x50+iLoop*4;
		printk("add:%x,val:%x,\n",Addr,act_readl(Addr));
	}	
	
	for(iLoop =0x00;iLoop<8;iLoop++)
	{
		Addr = 0xb0060000 + 0x0100 +iLoop*4+ 0x20 * Dmanr;
		printk("add:%x,val:%x,\n",Addr,act_readl(Addr));
	}
     
}
#define CARD_SYNC_REG()     __asm__ volatile ("sync")
/*send  command to card*/
INT32S card_send_msg(void * data, INT32S len, INT32S * actual_length, INT32S timeout)
{
	INT32U ctl = 0;
	INT32S i, tmp;
	volatile INT32S  iLoop;

	ctl = 0x18000000UL | (len & 0xffffff);
	act_writel(ctl, CRC_CTL);  //set tranfer length
	
	for(i=0; i<(len+3)/4; i++)
	{
		act_writel(*(INT32U*)(data+i*4), CRC_DATA);	
	}

	for(iLoop=0; iLoop<=timeout; iLoop++)
	{
		tmp = act_readl(CRC_STATUS);
		if(tmp&0x2)//transfer have done ,should break loop
		{	
            //Trace_CRC("CRC_STATUS: 0x%x,iLoop:%d\n", tmp,iLoop);
            break;
		}
		if(iLoop == timeout)
		{
			Trace_CRC("card_send_msg, timeout, CRC_STATUS: 0x%x,len: %d , L:%d\n", tmp,len,__LINE__);
			act_writel(0xe0000000, CRC_CTL);		//reset controller			
			return CARD_TIMEOUT;
		}
		
	}

	return CARD_GOOD;	

}
EXPORT_SYMBOL_GPL(card_send_msg);


/*recv response from card*/
INT32S card_recv_msg(void * data, INT32S len, INT32S * actual_length, INT32S timeout)
{
	INT32U ctl = 0;	
	INT32S i, tmp;
	volatile INT32S iLoop,jLoop;
	
	for(iLoop=0; iLoop<=timeout; iLoop++)
	{
		tmp = act_readl(CRC_STATUS);		
		if(tmp&0x8)
		{
			*actual_length = tmp>>8;
			if(len != *actual_length)
			{
				Trace_CRC("error! len: %d, actual_length: %d\n", len, *actual_length);
				return CARD_ERROR;
			}
			break;			
		}
	
		if(iLoop == timeout)
		{				
			Trace_CRC("%s, Timeout, CRC_STATUS: %x,L:%d\n",__FUNCTION__ ,tmp,__LINE__);
			act_writel(0xe0000000, CRC_CTL);		//reset controller			
			return CARD_TIMEOUT;
		}
	}

	ctl = len & 0xffffff;
	act_writel(ctl, CRC_CTL);
    //Trace_CRC("##recv CRC_CTL:0x%x,addr:0x %x\n",act_readl(CRC_CTL),CRC_CTL);
    
	for(iLoop=0; iLoop<=timeout; iLoop++)
	{
		tmp = act_readl(CRC_STATUS);		
		if(!(tmp&0x40))//if r_fifo is not empty,jump out for loop, not empty means have data
        {
            //Trace_CRC("recv CRC_STATUS: 0x%x,timeout:%d\n", tmp,timeout);
            break;
        }      
        
        //if r_fifo is empty , continue loop
		if(iLoop == timeout)
		{
			//SD_DBG("card_recv_msg, check fifo not empty timeout, CRC_STATUS: 0x%x,L:%d\n", tmp,__LINE__);
			Trace_CRC("%s,  fifo  empty Timeout, CRC_STATUS: %x,L:%d\n",__func__ ,tmp,__LINE__);
			act_writel(0xe0000000, CRC_CTL);		//reset controller			
			return CARD_TIMEOUT;
		}
	
	}
	
	for(i=0, jLoop=0; i<(len+3)/4;jLoop++)
	{
		tmp = act_readl(CRC_STATUS);		
		if(!(tmp&0x40))//have data, write data from reg to buffer
		{
			*(INT32U*)(data+i*4) = act_readl(CRC_DATA);				
			i++;
		}
		
		if(jLoop == timeout)
		{
			Trace_CRC("read fifo timeout, CRC_STATUS: 0x%x, i:%d,L:%d\n", tmp, i,__LINE__);
			act_writel(0xe0000000, CRC_CTL);		//reset controller			
			return CARD_TIMEOUT;
		}
		
	}
	
	return CARD_GOOD;	
}
EXPORT_SYMBOL_GPL(card_recv_msg);

 INT32S  card_get_dma_chan(INT8U Flag)
{
	INT32S dmano=0;
	INT32S chanmode;
	INT32U dma_weight;
	sd_dma_num = 0xFF;
	cf_dma_num = 0xFF;
	dma_weight=0x00;
	
	switch(Flag)
	{
		case Card_Type_SD:             
		case Card_Type_MS:                        
		case Card_Type_XD:  

	#if  SET_SD_SPECAL_DMA ==1    
			chanmode =DMA_CHAN_TYPE_SPECIAL;
	#else
			chanmode =DMA_CHAN_TYPE_BUS;
	#endif  
	
	#if SDCARD_TEST_IRQ  ==0x01
			dmano = am_request_dma(chanmode, "card", isr_sd, 0, (void *)"sd");
			sema_init(&sd_sem,0);
			sd_dma_num = dmano;
	#else
			dmano = am_request_dma(chanmode, "card", (void *)0, 0, (void *)0);			
	#endif
			break;
		case Card_Type_CF:
		{                
	#if SET_CF_SPECAL_DMA  ==1   
			chanmode =DMA_CHAN_TYPE_SPECIAL;
	#else
			chanmode =DMA_CHAN_TYPE_BUS;
	#endif 
	
	#if SDCARD_TEST_IRQ  ==0x01	
			dmano = am_request_dma(chanmode, "card", isr_cf, 0, (void *)"sd");
			sema_init(&cf_sem,0);
			cf_dma_num = dmano;
	#else			
			dmano = am_request_dma(chanmode, "card", (void *)0, 0, (void *)0);
	#endif
		break;
		}
		default:
		break;
	}

	if ((dmano <0)||(dmano>NUM_AM7X_DMA_CHANNELS)) 
	{
		printk("request sp dma error  dmano:%d !\n",dmano);
		return -1;
	}
	if (dmano>=4)
	{
#if SDCARD_TEST_IRQ  ==0x01	       
		//  am_dis_dma_timeout();
		dma_weight = am_get_dma_weight(dmano);
		am_set_dma_weight(dmano,0x0f);
#endif	
		//@fish add 2010-11-02 for special dma timeout value 
		am_en_dma_timeout(0xFFFFF);	
		//am_dis_dma_timeout();
		//print_Dma_reg(dmano);
		//  am_set_dma_weight(dmano,(dma_weight |(1UL<<28)) &(~(1UL<<24)));
	}
#if SDCARD_TEST_IRQ  ==0x01	
	DetBUG("### SDCARD_TEST_IRQ  True###\n");
#endif

	printk("##card_get_dma_chan: %d $$$ \n", dmano);

        return dmano; 
}
void  card_rel_dma_chan(  INT32S dmano)
{
    INT32U ret =0x00;
    if(dmano>=0)
    {
        am_free_dma(dmano);	
        printk("\n##card_release_dma_chan: %d,%d##\n", dmano,ret);	     
    }
    return; 
}
EXPORT_SYMBOL_GPL(card_rel_dma_chan);
EXPORT_SYMBOL_GPL(card_get_dma_chan);
INT32U _Get_TimeOut2(INT8U  Flag )
{
	static struct timeval tpstart,tpend; 
	INT32U wTime;
	wTime=0x00;
	if(0x0==Flag)
	{
	    	
		do_gettimeofday(&tpstart); 
	}
	else
	{
	     
		do_gettimeofday(&tpend); 
		wTime=1000000*(tpend.tv_sec-tpstart.tv_sec)+  tpend.tv_usec-tpstart.tv_usec; 
		//get ms
		if(Flag ==2)//Display ms
		{
			wTime/=1000;    
		}
		
	}
	return wTime;
	
	
}
EXPORT_SYMBOL_GPL(_Get_TimeOut2);
INT32S card_read_DMA(INT8U Card_DMA_NUM, void *buf, INT32U length, INT32S timeout)
{
    INT32U result = CARD_GOOD;
    INT32U mode;
    INT32S  bRet;
    volatile INT32S iLoop;
    INT32S dma_num = Card_DMA_NUM;

    if(((INT32S)buf&0x3) == 0x0)
    {
        act_writel(CRC_DRQ_SET_VALUE_D4, CRC_DRQ_SET);  			
        mode =DMA_READ_SDRAM_32;
    }
    else
    {
        act_writel(CRC_DRQ_SET_VALUE_D1, CRC_DRQ_SET);			
        mode =DMA_READ_SDRAM_8;
    }
		
#if SDCARD_TEST_IRQ  ==0x01  
	//if(CRC_DMA_Mode ==DMA_IRQ_Mode)
	{
         am_clear_dma_irq(dma_num,1,0);
       	am_set_dma_irq(dma_num,1,0);
	}
#endif   
    am_dma_cmd(dma_num,DMA_CMD_RESET);
    am_set_dma_addr(dma_num,CRC_DATA_P,(INT32U)buf&0x1fffffff); 	
    am_dma_config(dma_num,mode,length);

    am_dma_start(dma_num,0);	
	
#if SDCARD_TEST_IRQ ==0x01
    if(CRC_DMA_Mode ==DMA_IRQ_Mode)
    {
		if( InsertCardType==Card_Type_CF )
		{
			if(down_timeout(&cf_sem,200)!=0)
			{
				result = CARD_TIMEOUT;//  1000*10ms s time out                       
			}
		}
		else
		{
			if(down_timeout(&sd_sem,200)!=0)
			{
				result = CARD_TIMEOUT;//  1000*10ms s time out                       
			}	
		}			
	
    }
   else
#endif	   	
    {
        for(iLoop=0; iLoop<=timeout; iLoop++)
        {	
            bRet = am_dma_cmd(dma_num,DMA_CMD_QUERY);
            if(!bRet)
                    break;
            if(iLoop == timeout)
            {
        		print_Dma_reg(dma_num);
                Trace_CRC("card_Read_DMA, timeout,L:%d\n",__LINE__);
                am_dma_cmd(Card_DMA_NUM,DMA_CMD_RESET);
                result = CARD_TIMEOUT;
            }
        }
	} 

	return result;
}

s32 card_write_DMA(u8 Card_DMA_NUM, void *buf, u32 length, s32 timeout)
{
    u32 result = CARD_GOOD;
    u32 mode;
    volatile s32  iLoop;
    u8 bRet;
    s32 dma_num = Card_DMA_NUM;


    if(((u32)buf&0x3) == 0x0)
    {
        act_writel(CRC_DRQ_SET_VALUE_D4, CRC_DRQ_SET);			
        mode=DMA_WRITE_SDRAM_32;
    }
    else
    {
        act_writel(CRC_DRQ_SET_VALUE_D1, CRC_DRQ_SET);
        mode=DMA_WRITE_SDRAM_8;			
    }
#if SDCARD_TEST_IRQ ==0x01		
    //	if(CRC_DMA_Mode ==DMA_IRQ_Mode)
	{
		am_clear_dma_irq(dma_num,1,0);
		am_set_dma_irq(dma_num,1,0);
	}
#endif

    am_dma_cmd(dma_num,DMA_CMD_RESET);	
    am_dma_config(dma_num,mode,length);
    am_set_dma_addr(dma_num,(INT32U)buf&0x1fffffff,CRC_DATA_P); 		
    am_dma_start(dma_num,0);	

#if SDCARD_TEST_IRQ ==0x01		
	if(CRC_DMA_Mode ==DMA_IRQ_Mode)
	{
		if( InsertCardType==Card_Type_CF )
		{
			if(down_timeout(&cf_sem,200)!=0)
			{
				result = CARD_TIMEOUT;//  1000*10ms s time out       
				DetBUG("write dma timout %s\n",__func__);
			}
		}
		else
		{
			if(down_timeout(&sd_sem,200)!=0)
			{
				result = CARD_TIMEOUT;//  1000*10ms s time out
				DetBUG("write dma timout %s\n",__func__);
			}	
		}		
	}
	else
#endif		
	{
        for(iLoop=0; iLoop<=timeout; iLoop++)
        {		
            //bRet =dma_started(dma_num);		   
            bRet = am_dma_cmd(dma_num,DMA_CMD_QUERY);		
            if(!bRet)
            	break;

            if(iLoop == timeout)
            {
        		print_Dma_reg(dma_num);
                Trace_CRC("card_Write_DMA, timeout,L:%d,Addr:%x\n",__LINE__,(u32)buf);
                am_dma_cmd(dma_num,DMA_CMD_START);
				
                result = CARD_TIMEOUT;
            }

        }

	}
#if SDCARD_TEST_IRQ ==0x01	        
   // 	am_clear_dma_irq(dma_num,1,0);
#endif

	//Trace_CRC("end of:%s, cnt:%x\n",__func__,iLoop);
	return  result;	
}

INT32S card_bulk_tran_DMA(INT8U Card_DMA_NUM, INT32U direction,
		void *buf, INT32U length_left, INT32S *residual, INT32U timeout)
{
    INT32S result = CARD_GOOD;		
    INT32U partial = 0;
    INT32U tmp, i;
    INT32U ctl = 0;
    INT32U  length_left_t = length_left;
    volatile INT32S  iLoop;
    INT32U block_num; 

        for(iLoop=0; iLoop<=timeout; iLoop++)
        {
                tmp = act_readl(CRC_STATUS);  

                if(tmp&0x8)
                {
                        partial = tmp>>8;
                        if(partial != length_left_t)
                        {
                                Trace_CRC("error! tmp: 0x%x, partial: %d, length_left: %d\n", tmp, partial, length_left);
                                return CARD_ERROR;
                        }
                        break;
                }
                if(iLoop == timeout)
                {
                        Trace_CRC("CARD_DATA_IN, check tc_set timeout,L:%d\n",__LINE__);
                        act_writel(0xe0000000, CRC_CTL);		//reset controller				
                        return CARD_ERROR;
                }
        }

        if (direction == CARD_DATA_IN)
        {
        
        		dma_cache_wback_inv((INT32U)buf,length_left);
#ifdef PAGE_TRANS_N
                                 
                block_num = partial>>9;	
                for(i=0; i<block_num; i++)
                {
                        act_writel(0x200, CRC_CTL);	
		                CARD_SYNC_REG();			
                        result = card_read_DMA(Card_DMA_NUM, buf+i*512, 0x200, timeout);		
                        if(result == CARD_TIMEOUT)
                        {
                                Trace_CRC("CARD_DATA_IN, dma timeout,%d,i:%d***\n",__LINE__,i);
                                act_writel(0xe0000000, CRC_CTL);		//reset controller
                                return CARD_TIMEOUT;
                        }		
                }

                length_left_t -= 512*i;

#else
                ctl = partial & 0xffffff;
                act_writel(ctl, CRC_CTL);	
	            CARD_SYNC_REG();		
                result = card_read_DMA(Card_DMA_NUM, buf, partial, timeout *(partial>>9));
                if(result == CARD_TIMEOUT)
                {
                        Trace_CRC("2 CARD_DATA_IN, dma timeout,L:%d\n",__LINE__);
                        act_writel(0xe0000000, CRC_CTL);		//reset controller
                        return CARD_TIMEOUT;
                }		
                length_left_t -= partial;
#endif


        }
        else
        {		

                dma_cache_wback_inv((INT32U)buf,length_left);
#ifdef PAGE_TRANS_WRITE
                INT32U block_num;
                volatile INT32S jLoop;
                block_num = length_left_t>>9;
                for(i=0; i<block_num; i++)
                {
                        act_writel(0x08000200, CRC_CTL);	
		                CARD_SYNC_REG();				
                        result = card_write_DMA(Card_DMA_NUM, buf+i*512, 0x200, timeout);
                        if(result == CARD_TIMEOUT)
                        {
                                Trace_CRC("CARD_DATA_OUT, dma timeout,L:%d\n",__LINE__);
                                act_writel(0xe0000000, CRC_CTL);		//reset controller					
                                return CARD_TIMEOUT;
                        }
                        for(jLoop=0; jLoop<=timeout; jLoop++)
                        {
                                tmp = act_readl(CRC_STATUS);				
                                if(tmp&0x1)
                                break;
                                if(jLoop == timeout)
                                {
                                        Trace_CRC("CARD_DATA_OUT, check if_end timeout,L:%d\n",__LINE__);
                                        act_writel(0xe0000000, CRC_CTL);		//reset controller	
                                        return CARD_TIMEOUT;
                                }
                                else
                                {
                                    //usleep(100);
                                    schedule();
                                }
                        }

                }
                length_left_t -= 512*i;
#else
	  //     DetBUG("%s,%d timeout:%x\n",__func__,length_left_t,timeout);
                ctl = 0x08000000 | (length_left_t & 0xffffff);
                act_writel(ctl, CRC_CTL);
                CARD_SYNC_REG();
				
                result = card_write_DMA(Card_DMA_NUM, buf, length_left_t, timeout*(partial>>9));
                if(result == CARD_TIMEOUT)
                {
                        Trace_CRC("2 CARD_DATA_OUT, timeout,L:%d\n",__LINE__);
                        act_writel(0xe0000000, CRC_CTL);		//reset controller	
                        return CARD_TIMEOUT;
                }
                for(iLoop=0; iLoop<=timeout; iLoop++)
                {
                        tmp = act_readl(CRC_STATUS);
                        if(tmp&0x2)
                                break;
                        if(iLoop == timeout)
                        {
                                Trace_CRC("CARD_DATA_OUT, check dackb timeout,L:%d\n",__LINE__);
                                act_writel(0xe0000000, CRC_CTL);		//reset controller
                                return CARD_TIMEOUT;
                        }
                }

                length_left_t = 0;
#endif
        }

        if (residual)
        {
                *residual = length_left_t;
        }

        return result;		
}
/*send data to card  or recv data from card*/
INT32S card_bulk_transfer(INT8U Card_DMA_NUM, INT32U direction, void * buf, INT32U length_left, INT32S * residual, INT32S timeout, INT8U bMode)
{
	INT32S result = CARD_GOOD;		
	INT32U partial = 0;
	INT32U tmp, i;
	INT32U ctl = 0;
	INT32U length_left_t = length_left;
	//INT32U SectorCnt,jLoop;
	volatile INT32S  iLoop;

	if(bMode==DMA_RW)
	{
	        return card_bulk_tran_DMA(Card_DMA_NUM,direction,
		                   buf,   length_left,residual,timeout);
	}
         
//printk("%x,Dir:%x,Adrr:%x,len:%x,Tim:%x\n",Card_DMA_NUM,direction,(INT32U)buf,length_left,timeout);

	if (direction == CARD_DATA_IN)
	{
		for(iLoop=0; iLoop<=timeout; iLoop++)
		{
			tmp = act_readl(CRC_STATUS);
			if(tmp&0x8)//crc tc -set -->crc sie_tc active flag active high
			//if((tmp&0x8) && (!(tmp&20)))
			{
				partial = tmp>>8;
				break;
			}
			if(iLoop == timeout)
			{					
				Trace_CRC("CPU mode, check tc_set timeout,L:%d\n",__LINE__);
				act_writel(0xe0000000, CRC_CTL);		//reset controller
				return CARD_ERROR;
			}
		}
		  
		ctl = partial & 0xffffff;	
		act_writel(ctl, CRC_CTL);
		CARD_SYNC_REG();
		
		for(iLoop=0; iLoop<=timeout; iLoop++)
		{
			tmp = act_readl(CRC_STATUS);
			if(!(tmp&0x40))//rxififo is empty
				break;
			if(iLoop == timeout)
			{	
				Trace_CRC("CPU mode, before read, timeout,L:%d\n",__LINE__);
				act_writel(0xe0000000, CRC_CTL);		//reset controller
				return CARD_TIMEOUT;
			}
		}	
		if(((INT32U)buf&0x3) == 0x0)
		{	
		          
			for(i=0; i<(partial+3)/4; i++)
			{
			      *(INT32U*)(buf+i*4) = act_readl(CRC_DATA);	
			}
			//printk("i:%x,partial:%x,%x\n",i,partial,(partial+3)/4);
		}
		else
		{
			INT32U DataTmp;
			
			for(i=0; i<(partial+3)/4; i++)
			{
				DataTmp=act_readl(CRC_DATA);
				*(INT8U*)(buf+i*4)=(INT8U)(DataTmp&0xFF);
				*(INT8U*)(buf+i*4+1)=(INT8U)((DataTmp&0xFF00)>>8);
				*(INT8U*)(buf+i*4+2)=(INT8U)((DataTmp&0xFF0000)>>16);
				*(INT8U*)(buf+i*4+3)=(INT8U)((DataTmp&0xFF000000)>>24);
				//*(INT32U*)(buf+i*4)
				//*(INT32U*)(buf+i*4) = act_readl(CRC_DATA);	
			}
		}	
		length_left_t -= partial;	
	}
	else
	{
		ctl = 0x08000000 | (length_left_t & 0xffffff);
		act_writel(ctl, CRC_CTL);
		CARD_SYNC_REG();
		
		if(((INT32U)buf&0x3) == 0x0)
		{		    
			for(i=0; i<(length_left_t+3)/4; i++)
				act_writel(*(INT32U*)(buf+i*4), CRC_DATA);	
		}
		else
		{
			INT32U DataTmp;
	
			for(i=0; i<(partial+3)/4; i++)
			{
				//DataTmp=*(INT8U*)(buf+i*4)|*(INT8U*)(buf+i*4+1)<<8|
				//	*(INT8U*)(buf+i*4+2)<<16|*(INT8U*)(buf+i*4+3)<<26;
				ASSIGN_LONG(DataTmp,*(INT8U*)(buf+i*4),*(INT8U*)(buf+i*4+1),
					*(INT8U*)(buf+i*4+2),*(INT8U*)(buf+i*4+3));
				act_writel(DataTmp, CRC_DATA);				
			}
			
		}

		for(iLoop=0; iLoop<=timeout; iLoop++)
		{
			tmp = act_readl(CRC_STATUS);
			if(tmp&0x2)
				break;
			if(iLoop == timeout)
			{
				Trace_CRC("CPU mode, after write, timeout,L%d\n",__LINE__);
				act_writel(0xe0000000, CRC_CTL);		//reset controller
				return CARD_TIMEOUT;
			}
		}
		length_left_t = 0;
		
	}
	if (residual)
	{
		*residual = length_left_t;
	}
//	Card_Print_Buf("",buf,length_left+32);
//	printk("end of \n");
	return result;

}
EXPORT_SYMBOL_GPL(card_bulk_transfer);


INT32S card_switchclock(INT8U  clock_freq)
{
    INT8U cmnd[2];   
	INT32S result = CARD_ERROR;
	//printf("%s--%s, line: %d, clock_freq: %d,%d\n", __FILE__, __FUNCTION__, __LINE__, clock_freq,gCurrent_clock)
       
	
	cmnd[0] = (INT8U) (CARD_SWITCHCLOCK & 0xff);//RTS5121 cmd num
	cmnd[1] = clock_freq;                       //cmd value

	result = card_send_msg(cmnd, sizeof(cmnd), NULL, CMD_TIMEOUT );
	if (result != CARD_GOOD) {
		Trace_CRC("%s--%s, line: %d, result: %d\n", __FILE__, __FUNCTION__, __LINE__, result);
		return result;
	}


	return result;

}
EXPORT_SYMBOL_GPL(card_switchclock);


INT32S card_stop(INT32S card)
{
	INT8U cmnd[2];
	INT32S result = CARD_ERROR;

	cmnd[0] = (INT8U) CARD_STOP;
	switch (card) {
		case TYPE_CF:
			cmnd[1] = STOP_CF;
			break;
		case TYPE_XD:
			cmnd[1] = STOP_SM;
			break;
		case TYPE_SD:
			cmnd[1] = STOP_SD;
			break;
		case TYPE_MS_PRO:
		case TYPE_MS_STICK:	
			cmnd[1] = STOP_MS;
			break;
		default:
			return 0;
	}

	result = card_send_msg(cmnd, sizeof(cmnd), NULL, CMD_TIMEOUT );
	if (result != 0) {
		Trace_CRC("%s--%s, line: %d, result: %d\n", __FILE__, __FUNCTION__, __LINE__, result);
		return result;
	}

	return CARD_GOOD;
}
EXPORT_SYMBOL_GPL(card_stop);


INT32S card_handle_error (INT32S card, INT32U err)
{
	INT32S result = CARD_ERROR;

	if(err == CARD_GOOD)
		return CARD_GOOD;	
	
	act_writel(0xe0000000, CRC_CTL);
	CARD_SYNC_REG(); 
	
	result = card_stop(card);
	if (result != CARD_GOOD) {
		return CARD_ERROR;
	}

	return err;

}
EXPORT_SYMBOL_GPL(card_handle_error);


/*gpio operations*/
void	setgpio(INT8U num,INT8U val)
{
        volatile ssize_t*  reg=0;

        reg = (volatile ssize_t*)(GPIO_31_0INEN+(ssize_t)(num/32)*12);
        *reg = *reg&~(1<<(num%32));   // disable input
        reg = (volatile ssize_t*)(GPIO_31_0OUTEN+(ssize_t)(num/32)*12);
        *reg = *reg|(1<<(num%32));   // enable output

        reg += 2;  // shift to data reg
        if(val)
                *reg = *reg|(1<<(num%32));
        else
                *reg = *reg&~(1<<(num%32));
}
EXPORT_SYMBOL_GPL(setgpio);

INT8U	getgpio(INT8U num)
{
        volatile ssize_t*  reg=0;

        reg = (volatile ssize_t*)(GPIO_31_0OUTEN+(ssize_t)(num/32)*12);
        *reg = *reg&~(1<<(num%32));   // disable output
        reg = (volatile ssize_t*)(GPIO_31_0INEN+(ssize_t)(num/32)*12);
        *reg = *reg|(1<<(num%32));   // enable input

        reg += 1;  // shift to data reg
        return (*reg&(1<<(num%32))?1:0);
}
EXPORT_SYMBOL_GPL(getgpio);

void power_on(INT8U num)
{
	setgpio(num, 0);
}
EXPORT_SYMBOL_GPL(power_on);


void power_off(INT8U num)
{
	setgpio(num, 1);
}
EXPORT_SYMBOL_GPL(power_off);

INT8U	in_slot(INT8U num)
{
         if(num ==0xFF)
                    return 0x00;
	return (getgpio(num)==0)?1:0;
}
EXPORT_SYMBOL_GPL(in_slot);

void card_reader_on(void)
{
#if  CONFIG_AM_CHIP_ID == 1203
        act_writel(act_readl(CMU_DEVCLKEN)| (1<<3), CMU_DEVCLKEN);	
        act_writel(0x8,CMU_CARDCLKSEL);
        act_writel(0x17,CMU_CARDCLK); 
        
#elif CONFIG_AM_CHIP_ID == 1211 
#if 0
 	act_writel(act_readl(CMU_DEVCLKEN)| (1<<3), CMU_DEVCLKEN);	
        act_writel(0x8,CMU_CARDCLKSEL);
        act_writel(0x17,CMU_CARDCLK); 
#else
        act_writel(act_readl(CMU_DEVCLKEN)| (1<<3), CMU_DEVCLKEN);	
        act_writel(((1<<2)|(2<<4)|(3<<8)|(5<<12)|(7<<16)),CMU_CARDCLKSEL);
        act_writel(0x17,CMU_CARDCLK);      
#endif

#elif CONFIG_AM_CHIP_ID == 1220 
        act_writel(act_readl(CMU_DEVCLKEN)| (1<<3), CMU_DEVCLKEN);	
        act_writel(((1<<2)|(3<<4)|(4<<8)|(5<<12)|(7<<16)),CMU_CARDCLKSEL);
        act_writel(0x17,CMU_CARDCLK);
#elif  CONFIG_AM_CHIP_ID ==1213
		act_writel(act_readl(CMU_DEVCLKEN)| (1<<3), CMU_DEVCLKEN);	
		act_writel(((1<<2)|(3<<4)|(4<<8)|(5<<12)|(7<<16)),CMU_CARDCLKSEL);
		act_writel(0x17,CMU_CARDCLK);

#endif
}
EXPORT_SYMBOL_GPL(card_reader_on);

void card_reader_off(void)
{
        
#if CONFIG_AM_CHIP_ID == 1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID ==1213

    act_writel(act_readl(CMU_DEVCLKEN)&(~(1<<3)), CMU_DEVCLKEN);	

   // act_writel(((1<<2)|(2<<4)|(3<<8)|(5<<12)|(7<<16)),CMU_CARDCLKSEL);
   // act_writel(0x17,CMU_CARDCLK); 
#else

#endif
}
EXPORT_SYMBOL_GPL(card_reader_off);


 void Card_Print_Buf( const INT8U    * pad, const INT8U * pData, INT16U inLen)
{
	INT16U iLoop;
	printk("%s", pad);//

	for( iLoop=0; iLoop< inLen; iLoop++)
	{
		printk("%2x ", pData[iLoop]);
		if( 0== ((iLoop+1) &0x0f) )
		{
			printk("  %d\n",iLoop);
		}
	}
       printk("\n");
	
}
EXPORT_SYMBOL_GPL(Card_Print_Buf);

