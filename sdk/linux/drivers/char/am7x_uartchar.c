/**
 * drivers/char/am7x_uartchar.c
 *
 * Using uart to communicate
 *
 * Author: Charles
 * Date: 2012/11/17
 * 
 */

#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/major.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/semaphore.h>
#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/delay.h>


#include <sys_msg.h>

#include "actions_regs.h"
#include "actions_io.h"
#include "am7x-freq.h"
#include "am7x_gpio.h"



#define DBG_AM_MSG		1
#if  DBG_AM_MSG
#define DBG_UARTCOM_MSG(format,args...)   printk(format,##args)
#else
#define DBG_UARTCOM_MSG(format,args...)   do {} while (0)
#endif

#define AM_CHIP_ID	(1213)


#define UARTCOM_CTL		UART2_CTL
#define UARTCOM_STAT		UART2_STAT
#define UARTCOM_TXDAT		UART2_TXDAT
#define UARTCOM_RXDAT		UART2_RXDAT
#define COM_CMU_UARTCLK 	CMU_UART2CLK
#define COM_IRQ_UART		IRQ_UART2

#define UART_CTL		*(volatile INT32U*)(UART2_CTL)

#define UART_CTL_DATA	(0x0000cc03 | (1<<18))//Enable RX Irq

#define BUFSIZE  (512) 
#define Serial_Brg  38400

#define AM_NB_PORT		(2)
#define USE_UART_PORT	(1)
#define SERIAL_BRG  (38400)

#define UART_FIFO_LEN (64)
#define UART_CACHE_BUF_LEN	(UART_FIFO_LEN*4)

//#define USE_WORK_QUE_WRITE //使用任务队列去写串口

#define FUI_KEYACTION_DOWN_MASK (KEY_ACTION_DOWN<<KEY_ACTION_MASK_SHIFT)
 


#define SHOW_ALL_RX	0
#define DECODE_KEY_HERE	0



static const char * udev_name = "uart_char";
struct cdev *u_cdev;
int udev = 0;
struct class *g_u_class = NULL;




static const unsigned long cmu_uartclk[2]   = { CMU_UART1CLK, CMU_UART2CLK };


typedef struct{
	int start, end;
} Pair;

typedef struct {
	INT8U buffer[BUFSIZE];
	volatile int len;
	Pair pair; /* global point for data received */
	//struct semaphore sem;
	struct mutex sem;
	wait_queue_head_t wq;
} RingBuffer_T;

RingBuffer_T ring;



struct uart_char{
	RingBuffer_T rxRing;
	RingBuffer_T txRing;
};

struct uart_char ua_buf;


struct cache_cell{
	struct list_head list;
	INT8U buf[UART_CACHE_BUF_LEN];//FIFO LEN = 16
	int len;
};

struct ua_work_struct_t{
	struct list_head	list;
	RingBuffer_T * pRing;
	struct work_struct work_t;

	#ifdef USE_WORK_QUE_WRITE
	RingBuffer_T * tPRing;
	struct work_struct write_work;
	#endif
	
	struct workqueue_struct * wq;
};

struct ua_work_struct_t ua_work;



static void serial_putc (const INT8U c);

	
static void ring_instert_tail(register RingBuffer_T * const pRing, INT8U byte)
{

	pRing->buffer[pRing->pair.end] = byte;
	
	pRing->pair.end++;


	if(pRing->pair.end > BUFSIZE - 1){
		pRing->pair.end = 0;
	}
	pRing->len++;	

	if(pRing->len > BUFSIZE-1){
		pRing->len = BUFSIZE-1;
		
		pRing->pair.start++;
		if(pRing->pair.start > BUFSIZE - 1){
			pRing->pair.start = 0;
		}
	}
}


static int ring_pop_head(register RingBuffer_T * const pRing, INT8U *byte)
{
	int ret = 0;

	if(pRing->len > 0){
		*byte = pRing->buffer[pRing->pair.start];
		pRing->pair.start++;
		if(pRing->pair.start > BUFSIZE - 1){
			pRing->pair.start = 0;
		}
		pRing->len--;
	}else{
		ret = -1;
	}

	return ret;
	
}

static int ring_copy_all(register RingBuffer_T * const pRing, INT8U *const mem)
{
	register int i = 0, j = 0;
	
	if(pRing->len > 0){		
		for(i=pRing->pair.start; i != pRing->pair.end; i = (i+1)&(BUFSIZE-1), j++){
			mem[j] = pRing->buffer[i];
		}
		
		pRing->pair.end = pRing->pair.start = 0;
		pRing->len = 0;
	}

	return j;
	
}


