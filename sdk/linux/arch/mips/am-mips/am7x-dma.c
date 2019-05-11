/**
 * arch/mips/am-mips/am7x-dma.c
 *
 *dma functions for actions-micro series
 *note: cache flash part is not guaranteed as it is not implemented yet
 *
 *author: yekai
 *date:2010-03-01
 *version:0.1
 --------------
 *note: (modified by yekai)
 *   1. optimize cache flush
 *   2. export memcpy and memset by dma api
 *date:2010-07-23
 *revision:0.11
 */
 
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <dma.h>
#include <actions_io.h>
#include <actions_regs.h>
#include <am7x_dev.h>

#define DEBUG_DMA	0

#if DEBUG_DMA
#define DMSG_DMA(stuff...)		printk(stuff)
#else
#define DMSG_DMA(stuff...) 	do{}while(0)
#endif

struct am_dma_chan am7x_dma_table[NUM_AM7X_DMA_CHANNELS] = {
	{.is_used=0,},
	{.is_used=0,},
	{.is_used=0,},
	{.is_used=0,},
	{.is_used=0,},
	{.is_used=0,},
#if CONFIG_AM_CHIP_ID==1203||CONFIG_AM_CHIP_ID==1207
	{.is_used=0,},
	{.is_used=0,},
	{.is_used=0,},
	{.is_used=0,},
	{.is_used=0,},
	{.is_used=0,},
#endif
};

static const INT8U table[] = {
		5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	};
static const INT16U dma_mode[] = {
		0x0000, /*-- not aligned   --*/
		0x0000, /*-- aligned to 2  --*/
		0x0004, /*-- aligned to 4  --*/
		0x0004, /*-- aligned to 8  --*/
		0x6004, /*-- aligned to 16 --*/
#if !defined(CONFIG_AM_CHIP_ID)||CONFIG_AM_CHIP_ID==1201||CONFIG_AM_CHIP_ID==1205
		0x6004,//satisfy the requirement of de,0xa004, /*-- aligned to 32 --*/
#else
		0xa004,
#endif
	};

static int special_is_used = 0;

struct am_dma_chan *am_get_dma_chan(INT32U dmanr)
{
	if (dmanr >= NUM_AM7X_DMA_CHANNELS || am7x_dma_table[dmanr].is_used == 0)
		return NULL;

	return &am7x_dma_table[dmanr];
}

void am_set_dma_irq(INT32U dmanr, INT32U en_tc, INT32U en_htc)
{
	INT32U irq=0;

	if(en_tc)
		irq |= DMA_TCIRQ;
	if(en_htc)
		irq |= DMA_HTCIRQ;
	act_writel(act_readl(DMA_IRQEN)&~(3<<(dmanr*2)),DMA_IRQEN);
	act_writel(act_readl(DMA_IRQEN)|irq<<(dmanr*2),DMA_IRQEN);
}

void am_clear_dma_irq(INT32U dmanr, INT32U clear_tc, INT32U clear_htc)
{
	if(clear_tc){
		act_writel(act_readl(DMA_IRQEN)&~(DMA_TCIRQ<<(dmanr*2)),DMA_IRQEN);
		IRQ_CLEAR_PENDING((DMA_TCIRQ<<(dmanr*2)),DMA_IRQPD);
	}
	if(clear_htc){
		act_writel(act_readl(DMA_IRQEN)&~(DMA_HTCIRQ<<(dmanr*2)),DMA_IRQEN);
		IRQ_CLEAR_PENDING((DMA_HTCIRQ<<(dmanr*2)),DMA_IRQPD);
	}
}

INT32S am_get_dma_irq(INT32U dmanr, INT32U en_tc, INT32U en_htc)
{
	INT32U mask=0;

	if(en_tc)
		mask=DMA_TCIRQ;
	if(en_htc)
		mask|=DMA_HTCIRQ;

	return (act_readl(DMA_IRQPD)&(mask<<(2*dmanr)))?1:0;
}

INT32S am_set_dma_addr(INT32U dmanr, INT32U src, INT32U dst)
{
	struct am_dma_chan* chan= am_get_dma_chan(dmanr);

	if(chan==NULL)
		return -EINVAL;

        	DMSG_DMA("set DMA addr,src=%x,dst=%x\n",src, dst);
	act_writel(CPHYSADDR(src),chan->io+DMA_SRC);
	act_writel(CPHYSADDR(dst),chan->io+DMA_DST);

	return 0;
}

