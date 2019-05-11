#ifndef __AM7XXX_DMA_H__
#define __AM7XXX_DMA_H__
/**
*@file dma.h
*
*@author yekai
*@date 2010-01-28
*@version 0.2
*/

#include <am_types.h>
#include <linux/interrupt.h>

/**
*@addtogroup dma_driver_s
*@{
*/

/**
*@name DMA channel number definition
*@{
*/
#if !defined(CONFIG_AM_CHIP_ID)||CONFIG_AM_CHIP_ID==1201||CONFIG_AM_CHIP_ID==1205
#define NR_BUS_DMA_CHANNELS          4
#define NR_DMA_SPECIAL_CHANNELS      2
#elif CONFIG_AM_CHIP_ID==1203||CONFIG_AM_CHIP_ID==1207||CONFIG_AM_CHIP_ID==1211 || CONFIG_AM_CHIP_ID==1220 || CONFIG_AM_CHIP_ID == 1213
#define NR_BUS_DMA_CHANNELS          4
#define NR_DMA_SPECIAL_CHANNELS      8
#else
#error Unknown CONFIG_AM_CHIP_ID!
#endif
#define NUM_AM7X_DMA_CHANNELS	(NR_BUS_DMA_CHANNELS + NR_DMA_SPECIAL_CHANNELS)
/**
*@}
*/

/**
*@name DMA controller common registers 
*@{
*/
#define DMA_CON                  0xB0060000
#define DMA_IRQ_EN               0xB0060004
#define DMA_IRQ_PEND	         0xB0060008
/**
*@}
*/

/**
*@name DMA Channel Base Addresses
*@{
*/
#define DMA_CHANNEL_BASE   	0xB0060100
#define DMA_CHANNEL_LEN	    0x00000020
/**
*@}
*/

/** 
*@name DMA Channel Register Offsets 
*@{
*/
#define DMA_MODE          	0x00	
#define DMA_SRC             0x04 
#define DMA_DST             0x08
#define DMA_COUNT           0x0C
#define DMA_REMAIN          0x10
#define DMA_CMD             0x14
#define DMA_STR			0x18
#define DMA_WIN			0x1c
/**
*@}
*/

/** 
*@name DMA Mode register bits follow
*@{
*/
#define DMA_RESET                  	0x10000
#define DMA_START                  	0x01
#define DMA_PAUSE                	0x2
#define DMA_COUNT_MASK       	0x000fffff
#define DMA_TCIRQ			0x1
#define DMA_HTCIRQ			0x2
/**
*@}
*/

/** 
*@name DMA Cmd bellow
*@{
*/
#define DMA_CMD_RESET		0
#define DMA_CMD_PAUSE		1
#define DMA_CMD_START		2
#define DMA_CMD_QUERY		3
/**
*@}
*/

/**
*@name DMA Mode
*@{
*/
#define DMA_START_DEMOND		0
#define DMA_START_BLOCK		(1<<4)
#define DMA_START_STRIDE		(1<<8)
/**
*@}
*/

/**
*@name DMA Status
*@{
*/
#define DMA_STATE_MASK		0x10001
#define DMA_STATE_IDLE		1
#define DMA_STATE_DISABLE	0
#define DMA_STATE_BUSY		0x10001
/**
*@}
*/

/**
*@name DMA Timeout
*@{
*/
#if CONFIG_AM_CHIP_ID==1211 || CONFIG_AM_CHIP_ID==1220 || CONFIG_AM_CHIP_ID == 1213
#define DMA_TO_EN			(1<<28)
#elif CONFIG_AM_CHIP_ID==1203
#define DMA_TO_EN			(1<<16)
#endif
/**DMA timeout value*/
#define DMA_TIME_OUT	0xFFFFFF
/**
*@}
*/

/**
*@name DMA memcpy mode
*@{
*/
#define DMA_NORMAL_MODE		1
#define DMA_STRIDE_MODE		2
/**
*@}
*/

/**
*@brief DMA type
*/
enum {
	DMA_CHAN_TYPE_SPECIAL,	/**<special type DMA*/
	DMA_CHAN_TYPE_BUS,		/**<bus type DMA*/
	DMA_CHAN_TYPE_CH2,		/**<CH2 type DMA, used for windows copy etc.*/
	DMA_CHAN_TYPE_LBUS		/**<lowest channel of bus type*/
};

/**
*@brief struct of DMA channel info 
*/
struct am_dma_chan {
	INT32S	is_used;			/**<this channel is allocated if !=0, free if ==0*/
	INT32U 	io;				/**<base address of channel registers*/
	const INT8U 	*dev_str;		/**<device name*/
	INT32S	irq;				/**<global irq number*/
	void 	*irq_dev;			/**<device id for irq*/
	INT32U 	chan_type;		/**<DMA type*/
};

/**
*@brief struct of DMA windows copy
*/
struct dma_win{
	INT16U	src_pwidth;	/**< source width of the parent window*/
	INT16U	dst_pwidth;	/**< dstination width of the parent window*/
	INT16U	win_height;	/**< height of the child window*/
	INT16U 	win_width;	/**< widthe of the child window*/
	INT8U	per_pixel;	/**< bits of one pixel*/	
};

