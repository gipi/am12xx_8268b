#include "actions_regs.h"
#include "DEdrive.h"
#include <asm/system.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>


/*****************************************************/
/*     公用函数部分*/
/*****************************************************/
static void RegBitSet(int val,int reg,int msb,int lsb){                                             
unsigned int mask = 0xFFFFFFFF;
unsigned int old  = act_readl(reg);

	mask = mask << (31 - msb) >> (31 - msb + lsb) << lsb;
	act_writel((old & ~mask) | ((val << lsb) & mask), reg);		        
} 
                                                                                             
unsigned int RegBitRead(int reg,int msb,int lsb){                                                     
	unsigned int valr;
	
	valr=act_readl(reg);                                                                       
	return (valr<<(31-msb)>>(31-msb+lsb));                                                     
} 

OS_CPU_SR S_OSCPUSaveSR(void)
{
;//	local_irq_disable();
return 0;
}
void S_OSCPURestoreSR(OS_CPU_SR flags)
{
;//	local_irq_enable();
}

#define DeDriveDebug 0//1:de set error check on,;0:de set error check off;




#if (CONFIG_AM_CHIP_ID == 1201)
	#define LineBuffer 720
#elif ((CONFIG_AM_CHIP_ID == 1205) ||(CONFIG_AM_CHIP_ID == 1207))
	#define LineBuffer 800
#elif ((CONFIG_AM_CHIP_ID == 1203)||(CONFIG_AM_CHIP_ID == 1211))
	#define LineBuffer 1280
#elif (CONFIG_AM_CHIP_ID == 1220) || (CONFIG_AM_CHIP_ID == 1213)
	#define LineBuffer 1920
#endif

	INT32S  PixelToByte(	INT32S  mode,	INT32S  pixelnumber)
	{//%16byte=0
	INT32S  pixelbyte=0;
	
	switch(mode){
		case DE_RGB32:
			pixelbyte=pixelnumber*4;
			break;
		case DE_RGB565:
			pixelbyte=pixelnumber*2;
			break;
		case DE_YUV422:
			pixelbyte=pixelnumber*2;
			break;
		case DE_YUV420:
			pixelbyte=pixelnumber*1;
			break;
		case OSDBIT16MODE:
			pixelbyte=pixelnumber*2;
			break;
		case OSDBIT8MODE:
			pixelbyte=pixelnumber*1;
			break;				
		case OSDBIT4MODE:
			pixelbyte=pixelnumber/2;
			break;
		case OSDBIT2MODE:
			pixelbyte=pixelnumber/4;
			break;
		case OSDBIT1MODE:
			pixelbyte=pixelnumber/8;
			break;
		default:
			break;						
	}
	return(pixelbyte);
}

void EnterFrameOver(OS_CPU_SR* pFlag)
{
	int mode;

	*pFlag = S_OSCPUSaveSR();
#if (CONFIG_AM_CHIP_ID == 1211)||(CONFIG_AM_CHIP_ID == 1220)||(CONFIG_AM_CHIP_ID == 1213)
	if(RegBitRead(DE_INT,0,0)==1)
#else
	if(RegBitRead(DE_Con,9,9)==1)
#endif
	{
		mode=RegBitRead(DE_Con,8,7);
		if(mode==0x0)//lcd:rgb
		{
			RegBitSet(1,DE_INT,2,2);
			while(RegBitRead(DE_INT,2,2)==0);
		}else if(mode==0x1)//lcd:cpu
		{
			while(act_readl(cpumd_en)==1)
			{
				//printk("%s %d\n",__FILE__,__LINE__);
			}
		}else//bt
		{
		#if((CONFIG_AM_CHIP_ID == 1203)||(CONFIG_AM_CHIP_ID == 1207)||(CONFIG_AM_CHIP_ID == 1211)||(CONFIG_AM_CHIP_ID == 1220)||(CONFIG_AM_CHIP_ID == 1213))
				RegBitSet(1,DE_INT,10,10);
				while(RegBitRead(DE_INT,10,10)==0);	
		#elif((CONFIG_AM_CHIP_ID == 1201)||(CONFIG_AM_CHIP_ID == 1205))
				RegBitSet(1,DE_INT,2,2);
				while(RegBitRead(DE_INT,2,2)==0);
		#endif
		}
	}

}

void ExitFrameOver(OS_CPU_SR* pFlag)
{
	S_OSCPURestoreSR(*pFlag);
}

