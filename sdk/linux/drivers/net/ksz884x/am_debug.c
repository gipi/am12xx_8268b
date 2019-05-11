#include "ksz8841.h"
#define REG_BANK_SEL_OFFSET     0x0E

static unsigned short ksz884x_readw (unsigned int regno)
{
	return (*(volatile unsigned short *) regno);
}

static void ksz884x_writew (unsigned int regno, unsigned short val)
{
	*(volatile unsigned short *) regno = val;
}

void ksz884x_select_bank(unsigned short bank_number)
{
	ksz884x_writew(REG_BANK_SEL_OFFSET+KSZ8841_BASE, bank_number);
	return;
}

int am_init(void)
{
	unsigned int i;
	
    printk("ksz8841 initialized!\n");
    volatile u32 *reg;
    
     reg = (volatile u32*)(0xb01c0054); 
	*reg = (*reg & 0x0fffffff)|(0x5<<28); 
  
	 reg = (volatile u32*)(0xb01c004c); 
	*reg = (*reg & 0xffffff0f)|(0x4<<4); 
	
//	 reg =(volatile u32*)(0xb0020014); 
//	*reg = 0x03000000;//enable_int_1 

//	 reg =(volatile u32*)(0xb0020004); 
//	*reg = 0x00004000;//int_14
    
	ksz884x_select_bank(32);
	ksz884x_writew(REG16_Bank32_ChipId,0x0001);
	i = ksz884x_readw(REG16_Bank32_ChipId);	
	printk("chipid 0x%x!\n", i);	

	//set bus speed 125MHz
	ksz884x_select_bank(REG_BUS_CTRL_BANK);
	ksz884x_writew(REG_BUS_CTRL_OFFSET, 0x00);
	i = ksz884x_readw(REG_BUS_CTRL_OFFSET);	
	printk("Bus speed 0x%x!\n", i);	
	
	//set the mac address
//	ksz884x_select_bank(2);											
//	ksz884x_writew(REG16_Bank2_HostMacAddLow,bd->bi_enetaddr[4]*0x100+bd->bi_enetaddr[5]);
//	ksz884x_writew(REG16_Bank2_HostMacAddLow,bd->bi_enetaddr[2]*0x100+bd->bi_enetaddr[3]);
//	ksz884x_writew(REG16_Bank2_HostMacAddLow,bd->bi_enetaddr[0]*0x100+bd->bi_enetaddr[1]);

	//set intr enable,clear pending
	ksz884x_select_bank(18);
	ksz884x_writew(REG16_Bank18_InterruptEnable,0xff80);
	ksz884x_writew(REG16_Bank18_InterruptStatus,0x0000);
	
	//set tx:enable flow control;enable auto padding;enable auto crc;transmit enable
	ksz884x_select_bank(16);
	ksz884x_writew(REG16_Bank16_TransmitControl,0x000f);        
	
	//set rx:enable flow control;discard crc error;receive broadcast&unicast;no strip crc;receive enable
	ksz884x_writew(REG16_Bank16_ReceiveControl,0x0499);
	
	//set recieve frame point and auto increment
	ksz884x_select_bank(17);
	ksz884x_writew(REG16_Bank17_RXFrameDataPointer,0x4000);
    
    return TRUE;
}


void mem_dump_long(void *buf, int len)
{
	int i;
	unsigned int *p;
	
	for(i=0, p=(unsigned int *)buf; i<len; ) {
		if( i % 16 == 0 )
			printk("%08x:  ", p );
		printk("%08x ", *p);
		p++;
		i += 4;
		if( i % 16 == 0 )
			printk("\n");
	}
	if( i % 16 != 0 )
		printk("\n");
	printk("\n");
}

void mem_dump_byte(void *buf, int len)
{
	int i;
	volatile unsigned char *p;
	
	for(i=0, p=(unsigned char *)buf; i<len; ) {
		if( i % 16 == 0 )
			printk("%08x:  ", p );
		printk("%02x ", *p );
		p++;
		i ++ ;
		if( i % 16 == 0 )
			printk("\n");
	}
	
	if( i % 16 != 0 )
		printk("\n");
	printk("\n");
}

int mem_compare(void *src, void *dst, int count, int *pos)
{
	unsigned char *d = (unsigned char *)dst, *s = (unsigned char *)src;
	while( count-- > 0 ) {
		if( *s != *d ) {
			*pos = (int)s - (int)src;
			return (*s - *d);
		}
		s++, d++;
	}
	return 0;
}


