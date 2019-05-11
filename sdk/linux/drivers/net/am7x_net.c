#include "am7x_net.h"

/**
*************************************************************
* proc file system operations.
*************************************************************
*/
#ifndef INVALID
#define INVALID  0
#endif

#define  WRITE_MAC_REP    5       //write new mac address to eeprom repeat times if failed
#define PHY_STAT_FILENAME "driver/am7x_net_phy_stat"
struct proc_dir_entry *phy_stat_pe = NULL;
int net_link_status=0;
struct net_device *dev_tmp=NULL;
static struct  net_device  * net_device_tmp=NULL; 
static unsigned int    init_flag = 0;
#if(ENABLE_8201F_WOL_MODE)
static unsigned int gphy_reg=0;
static unsigned int gphy_page=0;
#endif

struct mac_addr_info{
	unsigned int  flag;
	mac_address_t mac_val;
};

struct mac_addr_info mac_in_eeprom={
	.flag = INVALID,
	.mac_val ={0,0,0,0,0,0,},
};

struct mii_ioctl_data_tmp {
	__u16		phy_id;
	__u16		reg_num;
	__u16		val_in;
	__u16		val_out;
};

#if EEPROM_TYPE==1

#endif

#if(ENABLE_8201F_WOL_MODE)
static int am7x_wol_test(struct net_device *dev);
static void am7x_wol_test_restore(struct net_device *dev);
static void am7x_set_phy_reg_value(struct net_device *dev, unsigned int reg);
static void am7x_set_phy_reg(unsigned int reg);
static void am7x_get_phy_reg(struct net_device *dev, unsigned long reg);
static void am7x_reset_phy(void);
#endif
// 2016/12/20, T.R, EN/DIS EEE funciton, it disabled EEE funciton when install net driver, user can use ioctl to enable it
// ref to RTL8201(F_FN_FL_FR)-VB_(R)MII_Behavior_in_EEE_App_Note_1.0 (Actions Microelectronics)  T3342
static void am7x_disable_EEE(struct net_device *dev);
static void am7x_enbable_EEE(struct net_device *dev);


static inline am7x_board_info_t *to_am7x_board(struct net_device *dev)
{
	return netdev_priv(dev);
}
/*
int check_netlinkstatus(){
while(1){
	mdelay(5000);
	struct ifreq ifr;
	struct mii_ioctl_data *mii_data;
    memset( &ifr,0, sizeof( ifr ) );
    strncpy( ifr.ifr_name, "eth0", 15 );
    ifr.ifr_name[15] = 0;
	am7x_board_info_t *b_info=to_am7x_board(dev_tmp);
	b_info->mii.reg_num_mask = 0x01;	
	mii_data->phy_id &= b_info->mii.phy_id_mask;
	mii_data->reg_num &= b_info->mii.reg_num_mask;

	mii_data->phy_id = b_info->mii.phy_id;
	mii_data->val_out =	b_info->mii.mdio_read(b_info->mii.dev, mii_data->phy_id,
					  mii_data->reg_num);
	if(mii_data->val_out& 0x0004&&mii_data->val_out!=65535 ){
			   net_link_status=1;
	 }
	 else{
			    net_link_status=0;
	 	}
			
}		

}
*/
int am7x_net_proc_show(struct seq_file *sf, void *data)
{
	struct net_device *dev=NULL;

	if(sf){
		dev = (struct net_device *)sf->private;
	}
	
	if(dev){
		seq_printf(sf,"%d",net_link_status);
	}

	return 0;
}


static int am7x_net_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, am7x_net_proc_show, PDE(inode)->data);
}


static const struct file_operations am7x_net_proc_ops = {
	.owner		= THIS_MODULE,
	.open		= am7x_net_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release    = single_release,
};

static void am7x_net_proc_create(struct net_device *ndev)
{
	phy_stat_pe = proc_create_data(PHY_STAT_FILENAME, 0, NULL, &am7x_net_proc_ops, ndev);
	if(phy_stat_pe ==NULL){
		printk("--- am7x net proc create error\n");
	}
	else{
		printk("--- am7x net proc create successfully\n");
	}
}

static void am7x_net_proc_destroy(void)
{
	if (phy_stat_pe){
		remove_proc_entry(PHY_STAT_FILENAME, NULL);
		phy_stat_pe = NULL;
	}
	return;
}


static void am7x_mac_reset(struct net_device *dev);
#if 0
static void dump_buf(unsigned char * data,int len)
{
	int i=0;
	//pr_info("data===0x%x,len==0x%x\n",(unsigned int)data,len);
	if(data==NULL)
		return;
	for(i=0;i<len;i++){
		if(i!=0 && i%16==0)
			printk("\n");
		printk("0x%02x ",data[i]);
	}
	printk("\n");
}

#endif


//#include <linux/skbuff.h>


static void am7x_net_msleep(unsigned int ms)
{
	//msleep(ms);
	mdelay(ms);
}
#if 0
static void dump_register(void)
{
	unsigned int value;
	value = am7x_readl(CMU_DEVCLKEN);
	//pr_info("%s,%d:CMU_DEVCLKEN==0x%x\n",__func__,__LINE__,value);

	value = am7x_readl(INTC_PD);
	//pr_info("%s,%d:INTC_PD==0x%x\n",__func__,__LINE__,value);

	value = am7x_readl(INTC_MSK);
	//pr_info("%s,%d:INTC_MSK==0x%x\n",__func__,__LINE__,value);


	value = am7x_readl(INTC_CFG0);
	//pr_info("%s,%d:INTC_CFG0==0x%x\n",__func__,__LINE__,value);
	value = am7x_readl(INTC_CFG0H);
	//pr_info("%s,%d:INTC_CFG0H==0x%x\n",__func__,__LINE__,value);
	
	value = am7x_readl(INTC_CFG1);
	//pr_info("%s,%d:INTC_CFG1==0x%x\n",__func__,__LINE__,value);
	value = am7x_readl(INTC_CFG1H);
	//pr_info("%s,%d:INTC_CFG1H==0x%x\n",__func__,__LINE__,value);
	
	value = am7x_readl(INTC_CFG2);
	//pr_info("%s,%d:INTC_CFG2==0x%x\n",__func__,__LINE__,value);
	value = am7x_readl(INTC_CFG2H);
	//pr_info("%s,%d:INTC_CFG2H==0x%x\n",__func__,__LINE__,value);
	
	value = am7x_readl(INTC_EXTCTL01);
	//pr_info("%s,%d:INTC_EXTCTL01==0x%x\n",__func__,__LINE__,value);
	value = am7x_readl(INTC_EXTCTL23);
	//pr_info("%s,%d:INTC_EXTCTL23==0x%x\n",__func__,__LINE__,value);


}
#endif
static void am7x_mac_dump_register(struct net_device *dev)
{
	int i=0;
	unsigned int value=0;

	printk("---dump mac reg:\n");
	for(i=0;i<28;i++){
		value = am7x_readl(dev->base_addr+i*4);
		printk("0x%x:0x%x\n",i*4,value);
	}
}


static void am7x_mac_config_mulitfunc(void)
{
	unsigned int value=0;
	
#if CONFIG_AM_CHIP_ID == 1220
	value = MAC_TD3|MAC_TD2|MAC_TD1|MAC_TD0|MAC_TVLD|MAC_TCLK|MAC_RD0|MAC_RD1|MAC_RD2|MAC_RD3|MAC_MDC|MAC_MDIO;
	//pr_info("%s,%d:MFC6==0x%x\n",__func__,__LINE__,value);
	am7x_writel(value,GPIO_MFCTL6);

	value = am7x_readl(GPIO_MFCTL4);
	value = (value&0xFE01FFFF)|MAC_TER|MAC_RER|MAC_RCLK|MAC_RVLD;
	//pr_info("%s,%d:MFC4==0x%x\n",__func__,__LINE__,value);
	am7x_writel(value,GPIO_MFCTL4);

	value = am7x_readl(GPIO_MFCTL3);
	value = value&~(3<<23);					///set the rstn_fpga pin as gpio for reset the phy
	//pr_info("%s,%d:MFC3==0x%x\n",__func__,__LINE__,value);
	am7x_writel(value,GPIO_MFCTL3);
#elif CONFIG_AM_CHIP_ID == 1213
	value = am7x_readl(GPIO_MFCTL1);
	value = (value&MAC_TD3_MASK)|MAC_TD3;
	value = (value&MAC_TD2_MASK)|MAC_TD2;
	value = (value&MAC_TD1_MASK)|MAC_TD1;
	value = (value&MAC_TD0_MASK)|MAC_TD0;
	value = (value&MAC_RD0_MASK)|MAC_RD0;
	value = (value&MAC_RD1_MASK)|MAC_RD1;
#ifdef CONFIG_AM_8251	
	am7x_writel((am7x_readl(GPIO_MFCTL2) & (~(0x03<<29)))|(0x02<<29),GPIO_MFCTL2);
	am7x_writel((am7x_readl(GPIO_MFCTL3) & (~(0x03)))|(0x02),GPIO_MFCTL3);
	pr_info("%s,%d:MFC2=0x%x,MFC3=0x%x\n",__func__,__LINE__,am7x_readl(GPIO_MFCTL2),am7x_readl(GPIO_MFCTL3));
#else
	value = (value&MAC_RD2_MASK)|MAC_RD2;
	value = (value&MAC_RD3_MASK)|MAC_RD3;
#endif
	value = (value&MAC_MDC_MASK)|MAC_MDC;
	value = (value&MAC_MDIO_MASK)|MAC_MDIO;
	value = (value&MAC_TVLD_MASK)|MAC_TVLD;
	am7x_writel(value,GPIO_MFCTL1);
	pr_info("%s,%d:MFC1==0x%x\n",__func__,__LINE__,am7x_readl(GPIO_MFCTL1));
#endif
	
	

}

static inline void am7x_mac_enable_transmit(struct net_device *dev)
{
	unsigned int value=am7x_readl(dev->base_addr+MAC_CtlReg);
	ctl_reg_info_t *ctl_reg=(ctl_reg_info_t *)&value;
	
	ctl_reg->tx_en = 1;
	am7x_writel(value,dev->base_addr+MAC_CtlReg);
}

static inline void am7x_mac_disable_transmit(struct net_device *dev)
{
	unsigned int value=am7x_readl(dev->base_addr+MAC_CtlReg);
	ctl_reg_info_t *ctl_reg=(ctl_reg_info_t *)&value;
	
	ctl_reg->tx_en = 0;
	
	am7x_writel(value,dev->base_addr+MAC_CtlReg);
}


static inline void am7x_mac_enable_receive(struct net_device *dev)
{
	ipg_reg_info_t *ipg_reg;
	ctl_reg_info_t *ctl_reg;
	unsigned int value=am7x_readl(dev->base_addr+MAC_CtlReg);
	
	ctl_reg=(ctl_reg_info_t *)&value;
	ctl_reg->rx_en = 1;

	//pr_info("%s,%d:RX Enable==0x%x\n",__func__,__LINE__,value);
	am7x_writel(value,dev->base_addr+MAC_CtlReg);

	value = am7x_readl(dev->base_addr+MAC_InterPacketGap);
	ipg_reg = (ipg_reg_info_t *)&value;
	ipg_reg->rx_pre_min = 0;
	ipg_reg->rx_pre_max = 0x10;
	ipg_reg->ipgt = 0;
	am7x_writel(value,dev->base_addr+MAC_InterPacketGap);
	//pr_info("%s,%d:MAC_InterPacketGap==0x%x\n",__func__,__LINE__,value);
	
}
static inline void am7x_mac_disable_receive(struct net_device *dev)
{
	ctl_reg_info_t *ctl_reg;
	unsigned int value=am7x_readl(dev->base_addr+MAC_CtlReg);
	ctl_reg=(ctl_reg_info_t *)&value;
	
	ctl_reg->rx_en = 0;

	am7x_writel(value,dev->base_addr+MAC_CtlReg);
}

static void am7x_mac_config_mac_ctl(struct net_device *dev)
{
	ctl_reg_info_t *ctl_reg=NULL;
	unsigned int value=am7x_readl(dev->base_addr+MAC_CtlReg);
	
	ctl_reg = (ctl_reg_info_t*)&value;
	//ctl_reg->loop_en = 1;
	ctl_reg->addr_check = 1;
#if 0 //[Sanders.130507] - Use default configurations.
	ctl_reg->bro =1;
	ctl_reg->huge_en =1;
	ctl_reg->r_gap_cnt = 0;
	//ctl_reg->rx_flow =0;
	ctl_reg->tx_error_en = 1;
	ctl_reg->tx_flow = 0;
#else
	ctl_reg->pad = 1; //Add pads to short frames.
#endif
	am7x_writel(value,dev->base_addr+MAC_CtlReg);
	//pr_info("%s,%d:MAC_CtlReg==0x%x\n",__func__,__LINE__,value);

	/** set IPGT to 0 */
	//am7x_writel(am7x_readl(dev->base_addr+MAC_InterPacketGap)&(~(0x7f<<16)),dev->base_addr+MAC_InterPacketGap);

#if 1 //[Sanders.130507] - Tx/Rx Buffer Descriptors has maximum 0x7ff byte_len.
	value = am7x_readl(dev->base_addr + MAC_PackageLen);
	value |= 0x7ff;
	am7x_writel(value, dev->base_addr + MAC_PackageLen);
#endif
}


static inline void am7x_mac_enable_inttrupt(struct net_device *dev)
{
	unsigned int value=0;
	unsigned int int_value=0;
	int_reg_info_t * int_reg=(int_reg_info_t *)&int_value;
	
	value = am7x_readl(INTC_PD);
	set_bit(6,value);
	am7x_writel(value,INTC_PD);

	value=am7x_readl(INTC_MSK);
	set_bit(6,value);
	am7x_writel(value,INTC_MSK);

	/** currently open all intr */
	int_value=am7x_readl(dev->base_addr+MAC_Interrupt);
	int_reg->txb_m = 1;
	int_reg->txe_m = 1;
	int_reg->rxb_m = 1;
	int_reg->rxe_m = 1;
	int_reg->busy_m = 1;
	int_reg->txc_m = 1;
	int_reg->rxc_m = 1;
	int_reg->swbdad_m = 1;
	am7x_writel(int_value,dev->base_addr+MAC_Interrupt);
	
}

static inline void am7x_mac_disable_inttrupt(void )
{
	unsigned int value=am7x_readl(INTC_MSK);
	clear_bit(6,value);
	am7x_writel(value,INTC_MSK);
}


static void am7x_mac_set_tx_bd_num(struct net_device *dev,unsigned int tx_bd_num)
{
	unsigned int value=am7x_readl(dev->base_addr+MAC_CtlReg);

	if(tx_bd_num != MAC_TX_BD_NUM){
		printk("set error tx_bd_num\n");
		return;
	}
	
	value = (value & ~0x7f)|tx_bd_num;
	am7x_writel(value,dev->base_addr+MAC_CtlReg);
}

static unsigned int am7x_mac_get_tx_bd_num(struct net_device *dev)
{
	unsigned int value=am7x_readl(dev->base_addr+MAC_CtlReg);
	
	return value&0x7f;
}

