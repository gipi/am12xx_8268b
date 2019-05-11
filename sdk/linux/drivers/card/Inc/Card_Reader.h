#ifndef _NEW_CARD_DRIVER_H
#define _NEW_CARD_DRIVER_H



#define CRC_DRQ_SET_VALUE_B8 0xee   //drivers/card/inc
#define CRC_DRQ_SET_VALUE_B4 0x66  //drivers/card/inc
#define CRC_DRQ_SET_VALUE_B1 0x00  //drivers/card/inc
#define CRC_DRQ_SET_VALUE_D8 0xff  //drivers/card/inc
#define CRC_DRQ_SET_VALUE_D4 0x77    //drivers/card/inc
#define CRC_DRQ_SET_VALUE_D1 0x11   //drivers/card/inc

#define DMA_READ_SDRAM_32   (2<<1)|(25<<3)|(1<<8)|(0<<13)|(2<<17)|(16<<19)|(0<<24)|(0<<29)
#define DMA_READ_SDRAM_8    (2<<1)|(25<<3)|(1<<8)|(0<<13)|(0<<17)|(16<<19)|(0<<24)|(0<<29)
#define DMA_WRITE_SDRAM_32   (2<<1)|(16<<3)|(0<<8)|(0<<13)|(2<<17)|(25<<19)|(1<<24)|(0<<29)
#define DMA_WRITE_SDRAM_8     (0<<1)|(16<<3)|(0<<8)|(0<<13)|(2<<17)|(25<<19)|(1<<24)|(0<<29)

#define DMA_READ_DSPRAM_32   (2<<1)|(25<<3)|(1<<8)|(0<<13)|(2<<17)|(17<<19)|(0<<24)|(0<<29)
#define DMA_READ_DSPRAM_8     (2<<1)|(25<<3)|(1<<8)|(0<<13)|(0<<17)|(17<<19)|(0<<24)|(0<<29)
#define DMA_WRITE_DSPRAM_32  (2<<1)|(17<<3)|(0<<8)|(0<<13)|(2<<17)|(25<<19)|(1<<24)|(0<<29)
#define DMA_WRITE_DSPRAM_8    (0<<1)|(17<<3)|(0<<8)|(0<<13)|(2<<17)|(25<<19)|(1<<24)|(0<<29)


#define CARD_TRANSPORT_GOOD	   0   /* Transport good, command good	   */
#define CARD_TRANSPORT_FAILED  1   /* Transport good, command failed   */
#define CARD_TRANSPORT_NO_SENSE 2  /* Command failed, no auto-sense    */
#define CARD_TRANSPORT_ERROR   3   /* Transport bad (i.e. device dead) */
#define CF_POWER_ON			0x00
#define CF_POWER_OFF		0x01
#define SM_POWER_ON			0x00
#define SM_POWER_OFF		0x01
#define SD_POWER_ON			0x00
#define SD_POWER_OFF		0x01
#define MS_POWER_ON			0x00
#define MS_POWER_OFF		0x01
#define CARD_STOP			0x83
#define STOP_CF				0x01
#define	STOP_SM				0x02
#define STOP_SD				0x04
#define STOP_MS				0x08
#define STOP_ICC			0x10
//------------------------------
#define NO_CARD		0xff
#define CF			0
#define SM			1
#define SD			2
#define MS			3
//#define xD			4
#define CF_Card  CF
#define xD_Card  SM
#define SD_Card  SD
#define MS_Card  MS
#endif
