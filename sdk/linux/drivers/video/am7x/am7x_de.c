#include "am7xfb.h"
#include "DEdrive.h"

#include <linux/kernel.h>

OutImageAttribute outimage;
InImageAttribute inimage;
DisplayAttribute display;
struct am7xfb_inimage in_temp;
struct am7xfb_outimage out_temp;

static int color_space_set(struct am7xfb_outimage* out,struct am7xfb_inimage* in)
{
	CSCAttribute csc;

	csc.SW=OFF;
	if((in->image_colorspace==YUV_444)||(in->image_colorspace==YUV_420)||(in->image_colorspace==YUV_420))
	{
		if((out->image_colorspace==RGB_565)||(out->image_colorspace==RGB_888)||(out->image_colorspace==ARGB_888))
		{
			csc.a1=0x100;
			csc.a2=0x100;
			csc.b=0x15e;
			csc.c=0xb2;
			csc.d=0x56;
			csc.e=0x1bb;
			csc.ysub16=0;
			csc.color_coef=0;
			csc.offset1=0;
			csc.offset2=0;
			csc.contrast1=0;
			csc.contrast2=0;
			csc.SW=ON;
		}
	}
	CscSet(&csc);

}
int de_set_input(struct am7xfb_inimage* in)
{
	inimage.InImageStride=in->image_xres;
	inimage.InImageAdd=in->framebuffer_addr;
	inimage.InCImageAdd=0;
	switch(in->image_colorspace)
	{
	case RGB_565:		
		inimage.InImageMode=DE_RGB565;
		break;
	case RGB_888:
	case YUV_444:	
	case ARGB_888:	
		inimage.InImageMode=DE_RGB32;
		break;
	case YUV_420:	
		inimage.InImageMode=DE_YUV420;
		break;
	case YUV_422:	
		inimage.InImageMode=DE_YUV422;
		break;
	}
	inimage.InImageWidth=in->image_xres;
	inimage.InImageHeight=in->image_yres;
	outimage.DefaultColor=in->background_default_color;
	outimage.DisplaySource=in->default_color_mode_enable;
	printk("inimage.InCImageAdddddddd:%x\n",inimage.InCImageAdd);
		

//	ImageSet(&outimage,&inimage,&display);
	in_temp=*in;

	return 0;
}
int de_set_output(struct am7xfb_outimage* out)
{
	display.DisplayWidth=out->image_xres;
	display.DisplayHeight=out->image_yres;
	outimage.OutImageXStart=out->image_x_offset;
	outimage.OutImageXEnd=outimage.OutImageXStart+out->image_xres_virtual-1;
	outimage.OutImageYStart=out->image_y_offset;
	outimage.OutImageYEnd=outimage.OutImageYStart+out->image_yres_virtual-1;

	out_temp=*out;
//	ImageSet(&outimage,&inimage,&display);
	return 0;
}
int de_enable(void)
{

	act_writel(1<<14|act_readl(CMU_DEVCLKEN),CMU_DEVCLKEN);


	printk("outimage.OutImageXStart:%x\n",outimage.OutImageXStart);
	printk("outimage.OutImageXEnd:%x\n",outimage.OutImageXEnd);
	printk("outimage.OutImageYStart:%x\n",outimage.OutImageYStart);
	printk("outimage.OutImageYEnd:%x\n",outimage.OutImageYEnd);
	
	ImageSet(&outimage,&inimage,&display);

	printk("inimage.mode :%x\n",inimage.InImageMode);
	color_space_set(&out_temp,&in_temp);

	return 0;
}
int de_disable(void)
{
	DeReset();
	return 0;
}


int de_deithermode()
{
//	if(inimage->InImageMode == DE_RGB32 && outimage->image_colorspace == RGB_666)
		return Dither_888To666;
	
//	return OFF;
}



//must after de_set_input and de_set_output
int de_set_display_mode(int mode)
{

	display.DitherMode=OFF;
	display.GammaMode=OFF;
	switch (mode)
	{
	case RGB:		
		display.Mode=RGBMODE;
		break;	
	case RGB_I:	
		display.Mode=RGBMODE_I;
		break;
	case BT_P:	
		display.Mode=BTMODE_P;
		break;
	case BT_I:	
		display.Mode=BTMODE_I;
		break;
	case CPU:	
		display.Mode=CPUMODE;
		break;
	}
	display.DitherMode = de_deithermode(); 
	DisplayDeviceSet(&display);

	return 0;
}
int de_set_framebuffer_addr(unsigned long addr)
{
	ChangeFrameAdd(addr,0);
	return 0;
}