void EnterFrameActive(OS_CPU_SR* pFlag)
{
	int mode;

	*pFlag = S_OSCPUSaveSR();

	if(RegBitRead(DE_Con,9,9)==1)
	{
		mode=RegBitRead(DE_Con,8,7);
		if(mode==0x0)//lcd:rgb
		{
			RegBitSet(1,DE_INT,0,0);
			while(RegBitRead(DE_INT,0,0)==0);
		}else//bt
		{
			#if((CONFIG_AM_CHIP_ID == 1203)||(CONFIG_AM_CHIP_ID == 1207)||(CONFIG_AM_CHIP_ID == 1211)||(CONFIG_AM_CHIP_ID == 1220)||(CONFIG_AM_CHIP_ID == 1213))
				RegBitSet(1,DE_INT,8,8);
				while(RegBitRead(DE_INT,8,8)==0);	
			#elif((CONFIG_AM_CHIP_ID == 1201)||(CONFIG_AM_CHIP_ID == 1205))
				RegBitSet(1,DE_INT,0,0);
				while(RegBitRead(DE_INT,0,0)==0);
			#endif
		}
	}
}
/*****************************************************/
/***********设置错误提示函数部分**********/
/*****************************************************/	
INT32S  DisplayDeviceSetError(DisplayAttribute* display)
{
	#if(DeDriveDebug==1)
		INT32S  i;

		#if ((CONFIG_AM_CHIP_ID == 1201)||(CONFIG_AM_CHIP_ID == 1205))
			if(display->GammaMode!=GAMMA_OFF){ 
				for(i=0;i<31;i++){
					if(*(display->Gamma+i)>*(display->Gamma+i+1))
						return GAMMA_ERROR;
				}
			}
		#endif
	#endif	
	return DE_OK;
}

INT32S  ImageSetError(OutImageAttribute* outimage, InImageAttribute* inimage,DisplayAttribute* display)
{	
	#if(DeDriveDebug==1)
		INT32S  inputwidth,inputheight;
		INT32S  outputwidth,outputheight;

		if(display->Mode==CPUMODE)
			return DE_OK;	
		if(PixelToByte(inimage->InImageMode,inimage->InImageStride)%16!=0)
			return STRIDE_ERROR;
		if((outimage->OutImageXStart<1)|(outimage->OutImageYStart<1))
			return COORDINATE_ERROR;		
		inputwidth=inimage->InImageWidth;
		inputheight=inimage->InImageHeight;
		outputwidth=outimage->OutImageXEnd-outimage->OutImageXStart+1;
		outputheight=outimage->OutImageYEnd-outimage->OutImageYStart+1;
		if((outputwidth<inputwidth)||(outputheight<inputheight))
			return IMAGESIZE_ERROR;
		if(inputwidth>LineBuffer)
			return IMAGESIZE_ERROR;
		#if ((CONFIG_AM_CHIP_ID == 1201)||(CONFIG_AM_CHIP_ID == 1205))
			if((inimage->InImageAdd%16!=0)||(inimage->InCImageAdd%16!=0))
				return ADD_ERROR;
			if(inputheight*2<outputheight)
				return IMAGESIZE_ERROR;
			if(((outputwidth==inputwidth)&&(outputheight>inputheight))||((outputwidth>inputwidth)&&(outputheight==inputheight)))
				return IMAGESIZE_ERROR;
		#endif
	#endif
	return DE_OK;
}


INT32S  Over(	INT32S  start1,	INT32S  end1,	INT32S  start2,	INT32S  end2)
{
	if((start2>end1)|(start1>end2))
		return DE_OK;
	else
		return OSDOVER_ERROR;		
}

INT32S  OsdOver(	INT32S  osdcoor1,	INT32S  osdsize1,	INT32S  osdcoor2,	INT32S  osdsize2,	INT32S  type)
{
	INT32S  retval=DE_OK;
	#if(DeDriveDebug==1)
		INT32S  xstart1,ystart1,xend1,yend1,xstart2,ystart2,xend2,yend2;


		xstart1=RegBitRead(osdcoor1,26,16);
		ystart1=RegBitRead(osdcoor1,10,0)+1;
		xend1=xstart1+RegBitRead(osdsize1,26,16);
		yend1=ystart1+RegBitRead(osdsize1,10,0);
		xstart2=RegBitRead(osdcoor2,26,16);
		ystart2=RegBitRead(osdcoor2,10,0)+1;
		xend2=xstart2+RegBitRead(osdsize2,26,16);
		yend2=ystart2+RegBitRead(osdsize2,10,0);
					
		switch(type){
			case 0://YOver		
				retval=Over(ystart1,yend1,ystart2,yend2);
				break;		
			case 1://RectangleOver		
				if((Over(xstart1,xend1,xstart2,xend2)==DE_OK)//x_ok
					|(Over(ystart1,yend1,ystart2,yend2)==DE_OK))//y_ok
					retval=DE_OK;
				else
					retval=OSDOVER_ERROR;
				break;		
		}	
	#endif	
	return 	retval;	
	
}

