/**
 * drivers/uartcom/uartcom.c
 *
 * Using uart to communicate
 *
 * Author: Andy
 * Date: Fri Aug 10 19:05:38 CST 2012
 * 
 */

#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <sys_msg.h>

#include "actions_regs.h"
#include "actions_io.h"
#include "am7x-freq.h"
#include "am7x_gpio.h"

#define AM_CHIP_ID 1211

#define UARTCOM_CTL		UART2_CTL
#define UARTCOM_STAT		UART2_STAT
#define UARTCOM_TXDAT		UART2_TXDAT
#define UARTCOM_RXDAT		UART2_RXDAT
#define COM_CMU_UARTCLK 	CMU_UART2CLK
#define COM_IRQ_UART		IRQ_UART2

#define UART_CTL		*(volatile INT32U*)(UART2_CTL)

#define BUFSIZE  512 
#define Serial_Brg  115200

#define FUI_KEYACTION_DOWN_MASK (KEY_ACTION_DOWN<<KEY_ACTION_MASK_SHIFT)
 
struct cdev *uartcom_cdev;
int uartcom_dev = 0;


typedef enum {
	HEADER,
	RW,
	CMD,
	DL,
	DATA,
	CHECKSUM
} State;

/* the order of enum and header_table must be same */
typedef enum {
	E_NULL = -1,
	E_KEY,
	E_DATA
} Event;

INT16U header_table[] = {
	0x2A20, // E_KEY
	0x2A21  // E_DATA
};

int header_count = 2;
State current_status = HEADER;
Event current_event = E_NULL;
INT16U raw_header = 0;
int length = 0;

// TODO: using array for quickly index the key Event
typedef enum {
	GP10_KEY_CODE_UP = 0x51,
	GP10_KEY_CODE_DOWN = 0x52,
	GP10_KEY_CODE_LEFT = 0x53,
	GP10_KEY_CODE_RIGHT = 0x54,
	GP10_KEY_CODE_POWER = 0x55,
	GP10_KEY_CODE_HOME = 0x56,
	GP10_KEY_CODE_ENTER = 0x57,
	GP10_KEY_CODE_MENU = 0x58,
	GP10_KEY_CODE_EXIT = 0x59,

	GP10_KEY_CODE_IR_UP = 0x61,
	GP10_KEY_CODE_IR_DOWN = 0x62,
	GP10_KEY_CODE_IR_LEFT = 0x63,
	GP10_KEY_CODE_IR_RIGHT = 0x64,
	GP10_KEY_CODE_IR_HOME = 0x65,
	GP10_KEY_CODE_IR_ENTER = 0x66,
	GP10_KEY_CODE_IR_MENU = 0x67,
	GP10_KEY_CODE_IR_EXIT = 0x68,

	GP10_KEY_CODE_IR_MUTE = 0x69,
	GP10_KEY_CODE_IR_AUTO = 0x6A,
	GP10_KEY_CODE_IR_BLANK = 0x6B,
	GP10_KEY_CODE_IR_KEYSTONE = 0x6C,
	GP10_KEY_CODE_IR_MODE = 0x6D,
	GP10_KEY_CODE_IR_POWER = 0x6E,
	GP10_KEY_CODE_IR_SMARTECO = 0x6F,

	GP10_KEY_CODE_IR_FFPLAY = 0x71,
	GP10_KEY_CODE_IR_BFPLAY = 0x72,
	GP10_KEY_CODE_IR_3MENU = 0x74,

	GET_RECALL_PASSWD = 0x75,
	GET_AM_SOURCE_ACT = 0x73,

	GP10_KEY_CODE_IR_B_PREVIOUS = 0x7c,
	GP10_KEY_CODE_IR_B_NEXT = 0x7d,
	GP10_KEY_CODE_IR_B_PAUSE = 0x7e,
	GP10_KEY_CODE_IR_B_PLAY = 0x7f,
	GP10_KEY_CODE_IR_B_STOP = 0x80,

} KeyType;

typedef struct {
	int start, end;
} Pair;

typedef struct {
	INT8U buffer[BUFSIZE];
	Pair pair; /* global point for data received */
} RingBuffer;

RingBuffer ring;

/* for create header search table */
static inline Event search_event(INT16U);
/* -- */


/* Event handler */
typedef void(*handler_point)(Pair);
handler_point handlers[2];
static void register_event_handler(Event, handler_point, handler_point[]);
static handler_point dispatch(Event);

typedef struct {
	Event e;
	Pair pair; // the raw data for handle
} DispatchEvent;

static void dispatch_handler(unsigned long de) {
	DispatchEvent *event = (DispatchEvent*)de;
	dispatch(event->e)(event->pair);
	kfree(event);
}
DECLARE_TASKLET(dispatch_tasklet, dispatch_handler, 0); // data is DispatchEvent
/* end of event handler */

