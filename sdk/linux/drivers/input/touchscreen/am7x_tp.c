/*
 * Driver for Kinds of Touch panel AM_CHIP_X capable of generating interrupts.
 *
 * Copyright 2010 Actions-Micro
 * 
 * Author: Kewen
 * Created date:2010-08-17
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/timer.h>
#include "actions_regs.h"
#include "actions_io.h"
#include "am7x_board.h"
/*
#define  DBG_AM_TP
#ifdef  DBG_AM_TP
#define DBG_MSG(format,args...)   printk(format, ##args)
#else
#define DBG_MSG(format,args...)   do {} while (0)
#endif
*/
#define DATA_OFFSET		0
#define ERROR_THRESLHOLD_NUM	50
#define open_debug 0

#define TOUCH_PANEL_CONFIG	"/am7x/case/data/tpconfig.bin"

#define act_writel(val,reg)  (*(volatile unsigned int *)(reg) = (val))                
#define act_readl(port)  (*(volatile unsigned int *)(port))      

struct touch_dev {
	struct	input_dev*	dev;
	int	xp;
	int	yp;
	int	pressed;
	int	count;
	int	shift;
};
struct touch_config{
	unsigned short x_y_switch;
	unsigned short x_mirror;
	unsigned short y_mirror;
};
#define  CHECK_TOTAL_NUM	7
struct touch_pos{
	int xpos;
	int ypos;
};
static struct touch_pos last_pos[CHECK_TOTAL_NUM];
static struct touch_pos cur_pos[CHECK_TOTAL_NUM];
static struct touch_pos middle_pos[CHECK_TOTAL_NUM];

static int lcd_width,lcd_height;

static struct touch_dev	am7x_tp;
static struct touch_config tp_conf;
static struct timer_list	touch_timer;
static struct timer_list	check_timer;

static int filterflg=0;
static int rcecivedateflg=0;

