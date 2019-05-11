/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
@file: am7x_spi.c

@abstract: actions-mircro spi controller driver source file.

@notice: Copyright (c), 2010-2015 Actions-Mirco Communications, Inc.
 *
 *  This program is develop for Actions-Mirco spi Control Device driver;
 *  	
 *
 *
 *
 *  The initial developer of the original code is scopengl
 *
 *  scopengl@gmail.com
 *
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>

#include <actions_io.h>
#include <actions_regs.h>

#include <linux/spi/am7x_spi.h>
#include <linux/delay.h>

#include "am7x_platform_device.h"
#include <am7x_board.h>


//#define DEBUG_SNOR

void reg_setbits(int val,int reg,int msb,int lsb)
{											 
	unsigned int mask = 0xFFFFFFFF;
	unsigned int old  = act_readl(reg);
	
	mask = mask << (31 - msb) >> (31 - msb + lsb) << lsb;
	act_writel((old & ~mask) | ((val << lsb) & mask), reg); 			
} 
void dump_spi_registers(void)
{
	int i=0;
	printk("cmu_devclken:%x\n",act_readl(CMU_DEVCLKEN));

	for(i=0;i<4;i++)
		printk("%x: %x\n",SPI_CTL+i*4,act_readl(SPI_CTL+i*4));
}


/*************************************************
*
*			am7x chip select/deselect 
*
**************************************************/

static void am7x_chip_select(int num)
{
	if(num == 1)
		reg_setbits(0,SPI_CTL,24,24);
	else if(num == 2)
		reg_setbits(0,SPI_CTL,23,23);
#if CONFIG_AM_CHIP_ID == 1213
	else if(num == 3)
		reg_setbits(1,GPIO_31_0DAT,22,22);						
#endif		
	else{
		printk("only %d spi chipselect supported\n",num);
	}
}

static void am7x_chip_deselect(int num)
{
	if(num == 1)
		reg_setbits(1,SPI_CTL,24,24);
	else if(num == 2)
		reg_setbits(1,SPI_CTL,23,23);
#if CONFIG_AM_CHIP_ID == 1213
	else if(num == 3)
		reg_setbits(0,GPIO_31_0DAT,22,22);		
#endif	
	else{
		printk("only %d spi chipselect supported\n",num);
	}	
}

static void am7x_set_spi_clkmode(int mode)
{
	reg_setbits(mode,SPI_CTL,10,9);
}

static void am7x_set_spi_bit_width(int width)
{
	switch(width)
	{
		case 8:
			reg_setbits(0,SPI_CTL,14,11);
			break;
		case 16:			
			reg_setbits(5,SPI_CTL,14,11);
			break;
		case 32:
			reg_setbits(10,SPI_CTL,14,11);
			break;
		default:
			printk("****error bit width:%d\n",width);
			break;
	}
	
}

static void am7x_set_spi_dmamode(int enable)  //fix later
{
	if(enable)  //to be finish later
		;
	else
		reg_setbits(12,SPI_CTL,22,19);
}

static void am7x_set_spi_workmode(int mode) //read write duplex
{
	reg_setbits(mode,SPI_CTL,16,15);
}

static void am7x_set_spi_irq(int enable)//fix later
{
	if(enable)
		;
	else
		reg_setbits(0,SPI_CTL,7,2);
	
}

static void am7x_set_spi_endian(int endian)
{
	if(endian == AM7X_SPI_MSB_FIRST)
		reg_setbits(0,SPI_CTL,8,8);	
	else
		reg_setbits(1,SPI_CTL,8,8);  //lsb first 			
}	