INT32S  OsdSetError(OsdAttribute* osd)
{
	#if(DeDriveDebug==1)
		INT32S  osdcon[4]={OSD0con,OSD1con,OSD2con,OSD3con};
		INT32S  osdcoor[4]={OSD0coor,OSD1coor,OSD2coor,OSD3coor};
		INT32S  osdsize[4]={OSD0size,OSD1size,OSD2size,OSD3size};
		INT32S  i;

		if(osd->SW==OSD_ON)
		{	
			if(PixelToByte(osd->Mode,osd->Stride)%16!=0)
				return STRIDE_ERROR;
			if((osd->XStart<1)|(osd->YStart<1))
				return COORDINATE_ERROR;	
			#if ((CONFIG_AM_CHIP_ID == 1201)||(CONFIG_AM_CHIP_ID == 1205))
				if(osd->Add%16!=0)
					return ADD_ERROR;
				for(i=0;i<4;i++)
					if((i!=osd->No)&(RegBitRead(osdcon[i],19,19)==1))
						if(OsdOver(osdcoor[osd->No],osdsize[osd->No],osdcoor[i],osdsize[i],1)==OSDOVER_ERROR)
						{
							printk("OSDOVER_ERROR\n");				
							return OSDOVER_ERROR;
						}
				
				for(i=0;i<4;i++)
					if((i!=osd->No)&(RegBitRead(osdcon[i],19,19)==1))//compare other open osd
						if(OsdOver(osdcoor[osd->No],osdsize[osd->No],osdcoor[i],osdsize[i],0)==OSDOVER_ERROR)//yover
							if(((i<osd->No)&(RegBitRead(osdcoor[i],26,16)>RegBitRead(osdcoor[osd->No],26,16)))
								|((i>osd->No)&(RegBitRead(osdcoor[i],26,16)<RegBitRead(osdcoor[osd->No],26,16))))
								{
									printk("OSDORDER_ERROR\n");						
									return OSDORDER_ERROR;
								}
			#endif
		}	
	#endif
	return DE_OK;	
}

