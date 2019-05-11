#include "keenhi_rfkey.h"

#define DBG_RFKEY
#ifdef  DBG_RFKEY
#define rf_debug(fmt,stuff...)   		pr_info("MINF[%s,%d]:"fmt"\n",__func__,__LINE__,##stuff);
#else
#define rf_debug(fmt,stuff...)   		do {} while (0)
#endif




#define P_LCDD8 		(3<<12)
#define P_CFDMARQ 	(0<<8)
#define P_CFDMACK	(0<<12)
#define P_SDCLK		(5<<12)
#define P_SPIMISO	(3<<4)


#define SPI_USE_4LINE

#define DELAY_TIME (50)

#define n_delay(nsec)	ndelay(nsec)
#define u_delay(usec)	udelay(usec)
#define m_delay(msec) 	mdelay(msec)

#define RFSCS_H	{am_set_gpio(RF_SCS_PIN,1);n_delay(DELAY_TIME);}
#define RFSCS_L	{am_set_gpio(RF_SCS_PIN,0);n_delay(DELAY_TIME);}

#define RFSCK_H	{am_set_gpio(RF_SCK_PIN,1);n_delay(DELAY_TIME);}
#define RFSCK_L	{am_set_gpio(RF_SCK_PIN,0);n_delay(DELAY_TIME);}

#define DOUT_H	{am_set_gpio(RF_SDIO_PIN,1);n_delay(20);}
#define DOUT_L	{am_set_gpio(RF_SDIO_PIN,0);n_delay(20);}

#ifdef SPI_USE_4LINE
#define DIN	am_get_gpio(RF_GIO1_PIN)
#else
#define DIN	am_get_gpio(RF_SDIO_PIN)
#endif

#define WTR  am_get_gpio(RG_GIO2_PIN)

#define k_set_bit(bit, var)	      		(var) |= 1 << (bit)
#define k_clear_bit(bit, var)	      (var) &= ~(1 << (bit))
#define k_test_bit(bit, var)	      ((var) & (1 << (bit)))
#define CMD_BIT	7	///0 control registers, 1 strobe command
#define RW_BIT	6	///0 write data to control register, 1 read data from control register

#define TIME_OUT 16
#define SCAN_MSEC	40	//the scan period is SCAN_MSEC millisecond

static unsigned int boardcast_ID=0x5475c52a; 
static unsigned int user_ID=0;
static int key_mode=0;

#define KEYMODE_USER 		0x00
#define KEYMODE_STUDY 		0x01