static unsigned int am7x_get_core_clk(void)
{
	unsigned int val = act_readl(CMU_COREPLL);

#if CONFIG_AM_CHIP_ID == 1203
	val >>= 2;
	val &= 0x3f;
	val *= 6;
#elif CONFIG_AM_CHIP_ID == 1211
	unsigned int spread = val&(1<<24);
	unsigned int bus_val = act_readl(CMU_BUSCLK);

	if(bus_val&(1<<11)){	//DDRPLL
		val = act_readl(CMU_DDRPLL);
#if CONFIG_AM_CHIP_MINOR == 8268
		val >>= 1;
		val &= 0x3f;
		val *= 24;
#else
		val >>=2;
		val &= 0x3f;
		val *= 8;
#endif
	}else{
#if CONFIG_AM_CHIP_MINOR == 8268
		val >>= 1;
		val &= 0x3f;
		val *= 24;
#else
		val >>= 2;
		val &= 0x3f;
		val *= 6;
#endif
	}

	if(spread)
		val += 6*6;	
	
#elif CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213 
	val >>= 2;
	val &= 0x7f;
	val *= 6;	
#endif

	return val;
}


unsigned int am7x_get_hclk_div(void)
{
#if CONFIG_AM_CHIP_ID == 1211

	unsigned int busclk_val=*(volatile unsigned int*)(CMU_BUSCLK);
	unsigned int core_clk;
	unsigned int hclk;

	core_clk = am7x_get_core_clk();

	if(busclk_val&(1<<4)){
		hclk = core_clk/2;
	}
	else{
		hclk = core_clk;
	}

	return hclk;
	
#elif CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213

	unsigned int busclk_val=*(volatile unsigned int*)(CMU_BUSCLK);
	unsigned int core_clk;
	unsigned int hclk;
	unsigned int core_clk_div;
	unsigned int core_clk_div_value =2;

	core_clk = am7x_get_core_clk();

	core_clk_div = (busclk_val>>28)&0x7;

	switch(core_clk_div)
	{
		case 1: 
			core_clk_div_value=2; 
			break;
		case 2: 
			core_clk_div_value=5; 
			break;
		case 3: 
			core_clk_div_value=6;
			break;
		case 4: 
			core_clk_div_value=3; 
			break;
		case 5: 
			core_clk_div_value=7; 
			break;
		case 6: 
			core_clk_div_value=8; 
			break;
		default:
			core_clk_div_value=2; 
			break;
	}

	if(busclk_val&(1<<3)){
		hclk = core_clk*2/core_clk_div_value;
	}
	else{
		hclk = core_clk;
	}

	return hclk;
	
#else

	return 180;
	
#endif 
}


static void am7x_set_spi_clk(int clk)
{
	int div = 0;
	int hclk = 180;

	hclk = am7x_get_hclk_div();	
/*	switch(clk)
	{
		case 1:
			div = hclk/2-1;
			break;
		case 2:
			div = 44;
			break;	
		case 4:	
			div = 22;
			break;
		default:
			div = 44;
			break;
	}*/


	/**
	* The actual formula is "div = hclk/(2*clk)-1",but
	* for integer operation,the frequency will not be the 
	* exact value we want. So we just let it be lower than 
	* that.
	*/
	div = hclk/(2*clk);
	
	act_writel(div,SPI_CLKDIV);

	printk("---------hclk is %d,spi div is %d\n",hclk,div);
	
}


static void am7x_spi_reset_fifo(void)
{
	unsigned int cfg;

	cfg = act_readl(SPI_STAT);
	cfg |= 0xc;
	act_writel(cfg,SPI_STAT);	  //clear SPI status register
}

static void spi_writel(struct am7x_spi* as,unsigned int reg,unsigned int val)
{	
	act_writel(val,reg);
}