int filter_large_data(int num)
{
	int i,j=0;
	int xbig,xsmall,ybig,ysmall;
	xsmall = xbig = last_pos[0].xpos;
	ybig = ysmall = last_pos[0].ypos;
	
	for(i=0;i<num;i++)
	{
		if(xbig<=last_pos[i].xpos)
			xbig = last_pos[i].xpos;
		if(xsmall>=last_pos[i].xpos)
			xsmall = last_pos[i].xpos;
		if(ybig<=last_pos[i].ypos)
			ybig = last_pos[i].ypos;
		if(ysmall>=last_pos[i].ypos)
			ysmall = last_pos[i].ypos;
	}

	for(i=0;i<num;i++)
	{
		if((xbig!=last_pos[i].xpos)&&(xsmall!=last_pos[i].xpos)&&(ybig!=last_pos[i].ypos)&&(ysmall!=last_pos[i].ypos))
		{
			cur_pos[j].xpos= last_pos[i].xpos;
			cur_pos[j].ypos= last_pos[i].ypos;
			j++;
		}
	}
	
	return j;
}
int checkdata_correct_pos (int num)
{
	int i=0,j=0,k=0,error_x=0,error_y=0;
	int tmp_x,tmp_y;
	int data_num=num;
	memset(middle_pos,0x00,sizeof(struct touch_pos)*CHECK_TOTAL_NUM);
	memset(cur_pos,0x00,sizeof(struct touch_pos)*CHECK_TOTAL_NUM);

	//y坐标数据从小到大排序
	for(i=1;i<data_num;i++)
	{
		for(j=0;j<data_num-i;j++)
		{
			if(last_pos[j].ypos>last_pos[j+1].ypos)
			{
				tmp_x = last_pos[j].xpos;
				tmp_y = last_pos[j].ypos;
				last_pos[j].xpos = last_pos[j+1].xpos;
				last_pos[j].ypos = last_pos[j+1].ypos;
				last_pos[j+1].xpos = tmp_x;
				last_pos[j+1].ypos = tmp_y;
			}
		}
	}
	#if open_debug
	for(i=0;i<data_num;i++)
		printk("sort y x=%d,y=%d\n",last_pos[i].xpos,last_pos[i].ypos);
	#endif
	//去除y最大最小值
	for(i=0;i<data_num-2;i++)
	{
		last_pos[i].xpos = last_pos[i+1].xpos;
		last_pos[i].ypos = last_pos[i+1].ypos;
	}
	data_num=data_num-2;
	#if open_debug
	for(i=0;i<data_num;i++)
		printk("remove y small and big data x=%d,y=%d\n",last_pos[i].xpos,last_pos[i].ypos);
	printk("data_num=%d,k=%d\n",data_num,k);
	#endif

	//X坐标数据从小到大排序
	for(i=1;i<data_num;i++)
	{
		for(j=0;j<data_num-i;j++)
		{
			if(last_pos[j].xpos>last_pos[j+1].xpos)
			{
				tmp_x = last_pos[j].xpos;
				tmp_y = last_pos[j].ypos;
				last_pos[j].xpos = last_pos[j+1].xpos;
				last_pos[j].ypos = last_pos[j+1].ypos;
				last_pos[j+1].xpos = tmp_x;
				last_pos[j+1].ypos = tmp_y;
			}
		}
	}
	#if open_debug
	for(i=0;i<data_num;i++)
		printk("sort x x=%d,y=%d\n",last_pos[i].xpos,last_pos[i].ypos);
	#endif
	//去除x最大最小值
	for(i=0;i<data_num-2;i++)
	{
		last_pos[i].xpos = last_pos[i+1].xpos;
		last_pos[i].ypos = last_pos[i+1].ypos;
	}
	data_num=data_num-2;
	k=0;
	#if open_debug
	for(i=0;i<data_num;i++)
		printk("remove x small and big data x=%d,y=%d\n",last_pos[i].xpos,last_pos[i].ypos);
	printk("data_num=%d,k=%d\n",data_num,k);
	#endif

	//检查x是否在允许错误范围内
	for(i=0;i<data_num-1;i++)
	{
		error_x = last_pos[i+1].xpos - last_pos[i].xpos;
		if(error_x<ERROR_THRESLHOLD_NUM)//在误差范围内
		{
			if(k==0)//存第一个数据
			{
				middle_pos[k].xpos = last_pos[i].xpos;
				middle_pos[k].ypos = last_pos[i].ypos;	
				k++;
			}
			middle_pos[k].xpos = last_pos[i+1].xpos;
			middle_pos[k].ypos = last_pos[i+1].ypos;
			k++;
		}
		else
		{
			printk("err x=%d\n",error_x);	
		}
	}
	data_num=k;
	k=0;
	#if open_debug
	printk("data_num 2=%d\n",data_num);
	for(i=0;i<data_num;i++)
		printk("check x data x=%d,y=%d\n",middle_pos[i].xpos,middle_pos[i].ypos);
	#endif
	if(data_num<=1)
		goto nodata1;
	//y坐标数据从小到大排序
	for(i=1;i<data_num;i++)
	{
		for(j=0;j<data_num-i;j++)
		{
			if(middle_pos[j].ypos>middle_pos[j+1].ypos)
			{
				tmp_x = middle_pos[j].xpos;
				tmp_y = middle_pos[j].ypos;
				middle_pos[j].xpos = middle_pos[j+1].xpos;
				middle_pos[j].ypos = middle_pos[j+1].ypos;
				middle_pos[j+1].xpos = tmp_x;
				middle_pos[j+1].ypos = tmp_y;
			}
		}
	}
	#if open_debug
	for(i=0;i<data_num;i++)
		printk("sort y-2 x=%d,y=%d\n",middle_pos[i].xpos,middle_pos[i].ypos);
	#endif
	//检查y是否在允许错误范围内
	for(i=0;i<data_num-1;i++)
	{	
		error_y = middle_pos[i+1].ypos - middle_pos[i].ypos;
		if(error_y<ERROR_THRESLHOLD_NUM)//在误差范围内
		{
			if(k==0)//存第一个数据
			{
				last_pos[k].xpos = middle_pos[i].xpos;
				last_pos[k].ypos = middle_pos[i].ypos;	
				k++;
			}
			last_pos[k].xpos = middle_pos[i+1].xpos;
			last_pos[k].ypos = middle_pos[i+1].ypos;	
			k++;
		}
		else
		{		
			printk("err y=%d\n",error_y);	
		}
	}
	data_num =k;
	#if open_debug
	for(i=0;i<data_num;i++)
		printk("check y x=%d,y=%d\n",last_pos[i].xpos,last_pos[i].ypos);
	#endif
	if(data_num<=1)
		goto nodata2;
	//存放到cur_pos[]
	for(i=0;i<data_num;i++)
	{
		cur_pos[i].xpos= last_pos[i].xpos;
		cur_pos[i].ypos= last_pos[i].ypos;
	}
	return data_num;
nodata1:
	printk("x err\n");//取中间值
	cur_pos[0].xpos= last_pos[1].xpos;
	cur_pos[0].ypos= last_pos[1].ypos;
	return 1;
nodata2:
	printk("y err\n");//取中间值
	cur_pos[0].xpos= middle_pos[1].xpos;
	cur_pos[0].ypos= middle_pos[1].ypos;
	return 1;
}
int bubblesort_correct_pos(int num)
{
	int i=0,j=0;
	int x_num,y_num;
	int inter_error;
	//bubble sort for right x pos
	for(i=0;i<num;i++)
	{
		for(j=i;j<num-1;j++)
		{
			int tmp_x,tmp_y;
			if(last_pos[j].xpos>last_pos[j+1].xpos)
			{
				tmp_x = last_pos[j].xpos;
				tmp_y = last_pos[j].ypos;
				last_pos[j].xpos = last_pos[j+1].xpos;
				last_pos[j].ypos = last_pos[j+1].ypos;
				last_pos[j+1].xpos = tmp_x;
				last_pos[j+1].ypos = tmp_y;
			}
		}
	}

	for(i=0;i<num-1;i++)
	{
		inter_error = last_pos[i+1].xpos - last_pos[i].xpos;
		if(inter_error>=ERROR_THRESLHOLD_NUM)
			break;
	}

	x_num = i+1;
	
	//bubble sort for right y pos
	for(i=0;i<x_num;i++)
	{
		for(j=i;j<x_num-1;j++)
		{
			int tmp_x,tmp_y;
			if(last_pos[j].ypos>last_pos[j+1].ypos)
			{
				tmp_x = last_pos[j].xpos;
				tmp_y = last_pos[j].ypos;
				last_pos[j].xpos = last_pos[j+1].xpos;
				last_pos[j].ypos = last_pos[j+1].ypos;
				last_pos[j+1].xpos = tmp_x;
				last_pos[j+1].ypos = tmp_y;
			}
		}
	}

	for(i=0;i<x_num-1;i++)
	{
		inter_error = last_pos[i+1].ypos - last_pos[i].ypos;
		if(inter_error>=ERROR_THRESLHOLD_NUM)
			break;
	}
	y_num = i+1;

	for(i=0;i<y_num;i++)
	{
		cur_pos[i].xpos= last_pos[i].xpos;
		cur_pos[i].ypos= last_pos[i].ypos;
	}
	return y_num;
}
static void touch_timer_isr(void)
{
	act_writel(act_readl(TP_CTL)&(~(1<<13)),TP_CTL);
	
	input_report_abs(am7x_tp.dev, ABS_PRESSURE, 0);
	input_sync(am7x_tp.dev);
	am7x_tp.pressed = 0;	
	filterflg = 0;
}
static void check_data(void)
{
	rcecivedateflg=1;
}
static irqreturn_t tp_isr(void)
{
	int tp_value=0;
	short x_value,y_value;
	if(act_readl(TP_CTL) & (1<<17))
	{
		am7x_tp.count = 0;
		am7x_tp.xp = 0;
		am7x_tp.yp = 0;
		IRQ_CLEAR_PENDING(act_readl(TP_CTL)|1<<17,TP_CTL);
		rcecivedateflg=0;
		mod_timer(&check_timer, jiffies+2);//jiffies=4ms
		return IRQ_HANDLED;
	}
		
	if(act_readl(TP_CTL)&(1<<19))
	{
		tp_value = act_readl(TP_DAT);

		if(tp_conf.x_y_switch)
		{
			y_value = (tp_value&0x07ff0000)>>16;
			x_value = tp_value&0x000007ff;
		}
		else
		{
			x_value = (tp_value&0x07ff0000)>>16;
			y_value = tp_value&0x000007ff;
		}
		
		if(x_value>=1024)
		{
			x_value  =(~((x_value&0x3ff)-1))&0x03ff;
			x_value = -x_value;
		}
		if(y_value>=1024)
		{
			y_value  =(~((y_value&0x3ff)-1))&0x03ff;
			y_value = -y_value;
		}
		x_value = x_value + 1024;
		y_value = y_value + 1024;
		if(rcecivedateflg==0)
		{
			#if open_debug
			printk("rcecivedateflg=0\n");
			#endif
			goto exit;
		}

		
		if(am7x_tp.count<CHECK_TOTAL_NUM)
		{
			last_pos[am7x_tp.count].xpos = x_value;
			last_pos[am7x_tp.count].ypos = y_value;
			am7x_tp.count++;
		}
		else
		{	
			int tmp_x=0,tmp_y=0;
			int i;
			int num;
			if((am7x_tp.pressed==0))
			{
				am7x_tp.pressed = 1;
				filterflg = 1;
				am7x_tp.count = 0;
				am7x_tp.xp = 0;
				am7x_tp.yp = 0;
				goto exit;
			}
			//num = filter_large_data(CHECK_TOTAL_NUM);
			//num = bubblesort_correct_pos(CHECK_TOTAL_NUM);
			num = checkdata_correct_pos(CHECK_TOTAL_NUM);
			if(num==0)
			{
				am7x_tp.count = 0;
				am7x_tp.xp = 0;
				am7x_tp.yp = 0;
				goto exit;
			}
			for(i=0;i<num;i++)
			{
				tmp_x += cur_pos[i].xpos;
				tmp_y += cur_pos[i].ypos;
			}
			am7x_tp.xp = tmp_x/num;
			am7x_tp.yp = tmp_y/num;
			
			am7x_tp.xp = (am7x_tp.xp*lcd_width)>>11;
			am7x_tp.yp = (am7x_tp.yp*lcd_height)>>11;
			
			if(tp_conf.x_mirror)
				am7x_tp.xp = lcd_width - am7x_tp.xp;
			if(tp_conf.y_mirror)
				am7x_tp.yp = lcd_height - am7x_tp.yp;
			
			input_report_abs(am7x_tp.dev, ABS_X, am7x_tp.xp);
			input_report_abs(am7x_tp.dev, ABS_Y, am7x_tp.yp);
			
			if(filterflg)
			{
				filterflg = 0;
				input_report_abs(am7x_tp.dev, ABS_PRESSURE, 1);
			}
			else if( am7x_tp.pressed==1)
			{
				input_report_abs(am7x_tp.dev, ABS_PRESSURE, 2);
			}
			//printk("XX==%d,    YY==%d\n",am7x_tp.xp,am7x_tp.yp);	
			input_sync(am7x_tp.dev);
			am7x_tp.count = 0;
			am7x_tp.xp = 0;
			am7x_tp.yp = 0;
		}
exit:		
		IRQ_CLEAR_PENDING(act_readl(TP_CTL)|3<<18, TP_CTL);
		mod_timer(&touch_timer,jiffies + 8);
		return IRQ_HANDLED;
	}
		

	return IRQ_NONE;

}