const unsigned char A7205Config[]=
{
    //      address   name
    //      -------   ----
    0x00,   //0x00  ; A7205_MODE_REG
    0x02,   //0x01  ; A7205_MODE_CONTROL_REG  AIF=0
    0x00,   //0x02  ; A7205_CALIBRATION_REG
    0x01,   //0x03  ; A7205_FIFO1_REG  64 Fifo depth
    0x40,   //0x04  ; A7205_FIFO2_REG
    0x00,   //0x05  ; A7205_FIFO_REG
    0x00,   //0x06  ; A7205_IDCODE_REG
    0x00,   //0x07  ; A7205_RCOSC1_REG
    0x00,   //0x08  ; A7205_RCOSC2_REG
    0x00,   //0x09  ; A7205_RCOSC3_REG
    0x00,   //0x0A  ; A7205_CKO_REG
#ifdef SPI_USE_4LINE
    0x19,   //0x0B  ; A7205_GIO1_REG		config the 7205 GIO1 as data output pin 0x19
#else
    0x01,
#endif
    0x01,   //0x0C  ; A7205_GIO2_REG		config the GIO2 as the WTR signal
    0x05,   //0x0D  ; A7205_CLOCK_REG
    0x00,   //0x0E  ; A7205_DATARATE_REG
    0x50,   //0x0F  ; A7205_PLL1_REG

/**set the base frequency=2400.001M  and the channel step=500K**/
    0x9E,   //0x10  ; A7205_PLL2_REG
    0x4B,   //0x11  ; A7205_PLL3_REG
    0x00,   //0x12  ; A7205_PLL4_REG
    0x02,   //0x13  ; A7205_PLL5_REG


    0x16,   //0x14  ; A7205_TX1_REG
    0x2B,   //0x15  ; A7205_TX2_REG
    0x12,   //0x16  ; A7205_DELAY1_REG
    0x40,   //0x17  ; A7205_DELAY2_REG
    0x62,   //0x18  ; A7205_RX_REG
    0x80,   //0x19  ; A7205_RXGAIN1
    0x80,   //0x1A  ; A7205_RXGAIN2
    0x00,   //0x1B  ; A7205_RXGAIN3
    0x0A,   //0x1C  ; A7205_RXGAIN4
    0x32,   //0x1D  ; A7205_RSSI_REG
    0x03,   //0x1E  ; A7205_ADC_REG
    0x07, // 0x0F,   //0x1F  ; A7205_CODE1_REG
    0x16,   //0x20  ; A7205_CODE2_REG
    0x00,   //0x21  ; A7205_CODE3_REG

/**write MFBS MVCS MVBS =0 for auto calibration**/	
    0x00,   //0x22  ; A7205_IFCAL1_REG
    0x00,   //0x23  ; A7205_IFCAL2_REG
    0x00,   //0x24  ; A7205_VCOCCAL_REG
    0x00,   //0x25  ; A7205_VCOCAL1_REG
    0x23,   //0x26  ; A7205_VCOCAL2_REG


    0x70,   //0x27  ; A7205_BATTERY_REG
    0x17,   //0x28  ; A7205_TXTEST_REG
    0x47,   //0x29  ; A7205_RXDEM1_REG
    0x80,   //0x2A  ; A7205_RXDEM2_REG
    0x57,   //0x2B  ; A7205_CPC_REG
    0x01,   //0x2C  ; A7205_CRYSTALTEST_REG
    0x45,   //0x2D  ; A7205_PLLTEST_REG
    0x19,   //0x2E  ; A7205_VCOTEST1_REG
    0x00,   //0x2F  ; A7205_VCOTEST2_REG
    0x01,   //0x30  ; A7205_IFAT_REG
    0x0F,   //0x31  ; A7205_RSCALE_REG
    0x00,   //0x32  ; A7205_FILTERTEST_REG
};

rfkey_info_t * rfkey_info=NULL;
int need_scan =0;

void pend_wait_sem(rfkey_info_t * rfkey_info,wait_queue_head_t *sem,unsigned int condition)
{
	wait_event_interruptible(*sem, k_test_bit(condition,rfkey_info->sync_bitflag));
	k_clear_bit(condition,rfkey_info->sync_bitflag);
}

void post_wait_sem(rfkey_info_t * rfkey_info,wait_queue_head_t *sem,unsigned int condition)
{
	k_set_bit(condition,rfkey_info->sync_bitflag);
	wake_up_interruptible(sem);
}

/********************
*@spi send one byte
*@
********************/
static void spi_txbyte(unsigned char data)
{
	int i;
	for(i=0; i<8; i++){
		RFSCK_L;
		if (data & 0x80){
			DOUT_H;//write bit
		}
		else{
			DOUT_L;
		}
		data <<= 1;
		RFSCK_H; 
	}
}

/**
spi receive one byte
**/

static unsigned char spi_rxbyte(void )
{
	unsigned char temp;
	unsigned char data_in;
	int i=0;
	temp = 0;

	for(i=0;i<8;i++){
		RFSCK_L;
		data_in = DIN;
		temp = temp*2 + ((data_in)?1:0);
		RFSCK_H; 
	}
	return temp;
}

static int A7205_write_reg(unsigned char RegAddr,unsigned char databyte)
{
	int ret=0;
	k_clear_bit(CMD_BIT,RegAddr);
	k_clear_bit(RW_BIT,RegAddr);
	RFSCK_L;
	RFSCS_L;
	spi_txbyte(RegAddr);
	spi_txbyte(databyte);
	RFSCK_L;
	RFSCS_H;

	return ret;
}

static unsigned char A7205_read_reg(unsigned char RegAddr)
{
	unsigned char value=0;
	k_clear_bit(CMD_BIT,RegAddr);
	k_set_bit(RW_BIT,RegAddr);
	RFSCK_L;
	RFSCS_L;
	spi_txbyte(RegAddr);
	value = spi_rxbyte();
	RFSCK_L;
	RFSCS_H;

	return value;
}