static const struct file_operations uartcom_fops =
{
	.owner   = THIS_MODULE,
};

/* uart setting functions */
INT32U act_get_cclk_div()
{
#if AM_CHIP_ID == 1203
	return 1;
#elif AM_CHIP_ID == 1211
	return ((act_readl(CMU_BUSCLK)&(1<<2))+1);
#elif AM_CHIP_ID == 1220 || AM_CHIP_ID == 1213 
	INT32U sclk_div =2,sclk_val=*(volatile INT32U*)(CMU_BUSCLK);

	sclk_val = (sclk_val>>28)&0x7;

	switch(sclk_val)
	{
		case 1: sclk_div=2; break;
		case 2: sclk_div=5; break;
		case 3: sclk_div=6; break;
		case 4: sclk_div=3; break;
		case 5: sclk_div=7; break;
		case 6: sclk_div=8; break;
		default:;
		//__asm__ volatile ("break 0x19");break;
	}
	return sclk_div;
#endif
}

INT32U act_getCorePll(void)
{
	unsigned long val = act_readl(CMU_COREPLL);
	unsigned int spread = val&(1<<24);

#if AM_CHIP_ID == 1203
	val >>= 2;
	val &= 0x3f;
	val *= 6;
#elif AM_CHIP_ID == 1211
	unsigned long bus_val = act_readl(CMU_BUSCLK);

	if(bus_val&(1<<11)){	//DDRPLL
		val = act_readl(CMU_DDRPLL);

		val >>=2;
		val &= 0x3f;
		val *= 8;
	}else{
		val >>= 2;
		val &= 0x3f;
		val *= 6;
	}
	if(spread)
		val += 6*6;
#elif AM_CHIP_ID == 1220 ||AM_CHIP_ID == 1213
		val >>= 2;
		val &= 0x7f;
		val *= 6;
#endif

	return val;
}

INT32U act_get_sclk_div(void)
{
	INT32U sclk_div=*(volatile INT32U*)(CMU_BUSCLK);

	sclk_div = (sclk_div>>4)&1;
#if AM_CHIP_ID == 1203
	sclk_div = (sclk_div+1)*act_get_cclk_div();
#elif AM_CHIP_ID == 1211
	sclk_div ++;
#elif AM_CHIP_ID == 1220 || AM_CHIP_ID == 1213
	sclk_div = act_get_cclk_div();
#endif
	return sclk_div;
}

#if AM_CHIP_ID == 1220 ||AM_CHIP_ID == 1213
static void serial_setbrg (INT32U corePll,INT32U cclk_div)
{
	INT32U tmp;
	volatile INT32U *uart1_clk = (volatile INT32U*)(CMU_UART1CLK);
	volatile INT32U *uart2_clk = (volatile INT32U*)(CMU_UART2CLK);

	tmp = corePll*1000000/(16*Serial_Brg*cclk_div);
	tmp = corePll*1000000*2/(16*Serial_Brg*cclk_div);	
	tmp |= 0x10000;
	*uart1_clk = *uart2_clk = tmp;//0x1004d(24M)/0x10185(120M)/0x10138(96M)/0x100c3(60M)
}
#else
static void serial_setbrg (unsigned int baud)
{
    unsigned int tmp;
	INT32U *uart_clk = (volatile INT32U*)(COM_CMU_UARTCLK);
	INT32U *uart_ctl = (INT32U*)(UARTCOM_CTL);

	*uart_ctl = *uart_ctl&~(1<<15);
	tmp = get_hclk()/(16*baud);
	tmp |= 0x10000;
	*uart_clk = tmp;
	*uart_ctl = *uart_ctl|(1<<15);
}
#endif	
/* end of uart setting functions */

static handler_point dispatch(Event e) {
	return handlers[e];
}

static inline Event search_event(INT16U header) {
	int i;
	for(i = 0; i < sizeof(header_table)/sizeof(INT16U); i++) {
		if( header == header_table[i] )
			return i;
	}
	return E_NULL;
}

/* ring buffer related functions */
inline static void ring_buffer_go_forward() {
	ring.pair.end = ring.pair.end + 1;
	if( ring.pair.end >= BUFSIZE )
		ring.pair.end = 0;
}

static inline void put_to_ring_buffer(INT8U byte) {
	ring.buffer[ring.pair.end] = byte;
	ring_buffer_go_forward();
}

static void reset_ring_buffer() {
	ring_buffer_go_forward();
	ring.pair.start = ring.pair.end;
}
/* end of ring buffer related functions */

