#include <linux/init.h>
#include<linux/module.h>
#include <linux/kernel.h>   
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/timer.h>
#include <sys_msg.h>
#include "actions_regs.h"
#include "actions_io.h"
#include "sys_cfg.h"

struct input_dev *am_touchpad_dev;
struct timer_list touchpad_timer;
struct timer_list checkerr_timer;
static INT32U gGPIO_Multi2;
static int send_message_state;
static char DownLeftFlag;
static char DownRightFlag;
static int touchpadx_data;
static int touchpady_data;

#define errdata_maxtime     		2	//8ms以上
#define TOUCHPAD_CLK_IO   		28
#define TOUCHPAD_DATA_IO 		59
#define Receive_cmd_data      	0x10
#define Receive_normal_data		0x20
#define Send_cmd				0x30
#define Other_state				0x40
#define Each_Init_Maxtimes          10//每次初始化允许再复位的次数
#define total_init_times                 20//开机初始化的总次数
#define close_interrupt_times	      10//连续N次关中断
#define defaultstate		0x00
#define LeftBtnDown		0x01
#define LeftBtnUp			0x02
#define RightBtnDown		0x03
#define RightBtnUp		0x04
#define move_x_y		0x09
#define other_interrupt         0xff
#define interrupt_touchpad  0xf0
#define interrupt_mouse  	   0xf1
#define TOUCHPAD_MOVE_FREQUENCE		1
static int xpos_buf[TOUCHPAD_MOVE_FREQUENCE];
static int ypos_buf[TOUCHPAD_MOVE_FREQUENCE];

#define BUFNUMMAX		3
static int TouchpadBuffer[BUFNUMMAX];//存放mouse的数据

const int BitTab[]={0x0001,0x0002,0x0004,0x0008,0x0010,0x0020,0x0040,0x0080,
                     0x0100,0x0200,0x0400,0x0800,0x1000,0x2000};

static struct interrupt{
	unsigned char ext_irq_source;
}interrupt_state;

static struct readboy_touchpad {
	int send_data; 	//send data
	int receive_data;//receive data
	int init_touchpad_index;//init touchpad 顺序	
	int each_init_maxtimes;//每次init touchpad 可重复init 的次数
	int first_init_touchpad;//总init 次数
	unsigned char state;	//interrupt state
	unsigned char receive_end;		//接收一字节完毕标示位
	unsigned char init_touchpad_flag;	//定时器init touchpad
	unsigned char send_bit_index;
	unsigned char receive_bit_index;
	unsigned char receive_byte_index;
	unsigned char receive_byte_index2;
	unsigned char interrupt_switch;		// 1--允许接收数据0--不接收数据
}touchpad;
/*************************
*************************/
#define act_writel(val,reg)  (*(volatile unsigned int *)(reg) = (val))                
#define act_readl(port)  (*(volatile unsigned int *)(port))  
void am_set_gpio(loff_t num,INT8U value);
INT8U am_get_gpio(loff_t num);
static int init_touchpad_io(void);
static void clear_touchpad_Pending(void);
static int  enable_touchpad_IRQ(void );
static int  disable_touchpad_IRQ(void );
static int check_receive_data(int data);
static int  Send_cmd_touchpad( int data);
static void init_touchpad(void);
static int receive_cmd_data(void);
static int receive_normal_data(void); 
static void timer_init_touchpad(void);
static int init_touchpad_timer(void);
static void errckeck_timer_isr(void);
static int init_errcheck_timer(void);
static void send_message(void);
static irqreturn_t touchpad_interrupt(void);