static void am7x_mac_set_sdr_base_addr(struct net_device *dev)
{
	am7x_board_info_t *b_info=to_am7x_board(dev);
	unsigned int base_addr=0;
	unsigned int value;
	
	base_addr = (unsigned int)b_info->bd_phy_addr>>20&0xfff;
	/** set the sdr base addr, RX_SDR_base addr is the same as TX_SDR_base addr */
	value =base_addr <<16 | base_addr;			
	am7x_writel(value,dev->base_addr+MAC_SDRBaseAddr);
	//pr_info("%s,%d:SDR Base Addr==0x%x\n",__func__,__LINE__,value);
}


static void am7x_mac_set_swap_addr(struct net_device *dev,bd_swap_addr_t *swap_addr)
{
//[Sanders.130507] - Useless read.
	unsigned int value;//=am7x_readl(dev->base_addr+MAC_BDSwapAddr);
	if(swap_addr->rx_swap_addr <= swap_addr->tx_swap_addr){
		printk(KERN_ERR"Set Swap addr error rx addr=0x%x,tx addr=0x%x\n",swap_addr->rx_swap_addr,swap_addr->tx_swap_addr);
		return;
	}
	value= swap_addr->rx_swap_addr << 16 | swap_addr->tx_swap_addr;

	
	am7x_writel(value,dev->base_addr+MAC_BDSwapAddr);
}
#if 0
static int am7x_mac_get_swap_addr(struct net_device *dev,bd_swap_addr_t *swap_addr)
{
	unsigned int value=am7x_readl(dev->base_addr+MAC_BDSwapAddr);
	if(swap_addr){
		swap_addr->rx_swap_addr = (value>>16)&0x7f;
		swap_addr->tx_swap_addr = value &0x7f;
		return 0;
	}
	return -1;
}

#endif
static int am7x_mac_get_bd_pointer(struct net_device *dev,bd_pointer_t *bd_pointer,bd_type_e bd_type)
{
	unsigned int value=0;
	
	if(bd_type==BD_TX){
		value = am7x_readl(dev->base_addr+MAC_TXBDPtr);
	}
	else if(bd_type==BD_RX){
		value = am7x_readl(dev->base_addr+MAC_RXBDPtr);
	}
	else{
		//pr_info("bd_type error!");
		return -1;
	}
	//pr_info("value ====== 0x%x\n",value);

	if(bd_pointer){
		bd_pointer->read_ptr= (value>>16)&0x7f;
		
		//pr_info("bd_pointer->read_ptr ====== 0x%x\n",bd_pointer->read_ptr);
		bd_pointer->read_around = (value>>23)&1;
		//pr_info("bd_pointer->read_around ====== 0x%x\n",bd_pointer->read_around);
		bd_pointer->write_ptr = value &0x7f;
		//pr_info("bd_pointer->write_ptr ====== 0x%x\n",bd_pointer->write_ptr);
		bd_pointer->write_around = (value>>7)&1;
		//pr_info("bd_pointer->write_around ====== 0x%x\n",bd_pointer->write_around);
		return 0;
	}
	return -1;
}


/**
* using this funciton to copy the package to the sdram for transmition
*/

static int copy_to_sdram(struct net_device *dev,int bd_idx,bd_type_e bd_type,int data_len,char* data)
{
	unsigned int sdram_addr=0;
	am7x_board_info_t *b_info=to_am7x_board(dev);
	
	if(data_len>=MTU_MAX_BUF_LEN){
		printk(KERN_ERR"Sorry Data  len=%d, MAX==2048\n",data_len);
		return -1;
	}
	
	if(bd_type==BD_TX){
		sdram_addr = (unsigned int)b_info->bd_mem_addr+bd_idx*MTU_MAX_BUF_LEN;
	}
	else{
		printk(KERN_ERR"bd_type is error!should be BD_TX,bd_type==%d",bd_type);
		return -1;
	}
	
	////pr_info("%s,%d:Transmit DATA  bd_idx=%d, sdram_addr=0x%x\n",__func__,__LINE__,bd_idx,sdram_addr);
	if(data){
		memcpy((unsigned char*)sdram_addr,data,data_len);
		dma_cache_wback_inv(sdram_addr,data_len);  //don't forget to flush the cache 
	}
	////pr_info("%s,%d:Transmit DATA\n",__func__,__LINE__);
	//dump_buf((unsigned char*)sdram_addr,data_len);
	return 0;
}


/**
* using this function to copy the redeived data from sdram to the package
*/
static int copy_from_sdram(struct net_device *dev,int bd_idx,bd_type_e bd_type,int data_len,char* data)
{
	unsigned int sdram_addr=0;
	am7x_board_info_t *b_info=to_am7x_board(dev);
	
	if(data_len>=MTU_MAX_BUF_LEN){
		printk(KERN_ERR"Sorry Data  len=%d, MAX==2048\n",data_len);
		return -1;
	}
	
	if(bd_type==BD_RX){
		sdram_addr = (unsigned int)b_info->bd_mem_addr+(bd_idx+b_info->tx_bd_num)*MTU_MAX_BUF_LEN;
	}
	else{
		printk(KERN_ERR"bd_type is error!should be BD_TX,bd_type==%d",bd_type);
		return -1;
	}
	
	////pr_info("%s,%d: Receive DATA SDRAM_ADDR=0x%x\n",__func__,__LINE__,sdram_addr);
	if(data){
		memcpy(data,(unsigned char*)sdram_addr,data_len);
	}
	else{
		return -1;
	}

	return 0;
}

/**
* set the bd descriptor
*/
static int am7x_mac_set_bd_descriptor(struct net_device *dev,int bd_idx,unsigned int byte_len,bd_type_e bd_type)
{
	unsigned int value=0;
	//unsigned int check_value;
	am7x_board_info_t *b_info=to_am7x_board(dev);
	
	if(byte_len>=2048){
		printk(KERN_ERR"Sorry Byte len=%d, MAX==2048\n",byte_len);
		return -1;
	}
	
	if(bd_type==BD_TX){
		
		if(bd_idx>=b_info->tx_bd_num){
			printk(KERN_ERR"Sorry set TX BD Idx=0x%x, Max===0x%x\n",bd_idx,b_info->tx_bd_num);
			return -1;
		}
		
//[Sanders.130507] - Useless read.
//		value=am7x_readl(dev->base_addr+MAC_BDDescription+4*bd_idx);
		value = (byte_len<<21) |( ((unsigned int)b_info->bd_phy_addr&0xfffff)+bd_idx*MTU_MAX_BUF_LEN);
		
		//pr_info("%s,%d:TX BD value==0x%x,offset==0x%x,addr=0x%x\n",__func__,__LINE__,value,4*bd_idx,dev->base_addr+MAC_BDDescription+4*bd_idx);
		am7x_writel(value,dev->base_addr+MAC_BDDescription+4*bd_idx);

		/** check write ok */
//[Sanders.130507] - Can we skip this?
//		do{
//			check_value = am7x_readl(dev->base_addr+MAC_BDDescription+4*bd_idx);
//		}while(((check_value>>21)&0x7ff)!=byte_len);		
	}

	/** 
	* if the bd_type is BD_RX, the byte_len is useless, 
	* because it is modified by the hardware itself 
	*/
	if(bd_type==BD_RX){
		
		if( (bd_idx+b_info->tx_bd_num) >= MAC_BD_TOTALNUM){
			printk(KERN_ERR"Sorry set RX BD Idx=0x%x, Max===0x%x\n",bd_idx,MAC_BD_TOTALNUM-b_info->tx_bd_num);
			return -1;
		}
//[Sanders.130507] - Useless read.
//		value=am7x_readl(dev->base_addr+MAC_BDDescription+4*(bd_idx+b_info->tx_bd_num));
		value = (byte_len<<21) | (((unsigned int)b_info->bd_phy_addr&0xfffff)+(bd_idx+b_info->tx_bd_num)*MTU_MAX_BUF_LEN);

		////pr_info("%s,%d:RX BD Des value==0x%x,offset==0x%x\n",__func__,__LINE__,value,4*(bd_idx+b_info->tx_bd_num));
		am7x_writel(value,dev->base_addr+MAC_BDDescription+4*(bd_idx+b_info->tx_bd_num));
	}
	
	return 0;
}

/**
* get bd descriptor, this is used by getting rx bd descriptor
*/
static int am7x_mac_get_bd_descriptor(struct net_device *dev,int bd_idx,unsigned int *byte_len,bd_type_e bd_type)
{
	unsigned int value=0;
	am7x_board_info_t *b_info=to_am7x_board(dev);
	
	if(bd_type==BD_TX){
		if(bd_idx>b_info->tx_bd_num){
			printk(KERN_ERR"Sorry get TX BD Idx=0x%x, Max===0x%x\n",bd_idx,b_info->tx_bd_num);
			return -1;
		}
		value=am7x_readl(dev->base_addr+MAC_BDDescription+4*bd_idx);
		////pr_info("%s,%d:bd descriptor TX==0x%x\n",__func__,__LINE__,value);
	}
	else if(bd_type==BD_RX){
		if( (bd_idx+b_info->tx_bd_num) >= MAC_BD_TOTALNUM){
			printk(KERN_ERR"Sorry get RX BD Idx=0x%x, Max===0x%x\n",bd_idx,MAC_BD_TOTALNUM-b_info->tx_bd_num);
			return -1;
		}
		value = am7x_readl(dev->base_addr+MAC_BDDescription+4*(bd_idx+b_info->tx_bd_num));
		////pr_info("%s,%d:bd _idx = %d,bd descriptor RX==0x%x\n",__func__,__LINE__,bd_idx,value);
		*byte_len = value>>21& 0xfff;
		if(*byte_len==0){
			//pr_info("%s,%d:bd descriptor RX==0x%x\n",__func__,__LINE__,value);
			am7x_mac_dump_register(dev);
		}
	}
	else{
		//pr_info("error bd descriptor type\n");
		*byte_len=0;
	}
	
	return 0;
}


static inline unsigned int get_rx_packet_cnt(struct net_device *dev)
{	
	unsigned int value = am7x_readl(dev->base_addr+MAC_RXPcktCnt);
	////pr_info("%s,%d:MAC_RXPcktCnt==0x%x\n",__func__,__LINE__,value);
	return value;
}

static inline unsigned int get_rx_drop_cnt(struct net_device *dev)
{
	unsigned int value = am7x_readl(dev->base_addr+MAC_RXDropCnt);
	////pr_info("%s,%d:MAC_RXDropCnt==0x%x\n",__func__,__LINE__,value);
	return value;
}

static inline unsigned int get_rx_crc_err_cnt(struct net_device *dev)
{
	unsigned int value = am7x_readl(dev->base_addr+MAC_CRCErrCnt);
	////pr_info("%s,%d:MAC_CRCErrCnt==0x%x\n",__func__,__LINE__,value);
	return value;
}

static inline unsigned int get_rx_nibble_cnt(struct net_device *dev)
{
	unsigned int value = am7x_readl(dev->base_addr+MAC_RXNibCnt);
	////pr_info("%s,%d:MAC_RXNibCnt==0x%x\n",__func__,__LINE__,value);
	return value;
}

static inline unsigned int get_rx_ctl_frame_cnt(struct net_device *dev)
{
	unsigned int value = am7x_readl(dev->base_addr+MAC_CtlFrameCnt);
	////pr_info("%s,%d:MAC_CtlFrameCnt==0x%x\n",__func__,__LINE__,value);
	return value;
}

static inline unsigned int get_rx_addr_cnt(struct net_device *dev)
{
	unsigned int value = am7x_readl(dev->base_addr+MAC_RXAddrCnt);
	////pr_info("%s,%d:MAC_RXAddrCnt==0x%x\n",__func__,__LINE__,value);
	return value;
}

static inline unsigned int get_rx_err_cnt(struct net_device *dev)
{
	unsigned int value = am7x_readl(dev->base_addr+MAC_RXErrCnt);
	////pr_info("%s,%d:MAC_RXErrCnt==0x%x\n",__func__,__LINE__,value);
	return value;
}

static inline unsigned int get_rx_frame_cnt(struct net_device *dev)
{
	unsigned int value = am7x_readl(dev->base_addr+MAC_RXFrameCnt);
	////pr_info("%s,%d:MAC_RXFrameCnt==0x%x\n",__func__,__LINE__,value);
	return value;
}
#if 0
static void get_rx_info(struct net_device *dev)
{
	get_rx_packet_cnt(dev);
	get_rx_drop_cnt(dev);
	get_rx_crc_err_cnt(dev);
	get_rx_nibble_cnt(dev);
	get_rx_ctl_frame_cnt(dev);
	get_rx_addr_cnt(dev);
	get_rx_err_cnt(dev);
	get_rx_frame_cnt(dev);
}
#endif
/**
get the MAC address, the length of the mac_addr array must large than 6
**/
static int get_MACAddr(struct net_device *ndev,macaddr_type_e mac_type,unsigned char mac_addr[])
{
	unsigned int macaddr0,macaddr1;
	unsigned char *macaddr=NULL;
	
	pr_info("%s,%d\n",__func__,__LINE__);
	if(ndev==NULL || mac_addr==NULL){
		printk(KERN_ERR"Get Mac Addr Err, ndev=0x%x,mac_addr=0x%x\n",(unsigned int)ndev,(unsigned int)mac_addr);
		return -EINVAL;
	}
	if(mac_type==MACADDR_TYPE_SOURCE || mac_type==MACADDR_TYPE_MULTICAST){
		if(mac_type==MACADDR_TYPE_SOURCE){
			macaddr0	= am7x_readl((AM7X_HWREG)ndev->base_addr+MAC_MACAddr0);	
			macaddr1 = am7x_readl((AM7X_HWREG)ndev->base_addr+MAC_MACAddr1);	
		}
		else{
			macaddr0	= am7x_readl((AM7X_HWREG)ndev->base_addr+MAC_MACAddr3);	
			macaddr1 = am7x_readl((AM7X_HWREG)ndev->base_addr+MAC_MACAddr4);	
		}
		macaddr = (unsigned char*)&macaddr0;
		mac_addr[0]=*(macaddr+0);
		mac_addr[1]=*(macaddr+1);
		mac_addr[2]=*(macaddr+2);
		mac_addr[3]=*(macaddr+3);
		macaddr = (unsigned char*)&macaddr1;
		mac_addr[4]=*(macaddr+0);
		mac_addr[5]=*(macaddr+1);
	}
	else if(mac_type==MACADDR_TYPE_DESTINATION){
		macaddr0	= am7x_readl((AM7X_HWREG)ndev->base_addr+MAC_MACAddr1);	
		macaddr1 = am7x_readl((AM7X_HWREG)ndev->base_addr+MAC_MACAddr2);	
		macaddr = (unsigned char*)&macaddr0;
		mac_addr[0]=*(macaddr+2);
		mac_addr[1]=*(macaddr+3);
		macaddr = (unsigned char*)&macaddr1;
		mac_addr[2]=*(macaddr+0);
		mac_addr[3]=*(macaddr+1);
		mac_addr[4]=*(macaddr+2);
		mac_addr[5]=*(macaddr+3);
	}
	else {
		printk(KERN_ERR"mac_type is not support,mac_type=%d\n",mac_type);
		return -EINVAL;
	}
	
	pr_info("%s,%d\n",__func__,__LINE__);
	return 0;
}