/*****************************************************/
/***********gamma设置函数**********/
/*****************************************************/	
INT32S  GammaSet(INT32U sw,INT32U * pGamma)
{
	INT32S  i;
	OS_CPU_SR	flag;
		
	RegBitSet(sw,DE_Con,5,5);// 1:enable,0:disable
	#if ((CONFIG_AM_CHIP_ID==1203)||(CONFIG_AM_CHIP_ID == 1207)||(CONFIG_AM_CHIP_ID == 1211)||(CONFIG_AM_CHIP_ID == 1220)||(CONFIG_AM_CHIP_ID == 1213))
	for(i=0;i<256;i++)
		act_writel((*(pGamma+i)&0xffffff)|(i<<24),GammaRAM);
	#elif ((CONFIG_AM_CHIP_ID == 1201)||(CONFIG_AM_CHIP_ID == 1205))
		for(i=0;i<32;i++)
			act_writel(*(pGamma+i),GammaRAM);	
	#endif

	act_writel(0x1,DB_WR);
	EnterFrameOver(&flag);
	ExitFrameOver(&flag);

	return DE_OK;
}
/*****************************************************/
/**************显示设备设置函数**************/
/**设置项包括: LCD 接口类型				***/
/*					Dither开关状态				***/
/*					gamma调整开关状态		***/
/*****************************************************/	
INT32S  DisplayDeviceSet(DisplayAttribute* display)
{
	//sema_destroy(&de_sem);

	switch(display->Mode)
	{
		case RGBMODE:
			RegBitSet(0,DE_Con,8,7);//0:rgb;1:cpu;2:bt			
			RegBitSet(0,DE_Con,0,0);//0:interlace;1:progressive;
			break;
		case RGBMODE_I:
			RegBitSet(0,DE_Con,8,7);//0:rgb;1:cpu;2:bt			
			RegBitSet(1,DE_Con,0,0);//0:interlace;1:progressive;
			break;							
		case BTMODE_I:
			RegBitSet(2,DE_Con,8,7);//0:rgb;1:cpu;2:bt
			RegBitSet(1,DE_Con,0,0);//1:interlace;0:progressive;
			break;
		case BTMODE_P:
			RegBitSet(2,DE_Con,8,7);//0:rgb;1:cpu;2:bt
			RegBitSet(0,DE_Con,0,0);//1:interlace;0:progressive;
			break;
		case CPUMODE:
			RegBitSet(1,DE_Con,8,7);//0:rgb;1:cpu;2:bt
			break;			
		default:
			break;	
	}
	act_writel(((display->DisplayWidth)<<16)|(display->DisplayHeight),Frm_Size);	
	if((display->Mode==RGBMODE)||(display->Mode==RGBMODE_I))
		act_writel(0x4,DE_INT_CFG);
	else if(display->Mode==CPUMODE)
		act_writel(0x0,DE_INT_CFG);
	else
		#if((CONFIG_AM_CHIP_ID == 1203)||(CONFIG_AM_CHIP_ID == 1207)||(CONFIG_AM_CHIP_ID == 1211)||(CONFIG_AM_CHIP_ID == 1220)||(CONFIG_AM_CHIP_ID == 1213))
			act_writel(0x400,DE_INT_CFG);
		#else
			act_writel(0x4,DE_INT_CFG);
		#endif		
	if((display->DitherMode==DITHER_OFF)||(display->Mode==CPUMODE))
		RegBitSet(0,DE_Con,4,4);// 1:enable,0:disable
	else
	{
		RegBitSet(display->DitherMode-1,Dithering,1,0);//dithering mode
		RegBitSet(1,DE_Con,4,4);// 1:enable,0:disable	
	}
	RegBitSet(display->GammaMode,DE_Con,5,5);// 1:enable,0:disable
	if(display->GammaMode!=GAMMA_OFF)
		GammaSet(1,(INT32U *)(display->Gamma));
	#if ((CONFIG_AM_CHIP_ID == 1201)||(CONFIG_AM_CHIP_ID == 1205))
		if(display->Mode==CPUMODE)
		{
			act_writel(0xa000a,Frm_FIFO_Thrd);//note1:Hclk/Dclk>6;
			act_writel(0xff,cpumd_lt);
		}else
		{
			RegBitSet(0x17,Frm_FIFO_Thrd,20,16);//main frame FIFO partial full threshold
			RegBitSet(0x17,OSD_FIFO_Thrd,20,16);//OSD FIFO partial full threshold
		}
	#endif
	#if (CONFIG_AM_CHIP_ID == 1211)||(CONFIG_AM_CHIP_ID == 1220)||(CONFIG_AM_CHIP_ID == 1213)
	act_writel(0x1f0008,	0xb0040000+0x30);
	act_writel(0x1f0008,	0xb0040000+0x34);
	act_writel(0x0,		0xb0040000+0x3c);
	act_writel(0x30002,	0xb0040000+0xf8);
	act_writel(0x30002,	0xb0040000+0xfc);
	#endif
	RegBitSet(1,DE_Con,2,2);//1:enable,0:disable main frame enable
	RegBitSet(1,DE_Con,9,9);//DE enable 1:enable,0:disable
	
	act_writel(0x1,DB_WR);
	return DisplayDeviceSetError(display);
}