/** init hareware for actions spi controller **/
int am7x_init_spi(struct am7x_spi* as)
{   
	//pad control   
	spin_lock(&as->lock);
	spi_writel(as,CMU_DEVRST,act_readl(CMU_DEVRST)|1<<SPI_RESET_BIT);
	spi_writel(as,CMU_DEVCLKEN,act_readl(CMU_DEVCLKEN)|1<<SPI_CLK_BIT);	
	
	am7x_set_spi_endian(AM7X_SPI_MSB_FIRST); //msb first
	am7x_set_spi_irq(0);  //diable spi irq
	am7x_set_spi_dmamode(0); //disable dmamode

	reg_setbits(1,SPI_CTL,1,1);
	reg_setbits(63,SPI_CTL,7,2);
	reg_setbits(3,SPI_CTL,18,17);	
	reg_setbits(0,SPI_CTL,27,27); //tx_def when in read_only mode
	spin_unlock(&as->lock);
	return 0;
}
/** disable hardware for actions spi controller **/
int am7x_diable_spi(struct am7x_spi* as)
{
	//pad recover
	spin_lock(&as->lock);
	spi_writel(as,CMU_DEVRST,~(1<<SPI_RESET_BIT));
	spi_writel(as,CMU_DEVCLKEN,~(1<<SPI_CLK_BIT));		
	spin_unlock(&as->lock);	
	return 0;
}
/*
static void am7x_spi_chipsel(struct spi_device *spi, int value)
{
	struct s3c24xx_spi *hw = to_hw(spi);
	unsigned int cspol = spi->mode & SPI_CS_HIGH ? 1 : 0;
	unsigned int spcon;

	switch (value) {
	case BITBANG_CS_INACTIVE:
		hw->set_cs(hw->pdata, spi->chip_select, cspol^1);
		break;

	case BITBANG_CS_ACTIVE:
		spcon = readb(hw->regs + S3C2410_SPCON);

		if (spi->mode & SPI_CPHA)
			spcon |= S3C2410_SPCON_CPHA_FMTB;
		else
			spcon &= ~S3C2410_SPCON_CPHA_FMTB;

		if (spi->mode & SPI_CPOL)
			spcon |= S3C2410_SPCON_CPOL_HIGH;
		else
			spcon &= ~S3C2410_SPCON_CPOL_HIGH;

		spcon |= S3C2410_SPCON_ENSCK;

		

		writeb(spcon, hw->regs + S3C2410_SPCON);
		hw->set_cs(hw->pdata, spi->chip_select, cspol);

		break;
	}
}
*/

inline int  am7x_spi_write_byte(unsigned int cmd,int timeout)
{
	int status;

	act_writel(0,SPI_SIZE);
	act_writel(cmd,SPI_TXDAT);
	act_writel(act_readl(SPI_CTL)|0x1,SPI_CTL);

	while(1){
		status = act_readl(SPI_CTL);
		if((status&0x1) == 0)
			break;
		
		if(--timeout <=0)
			return SPI_TIMEOUT;
	}
   	return 0;
}

inline int am7x_spi_read_byte(unsigned char* byte,int timeout)
{
	int ret= am7x_spi_write_byte(0,timeout);
	*byte = act_readl(SPI_RXDAT);
	return ret;
}

/** get hardware bus */
static inline void spi_get_bus(struct spi_device *spi)
{
	am_get_multi_res(FLASH_CARD);
	/** mfp for am1211 */
#if CONFIG_AM_CHIP_ID==1211	
	//reg_setbits(2,GPIO_MFCTL3,21,20); //NSS
	reg_setbits(6,GPIO_MFCTL2,11,8);  //NSS2
 	reg_setbits(0,GPIO_MFCTL3,6,4); //SPI MISO
	reg_setbits(1,GPIO_MFCTL3,10,8); //SPI CLK
	reg_setbits(5,GPIO_MFCTL7,18,16); // MOSI 1211 B ver IC definition 
#elif CONFIG_AM_CHIP_ID==1213
	reg_setbits(2,GPIO_MFCTL3,11,10);  //P_SPI0CLK
 	reg_setbits(2,GPIO_MFCTL3,14,12); //P_SPI0MOSI
	reg_setbits(2,GPIO_MFCTL3,17,15); //P_SPI0MISO
	//reg_setbits(2,GPIO_MFCTL3,20,18); //P_SPI0HOLD
	//reg_setbits(2,GPIO_MFCTL3,23,21); //P_SPI0WP
	reg_setbits(2,GPIO_MFCTL3,25,24); //P_SPI0NSS


	
	
//	reg_setbits(0,GPIO_MFCTL0,17,17); //SPI CS
//	reg_setbits(0,GPIO_31_0INEN,22,22);
//	reg_setbits(1,GPIO_31_0OUTEN,22,22);
#endif

	am7x_chip_select(spi->chip_select);
	am7x_spi_reset_fifo();
}
/** release hardware bus  */
static inline void spi_put_bus(struct spi_device *spi)
{
	am7x_spi_reset_fifo();
	//am7x_set_spi_workmode(SPI_DUPLEX_MODE);
	am7x_chip_deselect(spi->chip_select);	
	am_release_multi_res(FLASH_CARD);
}

