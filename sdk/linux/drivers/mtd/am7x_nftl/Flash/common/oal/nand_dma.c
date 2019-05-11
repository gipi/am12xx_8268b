/*
*********************************************************************************************************
*                                                USDK130
*                                          ucOS + MIPS,Nand Module
*
*                               (c) Copyright 2007, Actions Co,Ld.
*                                          All Right Reserved
* File    : nand_dma.c
* By      : nand flash group
* Version : V0.1
*********************************************************************************************************
*/
#include "flash_driver_config.h"
#include "nand_flash_driver_type.h"
#if (defined(__KERNEL__)) 
#include "do_dma.h"
#endif
INT32S request_act213x_dma(INT32S chanmode, INT8S owner[], void*p, INT32S s, void*q);

void start_dma(INT32S DMANR);
void stop_dma(INT32S dmanr);
void reset_dma(INT32S dmanr);


void set_dma_mode(INT32S dmanr, INT32S DMA_MODE);
void set_dma_src_addr(INT32S dmanr, INT32U memaddr);
void set_dma_dst_addr(INT32S dmanr, INT32U mem_addr);
void set_dma_count(INT32S dmanr, INT32S count);

void start_dma(INT32S dmanr)
{
    INT32U volatile *DMA_Command;
    DMA_Command = (INT32U *)(0xb0060000 + 0x0114 + 0x20 * dmanr);
#if BUS_DMA_BlockMode_Test==1
 *DMA_Command = (*DMA_Command) | ((1 << 0)|(1<<4));
#else
    *DMA_Command = (*DMA_Command) | (1 << 0);
#endif
}

void dma_end(INT32S dmanr)
{
    INT32U volatile *DMA_Command;
    DMA_Command = (INT32U *)(0xb0060000 + 0x0114 + 0x20 * dmanr);

    while((*DMA_Command) & (1 << 0))
    {
        ;
    }
}


void stop_dma(INT32S dmanr)
{
    INT32U volatile *DMA_Command;
    DMA_Command = (INT32U *)(0xb0060000 + 0x0114 + 0x20 * dmanr);
    *DMA_Command = ((*DMA_Command) & 0xf0) | (1 << 1);
}

void reset_dma(INT32S dmanr)
{
    INT32U volatile *DMA_Control;
    DMA_Control = (INT32U *)(0xb0060000 + 0x0000);
    *DMA_Control = ((*DMA_Control) & 0x0f) | (1 << (16 + dmanr));
}

void set_dma_mode(INT32S dmanr, INT32S DMA_MODE)
{
    INT32U volatile *DMA_Mode;
    DMA_Mode = (INT32U *)(0xb0060000 + 0x0100 + 0x20 * dmanr);
    *DMA_Mode = DMA_MODE;
}
void set_dma_src_addr(INT32S dmanr, INT32U memaddr)
{

    INT32U volatile *DMA_Src;
    DMA_Src = (INT32U *)(0xb0060000 + 0x0104 + 0x20 * dmanr);
    *DMA_Src = memaddr;

}
void set_dma_dst_addr(INT32S dmanr, INT32U mem_addr)
{
	
    INT32U volatile *DMA_Dst;
    DMA_Dst = (INT32U *)(0xb0060000 + 0x0108 + 0x20 * dmanr);
    *DMA_Dst = mem_addr;
	

}
void set_dma_count(INT32S dmanr, INT32S count)
{
    INT32U volatile *DMA_Count;
    DMA_Count = (INT32U *)(0xb0060000 + 0x010c + 0x20 * dmanr);
    *DMA_Count = count;
}

void free_act213x_dma(INT32S dmanr)
{
    return;
}