static inline void ring_reset(RingBuffer_T * pRing)
{
	pRing->pair.end = pRing->pair.start = 0;
	pRing->len = 0;
}



 
static void ua_work_queue_do_read(struct work_struct *work)
{
	struct ua_work_struct_t * pua_work = container_of(work, struct ua_work_struct_t, work_t);
	RingBuffer_T *pRing = pua_work->pRing;
	int i;
	struct list_head *entry, *temp;
	struct cache_cell * cache = NULL;
	
	while(mutex_lock_interruptible(&pRing->sem)){
		printk("[%s]get mutex error!\n", __func__);
	}

	printk("[%s]\n", __func__);

	
	list_for_each_safe(entry, temp, &pua_work->list){
		cache = list_entry(entry, struct cache_cell, list);
		
		disable_irq(COM_IRQ_UART);
		list_del(entry);
		enable_irq(COM_IRQ_UART);
		
		for(i=0; i < cache->len; i++){
			ring_instert_tail(pRing, cache->buf[i]);
			printk("0x%x,  ", cache->buf[i]);
		}
		
		kfree(cache);
		
	}

	printk("[END]\n");
	
	mutex_unlock(&pRing->sem);
	
	wake_up_interruptible(&pRing->wq);
	
	return;
}


#ifdef USE_WORK_QUE_WRITE
static void ua_work_queue_do_write(struct work_struct *work)
{
	struct ua_work_struct_t * pua_work = container_of(work, struct ua_work_struct_t, work_t);
	RingBuffer_T *pRing = pua_work->tPRing;

	
	wake_up_interruptible(&pRing->wq);
	printk("[%s]\n", __func__);
	
	return;
}

#endif


//queue_work(ua_wq, &(ua_work.work_t));


static ssize_t am_uart_read(struct file *pfile, char __user *buf, size_t size, loff_t *offset)
{
	int len = 0;
	int status = 0;
	RingBuffer_T *pRing = NULL;
	static INT8U mem[BUFSIZE];
	

	pRing = &ua_buf.rxRing;

	if(pRing->len <= 0){
		do{
			wait_event_interruptible(pRing->wq, pRing->len > 0);
			//DBG_UARTCOM_MSG("wq up!len = %d\n", pRing->len);
		}while(pRing->len <= 0);
		DBG_UARTCOM_MSG("rq up!len = %d\n", pRing->len);
	}
	

	status = mutex_lock_interruptible(&pRing->sem);
	if (status)
		return status;
	
	len = ring_copy_all(pRing, mem);	

	if(len > 0)
		copy_to_user(buf, mem, len);
	
//out:
	mutex_unlock(&pRing->sem);
	
	return len;
}


static ssize_t  am_uart_write(struct file *pfile, const char __user *buf, size_t size, loff_t *offset)
{
	int i = 0;
	int status = 0;
	RingBuffer_T *pRing = NULL;
	static INT8U mem[BUFSIZE];
	INT8U byte = 0;
	
//	DBG_UARTCOM_MSG("uartcom: user space read\n!!");

	pRing = &ua_buf.txRing;

	if(BUFSIZE-1 < size){
		printk("[%s]error", __func__);
		return -ENOMEM;
	}

	#ifdef USE_WORK_QUE_WRITE
	if((BUFSIZE - pRing->len) < size){
		do{
			wait_event_interruptible(pRing->wq, (BUFSIZE - pRing->len) >= size);
			DBG_UARTCOM_MSG("wq up!W len = %d\n", pRing->len);
		}while((BUFSIZE - pRing->len) < size);
	}
	#endif


	status = mutex_lock_interruptible(&pRing->sem);
	if (status)
		return status;

	copy_from_user(mem, buf, size);

	#ifndef USE_WORK_QUE_WRITE
	for(i=0; i<size; i++){
		serial_putc(mem[i]);
	}
	#else
	for(i=0; i<size; i++){
		ring_instert_tail(pRing, mem[i]);
	}
	while(-1 != ring_pop_head(pRing, &byte)){
		//printk("0x%x, ", byte);
		serial_putc(byte);
	}
	#endif
	
	mutex_unlock(&pRing->sem);

//out:	
	return i;
}