static int A7205_write_ID(unsigned char RegAddr,unsigned int ID)
{
	int ret=0;
	unsigned char *tmp=(unsigned char*)&ID;
	int i=0;
	k_clear_bit(CMD_BIT,RegAddr);
	k_clear_bit(RW_BIT,RegAddr);
	RFSCK_L;
	RFSCS_L;
	spi_txbyte(RegAddr);
	for(i=3;i>=0;i--){
		rf_debug("ID[%d]=0x%02x   ",i,*(tmp+i));
		spi_txbyte(*(tmp+i));
	}
	RFSCK_L;
	RFSCS_H;
	return ret;
}

static unsigned int A7205_read_ID(unsigned char RegAddr)
{
	unsigned int ID=0;
	unsigned char *tmp=(unsigned char*)&ID;
	int i=0;
	k_clear_bit(CMD_BIT,RegAddr);
	k_set_bit(RW_BIT,RegAddr);
	RFSCK_L;
	RFSCS_L;
	spi_txbyte(RegAddr);
	for(i=3;i>=0;i--){
		*(tmp+i) = spi_rxbyte();
	}
	RFSCK_L;
	RFSCS_H;
	return ID;
}


static int A7205_write_fifo(unsigned char RegAddr,unsigned char *fifo,int fifo_len)
{
	int i=0;
	k_clear_bit(CMD_BIT,RegAddr);
	k_clear_bit(RW_BIT,RegAddr);
	RFSCK_L;
	RFSCS_L;
	spi_txbyte(RegAddr);
	for(i=0;i<fifo_len;i++){
		spi_txbyte(*(fifo+i));
	}
	RFSCK_L;
	RFSCS_H;
	return fifo_len;
}

static int A7205_read_fifo(unsigned char RegAddr,unsigned char *fifo,int fifo_len)
{
	int i=0;
	k_clear_bit(CMD_BIT,RegAddr);
	k_set_bit(RW_BIT,RegAddr);
	RFSCK_L;
	RFSCS_L;
	spi_txbyte(RegAddr);
	for(i=0;i<fifo_len;i++){
		*(fifo+i)=spi_rxbyte();
	}
	RFSCK_L;
	RFSCS_H;
	return fifo_len;
}


/**
write the strobe command to A7205
**/
static int A7205_write_StrobeCmd(strobe_mode_e strobe_mode)
{
	unsigned char ByteSend=0;
	switch(strobe_mode){
		case MODE_SLEEP:
			ByteSend = 0x80;
			break;
		case MODE_IDLE:
			ByteSend = 0x90;
			break;
		case MODE_STANDBY:
			ByteSend = 0xA0;
			break;
		case MODE_PLL:
			ByteSend = 0xB0;
			break;
		case MODE_RX:
			ByteSend = 0xC0;
			break;
		case MODE_FIFO:
			ByteSend = 0xF0;
			break;
		default :
			rf_debug("Strobe Mode Error! strobe_mode=%d",strobe_mode);
			break;
	}
	RFSCK_L;
	RFSCS_L;
	spi_txbyte(ByteSend);
	RFSCK_L;
	RFSCS_H;

	return 0;
}

static int A7205_setID(unsigned int ID)
{
	unsigned int ID_read=0;
	rf_debug("Write ID=0x%x",ID);
	A7205_write_ID(A7205_IDCODE_REG,ID);
	ID_read = A7205_read_ID(A7205_IDCODE_REG);
	rf_debug("Read ID=0x%x",ID_read);
	if(ID_read == ID)
		return 0;
	else{
		rf_debug("Set ID Error!");
		return -1;
	}
	
}

static void A7205_reset(void)
{
	A7205_write_reg(A7205_MODE_REG,0x00);
}

static void A7205_config(void)
{
	int i=0;
	for(i=0;i<=0x04;i++){
		A7205_write_reg(i,A7205Config[i]);
		rf_debug("Reg[0x%x]=0x%x",i,A7205Config[i]);
	}

	for(i=7;i<=0x32;i++){
		A7205_write_reg(i,A7205Config[i]);
		rf_debug("Reg[0x%x]=0x%x",i,A7205Config[i]);
	}
}

static void dump_A7205_config(void)
{
	unsigned char reg_value=0;
	int i=0;
	for(i=0;i<=0x04;i++){
		reg_value= A7205_read_reg(i);
		rf_debug("Dump Reg[0x%x]=0x%x",i,reg_value);
	}

	for(i=7;i<=0x33;i++){
		reg_value= A7205_read_reg(i);
		rf_debug("Dump Reg[0x%x]=0x%x",i,reg_value);
	}
}