static INT32S am_set_dma_window(INT32U dmanr, INT32U window, INT32U stride)
{	
	struct am_dma_chan* chan= am_get_dma_chan(dmanr);	

	if(chan==NULL)	
		return -EINVAL;	
 
	DMSG_DMA("set DMA window=%x,stride=%x\n",window, stride);	
	act_writel(window,chan->io+DMA_WIN);	
	act_writel(stride,chan->io+DMA_STR);
	
	return 0;
}

INT32S am_dma_config(INT32U dmanr, INT32U mode, INT32U length)
{
	struct am_dma_chan* chan= am_get_dma_chan(dmanr);

	if(chan==NULL)
		return -EINVAL;

	act_writel(mode, chan->io+DMA_MODE);
	act_writel(length, chan->io+DMA_COUNT);

	return 0;
}

INT32S am_dma_start(INT32U dmanr, INT32U cmd)
{
	struct am_dma_chan* chan= am_get_dma_chan(dmanr);

	if(chan==NULL)
		return -EINVAL;

	act_writel(cmd|DMA_START, chan->io+DMA_CMD);

	return 0;
}

INT32S am_dma_cmd(INT32U dmanr, INT32U cmd)
{
	struct am_dma_chan* chan= am_get_dma_chan(dmanr);

	if(chan==NULL)
		return -EINVAL;

	switch(cmd){
	case DMA_CMD_RESET:
		act_writel((act_readl(DMA_CTL)&0xffff)|DMA_RESET<<dmanr, DMA_CTL);
		break;
	case DMA_CMD_PAUSE:
		act_writel(act_readl(chan->io+DMA_CMD)|DMA_PAUSE, chan->io+DMA_CMD);
		break;
	case DMA_CMD_START:
		act_writel(act_readl(chan->io+DMA_CMD)|DMA_START, chan->io+DMA_CMD);
		break;
	case DMA_CMD_QUERY:
		return (act_readl(chan->io+DMA_CMD)&DMA_START)?1:0;
	default:
		break;
	}

	return 0;
}

INT32S am_request_dma(INT32U  chan_type, const INT8S *dev_str,irq_handler_t irqhandler, INT32U irqflags, void *irq_dev_id)
{
	struct am_dma_chan *chan;
	INT32S i, ret;
	INT32S start,end;

	if(chan_type==DMA_CHAN_TYPE_SPECIAL){
		start = NR_BUS_DMA_CHANNELS;
		end = NUM_AM7X_DMA_CHANNELS;
	}else if(chan_type==DMA_CHAN_TYPE_BUS){     /*DMA1,3 for bus dma*/
		start = 1;
		end = NR_BUS_DMA_CHANNELS;
	}else if(chan_type==DMA_CHAN_TYPE_CH2) {     /*dma2 for stride*/
		start = 2;
		end = 3;
	}else if(chan_type==DMA_CHAN_TYPE_LBUS){
		start = NR_BUS_DMA_CHANNELS-1;
		end = NR_BUS_DMA_CHANNELS;
	}else
		return -ENODEV;

	for (i = start; i < end; i++){
		if (!(am7x_dma_table[i].is_used)){
			if((i==2)&&(chan_type!=DMA_CHAN_TYPE_CH2))
				continue;
			else
				break;
		}
	}
	
	if(i==end)
		return -EBUSY;

	chan = &am7x_dma_table[i];
	chan->is_used= 1;

	if (irqhandler){
		chan->irq = IRQ_DMA;
		if(!irq_dev_id)
			chan->irq_dev = "dma_default";	/*hared interrupts must pass in a real dev-ID*/
		else
			chan->irq_dev = irq_dev_id;
		am_clear_dma_irq(i, 1, 1);

		if ((ret = request_irq(chan->irq, irqhandler, irqflags|IRQF_SHARED|IRQF_DISABLED,dev_str, chan->irq_dev))){
			chan->irq = 0;
			chan->irq_dev = NULL;
			chan->is_used= 0;
			return ret;
		}
	}else{
		chan->irq = 0;
		chan->irq_dev = NULL;
	}

	if(i>=NR_BUS_DMA_CHANNELS && i<NUM_AM7X_DMA_CHANNELS){
		if(!special_is_used)
			am_enable_dev_clk(0,CMU_DMACLK);
		special_is_used++;
	}

	chan->io = DMA_CHANNEL_BASE + i * DMA_CHANNEL_LEN;
	chan->dev_str = dev_str;
	chan->chan_type = chan_type;
	DMSG_DMA("[DMA]request:dmanr=%d,dev_str=%s\n",i,chan->dev_str);
	return i;
}