static irqreturn_t interrupt_handler(int irq, void *dev_id) {
	INT32U status = 0;
	INT8U byte = 0;
	DispatchEvent *de;

	status = act_readl(UARTCOM_STAT);

	if(status & (1<<4)) { // rx error
		goto rx_error;
	}
	
	while(!(act_readl(UARTCOM_STAT) & (1<<5))) {
		byte = act_readl(UARTCOM_RXDAT) & 0xFF;
		put_to_ring_buffer(byte);
		switch(current_status) {
			case HEADER:
				header_count--; 
				if( !header_count ) {
					header_count = 2;
					raw_header = ring.buffer[ring.pair.start];
					raw_header = raw_header << 8 | ring.buffer[ring.pair.start+1];
					current_event = search_event(raw_header);
					raw_header = 0;
					if( current_event != E_NULL ) {
						current_status++;
					} else {
						printk("unknown header!\n");
						current_status = HEADER;
						reset_ring_buffer();
					}
				}
				break;
			case RW:
				current_status++;
				break;
			case CMD:
				current_status++;
				break;
			case DL:
				length = byte;
				current_status++;
				break;
			case DATA:
				length--;
				if( length <= 0 ) {
					current_status++;
				}
				break;
			case CHECKSUM:
				/* dispatch event */
				de = kmalloc(sizeof(DispatchEvent), GFP_ATOMIC);
				if( !de ) {
					printk("memory allocation was faild\n");
					current_status = HEADER;
					reset_ring_buffer();
					goto rx_error;
				}
				dispatch_tasklet.data = (unsigned long)de; 
				de->e = current_event; de->pair = ring.pair;
				tasklet_schedule(&dispatch_tasklet);
				reset_ring_buffer();
				current_status = HEADER;
		}
	}

rx_error:
	IRQ_CLEAR_PENDING(status, UARTCOM_STAT);
	return IRQ_HANDLED;
}

static
void serial_putc (const INT8S c)
{
	/* Wait for fifo to shift out some bytes */
	while(UARTCOM_STAT&0x00000040);
	*(volatile INT32U*)UARTCOM_TXDAT = (INT8U)c;
}

/*
 * init components for uart communication
 */
static int init_uart(void) {
	int rc;
	volatile INT32U *uart_config = (volatile INT32U*)(UARTCOM_CTL);

	INT32U temp;


#if AM_CHIP_ID==1213
	// dport->save_config = act_readl(UARTCOM_CTL); // for restore after shutdown this module
	INT32U* gpio3 = (INT32U*)(GPIO_MFCTL3);
	INT32U* gpio4 = (INT32U*)(GPIO_MFCTL4);

	*gpio3 = (*gpio3 & 0x3fffffff) | 0x40000000;
	*gpio4 = (*gpio4 & 0xfffffff8) | 0x00000001;

#elif AM_CHIP_ID==1211

	INT32U* gpio5 = (INT32U*)(GPIO_MFCTL5);
	INT32U* gpio6 = (INT32U*)(GPIO_MFCTL6);

	*gpio5 = (*gpio5 & 0x0fffffff) | 0x20000000;
	*gpio6 = (*gpio6 & 0xfffffff0) | 0x00000002;
//	temp = *gpio5 & 0x0fffffff;
//	*gpio5 = temp | 0x20000000;
//	temp = *gpio6 & 0xfffffff0;
//	*gpio6 = temp | 0x00000002;

#endif


#if AM_CHIP_ID==1213
	serial_setbrg(act_getCorePll(), act_get_sclk_div() );
#else
	serial_setbrg(Serial_Brg);
#endif

	*uart_config = 0x0000cc03 |(1<<18); // Enable UART1 RX IRQ
	// *uart_config = 0x000c8403;

	// init uart
	act_writel(1,UARTCOM_STAT); /* clear IRQ pending bit */
	while((act_readl(UARTCOM_STAT) & 0x20) == 0)
		act_readl(UARTCOM_RXDAT) ;


	rc = request_irq(COM_IRQ_UART, interrupt_handler, IRQF_DISABLED, "uartcom", NULL);
	if( rc < 0 )
		printk(KERN_WARNING "uart2 irq error\n");
	else
		printk(KERN_INFO "uart2 irq ok\n");

	return 0;
}

static void send_key_press(INT32U type,INT32U key){
	struct am_sys_msg uMsg;
	key = key & 0x000000ff;
	//key  = key | 0x100; // for ir
	//printk("type = %x, key = %x\n",type,key);
	uMsg.type = SYSMSG_KEY;
	uMsg.subtype = KEY_SUBTYPE_IR;
	uMsg.dataload = key | type;
	//printk("\n=============dataload = %x\n",uMsg.dataload);
	am_put_sysmsg(uMsg);
}