static int A7205_calibration(void)
{
	unsigned char temp=0;
	unsigned char fbcf=0,fvcc=0,vbcf=0;

	///calibration rpocedure
	A7205_write_StrobeCmd(MODE_PLL);
	A7205_write_reg(A7205_CALIBRATION_REG,0x0F);
	do{
		temp = A7205_read_reg(A7205_CALIBRATION_REG);
	}while(temp&0x0F);

	///check the calibration result
	//check IF calibration
	temp = A7205_read_reg(A7205_IFCAL1_REG);
	fbcf = temp>>4 & 0x1;

	//check VCO current calibration
	temp = A7205_read_reg(A7205_VCOCCAL_REG);
	fvcc = temp>>4 & 0x1;


	//check VCO single band calibration
	temp =  A7205_read_reg(A7205_VCOCAL1_REG);
	vbcf = temp>>3 &0x01;

	if(fbcf || fvcc ||vbcf){
		rf_debug("Calibration Error!");
		return -1;
	}

	rf_debug("RF Calibration OK!");

	return 0;
}

/**
set the frequency channel, see the spec of A7205 for details
**/
static int A7205_set_channel(unsigned char channel)
{
	A7205_write_reg(A7205_PLL1_REG,channel);
	return 0;
}


static int A7205_check_crc(void)
{
	unsigned char val=0;
	val = A7205_read_reg(A7205_MODE_REG);
	return (val&0x20)?0:1;		///if CRCF bit is 0 ,it means the received payload is correct
}

static int A7205_rx_packet(unsigned char *buf,int buflen)
{
	int ret=0;
	int timeout=0;
	unsigned int cnt=0;
	A7205_write_StrobeCmd(MODE_FIFO);
	A7205_write_StrobeCmd(MODE_RX);

	while(!WTR){
		n_delay(30);
	};
	while(WTR){
		cnt++;
		if(cnt>=TIME_OUT){
			timeout=1;
			break;
		}
		m_delay(1);
	};
	if(timeout==0){
		if(A7205_check_crc()==1){//CRC check success
			A7205_read_fifo(A7205_FIFO_REG,buf,buflen);
			ret = 0;
		}
		else{
			ret = -1; ///the received data is incorrect
			rf_debug("the received data is incorrect!");
		}
	}
	else{
		//rf_debug("RX Packet Time out!");
		ret = -1;
	}
	return ret;
}

static int A7205_receive_data(unsigned char *buf,int buflen)
{
	int ret=0;
	ret = A7205_rx_packet(buf,buflen);
	return ret;
}

/**
init A7205
**/

static int A7205_initRF(void)
{
	int ret=0;
	RFSCS_H;
	RFSCK_L;
	DOUT_H;
	rf_debug("Init RF");
	A7205_reset();
	A7205_config();
	ret = A7205_setID(boardcast_ID);
	
	if(ret==0){
		ret = A7205_calibration();
	}
	return ret;
}




static void dump_mulitfunc(void)
{
#if CONFIG_AM_CHIP_ID == 1213
	// to be done.
#else
	unsigned int value=0;
	value = am7x_readl(GPIO_MFCTL3);
	rf_debug("GPIO_MFCTL3=0x%x",value);

	value = am7x_readl(GPIO_MFCTL4);
	rf_debug("GPIO_MFCTL4=0x%x",value);

	value = am7x_readl(GPIO_MFCTL7);
	rf_debug("GPIO_MFCTL7=0x%x",value);
#endif
}