void am_free_dma(INT32U dmanr)
{
	struct am_dma_chan *chan = am_get_dma_chan(dmanr);

        	DMSG_DMA("free:dmanr=%d,dev_str=%s\n",dmanr,chan->dev_str);
	if (!chan){
		DMSG_DMA("Trying to free DMA%d\n", dmanr);
		return;
	}

     //   if(chan->chan_type == DMA_CHAN_TYPE_CARD)
	//	return;

	am_dma_cmd(dmanr,DMA_CMD_PAUSE);
	am_dma_cmd(dmanr,DMA_CMD_RESET);
	//pause_dma(dmanr);
	//reset_dma(dmanr);
    	am_clear_dma_irq(dmanr,1,1);
	//act_writel(act_readl(DMA_IRQEN)&~((1<<(dmanr*2)) | (1<<(dmanr*2+1))),DMA_IRQEN);

	if (chan->irq)
		free_irq(chan->irq, chan->irq_dev);

	if(dmanr>=NR_BUS_DMA_CHANNELS && dmanr<NUM_AM7X_DMA_CHANNELS){
		special_is_used--;
		if(!special_is_used)
			am_disable_dev_clk(0,CMU_DMACLK);
	}

	chan->irq = 0;
	chan->irq_dev = NULL;
	chan->is_used= 0;
}

static INT32U auto_config_mode(INT32U dst, INT32U src, INT32U mode)
{
	INT8U a = table[src&0x1f];
	INT8U b = table[dst&0x1f];
	INT32U m = (dma_mode[b]<<16) | dma_mode[a];
	mode &= 0x1ff91ff9;
	mode |= m;
	DMSG_DMA("auto mode = %8x\n",mode);
	return mode;
}

static INT32S dma_memcpy(void* dst, void* src, INT32U count, INT32U flag, struct dma_win *d_win)
{
	INT32S dma_no=-1;
	INT32U mode;
	INT32U length=0,time_count=0;
	INT32U window=0,stride=0;

	if(dst==0||src==0)
		return -EFAULT;
	//act_printf("[DMA]dst=%8x,src=%8x,count=%x\n",dst,src,count);
	if(flag == DMA_NORMAL_MODE)
		dma_no = am_request_dma(DMA_CHAN_TYPE_BUS,NULL,0,0,NULL);
	else{
		window = (d_win->win_height<<16)|((d_win->win_width*d_win->per_pixel)/8);
		stride = (((d_win->dst_pwidth*d_win->per_pixel)/8)<<16)|(d_win->src_pwidth*d_win->per_pixel)/8;
		dma_no = am_request_dma(DMA_CHAN_TYPE_CH2,NULL,0,0,NULL);
	}
	if(dma_no<0)
	{
		DMSG_DMA("BUS DMA BUSY!\n");
		return -EBUSY;
	}

	if((INT32U)src<0xa0000000)
		_dma_cache_wback_inv((INT32U)src,(INT32U)src+count);
	if((INT32U)dst<0xa0000000)
		_dma_cache_wback_inv((INT32U)dst,(INT32U)dst+count);

	mode = auto_config_mode((INT32U)dst, (INT32U)src, 0xa084a084);
	while(count){
		//mode = auto_config_mode(S_LogToPhy((INT32U)src+length), S_LogToPhy((INT32U)dst+length), 0xa084a084);
		src = (void*)((INT32U)src+length);
		dst = (void*)((INT32U)dst+length);
	#if CONFIG_AM_CHIP_ID == 1203
		if(count>0xfffff)
			length = 0xfffe0;
	#elif CONFIG_AM_CHIP_ID == 1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213
		if(count>0x7fffff)
			length = 0x7fffe0;
	#endif
		else
			length = count;
		count -= length;
		//act_printf("[DMA]no=%x,mode=%x,len=%x\n",dma_no,mode,length);
		am_set_dma_addr(dma_no, CPHYSADDR(src), CPHYSADDR(dst));
		am_dma_config(dma_no, mode, length);
		if(flag == DMA_STRIDE_MODE)
		{
			am_set_dma_window(dma_no, window, stride);
			am_dma_start(dma_no, DMA_START_STRIDE);
		}else{
			am_dma_start(dma_no, DMA_START_DEMOND);
		}
		while(am_dma_cmd(dma_no,DMA_CMD_QUERY)){
			if(DMA_TIME_OUT<=time_count++)
				break;
			else
				msleep(2);
		}
		if(time_count<DMA_TIME_OUT){
			time_count=0;
		}else{
			am_dma_cmd(dma_no,DMA_CMD_RESET);
			break;
		}
	}
	am_free_dma(dma_no);

#ifdef  DEBUG_DMA
	if(memcmp(dst,src,count))
		DMSG_DMA("memcpy dma fail !!!\n");
#endif
	if(time_count==0)
		return 0;
	else
		return -EAGAIN;
}