static int am7x_spi_do_tranfer(struct spi_device *spi,struct spi_transfer *xfer)
{
	int ret = 0;
	int i;
	int timeout = 0xffffff;
	unsigned char* p = NULL;
	
	if(xfer->tx_buf){ //write 
		am7x_set_spi_workmode(SPI_WRITE_MODE);
		//act_writel(xfer->len-1,SPI_SIZE);
		p = (unsigned char*)xfer->tx_buf;
		for(i=0;i<xfer->len;i++)
			ret = am7x_spi_write_byte(*p++,timeout);
	}
	else if(xfer->rx_buf){//read 
		am7x_set_spi_workmode(SPI_READ_MODE);
		act_writel(xfer->len-1,SPI_SIZE);
		act_writel(act_readl(SPI_CTL)|1,SPI_CTL);
		p = (unsigned char*)xfer->rx_buf;
		for(i=0;i<xfer->len;){
			if((act_readl(SPI_STAT)&0x20) ==0){			
				*p++ = act_readl(SPI_RXDAT);
				i++;
			}
			else
			{
				if(--timeout <=0){
					printk("[ am7x_spi_do_tranfer ] read timeout, i=%d\n",i);
					return SPI_TIMEOUT;
				}
			}
		}
	}
	return ret;
}



static int am7x_spi_setup(struct spi_device *spi)
{
	struct am7x_spi	*as = NULL;
	int bit_width;
	/*** hareware setup***/
	as = spi_master_get_devdata(spi->master);
	if(!spi->bits_per_word)
		bit_width = as->bits_per_word;
	else
		bit_width = spi->bits_per_word;
	spi_get_bus(spi);
	
	am7x_set_spi_clkmode(spi->mode);
	am7x_set_spi_bit_width(bit_width);
	am7x_set_spi_clk(spi->max_speed_hz);

	spi_put_bus(spi);
	
	return 0;
}

static int am7x_spi_transfer(struct spi_device *spi,struct spi_message *mesg)
{
	struct spi_transfer	*xfer;
	int ret =0;
	spi_get_bus(spi);
	


	mesg->actual_length = 0;
	list_for_each_entry(xfer, &mesg->transfers, transfer_list) {
		if (!(xfer->tx_buf || xfer->rx_buf) && xfer->len) {
			dev_dbg(&spi->dev, "missing rx or tx buf\n");
			mesg->actual_length=0;
			return -EINVAL;
		}
#if 0
		/* FIXME implement these protocol options!! */
		if (xfer->bits_per_word || xfer->speed_hz) {
			dev_dbg(&spi->dev, "no protocol options yet\n");
			return -ENOPROTOOPT;
		}
#endif	
//		act_writel(0x0c | act_readl(SPI_STAT),SPI_STAT);
		ret = am7x_spi_do_tranfer(spi,xfer);
		if(ret){
			mesg->actual_length=0;
			goto out;
		}else{
			mesg->actual_length +=xfer->len;
		}
	}
	
out:
	
	mesg->complete(mesg->context);
	spi_put_bus(spi);
	return ret;	
}
						
static void am7x_spi_cleanup(struct spi_device *spi)
{
	/**hareware release**/
	am7x_spi_reset_fifo();
	am7x_chip_deselect(spi->chip_select);
	
	/**release hardware bus**/
	
}