/**
config the multi function register 
**/
static int config_multi_func(void)
{
	unsigned int value=0;
	
#if CONFIG_AM_CHIP_ID == 1213
	// to be done
#else

#ifdef  CHIP_8201 ///8201
	/**config RF_SCK**/
	rf_debug("RF key Call Init Multifunction For 8201");
	value = am7x_readl(GPIO_MFCTL2);
	value = (value &~(0xf<<12))|P_LCDD8;
	am7x_writel(value, GPIO_MFCTL2);

	/**config RF_SCS RF_SDIO**/
	value = am7x_readl(GPIO_MFCTL7);
	value = (value &~(0x7<<8) & ~(0x7<<12)) | P_CFDMARQ |P_CFDMACK;
	am7x_writel(value, GPIO_MFCTL7);

	/**config RF_GIO2 **/
	value = am7x_readl(GPIO_MFCTL3);
	value = (value &~(0x7<<4)) | P_SPIMISO;
	am7x_writel(value, GPIO_MFCTL3);


	/**config RF_GIO1 **/
	value = am7x_readl(GPIO_MFCTL4);
	value = (value &~(0x7<<12) )| P_SDCLK;
	am7x_writel(value, GPIO_MFCTL4);
#else	///for 7555
	/**config RF_SCK**/

	rf_debug("RF key Call Init Multifunction For 7555");
	value = am7x_readl(GPIO_MFCTL3);
	value = (value &~(0x3<<12))|(2<<12);
	am7x_writel(value, GPIO_MFCTL3);

	/**config RF_SCS RF_SDIO**/
	value = am7x_readl(GPIO_MFCTL7);
	value = (value &~(0x7<<8) & ~(0x7<<12)) | P_CFDMARQ |P_CFDMACK;
	am7x_writel(value, GPIO_MFCTL7);

	/**config RF_GIO2 **/
	value = am7x_readl(GPIO_MFCTL3);
	value = (value &~(0x7<<4)) | P_SPIMISO;
	am7x_writel(value, GPIO_MFCTL3);


	/**config RF_GIO1 **/
	value = am7x_readl(GPIO_MFCTL4);
	value = (value &~(0x7<<12) )| P_SDCLK;
	am7x_writel(value, GPIO_MFCTL4);

	
	dump_mulitfunc();
#endif

#endif
	return 0;
}

static int spi_init(rfkey_info_t *rfkey_info)
{
	int ret=0;
	config_multi_func();
	ret = A7205_initRF();
	return ret;
}

static const unsigned char channels[]={48,02,90,144}; ///if AIF==0,read channel 0,29,49,78, else the channel num should be added 1

static int save_user_ID(unsigned char *data_rev,unsigned int *user_ID)
{
	unsigned char key=0xFF;
	unsigned char*tmp=(unsigned char*)user_ID;
	if(data_rev){
		switch(data_rev[1]){
			case 0xA0:
				tmp[3]=data_rev[0];
				break;
			case 0xA1:
				tmp[2]=data_rev[0];
				break;
			case 0xA2:
				tmp[1]=data_rev[0];
				break;
			case 0xA3:
				tmp[0]=data_rev[0];
				break;
			case 0x2E:
				key = data_rev[0];
				break;
			default:
				break;
		}
	}
	return key;
}

unsigned int study_mode_cnt=0;


void send_key_msg(rfkey_info_t * rfkey_info,unsigned char key_data)
{
	if(key_data!=0xFF){
		rfkey_info->press_info.cnt++;
		//rf_debug("Cnt===%d,unpress_cnt=%d",rfkey_info->press_info.cnt,rfkey_info->press_info.unpress_cnt);
		rfkey_info->press_info.unpress_cnt=0;
		if(rfkey_info->press_info.cnt>=3){
			rfkey_info->press_info.keysaved = key_data;
			rfkey_info->press_info.iskeystroed = 1;
			if(rfkey_info->press_info.iskeypress==0){
				rf_debug("RFKey ===%d Press down",rfkey_info->press_info.keysaved);
				rfkey_info->press_info.iskeypress = 1;
				input_report_key(rfkey_info->rfkey_dev,rfkey_info->press_info.keysaved,1);
			}
		}
	}
	else{
		if(rfkey_info->press_info.iskeypress){
			rf_debug("RFKey ===%d Press UP",rfkey_info->press_info.keysaved);
			rfkey_info->press_info.iskeypress = 0;
			input_report_key(rfkey_info->rfkey_dev,rfkey_info->press_info.keysaved,0);
				/**one report had been done**/
			input_sync(rfkey_info->rfkey_dev);
			memset(&rfkey_info->press_info,0,sizeof(struct press_info_s));
		}
		rfkey_info->press_info.unpress_cnt ++;
		if(rfkey_info->press_info.unpress_cnt>=10){///it means the key is unpressed
			//memset(&rfkey_info->press_info,0,sizeof(struct press_info_s));
		}
		
	}

	#if 0
	input_report_key(rfkey_info->rfkey_dev,key_data,1);
	input_report_key(rfkey_info->rfkey_dev,key_data,0);
	/**one report had been done**/
	input_sync(rfkey_info->rfkey_dev);
	#endif

}

