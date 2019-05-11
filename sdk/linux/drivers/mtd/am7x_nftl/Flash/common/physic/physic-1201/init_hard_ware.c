/*
*********************************************************************************************************
*                                                USDK130
*                                          ucOS + MIPS,Nand Module
*
*                               (c) Copyright 2007, Actions Co,Ld.
*                                          All Right Reserved
* File    : init_hard_ware.c
* By      : nand flash group
* Version : V0.1
*********************************************************************************************************
*/

#include "nand_reg_def-1201.h"


#if __KERNEL__
#include "sys_cfg.h"
#include "dma.h"
#else
#include "syscfg.h"
#include "mipsregs.h"
#include "mfp_config.h"
#endif

#define MULTI_PAD_MODE
#define REG32_DUMP(reg) INIT_DBG(#reg" [%p]: %x\n",(void*)reg,REG_READ(reg))
extern INT32U NandDMAChannel;

#define NAND_FLASH_INIT_CLK        20    /* 20M clk*/
//static INT32U nand_clock = NAND_FLASH_INIT_CLK;


static  unsigned long _get_corePLL(void)
{
	unsigned long val, div2;
	unsigned long bus_val;        

	val = REG_READ(CMU_BUSCLK);
	div2 = (val>>6) & 3;
	switch(div2) 
	{
		case 0: 
			// return 32 * _1K;
			val =0;
		break;
		
		case 1: 
			// return 24 * _1M;
			val =24;
		break;
		
		default: 
		{
#if CONFIG_AM_CHIP_ID == 1203              
			val = REG_READ(CMU_COREPLL);
			val >>= 2;
			val &= 0x3f;
			val *= 6;
#elif (CONFIG_AM_CHIP_ID == 1211) || (CONFIG_AM_CHIP_ID == 1220) || (CONFIG_AM_CHIP_ID == 1213)
			bus_val = REG_READ(CMU_BUSCLK);
#if (CONFIG_AM_CHIP_ID == 1211)
			if(bus_val&(1<<11)){	//DDRPLL
				val = REG_READ(CMU_DDRPLL);
				val >>=2;
				val &= 0x3f;
				val *= 8;
			}
			else{
				val = REG_READ(CMU_COREPLL);
				val >>= 2;
				val &= 0x3f;
				val *= 6;
			}
#elif (CONFIG_AM_CHIP_ID == 1220) || (CONFIG_AM_CHIP_ID == 1213)
			if(bus_val&(1<<11)){	//DDRPLL
				val = REG_READ(CMU_DDRPLL);
#if CONFIG_AM_CHIP_MINOR == 8268
				val >>= 1;
				val &= 0x3f;
				val *= 24;
#else
				val &= 0x7f;
				val *= 8;
#endif
				printf("--------- getcorepll() DDRPLL val:%d\n",val);
			}
			else{
				val = REG_READ(CMU_COREPLL);
#if CONFIG_AM_CHIP_MINOR == 8268
				val >>= 1;
				val &= 0x3f;
				val *= 24;
#else
				val >>= 2;
				val &= 0x7f;
				val *= 6;
#endif
				printf("--------- getcorepll() COREPLL val:%d\n",val);
			}
#endif	
		
		//val*=1000000;
#endif       
		break;
		}
	}

	return val;
}

#if 0
static  unsigned long Flash_get_hclk(void)
{
	unsigned long div ;
	unsigned long HCLK_val;
	unsigned long val;
	
	val =_get_corePLL();
	
	div = REG_READ(CMU_BUSCLK);
	div >>= 4;
	div &=  1;            
	HCLK_val = val/(1+div);      
	printf("BUSCLK:0x%x corepLL:0x%x,DDRPLL:0x%x\n",REG_READ(CMU_BUSCLK),
			REG_READ(CMU_COREPLL),REG_READ(CMU_DDRPLL));
	printf("HCLK_val:%d Mhz,div:%x\n",(INT32U)HCLK_val,(INT32U)div);
	return HCLK_val;
}
#endif