static int __init	am7x_spi_probe(struct platform_device* pdev)
{
	int			ret = 0;
	struct spi_master	*master = NULL;
	struct am7x_spi	*as = NULL;
	struct resource		*regs = NULL;	
	
	regs =  platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!regs)
		return -ENXIO;
	master = spi_alloc_master(&pdev->dev, sizeof *as);
	if (!master){
		ret = -ENOMEM;	
		goto alloc_master_err;
	}
	master->bus_num = 0;  //only one spi controller bus for am7x 7555 IC
	master->num_chipselect = 4; // 
	master->setup = am7x_spi_setup;
	master->transfer = am7x_spi_transfer;
	master->cleanup = am7x_spi_cleanup;
	platform_set_drvdata(pdev, master);
	as = spi_master_get_devdata(master);
	
	/** init am7x_spi */
	spin_lock_init(&as->lock);
	as->pdev = pdev;
	as->bits_per_word = 8; /*default 8 bit width*/
	/** init hardware */
	am7x_init_spi(as);
	ret = spi_register_master(master);
	if(ret)
		goto register_master_err;
	
	printk("am7x spi probe successs\n");
register_master_err:
			
alloc_master_err:
	return ret;	
}

static int __exit am7x_spi_remove(struct platform_device* pdev)
{
	return 0;
}

static int am7x_spi_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int am7x_spi_resume(struct platform_device* pdev)
{
	return 0;
}

static struct platform_driver am7x_spi_driver = {
	.driver		= {
		.name	= "am7x_spi",
		.owner	= THIS_MODULE,
	},
	.suspend	= am7x_spi_suspend,
	.resume		= am7x_spi_resume,
	.remove		= __exit_p(am7x_spi_remove),
};

#ifdef DEBUG_SNOR

#define RDID		0x90

/** for snor device **/
static struct spi_board_info am7x_spi_board_info[]  = {
	[0] = {
		.modalias = "am7x_snor",
		.platform_data = NULL,
		.bus_num = 0,	
		.chip_select = 1,	
		.mode = 0,
		.max_speed_hz =10,
	},
	
};

void read_snor_chipid(void)
{
	char cmd = RDID;
	int timeout =  100000000;
	unsigned char ID[4];
	int i;
	act_writel(act_readl(CMU_DEVRST)|1<<SPI_RESET_BIT,CMU_DEVRST);
	act_writel(act_readl(CMU_DEVCLKEN)|1<<SPI_CLK_BIT,CMU_DEVCLKEN);	

	act_writel(5,SPI_CLKDIV);
	act_writel(0x1e18002,SPI_CTL);

#if CONFIG_AM_CHIP_ID == 1211	
	reg_setbits(2,GPIO_MFCTL3,21,20); //NSS
	//reg_setbits(6,GPIO_MFCTL2,11,8);	//NSS2
	reg_setbits(0,GPIO_MFCTL3,6,4); //SPI MISO
	reg_setbits(1,GPIO_MFCTL3,10,8); //SPI CLK
	//reg_setbits(3,GPIO_MFCTL4,1,0); // MOSI 
	//reg_setbits(4,GPIO_MFCTL4,10,8); //MOSI[1]
	reg_setbits(5,GPIO_MFCTL7,18,16);//MOSI[1]
#endif
	
	am7x_chip_select(1);

	am7x_spi_write_byte(cmd,timeout);
	for(i=0;i<3;i++){ 
		am7x_spi_read_byte(&ID[i],timeout);
		printk("%x\n",ID[i]);
	}
	printk("test snor end\n");
	dump_spi_registers();
	
}
#endif

static int __init am7x_spi_init(void)
{
#ifndef  DEBUG_SNOR
 	return  platform_driver_probe(&am7x_spi_driver, am7x_spi_probe);	 
#else
	spi_register_board_info(am7x_spi_board_info,ARRAY_SIZE(am7x_spi_board_info));
	read_snor_chipid();
	return 0;
#endif	
}
module_init(am7x_spi_init);

static void __exit am7x_spi_exit(void)
{
	platform_driver_unregister(&am7x_spi_driver);
}
module_exit(am7x_spi_exit);

MODULE_DESCRIPTION("Action Mirco SPI Controller driver");
MODULE_AUTHOR("Zeng Tao <scopengl@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:am7x_spi");