void rfkey_scan(unsigned long data)
{
	rfkey_info_t * rfkey_info = (rfkey_info_t*)data;
	if(need_scan==0){
		post_wait_sem(rfkey_info,&rfkey_info->cowork,1);
		need_scan=1;
	}
	mod_timer(&rfkey_info->rfkey_timer,jiffies+msecs_to_jiffies(SCAN_MSEC));
}


void rfkey_poll(unsigned long data)
{
	rfkey_info_t * rfkey_info = (rfkey_info_t*)data;
	unsigned char data_rev[2];
	unsigned char key_data=0xFF;
	int ret=0;
	int i=0;
	/**report the key to the upper layer**/
	memset(data_rev,0xFF,2);
	//rf_debug("RF Key Scan##222222222");

#if 1
	for(i=0;i<4;i++){
		m_delay(2);
		A7205_write_StrobeCmd(MODE_PLL);  //should delay before or after the mode pll been written
		A7205_set_channel(channels[i]-1);
		m_delay(2);
		//rf_debug("Setchannel====%d",channels[i]-1);
		ret = A7205_receive_data(data_rev,2);
		if(ret==-1){
			continue;
		}
		else{
			rf_debug("receive data[channel=%d],0x%x,0x%x",channels[i],data_rev[0],data_rev[1]);
			if(key_mode==KEYMODE_STUDY){
				key_data = save_user_ID(data_rev,&user_ID);
			}
			else{
				if(data_rev[1]==0x2E){
					key_data = data_rev[0];
				}
				break;
			}
		}
	}
	send_key_msg(rfkey_info,key_data);
#else
	m_delay(2);
	A7205_write_StrobeCmd(MODE_PLL);  //should delay before or after the mode pll been written
	A7205_set_channel(channels[which_channel]-1);
	m_delay(2);
	//rf_debug("Setchannel====%d",channels[which_channel]-1);
	ret = A7205_receive_data(data_rev,2);
	if(ret==-1){
		return ;
	}
	else{
		rf_debug("receive data[channel=%d],0x%x,0x%x",channels[which_channel],data_rev[0],data_rev[1]);
		if(key_mode==KEYMODE_STUDY){
			key_data = save_user_ID(data_rev,&user_ID);
		}
		else{
			if(data_rev[1]==0x2E){
				key_data = data_rev[0];
			}
			
		}
	}

	send_key_msg(rfkey_info,key_data);
	which_channel++;
	if(which_channel>=4)
		which_channel=0;
#endif
	#if 0
	
	study_mode_cnt++;
	if(study_mode_cnt>=100){
		if(key_mode==KEYMODE_USER){
			A7205_setID(boardcast_ID);
			key_mode=KEYMODE_STUDY;
		}
		if(study_mode_cnt>=110){
			if(user_ID!=0){
				A7205_setID(user_ID);
				key_mode=KEYMODE_USER;
			}
			study_mode_cnt=0;
		}
	}

	#endif

	return ;
}

/********************

********************/
static int init_rfkey_timer(rfkey_info_t *rfkey_info)
{
	init_timer(&rfkey_info->rfkey_timer);		
	rfkey_info->rfkey_timer.data =(unsigned long)rfkey_info;
	rfkey_info->rfkey_timer.function = (void*)rfkey_scan;
	rfkey_info->rfkey_timer.expires = jiffies +msecs_to_jiffies(SCAN_MSEC);
	add_timer(&rfkey_info->rfkey_timer);	
	return 0;
}

static void delete_rfkey_timer(rfkey_info_t* rfkey_info)
{
	del_timer(&rfkey_info->rfkey_timer);
}


static void init_rfkey_keys(rfkey_info_t* rfkey_info,struct input_dev *rfkey_dev)
{
	int i=0;
	if(rfkey_info){
		for(i=0;i<RFKEY_NUM;i++){
			rfkey_info->keys[i]=0x01+i;
			set_bit(rfkey_info->keys[i],rfkey_dev->keybit);
		}
	}
}

static int init_rfkey_dev(struct input_dev * rfkey_dev)
{
	if(!rfkey_dev)
		return -1;

	rfkey_dev->name="KeenHigh RF Key";
	rfkey_dev->phys="AM/input0";
	rfkey_dev->id.bustype = BUS_HOST;
	rfkey_dev->id.vendor = 0x0010;
	rfkey_dev->id.product = 0x0010;
	rfkey_dev->id.version = 0x0100;

	rfkey_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP)|BIT_MASK(EV_PWR);
	
	rfkey_dev->keycodemax = 0;
	rfkey_dev->keycodesize = sizeof(unsigned char);
	return 0;
}