INT32U   _Get_TimeOut3(INT8U  Flag )
{
	static INT32U Tmp1;
	INT32U rcore,Tmp2,wTime;
	//rcore= getCorePll(); 
	wTime=0x00;
	if(0x0==Flag)
	{
	        write_c0_count(0);
		Tmp1=read_c0_count();
		
	}
	else
	{	
		Tmp2=read_c0_count();
		rcore = _get_corePLL();				
		wTime=(Tmp2-Tmp1)/(rcore/4);
		if(Flag ==2)//Display ms
		{
			wTime=wTime/1000;
		}
		
	}
	return wTime;
	
	
}
/*
*********************************************************************************************************
*                       SettingNandFreq
*
* Description: setting nand frequence parameter.
*
* Aguments   : NandFreq - NAND Flash Clock (MHz)
*
* Returns    : NULL
*********************************************************************************************************
*/
void SettingNandFreq(INT32U NandFreq, INT32U CoreClock)
{
        INT32U cfg, div, clk_div, clk_ws;
        INT32U	nand_div;
        INT32U CoreClockTmp;     
        if(NandFreq==0)
        	{
        		return ;
        	}
#if  CONFIG_AM_CHIP_ID ==1211 || CONFIG_AM_CHIP_ID ==1203 ||CONFIG_AM_CHIP_ID ==1220 || (CONFIG_AM_CHIP_ID == 1213)
        //cfg = ((REG_READ(CMU_COREPLL)>>2)&0x3f)*6;
        //cfg= Flash_get_hclk();
        cfg =_get_corePLL();
        if(cfg==0x00|| cfg ==24)
        {
                return ;
        }
	if(0)//if(cfg>300  && NandFreq<20)
	{
		REG_WRITE(CMU_NANDCLK, 0x04);
		INIT_DBG("###CMU_NANDCLK:0x%x,CLKCTL:0x%x,NANDCTL%x###\n\n",REG_READ(CMU_NANDCLK),  NAND_REG_READ(NAND_CLKCTL),NAND_REG_READ(NAND_CTL));
		return ;
	}
#else
        # ERROR
#endif	
        
        div =0x00;
        CoreClockTmp =cfg;
        nand_div = NAND_REG_READ(NAND_CLKCTL);
        switch(nand_div)
        {
                case 0x04:
                        cfg = cfg/3;
                        break;
                case 0x08:
                        cfg =cfg/4;
                        break;
                case 0x0c:
                        cfg =cfg/5;
                        break;
                default:
                        cfg =cfg/3;
                        break;
        }
        //	printf("cfg:%d,%d\n",cfg,CoreClockTmp);
        clk_div =(cfg*100/NandFreq);         
        if((clk_div%100)>=50)
        {
                clk_div =clk_div/100;
        }
        else
        {
                clk_div =clk_div/100-1;
        }
        clk_ws =(cfg*1000)/(clk_div+1);
        INIT_DBG("CMU_COREPLL:0x%x\n",REG_READ(CMU_COREPLL));
        INIT_DBG("###%d,%d,%dMhz,%dMHz,%dKhz ###\n",clk_div,nand_div,CoreClockTmp,NandFreq,clk_ws);
	#if CONFIG_AM_CHIP_ID == 1213
		if(NandFreq>24)
			div = CoreClockTmp * 100/96;
		else 
			div = CoreClockTmp * 25/NandFreq;
		if(div>=100)
		{
	        if((div%100)>=50)
	        {
	        	div =div/100;
	        }
	        else
	        {
	        	div =div/100 - 1;
	        }		
		}
		else
		{
			div = 0;		
		}
	    clk_div = ((INT32U)div << 8) | clk_div;
		REG_WRITE(CMU_NANDCLK, clk_div);
        INIT_DBG("###Set BMA:%dMhz###\n",CoreClockTmp/(div+1));		
	#else
		REG_WRITE(CMU_NANDCLK, clk_div);
	#endif
        INIT_DBG("###CMU_NANDCLK:0x%x,CLKCTL:0x%x,NANDCTL%x###\n\n",REG_READ(CMU_NANDCLK),  NAND_REG_READ(NAND_CLKCTL),NAND_REG_READ(NAND_CTL));
        return ;
}


/*
*********************************************************************************************************
*                       INIT NAND FLASH HARDWARE MODULE
*
* Description: init nand flash hard ware module.
*
* Aguments   : none
*
* Returns    : always return 0 value
*********************************************************************************************************
*/

extern INT32S bflash_end_data;
extern INT32S bflash_end;

void init_sbss(void)
{
#if !__KERNEL__
    INT32U *p;

	p = &bflash_end_data;
    while(p < &bflash_end)
        * p ++ = 0;
#endif
}