/**
set the source MAC address, the length
**/
static int set_MACAddr(struct net_device *ndev,macaddr_type_e mac_type,unsigned char mac_addr[])
{
	unsigned int macaddr0,macaddr1;
	unsigned char *macaddr=NULL;
	if(ndev==NULL || mac_addr==NULL){
		printk(KERN_ERR"Get Mac Addr Err, ndev=0x%x,mac_addr=0x%x\n",(unsigned int)ndev,(unsigned int)mac_addr);
		return -EINVAL;
	}
	if(mac_type==MACADDR_TYPE_SOURCE || mac_type==MACADDR_TYPE_MULTICAST){
		if(mac_type==MACADDR_TYPE_SOURCE){
			macaddr0	= am7x_readl((AM7X_HWREG)ndev->base_addr+MAC_MACAddr0);	
			macaddr1 = am7x_readl((AM7X_HWREG)ndev->base_addr+MAC_MACAddr1);	
		}
		else{
			macaddr0	= am7x_readl((AM7X_HWREG)ndev->base_addr+MAC_MACAddr3);	
			macaddr1 = am7x_readl((AM7X_HWREG)ndev->base_addr+MAC_MACAddr4);	
		}
		macaddr = (unsigned char*)&macaddr0;
		*(macaddr+0)=mac_addr[0];
		*(macaddr+1)=mac_addr[1];
		*(macaddr+2)=mac_addr[2];
		*(macaddr+3)=mac_addr[3];
		macaddr = (unsigned char*)&macaddr1;
		*(macaddr+0)=mac_addr[4];
		*(macaddr+1)=mac_addr[5];

		if(mac_type==MACADDR_TYPE_SOURCE){
			am7x_writel(macaddr0,(AM7X_HWREG)ndev->base_addr+MAC_MACAddr0);
			am7x_writel(macaddr1,(AM7X_HWREG)ndev->base_addr+MAC_MACAddr1);
		}
		else{
			am7x_writel(macaddr0,(AM7X_HWREG)ndev->base_addr+MAC_MACAddr3);
			am7x_writel(macaddr1,(AM7X_HWREG)ndev->base_addr+MAC_MACAddr4);
		}
	}
	else if(mac_type==MACADDR_TYPE_DESTINATION){
		macaddr0	= am7x_readl((AM7X_HWREG)ndev->base_addr+MAC_MACAddr1);	
		macaddr1 = am7x_readl((AM7X_HWREG)ndev->base_addr+MAC_MACAddr2);	
		macaddr = (unsigned char*)&macaddr0;
		*(macaddr+2)=mac_addr[0];
		*(macaddr+3)=mac_addr[1];
		macaddr = (unsigned char*)&macaddr1;
		*(macaddr+0)=mac_addr[2];
		*(macaddr+1)=mac_addr[3];
		*(macaddr+2)=mac_addr[4];
		*(macaddr+3)=mac_addr[5];
		am7x_writel(macaddr0,(AM7X_HWREG)ndev->base_addr+MAC_MACAddr1);
		am7x_writel(macaddr1,(AM7X_HWREG)ndev->base_addr+MAC_MACAddr2);
	}
	else {
		printk(KERN_ERR"mac_type is not support,mac_type=%d\n",mac_type);
		return -EINVAL;
	}
	return 0;
}

static int am7x_set_mac_addr(struct net_device *dev, void *p)
{
	struct sockaddr *addr = p;
	if (netif_running(dev))
		return -EBUSY;
	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;
	memcpy(dev->dev_addr, addr->sa_data, ETH_ALEN);
	if(set_MACAddr(dev,MACADDR_TYPE_SOURCE,dev->dev_addr))
		return -EAGAIN;
	return 0;
}


void am7x_mac_get_mii_info(struct net_device *dev)
{
	unsigned int value;
//	mii_info_reg_t *mii_info=(mii_info_reg_t *)&value;
	value = am7x_readl(dev->base_addr+MAC_MIIInfo);
	//pr_info("%s,%d:Mii Info==0x%x\n",__func__,__LINE__,value);
}

void am7x_mac_set_mii_info(struct net_device *dev)
{
	unsigned int value;
	mii_info_reg_t *mii_info=(mii_info_reg_t *)&value;
	
	value = am7x_readl(dev->base_addr+MAC_MIIInfo);
	mii_info->mii_no_pre=1;
	//pr_info("%s,%d:Mii Info==0x%x\n",__func__,__LINE__,value);
	am7x_writel(value,dev->base_addr+MAC_MIIInfo);
	
}

void am7x_mac_get_mii_data(struct net_device *dev)
{
	unsigned int value;
	value = am7x_readl(dev->base_addr+MAC_MIIData);
	//pr_info("%s,%d:Mii Data==0x%x\n",__func__,__LINE__,value);
}

static int am7x_mac_phy_check_mii_ready(struct net_device *dev)
{
	unsigned int value;
	value = am7x_readl(dev->base_addr+MAC_MIIInfo);
	//printk("value=====================%x\n",value);
	return (value >>1)&1;
}



static int am7x_mac_phy_set_ctl_data(struct net_device *dev,int phy_addr,int phy_reg_addr,unsigned int ctl_data)
{
	unsigned int info_value;
	unsigned int data_value;
	mii_info_reg_t *mii_info;
	mii_data_reg_t *mii_data;
	
	mii_info = (mii_info_reg_t *)&info_value;
	mii_data = (mii_data_reg_t*)&data_value;
	
	info_value = am7x_readl(dev->base_addr+MAC_MIIInfo);
	
	mii_info->phy_addr = phy_addr;
	mii_info->reg_addr = phy_reg_addr;
	mii_info->wr_ctl_data = 1;
	mii_info->rd_status= 0;  // Force set the read bit to 0
	mii_info->mii_clk_en = 1;

	mii_data->ctl_data = ctl_data;

	//pr_info("%s,%d:regaddr=0x%x,Info Value==0x%x,Ctl_data=0x%x\n",__func__,__LINE__,phy_reg_addr,info_value,ctl_data);
	am7x_writel(data_value,dev->base_addr+MAC_MIIData);
	am7x_writel(info_value,dev->base_addr+MAC_MIIInfo);
	msleep(2);
	do{
		msleep(10);	
	}while(am7x_mac_phy_check_mii_ready(dev)==1);
	
	mii_info->wr_ctl_data = 0;
	am7x_writel(info_value,dev->base_addr+MAC_MIIInfo);
	
	return 0;
}

static unsigned int am7x_mac_phy_read_rd_status(struct net_device *dev,int phy_addr,int phy_reg_addr)
{
	unsigned int info_value;
	unsigned int data_value;
	mii_info_reg_t *mii_info;
	mii_data_reg_t *mii_data;
	mii_info = (mii_info_reg_t *)&info_value;
	mii_data = (mii_data_reg_t*)&data_value;
	
	info_value = am7x_readl(dev->base_addr+MAC_MIIInfo);
	
	mii_info->phy_addr = phy_addr;
	mii_info->reg_addr = phy_reg_addr;

	mii_info->rd_status= 1;
	mii_info->wr_ctl_data = 0;  //Force set the write bit to 0
	mii_info->mii_clk_en = 1;
	am7x_writel(info_value,dev->base_addr+MAC_MIIInfo);
	/** 
	* For 1213, the clock source is 32K.
	* Delay 4 cycles which is about 0.125ms.
	* Here we set it to 1ms.
	*/
	msleep(2);;
	do{
		msleep(1);;	
	}while(am7x_mac_phy_check_mii_ready(dev)==1);
	mii_info->rd_status= 0;
	am7x_writel(info_value,dev->base_addr+MAC_MIIInfo);

	data_value=am7x_readl(dev->base_addr+MAC_MIIData);
	////pr_info("%s,%d:InfoValue==0x%x,phy_addr=%d,REG addr=0x%x,Rd_status=0x%x\n",__func__,__LINE__,info_value,mii_info->phy_addr,phy_reg_addr,mii_data->rd_status);
	return mii_data->rd_status;
	
}


static int phy_get_strap_addr(struct net_device *dev)
{
	unsigned int value=0;
	value = am7x_mac_phy_read_rd_status(dev,0,PHY_OP_MODESTRAP_STATUS);
	//pr_info("%s,%d:value=0x%x,PHY Addr=%d\n",__func__,__LINE__,value,(value>>13)&0x7);
	return (value>>13)&0x7;
}

static int phy_mdio_read(struct net_device *dev, int phy_id, int location)
{
	int ret=0;
//	unsigned long flags;
	am7x_board_info_t *b_info=to_am7x_board(dev);
	////pr_info("%s,%d:Phy Read phy_id=%d,reg_addr=0x%x\n",__func__,__LINE__,phy_id,location);

	mutex_lock(&b_info->phy_lock);
	//spin_lock_irqsave(&b_info->lock,flags);
#ifdef CONFIG_AM_8251
		am_get_multi_res(MAC_I2C);
		am7x_writel((am7x_readl(GPIO_MFCTL1) & (~(0x03<<26)))|(0x01<<26),GPIO_MFCTL1);
	//	am7x_mac_config_mulitfunc();
#endif
	
	ret = am7x_mac_phy_read_rd_status(dev,phy_id,location);

#ifdef CONFIG_AM_8251
	am_release_multi_res(MAC_I2C);
#endif

	//spin_unlock_irqrestore(&b_info->lock,flags);
	mutex_unlock(&b_info->phy_lock);
	
	return ret;
}

void phy_mdio_write (struct net_device *dev, int phy_id, int location, int val)
{
//	unsigned long flags;
	am7x_board_info_t *b_info=to_am7x_board(dev);
	//pr_info("%s,%d:Phy Write phy_id=%d,reg_addr=0x%x,val=0x%x\n",__func__,__LINE__,phy_id,location,val);

	mutex_lock(&b_info->phy_lock);
	//spin_lock_irqsave(&b_info->lock,flags);
#ifdef CONFIG_AM_8251
		am_get_multi_res(MAC_I2C);
		am7x_writel((am7x_readl(GPIO_MFCTL1) & (~(0x03<<26)))|(0x01<<26),GPIO_MFCTL1);
	//	am7x_mac_config_mulitfunc();
#endif

	am7x_mac_phy_set_ctl_data(dev,phy_id,location,val);

#ifdef CONFIG_AM_8251
	am_release_multi_res(MAC_I2C);
#endif

	//spin_unlock_irqrestore(&b_info->lock,flags);
	mutex_unlock(&b_info->phy_lock);

	msleep(300);  //for waiting write command
	//pr_info("after wait 500ms\n");

}



static int init_mii_if_info(struct net_device *dev)
{
	
	//pr_info("%s,%d\n",__func__,__LINE__);
	am7x_board_info_t *b_info=to_am7x_board(dev);

	//pr_info("%s,%d:#####INIT MII IF INFO########\n",__func__,__LINE__);

	b_info->msg_enable = NETIF_MSG_LINK;
	b_info->mii.phy_id = phy_get_strap_addr(dev);
	b_info->mii.phy_id_mask = 0x1f;
	b_info->mii.reg_num_mask = 0x1f;
	
	b_info->mii.full_duplex = 1;
	b_info->mii.force_media = 0;
	b_info->mii.supports_gmii = 0;

	b_info->mii.dev = dev;
	b_info->mii.mdio_read = phy_mdio_read;
	b_info->mii.mdio_write = phy_mdio_write;
	
	//pr_info("%s,%d\n",__func__,__LINE__);
	//b_info->mii.advertising = 1;
	return 0;
}

#define PHY_100FULL 	0x0100
#define PHY_100HALF 	0x0080
#define PHY_10FULL	0x0040
#define PHY_10HALF	0x0020
static int phy_show_carrier(am7x_board_info_t *b_info)
{
	int lpa=0;
	lpa = phy_mdio_read(b_info->ndev, b_info->mii.phy_id, PHY_AUTONEGO_LINK);	

	printk(KERN_INFO "%s: link up, %sMbps, %s-duplex\n",
		       b_info->mii.dev->name,
		       lpa & (PHY_100FULL | PHY_100HALF) ? "100" : "10",
		       lpa &( PHY_100FULL | PHY_10FULL)? "full" : "half"
		       );
	return 0;
}


void am7x_net_schedule_poll(am7x_board_info_t *b_info)
{
	static int first=1;

	if(first){
		schedule_delayed_work(&b_info->phy_poll, HZ * 2);
		first = 0;
	}
	else{
		schedule_delayed_work(&b_info->phy_poll, HZ * 5);
	}
}

static void am7x_net_poll_work(struct work_struct *w)
{
	
	struct delayed_work *dw = container_of(w, struct delayed_work, work);
	am7x_board_info_t *b_info = container_of(dw, am7x_board_info_t, phy_poll);
	struct net_device *ndev = b_info->ndev;
	unsigned int link_ok, link_fail_temp=0;
	
	unsigned old_carrier = netif_carrier_ok(ndev) ? 1 : 0;
	unsigned new_carrier=0;
    int i=0;

	link_ok = (unsigned int) mii_link_ok(&b_info->mii);
	// https://teamwork.iezvu.com/T2730
	// Double check whether LAN link is ok or not.
    if (old_carrier != link_ok) {
        for( i=0 ; i<=7 ; i++ )
        {
            link_ok = (unsigned int) mii_link_ok(&b_info->mii);

            if(link_ok) {
                printk("link is ok.\n");
                link_ok = 1;
                break;
            }
            else {
                printk("link is fail !!\n");
                link_fail_temp++;
            }

            if(( i == 7 ) && ( link_fail_temp == 8 )){
                link_ok = 0;
                printk("link_not_ok, link_fail_temp=%d \n", link_fail_temp);
            }
        }
    }
	
	new_carrier = link_ok?1:0;
	net_link_status=new_carrier;
	//printk("old_carrier=====%d,new_carrier=======%d\n",old_carrier,new_carrier);
	if (old_carrier != new_carrier) {
		
		if (netif_msg_link(b_info)){
			if(new_carrier){
				phy_show_carrier(b_info);
			}
			else{
				printk(KERN_INFO "%s: link down\n",b_info->mii.dev->name);
			}
		}
		
		if (!new_carrier){
			netif_carrier_off(ndev);
			
		}
		else{
			netif_carrier_on(ndev);
			
		}
	}
	
	if (netif_running(ndev)){
		am7x_net_schedule_poll(b_info);
	}
	//pr_info("%s,%d\n",__func__,__LINE__);

}




/******KSZ8051 END*************/
void test_phy(struct net_device *dev)
{
	int i=0;
	am7x_board_info_t *b_info=to_am7x_board(dev);
	//pr_info("%s,%d:######TEST PHY START########\n",__func__,__LINE__);
//	am7x_mac_get_mii_info(dev);
	//am7x_mac_get_mii_data(dev);
//	am7x_mac_phy_read_rd_status(dev,b_info->phy_strap_addr,PHY_IDENTIFY1);
/*
	am7x_mac_phy_read_rd_status(dev,0,PHY_OP_MODESTRAP_STATUS);
	value = am7x_mac_phy_read_rd_status(dev,PHY_ADDR,PHY_OP_MODESTRAP_OVERRIDE);
	set_bit(9,value);
	am7x_mac_phy_set_ctl_data(dev,PHY_ADDR,PHY_OP_MODESTRAP_OVERRIDE,value);
	am7x_mac_phy_read_rd_status(dev,PHY_ADDR,PHY_OP_MODESTRAP_STATUS);

	am7x_mac_phy_read_rd_status(dev,PHY_ADDR,PHY_OP_MODESTRAP_STATUS);
	am7x_mac_phy_read_rd_status(dev,0,PHY_OP_MODESTRAP_STATUS);
	//am7x_net_msleep(10);
*/

	for(i=0;i<=0x08;i++){
		am7x_mac_phy_read_rd_status(dev,b_info->mii.phy_id,i);
	}

	//pr_info("%s,%d:######TEST PHY END########\n",__func__,__LINE__);
}


