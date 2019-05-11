#ifndef _AM7X_DAC_H
#define _AM7X_DAC_H

#include <sys/ioctl.h>

#define INT32U unsigned int


#ifndef EFAULT
#define EFAULT 1
#endif

#ifndef EINVAL
#define EINVAL 1
#endif


typedef struct{
	INT32U fifo_drq_enable;	//0: disable; 1:enable;
	INT32U fifo_drq_kesound_enable;					
	INT32U fifo_irq_mid_enable;	//0: disable; 1:enable;
	INT32U fifo_irq_end_enable;	//0: disable; 1:enable;
} dac_fifo_para_t;

/**
Audio Clock Divisor,output is FS*256
PLL CLK  24.576M,   22.5792M

0	0000:/1 96k 1000:Useless
1	0001:/2 48k 1001:44.1k
2	0010:/3 32k 1010:Useless
3	0011:/4 24k 1011:22.05k
4	0100:/6 16k 1100:Useless
5	0101:/8 12k 1101:11.025k
6	0110:/12 8k 1110:Useless
7	0111:Reserved 1111:Useless
*/

/** 
* @brie the macro value that set to struct dac_para_t's sample_rate 
*/
#define ACT_SAMPLE_96K		0x0
#define ACT_SAMPLE_48K		0x1
#define ACT_SAMPLE_44K		0x9
#define ACT_SAMPLE_32K		0x2
#define ACT_SAMPLE_24K		0x3
#define ACT_SAMPLE_22K		0xb
#define ACT_SAMPLE_16K		0x4
#define ACT_SAMPLE_12K		0x5
#define ACT_SAMPLE_11K		0xd
#define ACT_SAMPLE_8K		0x6

#define close_SAMPLE_11K	0
#define close_SAMPLE_22K	1
#define close_SAMPLE_33K	2
#define close_SAMPLE_44K	3	
#define close_SAMPLE_55K	4
#define close_SAMPLE_66K	5
#define close_SAMPLE_77K	6
#define close_SAMPLE_88K	7
#define close_SAMPLE_99K	8
/** 
* @brie the macro value that set to struct dac_para_t's fifo_input_sel 
*/
#define apb 0
#define i2s 1
#define pcm 2
#define adc 3
#define dac_sdram 4
/** 
* @brie the macro value that set to struct dac_para_t's precision 
*/
#define data_pre_8 0
#define data_pre_16 1
#define data_pre_32 2

/**
* @brie dac param struct.
*/
typedef struct {
	INT32U sample_rate;	//set sample rate, set 0:96k; 1:48k; 2:32k; ...... 6:8k; refer to upside;
	INT32U fifo_input_sel;
	INT32U channel;		//set stearo or mono voice; set 0 is stearo, set 1 is mono;
	INT32U precision;		//set sample precision, reserved;
} dac_para_t;	


/**
* @brie ioctl's cmd tag.
*/

#define DACIO_SET_PARA		_IOW('c',1,dac_para_t)
#define DACIO_SET_VOLUME	_IOW('c',2,unsigned char)	/* the volum's value is 0x0 ~ 0x1f */
#define DACIO_GET_VOLUME	_IOR('c',3,unsigned char)
#define DACIO_SET_FIFO		_IOW('c',4,dac_fifo_para_t)

#define DACIO_SET_STOP		_IO('c',5)
#define DACIO_SET_PAUSE  	_IO('c',6)
#define DACIO_SET_CONTINUE  _IO('c',7)
#define DACIO_SET_START		_IO('c',8)

#define DACIO_SET_PLAY_TIME _IOW('c',9,unsigned int)
#define DACIO_GET_PLAY_TIME _IOR('c',10,unsigned int)
#define DACIO_SET_WRITE_TIME _IOW('c',11,unsigned int)
#define DACIO_GET_WRITE_TIME _IOR('c',12,unsigned int)
#define DACIO_SET_ADDR _IOW('c',13,unsigned int)
#define DACIO_SET_LEN _IOW('c',14,unsigned int)
#define DACIO_SET_HDMI _IOW('c',15,unsigned int)
#define SET_AUDIO_FS _IOW('c',16,unsigned int)
#define SET_SOURCE_GAIN _IOW('c',17,unsigned int)
#define ADC_SET_STOP _IO('c',18)
#define ADC_SET_PAUSE _IO('c',19)
#define ADC_SET_CONTINUE _IO('c',20)
#define ADCIO_SET_ADDR _IOW('c',21,unsigned int)
#define ADCIO_SET_LEN _IOW('c',22,unsigned int)
#define DACIO_SET_I2S _IOW('c',23,unsigned int)
#define DACIO_GET_HDMI _IOR('c',24,unsigned int)
#define ADCIO_GET_WRITE_TIME _IOR('c',25,unsigned int)
#define ADCIO_SET_START		_IO('c',26)
#define DACIO_GET_POINTER	_IOW('c',27,unsigned int)
#define DACIO_SET_WRITE_POINTER _IOW('c',28,unsigned int)



/**
* @brie volume mapping operation.
*/

#define _MAX_VOLUME_LEVEL    16
#define _VOLUME_LEVEL_0      0
#define _VOLUME_LEVEL_1      8
#define _VOLUME_LEVEL_2      12
#define _VOLUME_LEVEL_3      16
#define _VOLUME_LEVEL_4      18
#define _VOLUME_LEVEL_5      20
#define _VOLUME_LEVEL_6      22
#define _VOLUME_LEVEL_7      24
#define _VOLUME_LEVEL_8      26
#define _VOLUME_LEVEL_9      28
#define _VOLUME_LEVEL_10     30
#define _VOLUME_LEVEL_11     32
#define _VOLUME_LEVEL_12     34
#define _VOLUME_LEVEL_13     36
#define _VOLUME_LEVEL_14     38
#define _VOLUME_LEVEL_15     40

#define _VOLUME_LEVEL_MUTE 100

#define _VOLUME_CTRL_INC    1
#define _VOLUME_CTRL_DEC    2
#define _VOLUME_CTRL_MUTE   3
#define _VOLUME_CTRL_SET    4
#define _VOLUME_CTRL_GET    5

#define HDMI_VOLUME_SW_EN 0x10


/**
* @brief init the system volume.
*/
extern int sys_volume_init();


/**
* @brief volume operations.
* @param cmd: operations for the volume.
*/
extern int sys_volume_ctrl(int cmd,void *param);
extern int sys_volume_set(int volume);

#endif