static void key_handler(Pair pair) {
	int i;
	INT8U key = 0;
	if( pair.start != pair.end ) {
		printk("raw data: ");
		for(i = pair.start; i != pair.end; i=(i+1)%BUFSIZE)
			printk("%x ", ring.buffer[i]);
		printk("\n");

		switch(ring.buffer[pair.start+3]) {
			case GP10_KEY_CODE_EXIT:
			case GP10_KEY_CODE_IR_EXIT:
				key = 0x14;
				break;
			case GP10_KEY_CODE_UP:
			case GP10_KEY_CODE_IR_UP:
				key = 0x16;
				break;
			case GP10_KEY_CODE_DOWN:
			case GP10_KEY_CODE_IR_DOWN:
				key = 0x18;
				break;
			case GP10_KEY_CODE_LEFT:
			case GP10_KEY_CODE_IR_LEFT:
				key = 0x19;
				break;
			case GP10_KEY_CODE_RIGHT:
			case GP10_KEY_CODE_IR_RIGHT:
				key = 0x1b;
				break;
			case GP10_KEY_CODE_ENTER:
			case GP10_KEY_CODE_IR_ENTER:
				key = 0x1a;
				break;
			case GP10_KEY_CODE_MENU:
			case GP10_KEY_CODE_IR_MENU:
				// key = 0x0d; am_set_gpio(82, 0); printk("set gpio 82 to 0\n");
				break;
			case GP10_KEY_CODE_IR_MUTE:
				key = 0x09;
				break;
			case GP10_KEY_CODE_IR_KEYSTONE:
				key = 0x3c;
				break;
			case GP10_KEY_CODE_IR_MODE:
				key = 0x3d;
				break;
			case GP10_KEY_CODE_HOME:
			case GP10_KEY_CODE_IR_HOME:
				// key = 0x3e;
				am_set_gpio(82, 1);
				printk("set gpio 82 to 1\n");
				break;
			case GP10_KEY_CODE_IR_FFPLAY:
				// key = 0x0c;
				am_set_gpio(83, 1);
				printk("set gpio 83 to 1\n");
				break;
			case GP10_KEY_CODE_IR_BFPLAY:
				// key = 0x15;
				am_set_gpio(83, 0);
				printk("set gpio 83 to 0\n");
				break;
			case GP10_KEY_CODE_IR_3MENU:
				key = 0x49;
				break;
			case GP10_KEY_CODE_IR_B_PREVIOUS:
				key = 0x50;
				break;
			case GP10_KEY_CODE_IR_B_NEXT:
				key = 0x51;
				break;
			case GP10_KEY_CODE_IR_B_PAUSE:
				key = 0x52;
				break;
			case GP10_KEY_CODE_IR_B_PLAY:
				key = 0x53;
				break;
			case GP10_KEY_CODE_IR_B_STOP:
				key = 0x54;
				break;
		}
		serial_putc(0xFF);
		printk("key: %x\n", key);
		send_key_press(FUI_KEYACTION_DOWN_MASK,key);
	}
}

static void data_handler(Pair pair) {
	int i;
	printk("Pair: (%d, %d) \n", pair.start, pair.end);
	printk("## UART DATA RECEIVED ## ");

	for(i = pair.start; i != pair.end; i=(i+1)%BUFSIZE) {
		printk("%x ", ring.buffer[i]);
	}
	printk("\n");
}

static void init_event_dispatch() {
	handlers[E_KEY] = key_handler;
	handlers[E_DATA] = data_handler;

}

static int  __init uartcom_init(void)
{
	int err, result;

	printk(KERN_INFO "uartcom init...\n");

	result = alloc_chrdev_region(&uartcom_dev, 0, 1, "uartcomdev");

	if( result < 0 ) return result;

	printk(KERN_INFO "uartcom (major, minor) = (%d, %d)\n", MAJOR(uartcom_dev), MINOR(uartcom_dev));

	uartcom_cdev = cdev_alloc();
  	cdev_init(uartcom_cdev, &uartcom_fops);
	err = cdev_add(uartcom_cdev, uartcom_dev, 1);
	if( err ) printk(KERN_NOTICE "Error %d adding uartcom", err);

	init_uart();
	init_event_dispatch();

	return 0;
}

static void __exit uartcom_exit(void)
{
	if( uartcom_cdev ) {
		cdev_del(uartcom_cdev);
	}

	if( uartcom_dev ) {
		unregister_chrdev_region(uartcom_dev, 1);
	}
	// TODO: release irq
	// TODO: kill dispatch_tasklet
}


module_init(uartcom_init);
module_exit(uartcom_exit);

MODULE_AUTHOR("Andy");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Using uart to communicate");