static int am7x_mac_set_rx_bd(struct net_device *dev)
{
	int i=0;
	
	am7x_board_info_t *b_info=to_am7x_board(dev);

	/** MAC_BD_TOTALNUM-b_info->tx_bd_num */
	for(i=0;i<MAC_BD_TOTALNUM-b_info->tx_bd_num;i++){
		//am7x_mac_get_bd_pointer(dev,&b_info->rxbd_pointer,BD_RX);
		////pr_info("\n%s,%d: RX BD write_ptr=%d,read_ptr=%d,bd_idx=%d\n",__func__,__LINE__,b_info->rxbd_pointer.write_ptr,b_info->rxbd_pointer.read_ptr,i);
		am7x_mac_set_bd_descriptor(dev,i,0,BD_RX);
	}

	/** 
	* save the rxbd status, but here the IC's reset process has not 
	* been finished, so i move it later.
	*/
	///am7x_mac_get_bd_pointer(dev,&b_info->rxbd_pointer,BD_RX);

	return 0;
}

/**
* check the overflow
*/
static int  check_rxbd_overflow(struct net_device *dev)
{
/**	
	am7x_board_info_t *b_info=to_am7x_board(dev);
	bd_pointer_t rx_pointer;


	am7x_mac_get_bd_pointer(dev,&rx_pointer,BD_RX);
	if(unlikely(rx_pointer.read_ptr>=rx_pointer.write_ptr&& rx_pointer.read_around==rx_pointer.write_around)){
		//pr_info("read_around=%d,write_around=%d,read_ptr=%d,write_ptr=%d,Over Flow will be Occurs\n",rx_pointer.read_around,\
			rx_pointer.write_around,rx_pointer.read_ptr,rx_pointer.write_ptr);
	}
*/	
	return 0;
}

#if 0
static int check_tx_restart(struct net_device *dev)
{
/**
	am7x_board_info_t *b_info=to_am7x_board(dev);
	bd_pointer_t tmp_pointer;
	
	am7x_mac_get_bd_pointer(dev,&tmp_pointer,BD_TX);

	if(tmp_pointer.read_around==tmp_pointer.write_around && \
		tmp_pointer.write_ptr==tmp_pointer.read_ptr){
		return 1;
	}
	else{
		return 0;
	}
*/
	am7x_board_info_t *b_info=to_am7x_board(dev);
	bd_pointer_t tmp_pointer;
	int gap;

	if(b_info->tx_bd_num == 1){
		return 1;
	}

	am7x_mac_get_bd_pointer(dev,&tmp_pointer,BD_TX);

	if(tmp_pointer.write_ptr >= tmp_pointer.read_ptr){
		gap = tmp_pointer.write_ptr + b_info->tx_bd_num - tmp_pointer.read_ptr;
	}
	else{
		gap = tmp_pointer.read_ptr - tmp_pointer.write_ptr;
	}

	if(gap >= MAC_TX_BD_GAP){
		return 1;
	}
	else{
		return 0;
	}
}
#endif
static int am7x_net_rx(struct net_device *dev)
{
	struct sk_buff *skb=NULL;
	int frame_rx;
	char *skb_buff=NULL;
	am7x_board_info_t *b_info=to_am7x_board(dev);
	unsigned int bd_idx;
	unsigned int byte_len=0;
	bd_pointer_t now_ptr;
	
	/*get the rx bd read pointer first*/
	am7x_mac_get_bd_pointer(dev,&now_ptr,BD_RX);

//[Sanders.130507] - Overflow check.
//	if(now_ptr.read_around==b_info->rxbd_pointer.read_around){
	if (now_ptr.read_ptr > b_info->rxbd_pointer.read_ptr) {
		frame_rx = now_ptr.read_ptr-b_info->rxbd_pointer.read_ptr;
	}
	else{
//[Sanders.130507] - unsigned --> signed
//		frame_rx = (now_ptr.read_ptr-b_info->rxbd_pointer.read_ptr)+MAC_BD_TOTALNUM-b_info->tx_bd_num;
		frame_rx = (MAC_BD_TOTALNUM-b_info->tx_bd_num)+now_ptr.read_ptr-b_info->rxbd_pointer.read_ptr;
	}

	if(unlikely(frame_rx<0)){
		//pr_info("%s,%d NET RX ERROR:: now_ptr->read_ptr=%d,before=%d,frame_rx=%d,now_readaround=%d,read_around=%d\n",__func__,__LINE__,now_ptr.read_ptr,
//			b_info->rxbd_pointer.read_ptr,frame_rx,now_ptr.read_around,b_info->rxbd_pointer.read_around);
	}


	bd_idx = b_info->rxbd_pointer.read_ptr-b_info->tx_bd_num;
	//pr_info("%s,%d:: frame_rx=%d,bd_idx=%d\n",__func__,__LINE__,frame_rx,bd_idx);
	while(frame_rx>0){
		am7x_mac_get_bd_descriptor(dev,bd_idx,&byte_len,BD_RX);

		if(byte_len > 0){
			skb = dev_alloc_skb(byte_len+2);

			if(!skb){
				/** if buffer not enough, we will drop all the frame received */
				if(printk_ratelimit()){
					printk(KERN_ERR" alloc skb failed!\n");
				}
				b_info->stats.rx_dropped++;
				goto NET_RX_OUT;
			}
			skb_buff = skb_put(skb, byte_len);
			copy_from_sdram(dev,bd_idx,BD_RX,byte_len,skb_buff);
			b_info->stats.rx_packets++;
			b_info->stats.rx_bytes += byte_len;
			////pr_info("Receive Frame Len==%d\n",byte_len);
			//dump_buf(skb_buff,byte_len);
			
			check_rxbd_overflow(dev);

			am7x_mac_set_bd_descriptor(dev,bd_idx,0,BD_RX);///reset the rx bd descriptor

			skb->protocol = eth_type_trans(skb, dev);
			netif_rx(skb);
		}
		else{
			printk(KERN_ERR"am7x net RX 0 or less BYTES\n");
		}
		frame_rx--;
		bd_idx++;
		if(bd_idx>=MAC_BD_TOTALNUM-b_info->tx_bd_num){
			bd_idx=0;
		}
	}

NET_RX_OUT:
	memcpy(&b_info->rxbd_pointer,&now_ptr,sizeof(bd_pointer_t));
	
	return 0;
	
}
#if 0
static void am7x_mac_dump_int_stat(unsigned int stat)
{
	if((stat>>21)&0x1){
		printk("- rx crc error\n");
	}

	if((stat>>20)&0x1){
		printk("- rx short frame error\n");
	}

	if((stat>>19)&0x1){
		printk("- rx dribble nibble error\n");
	}

	if((stat>>18)&0x1){
		printk("- rx packet too big error\n");
	}

	if((stat>>17)&0x1){
		printk("- invalid symbol error\n");
	}

	if((stat>>16)&0x1){
		printk("- rx over run error\n");
	}

	if((stat>>15)&0x1){
		printk("- software bd write addr error\n");
	}

	if((stat>>14)&0x1){
		printk("- receive control frame\n");
	}

	if((stat>>13)&0x1){
		printk("- transmit control frame\n");
	}

	if((stat>>12)&0x1){
		printk("- busy\n");
	}

	if((stat>>11)&0x1){
		printk("- receive error\n");
	}
/**
	if((stat>>10)&0x1){
		printk("- receive frame\n");
	}
*/
	if((stat>>9)&0x1){
		printk("- transmit error\n");
	}

	if((stat>>8)&0x1){
		printk("- transmit buffer\n");
	}

/**
	if((stat>>7)&0x1){
		printk("- software bd write addr error Mask\n");
	}

	if((stat>>6)&0x1){
		printk("- receive control frame Mask\n");
	}

	if((stat>>5)&0x1){
		printk("- transmit control frame Mask\n");
	}

	if((stat>>4)&0x1){
		printk("- busy mask\n");
	}

	if((stat>>3)&0x1){
		printk("- receive error mask\n");
	}

	if((stat>>2)&0x1){
		printk("- receive frame mask\n");
	}

	if((stat>>1)&0x1){
		printk("- transmit error mask\n");
	}

	if((stat>>0)&0x1){
		printk("- transmit buffer mask\n");
	}
*/

}
#endif
static irqreturn_t am7x_net_interrupt(int irq, void *dev_id)
{
	unsigned int i_status;
	struct net_device *dev = dev_id;
	int_reg_info_t *int_reg=(int_reg_info_t*)&i_status;
	am7x_board_info_t *b_info=to_am7x_board(dev);
	//bd_pointer_t tmp_pointer;
	
	spin_lock(&b_info->lock);
	
	i_status =am7x_readl(dev->base_addr+MAC_Interrupt);
	//am7x_mac_dump_int_stat(i_status);
	
	//pr_info("int_reg->rxc=%d,int_reg->rxb=%d\n",int_reg->rxc,int_reg->rxb);
#if 1 //[Sanders.130507] - Simplify interrupt handling.
	if (int_reg->swad || int_reg->rxe || int_reg->txe)
	{
		//Do something.
	}
	if (int_reg->rxc || int_reg->rxb)
	{
		am7x_net_rx(dev);
	}
	if (int_reg->txc || int_reg->txb)
	{
		if (int_reg->txb && b_info->txdb_full)
		{
			b_info->txdb_full = 0;
			netif_wake_queue(dev);
		}
	}
  #if 0 //For debug purpose.
	if (int_reg->rx_crc_error_pd)
	{
		printk("%s::%d::rx_crc_error_pd\n", __FUNCTION__, __LINE__);
	}
	if (int_reg->rx_short_frame_pd)
	{
		printk("%s::%d::rx_short_frame_pd\n", __FUNCTION__, __LINE__);
	}
	if (int_reg->rx_dribble_nib)
	{
		printk("%s::%d::rx_dribble_nib\n", __FUNCTION__, __LINE__);
	}
	if (int_reg->rx_pckt_too_big_pd)
	{
		printk("%s::%d::rx_pckt_too_big_pd\n", __FUNCTION__, __LINE__);
	}
	if (int_reg->invalid_symbol_pd)
	{
		printk("%s::%d::invalid_symbol_pd\n", __FUNCTION__, __LINE__);
	}
	if (int_reg->rx_over_run_pd)
	{
		printk("%s::%d::rx_over_run_pd\n", __FUNCTION__, __LINE__);
	}
	if (int_reg->busy)
	{
		printk("%s::%d::busy\n", __FUNCTION__, __LINE__);
	}
  #endif
#else
	if(int_reg->swad){
		int_reg->swad = 1;
		am7x_mac_get_bd_pointer(dev,&tmp_pointer,BD_RX);
		//pr_info("Soft BD Write Error! RX:write_ptr=%d,read_ptr=%d\n",tmp_pointer.write_ptr,tmp_pointer.read_ptr);
		am7x_mac_get_bd_pointer(dev,&tmp_pointer,BD_TX);
		//pr_info("Soft BD Write Error! TX:write_ptr=%d,read_ptr=%d\n",tmp_pointer.write_ptr,tmp_pointer.read_ptr);
	}

	if(int_reg->rxc){
		am7x_net_rx(dev);
		int_reg->rxc = 1;
	}

/**
	if(int_reg->busy){
		am7x_mac_get_bd_pointer(dev,&tmp_pointer,BD_RX);
		//pr_info("BUSY! RX:wp=%d,rp=%d,wa=%d,ra=%d\n",tmp_pointer.write_ptr,tmp_pointer.read_ptr,tmp_pointer.write_around,tmp_pointer.read_around);
		//pr_info("BUSY! SAVE:wp=%d,rp=%d,wa=%d,ra=%d\n",b_info->rxbd_pointer.write_ptr,b_info->rxbd_pointer.read_ptr,b_info->rxbd_pointer.write_around,b_info->rxbd_pointer.read_around);
		//am7x_mac_get_bd_pointer(dev,&tmp_pointer,BD_TX);
		////pr_info("BUSY! TX:write_ptr=%d,read_ptr=%d\n",tmp_pointer.write_ptr,tmp_pointer.read_ptr);
	}
*/

	if(int_reg->txb){
		if(b_info->txdb_full){
			unsigned int ctl_reg=0;

			ctl_reg = am7x_readl(dev->base_addr+MAC_CtlReg);

			/** wake queue if the transmit fifo is empty */
			if(check_tx_restart(dev)==1){
				//printk("Restart the transmit queue\n");
				b_info->txdb_full=0;
				netif_wake_queue(dev);
			}
		}
	}

#if 0
	if(b_info->txdb_full){
		unsigned int ctl_reg=0;
		
		ctl_reg = am7x_readl(dev->base_addr+MAC_CtlReg);

		/** wake queue if the transmit fifo is empty */
		if(check_tx_restart(dev)==1){ 
			netif_wake_queue(dev);
			b_info->txdb_full=0;	
			b_info->rx_timeout=0;
		}
		else{
			b_info->rx_timeout++;
			if(b_info->rx_timeout>=1000){
				//pr_info("%s,%d: rx time out reset the mac\n",__func__,__LINE__);
				am7x_mac_dump_register(dev);
				am7x_mac_reset(dev);
				b_info->rx_timeout=0;
			}
		}
	}
#endif

	if(int_reg->txe){
		int_reg->txe = 1;
	}
	
	if(int_reg->rxb){
		am7x_net_rx(dev);
		int_reg->rxb = 1;
		//am7x_mac_get_bd_pointer(dev,&tmp_pointer,BD_TX);
		////pr_info("RECV! TX:write_ptr=%d,read_ptr=%d,ctrl=0x%x\n",tmp_pointer.write_ptr,tmp_pointer.read_ptr,am7x_readl(dev->base_addr+MAC_CtlReg));
	}
#endif

	am7x_writel(i_status,dev->base_addr+MAC_Interrupt);

	spin_unlock(&b_info->lock);
	
	return IRQ_HANDLED;
}


static int am7x_net_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	am7x_board_info_t *b_info=to_am7x_board(dev);
	//pr_info("\n%s,%d: am7x_net_get_settings\n",__func__,__LINE__);
	mii_ethtool_gset(&b_info->mii, cmd);
	return 0;
}
static int am7x_net_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	am7x_board_info_t *b_info=to_am7x_board(dev);
	//pr_info("\n%s,%d: am7x_net_set_settings\n",__func__,__LINE__);
	return mii_ethtool_sset(&b_info->mii, cmd);
}
static u32 am7x_net_get_msglevel(struct net_device *dev)
{
	am7x_board_info_t *b_info=to_am7x_board(dev);
	//pr_info("\n%s,%d: am7x_net_get_msglevel\n",__func__,__LINE__);
	return b_info->msg_enable;
}

static void am7x_net_set_msglevel(struct net_device *dev, u32 value)
{
	am7x_board_info_t *b_info=to_am7x_board(dev);
	//pr_info("\n%s,%d: am7x_net_set_msglevel\n",__func__,__LINE__);
	b_info->msg_enable=value;
}
static u32 am7x_net_get_link(struct net_device *dev)
{
	u32 ret;
	am7x_board_info_t *b_info=to_am7x_board(dev);
	//pr_info("\n%s,%d: am7x_net_get_link\n",__func__,__LINE__);
	ret = mii_link_ok(&b_info->mii);
	return ret;
}