/*****************************************************/
/***********输入输出图像显示设置函数****/
/*****************************************************/	
INT32S  ImageSet(OutImageAttribute* outimage, InImageAttribute* inimage,DisplayAttribute* display)
{
	OS_CPU_SR	flag;
	INT32S  inputwidth,inputheight;
	INT32S  outputwidth,outputheight;
	

	if(display->Mode==CPUMODE)
	{
		EnterFrameOver(&flag);
		act_writel(inimage->InImageAdd,Frm_BA);//sdram position of input frame %16Xbyte=0
		act_writel(inimage->InCImageAdd,UV_BA);//sdram position of input frame %16Xbyte=0
		act_writel(PixelToByte(inimage->InImageMode,inimage->InImageWidth*inimage->InImageHeight),cpumd_tc);
	}else
	{
	#if ((CONFIG_AM_CHIP_ID == 1201)||(CONFIG_AM_CHIP_ID == 1205)||(CONFIG_AM_CHIP_ID == 1203))
		if(1)
		{
		OS_CPU_SR	temp;
		EnterFrameActive(&flag);
		act_writel(inimage->InImageAdd,Frm_BA);//sdram position of input frame %16Xbyte=0
		act_writel(inimage->InCImageAdd,UV_BA);//sdram position of input frame %16Xbyte=0		
		EnterFrameOver(&temp);
		}

	#endif
	#if ((CONFIG_AM_CHIP_ID == 1207)||(CONFIG_AM_CHIP_ID == 1211)||(CONFIG_AM_CHIP_ID == 1220)||(CONFIG_AM_CHIP_ID == 1213))
		act_writel(inimage->InImageAdd,Frm_BA);//sdram position of input frame %16Xbyte=0
		act_writel(inimage->InCImageAdd,UV_BA);//sdram position of input frame %16Xbyte=0		
	#endif	
		act_writel(PixelToByte(inimage->InImageMode,inimage->InImageStride),Frm_Stride);//stride of input frame		
		//SCALER		
		inputwidth=inimage->InImageWidth;
		inputheight=inimage->InImageHeight;
		outputwidth=outimage->OutImageXEnd-outimage->OutImageXStart+1;
		outputheight=outimage->OutImageYEnd-outimage->OutImageYStart+1;
		act_writel((inputwidth<<16)|inputheight,Input_size);//input width//input height	
		act_writel((outputwidth<<16)|outputheight,Output_size);//output width//output height
		if((outputwidth==1)||(outputheight==1))
		{
			printk("de parameter error\n");
//			return -1;
		}
		act_writel(((65536*(inputwidth-1)/(outputwidth-1))<<16)|(65536*(inputheight-1)/(outputheight-1)),Scalar_step);//horizontal step//vertical step		
		#if ((CONFIG_AM_CHIP_ID == 1201)||(CONFIG_AM_CHIP_ID == 1205))
			if((inputwidth<=LineBuffer)&&((outputwidth>inputwidth)&&(outputheight>inputheight)))
				RegBitSet(1,DE_Con,10,10);//1:enable,0:disable
			else
				RegBitSet(0,DE_Con,10,10);//1:enable,0:disable
		#elif((CONFIG_AM_CHIP_ID == 1203)||(CONFIG_AM_CHIP_ID == 1207)||(CONFIG_AM_CHIP_ID == 1211)||(CONFIG_AM_CHIP_ID == 1220)||(CONFIG_AM_CHIP_ID == 1213))
			if((inputwidth<=LineBuffer)&&((outputwidth>=inputwidth)&&(outputheight>=inputheight)))
			{
				if(outputwidth==inputwidth)
					RegBitSet(1,DE_Con,27,27);
				else
					RegBitSet(0,DE_Con,27,27);
				if(outputheight==inputheight)
					RegBitSet(1,DE_Con,28,28);
				else
					RegBitSet(0,DE_Con,28,28);				
				RegBitSet(1,DE_Con,10,10);//1:enable,0:disable
			}else
				RegBitSet(0,DE_Con,10,10);//1:enable,0:disable	
		#endif
		//WINDOWS
		if((inimage->InImageMode==DE_YUV420)&&(outimage->OutImageYStart%2==0))
			act_writel((outimage->OutImageXStart<<16)|(outimage->OutImageYStart),Win_Coor1);
		else
			act_writel((outimage->OutImageXStart<<16)|(outimage->OutImageYStart-1),Win_Coor1);
		act_writel((outimage->OutImageXEnd<<16)|(outimage->OutImageYEnd-1),Win_Coor2);
		RegBitSet(1,DE_Con,3,3);//1:enable,0:disable	
		//INPUT CONFIG
		if(inimage->InImageWidth<=LineBuffer)
			RegBitSet(1,DE_Con,11,11);//line buffer 1:enable,0:disable
		else
			RegBitSet(0,DE_Con,11,11);//line buffer 1:enable,0:disable
	}
	//OUTPUT CONFIG
	act_writel(outimage->DefaultColor,D_Color);//default corlor	
	RegBitSet(outimage->DisplaySource,DE_Con,12,12);//1:enable,0:disable	
	RegBitSet(inimage->InImageMode,DE_Con,17,16);//InImageMode
	//if(display->Mode==CPUMODE)
		//act_writel(1,cpumd_en);
	
	#if ((CONFIG_AM_CHIP_ID == 1207)||(CONFIG_AM_CHIP_ID == 1211)||(CONFIG_AM_CHIP_ID == 1220)||(CONFIG_AM_CHIP_ID == 1213))
		act_writel(0x1,DB_WR);
	EnterFrameOver(&flag);
	
	#endif
	ExitFrameOver(&flag);
	
	return ImageSetError(outimage,inimage,display);
}
void  pallet565topallet888(int* pallet888,int pallet565,int transcolor)
{
	int a,r,g,b;
	if(pallet565==transcolor)
		a=0;
	else
		a=0xf;
	r=(pallet565>>8)&0xf8;
	g=(pallet565>>3)&0xfc;
	b=(pallet565<<3)&0xf8;
	*pallet888=((a<<24)|(r<<16)|(g<<8)|(b));
}
/*****************************************************/
/***********OSD调色板数据设置函数**********/
/*****************************************************/	
void Pallet16Set(	INT32S * Pallet,	INT32S  TransColor)
{
	INT32S  i;
	INT32S  pallet565,pallet888;
	OS_CPU_SR	flag;

	EnterFrameOver(&flag);	
	#if ((CONFIG_AM_CHIP_ID == 1201)||(CONFIG_AM_CHIP_ID == 1205))
		for(i=0;i<8;i++)	
			act_writel(*(Pallet+i),Pallet0Ram);		
		act_writel(TransColor,Pallet_tcolor);
	#elif ((CONFIG_AM_CHIP_ID == 1203)||(CONFIG_AM_CHIP_ID == 1207)||(CONFIG_AM_CHIP_ID == 1211)||(CONFIG_AM_CHIP_ID == 1220)||(CONFIG_AM_CHIP_ID == 1213))
		for(i=0;i<16;i++)
		{
			pallet565=(*(((INT16U*)Pallet)+i))&0xffff;
			pallet565topallet888(&pallet888,pallet565,TransColor);
			act_writel(pallet888,Pallet1Ram);
		}	
	#endif
	ExitFrameOver(&flag);

}