extern struct am_dma_chan am7x_dma_table[];

/**
*@brief request a dma channel
*
*@param[in] chan_type : dma channel type, DMA_CHAN_TYPE_SPECIAL/DMA_CHAN_TYPE_BUS
*@param[in] dev_str	: device name
*@param[in] irqhandler	: function to be called when the IRQ occurs
*@param[in] irqflags	: interrupt type flags
*@param[in] irq_dev_id	: a cookie passed back to the handler function
*@retval >=0	: dma channel number
*@retval <0	: standard error number
*/
INT32S am_request_dma(INT32U  chan_type, const INT8S *dev_str, irq_handler_t irqhandler, INT32U irqflags, void *irq_dev_id);

/**
*@brief free a dma channel
*
*@param[in] dmanr : dma channel number
*@return NULL
*/
void am_free_dma(INT32U dmanr);

/**
*@brief get a dma channel info
*
*@param[in] dmanr : dma channel number
*@return pointer to the specified channel info
*/
struct am_dma_chan *am_get_dma_chan(INT32U dmanr);

/**
*@brief set addresses of a dma channel
*
*@param[in] dmanr	: dma channel number
*@param[in] src		: device address where data from
*@param[in] dst		: device address where data to
*@retval	0	: success
*@retval <0	: standard error number
*/
INT32S am_set_dma_addr(INT32U dmanr, INT32U src, INT32U dst);

/**
*@brief config a dma channel
*
*@param[in] dmanr	: dma channel number
*@param[in] mode		: dma transfer mode
*@param[in] length	: data length of the required transfer
*@retval	0	: success
*@retval <0	: standard error number
*/
INT32S am_dma_config(INT32U dmanr, INT32U mode, INT32U length);

/**
*@brief start a dma channel
*
*@param[in] dmanr	:	dma channel number
*@param[in]	cmd		:	the start cmd
*@retval 0	: success
*@retval <0	: standard error number
*/
INT32S am_dma_start(INT32U dmanr, INT32U cmd);

/**
*@brief send a command to a dma channel
*
*@param[in] dmanr	: dma channel number
*@param[in] cmd		: the command defined above
*@retval 0	: success
*@retval <0	: standard error number
*/
INT32S am_dma_cmd(INT32U dmanr, INT32U cmd);

/**
*@brief config the irq condition of a dma channel
*
*@param[in] dmanr	: dma channel number
*@param[in] en_tc		: interrupt when whole transfer finished
*@param[in] en_htc	: interrpt when half transfer finished
*@retval 0	: success
*@retval <0	: standard error number
*/
void am_set_dma_irq(INT32U dmanr, INT32U en_tc, INT32U en_htc);

/**
*@brief clear the irq condition of a dma channel
*
*@param[in] dmanr	: dma channel number
*@param[in] clear_tc	: disable interrupt when whole transfer finished
*@param[in] clear_htc	: disable interrpt when half transfer finished
*@retval 0	: success
*@retval <0	: standard error number
*/
void am_clear_dma_irq(INT32U dmanr, INT32U clear_tc, INT32U clear_htc);

/**
*@brief get the irq pending status of a dma channel
*
*@param[in] dmanr	: dma channel number
*@param[in] en_tc		: enable whole transfer finish interrupt
*@param[in] en_htc	: enable half transfer finish interrupt
*@return irq pending status
*/
INT32S am_get_dma_irq(INT32U dmanr, INT32U en_tc, INT32U en_htc);

/**
*@brief get all dma status
*
*@param[in] NULL
*@return dma status register
*/
INT32U am_get_all_dma_state(void);

/**
*@brief get a specified dma channel status
*
*@param[in] dmanr	:  dma channel number
*@return dma status
*/
INT32U am_get_dma_state(INT32U dmanr);

/**
*@brief set timeout value to a channel
*
*@param[in] shreshold	:  timeout value
*@return NULL
*/
void am_en_dma_timeout(INT32U shreshold);

/**
*@brief disable timeout function to a channel
*
*@param[in] NULL
*@return NULL
*/
void am_dis_dma_timeout(void);

/**
*@brief set weight value to a channel
*
*@param[in] dmanr	: dma channel number
*@param[in] value		: weight value
*@retval 0 	: success
*@retval <0	: standard error number
*/
INT32S am_set_dma_weight(INT32U dmanr,INT32U value);

/**
*@brief get weight value of a channel
*
*@param[in] dmanr	: dma channel number
*@return weight value
*/
INT32S am_get_dma_weight(INT32U dmanr);

/**
*@brief memset by dma
*
*@param[in] dst		: destination address
*@param[in] src		: source address
*@param[in] count		: data length
*@retval 0	: success
*@retval <0	: standard error number
*/
INT32S am_memset_dma(void* dst, INT32U val, INT32U count);

/**
*@brief memcpy by dma
*
*@param[in] dst		: destination address
*@param[in] src		: source address
*@param[in] count		: data length
*@retval 0	: success
*@retval <0	: standard error number
*/
INT32S am_memcpy_dma(void* dst, void* src, INT32U count);

/**
 *@}
 */

#endif /* __AM7XXX_DMA_H__ */