static int am7x_net_nway_reset(struct net_device *dev)
{
	am7x_board_info_t *b_info=to_am7x_board(dev);
	//pr_info("\n%s,%d: am7x_net_nway_reset\n",__func__,__LINE__);
	return mii_nway_restart(&b_info->mii);
}



static const struct ethtool_ops am7x_nets_ethtool_ops = {

	.get_settings		= am7x_net_get_settings,
	.set_settings		= am7x_net_set_settings,
	.get_msglevel		= am7x_net_get_msglevel,
	.set_msglevel		= am7x_net_set_msglevel,
	.get_link		= am7x_net_get_link,
	.nway_reset		= am7x_net_nway_reset,
};


static void am7x_mac_ctl_reset(void)
{
	unsigned int value;
	
	value = am7x_readl(CMU_DEVRST2);
	clear_bit(0,value);
	am7x_writel(value,CMU_DEVRST2);
	//pr_info("%s,%d: value =0x%x\n",__func__,__LINE__,value);
	am7x_net_msleep(10);
	set_bit(0,value);
	am7x_writel(value,CMU_DEVRST2);
}

static void am7x_mac_reset(struct net_device *dev)
{
	am7x_board_info_t *b_info=to_am7x_board(dev);

	am7x_mac_ctl_reset();

	/** set transmit data buffer number */
	b_info->tx_bd_num = am7x_mac_get_tx_bd_num(dev);
	if(b_info->tx_bd_num != MAC_TX_BD_NUM){
		b_info->tx_bd_num = MAC_TX_BD_NUM;
//[Sanders.130507] - Should be here.
		am7x_mac_set_tx_bd_num(dev,b_info->tx_bd_num);
	}
//[Sanders.130507] - Should not be here.
//	am7x_mac_set_tx_bd_num(dev,b_info->tx_bd_num);
	
	//printk(KERN_INFO"Call Function %s,%d,bd_num=%d\n",__func__,__LINE__,b_info->tx_bd_num);

	/** swap addr should be set according to the TX_BD_NUM */
	b_info->swap_addr.tx_swap_addr = b_info->tx_bd_num-1;
	b_info->swap_addr.rx_swap_addr = MAC_BD_TOTALNUM-1;
	//printk("******am7x_mac_set_swap_addr_begin******\n");
	am7x_mac_set_swap_addr(dev,&b_info->swap_addr);
	
	//printk("******am7x_mac_set_swap_addr_over******\n");
	memset(&b_info->stats, 0, sizeof(struct net_device_stats));

	//am7x_mac_set_mii_info(dev);
	
	/** set base address */
	am7x_mac_set_sdr_base_addr(dev);
	
	//printk("******1******\n");
	/** enable rx  and initial the rx bd */
	am7x_mac_enable_receive(dev);

//[Sanders.130507] - Moved from 'am7x_net_start_xmit'
	am7x_mac_enable_transmit(dev);

	//printk("******2******\n");
	am7x_mac_config_mac_ctl(dev);
	
	//printk("******3******\n");
	am7x_mac_set_rx_bd(dev);
	
	//printk("******4******\n");
	
	/** sync the bd pointer with IC */
	am7x_mac_get_bd_pointer(dev,&b_info->rxbd_pointer,BD_RX);

	//printk("******5******\n");
	am7x_mac_get_bd_pointer(dev,&b_info->txbd_pointer,BD_TX);

	//printk("******6******\n");
	/** enable the mac interrupt and config the interrupt function */
	am7x_mac_enable_inttrupt(dev);
	
	//printk("******7******\n");
	///am7x_mac_dump_register(dev);
	
}

#if 0

////for test receive and transmit////

unsigned int send_num=0;
static bd_pointer_t old_ptr={
	.read_around =0,
	.write_around =0,
	.write_ptr =0,
	.read_ptr =0
};

static int check_error(bd_pointer_t *new_ptr)
{
	if(new_ptr->write_ptr==old_ptr.write_ptr && new_ptr->read_ptr==old_ptr.read_ptr &&
		new_ptr->read_around==old_ptr.read_around && new_ptr->write_around == old_ptr.write_around){
		//dump_register();
		return -1;

	}
	else{
		memcpy(&old_ptr,new_ptr,sizeof(bd_pointer_t));
		return 0;
	}
}

static int err_num=0;

static int test_check_tx_restart(struct net_device *dev)
{
	bd_pointer_t new_ptr;
	am7x_board_info_t *b_info=to_am7x_board(dev);
	am7x_mac_get_bd_pointer(dev,&new_ptr,BD_TX);
	
	//pr_info("TX BD{Tcheck} write_ptr=%d,read_ptr=%d,r_round=%d,w_round=%d\n",new_ptr.write_ptr,new_ptr.read_ptr,
		new_ptr.read_around,new_ptr.write_around);
	if(check_error(&new_ptr)){
		err_num ++;
		if(err_num>=100){
			//pr_info("%s,%d: Errr Happen send_num=%d\n",__func__,__LINE__,send_num);
				//pr_info("TX BD{Tcheck} write_ptr=%d,read_ptr=%d,r_round=%d,w_round=%d\n",new_ptr.write_ptr,new_ptr.read_ptr,
		new_ptr.read_around,new_ptr.write_around);
			am7x_mac_dump_register(dev);
			//am7x_mac_reset(dev);
			err_num=0;
		}
	}
	else
		err_num=0;
	
	/*if((new_ptr.write_ptr-new_ptr.read_ptr<=2&& new_ptr.read_around==new_ptr.write_around)
		||(new_ptr.read_ptr-new_ptr.write_ptr>=b_info->tx_bd_num-2 && new_ptr.read_around!=new_ptr.write_around) )*/
	if((new_ptr.write_ptr-new_ptr.read_ptr==0&& new_ptr.read_around==new_ptr.write_around))	
		return 1;
	else
		return 0;
}

static int test_check_txptr(struct net_device *dev)
{
	unsigned int i_status;
	int_reg_info_t *int_reg=(int_reg_info_t*)&i_status;
	am7x_board_info_t *b_info=to_am7x_board(dev);
	if(b_info->txdb_full){
		unsigned int ctl_reg=0;
		ctl_reg = am7x_readl(dev->base_addr+MAC_CtlReg);
		i_status =am7x_readl(dev->base_addr+MAC_Interrupt);
		////pr_info("%s,%d:: CTL REG=0x%x\n",__func__,__LINE__,ctl_reg);
		//pr_info("%s,%d:: i_status=0x%x\n",__func__,__LINE__,i_status);
		if(int_reg->rxb)
			int_reg->rxb = 1;
		if(test_check_tx_restart(dev)==1){  ///wake queue if the transmit fifo is half full
			//pr_info("%s,%d: wake up transmit queue\n",__func__,__LINE__);
			//netif_wake_queue(dev);
			b_info->txdb_full=0;	
		}
		am7x_writel(i_status,dev->base_addr+MAC_Interrupt);
	}
}

static int test_set_feature(struct net_device *dev)
{
	unsigned int value=0;
	unsigned int flags;
	am7x_board_info_t *b_info=to_am7x_board(dev);
	dump_register();

	//value = am7x_readl(INTC_PD);
	////pr_info("%s,%d:INTC_PD==0x%x\n",__func__,__LINE__,value);
	//spin_lock_irqsave(&b_info->lock,flags);
}


static int test_tx_send(struct net_device *dev)
{
	int bd_idx=0;
	int data_len=0;
	char *data=NULL;
	am7x_board_info_t *b_info=to_am7x_board(dev);
	unsigned char data_send[128];
	bd_pointer_t new_ptr;
	am7x_mac_get_bd_pointer(dev,&b_info->txbd_pointer,BD_TX);

//	//pr_info("TX BD write_ptr=%d,read_ptr=%d,r_round=%d,w_round=%d\n",b_info->txbd_pointer.write_ptr,b_info->txbd_pointer.read_ptr,
//			b_info->txbd_pointer.read_around,b_info->txbd_pointer.write_around);

//	i_status =am7x_readl(dev->base_addr+MAC_Interrupt);
//	//pr_info("%s,%d:: TX i_status=0x%x\n",__func__,__LINE__,i_status);

	memset(data_send,0,128);
	data_len=128;
	data = data_send;
	
	bd_idx = b_info->txbd_pointer.write_ptr;
	copy_to_sdram(dev,bd_idx,BD_TX,data_len,data);	
	
	dev->trans_start = jiffies;	/* save the time stamp */
	am7x_mac_set_bd_descriptor(dev, bd_idx,data_len,BD_TX);
	send_num++;	
	am7x_mac_get_bd_pointer(dev,&new_ptr,BD_TX);
	
	if(new_ptr.read_around!=new_ptr.write_around&& new_ptr.write_ptr >=new_ptr.read_ptr){//if the next write pointer is equal the read pointer, just stop the queue
	/*if((new_ptr.write_ptr-new_ptr.read_ptr==b_info->tx_bd_num/2 && new_ptr.read_around==new_ptr.write_around)
		||(new_ptr.read_ptr-new_ptr.write_ptr==b_info->tx_bd_num/2 && new_ptr.read_around!=new_ptr.write_around) ){
		b_info->txdb_full=1;	

	if((new_ptr.write_ptr-new_ptr.read_ptr>=b_info->tx_bd_num-1 && new_ptr.read_around==new_ptr.write_around)
		||(new_ptr.read_ptr-new_ptr.write_ptr<=1 && new_ptr.read_around!=new_ptr.write_around) ){
		*/
		b_info->txdb_full=1;	
		//pr_info("\n%s,%d: NEW TX BD write_ptr=%d,read_ptr=%d,r_round=%d,w_round=%d\n",__func__,__LINE__,new_ptr.write_ptr,new_ptr.read_ptr,
			new_ptr.read_around,new_ptr.write_around);
		//pr_info("%s,%d: netif_stop_queue send_num=%d\n",__func__,__LINE__,send_num);
	}
	else{
		am7x_mac_enable_transmit(dev);
		
	}
}

static int test_rx_debug(struct net_device *dev)
{
	am7x_board_info_t *b_info=to_am7x_board(dev);

	am7x_mac_disable_receive(dev);
	test_set_feature(dev);
	am7x_mac_dump_register(dev);
	while(1){
		if(b_info->txdb_full==0){
			test_tx_send(dev);
		}

		test_check_txptr(dev);
	}
}


////for test receive

#define TEST_BUF_SIZE 1200

void dump_txbd_descriptor(struct net_device *dev)
{
	int bd_idx=0;
	unsigned int value=0;
	am7x_board_info_t *b_info=to_am7x_board(dev);
	for(bd_idx=0;bd_idx<b_info->tx_bd_num;bd_idx++){
		value = am7x_readl(dev->base_addr+MAC_BDDescription+4*(bd_idx));
		//pr_info("txbd[%d]offset=0x%x,value=0x%x\n",bd_idx,4*(bd_idx),value);
	}
}

void dump_rxbd_descriptor(struct net_device *dev)
{
	int bd_idx=0;
	unsigned int value=0;
	am7x_board_info_t *b_info=to_am7x_board(dev);
	for(bd_idx=0;bd_idx+b_info->tx_bd_num<MAC_BD_TOTALNUM;bd_idx++){
		value = am7x_readl(dev->base_addr+MAC_BDDescription+4*(bd_idx+b_info->tx_bd_num));
		//pr_info("rxbd[%d]offset=0x%x,value=0x%x\n",bd_idx,4*(bd_idx+b_info->tx_bd_num),value);
	}
}

void dump_receive(struct net_device *dev)
{
	int bd_idx=0;
	unsigned char buf_received[TEST_BUF_SIZE]="";
	unsigned int byte_len=0;
	am7x_board_info_t *b_info=to_am7x_board(dev);
	am7x_mac_get_bd_pointer(dev,&b_info->rxbd_pointer,BD_RX);
				/*get the rx bd read pointer first*/
	bd_idx = b_info->rxbd_pointer.read_ptr-b_info->tx_bd_num;

	if(bd_idx==0){//if the bd_idx is equal to zero, it means the data had been stored at the rx_swap_addr, so the bd_idx should be changed
		bd_idx = b_info->swap_addr.rx_swap_addr-b_info->tx_bd_num;
	}
	else{///the read_ptr is the bd which will be filled by hardware next, so the bd_idx-1 is the bd which had been filled
		bd_idx = bd_idx-1;
	}
	am7x_mac_get_bd_descriptor(dev,bd_idx,&byte_len,BD_RX);
/*
	pr_info("\n%s,%d:byte_len=%d, tx_bd_num=%d, RX BD write_ptr=%d,read_ptr=%d,bd_idx=%d\n",__func__,__LINE__,\
	byte_len,b_info->tx_bd_num,b_info->rxbd_pointer.write_ptr,b_info->rxbd_pointer.read_ptr,bd_idx );
*/
	if(byte_len){
		memset(buf_received,0,TEST_BUF_SIZE);
		copy_from_sdram(dev,bd_idx,BD_RX,byte_len,buf_received);
		dump_buf(buf_received,byte_len);
	}
	else{
//		pr_info("Sorry Receive Byte = 0\n");
	}
}


int  test_receive(struct net_device *dev)
{
	unsigned int i_status;
	unsigned int flags;
	int_reg_info_t *int_reg;
	unsigned int byte_len=0;
	int_reg = (int_reg_info_t*)&i_status;
	unsigned char buf_send[TEST_BUF_SIZE]="";
	int bd_idx=0;
	am7x_board_info_t *b_info=to_am7x_board(dev);

	#if 0
	am7x_mac_get_bd_pointer(dev,&b_info->rxbd_pointer,BD_RX);
	bd_idx = b_info->rxbd_pointer.read_ptr-b_info->tx_bd_num;
	am7x_mac_get_bd_descriptor(dev,bd_idx,&byte_len,BD_RX);
/*
	pr_info("\n%s,%d:byte_len=%d, tx_bd_num=%d, RX BD write_ptr=%d,read_ptr=%d,bd_idx=%d\n",__func__,__LINE__,\
	byte_len,b_info->tx_bd_num,b_info->rxbd_pointer.write_ptr,b_info->rxbd_pointer.read_ptr,bd_idx );
	*/
	//if(byte_len){
		copy_from_sdram(dev,bd_idx,BD_RX,byte_len,NULL);  ///just dump the data
		//dump_buf(buf_received,64);
	//}
	//else{
	//	//pr_info("%s,%d: Sorry Receive byte_len===0\n",__func__,__LINE__);
	//}
	#endif
	////pr_info("Dump RX MEMSTART before receive \n");
	//dump_buf(b_info->bd_rx_mem_addr,TEST_BUF_SIZE);

	spin_lock_irqsave(&b_info->lock, flags);
	am7x_mac_dump_register(dev);
	//pr_info("%s,%d: TEST_BUF_SIZE===%d\n",__func__,__LINE__,TEST_BUF_SIZE);
	/****send the buffer*****/
	{
		int i=0;
		for(i=0;i<TEST_BUF_SIZE;i++){
			if(i%64==0)
				buf_send[i]=i/64;
			else
				buf_send[i]=0x5a;
		}
	}
	am7x_mac_get_bd_pointer(dev,&b_info->txbd_pointer,BD_TX);
	bd_idx = b_info->txbd_pointer.write_ptr;

	////pr_info("Dump TX MEMSTART before copy to sdram\n");
	//dump_buf(b_info->bd_mem_addr,TEST_BUF_SIZE);
	
	copy_to_sdram(dev,bd_idx,BD_TX,TEST_BUF_SIZE,buf_send);	


	////pr_info("Dump TX MEMSTART after copy to sdram\n");
	//dump_buf(b_info->bd_mem_addr,TEST_BUF_SIZE);

	////pr_info("Dump buf_send\n");
	//dump_buf(buf_send,TEST_BUF_SIZE);
	
	am7x_mac_set_bd_descriptor(dev, bd_idx,TEST_BUF_SIZE,BD_TX);
	am7x_mac_enable_transmit(dev);
	dump_txbd_descriptor(dev);
	spin_unlock_irqrestore(&b_info->lock, flags);
	#if 1
	/***receive buffer****/
	

	while(1){
		am7x_net_msleep(10);
		i_status =am7x_readl(dev->base_addr+MAC_Interrupt);
		//pr_info("%s,%d:interrupt status read=0x%x\n",__func__,__LINE__,i_status);
		if(int_reg->txe)
			int_reg->txe =1;
		if(int_reg->txb){
			int_reg->txb=1;
			////pr_info("Dump TX MEMSTART before clear sdram\n");
			//dump_buf(b_info->bd_mem_addr,TEST_BUF_SIZE);
		}
		if(int_reg->rxb){
			dump_receive(dev);
			////pr_info("Dump RX MEMSTART after receive \n");
			//dump_buf(b_info->bd_rx_mem_addr,TEST_BUF_SIZE);

			break;
		}
	}
	//pr_info("%s,%d:interrupt status set=0x%x\n",__func__,__LINE__,i_status);
	dump_rxbd_descriptor(dev);
	am7x_writel(i_status,dev->base_addr+MAC_Interrupt);
	test_regwrite(MAC_BaseAddr+MAC_BDDescription);
	test_regwrite(MAC_BaseAddr+MAC_BDDescription+4);
	test_regwrite(MAC_BaseAddr+MAC_BDDescription+8);
	test_regwrite(MAC_BaseAddr+MAC_TypeLenOp);
	
	#endif
		while(1);
	return 0;
}