void Pallet256Set(INT32S * Pallet)
{
	INT32S  i;
	OS_CPU_SR	flag;

	EnterFrameOver(&flag);		
	act_writel(0,Pallet0_ADD);
	for(i=0;i<256;i++)	
		act_writel(*(Pallet+i),Pallet0Ram);
	ExitFrameOver(&flag);
}

/*****************************************************/
/***********OSD设置函数**********/
/*****************************************************/	
INT32S  OsdSet(OsdAttribute* osd)
{
	INT32S  osdba[2]={OSD0_BA,OSD1_BA};
	INT32S  osdstride[2]={OSD0_Stride,OSD1_Stride};
	INT32S  osdcoor[2]={OSD0coor,OSD1coor};
	INT32S  osdsize[2]={OSD0size,OSD1size};
	INT32S  osdcon[2]={OSD0con,OSD1con};
	INT32S  osdtcolor[2]={OSD0_transparent,OSD1_transparent};
	OS_CPU_SR	flag;

	act_writel(osd->Add,osdba[osd->No]);//sdram position of osd index	

	if(osd->SW==OSD_ON)
	{
		act_writel(0x80000000|osd->TransColor,osdtcolor[osd->No]);
		act_writel(PixelToByte(osd->Mode,osd->Stride),osdstride[osd->No]);//stride of osd
		if(RegBitRead(DE_Con,0,0)==1)
			act_writel(((osd->XStart)<<16)|((osd->YStart-1)/2),osdcoor[osd->No]);//osd x_start//osd y_start
		else
			act_writel(((osd->XStart)<<16)|(osd->YStart-1),osdcoor[osd->No]);//osd x_start//osd y_start
		if(RegBitRead(DE_Con,0,0)==1)
			act_writel(((osd->Width)<<16)|((osd->Height)/2),osdsize[osd->No]);//osd x_size;//osd y_size
		else			
			act_writel(((osd->Width)<<16)|(osd->Height),osdsize[osd->No]);//osd x_size;//osd y_size

		//act_writel(osd->Alpha,osdcon[osd->No]);// 1:enable,0:disable OSD // osd color mode// osd alpha
		RegBitSet(osd->Alpha,osdcon[osd->No],3,0);
		if(osd->Mode==OSDBIT8MODE)
			RegBitSet(0,osdcon[osd->No],18,15);
		else if(osd->Mode==OSDBIT4MODE)
			RegBitSet(0x8,osdcon[osd->No],18,15);
		else if(osd->Mode==OSDBIT2MODE)
			RegBitSet(0xa,osdcon[osd->No],18,15);
		else if(osd->Mode==OSDBIT1MODE)
			RegBitSet(0xc,osdcon[osd->No],18,15);
		else if(osd->Mode==OSDBIT16MODE)
			RegBitSet(0xe,osdcon[osd->No],18,15);
		else if(osd->Mode==OSDBIT32MODE)
		{
			RegBitSet(0x9,osdcon[osd->No],18,15);
			RegBitSet(0x0,osdcon[osd->No],14,14);
		}
	}	
	RegBitSet(osd->SW,osdcon[osd->No],19,19);// 1:enable,0:disable OSD 
	#if ((CONFIG_AM_CHIP_ID == 1207)||(CONFIG_AM_CHIP_ID == 1211)||(CONFIG_AM_CHIP_ID == 1220)||(CONFIG_AM_CHIP_ID == 1213))
	act_writel(0x1,DB_WR);
	#endif
	EnterFrameOver(&flag);
		

	return OsdSetError(osd);
}
INT32S LimitRange(INT32S* p,INT32S max,INT32S min)
{
	INT32S ret =0;
	
	if(*p>max)
	{
		*p=max;
		ret=1;
	}else if(*p<min)
	{
		*p=min;
		ret=1;
	}
	return ret;
}
/*****************************************************/
/***********CSC设置函数**********/
/*****************************************************/	
INT32S  CscSet( CSCAttribute* csc)
{
	INT32S  retval=DE_OK;	
	OS_CPU_SR	flag;

	  if(LimitRange(&(csc->a1),1023,0)==1)
	  	retval=CSC_ERROR;
	  if(LimitRange(&(csc->a2),1023,0)==1)
	  	retval=CSC_ERROR;	    
	  if(LimitRange(&(csc->b),1023,0)==1)
	  	retval=CSC_ERROR;	    
	 if(LimitRange(&(csc->c),1023,0)==1)
	  	retval=CSC_ERROR;    
	  if(LimitRange(&(csc->d),1023,0)==1)
	  	retval=CSC_ERROR;	    
	  if(LimitRange(&(csc->e),1023,0)==1)
	  	retval=CSC_ERROR;
	  if(LimitRange(&(csc->color_coef),127,-128)==1)
	  	retval=CSC_ERROR;	    
	  if(LimitRange(&(csc->offset1),511,-512)==1)
	  	retval=CSC_ERROR;	    
	  if(LimitRange(&(csc->offset2),511,-512)==1)
	  	retval=CSC_ERROR;	    
/*	
	printk(" __________________%s %d\n ",__FILE__,__LINE__);
	printk("csc->a1=%d\n",csc->a1);
	printk("csc->a2=%d\n",csc->a2);
	printk("csc->c=%d\n",csc->c);
	printk("csc->d=%d\n",csc->d);
	printk("csc->e=%d\n",csc->e);
	printk("csc->ysub16=%d\n",csc->ysub16);
	printk("csc->color_coef=%d\n",csc->color_coef);
	printk("csc->offset1=%d\n",csc->offset1);
	printk("csc->offset2=%d\n",csc->offset2);
	printk("csc->contrast1=%d\n",csc->contrast1);
	printk("csc->contrast2=%d\n",csc->contrast2);
*/	
	act_writel((csc->a1<<16)|csc->a2,CSCcef1);
	act_writel((csc->b<<16)|csc->c,CSCcef2);
	act_writel((csc->d<<16)|csc->e,CSCcef3);
	act_writel((csc->ysub16<<16)|csc->color_coef,Color_coef);
	act_writel((csc->offset1<<16)|csc->offset2,DE_Offset);
	act_writel((csc->contrast1<<16)|csc->contrast2,Contrast);
	RegBitSet(csc->SW,DE_Con,6,6);
	
	act_writel(0x1,DB_WR);
	EnterFrameOver(&flag);
	ExitFrameOver(&flag);
	return retval;

}
INT32S  LtiCti( LtiCtiAttribute* lticti)
{		
	OS_CPU_SR	flag;
	act_writel((lticti->dlti_thd<<24)|(lticti->dlti_thd1<<16)|(lticti->dlti_gain<<8)|(lticti->dlti_step),DE_DLTI);									
	act_writel((lticti->dcti_thd<<24)|(lticti->dcti_thd1<<16)|(lticti->dcti_gain<<8)|(lticti->dcti_step),DE_DCTI);	
	RegBitSet(lticti->SW,DE_Con,23,23);// 1:enable,0:disable
	RegBitSet(lticti->SW,DE_Con,24,24);// 1:enable,0:disable
	RegBitSet(~(lticti->SW),DE_Con,25,25);// 1:bypass,0:no bypass
	
	act_writel(0x1,DB_WR);
	EnterFrameOver(&flag);
	ExitFrameOver(&flag);	
	return DE_OK;
}
/*****************************************************/
/***********Peak-Coring设置函数**********/
/*****************************************************/	
INT32S  LumHorFilter( FilterAttribute* filter)
{	
	OS_CPU_SR	flag;
	filter->c0=	filter->c0&0xff;
	filter->c1=	filter->c1&0xff;
	filter->c2=	filter->c2&0xff;
	act_writel((filter->c0<<24)|(filter->c1<<16)|(filter->c2<<8)|(filter->lv<<1)|(filter->mode),PEAK_Coring);															
	RegBitSet(filter->SW,DE_Con,18,18);// 1:enable,0:disable
	
	act_writel(0x1,DB_WR);
	EnterFrameOver(&flag);
	ExitFrameOver(&flag);	
	return DE_OK;
}
/*****************************************************/
/***********BleWle设置函数**********/
/*****************************************************/	
void BleWle(BleWleAttribute* blewle)
{
	OS_CPU_SR	flag;
	RegBitSet(blewle->ble_gain,BLE,7,0);//black-level
	RegBitSet(blewle->ble_b,BLE,15,8);//black-gain
	RegBitSet(blewle->wle_gain,WLE,7,0);//white-level
	RegBitSet(blewle->wle_w,WLE,15,8);//white-gain
	//RegBitSet(blewle->offset,OFFSET,7,0);//offset
	RegBitSet(blewle->SW,DE_Con,30,30);//enable
	RegBitSet(blewle->SW,DE_Con,31,31);//enable
	act_writel(0x1,DB_WR);
	EnterFrameOver(&flag);
	ExitFrameOver(&flag);	
}
/*****************************************************/
/***********HistogramGet函数**********/
/*****************************************************/	
INT32S  HistogramGet(INT32S  channel)
{
	RegBitSet(channel,DE_Con,20,19);
	RegBitSet(1,DE_Con,22,22);
	while(RegBitRead(DE_Con,22,22)==1);
	
	return DE_OK;
}