static int __init am_tp_init(void)
{
	int error,ret;
	int temp_v;
	printk(KERN_ERR "*********am_tp_init************\n");
	am7x_tp.dev = input_allocate_device();
	if (!am7x_tp.dev)
	{
		printk(KERN_ERR "input_allocate_device() failed for am tp\n");
		return -ENOMEM;
	}

	//printk("++++++++MTL5=%x   KEY_ACTION_MASK\n",am_get_mfl(5));
	am7x_tp.dev->name = "AM TouchScreen";
	am7x_tp.dev->phys = "AM/input0";
	am7x_tp.dev->id.bustype = BUS_HOST;
	am7x_tp.dev->id.vendor = 0x0001;
	am7x_tp.dev->id.product = 0x0002;
	am7x_tp.dev->id.version = 0x0100;

	am7x_tp.shift = 2;
	
	am7x_tp.dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	//am7x_tp.dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);

	input_set_abs_params(am7x_tp.dev, ABS_X, 0, 0x3FF, 0, 0);
	input_set_abs_params(am7x_tp.dev, ABS_Y, 0, 0x3FF, 0, 0);
	input_set_abs_params(am7x_tp.dev, ABS_PRESSURE, 0, 2, 0, 0);
	
	act_writel(act_readl(CMU_DEVCLKEN)|(1<<19), CMU_DEVCLKEN);
	mdelay(100);
	act_writel(act_readl(CMU_DEVRST)|(1<<23), CMU_DEVRST);
	mdelay(100);
	
	act_writel(act_readl(AMU_CTL)|(1<<9), AMU_CTL);
		
	temp_v = act_readl(TP_CTL);
	printk("@@TP_CTL-0===0x%x\n",temp_v);
	temp_v &= ~((1<<16) | (0x3f<<7));
	temp_v |= ((0<<9) | (2<<5) | (1<<0));// (1<<16) |(1<<14)|(1<<15)| 
	act_writel(temp_v, TP_CTL);
	printk("@@TP_CTL-1===%x\n",act_readl(TP_CTL));
	lcd_width = (act_readl(LCD_SIZE)&0x000007ff)+1;
	lcd_height = ((act_readl(LCD_SIZE)&0x07ff0000)>>16)+1;	
	printk("tp lcd_width==%d,lcd_height==%d\n",lcd_width,lcd_height);
	
	ret = request_irq(IRQ_TP, (irq_handler_t)tp_isr, 0, "touchpanel", NULL);
	if(ret < 0){
		printk("request irq error\n");
		return ret;
	}else
		printk("request irq ok\n");

	/* error check */
	
	error = input_register_device(am7x_tp.dev);
	if (error) {
		printk(KERN_ERR "input_allocate_device() failed for am tp\n");
		input_free_device(am7x_tp.dev);
		return error;
	}
	am_get_config(TOUCH_PANEL_CONFIG, (char*)&tp_conf, DATA_OFFSET, sizeof(struct touch_config));
	/*enable tp & tp irq at last*/
	temp_v |= ((1<<16) |(1<<14)|(1<<15));
	act_writel(temp_v, TP_CTL);
	
#if 0
	printk("x_y_switch==%d\n",tp_conf.x_y_switch);
	printk("x_mirror==%d\n",tp_conf.x_mirror);
	printk("y_mirror==%d\n",tp_conf.y_mirror);
#endif
	init_timer(&touch_timer);
	touch_timer.expires = 0;//jiffies +8;
	touch_timer.function = (void*)touch_timer_isr;
	add_timer(&touch_timer);
	/**/
	init_timer(&check_timer);			
	check_timer.function = (void*)check_data;
	check_timer.expires = 0;//jiffies=4ms
	add_timer(&check_timer);	
	
	return 0;
}

static void __exit am_tp_exit(void)
{
	printk(KERN_ERR "end tp device driver\n");
	input_unregister_device(am7x_tp.dev);
}

module_init(am_tp_init);
module_exit(am_tp_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kewen");
MODULE_DESCRIPTION("Touch panel driver for Actions-Micro");
MODULE_ALIAS("platform:am7x_tp");