#endif

static void am7x_phy_reset(struct net_device *dev){
	unsigned int value = 0;
	
	am7x_board_info_t *b_info=to_am7x_board(dev);
	
	value = phy_mdio_read(dev,b_info->mii.phy_id, PHY_BASIC_CONTROL);
	phy_mdio_write(dev,b_info->mii.phy_id, PHY_BASIC_CONTROL,value|0x8000);
	
	msleep(10);
	
}
static int am7x_net_open(struct net_device *dev)
{

	am7x_board_info_t *b_info=to_am7x_board(dev);

	if (netif_msg_ifup(b_info))
		dev_dbg(b_info->dev, "enabling %s\n", dev->name);
	am7x_mac_reset(dev);
	
#ifdef CONFIG_AM_8251
	
	am7x_phy_reset(dev);
	
	//msleep(10);
#endif

    if(0 == init_flag)
    {
		am7x_writel(MAC_DEFAULT_WAKEUPMODE,MAC_WAKEUP);
		am7x_writel(MAC_DEFAULT_WAKEUPIP,MAC_WAKEUP_IP);
		am7x_writel(MAC_DEFAULT_WAKEUPMASK,MAC_WAKEUP_MASK);
        am7x_writel((am7x_readl(MAC_MII_INFO) & 0xf00fffff), MAC_MII_INFO);  // 2017/4/19, T.R, default MII clk divide is 100(0x64), it's too slow, set to 0 (32 kHz)
//		printk("mode=0x%08x,ip=0x%08x,mask=0x%08x.\n",act_readl(MAC_WAKEUP),act_readl(MAC_WAKEUP_IP),act_readl(MAC_WAKEUP_MASK));	
        //init_flag = 1;
	}

	if(request_irq(dev->irq, am7x_net_interrupt,IRQF_SHARED, "am7x-net", dev)){
		printk(KERN_ERR"Request IRQ Error!Am7x_NET\n");
		return -EAGAIN;
	}
	
	mii_check_media(&b_info->mii, netif_msg_link(b_info), 1);

    //Mos: Disable EEE function only for PHY RTL8201, that use on Probox
    if ((phy_mdio_read(dev,b_info->mii.phy_id, PHY_IDENTIFY1) == 0x1c) && (phy_mdio_read(dev,b_info->mii.phy_id, PHY_IDENTIFY2) == 0xc816))
    {
        // 2016/12/20, T.R, disable EEE function to default, ref to T3342
        am7x_disable_EEE(dev);
    }
	netif_start_queue(dev);

	am7x_net_schedule_poll(b_info);

    if(0 == init_flag)
    {
        init_flag = 1;
    }
	
	return 0;
}

static int am7x_net_stop(struct net_device *ndev)
{
	am7x_board_info_t *b_info=to_am7x_board(ndev);
	
	pr_info("%s,%d\n",__func__,__LINE__);

	if(b_info){
		mutex_lock(&b_info->phy_lock);
		cancel_delayed_work_sync(&b_info->phy_poll);
		mutex_unlock(&b_info->phy_lock);
	}
	//mutex_lock(&b_info->phy_lock);
	//pr_info("%s,%d\n",__func__,__LINE__);
	netif_stop_queue(ndev);
	
	//pr_info("%s,%d\n",__func__,__LINE__);
	/* free interrupt */
	free_irq(ndev->irq, ndev);
	//pr_info("%s,%d\n",__func__,__LINE__);
	//mutex_unlock(&b_info->phy_lock);

	return 0;
}

#define sendbuf_len 1024
static int am7x_net_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	unsigned long flags;

	int bd_idx=0;
	int data_len=0;
	char *data=NULL;
	bd_pointer_t now_ptr ;
	am7x_board_info_t *b_info=to_am7x_board(dev);

	spin_lock_irqsave(&b_info->lock, flags);

#if 0 //[Sanders.130507] - Simplify Tx handling.
	am7x_mac_get_bd_pointer(dev,&b_info->txbd_pointer,BD_TX);
#else
	am7x_mac_get_bd_pointer(dev, &now_ptr, BD_TX);
	if (now_ptr.write_ptr == now_ptr.read_ptr &&
		now_ptr.write_around != now_ptr.read_around)
	{
		b_info->txdb_full = 1;
		netif_stop_queue(dev);
		spin_unlock_irqrestore(&b_info->lock, flags);
		return NETDEV_TX_BUSY;
	}
#endif
	//pr_info("TX BD write_ptr=%d,read_ptr=%d,r_round=%d,w_round=%d\n",b_info->txbd_pointer.write_ptr,b_info->txbd_pointer.read_ptr,
	//		b_info->txbd_pointer.read_around,b_info->txbd_pointer.write_around);

	data_len=skb->len;
	data = skb->data;
	//pr_info("data_len=%d,data=%s\n",data_len,data);

	if(data_len <= 0){
		if(skb)
			dev_kfree_skb(skb);
		goto _NET_XMIT_OUT;
	}

#if 0 //[Sanders.130507] - Simplify Tx handling.
	bd_idx = b_info->txbd_pointer.write_ptr;
#else
	bd_idx = now_ptr.write_ptr;
#endif
	copy_to_sdram(dev,bd_idx,BD_TX,data_len,data);	
	
	dev->trans_start = jiffies;	/* save the time stamp */

	//am7x_mac_disable_transmit(dev);
	am7x_mac_set_bd_descriptor(dev, bd_idx,data_len,BD_TX);
	if(skb)
		dev_kfree_skb(skb);
#if 0 //[Sanders.130507] - Simplify Tx handling.
	am7x_mac_get_bd_pointer(dev,&new_ptr,BD_TX);
	if(b_info->tx_bd_num == 1){
		b_info->stats.tx_packets++;
		b_info->stats.tx_bytes += data_len;
		b_info->txdb_full=1;	
		netif_stop_queue(dev);
		//am7x_mac_enable_transmit(dev);
	}
	else{

		if(new_ptr.write_ptr > new_ptr.read_ptr){
			gap = b_info->tx_bd_num - new_ptr.write_ptr + new_ptr.read_ptr;
		}
		else if(new_ptr.write_ptr == new_ptr.read_ptr){
			if(new_ptr.write_around != new_ptr.read_around){
				gap = 0;
			}
			else{
				gap = b_info->tx_bd_num;
			}
		}
		else{
			gap = new_ptr.read_ptr - new_ptr.write_ptr;
		}

		//if((new_ptr.write_ptr-new_ptr.read_ptr>=b_info->tx_bd_num-1 && new_ptr.read_around==new_ptr.write_around)
		//	||(new_ptr.read_ptr-new_ptr.write_ptr<=1 && new_ptr.read_around!=new_ptr.write_around) ){
		if(gap <= MAC_TX_BD_GAP){

			////pr_info("TX QUEUE FULL, Stop queue!%d,%d,%d,%d\n",new_ptr.write_ptr,new_ptr.read_ptr,new_ptr.write_around,new_ptr.read_around);
			b_info->txdb_full=1;	
			netif_stop_queue(dev);
		}
		//else{
			//am7x_mac_enable_transmit(dev);
			b_info->stats.tx_packets++;
			b_info->stats.tx_bytes += data_len;
		//}
		
	}
#endif

//[Sanders.130507] - Moved to 'am7x_mac_reset'
//	am7x_mac_enable_transmit(dev);

_NET_XMIT_OUT:	
	
	spin_unlock_irqrestore(&b_info->lock, flags);

	return 0;
}


static void am7x_net_timeout(struct net_device *dev)
{
	
	//pr_info("%s,%d\n",__func__,__LINE__);
	unsigned long flags;
	am7x_board_info_t *b_info=to_am7x_board(dev);
	spin_lock_irqsave(&b_info->lock, flags);
	netif_stop_queue(dev);
	/*do something else such as reset the hareware*/
	am7x_mac_reset(dev);
	
	
	netif_wake_queue(dev);
	
	/* Restore previous register address */
	
	spin_unlock_irqrestore(&b_info->lock, flags);
	
	//pr_info("%s,%d\n",__func__,__LINE__);
}


struct net_device_stats* am7x_get_stats(struct net_device *dev)
{
	
	am7x_board_info_t *b_info=to_am7x_board(dev);
	//printk("b_info->stats===================%d\n",b_info->stats);
	return &b_info->stats;
	
}

#ifdef CONFIG_NET_POLL_CONTROLLER
static void am7x_poll_controller(struct net_device *dev)
{
	//pr_info("%s,%d\n",__func__,__LINE__);

	disable_irq(dev->irq);
	am7x_net_interrupt(dev->irq, dev);
	enable_irq(dev->irq);
	
	//pr_info("%s,%d\n",__func__,__LINE__);
}
#endif

static void am7x_net_hash_table(struct net_device *dev)
{
	////pr_info("%s,%d\n",__func__,__LINE__);
}


static int am7x_net_ioctl(struct net_device *dev, struct ifreq *req, int cmd)
{
	
	//pr_info("%s,%d\n",__func__,__LINE__);
	am7x_board_info_t *b_info=to_am7x_board(dev);
	if (!netif_running(dev))
		return -EINVAL;
	return generic_mii_ioctl(&b_info->mii, if_mii(req), cmd, NULL);
	
	//pr_info("%s,%d\n",__func__,__LINE__);
}


static int am7x_net_drv_suspend(struct platform_device *dev, pm_message_t state)
{
	//pr_info("%s,%d\n",__func__,__LINE__);
	return 0;
}


static int am7x_net_drv_resume(struct platform_device *dev)
{
	//pr_info("%s,%d\n",__func__,__LINE__);
	return 0;
}

static int am_mac_release(struct inode *pinode, struct file *pfile)
{
	printk("am_mac_release\n");	
	return 0;
}

static int am_mac_ioctl(struct inode * inode, struct file * file, unsigned int cmd, unsigned long arg)//(unsigned int cmd, unsigned int arg)
{
//	unsigned int ii; 
	unsigned char wakeupmode;
	mac_address_t mac_address_temp;
	ip_address_t ip_address_temp;
	unsigned int reg_value;
	int  retval=0;
	if(_IOC_TYPE(cmd)!='d') return -ENOTTY;
	if(_IOC_NR(cmd)>MACIO_MAXNR) return -ENOTTY;
	switch (cmd) {
	case MACIO_SET_MAC_ADDR:		
		if(copy_from_user((mac_address_t *)&mac_address_temp, (mac_address_t *)arg, sizeof(mac_address_temp)))
				return -EFAULT;
        reg_value = mac_address_temp.mac_addr_0 + 0x100 * mac_address_temp.mac_addr_1 \
                    + 0x10000 * mac_address_temp.mac_addr_2 + 0x1000000 * mac_address_temp.mac_addr_3;
		act_writel(reg_value,MAC_ADDR0);
		reg_value = act_readl(MAC_ADDR1);
		reg_value &= 0xffff0000;
		reg_value += mac_address_temp.mac_addr_4 + 0x100 * mac_address_temp.mac_addr_5;
		act_writel(reg_value,MAC_ADDR1);
/*
#if EEPROM_TYPE == 1
		ii = WRITE_MAC_REP;
		if((INVALID == mac_in_eeprom.flag)\
			||(memcmp(&mac_address_temp,&mac_in_eeprom.mac_val,sizeof(mac_address_temp)))){
			while(ii){
					if(0==i2c_write_mac_addr(&mac_address_temp)){
						mac_in_eeprom.flag = (!INVALID);
						mac_in_eeprom.mac_val = mac_address_temp;
						break;
					}
					ii--;
				}
			if(0==ii){
				mac_in_eeprom.flag = (INVALID);
				printk("write new mac address to EEPROM failed\n");
				}
			}
#endif
*/
		if(get_MACAddr(dev_tmp,MACADDR_TYPE_SOURCE,dev_tmp->dev_addr))
			return -EFAULT; 
		break;
		case MACIO_GET_MAC_ADDR:	
			reg_value = act_readl(MAC_ADDR0);
			mac_address_temp.mac_addr_0 = (unsigned char)reg_value;
			mac_address_temp.mac_addr_1 = (unsigned char)(reg_value >> 8);
			mac_address_temp.mac_addr_2 = (unsigned char)(reg_value >> 16);	
			mac_address_temp.mac_addr_3 = (unsigned char)(reg_value >> 24);
			reg_value = act_readl(MAC_ADDR1);
			mac_address_temp.mac_addr_4 = (unsigned char)reg_value;
			mac_address_temp.mac_addr_5 = (unsigned char)(reg_value >> 8);			
			if(copy_to_user(( mac_address_t *)arg,( mac_address_t *)&mac_address_temp,  sizeof( mac_address_temp)))
					return -EFAULT;
//			printk("MACIO_GET_PARA\n");
			break;
		case MACIO_SET_WAKEUP_IP:
		if(copy_from_user(( ip_address_t *)&ip_address_temp, ( ip_address_t *)arg, sizeof( ip_address_temp)))
				return -EFAULT;
//		printk("MACIO_SET_WAKEUP IP\n");	
			reg_value = 0X1000000 * ip_address_temp.ip_addr_0 +  0X10000 * ip_address_temp.ip_addr_1 \
				        + 0X100 * ip_address_temp.ip_addr_2 + ip_address_temp.ip_addr_3;
			act_writel(reg_value,MAC_WAKEUP_IP);
			break;
		case MACIO_SET_WAKEUP_MASK:
		if(copy_from_user(( ip_address_t *)&ip_address_temp, ( ip_address_t *)arg, sizeof(ip_address_temp)))
				return -EFAULT;
//		printk("MACIO_SET_WAKEUP IP MASK\n");	
			reg_value = 0X1000000 * ip_address_temp.ip_addr_0 +  0X10000 * ip_address_temp.ip_addr_1 \
				        + 0X100 * ip_address_temp.ip_addr_2 + ip_address_temp.ip_addr_3;
			act_writel(reg_value,MAC_WAKEUP_MASK);
			break;		
		case MACIO_GET_WAKEUP_IP:
			reg_value = act_readl(MAC_WAKEUP_IP);
			ip_address_temp.ip_addr_0 = (unsigned char)(reg_value >> 24);
			ip_address_temp.ip_addr_1 = (unsigned char)(reg_value >> 16);
			ip_address_temp.ip_addr_2 = (unsigned char)(reg_value >> 8);
			ip_address_temp.ip_addr_3 = (unsigned char)(reg_value );
		if(copy_to_user(( ip_address_t *)arg,( ip_address_t *)&ip_address_temp,  sizeof(ip_address_temp)))
				return -EFAULT;
			
			break;	
		case MACIO_GET_WAKEUP_MASK:
			reg_value = act_readl(MAC_WAKEUP_MASK);
			ip_address_temp.ip_addr_0 = (unsigned char)(reg_value >> 24);
			ip_address_temp.ip_addr_1 = (unsigned char)(reg_value >> 16);
			ip_address_temp.ip_addr_2 = (unsigned char)(reg_value >> 8);
			ip_address_temp.ip_addr_3 = (unsigned char)(reg_value );
			if(copy_to_user(( ip_address_t *)arg, (ip_address_t *)&ip_address_temp, sizeof(ip_address_temp)))
					return -EFAULT;
//			printk("MACIO_GET_WAKEUP MASK\n");	
			break;
		case MACIO_SET_WAKEUP_MODE:
		if(copy_from_user((unsigned char *)&wakeupmode, ( unsigned char* )arg, sizeof(unsigned char)))
				return -EFAULT;
		if(wakeupmode <= 0x1f){
			reg_value = act_readl(MAC_WAKEUP);
			reg_value &= 0xffff0000;
			reg_value |= wakeupmode;
			act_writel(reg_value,MAC_WAKEUP);
			}
		else
			retval = -EINVAL;
			break;
	    case MACIO_GET_WAKEUP_MODE:
			reg_value = act_readl(MAC_WAKEUP);
			wakeupmode =(unsigned char)(reg_value & 0x1F);
		if(copy_to_user(( unsigned char* )arg,(unsigned char *)&wakeupmode,  sizeof(unsigned char)))
				return -EFAULT;
//		printk("MACIO_GET_WAKEUP MODE\n");			
			break;
        case MACIO_CONTROL_EEE_FUNCTION:
                if(1==arg)
                {
                    am7x_enbable_EEE(net_device_tmp);
                }
                else if(0==arg)
                {
                    am7x_disable_EEE(net_device_tmp);
                }
                else
                {
                    printk(KERN_ERR"%s %d wrong arg to control EEE function",__func__, __LINE__);
                }
                return retval;
        break;
#if(ENABLE_8201F_WOL_MODE)
        case MACIO_SET_WOL_MODE:
            retval = am7x_wol_test(net_device_tmp);
            return retval;
            break;
        case MACIO_RESTORE_FROM_WOL_MODE:
            am7x_wol_test_restore(net_device_tmp);
            break;
        case MACIO_SET_PHY_REG:
            am7x_set_phy_reg(arg);
            break;
        case MACIO_SET_PHY_REG_VALUE:
            am7x_set_phy_reg_value(net_device_tmp, arg);
            break;                   
        case MACIO_GET_PHY_REG:
            am7x_get_phy_reg(net_device_tmp, arg);
            break;
#endif
	    default:
			return -ENOTTY;
		}
	return retval;
}
static int am_mac_open(struct inode * inode, struct file * filp)
{	
	printk("am_mac_open\n");
 	return 0;
	
}

