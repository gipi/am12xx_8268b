#ifndef __DO_DMA_H
#define __DO_DMA_H

#define DMAFLG_SYNC       0x00 /* Transfer sync on the dma channel */ 
#define DMAFLG_ASYNC      0x01 /* Transfer async on the dma channel */ 

#define DMAFLG_SRC_FIFO   0x10
#define DMAFLG_DST_FIFO   0x20

#define MAX_DMA_SIZE      0x100000
#if __KERNEL__
//void do_dma(INT32U dest, INT32U src, INT32U count, INT32U mode, int dmanr, INT32U flags);
void start_dma(int dmanr);
void wait_dma(int dmanr);
void start_dma(INT32S DMANR);
void stop_dma(INT32S dmanr);
void reset_dma(INT32S dmanr);


void set_dma_mode(INT32S dmanr, INT32S DMA_MODE);
void set_dma_src_addr(INT32S dmanr, INT32U memaddr);
void set_dma_dst_addr(INT32S dmanr, INT32U mem_addr);
void set_dma_count(INT32S dmanr, INT32S count);
#endif
#if 1
//#define  reset_dma(dmanr)
#else
void reset_dma(dmanr);
#endif

extern INT32U NandDMAChannel;

#endif

