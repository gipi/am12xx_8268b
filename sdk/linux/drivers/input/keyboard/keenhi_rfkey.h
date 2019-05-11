#ifndef KEENHI_RFKEY_H
#define KEENHI_RFKEY_H

#include "am_types.h"
#include "actions_regs.h"
#include "am7x_gpio.h"
#include "actions_io.h"
#include <linux/io.h>
#include <linux/spinlock_types.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/kthread.h>
#include <linux/wait.h>

#ifdef  __cplusplus
extern "C" {
#endif

#define A7205_MODE_REG        				0x00			///mode register
#define A7205_MODE_CONTROL_REG		0x01			///mode control register
#define A7205_CALIBRATION_REG 			0x02			///calibration control register
#define A7205_FIFO1_REG					0x03       		///fifo register 1
#define A7205_FIFO2_REG       				0x04			///fifo register 2
#define A7205_FIFO_REG      				0x05  		///fifo data register
#define A7205_IDCODE_REG      				0x06			///id data register
#define A7205_RCOSC1_REG    				0x07  		///rc osc register 1
#define A7205_RCOSC2_REG 				0x08     		///rc osc register 2
#define A7205_RCOSC3_REG      				0x09			///rc osc register 3
#define A7205_CKO_REG    					0x0a     		///cko pin control register
#define A7205_GIO1_REG       				0x0b 		///GIO1 pin control register 1
#define A7205_GIO2_REG       				0x0c 		///GIO2 pin control register 2
#define A7205_CLOCK_REG  				0x0d     		///clock register
#define A7205_DATARATE_REG    			0x0e			///data rate register
#define A7205_PLL1_REG      				0x0f  		///PLL register 1
#define A7205_PLL2_REG       				0x10 		///PLL register 2
#define A7205_PLL3_REG     				0x11   		///PLL register 3
#define A7205_PLL4_REG   					0x12     		///PLL register 4
#define A7205_PLL5_REG  					0x13      		///PLL register 5
#define A7205_TX1_REG    					0x14     		///reserved 1
#define A7205_TX2_REG         				0x15			///reserved 2
#define A7205_DELAY1_REG   				0x16   		///delay register 1
#define A7205_DELAY2_REG      				0x17			///delay register 2
#define A7205_RX_REG          				0x18			///Rx register
#define A7205_RXGAIN1         				0x19			///RX gain register 1
#define A7205_RXGAIN2        				0x1a	 		///RX gain register 2
#define A7205_RXGAIN3   					0x1b     		///RX gain register 3
#define A7205_RXGAIN4        				0x1c 		///RX gain register 4
#define A7205_RSSI_REG        				0x1d			///RSSI Threshold register
#define A7205_ADC_REG         				0x1e			///ADC Control register
#define A7205_CODE1_REG       				0x1f			///code register 1
#define A7205_CODE2_REG      				0x20	 		///code register 2
#define A7205_CODE3_REG       				0x21     		///code register 3
#define A7205_IFCAL1_REG      				0x22   		///IF calibration register 1
#define A7205_IFCAL2_REG      				0x23 		///IF calibration register 2
#define A7205_VCOCCAL_REG     			0x24 		///VCO current calibration register
#define A7205_VCOCAL1_REG     			0x25 		///VCO single band calibration register 1
#define A7205_VCOCAL2_REG     			0x26   		///VCO single band calibration register 2
#define A7205_BATTERY_REG  				0x27 		///Battery detect register
#define A7205_TXTEST_REG      				0x28 		///Reserved 3
#define A7205_RXDEM1_REG      				0x29 		///Rx DEM test Register 1
#define A7205_RXDEM2_REG      				0x2a    		///Rx DEM test Register 2
#define A7205_CPC_REG             				0x2b   		///Charge Pump current Register
#define A7205_CRYSTALTEST_REG     		0x2c   		///Crystal test register
#define A7205_PLLTEST_REG         			0x2d     		///PLL test Register
#define A7205_VCOTEST1_REG        			0x2e 		///VCO test register 1
#define A7205_VCOTEST2_REG        			0x2f 		///VCO test register 2
#define A7205_IFAT_REG      				0x30 		///IFAT register
#define A7205_RSCALE_REG      				0x31			///Rscale register
#define A7205_FILTERTEST_REG  			0x32			///filter test register
#define A7205_TMV_REG           				0x33

//#define CHIP_8201 1
#ifdef  CHIP_8201 ///8201
#define RF_SCS_PIN		64			///P_CFDMACK 	GPIO64
#define RF_SCK_PIN		58			///P_LCDD8 	GPIO58  
#define RF_SDIO_PIN		63			///P_CFDMARQ	GPIO63
#define RF_GIO1_PIN		42			///P_SDCLK 	GPIO42
#define RG_GIO2_PIN		31			///P_SPIMISO	GPIO31
#else
#define RF_SCS_PIN		64			///P_CFDMACK PIN89:GPIO64
#define RF_SCK_PIN		29			///P_XDRB PIN58:GPIO29  
#define RF_SDIO_PIN		63			///P_CFDMARQ PIN88:GPIO63
#define RF_GIO1_PIN		42			///P_SDCLK PIN68:GPIO42
#define RG_GIO2_PIN		31			///P_SPIMISO PIN56:GPIO31

#endif





typedef enum{
	MODE_SLEEP,
	MODE_IDLE,
	MODE_STANDBY,
	MODE_PLL,
	MODE_RX,
	MODE_FIFO
}strobe_mode_e;


#define RFKEY_NUM 8

struct press_info_s{
	int cnt;
	int unpress_cnt;
	unsigned char keysaved;
	char iskeystroed;
	char iskeypress;
};

enum{
	RF_THREAD_RUN,
	RF_THREAD_SLEEP,
	RF_THREAD_EXIT,
};

typedef struct rfkey_info_s
{
	spinlock_t  lock;
	wait_queue_head_t cowork;
	int thread_status;
	unsigned int sync_bitflag;
#define RF_WORK_READY 1
	struct timer_list rfkey_timer;
	struct press_info_s press_info;
	struct input_dev * rfkey_dev;
	struct delayed_work key_poll;///key status poll]
	struct task_struct *thread_task;
	unsigned int keys[RFKEY_NUM];
}rfkey_info_t;





#ifdef  __cplusplus
}
#endif

#endif