INT32S nand_flash_inithw(void)
{
	//printk("nand_flash_inithw\n");
        INT32U cfg;
        
        //enable NAND controller clock 
        cfg = REG_READ(CMU_DEVCLKEN);
        cfg |= CMU_DEVCLKEN_NAND| CMU_DEVCLKEN_DMAC  | CMU_DEVCLKEN_SRAMR;
        REG_WRITE(CMU_DEVCLKEN, cfg);

		REG_WRITE(MU_REG1,REG_READ(MU_REG1)|0x1f<<9);
		REG_WRITE(MU_REG2,REG_READ(MU_REG2)|0x1f<<9);	
		INIT_BOOT("MU_REG1:0x%08x\n",REG_READ(MU_REG1));
		INIT_BOOT("MU_REG2:0x%08x\n",REG_READ(MU_REG2));

        //enable special dma channel 4,5 clock
        //cfg = REG_READ(CMU_DMACLK);
        //cfg |= CMU_DMACLK_D4EN;
        REG_WRITE(CMU_DMACLK, 0x3);
	
#if CONFIG_AM_CHIP_ID==1203//208&176pin
        INT32U MF1Val,MF2Val,MF3Val,MF4Val,MF5Val;
        MF1Val=REG_READ(GPIO_MFCTL1);
        MF2Val=REG_READ(GPIO_MFCTL2);
        MF3Val=REG_READ(GPIO_MFCTL3);
        MF4Val=REG_READ(GPIO_MFCTL4);
        MF5Val=REG_READ(GPIO_MFCTL5);

        if (CHIP_TYPE==AM_CHIP_7531)  //176 Pin
        {
                MF1Val=(MF1Val&NF_MF1_MSK_7531)|NF_MF1_VAL_7531;
                REG_WRITE(GPIO_MFCTL1, MF1Val);
                if(MAX_CHIP_NUM == 1)
                        MF2Val=(MF2Val&NF_MF2_MSK_7531_1CE)|NF_MF2_VAL_7531_1CE;
                else
                        MF2Val=(MF2Val&NF_MF2_MSK_7531)|NF_MF2_VAL_7531;
                REG_WRITE(GPIO_MFCTL2, MF2Val); 
        }
	
#elif CONFIG_AM_CHIP_ID==1211
#if 0
	if(CHIP_TYPE == AM_CHIP_7051)
		am7x_do_mfp_configs(&am7051_nand_mfp_configs);
#endif	
        INT32U MF3Val,MF4Val,MF5Val;
        MF3Val=REG_READ(GPIO_MFCTL3);
        MF4Val=REG_READ(GPIO_MFCTL4);
        MF5Val=REG_READ(GPIO_MFCTL5);  
        if(MAX_CHIP_NUM == 1) {
			MF3Val=(MF3Val&NF_MF3_MSK_7555_1CE)|NF_MF3_VAL_7555_1CE;
			REG_WRITE(GPIO_MFCTL3, MF3Val);
        }
        else {

			MF3Val=(MF3Val&NF_MF3_MSK_7555)|NF_MF3_VAL_7555;
            REG_WRITE(GPIO_MFCTL3, MF3Val);
        }
        MF4Val=(MF4Val&NF_MF4_MSK_7555)|NF_MF4_VAL_7555;
        REG_WRITE(GPIO_MFCTL4, MF4Val);
        MF5Val=(MF5Val&NF_MF5_MSK_7555)|NF_MF5_VAL_7555;
        REG_WRITE(GPIO_MFCTL5, MF5Val);
//------------------------------------
#elif CONFIG_AM_CHIP_ID == 1220
    INT32U MF5Val;
    MF5Val=0x55555489;  

    REG_WRITE(GPIO_MFCTL5, MF5Val);
#elif CONFIG_AM_CHIP_ID == 1213
	INT32U MF2Val,MF3Val;
	MF2Val=REG_READ(GPIO_MFCTL2);
	MF3Val=REG_READ(GPIO_MFCTL3);
	if(MAX_CHIP_NUM==1)
		{
  		MF2Val=(MF2Val&NF_MF2_MSK_8250_1CE|NF_MF2_VAL_8250_1CE);
 		MF3Val=(MF3Val&NF_MF3_MSK_8250_2CE |NF_MF3_VAL_8250_2CE);		
		}
	else if(MAX_CHIP_NUM==2)
		{
  		MF2Val=(MF2Val&NF_MF2_MSK_8250_2CE|NF_MF2_VAL_8250_2CE);
 		MF3Val=(MF3Val&NF_MF3_MSK_8250_2CE |NF_MF3_VAL_8250_2CE);
		}
	else if(MAX_CHIP_NUM==3)
		{
		MF2Val=(MF2Val&NF_MF2_MSK_8250|NF_MF2_VAL_8250);
 		MF3Val=(MF3Val&NF_MF3_MSK_8250_2CE |NF_MF3_VAL_8250_2CE);
		}
	else
		{
		MF2Val=(MF2Val&NF_MF2_MSK_8250|NF_MF2_VAL_8250);
 		MF3Val=(MF3Val&NF_MF3_MSK_8250|NF_MF3_VAL_8250);
		}
 // MF2Val=(MF2Val&0x003fffff |((1<<22)|(1<<23)|(1<<25)|(1<<26)|(1<<29)));
 // MF3Val=(MF3Val&0xfffffc00 |((1<<0)|(1<<2)|(1<<5)|(1<<7)));
  REG_WRITE(GPIO_MFCTL2, MF2Val);	
  REG_WRITE(GPIO_MFCTL3, MF3Val);	
	  
#endif

        //slc flash 可以跑30M
        //REG_WRITE(CMU_NANDCLK, 0x3);//cmu_nand
#if CONFIG_AM_CHIP_ID == 1213
	      REG_WRITE(CMU_NANDCLK, 0x50e);//cmu_nand
#else
        REG_WRITE(CMU_NANDCLK, CFG_CMU_NANDCLK);//cmu_nand
#endif
        REG_WRITE(CMU_DMACLK, 1);
//	NandDMAChannel = NAND_DMA_CHANNEL_NUM;

/*=================== =====================*/
#if (defined(__KERNEL__)) && (EIP_IRQ_MODE ==0x01)

#if SPECIAL_DMA_Test ==1
        NandDMAChannel =am_request_dma(DMA_CHAN_TYPE_SPECIAL,"NAND",NULL,NULL,(void*)"Flash");
#else   
	NandDMAChannel =0;
#endif
	flash_irq_init();
/*=================== =====================*/
#elif (defined(__KERNEL__))

#if SPECIAL_DMA_Test ==1
	NandDMAChannel = 4;
#else
   	NandDMAChannel =0;//am_request_dma(DMA_CHAN_TYPE_BUS,"NAND",Isr_Flashdriver,NULL,(void*)"Flash");
#endif
	//NandDMAChannel =am_request_dma(DMA_CHAN_TYPE_SPECIAL,"NAND",NULL,NULL,(void*)"Flash");
/*=================== =====================*/		
#else

#if SPECIAL_DMA_Test ==1
	NandDMAChannel = 4;
#else
        NandDMAChannel = 0;
#endif

	write_c0_cause(read_c0_cause()&0xf7ffffff);   //enable counter	
#endif

        
        //REG_WRITE(DMA_IOMAP_BASE + DMA_con, 0xff04); //DMA priority: dma0>dma1>cpu 
        //	REG_WRITE(DMA_IOMAP_BASE + DMA_con, 0xff0004); //tl
        //REG_WRITE(DMA_IOMAP_BASE + DMA_IRQEn, 0x0); // diable dma interrupt 
        //REG_WRITE(DMA_IOMAP_BASE + DMA_IRQPending, 0xffff); // diable dma interrupt 

        REG_WRITE(DMA_CTL, 0xff04); //DMA priority: dma0>dma1>cpu 
        REG_WRITE(DMA_IRQEN, 0x0);  // Disable dma interrupts
        REG_WRITE(DMA_IRQPD, 0xffff); // Clear dma interrupt pendings

        //reset and start and config the sm
        //cfg = NAND_REG_READ(NAND_CTL);
        //wait_fsm_ready();	//tl
        //cfg = NAND_CTL_EN | NAND_CTL_RST | NAND_CTL_BWID_8BIT |NAND_CTL_TADL_8CLK | NAND_CTL_MODE_EDO;

 //       cfg = NAND_CTL_EN | NAND_CTL_RST | NAND_CTL_BWID_8BIT | NAND_TADL_WAIT_8CLK | \
//        NAND_RB_RB1PE |NAND_CTL_MODE_EDO;
	   cfg = NAND_CTL_EN | NAND_CTL_RST | NAND_CTL_BWID_8BIT | NAND_TADL_WAIT_8CLK | \
       		 NAND_RB_RB1PE |NAND_CTL_MODE_EDO;	
        NAND_REG_WRITE(NAND_CTL, cfg);
		
#if BUS_DMA_BlockMode_Test==0x01		
	NAND_REG_WRITE(NAND_CTL, cfg|NAND_BLOC_MODE);	
#endif     

        //nand clock config.duty_CTL=0(duty_CTL has no effcet on 8bit flash),DMA 1 low wait
        //cfg =0x08;	//tl	 0x06;
        //NAND_REG_WRITE(NAND_CLKCTL, 1<<2);
        NAND_REG_WRITE(NAND_CLKCTL, CFG_NAND_CLKCTL);
        //ATTENSTIONS: bit0 of this register is inverse with spec,write 0 be seen as 1 and write 1 will seen as 0.

        //ecc config
        //USER DATA参与编码
        NAND_REG_WRITE(NAND_ECCCTL, 0x1);

        //address init
        NAND_REG_WRITE(NAND_SPARE_COLADDR, 0);
        NAND_REG_WRITE(NAND_MAIN_COLADDR, 0);
        NAND_REG_WRITE(NAND_ROWADDRLO, 0);
        NAND_REG_WRITE(NAND_ROWADDRHI, 0);

        NAND_REG_WRITE(NAND_BC, 0);

        REG32_DUMP(CMU_COREPLL);
        REG32_DUMP(CMU_BUSCLK);
        REG32_DUMP(SDR_CTL);
        REG32_DUMP(SDR_CMD);
        REG32_DUMP(SDR_TIMING);
        REG32_DUMP(SDR_RFSH);
        REG32_DUMP(SDR_EN);
        REG32_DUMP(SDR_MODE);
        REG32_DUMP(SDR_CLKDLY);
        //REG32_DUMP(SDR_PD);
        //REG32_DUMP(SDR_INITD);
#if CONFIG_AM_CHIP_ID==1213
        REG32_DUMP(SDR_MSTDLY);
        REG32_DUMP(SDR_LATENCY);       
#endif
#if CONFIG_AM_CHIP_ID==1211
        REG32_DUMP(CMU_DDRPLL);
#endif

#if(CONFIG_AM_CHIP_ID==1203) || (CONFIG_AM_CHIP_ID==1211)||(CONFIG_AM_CHIP_ID==1220)|| (CONFIG_AM_CHIP_ID == 1213)
     //   cfg = Flash_get_hclk();
	cfg =_get_corePLL();
#else
        #error Inv chip!
#endif
        cfg =(cfg*1000)/((REG_READ(CMU_NANDCLK) & 0x0F)+1);
        switch(NAND_REG_READ(NAND_CLKCTL))
        {
                case 0x04:
                        cfg = cfg/3;
                        break;
                case 0x08:
                        cfg =cfg/4;
                        break;
                case 0x0c:
                        cfg =cfg/5;
                        break;
                default:
                        break;
        }
	INIT_BOOT("##Set nand Flash:%d Khz##\n",cfg);
	INIT_BOOT("##CMU_COREPLL:0x%x ,NAND_CTL: 0x%x\n",REG_READ(CMU_COREPLL),NAND_REG_READ(NAND_CTL));
	INIT_BOOT("###CMU_NANDCLK:0x%x,CLKCTL:0x%x,NANDCTL%x###\n",REG_READ(CMU_NANDCLK),  NAND_REG_READ(NAND_CLKCTL),NAND_REG_READ(NAND_CTL));
	INIT_BOOT("###NandDMAChannel:%d ###\n",NandDMAChannel);
	//INIT_BOOT("###3.Flash_PAGE_RW_BCH24:%d\n",Flash_PAGE_RW_BCH24);
	//INIT_BOOT("###3.Flash_PAGE_RW_BCH8:%d\n",Flash_PAGE_RW_BCH8);
	INIT_BOOT("###4.User_Data_2Byte:%d\n",User_Data_2Byte);
	INIT_BOOT("###5.EIP_IRQ_MODE:%d\n",EIP_IRQ_MODE);
	INIT_BOOT("###6.TEST_RB_CTL:%d\n",TEST_RB_CTL);
	INIT_BOOT("##7.SPECIAL_DMA_Test:%d\n",SPECIAL_DMA_Test);
	INIT_BOOT("##8.PageBCH24 Read:%d,Write:%d\n",Flash_PAGE_R_BCH24,Flash_PAGE_W_BCH24);
	INIT_BOOT("##9.PageBCH8 Read:%d,Write:%d\n",Flash_PAGE_R_BCH8,Flash_PAGE_W_BCH8);
	INIT_BOOT("##10.pseudoNoise:%d\n",PSEUDO_NOISE_TEST);
	INIT_BOOT("mode32:0x%x,mode8:0x%x\n",FlashDMA_R_SDR_32,FlashDMA_R_SDR_8);
	INIT_BOOT("mode32:0x%x,mode8:0x%x\n",FlashDMA_W_SDR_32,FlashDMA_W_SDR_8);


	//init variable in sbss to 0
	init_sbss();

	return 0;
}