/* uart setting functions */
INT32U act_get_cclk_div(void)
{
#if AM_CHIP_ID == 1203
	return 1;
#elif AM_CHIP_ID == 1211
	return ((act_readl(CMU_BUSCLK)&(1<<2))+1);
#elif AM_CHIP_ID == 1220 || AM_CHIP_ID == 1213 
	INT32U sclk_div =2,sclk_val=*(volatile INT32U*)(CMU_BUSCLK);

	sclk_val = (sclk_val>>28)&0x7;

	switch(sclk_val){
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
#if CONFIG_AM_CHIP_MINOR == 8268
		val >>= 1;
		val &= 0x3f;
		val *= 24;
#else

		val >>= 2;
		val &= 0x7f;
		val *= 6;
#endif
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
	//volatile INT32U *uart1_clk = (volatile INT32U*)(CMU_UART1CLK);
	volatile INT32U *uart2_clk = (volatile INT32U*)(CMU_UART2CLK);

	tmp = corePll*1000000/(16*Serial_Brg*cclk_div);
	tmp = corePll*1000000*2/(16*Serial_Brg*cclk_div);	
	tmp |= 0x10000;
	*uart2_clk = tmp;//0x1004d(24M)/0x10185(120M)/0x10138(96M)/0x100c3(60M)
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



static irqreturn_t interrupt_handler(int irq, void *dev_id) {
	INT32U status = 0;
	INT8U byte = 0;
	int i = 0;
	struct cache_cell *cache = NULL;
	
	status = act_readl(UARTCOM_STAT);

	#ifdef USE_WORK_QUE_WRITE
	if(status & 0x2){//tx irq
		if(0 == queue_work(ua_work.wq, &ua_work.write_work)){
			//printk("[%s]start write work error!\n", __func__);
		}
	}else
	#endif
	{
		if(status & (1<<4)) { // rx error
			goto irq_error;
		}
		
		cache = kmalloc(sizeof(struct cache_cell), GFP_ATOMIC);//FIFO length = 16

		if(cache == NULL){
			printk("kmalloc error![%s]\n", __func__);
			goto irq_error;
		}
		
		while(!(act_readl(UARTCOM_STAT) & (1<<5))) {
			cache->buf[i++] = byte = act_readl(UARTCOM_RXDAT) & 0xFF;

			#if SHOW_ALL_RX
			DBG_UARTCOM_MSG(" %0x, ",byte);		
			#endif	

			if(i > UART_CACHE_BUF_LEN - 1){
				break;
			}
		}

		if(likely(i > 0)){
			cache->len = i;
			list_add_tail(&cache->list, &ua_work.list);
			if(0 == queue_work(ua_work.wq, &ua_work.work_t)){//启动读任务
				//printk("[%s]start read work error!\n", __func__);
			}
		}else{
			kfree(cache);
		}
	}
	
irq_error:
	IRQ_CLEAR_PENDING(status, UARTCOM_STAT);
	return IRQ_HANDLED;
}

static void serial_putc (const INT8U c)
{
	volatile INT32U *uart_stat = (volatile INT32U*)(UARTCOM_STAT);	
	/* Wait for fifo to shift out some bytes */	
	while(*uart_stat & 0x00000040){
		DBG_UARTCOM_MSG("uartchar:serial_putc wait for fifo\n");
		msleep(4);
	}
	*(volatile INT32U*)UARTCOM_TXDAT = (INT8U)c;
}

/*
 * init components for uart communication
 */
static int init_uart(void) {
	int err = 0;
	int i = 5000;
	//volatile INT32U *uart_config = (volatile INT32U*)(UARTCOM_CTL);

	//INT32U temp;

	DBG_UARTCOM_MSG(KERN_INFO "init_uart11\n");
#if CONFIG_AM_CHIP_ID == 1213
	#if 1
	act_andl(0x3fffffff, GPIO_MFCTL3);
	act_orl(0x40000000, GPIO_MFCTL3);

	act_andl(0xfffffff8, GPIO_MFCTL4);
	act_orl(0x00000001, GPIO_MFCTL4);
	#else
	INT32U* gpio3 = (INT32U*)(GPIO_MFCTL3);
	INT32U* gpio4 = (INT32U*)(GPIO_MFCTL4);

	*gpio3 = (*gpio3 & 0x3fffffff) | 0x40000000;
	*gpio4 = (*gpio4 & 0xfffffff8) | 0x00000001;
	#endif
#elif AM_CHIP_ID==1211

	INT32U* gpio5 = (INT32U*)(GPIO_MFCTL5);
	INT32U* gpio6 = (INT32U*)(GPIO_MFCTL6);

	*gpio5 = (*gpio5 & 0x0fffffff) | 0x20000000;
	*gpio6 = (*gpio6 & 0xfffffff0) | 0x00000002;

#endif

#if 1
#if AM_CHIP_ID==1213
	serial_setbrg(act_getCorePll(), act_get_sclk_div() );
#else
	serial_setbrg(Serial_Brg);
#endif
#endif

	//serial_setbrg(SERIAL_BRG, USE_UART_PORT);

	act_writel(UART_CTL_DATA, UARTCOM_CTL);// Enable UART1 RX IRQ
	//*uart_config = 0x0000cc03 |(1<<18); 
	//*uart_config = 0x48403;

	// init uart
	act_writel(1,UARTCOM_STAT); /* clear IRQ pending bit */
	while(((act_readl(UARTCOM_STAT) & 0x20) == 0) && i--){
		act_readl(UARTCOM_RXDAT) ;
	}

	err = request_irq(COM_IRQ_UART, interrupt_handler, IRQF_DISABLED, "UartChar", NULL);
	
	if(err < 0)
		DBG_UARTCOM_MSG(KERN_WARNING "uart2 irq error\n");
	else
		DBG_UARTCOM_MSG(KERN_INFO "uart2 irq ok\n");


	#if 1 //-- Paul add for USB P1 Power.  //如果不设置fui读取rtc会出错
	//unsigned int  rtc_reg=0;
	//rtc_reg = RTC_READ(RTC_ALARM);
	RTC_WRITE(((RTC_READ(RTC_ALARM)&0xffffdfff)|0x300),RTC_ALARM);
	DBG_UARTCOM_MSG("set USB1_PW to high\n");
	#endif

	
	return 0;
}





static int am_uartchar_open(struct inode *pinode, struct file *pfile)
{
	return 0;
}

static int am_uartchar_release(struct inode *pinode, struct file *pfile)
{
	DBG_UARTCOM_MSG("am_uartcom_release\n");
	return 0;
}


static const struct file_operations uart_fops =
{
	.owner   = THIS_MODULE,
	.read = am_uart_read,
	.write = am_uart_write,
	.open = am_uartchar_open,
	.release = am_uartchar_release,
};

static int  __init uart_char_init(void)
{
	int result;
	int major = 0;
	int minor = 0;

	DBG_UARTCOM_MSG(KERN_INFO "uart char init...\n");

	if(major)
	{
		udev = MKDEV(major, minor);
		result = register_chrdev_region(udev, 1, udev_name);
	}
	else
	{
		result = alloc_chrdev_region(&udev, major, 1, udev_name);
		major = MAJOR(udev);
	}
	printk(KERN_INFO"[%s]major =%d\n", __func__, major);

	u_cdev = cdev_alloc();
	cdev_init(u_cdev,&uart_fops);
	u_cdev->owner=THIS_MODULE;
	result = cdev_add(u_cdev, udev, 1);
	if(result < 0)
	{
		printk("can't add [%s]dev", __func__);
		unregister_chrdev_region(udev, 1);
		cdev_del(u_cdev);
		return result;
	}

	g_u_class = class_create(THIS_MODULE, "uart_char_class");
	if(IS_ERR(g_u_class)) {
		unregister_chrdev_region(udev, 1);	
		cdev_del(u_cdev);
		printk("Err: failed in creating class.\n");
		return -1; 
	}

	device_create(g_u_class, NULL, udev, NULL, udev_name);//创建设备文件


	//工作队列初始化
	ua_work.wq = create_workqueue("uartchar");
	ua_work.pRing = &ua_buf.rxRing;
	INIT_WORK(&(ua_work.work_t), ua_work_queue_do_read);

	INIT_LIST_HEAD(&ua_work.list);
	
	#ifdef USE_WORK_QUE_WRITE
	ua_work.tPRing = &ua_buf.txRing;
	INIT_WORK(&(ua_work.write_work), ua_work_queue_do_write);
	#endif
	
	//ringbuffer init
	init_waitqueue_head(&ua_buf.rxRing.wq);
	init_waitqueue_head(&ua_buf.txRing.wq);
	mutex_init(&ua_buf.rxRing.sem);
	mutex_init(&ua_buf.txRing.sem);

	init_uart();//初始化硬件


	return 0;
}

static void __exit uart_char_exit(void)
{

	free_irq(COM_IRQ_UART, NULL);
	
	if(u_cdev)
		cdev_del(u_cdev);
	
	if(g_u_class){
		device_destroy(g_u_class, udev);    //delete device node under /dev//必须先删除设备，再删除class类
		class_destroy(g_u_class);                 //delete class created by us
	}
	
	if(udev)
		unregister_chrdev_region(udev, 1);	

	if(ua_work.wq)
		destroy_workqueue(ua_work.wq);


	
	// TODO: release irq
	// TODO: kill dispatch_tasklet
}


module_init(uart_char_init);
module_exit(uart_char_exit);

MODULE_AUTHOR("Charles");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Using uart to communicate");