ssize_t am_mac_read(struct file *pfile, const char __user *buf, size_t size, loff_t *offset)//( unsigned char *PCM_DATA , unsigned int  PCM_LENS)
{
	printk("am_mac_read\n");	
	return 0;
}
ssize_t am_mac_write(struct file *pfile, char __user *buf, size_t size, loff_t *offset)//( unsigned char *PCM_DATA , unsigned int  PCM_LENS)
{		
	printk("am_mac_write\n");				
	return 0;
}

static void free_resource(am7x_board_info_t * b_info)
{
	if(b_info){
		if(b_info->bd_mem_addr){
			kfree(b_info->bd_mem_addr);
			b_info->bd_mem_addr=0;
		}
	}
}
struct cdev  *mac_cdev=NULL;
dev_t  mac_dev;
static struct  class *cdev_class=NULL; 
static struct file_operations macdrv_fops=
{
	.owner  = THIS_MODULE,	
	//.read 	= am_mac_read,
	//.write = am_mac_write,
	.ioctl = am_mac_ioctl,
	.open = am_mac_open,
	.release = am_mac_release,
};


static int __devinit
am7x_net_drv_probe(struct platform_device *pdev)
{
	am7x_board_info_t *b_info;	/* Point a board information structure */
	struct net_device *ndev=NULL;
	int ret = 0;
#if EEPROM_TYPE != 0
	struct mac_address_s mac_value;
	unsigned int  reg_temp;
	int		result = -1;
#if EEPROM_TYPE==1
	result = i2c_read_mac_addr(&mac_value);
#elif	EEPROM_TYPE==2
	result = spi_read_mac_addr(&mac_value);
#endif
	if(!result){
		reg_temp = mac_value.mac_addr_0 + 0x100 * mac_value.mac_addr_1 \
				+ 0x10000 * mac_value.mac_addr_2 + 0x1000000 * mac_value.mac_addr_3; 
		
	act_writel(reg_temp,MAC_ADDR0);
	reg_temp = act_readl(MAC_ADDR1);
	reg_temp &= 0xffff0000;
	reg_temp += mac_value.mac_addr_4 + 0x100 * mac_value.mac_addr_5;
	act_writel(reg_temp,MAC_ADDR1); 	
	mac_in_eeprom.mac_val = mac_value;
	mac_in_eeprom.flag = (!INVALID);
	}
#endif
	mac_dev =MKDEV(AM7X_MAC_MAJOR,0);
	ret = register_chrdev_region(mac_dev,1,"MAC");
	if(ret){
		printk(KERN_ERR "alloc_chrdev_region() failed for mac\n");
		return -EIO;
	}
	printk("mac major=%d, minor=%d\n",MAJOR(mac_dev),MINOR(mac_dev));	
	mac_cdev = kzalloc(sizeof(struct cdev),GFP_KERNEL);
	if(!mac_cdev){
		printk(KERN_ERR "malloc memory	fails for mac char device\n");
		unregister_chrdev_region(mac_dev,1);
		return -ENOMEM;
	}
	cdev_init(mac_cdev, &macdrv_fops);
	if(cdev_add(mac_cdev, mac_dev, 1))
		goto out_err;
	cdev_class = class_create(THIS_MODULE,"MAC");	
	if(IS_ERR(cdev_class)){
		printk(KERN_ERR "can not create a cdev_class\n");
		goto out_err;		
		}
	device_create(cdev_class,NULL,mac_dev,0,"MAC");
	goto reg_net_drv;
out_err:
	printk(KERN_ERR "register failed  for char device\n");
	kfree(mac_cdev);
	unregister_chrdev_region(mac_dev,1);
	return -ENODEV; 
reg_net_drv:
	//dump_register();
#ifdef CONFIG_AM_8251
	am_get_multi_res(MAC_I2C);
#endif
	am7x_mac_config_mulitfunc();

#if(ENABLE_8201F_WOL_MODE)
    am7x_reset_phy();  // HW reset phy
#endif
	
	ndev = alloc_etherdev(sizeof(am7x_board_info_t));
	if (!ndev) {
		dev_err(&pdev->dev, "could not allocate device.\n");
		return -ENOMEM;
	}
	//SET_NETDEV_DEV(ndev, &pdev->dev);
	b_info = to_am7x_board(ndev);
	memset(b_info, 0, sizeof(am7x_board_info_t));
	
	b_info->dev = &pdev->dev;
	b_info->ndev = ndev;
	dev_tmp=ndev;
	net_device_tmp=ndev;
	spin_lock_init(&b_info->lock);
	mutex_init(&b_info->phy_lock);
	
	INIT_DELAYED_WORK(&b_info->phy_poll, am7x_net_poll_work);
	
	b_info->bd_mem_addr = (unsigned char *)kmalloc(MTU_MAX_BUF_LEN*MAC_BD_TOTALNUM,GFP_KERNEL);		///the number of bd is MAC_BD_TOTALNUM, and each bd is 2K
	if(b_info->bd_mem_addr==NULL){
		goto __end;
	}

	b_info->bd_phy_addr= (unsigned int)__pa(b_info->bd_mem_addr);

	//pr_info("%s,%d:bd_mem_addr=0x%x,bd_phy_addr=0x%x\n",__func__,__LINE__,(unsigned int)b_info->bd_mem_addr,(unsigned int)b_info->bd_phy_addr);
	ndev->irq = IRQ_MAC;
	
	ether_setup(ndev);//this function is mostly useless, because it is done in the alloc_etherdev function

	ndev->open		 	= am7x_net_open;
	ndev->hard_start_xmit   = am7x_net_start_xmit;
	ndev->tx_timeout         	= am7x_net_timeout;
	ndev->watchdog_timeo 	= msecs_to_jiffies(watchdog);
	ndev->stop		 	= am7x_net_stop;
	ndev->set_multicast_list	= am7x_net_hash_table;
	ndev->ethtool_ops	 	= &am7x_nets_ethtool_ops;
	ndev->do_ioctl		= am7x_net_ioctl;
	ndev->get_stats		= am7x_get_stats;
	ndev->set_mac_address = am7x_set_mac_addr;

#ifdef CONFIG_NET_POLL_CONTROLLER
	ndev->poll_controller	 = am7x_poll_controller;
#endif

		
	ndev->base_addr = MAC_BaseAddr;

	init_mii_if_info(ndev);	
	ret = get_MACAddr(ndev,MACADDR_TYPE_SOURCE,ndev->dev_addr);
	if(ret)
		goto __end;

	if (!is_valid_ether_addr(ndev->dev_addr))
		dev_warn(b_info->dev, "%s: Invalid ethernet MAC address. Please "
			 "set using ifconfig\n", ndev->name);
	
	platform_set_drvdata(pdev, ndev);
	ret = register_netdev(ndev);
	if (ret == 0) {
		DECLARE_MAC_BUF(mac);
		printk(KERN_INFO "%s: MAC: %s\n", ndev->name, print_mac(mac, ndev->dev_addr));
	}

	am7x_net_proc_create(ndev);
#ifdef CONFIG_AM_8251
	am_release_multi_res(MAC_I2C);
#endif	

#if 1 //[Sanders.130507] - MAC (DRAM) priority/weight.
	do
	{
		unsigned long SDR_PRIORIYr = 0xb0070050;
		//unsigned long SDR_WEIGHTr = 0xb0070054;
		unsigned long reg_val, mac_pri, de_pri;
		reg_val = am7x_readl(SDR_PRIORIYr);
		mac_pri = (reg_val >> 24) & 0xf;
		de_pri = (reg_val >> 12) & 0xf;
		if (mac_pri < de_pri)
		{
			mac_pri = (de_pri + 1) & 0xf;
			if (mac_pri == 0x0)
			{
				mac_pri = de_pri;
			}
			reg_val |= (mac_pri << 24);
			am7x_writel(reg_val, SDR_PRIORIYr);
		}
  #if 0 //For debug purpose.
		reg_val = am7x_readl(SDR_PRIORIYr);
		printk("%s::%d::SDR_PRIORIYr.SDMA_PRIORITY(%02x)\n",
				__FUNCTION__, __LINE__, reg_val >> 28 & 0xf);
		printk("%s::%d::SDR_PRIORIYr.MAC_PRIORITY(%02x)\n",
				__FUNCTION__, __LINE__, reg_val >> 24 & 0xf);
		printk("%s::%d::SDR_PRIORIYr.AHBBUS_PRIORITY(%02x)\n",
				__FUNCTION__, __LINE__, reg_val >> 20 & 0xf);
		printk("%s::%d::SDR_PRIORIYr.TWOD_PRIORITY(%02x)\n",
				__FUNCTION__, __LINE__, reg_val >> 16 & 0xf);
		printk("%s::%d::SDR_PRIORIYr.DE_PRIORITY(%02x)\n",
				__FUNCTION__, __LINE__, reg_val >> 12 & 0xf);
		printk("%s::%d::SDR_PRIORIYr.DAC_PRIORITY(%02x)\n",
				__FUNCTION__, __LINE__, reg_val >> 8 & 0xf);
		printk("%s::%d::SDR_PRIORIYr.VDC_PRIORITY(%02x)\n",
				__FUNCTION__, __LINE__, reg_val >> 4 & 0xf);
		printk("%s::%d::SDR_PRIORIYr.AXIBUS_PRIORITY(%02x)\n",
				__FUNCTION__, __LINE__, reg_val >> 0 & 0xf);

		reg_val = am7x_readl(SDR_WEIGHTr);
		printk("%s::%d::SDR_WEIGHTr.SDMA_WEIGHT(%02x)\n",
				__FUNCTION__, __LINE__, reg_val >> 28 & 0x3);
		printk("%s::%d::SDR_WEIGHTr.MAC_WEIGHT(%02x)\n",
				__FUNCTION__, __LINE__, reg_val >> 24 & 0x3);
		printk("%s::%d::SDR_WEIGHTr.AHBBUS_WEIGHT(%02x)\n",
				__FUNCTION__, __LINE__, reg_val >> 20 & 0x3);
		printk("%s::%d::SDR_WEIGHTr.TWOD_WEIGHT(%02x)\n",
				__FUNCTION__, __LINE__, reg_val >> 16 & 0x3);
		printk("%s::%d::SDR_WEIGHTr.DE_WEIGHT(%02x)\n",
				__FUNCTION__, __LINE__, reg_val >> 12 & 0x3);
		printk("%s::%d::SDR_WEIGHTr.DAC_WEIGHT(%02x)\n",
				__FUNCTION__, __LINE__, reg_val >> 8 & 0x3);
		printk("%s::%d::SDR_WEIGHTr.AMVDC_WEIGHT(%02x)\n",
				__FUNCTION__, __LINE__, reg_val >> 4 & 0x3);
		printk("%s::%d::SDR_WEIGHTr.AXIBUS_WEIGHT(%02x)\n",
				__FUNCTION__, __LINE__, reg_val >> 0 & 0x3);
  #endif
	} while (0);
#endif
	return 0;

__end:
	
	if(ndev){
		free_netdev(ndev);
	}
	if(b_info)	
		free_resource(b_info);
	return ret;
}


static int __devexit
am7x_net_drv_remove(struct platform_device *pdev)
{
	
	struct net_device *ndev = platform_get_drvdata(pdev);
	
	am7x_board_info_t *b_info;	/* Point a board information structure */
	if(mac_cdev)
	{
		cdev_del(mac_cdev);
		kfree(mac_cdev);
	}
	device_destroy(cdev_class,mac_dev);
	class_destroy(cdev_class);
	unregister_chrdev_region(mac_dev,1);	
	
	platform_set_drvdata(pdev, NULL);
	
	b_info = to_am7x_board(ndev);
	
	free_resource(b_info);

	am7x_net_proc_destroy();

	unregister_netdev(ndev);
	
	free_netdev(ndev);		/* free device structure */
	//dev_dbg(&pdev->dev, "released and freed device\n");
	return 0;
}