INT32S am_memcpy_dma(void* dst, void* src, INT32U count)
{
	return dma_memcpy(dst,src,count,DMA_NORMAL_MODE,NULL);
}
EXPORT_SYMBOL(am_memcpy_dma);

INT32S am_wincpy_dma(void* dst, void* src, INT32U count, struct dma_win *d_win)
{
	return dma_memcpy(dst,src,count,DMA_STRIDE_MODE,d_win);
}

INT32S am_memset_dma(void* dst, INT32U val, INT32U count)
{
	INT32S dma_no=-1;
	void* ptr=0;
	INT32U n=0,pattern=0;
	INT32U length=0,time_count=0;

	if(dst==0)
		return -EFAULT;
	//act_printf("[DMA]dst=%8x,val=%8x,count=%x\n",dst,val,count);
	/*here deal with the unaligned part*/
	ptr = (void*)(((INT32U)dst+31)&~31);
	n = ptr - dst;
	if(n){
		if(n>count)
			n = count;
		memset(dst,val,n);
	}

	count -= n;
	if(count)
		dst = ptr;
	else
		return 0;

	dma_no = am_request_dma(DMA_CHAN_TYPE_BUS,NULL,0,0,NULL);
	if(dma_no<0){
		DMSG_DMA("BUS DMA BUSY!\n");
		return -EBUSY;
	}

	pattern = val&0xff;
	pattern = (pattern<<24)|(pattern<<16)|(pattern<<8)|pattern;
	act_writel(pattern,0xb0090000);

	if((INT32U)dst<0xa0000000)
		_dma_cache_wback_inv((INT32U)dst,(INT32U)dst+count);

	while(count){
		dst = (void*)((INT32U)dst+length);
		if(count>0xfffff)
			length = 0xfffe0;
		else
			length = count;
		count -= length;

		am_set_dma_addr(dma_no, 0xb0090000, CPHYSADDR(dst));
	#if CONFIG_AM_CHIP_ID == 1203
		am_dma_config(dma_no, 0xa084019c, length);
	#elif CONFIG_AM_CHIP_ID == 1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213
		am_dma_config(dma_no, 0xa084012c, length);
	#endif
		am_dma_start(dma_no, DMA_START_DEMOND);
		while(am_dma_cmd(dma_no,DMA_CMD_QUERY)){
			if(DMA_TIME_OUT<=time_count++)
				break;
			else
				msleep(2);
		}
		if(time_count<DMA_TIME_OUT){
			time_count=0;
		}else{
			am_dma_cmd(dma_no,DMA_CMD_RESET);
			break;
		}
	}

	am_free_dma(dma_no);

	if(time_count==0)
		return 0;
	else
		return -EAGAIN;
}
EXPORT_SYMBOL(am_memset_dma);

INT32U am_get_all_dma_state(void)
{
	return act_readl(DMA_STATE);
}

INT32U am_get_dma_state(INT32U dmanr)
{
	return (act_readl(DMA_STATE)&(DMA_STATE_MASK<<dmanr))>>dmanr;
}

void am_en_dma_timeout(INT32U shreshold)
{
	act_writel(shreshold|DMA_TO_EN,DMA_TIMEOUT);
}

void am_dis_dma_timeout(void)
{
	act_writel(0,DMA_TIMEOUT);
}