/*************************
*@当中断来临后
*@check which interrupt 
**************************/
static int  check_whick_interrupt(void)
{	
	interrupt_state.ext_irq_source=other_interrupt;
	if(act_readl(INTC_EXTCTL01) & 0x00010000)
	{
		interrupt_state.ext_irq_source=interrupt_mouse;//External Interrupt 1
		return 1;
	}
	if(act_readl(INTC_EXTCTL01) & 0x01)
	{
		interrupt_state.ext_irq_source=interrupt_touchpad;
		return 0;//External Interrupt 0
	}
	return -1;//err interrupt 
}
/*****************************
*@init touchpad io
*@GPIO-59 for touchpad data (IO)
*@GPIO-28 for touchpad clk  (IRQ 0)
*****************************/
static int  init_touchpad_io(void)
{
	int mm;
	int dev_id = 0;
	gGPIO_Multi2=act_readl(GPIO_MFCTL2);	
	act_writel((gGPIO_Multi2&(~(15<<8)))|(3<<8),GPIO_MFCTL2); //set IO 59 for Touchpad DAT (IO)
	gGPIO_Multi2=act_readl(GPIO_MFCTL2);	
	act_writel((gGPIO_Multi2&(~(7<<16)))|(2<<16),GPIO_MFCTL2); //set IO 28 for Touchpad CLK (IO)
	am_set_gpio(TOUCHPAD_CLK_IO,1);
	am_set_gpio(TOUCHPAD_DATA_IO,1);
	
//	mm = request_irq(IRQ_EXT, (irq_handler_t)touchpad_interrupt, 0, "touchpad", NULL);
	mm = request_irq(IRQ_EXT, (irq_handler_t)touchpad_interrupt, IRQF_SHARED, "touchpad", &dev_id);
	if(mm< 0)
	{
		printk(">>>>>request ext_irq0 error<<<<<<\n");
		return mm;
	}
	else
		printk(">>>>>request ext_irq0 ok<<<<<<\n");
	return 0;
}
/*****************************
*@clean touchpad interrupt flag bit
*@
*****************************/
static void clear_touchpad_Pending(void)
{
	if(act_readl(INTC_EXTCTL01) & 0x01)
	{
		act_writel(act_readl(INTC_EXTCTL01)|(1<<0), INTC_EXTCTL01);
	}	
}
/*****************************
*@enable touchpad interrupt
*@
*****************************/
static int  enable_touchpad_IRQ(void )
{
	gGPIO_Multi2=act_readl(GPIO_MFCTL2);	
	act_writel((gGPIO_Multi2&(~(7<<16)))|(1<<16),GPIO_MFCTL2); //set GPIO28 for touchpad CLK (EXT_INT0)	
	act_writel(((act_readl(INTC_EXTCTL01)&(~(7<<8)))|(7<<8)), INTC_EXTCTL01);//Falling edge-triggered and enable interrupt
	clear_touchpad_Pending();
	return 0;
}
/*****************************
*@disalbe touchpad interrupt
*@
*****************************/
static int  disable_touchpad_IRQ(void )
{
	act_writel(act_readl(INTC_EXTCTL01)&(~(7<<8)), INTC_EXTCTL01);//close interrupt
	gGPIO_Multi2=act_readl(GPIO_MFCTL2);	
	act_writel((gGPIO_Multi2&(~(7<<16)))|(2<<16),GPIO_MFCTL2); 	//set IO 28 for Touchpad CLK (IO)
	return 0;
}
/*****************************
*@check receive data err or right
*@
*****************************/
static int check_receive_data(int data)
{
	int temp ,t1,i,tt;
	temp = data ;
	if(data < 0x400 || data >= 0x800) return 1;
	if((data%2)==1)	return 1;
	t1=1;
	for(i=0;i < 11;i++)	
	{
		tt = temp&BitTab[i];
		if(tt)
		{	t1^=1;	}
	}
	if(!t1) return 1;
	return 0;
}
/*****************************
*@检查组合成的三个字节
*@
*****************************/
static int check_3byte(int *PacketReg)
{	
	int temph = PacketReg[0];
	if((!(temph&0x08))|| ((temph&0x40)) || ((temph&0x80)))//Allways1,YOverFlow ,XOverFlow
	{													
		return 1;
	}
	return 0;	
}
/*****************************
*@init touchpad 
*@
*****************************/
static void init_touchpad(void)
{
	touchpad.init_touchpad_index=0;
	Send_cmd_touchpad(0xff);
}
/*****************************
*@send cmd to touchpad 
*@
*****************************/
static int  Send_cmd_touchpad( int data)
{
	int temp=0;
	int t1,i,tt;
//	disable_touchpad_IRQ();
	temp=data&0xff;
	t1=1;
	for(i=0;i<8;i++)//校验位
	{
		tt = temp&BitTab[i];
		if(tt)
		{	t1^=1;	}
	}	
	t1<<=8;
	temp=data+t1;//10位
	touchpad.state=Send_cmd;
	touchpad.send_bit_index=0;
	touchpad.send_data=temp;
	am_set_gpio(TOUCHPAD_CLK_IO,0);
	udelay(120);
	am_set_gpio(TOUCHPAD_CLK_IO,1);
//	enable_touchpad_IRQ();
	udelay(10);
	am_set_gpio(TOUCHPAD_DATA_IO,0);
	return 0;
}
/*****************************
*@receive touchpad data after send cmd
*@
*****************************/
static int receive_cmd_data(void)
{
	int Bit;
	Bit = am_get_gpio(TOUCHPAD_DATA_IO);
	if(Bit)
	{
		touchpad.receive_data |= BitTab[touchpad.receive_bit_index];
		touchpad.receive_bit_index += 1;
	}
	else
	{
		touchpad.receive_data &= ~BitTab[touchpad.receive_bit_index];
		touchpad.receive_bit_index += 1;
	}
	
	if( touchpad.receive_bit_index >= 11 )
	{
		int err =0;
		touchpad.receive_bit_index = 0 ;
		touchpad.receive_end=1;
		err=check_receive_data(touchpad.receive_data);
		if(err==0)
		{
			touchpad.receive_data=(touchpad.receive_data>>1)&0xff;
		//      printk("d=%x\n",touchpad.receive_data);
		}
		else
		{
		//	printk("dr\n");
		}	
	}
	return 0;
}
/*****************************
*@receive normal data after init touchpad sucess
*@
*****************************/
static int receive_normal_data(void)
{
	int Bit;
	int err =0;
	Bit = am_get_gpio(TOUCHPAD_DATA_IO);
	if(Bit)
	{
		touchpad.receive_data |= BitTab[touchpad.receive_bit_index];
		touchpad.receive_bit_index += 1;
	}
	else
	{
		touchpad.receive_data &= ~BitTab[touchpad.receive_bit_index];
		touchpad.receive_bit_index += 1;
	}
	
	if( touchpad.receive_bit_index >= 11 )
	{	
		touchpad.receive_bit_index = 0 ;
		touchpad.receive_byte_index2+=1;//记录实际接收的字节数
		err=check_receive_data(touchpad.receive_data);
		if(err==0)
		{	
			touchpad.receive_data=(touchpad.receive_data>>1)&0xff;
// 			printk("d=%x\n",touchpad.receive_data);
			if( touchpad.receive_byte_index<BUFNUMMAX )
			{
				TouchpadBuffer[touchpad.receive_byte_index] = touchpad.receive_data ;//存数据
			}
			touchpad.receive_byte_index+=1;//记录正确的字节	
		}
		else
		{
	//		printk("bit\n");
			touchpad.interrupt_switch=0;//close interrupt
			mod_timer(&checkerr_timer,jiffies+errdata_maxtime);	
		}	
	}
	return 0;
}
/*****************************
*@touchpad timer enter 
*@
*****************************/
static void timer_init_touchpad(void)
{
	static int times=0;
	static int interrupt_switch_index1=0;
	static int interrupt_switch_index2=0;
	
	times++;
	interrupt_switch_index1++;
	#if 1
	if(touchpad.first_init_touchpad==1)
	{
		printk(">>>first init touchpad<<<\n");
		init_touchpad();
		touchpad.first_init_touchpad++;
	}
	#endif
	#if 1
	if(1==touchpad.init_touchpad_flag)//定时器init touchpad
	{
		touchpad.init_touchpad_flag=0;	
		memset(TouchpadBuffer,0x00,sizeof(int)*BUFNUMMAX);
		if(touchpad.each_init_maxtimes<Each_Init_Maxtimes)
		{
//			printk(">>>init touchpad<<<\n");
			init_touchpad();
		}
	}
	#endif
	#if 1
	if(times==10)
	{
		if(touchpad.first_init_touchpad==total_init_times)
		{
			times++;	
			touchpad.state=Receive_normal_data;		
			touchpad.send_bit_index=0;
			touchpad.send_data=0;
			touchpad.receive_bit_index=0;
			touchpad.receive_byte_index=0;
			touchpad.receive_byte_index2=0;
			touchpad.receive_data=0;
			touchpad.init_touchpad_index =0;
			touchpad.receive_end=0;
			touchpad.each_init_maxtimes=0;			
			touchpad.init_touchpad_flag=0;
			touchpad.first_init_touchpad=total_init_times+1;
			printk(">>>no touchpad<<<\n");			
		}
		if((touchpad.first_init_touchpad<total_init_times)&(touchpad.init_touchpad_index !=0xff))
		{
			times=0;	
			touchpad.each_init_maxtimes=0;
			touchpad.init_touchpad_flag=1;
			touchpad.first_init_touchpad++;
//			printk(">>>init touchpad times=%d <<<\n",touchpad.first_init_touchpad);
		}
		if((touchpad.first_init_touchpad<total_init_times)&(touchpad.init_touchpad_index ==0xff))
		{
			touchpad.first_init_touchpad=total_init_times+1;
		}
	}
	#endif 
	#if 1
	if(touchpad.interrupt_switch==0)
	{	
		interrupt_switch_index2++;
		if(interrupt_switch_index2==1)//同步
		{
			interrupt_switch_index1=1;
		}
		if(interrupt_switch_index1>=close_interrupt_times)
		{
			if((interrupt_switch_index1==close_interrupt_times)&(interrupt_switch_index2==close_interrupt_times))//连续N次状态是other state
			{
				touchpad.state=Receive_normal_data;		
				touchpad.receive_bit_index=0;
				touchpad.receive_byte_index=0;
				touchpad.receive_byte_index2=0;
				touchpad.receive_data=0;
				touchpad.interrupt_switch=1;//open interrupt
				memset(TouchpadBuffer,0x00,sizeof(int)*BUFNUMMAX);
//				printk(">>>open touchpad interrupt <<<\n");
			}
			interrupt_switch_index2=0;
		}
	}	
	#endif 
	mod_timer(&touchpad_timer,jiffies + 25);
}
/*****************************
*@init a timer for init touchpad 
*@check touchpad work status
*@deal touchpad err status
*****************************/
static int init_touchpad_timer(void)
{
	init_timer(&touchpad_timer);			
	touchpad_timer.function = (void*)timer_init_touchpad;
	touchpad_timer.expires = jiffies +100;
	add_timer(&touchpad_timer);	
	return 0;
}
/*****************************
*@deal err data function
*@
*****************************/
static void errckeck_timer_isr(void)
{
	if(touchpad.interrupt_switch==0)//close interrupt
	{
		touchpad.interrupt_switch=1;//open interrupt
		touchpad.receive_bit_index=0;
		touchpad.receive_byte_index=0;
		touchpad.receive_byte_index2=0;
		touchpad.receive_data=0;
		memset(TouchpadBuffer,0x00,sizeof(int)*BUFNUMMAX);	
//		printk(">>open interrupt<<\n");
	}
}
/*****************************
*@
*@
*****************************/
static int init_errcheck_timer(void)
{
	init_timer(&checkerr_timer);			
	checkerr_timer.function = (void*)errckeck_timer_isr;
	checkerr_timer.expires = jiffies +0xffff;
	add_timer(&checkerr_timer);	
	return 0;
}
/*****************************
*@ send message to input subsystem
*@
*****************************/
static void send_message(void)
{
	int tdata;
	static int movefreq = 0;
	if(TouchpadBuffer[0]&0x10)//x轴1=负0=正
	{
		tdata = TouchpadBuffer[1]&0xff;
		tdata += 0xffffff00;
	}
	else
	{
		tdata = TouchpadBuffer[1]&0xff;
	}
	touchpadx_data=tdata;

	if(TouchpadBuffer[0]&0x20)//y轴1=负0=正
	{
		tdata = TouchpadBuffer[2]&0xff;
		tdata += 0xffffff00;			
	}
	else
	{
		tdata = TouchpadBuffer[2]&0xff;		
	}			
	touchpady_data=tdata;
	
	send_message_state=defaultstate;
	
	if(TouchpadBuffer[0]&0x01)// press left
	{
		send_message_state=LeftBtnDown;
		DownLeftFlag=1;
	}
	else
	{
		if(DownLeftFlag==1)
		{
			send_message_state=LeftBtnUp;
			DownLeftFlag=0;
		}
	}
	
	 if(TouchpadBuffer[0]&0x02)//press right
	 {
		send_message_state=RightBtnDown;
		DownRightFlag=1;
	 }
	else
	{
		if(DownRightFlag==1)
		{
			send_message_state=RightBtnUp;
			DownRightFlag=0;
		}
	}
	 
	if(send_message_state==defaultstate)
		send_message_state=move_x_y;


	switch(send_message_state)
	{
		case LeftBtnDown:
			input_report_rel(am_touchpad_dev,REL_X, touchpadx_data);
			input_report_rel(am_touchpad_dev,REL_Y,touchpady_data);
			input_report_key(am_touchpad_dev,BTN_LEFT, 0x01);//0x01按下(不为0则可以)
			input_sync(am_touchpad_dev);
			break;
		case LeftBtnUp:	
			input_report_rel(am_touchpad_dev,REL_X, touchpadx_data);
			input_report_rel(am_touchpad_dev,REL_Y,touchpady_data);
			input_report_key(am_touchpad_dev,BTN_LEFT, 0x00);//0x00抬起
			input_sync(am_touchpad_dev);
			break;	
		case RightBtnDown:
			input_report_rel(am_touchpad_dev,REL_X, touchpadx_data);
			input_report_rel(am_touchpad_dev,REL_Y,touchpady_data);
			input_report_key(am_touchpad_dev,BTN_RIGHT, 0x02);
			input_sync(am_touchpad_dev);	
			break;
		case RightBtnUp:
			input_report_rel(am_touchpad_dev,REL_X, touchpadx_data);
			input_report_rel(am_touchpad_dev,REL_Y,touchpady_data);
			input_report_key(am_touchpad_dev,BTN_RIGHT, 0x00);
			input_sync(am_touchpad_dev);	
			break;
		case move_x_y:
			xpos_buf[movefreq] = touchpadx_data;
			ypos_buf[movefreq] = touchpady_data;
			movefreq++;
			if(movefreq==TOUCHPAD_MOVE_FREQUENCE)
			{
				int cur_x=0,cur_y=0;
				int i;
				for(i=0;i<TOUCHPAD_MOVE_FREQUENCE;i++)
				{
					cur_x += xpos_buf[i];
					cur_y += ypos_buf[i];
				}
				cur_x = cur_x/TOUCHPAD_MOVE_FREQUENCE;
				cur_y = cur_y/TOUCHPAD_MOVE_FREQUENCE;
				movefreq = 0;
//				printk("x=%d,y=%d\n",cur_x,cur_y);
				input_report_rel(am_touchpad_dev,REL_X, cur_x);
				input_report_rel(am_touchpad_dev,REL_Y,cur_y);
				input_sync(am_touchpad_dev);	
			}
			break;	
	}
}
/*****************************
*@touchpad  interrupt enter
*@
*****************************/
static irqreturn_t touchpad_interrupt(void)
{	
 	check_whick_interrupt();
	if(interrupt_state.ext_irq_source!=interrupt_touchpad)
		return IRQ_NONE;
	if(touchpad.interrupt_switch==0)
		goto errdata;
	switch(touchpad.state)
	{
		case Send_cmd:
		{		
			switch(touchpad.send_bit_index)
			{
			case 0://bit 0
			case 1://bit 1
			case 2://bit 2
			case 3://bit 3
			case 4://bit 4
			case 5://bit 5
			case 6://bit 6
			case 7://bit 7
			case 8://校验位
				if(touchpad.send_data&0x1)//write data
				{
					am_set_gpio(TOUCHPAD_DATA_IO,1);
				}
				else
				{
					am_set_gpio(TOUCHPAD_DATA_IO,0);	
				}
				touchpad.send_bit_index+=1;
				touchpad.send_data =touchpad.send_data>>1;
				break;
			case 9:
				{
					am_set_gpio(TOUCHPAD_DATA_IO,1);
					touchpad.send_bit_index++;
					am_get_gpio(TOUCHPAD_DATA_IO);
					break;//停止位
				}
			case 10://ACK
				{
					if(0==am_get_gpio(TOUCHPAD_DATA_IO))	
					{
						touchpad.receive_end=0;
						touchpad.receive_bit_index=0;
						touchpad.receive_byte_index=0;
						touchpad.receive_byte_index2=0;
						touchpad.receive_data=0;
						touchpad.state=Receive_cmd_data;
					}
					else
					{
	//					printk("send fail\n");
						if(touchpad.init_touchpad_flag==0)
						{
							touchpad.init_touchpad_flag=1;//定时器init touchpad
							touchpad.each_init_maxtimes=0;
							touchpad.state=Other_state;
						}
					}
					break;	
				}						
			}	
		break;
		}
		case Receive_cmd_data:
		{
			receive_cmd_data();
			if(touchpad.receive_end==1)//收1byet	
			{
				touchpad.receive_end=0;
				switch(touchpad.receive_data)
				{
					case 0xaa:
					{
						if(touchpad.init_touchpad_index==0x01)
						{
							touchpad.init_touchpad_index=0x02;
						}
						else
						{
							if(touchpad.init_touchpad_flag==0)
							{
								touchpad.init_touchpad_flag=1;//定时器init touchpad
								touchpad.each_init_maxtimes++;
							}
						}
						break;
					}
					case 0x00:
					{
						if(touchpad.init_touchpad_index==0x02)
						{
							Send_cmd_touchpad(0xf4);//回复oxfa
							touchpad.init_touchpad_index=0x03;
						}
						else
						{
							if(touchpad.init_touchpad_flag==0)
							{
								touchpad.init_touchpad_flag=1;//定时器init touchpad
								touchpad.each_init_maxtimes++;
							}
						}
						break;
					}
					case 0xfa://发送命令成功
					{
						switch(touchpad.init_touchpad_index)
						{
							case 0x0:
							{
								touchpad.init_touchpad_index=0x01;
								break;
							}
							case 0x03:
							{
								Send_cmd_touchpad(0xf3);//设置采样速率回复oxfa
								touchpad.init_touchpad_index=0x04;		
								break;
							}
							case 0x04:
							{
								Send_cmd_touchpad(60);//设置采样率的值 可改为(10 20 40 60 80 100 200)
								touchpad.init_touchpad_index=0x05;		
								break;
							}							
							case 0x05:
							{
								touchpad.init_touchpad_index=0xff;
								touchpad.receive_bit_index=0;
								touchpad.receive_byte_index=0;
								touchpad.receive_byte_index2=0;
								touchpad.receive_data=0;
								touchpad.state=Receive_normal_data;
								printk(">>>init touchpad ok<<<\n");
								break;
							}
							default:
							{
								if(touchpad.init_touchpad_flag==0)
								{
									touchpad.init_touchpad_flag=1;//定时器init touchpad
									touchpad.each_init_maxtimes++;
								}
								break;
							}
						}
						break;
					}
					default:
					{
						if(touchpad.init_touchpad_flag==0)
						{
							touchpad.init_touchpad_flag=1;//定时器init touchpad
							touchpad.each_init_maxtimes++;
						}
						break;
					}
				}
			}
		break;
		}
		case Receive_normal_data:
		{
			receive_normal_data();
			if( touchpad.receive_byte_index2>=3 )//满三个byte
			{

				if(touchpad.receive_byte_index2==touchpad.receive_byte_index)//中间无错byte
				{
					int erro; 
					erro = check_3byte((int *)TouchpadBuffer);//检查三个字节
					if(erro)//错数据
					{
						touchpad.interrupt_switch=0;//close interrupt
						mod_timer(&checkerr_timer,jiffies+errdata_maxtime);	//收四个字节所用时间
	//					printk("byte err\n");
					}
					else
					{
						send_message();	
					}	
				}
				touchpad.receive_bit_index=0;
				touchpad.receive_byte_index=0;
				touchpad.receive_byte_index2=0;
				touchpad.receive_data=0;
				memset(TouchpadBuffer,0x00,sizeof(int)*BUFNUMMAX);
			}
			break;
		}
		case Other_state:
		{
			break;
		}
	}
errdata:
	clear_touchpad_Pending();
	return IRQ_HANDLED;
}
/*****************************
*@
*@
*****************************/
static int __init touchpad_init(void)
{
	int error;
	printk(KERN_ERR "\n\n*********readboy_touchpad_init************\n\n");
	am_touchpad_dev = input_allocate_device();
	if (!am_touchpad_dev)
	{
		printk(KERN_ERR "input_allocate_device() failed for touchpad 1\n");
		return -ENOMEM;
	}
	#if 1
	am_touchpad_dev->name = "PS/2 touchpad";
	am_touchpad_dev->phys = "AM/input0";
	
	am_touchpad_dev->id.bustype = BUS_HOST;
	am_touchpad_dev->id.vendor = 0x0001;
	am_touchpad_dev->id.product = 0x0001;
	am_touchpad_dev->id.version = 0x0100;

	am_touchpad_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL);
	am_touchpad_dev->relbit[0] = BIT_MASK(REL_X) | BIT_MASK(REL_Y) | BIT_MASK(REL_WHEEL) ;
	am_touchpad_dev->keybit[BIT_WORD(BTN_LEFT)] = BIT_MASK(BTN_LEFT) |
		BIT_MASK(BTN_MIDDLE) | BIT_MASK(BTN_RIGHT);
	#endif
	error = input_register_device(am_touchpad_dev);
	if (error)
	{
		printk(KERN_ERR "input_allocate_device() failed for touchpad 2\n");
		input_free_device(am_touchpad_dev);
		return error;
	}
	init_touchpad_io();
	touchpad.first_init_touchpad=1;	//首次初始化touchpad
	touchpad.interrupt_switch=1;	//interrupt open
	touchpad.state=Other_state;
	enable_touchpad_IRQ();
	init_touchpad_timer();
	init_errcheck_timer();
	return 0;
}
/*****************************
*@
*@
*****************************/
static void __exit touchpad_exit(void)
{	
	del_timer(&touchpad_timer);
	del_timer(&checkerr_timer);
	free_irq(IRQ_EXT, touchpad_interrupt);
	input_unregister_device(am_touchpad_dev);	
}
module_init(touchpad_init);
module_exit(touchpad_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("luozhenghai");
MODULE_DESCRIPTION("Readboy touchpad driver ");