/*****************************************************/
/***********DE模块初始化函数**********/
/*****************************************************/	
INT32S  DeReset(void)
{
	OS_CPU_SR	flag;
	
	EnterFrameOver(&flag);
	RegBitSet(0,CMU_DEVRST,15,15);
	RegBitSet(1,CMU_DEVRST,15,15);
	RegBitSet(0,CMU_DISPLAYCLK,7,7);
	ExitFrameOver(&flag);

	return DE_OK;
}

void  DisplayClkDisable(void)
{
	OS_CPU_SR	flag;
	
	EnterFrameOver(&flag);
	RegBitSet(0,DE_Con,9,9);			//DE enable 1:enable,0:disable
	RegBitSet(0,CMU_LCDPLL,6,6);		//LCD PLL enable 1:enable,0:disable 注意1201 spec有误
	ExitFrameOver(&flag);
}
 
void ClearIntFlag(void)
{
	act_writel(0x404, DE_INT);
} 

 
void ChangeFrameAdd(	INT32S  YAdd,	INT32S  CAdd)
{

	OS_CPU_SR flag ;	
	int mode;
		
	mode=RegBitRead(DE_Con,8,7);
	if(mode==0x1)//lcd:cpu
	{
		EnterFrameOver(&flag);
		act_writel(YAdd,Frm_BA);//sdram position of input frame %16Xbyte=0
		act_writel(CAdd,UV_BA);//sdram position of input frame %16Xbyte=0
		//act_writel(1,cpumd_en);
		ExitFrameOver(&flag);
	}else 
	{
		flag = S_OSCPUSaveSR();
		act_writel(YAdd,Frm_BA);//sdram position of input frame %16Xbyte=0
		act_writel(CAdd,UV_BA);//sdram position of input frame %16Xbyte=0
		S_OSCPURestoreSR(flag);
	}
	act_writel(0x1,DB_WR);
}