static struct platform_driver am7x_net_driver = {
	.driver	= {
		.name    = "net-am7x",
		.owner	 = THIS_MODULE,
	},
	.probe   = am7x_net_drv_probe,
	.remove  = __devexit_p(am7x_net_drv_remove),
	.suspend = am7x_net_drv_suspend,
	.resume  = am7x_net_drv_resume,
};

static int __init
am7x_net_init(void)
{
	printk(KERN_INFO "%s Ethernet Driver, V%s\n", "Actions", "1.0");

	return platform_driver_register(&am7x_net_driver);
}

static void __exit
am7x_net_cleanup(void)
{
	return platform_driver_unregister(&am7x_net_driver);
}

#if(ENABLE_8201F_WOL_MODE)
static int am7x_wol_test(struct net_device *dev)
{
    unsigned long value=0,value1=0,value2=0,value3=0, RX_D, RX_E, TX_D, TX_E, Reg0_O, Reg0_F;
    unsigned short phyaddr_1=0x0, phyaddr_2=0x0, phyaddr_3=0x0, shiftbits=0x100;
    am7x_board_info_t *b_info=to_am7x_board(dev);
  
    while (0 == init_flag)
    {
        msleep(200);
        printk(KERN_INFO"am7x_wol_test:wait init_flag \n");
    };
    // Set Mac address 
    printk(KERN_INFO"Set Mac address \n");
    phy_mdio_write(dev,b_info->mii.phy_id, 31, 18);  // select page 18
    //value = am7x_mac_phy_read_rd_status(dev, 0, 31); 
    //printk(" 0x%x\n", value);

    // e.g:dev_addr[0]=D0, dev_addr[1]=C0, combine [0], [1] represent to 0xC0D0
    phyaddr_1 = b_info->ndev->dev_addr[0]+b_info->ndev->dev_addr[1]*shiftbits;
    phyaddr_2 = b_info->ndev->dev_addr[2]+b_info->ndev->dev_addr[3]*shiftbits;
    phyaddr_3 = b_info->ndev->dev_addr[4]+b_info->ndev->dev_addr[5]*shiftbits;

    // Set mac address to phy
    phy_mdio_write(dev,b_info->mii.phy_id, 16, phyaddr_1);
    value1 = phy_mdio_read(dev,b_info->mii.phy_id, 16);
    phy_mdio_write(dev,b_info->mii.phy_id, 17, phyaddr_2);
    value2 = phy_mdio_read(dev,b_info->mii.phy_id, 17);
    phy_mdio_write(dev,b_info->mii.phy_id, 18, phyaddr_3);
    value3 = phy_mdio_read(dev,b_info->mii.phy_id, 18);

    // Read back the mac addr from Phy
    if(value1 != phyaddr_1 || value2 != phyaddr_2 || value3!=phyaddr_3)
    {
        printk(KERN_INFO"Set mac addr to PHY FAIL!!\n");
        printk(KERN_INFO"Read back mac is \n%x:\n%x:\n%x:\n", value1, value2, value3);
        return -EFAULT;
    }

    // Set WOL EVENT
    printk(KERN_INFO"Set WOL enable \n");
    phy_mdio_write(dev,b_info->mii.phy_id, 31, 0x0011);  // select page 17
    value = phy_mdio_read(dev, b_info->mii.phy_id, 31); 
//    printk(" 0x%x\n", value);

    phy_mdio_write(dev,b_info->mii.phy_id, 16, ENABLE_MAGIC_PACKET_EVENT);  // enable magic packet event
    value1 = phy_mdio_read(dev, b_info->mii.phy_id, 16);
    phy_mdio_write(dev,b_info->mii.phy_id, 17, MAX_PACKET_LENGTH);  // Maximum packet length
    value2 = phy_mdio_read(dev, b_info->mii.phy_id, 17);

    if(value1 != ENABLE_MAGIC_PACKET_EVENT || value2 != MAX_PACKET_LENGTH)
    {
        printk(KERN_INFO"Set WOL FAIL!!\n");
        printk(KERN_INFO"Read back wol mode is \n%x:\n%x:\n", value1, value2);
        return -EFAULT;
    }
//    printk("Wol event: 0x%x\n Length: 0x%x\n", value1,value2);
#if (ENABLE_8201F_TX_RX_ISOLATION)
    // Enable MII TX isolation for power saveing
    printk(KERN_INFO"Enable MII TX \n");

    phy_mdio_write(dev,b_info->mii.phy_id, 31, 0x0007);
    TX_D = phy_mdio_read(dev, b_info->mii.phy_id, 20);
    phy_mdio_write(dev,b_info->mii.phy_id, 20, TX_D |ENABLE_TX_RX_ISOLATION_BIT);
    TX_E = phy_mdio_read(dev, b_info->mii.phy_id, 20);

    // Enable MII RX isolation for power saveing
    printk(KERN_INFO"Enable MII RX \n");
    phy_mdio_write(dev,b_info->mii.phy_id, 31, 0x0011);
    RX_D = phy_mdio_read(dev, b_info->mii.phy_id, 19);
    phy_mdio_write(dev,b_info->mii.phy_id, 19, RX_D|ENABLE_TX_RX_ISOLATION_BIT);
    //phy_mdio_write(dev,b_info->mii.phy_id, 19, 0xC002);
    RX_E = phy_mdio_read(dev, b_info->mii.phy_id, 19);

    if(0== (RX_E &ENABLE_TX_RX_ISOLATION_BIT) ||0 == (TX_E & ENABLE_TX_RX_ISOLATION_BIT) )
    {
        printk(KERN_INFO"Enable TX_RX ISOLATION FAIL!!\n");
        printk(KERN_INFO"%x\n", TX_E);
        printk(KERN_INFO"%x\n", RX_E);
        return -EFAULT;
    }

    // 2017/2/3, T.R, not sure this configuration needs for TX/RX isolate function, default marked it
    //phy_mdio_write(dev,b_info->mii.phy_id, 31, PHY_BASIC_CONTROL);
    //Reg0_O = phy_mdio_read(dev, b_info->mii.phy_id, PHY_BASIC_CONTROL);
    //phy_mdio_write(dev,b_info->mii.phy_id, PHY_BASIC_CONTROL, Reg0_O|ENABLE_PHY_ISOLATE_BIT);
    //Reg0_F = phy_mdio_read(dev, b_info->mii.phy_id, PHY_BASIC_CONTROL);

    printk(KERN_INFO"==========================\n");
    printk(KERN_INFO"Isolate configure ready!!\n");
    printk(KERN_INFO"RX_D %x \n", RX_D);
    printk(KERN_INFO"RX_E %x \n", TX_D);
    printk(KERN_INFO"TX_D %x \n", TX_D);
    printk(KERN_INFO"TX_E %x \n", TX_E);
    //printk("Reg0_O %x \n", Reg0_O);
    //printk("Reg0_F %x \n", Reg0_F);


#endif
    printk(KERN_INFO"==========================\n");
    printk(KERN_INFO"Enable WOL ready!!\n");
    printk(KERN_INFO"==========================\n");
    return 0;  // Wol enable success
}

static void am7x_wol_test_restore(struct net_device *dev)
{
    unsigned int value=0,value1=0,value2=0,value3=0, RX_D, RX_E, TX_D, TX_E;
    am7x_board_info_t *b_info=to_am7x_board(dev);

    while (0 == init_flag)
    {
        msleep(200);
        printk(KERN_INFO"am7x_wol_test_restore:wait init_flag \n");
    };
#if(ENABLE_8201F_TX_RX_ISOLATION)
    // 1 Disable MII TX isolation for power saveing
    printk(KERN_INFO"Disable MII TX \n");
    phy_mdio_write(dev,b_info->mii.phy_id, 31, 0x0007);
    TX_D = phy_mdio_read(dev, b_info->mii.phy_id, 20);
    phy_mdio_write(dev,b_info->mii.phy_id, 20, (TX_D & (~ENABLE_TX_RX_ISOLATION_BIT)) );
    TX_E = phy_mdio_read(dev, b_info->mii.phy_id, 20);

    // 2 Disable MII RX isolation for power saveing
    printk(KERN_INFO"Disable MII RX \n");
    phy_mdio_write(dev,b_info->mii.phy_id, 31, 0x0011);
    RX_D = phy_mdio_read(dev, b_info->mii.phy_id, 19);
    phy_mdio_write(dev,b_info->mii.phy_id, 19, (RX_D & (~ENABLE_TX_RX_ISOLATION_BIT)) );
    //phy_mdio_write(dev,b_info->mii.phy_id, 19, 0xC002);
    RX_E = phy_mdio_read(dev, b_info->mii.phy_id, 19);
#endif

    // 3 Disable all WOL event
    printk(KERN_INFO"Disable all WOL event \n");
    phy_mdio_write(dev,b_info->mii.phy_id, 31, 0x0011);
    phy_mdio_write(dev,b_info->mii.phy_id, 16, 0x0);  // clear all wol event 15:0
//    value2 = phy_mdio_read(dev, b_info->mii.phy_id, 16);

    // 4 Reset WOL
    printk(KERN_INFO"Reset WOL \n");
    phy_mdio_write(dev,b_info->mii.phy_id, 31, 0x0011);
    phy_mdio_write(dev,b_info->mii.phy_id, 17, 0x8000);  // clear all wol event 15:0
//    value3 = phy_mdio_read(dev, b_info->mii.phy_id, 17);
}

static void am7x_set_phy_reg_value(struct net_device *dev, unsigned int reg)
{
    am7x_board_info_t *b_info=to_am7x_board(dev);

    printk("Current Phy reg is %d\n", gphy_reg);
    printk("Set reg to: %d\n", reg);
    phy_mdio_write(dev, b_info->mii.phy_id, gphy_reg, reg);
}

static void am7x_set_phy_reg(unsigned int reg)
{
    gphy_reg = reg;
}

static void am7x_get_phy_reg(struct net_device *dev, unsigned long reg)
{
    unsigned int value=0;
    am7x_board_info_t *b_info=to_am7x_board(dev);

    printk("Current page is : %d\n", gphy_page);
    printk("Get Phy reg is : 0x%d\n", gphy_reg);

    value = phy_mdio_read(dev, b_info->mii.phy_id, reg);

    printk("Get Phy reg value is : 0x%x\n", value);
}

static void am7x_reset_phy(void)
{
    unsigned int reg=0;

    // Make sure the multi function pin was set to GPIO[77], for reset Phy
    reg = am7x_readl(GPIO_MFCTL2);
    //printk("reg is 0x%x\n", reg);
    am7x_writel((reg&0xfffffff3) , GPIO_MFCTL2); 
    //reg=am7x_readl(GPIO_MFCTL2); // for MIIM controller
    //printk("MFCTL2 reg is 0x%x\n", reg);

    // GPIO77 disable input
    reg = am7x_readl(GPIO_95_64INEN);
    am7x_writel((reg&(~(1<<13))), GPIO_95_64INEN);

    // GPIO77 enable output
    reg = am7x_readl(GPIO_95_64OUTEN);
    am7x_writel((reg|1<<13), GPIO_95_64OUTEN);

    // Set GPIO77 to low
    reg = am7x_readl(GPIO_95_64DAT);
    //printk("GPIO77 DATA start is 0x%x\n", reg);
    am7x_writel((reg&(~(1<<13))), GPIO_95_64DAT);
    //reg = am7x_readl(GPIO_95_64DAT);
    //printk("GPIO77 DATA change is 0x%x\n", reg);

    // wait at leat 10ms, refer to 8201f data sheet 9.1.3 Power On and PHY Reset sequence
    msleep(20);

    // Set GPIO77 to high
    am7x_writel((reg|1<<13), GPIO_95_64DAT);
    //reg = am7x_readl(GPIO_95_64DAT);
    //printk("GPIO77 DATA last 0x%x\n", reg);

    // If PHY reset sequence, MAC acces the PHY register at least 150 ms, refer to 8201f data sheet 9.1.3 Power On and PHY Reset sequence
    msleep(200);
}
#endif

static void am7x_disable_EEE(struct net_device *dev)
{
    am7x_board_info_t *b_info=to_am7x_board(dev);
    unsigned int value = 0xffff;
    // Set
    printk(KERN_INFO"Disable EEE function \n");

    phy_mdio_write(dev,b_info->mii.phy_id, 31, 0x0004);  // select page 4
    value = phy_mdio_read(dev, b_info->mii.phy_id, 16);
    printk(KERN_INFO"Write EEE_nway_disable status: 0x%x \n", value);

    phy_mdio_write(dev,b_info->mii.phy_id, 16, 0x4077);  // EEE_nway_disable
    value = phy_mdio_read(dev, b_info->mii.phy_id, 16);
    printk(KERN_INFO"Read EEE_nway_disable status: 0x%x \n", value);

    phy_mdio_write(dev,b_info->mii.phy_id, 31, 0x0000);  // select page 0

    // Turn off EEE capability
    phy_mdio_write(dev,b_info->mii.phy_id, 13, 0x0007);  // Select Address mode and MMD device=7
    phy_mdio_write(dev,b_info->mii.phy_id, 14, 0x003C);  // Set address value
    phy_mdio_write(dev,b_info->mii.phy_id, 13, 0x4007);  // Set Data mode and MMD device=7
    phy_mdio_write(dev,b_info->mii.phy_id, 14, 0x0000);  // Turn off 100BASE-TX EEE capability

    phy_mdio_write(dev,b_info->mii.phy_id,    0, 0x1200);  // Restart Auto-Negotiation
}

static void am7x_enbable_EEE(struct net_device *dev)
{
    am7x_board_info_t *b_info=to_am7x_board(dev);
    unsigned int value = 0xffff;
    // Set
    printk(KERN_INFO"Enable EEE function \n");

    phy_mdio_write(dev,b_info->mii.phy_id, 31, 0x0004);  // select page 4
    value = phy_mdio_read(dev, b_info->mii.phy_id, 16);
    printk(KERN_INFO"Write EEE_nway_disable status: 0x%x \n", value);

    phy_mdio_write(dev,b_info->mii.phy_id, 16, 0x7377);  // EEE_nway_enable
    value = phy_mdio_read(dev, b_info->mii.phy_id, 16);
    printk(KERN_INFO"Read EEE_nway_disable status: 0x%x \n", value);

    phy_mdio_write(dev,b_info->mii.phy_id, 31, 0x0000);  // select page 0

    // Turn off EEE capability
    phy_mdio_write(dev,b_info->mii.phy_id, 13, 0x0007);  // Select Address mode and MMD device=7
    phy_mdio_write(dev,b_info->mii.phy_id, 14, 0x003C);  // Set address value
    phy_mdio_write(dev,b_info->mii.phy_id, 13, 0x4007);  // Set Data mode and MMD device=7
    phy_mdio_write(dev,b_info->mii.phy_id, 14, 0x0002);  // Turn on 100BASE-TX EEE capability

    phy_mdio_write(dev,b_info->mii.phy_id,    0, 0x1200);  // Restart Auto-Negotiation
}

module_init(am7x_net_init);
module_exit(am7x_net_cleanup);

MODULE_AUTHOR("sllin");
MODULE_DESCRIPTION("actions network driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:am7x");