INT32S am_set_dma_weight(INT32U dmanr,INT32U value)
{
	if(dmanr<4)
		return -EINVAL;
	
	act_writel(value,DMA_WEIGHT4+(dmanr-4)*4);
	return 0;
}

INT32S am_get_dma_weight(INT32U dmanr)
{
	if(dmanr<4)
		return -EINVAL;
	
	return act_readl(DMA_WEIGHT4+(dmanr-4)*4);
}

#if 0
/* Add memset4 dma function for FUI engineer to speed up... */
#include "r4ktlb.h"
enum {  DMA_FLG_SA_FIXED=0x01, DMA_FLG_DA_FIXED=0x02 };

struct hw_dma_chan {
	INT32U  dma_mode;
	INT32U  dma_src;
	INT32U  dma_dest;
	INT32U  dma_count;
	INT32U  dma_rem;
	INT32U  dma_cmd;
// For channel 2 only
	INT32U  dma_stride;
	INT32U  dma_window;
};

static inline void am_do_dma(volatile struct hw_dma_chan *dc, INT32U mode,
	void *src, void *dst, INT32U count, int flags)
{
	const INT32U MAX_DMA_SIZE = 0xfffe0;

	src = (void *) KVA_TO_PA(src);
	dst = (void *) KVA_TO_PA(dst);
	dc->dma_mode = mode;
	while(count >= MAX_DMA_SIZE) {
		const INT32U blk_size = (count < MAX_DMA_SIZE+32) ? count : MAX_DMA_SIZE;
		dc->dma_src   = (INT32U)src;
		dc->dma_dest  = (INT32U)dst;
		dc->dma_count = blk_size;
		dc->dma_cmd   = 1;
		count -= blk_size;
		if( !(flags & DMA_FLG_SA_FIXED) )
			src += blk_size;
		if( !(flags & DMA_FLG_DA_FIXED) )
			dst += blk_size;
		while( dc->dma_cmd & 1 ) {}
	}
	if(count > 0) {
		dc->dma_src   = (INT32U)src;
		dc->dma_dest  = (INT32U)dst;
		dc->dma_count = count;
		dc->dma_cmd   = 1;
		while( dc->dma_cmd & 1 ) {}
	}
}

#define AM_GET_DMA_CHANNEL(chn)  \
	((struct hw_dma_chan *)(0xb0060100 + 0x20 * (chn)))
#define IS_ALIGNED(x,a) (((INT32U)(x) & ((a)-1)) == 0)

INT32S am_memset4_dma(void* dst, INT32U pattern, INT32U count)
{
	INT32S dma_no;
	INT32U length;

	if(dst == NULL)
		return -1;
	if( ! IS_ALIGNED(dst, 4) || ! IS_ALIGNED(count,4) ) {
		DMSG_DMA("%s: dst and count must be 4-aligned\n", __func__);
		return -2;
	}
	dma_no = act_request_act213x_dma(DMA_CHAN_TYPE_BUS,NULL,0,0,NULL);
	if(dma_no < 0)
	{
		DMSG_DMA("%s: BUS DMA BUSY!\n", __func__);
		return -3;
	}

	if((INT32U)dst<0xa0000000)
		am_r4k_dma_cache_wback_inv((INT32U)dst,count);

	act_writel(pattern,0xb0090000);
	am_do_dma(AM_GET_DMA_CHANNEL(dma_no), 0xa084019c, (void*)0xb0090000, (void*)dst,
		count, DMA_FLG_SA_FIXED );
	act_free_act213x_dma(dma_no);
	return 0;
}
#endif 

EXPORT_SYMBOL(am_request_dma);
EXPORT_SYMBOL(am_free_dma);

EXPORT_SYMBOL(am_set_dma_irq);
EXPORT_SYMBOL(am_clear_dma_irq);
EXPORT_SYMBOL(am_get_dma_irq);

EXPORT_SYMBOL(am_set_dma_addr);
EXPORT_SYMBOL(am_dma_config);
EXPORT_SYMBOL(am_dma_start);
EXPORT_SYMBOL(am_dma_cmd);
EXPORT_SYMBOL(am_en_dma_timeout);
EXPORT_SYMBOL(am_dis_dma_timeout);
EXPORT_SYMBOL(am_set_dma_weight);
EXPORT_SYMBOL(am_get_dma_weight);