#if 0
void DeRegRead(DE_REG* de)
{	
	de->de_con=act_readl(DE_Con);
	de->d_color=act_readl(D_Color);
	de->frm_size=act_readl(Frm_Size);
	de->frm_ba=act_readl(Frm_BA);
	de->frm_stride=act_readl(Frm_Stride);
	de->win_coor1=act_readl(Win_Coor1);
	de->win_coor2=act_readl(Win_Coor2);
	de->csccef1=act_readl(CSCcef1);
	de->csccef2=act_readl(CSCcef2);
	de->csccef3=act_readl(CSCcef3);
	de->color_coef=act_readl(Color_coef);
	de->offset=act_readl(DE_Offset);
	de->contrast=act_readl(Contrast);		
	de->input_size=act_readl(Input_size);
	de->output_size=act_readl(Output_size);
	de->scaler_step=act_readl(Scalar_step);	
	
}

void DeRegWrite(DE_REG* de)
{
	OS_CPU_SR	flag;
	OS_CPU_SR	temp;
	int mode;

	EnterFrameActive(&flag);
	act_writel(de->frm_ba,Frm_BA);
	EnterFrameOver(&temp);
	act_writel(de->de_con,DE_Con);	
	act_writel(de->d_color,D_Color);	
	act_writel(de->frm_size,Frm_Size);
	act_writel(de->frm_stride,Frm_Stride);
	act_writel(de->win_coor1,Win_Coor1);
	act_writel(de->win_coor2,Win_Coor2);
	act_writel(de->csccef1,CSCcef1);
	act_writel(de->csccef2,CSCcef2);
	act_writel(de->csccef3,CSCcef3);
	act_writel(de->color_coef,Color_coef);
	act_writel(de->offset,DE_Offset);
	act_writel(de->contrast,Contrast);
	act_writel(de->input_size,Input_size);			
	act_writel(de->output_size,Output_size);
	act_writel(de->scaler_step,Scalar_step);
	ExitFrameOver(&flag);
	act_writel(0x1,DB_WR);
	
}	
#endif