static void free_rfkey_info(rfkey_info_t * rfkey_info)
{
	if(rfkey_info){
		if(rfkey_info->rfkey_dev){
			input_free_device(rfkey_info->rfkey_dev);
			rfkey_info->rfkey_dev = NULL;
		}
		kfree(rfkey_info);
	}
}



static void set_thread_status(rfkey_info_t * rfkey_info,int new_status)
{
	if(rfkey_info){
		spin_lock_irq(&rfkey_info->lock);
		rfkey_info->thread_status = new_status;
		spin_unlock_irq(&rfkey_info->lock);
	}
}

static int check_thread_exit(rfkey_info_t * rfkey_info)
{
	int ret=0;
	if(rfkey_info){
		spin_lock_irq(&rfkey_info->lock);
		ret = (rfkey_info->thread_status==RF_THREAD_EXIT)?1:0;
		spin_unlock_irq(&rfkey_info->lock);
	}
	return ret;
}

static int rfkey_thread(void *args)
{
	rfkey_info_t * rfkey_info=(rfkey_info_t * )args;
	if(!rfkey_info)
		return -1;

	while(1){
		pend_wait_sem(rfkey_info,&rfkey_info->cowork,1);
		if(check_thread_exit(rfkey_info)==1){
			rf_debug("rf thread exit!!!!!!");
			break;
		}
		if(need_scan==1){
			rfkey_poll((unsigned long)rfkey_info);
			need_scan=0;
		}		
	}
	return 0;
}




static int rfkey_create_thread(rfkey_info_t * rfkey_info)
{
	if(rfkey_info){
		rfkey_info->thread_task = kthread_create(rfkey_thread,rfkey_info,"rfkey_poll");
		if(IS_ERR(rfkey_info->thread_task)){
			rf_debug("key poll thread err!");
			return -1;
		}
		rf_debug("create RF key Thread OK!");
		set_thread_status(rfkey_info,RF_THREAD_RUN);
		wake_up_process(rfkey_info->thread_task);
		return 0;
	}
	else
		return -1;
}

static int rfkey_stop_thread(rfkey_info_t * rfkey_info)
{
	set_thread_status(rfkey_info,RF_THREAD_EXIT);
	kthread_stop(rfkey_info->thread_task);
	return 0;
}

static int __init keenhi_rfkey_init(void)
{
	int ret=0;
	rfkey_info = kzalloc(sizeof(rfkey_info_t), GFP_KERNEL);
	if(!rfkey_info){
		rf_debug("Malloc rfkey  error!");
		return -ENOMEM;
	}
	
	spin_lock_init(&rfkey_info->lock);
	rfkey_info->rfkey_dev = input_allocate_device();
	if(!rfkey_info->rfkey_dev){
		rf_debug("Malloc rfkey_dev  error!");
		ret = -ENOMEM;
		goto __end;
	}

	if(spi_init(rfkey_info)==-1){
		ret = -ENOMEM;
		goto __end;
	}

	init_waitqueue_head(&rfkey_info->cowork);
	init_rfkey_timer(rfkey_info);
	init_rfkey_keys(rfkey_info,rfkey_info->rfkey_dev);
	init_rfkey_dev(rfkey_info->rfkey_dev);
	
        ret =  input_register_device(rfkey_info->rfkey_dev);
	if(ret!=0){
		rf_debug("Register Device RFKEY Error!");
		goto __end;
	}

	rf_debug("Regisetr KEEN HI KEY OK!");

	rfkey_create_thread(rfkey_info);
	return ret;
      
__end:
	if(ret!=0){
		if(rfkey_info){
			free_rfkey_info(rfkey_info);
			rfkey_info = NULL;
		}
	}

	return ret;
}


static void __exit keenhi_rfkey_exit(void)
{
	if(rfkey_info){
		if(rfkey_info->rfkey_dev)
			input_unregister_device(rfkey_info->rfkey_dev);

		rfkey_stop_thread(rfkey_info);
		delete_rfkey_timer(rfkey_info);
		free_rfkey_info(rfkey_info);
		rfkey_info = NULL;
	}
}


module_init(keenhi_rfkey_init);
module_exit(keenhi_rfkey_exit);
MODULE_AUTHOR("sllin");
MODULE_DESCRIPTION("RF Key For KeenHigh");
MODULE_LICENSE("GPL");


