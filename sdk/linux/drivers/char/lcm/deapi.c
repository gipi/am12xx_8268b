#include "deapi.h"
#include <linux/kernel.h>

#include <asm/io.h>

DE_INFO de_ifo_buffer;

extern void RegBitSet(int val,int reg,int msb,int lsb);

                                                                                             
extern unsigned int RegBitRead(int reg,int msb,int lsb);

void update_color_info(DE_INFO *de_ifo,CSCAttribute *csc)
{
	INT32S	graycenter;
	INT32S	thr1y, thr2y, off1, off2, thr1, thr2, a1, a2;//
	
	 /*  brightness */
	csc->color_coef=de_ifo->brightness;
	csc->ysub16=de_ifo->ysub16;
	/* Contrast */
	switch(de_ifo->ysub16){
		case 0:
			graycenter=124;
			break;
		case 1:
			graycenter=110;
			break;
		default:
			graycenter=124;
			//printf("Error ysub16=[%d]\n",de_ifo->ysub16);
			break;
	}
	thr1=(de_ifo->contrast+graycenter)/2;
	thr1y=graycenter-thr1;
	thr2=graycenter*2-thr1;
	thr2y=graycenter*2-thr1y;	
	a1=256*thr1y/thr1;
	if ((thr2 - thr1)==0)
	{
		printk("Error (thr2 - thr1)  return\n");
	//	return -1;
	}
	a2=256*(thr2y - thr1y)/(thr2 - thr1);
	off1=thr1y-a2*thr1/256;
	off2=thr2y-a1*thr2/256;


	csc->contrast1=thr1;
	csc->contrast2=thr2;	
	csc->a1=a1*de_ifo->a/256;
	csc->a2=a2*de_ifo->a/256;
	csc->offset1=off1*de_ifo->a/256; 
	csc->offset2=off2*de_ifo->a/256;
	/* saturation */
	csc->b=(de_ifo->saturation+64)*de_ifo->b/64;
	csc->c=(de_ifo->saturation+64)*de_ifo->c/64;
	csc->d=(de_ifo->saturation+64)*de_ifo->d/64;
	csc->e=(de_ifo->saturation+64)*de_ifo->e/64;
	/* hue */
	if(de_ifo->hue>0){
		csc->d=(60-de_ifo->hue)*csc->d/60;
		csc->e=(60-de_ifo->hue)*csc->e/60;
	}else{
		csc->b=(60+de_ifo->hue)*csc->b/60;
		csc->c=(60+de_ifo->hue)*csc->c/60;		
	}
	csc->SW=ON;

}

	INT32S  de_color_adjust(DE_INFO *de_ifo){
	CSCAttribute csc;
		INT32S  graycenter;
		INT32S  thr1y, thr2y, off1, off2, thr1, thr2, a1, a2;//

	 /*  brightness */
	csc.color_coef=de_ifo->brightness;
	csc.ysub16=de_ifo->ysub16;
	/* Contrast */
	switch(de_ifo->ysub16){
		case 0:
			graycenter=124;
			break;
		case 1:
			graycenter=110;
			break;
		default:
			graycenter=124;
			//printf("Error ysub16=[%d]\n",de_ifo->ysub16);
			break;
	}
	thr1=(de_ifo->contrast+graycenter)/2;
	thr1y=graycenter-thr1;
	thr2=graycenter*2-thr1;
	thr2y=graycenter*2-thr1y;	
	a1=256*thr1y/thr1;
	if ((thr2 - thr1)==0)
	{
		//printf("Error (thr2 - thr1)  return\n");
		return -1;
	}
	a2=256*(thr2y - thr1y)/(thr2 - thr1);
	off1=thr1y-a2*thr1/256;
	off2=thr2y-a1*thr2/256;


	csc.contrast1=thr1;
	csc.contrast2=thr2;	
	csc.a1=a1*de_ifo->a/256;
	csc.a2=a2*de_ifo->a/256;
	csc.offset1=off1*de_ifo->a/256; 
	csc.offset2=off2*de_ifo->a/256;
	/* saturation */
	csc.b=(de_ifo->saturation+64)*de_ifo->b/64;
	csc.c=(de_ifo->saturation+64)*de_ifo->c/64;
	csc.d=(de_ifo->saturation+64)*de_ifo->d/64;
	csc.e=(de_ifo->saturation+64)*de_ifo->e/64;
	/* hue */
	if(de_ifo->hue>0){
		csc.d=(60-de_ifo->hue)*csc.d/60;
		csc.e=(60-de_ifo->hue)*csc.e/60;
	}else{
		csc.b=(60+de_ifo->hue)*csc.b/60;
		csc.c=(60+de_ifo->hue)*csc.c/60;		
	}
	csc.SW=ON;
	return CscSet(&csc);
		
}

	INT32S  de_sharpness_adjust(DE_INFO *de_ifo){
	LtiCtiAttribute lticti;
	FilterAttribute filter;
	
	if(de_ifo->sharpness>0){
		lticti.SW=1;
		lticti.dlti_thd=255*(31-de_ifo->sharpness)/31;
		lticti.dlti_thd1=31-de_ifo->sharpness;
		lticti.dlti_gain=de_ifo->sharpness;
		lticti.dlti_step=3;
		lticti.dcti_thd=lticti.dlti_thd;
		lticti.dcti_thd1=lticti.dlti_thd1;
		lticti.dcti_gain=lticti.dlti_gain;
		lticti.dcti_step=lticti.dlti_step;
		filter.SW=1;
		filter.c0=126*de_ifo->sharpness/31;
		filter.c1=-42*de_ifo->sharpness/31;
		filter.c2=-21*de_ifo->sharpness/31;			
		filter.lv=31-de_ifo->sharpness;;
		filter.mode=0;				
	}else if(de_ifo->sharpness==0){
		lticti.SW=0;
		filter.SW=0;
	}else{
		lticti.SW=0;
		filter.SW=1;
		filter.c0=127+102*de_ifo->sharpness/31;
		filter.c1=-102*de_ifo->sharpness/124;
		filter.c2=filter.c1;			
		filter.lv=0;
		filter.mode=1;		
	}
	LtiCti(&lticti);
	LumHorFilter(&filter);
	
	return DE_OK;	
} 

INT32S  de_gamma_adjust(DE_INFO *de_ifo)
{
	INT32U ret=0;
	INT32U * gamma;

	gamma = de_ifo->Gamma;
	ret=GammaSet(de_ifo->GammaMode,gamma);		
	
	return ret;
}

	INT32S  de_cfg_update(DE_INFO *de_ifo)
{
	OutImageAttribute outimage;
	InImageAttribute inimage;
	DisplayAttribute display;
	CSCAttribute csc;
	INT32S  retval;
	INT32S  pixel_deviation;
	display.Mode= de_ifo->Mode;
	outimage.DisplaySource=de_ifo->DisplaySource;
	outimage.DefaultColor=de_ifo->DefaultColor;
	outimage.OutImageXStart=de_ifo->pip_ori_x;
	outimage.OutImageYStart=de_ifo->pip_ori_y;
	outimage.OutImageXEnd=de_ifo->pip_ori_x+de_ifo->pip_frame_width-1;
	outimage.OutImageYEnd=de_ifo->pip_ori_y+de_ifo->pip_frame_height-1;
	
	inimage.InImageStride=de_ifo->input_width;	
	switch(de_ifo->input_pix_fmt){
		case PIX_FMT_YCBCR_4_2_0_SEMIPLANAR:
			pixel_deviation=(de_ifo->crop_ori_y-1)/2*2*(de_ifo->input_width)+(de_ifo->crop_ori_x-1)/16*16;			
			inimage.InImageAdd=de_ifo->input_luma_bus_addr+pixel_deviation;
			inimage.InCImageAdd=de_ifo->input_chroma_bus_addr+(de_ifo->crop_ori_y-1)/2*(de_ifo->input_width)+(de_ifo->crop_ori_x-1)/16*16;
			inimage.InImageMode=DE_YUV420;
			break;
		case PIX_FMT_YCBCR_4_2_2_INTERLEAVED:
			pixel_deviation=(de_ifo->crop_ori_y-1)*(de_ifo->input_width)+(de_ifo->crop_ori_x-1)/8*8;
			inimage.InImageAdd=de_ifo->input_luma_bus_addr+pixel_deviation*2;
			inimage.InImageMode=DE_YUV422;
			break;
		case PIX_FMT_RGB16_5_6_5:
			pixel_deviation=(de_ifo->crop_ori_y-1)*(de_ifo->input_width)+(de_ifo->crop_ori_x-1)/8*8;
			inimage.InImageAdd=de_ifo->input_rgb_bus_addr+pixel_deviation*2;
			inimage.InImageMode=DE_RGB565;
			break;
		case PIX_FMT_RGB32:
			pixel_deviation=(de_ifo->crop_ori_y-1)*(de_ifo->input_width)+(de_ifo->crop_ori_x-1)/4*4;
			inimage.InImageAdd=de_ifo->input_rgb_bus_addr+pixel_deviation*4;
			inimage.InImageMode=DE_RGB32;
			break;
		default:
			//printf("DE:Error input_pix_fmt [%d]\n",de_ifo->input_pix_fmt);
			break;		
	}
	inimage.InImageWidth=de_ifo->crop_width;
	inimage.InImageHeight=de_ifo->crop_height;
	switch(de_ifo->colorspace){
		case CSC_OFF:
			csc.SW=OFF;
			break;		
		case SDTV_SRGB:
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
			break;
		case SDTV_PCRGB:
			csc.a1=0x12a;
			csc.a2=0x12a;
			csc.b=0x199;
			csc.c=0xd0;
			csc.d=0x64;
			csc.e=0x204;
			csc.ysub16=1;
			csc.color_coef=0;
			csc.offset1=0;
			csc.offset2=0;
			csc.contrast1=0;
			csc.contrast2=0;
			csc.SW=ON;			
			break;
		case HDTV_SRGB:
			csc.a1=0x100;
			csc.a2=0x100;
			csc.b=0x18a;
			csc.c=0x75;
			csc.d=0x2e;
			csc.e=0x1d0;
			csc.ysub16=0;
			csc.color_coef=0;
			csc.offset1=0;
			csc.offset2=0;
			csc.contrast1=0;
			csc.contrast2=0;
			csc.SW=ON;			
			break;	
		case HDTV_PCRGB:
			csc.a1=0x12a;
			csc.a2=0x12a;
			csc.b=0x1cb;
			csc.c=0x88;
			csc.d=0x36;
			csc.e=0x21d;
			csc.ysub16=1;
			csc.color_coef=0;
			csc.offset1=0;
			csc.offset2=0;
			csc.contrast1=0;
			csc.contrast2=0;
			csc.SW=ON;			
			break;
		default:
			break;		
	}	
	
	if(de_ifo->colorspace!=CSC_NOCHANGE)
	{
		de_ifo->a=csc.a1;
		de_ifo->b=csc.b;	
		de_ifo->c=csc.c;
		de_ifo->d=csc.d;
		de_ifo->e=csc.e;
		de_ifo->ysub16=csc.ysub16;
		update_color_info(de_ifo,&csc);
		if(de_ifo->colorspace == CSC_OFF)
			csc.SW=OFF;
		CscSet(&csc);				
	}
	retval=ImageSet(&outimage,&inimage,&display);
	return retval;
}
	
static INT32S  de_init(DE_INFO *de_ifo)//must before lcd config
{
	DisplayAttribute display;
		INT32S  retval;

	DeReset(); 
	de_ifo_buffer.updateFlag=0;
	retval=de_cfg_update(de_ifo);
	
	/*DisplayDeviceSet*/
	display.Mode=de_ifo->Mode;
	display.DisplayWidth=de_ifo->screen_width;
	display.DisplayHeight=de_ifo->screen_height;
	display.DitherMode=de_ifo->DitherMode;
	display.GammaMode=de_ifo->GammaMode;
	display.Gamma=de_ifo->Gamma;
	retval=DisplayDeviceSet(&display);

	return retval;
}

INT32S  de_release(DE_INFO *de_ifo)
{
	DisplayClkDisable();
	return 0;
}


INT32S  de_set_osd(DE_INFO *de_ifo)
{

	#if ((CONFIG_AM_CHIP_ID == 1201)||(CONFIG_AM_CHIP_ID == 1205)||(CONFIG_AM_CHIP_ID == 1203))
	memcpy(&de_ifo_buffer,de_ifo,sizeof(DE_INFO));
	return 0;
	#endif
	#if ((CONFIG_AM_CHIP_ID == 1207)||(CONFIG_AM_CHIP_ID == 1211)||(CONFIG_AM_CHIP_ID == 1220)||(CONFIG_AM_CHIP_ID == 1213))
	OsdAttribute osd={de_ifo->osd_no,de_ifo->osd_sw,de_ifo->osd_bitmode,de_ifo->osd_alpha,de_ifo->osd_addr,de_ifo->osd_stride,de_ifo->osd_xstart,de_ifo->osd_ystart,de_ifo->osd_width,de_ifo->osd_height,de_ifo->TransColor};
	
	return OsdSet(&osd);	
	#endif

	de_ifo_buffer.updateFlag=1;
	return 0;
}

void WriteIndex(	INT32S  * pIndex,	INT32S  iIndexLen, 	INT32S  iOsdAdd){
		INT32S  i,total;
	total=iIndexLen/4;
	for(i=0;i<total;i++)	
			act_writel(pIndex[i],(	INT32U )(iOsdAdd+i*4)); //4bit	
}
void de_write_osdIndex(DE_INFO *de_ifo)
{
	WriteIndex(de_ifo->pIndex,de_ifo->iIndexLen,de_ifo->osd_addr);	
}

INT32S  de_get_status(DE_INFO *de_ifo)
{
	if(de_ifo_buffer.updateFlag==1)
		de_ifo->updateFlag=1;
	else
		de_ifo->updateFlag=0;
	de_ifo->input_luma_bus_addr=act_readl(Frm_BA);
	de_ifo->input_chroma_bus_addr=act_readl(UV_BA);

	return 0;
}
INT32S  de_get_osd_status(void)
{
	int ret=0;

	if(de_ifo_buffer.updateFlag==1)
		ret=1;

	return ret=0;
}
#define RGB2YUV(y,u,v,r,g,b)\
{\
	y = (((66 * r + 129 * g + 25 * b) + 128) >> 8) + 16;\
	u = (((-38 * r - 74 * g + 112 * b) + 128) >> 8) + 128;\
	v = (((112 * r - 94 * g - 18 * b) + 128) >> 8) + 128;\
}

#define YUV2RGB(r, g, b, y, u, v)\
{\
	int c,d,e;\
	c = y - 16;\
	d = u - 128;\
	e = v - 128;\
	r = MAX(0,MIN(255,(298 * c + 409 * e + 128) >> 8));\
	g = MAX(0,MIN(255,(298 * c - 100 * d - 208 * e + 128) >> 8));\
	b = MAX(0,MIN(255,(298 * c + 516 * d + 128) >> 8));\
}

#define ARGB_MUX(a,r,g,b)\
    (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

#define ARGB_DEMUX(v,a,r,g,b)\
{\
    a = (v) >> 24;\
    r = (v) >> 16;\
    g = (v) >> 8;\
    b = (v);\
}
//U16
#define RGB565_MUX(r,g,b)\
    (((r&0xf8) << 8) | ((g&0xfc) << 3) | (b>>3))

#define RGB565_DEMUX(v,r,g,b)\
{\
    r = (v&0xf800) >> 8;\
    g = (v&0xfe0) >> 3;\
    b = (v&0x1f)<<3;\
}

	
//void de_set_pallet(DE_INFO *de_ifo)
//{
//	int i;
//	unsigned char a,r,g,b,y,u,v;
//
//
//	if (OSDBIT8MODE == de_ifo->osd_bitmode)
//	{
//		if(de_ifo->colorspace==CSC_OFF)
//		{
//			for(i=0;i<256;i++)
//			{
//				ARGB_DEMUX(*(de_ifo->Pallet+i),a,r,g,b);	
//				RGB2YUV(y,u,v,r,g,b);
//				*(de_ifo->Pallet+i)=ARGB_MUX(a,y,u,v);
//			}
//		}
//		Pallet256Set(de_ifo->Pallet);
//	}
//	else
//	{
//		if(de_ifo->colorspace==CSC_OFF)
//		{
//			for(i=0;i<16;i++)
//			{
//				unsigned int val;
//				if(i%2==0)
//					val=RegBitRead((int)(de_ifo->Pallet+i/2),15,0);
//				else
//					val=RegBitRead((int)(de_ifo->Pallet+i/2),31,16);
//				RGB565_DEMUX(val,r,g,b);	
//				RGB2YUV(y,u,v,r,g,b);
//				val=RGB565_MUX(y,u,v);
//				if(i%2==0)
//					RegBitSet(val,(int)(de_ifo->Pallet+i/2),15,0);
//				else
//					RegBitSet(val,(int)(de_ifo->Pallet+i/2),31,16);
//			}
//		}
//		Pallet16Set(de_ifo->Pallet,de_ifo->TransColor);
//	}
//
//}

void de_set_pallet(DE_INFO *de_ifo)
{
	int i;
	unsigned char a,r,g,b,y,u,v;
	unsigned int pallet[256];
	
	for(i=0;i<256;i++)
		pallet[i]=*(de_ifo->Pallet+i);
		
	if(de_ifo->colorspace==CSC_OFF)
	{
		for(i=0;i<256;i++)
		{
			ARGB_DEMUX(pallet[i],a,r,g,b);	
			RGB2YUV(y,u,v,r,g,b);
			pallet[i]=ARGB_MUX(a,y,u,v);
		}
	}
	if (OSDBIT8MODE == de_ifo->osd_bitmode)
		Pallet256Set(pallet);
	else
		Pallet16Set(pallet,de_ifo->TransColor);
}
INT32S  de_change_frame_addr(DE_INFO *de_ifo)
{
		INT32S  pixel_deviation;
		INT32S  YAdd = 0 ;
		INT32S  CAdd = 0;
	switch(de_ifo->input_pix_fmt){
		case PIX_FMT_YCBCR_4_2_0_SEMIPLANAR:
			pixel_deviation=(de_ifo->crop_ori_y-1)/2*2*(de_ifo->input_width)+(de_ifo->crop_ori_x-1)/16*16;			
			YAdd=de_ifo->input_luma_bus_addr+pixel_deviation;
			CAdd=de_ifo->input_chroma_bus_addr+(de_ifo->crop_ori_y-1)/2*(de_ifo->input_width)+(de_ifo->crop_ori_x-1)/16*16;
			break;
		case PIX_FMT_YCBCR_4_2_2_INTERLEAVED:
			pixel_deviation=(de_ifo->crop_ori_y-1)*(de_ifo->input_width)+(de_ifo->crop_ori_x-1)/8*8;
			YAdd=de_ifo->input_luma_bus_addr+pixel_deviation*2;
			break;
		case PIX_FMT_RGB16_5_6_5:
			pixel_deviation=(de_ifo->crop_ori_y-1)*(de_ifo->input_width)+(de_ifo->crop_ori_x-1)/8*8;
			YAdd=de_ifo->input_rgb_bus_addr+pixel_deviation*2;
			break;
		case PIX_FMT_RGB32:
			pixel_deviation=(de_ifo->crop_ori_y-1)*(de_ifo->input_width)+(de_ifo->crop_ori_x-1)/4*4;
			YAdd=de_ifo->input_rgb_bus_addr+pixel_deviation*4;
			break;
		default:
			//printf("%s %d Error input_pix_fmt [%d]\n",__FILE__,__LINE__,de_ifo->input_pix_fmt);
			break;		
	}
	de_ifo_buffer.updateFlag=1;
	ChangeFrameAdd(YAdd,CAdd);
	return 0;
}


/********************************/
/*
letterbox：
16:9片源在4:3显示器上在上下加2个黑边
4:3的片源在16:9显示器上在左右加2个黑边

panscan：
16:9片源在4:3显示器上切掉左右两条
4:3的片源在16:9显示器上切掉上下两条
*/

INT32S  de_set_display_mode(DE_INFO *de_ifo){
		INT32U  display_ratio_x,display_ratio_y;
		INT32U  outwidth_ratio,outheight_ratio;	
		INT32U  outwidth,outheight;
		INT32U  status=0;
		INT32U  temp;
		INT32U  retval;
	
	if(de_ifo->par_width_screen==0)
		de_ifo->par_width_screen=1;
	if(de_ifo->par_height_screen==0)
		de_ifo->par_height_screen=1;	
	if(de_ifo->par_width_image==0)
		de_ifo->par_width_image=1;
	if(de_ifo->par_height_image==0)
		de_ifo->par_height_image=1;
	
	if ((de_ifo->input_width==0) || (de_ifo->input_height==0))
	{
		//printf("%s %d Error DE input Width[%d] & Height[%d]\n",__FILE__,__LINE__,de_ifo->input_width,de_ifo->input_height);
		return -1;
	}
	if ((de_ifo->screen_width==0) || (de_ifo->screen_height==0))
	{
		//printf("%s %d Error DE screen Width[%d] & Height[%d]\n",__FILE__,__LINE__,de_ifo->screen_width,de_ifo->screen_height);
		return -1;
	}
	de_ifo->input_width = de_ifo->input_width/8*8;
	if((de_ifo->dar_width_screen>0)&&(de_ifo->dar_height_screen>0)){
		display_ratio_x=de_ifo->dar_width_screen;
		display_ratio_y=de_ifo->dar_height_screen;
	}else if((de_ifo->dar_width_image>0)&&(de_ifo->dar_height_image>0)){
		display_ratio_x=de_ifo->dar_width_image;
		display_ratio_y=de_ifo->dar_height_image;
	}else{
		display_ratio_x=(de_ifo->par_width_image)*(de_ifo->input_width);
		display_ratio_y=(de_ifo->par_height_image)*(de_ifo->input_height);		
	}
	outwidth_ratio=32*display_ratio_x/(de_ifo->par_width_screen);
	outheight_ratio=32*display_ratio_y/(de_ifo->par_height_screen);
	if ((outwidth_ratio ==0) || (outheight_ratio==0))
	{
		//printf("%s %d Error DE ratio Width[%d] & Height[%d]\n",__FILE__,__LINE__,outwidth_ratio,outheight_ratio);
		return -1;
	}
	switch(de_ifo->display_mode){
		case E_DISPLAYMODE_PAN_AND_SCAN:
			if(de_ifo->screen_width*outheight_ratio<de_ifo->screen_height*outwidth_ratio){
				outheight=de_ifo->screen_height;
				outwidth=outheight*outwidth_ratio/outheight_ratio;
				if(outheight>=de_ifo->input_height*2){
					#if ((CONFIG_AM_CHIP_ID==1201) ||(CONFIG_AM_CHIP_ID == 1205))
					outheight=de_ifo->input_height*2-2;
					outwidth=outheight*outwidth_ratio/outheight_ratio;
					#else 
					;
					#endif
				}
				 if(de_ifo->input_height<outheight){
					if(de_ifo->input_width<outwidth){
						de_ifo->pip_ori_x=1;
						de_ifo->pip_ori_y=1;
						de_ifo->pip_frame_width=de_ifo->screen_width;
						de_ifo->pip_frame_height=de_ifo->screen_height;
						temp=(de_ifo->input_width)*(de_ifo->screen_width)/outwidth;
						de_ifo->crop_ori_x=(de_ifo->input_width-temp)/2+1;
						de_ifo->crop_ori_y=1;
						de_ifo->crop_width=temp;
						de_ifo->crop_height=de_ifo->input_height;					
					}else{
						status=1;
					}
				}else
					status=1;
			}else{				
				outwidth=de_ifo->screen_width;
				outheight=outwidth*outheight_ratio/outwidth_ratio;
				if(outheight>=de_ifo->input_height*2){
					#if ((CONFIG_AM_CHIP_ID==1201) ||(CONFIG_AM_CHIP_ID == 1205))
					outheight=de_ifo->input_height*2-2;
					outwidth=outheight*outwidth_ratio/outheight_ratio;
					#else
					;
					#endif
				}				
				 if(de_ifo->input_height<outheight){
					if(de_ifo->input_width<outwidth){
						de_ifo->pip_ori_x=1;
						de_ifo->pip_ori_y=1;
						de_ifo->pip_frame_width=de_ifo->screen_width;
						de_ifo->pip_frame_height=de_ifo->screen_height;
						temp=(de_ifo->input_height)*(de_ifo->screen_height)/outheight;
						de_ifo->crop_ori_x=1;
						de_ifo->crop_ori_y=(de_ifo->input_height-temp)/2+1;
						de_ifo->crop_width=de_ifo->input_width;
						de_ifo->crop_height=temp;		
										
					}else{
						status=1;
					}
				}else
					status=1;
			}
		break;
	case E_DISPLAYMODE_LETTER_BOX:
			if(de_ifo->screen_width*outheight_ratio<de_ifo->screen_height*outwidth_ratio){
				outwidth=de_ifo->screen_width;
				outheight=outwidth*outheight_ratio/outwidth_ratio;
				if(outheight>=de_ifo->input_height*2){
					#if ((CONFIG_AM_CHIP_ID==1201) ||(CONFIG_AM_CHIP_ID == 1205))
					outheight=de_ifo->input_height*2-2;
					outwidth=outheight*outwidth_ratio/outheight_ratio;
					#else
					;
					#endif
				}				
				if(de_ifo->input_height<outheight){
					if(de_ifo->input_width<outwidth){
						de_ifo->pip_ori_x=1;
						de_ifo->pip_ori_y=(de_ifo->screen_height-outheight)/2+1;
						de_ifo->pip_frame_width=outwidth;
						de_ifo->pip_frame_height=outheight;
						de_ifo->crop_ori_x=1;
						de_ifo->crop_ori_y=1;
						de_ifo->crop_width=de_ifo->input_width;
						de_ifo->crop_height=de_ifo->input_height;
					}else{
						status=1;
					}
				}else
					status=1;
			}else{				

				outheight=de_ifo->screen_height;
				outwidth=outheight*outwidth_ratio/outheight_ratio;
				if(outheight>=de_ifo->input_height*2){
					#if ((CONFIG_AM_CHIP_ID==1201) ||(CONFIG_AM_CHIP_ID == 1205))
					outheight=de_ifo->input_height*2-2;
					outwidth=outheight*outwidth_ratio/outheight_ratio;
					#else
					;
					#endif
				}				
				if(de_ifo->input_height<outheight){
					if(de_ifo->input_width<outwidth){
						de_ifo->pip_ori_x=(de_ifo->screen_width-outwidth)/2+1;
						de_ifo->pip_ori_y=1;
						de_ifo->pip_frame_width=outwidth;
						de_ifo->pip_frame_height=outheight;
						de_ifo->crop_ori_x=1;
						de_ifo->crop_ori_y=1;
						de_ifo->crop_width=de_ifo->input_width;
						de_ifo->crop_height=de_ifo->input_height;	

					}else{
						status=1;
					}
				}else
					status=1;
			}
		break;
	case E_DISPLAYMODE_FULL_SCREEN:
		#if ((CONFIG_AM_CHIP_ID == 1201)||(CONFIG_AM_CHIP_ID == 1205))
			if(de_ifo->input_height*2<=de_ifo->screen_height)
				status=1;
			else if(de_ifo->input_height<de_ifo->screen_height){
				if(de_ifo->input_width<de_ifo->screen_width){
					de_ifo->pip_ori_x=1;
					de_ifo->pip_ori_y=1;
					de_ifo->pip_frame_width=de_ifo->screen_width;
					de_ifo->pip_frame_height=de_ifo->screen_height;	
					de_ifo->crop_ori_x=1;
					de_ifo->crop_ori_y=1;
					de_ifo->crop_width=de_ifo->input_width;
					de_ifo->crop_height=de_ifo->input_height;					
				}else{
					status=1;
				}
			}else{
				status=1;
			}
		#else
			if((de_ifo->input_height<=de_ifo->screen_height)&&(de_ifo->input_width<=de_ifo->screen_width))
			{
				de_ifo->pip_ori_x=1;
				de_ifo->pip_ori_y=1;
				de_ifo->pip_frame_width=de_ifo->screen_width;
				de_ifo->pip_frame_height=de_ifo->screen_height;	
				de_ifo->crop_ori_x=1;
				de_ifo->crop_ori_y=1;
				de_ifo->crop_width=de_ifo->input_width;
				de_ifo->crop_height=de_ifo->input_height;
			}else{
				status=1;
			}
		#endif	
		break;		
	case E_DISPLAYMODE_ACTUAL_SIZE:
		status=1;
		break;		
	case E_DISPLAYMODE_DAR_SIZE:
		if (de_ifo->dar_width_screen > de_ifo->screen_width)
			de_ifo->dar_width_screen = de_ifo->screen_width;
		if (de_ifo->dar_height_screen > de_ifo->screen_height)
			de_ifo->dar_height_screen = de_ifo->screen_height;
		de_ifo->pip_ori_x = (de_ifo->screen_width-de_ifo->dar_width_screen)/2;
		de_ifo->pip_ori_y = (de_ifo->screen_height-de_ifo->dar_height_screen)/2;
		de_ifo->pip_frame_width = de_ifo->dar_width_screen;
		de_ifo->pip_frame_height = de_ifo->dar_height_screen;
		if  (de_ifo->pip_ori_x <1)  de_ifo->pip_ori_x=1;
		if  (de_ifo->pip_ori_x > de_ifo->screen_width)  de_ifo->pip_ori_x=de_ifo->screen_width;
		if  (de_ifo->pip_ori_y <1)  de_ifo->pip_ori_y=1;
		if  (de_ifo->pip_ori_y > de_ifo->screen_height)  de_ifo->pip_ori_y=de_ifo->screen_height;
		if  ((de_ifo->pip_ori_x+ de_ifo->pip_frame_width) > (de_ifo->screen_width +1))  
			de_ifo->pip_frame_width=(de_ifo->screen_width -de_ifo->pip_ori_x +1);
		if  ((de_ifo->pip_ori_y+ de_ifo->pip_frame_height) > (de_ifo->screen_height +1))  
			de_ifo->pip_frame_height=(de_ifo->screen_height -de_ifo->pip_ori_y +1);

		if  (de_ifo->crop_ori_x <1)  de_ifo->crop_ori_x=1;
		if  (de_ifo->crop_ori_x > de_ifo->input_width)  de_ifo->crop_ori_x=de_ifo->input_width;
		if  (de_ifo->crop_ori_y <1)  de_ifo->crop_ori_y=1;
		if  (de_ifo->crop_ori_y > de_ifo->input_height)  de_ifo->crop_ori_y=de_ifo->input_height;
		if  ((de_ifo->crop_ori_x+ de_ifo->crop_width) > (de_ifo->input_width +1))  
			de_ifo->crop_width=(de_ifo->input_width -de_ifo->crop_ori_x +1);
		if  ((de_ifo->crop_ori_y+ de_ifo->crop_height) > (de_ifo->input_height +1))  
			de_ifo->crop_height=(de_ifo->input_height -de_ifo->crop_ori_y +1);
		status=0;
		printk("de_ifo->dar_width_screen=%d de_ifo->dar_height_screen=%d %d %d %d %d\n",de_ifo->dar_width_screen,de_ifo->dar_height_screen,de_ifo->pip_ori_x,de_ifo->pip_ori_y,de_ifo->pip_frame_width,de_ifo->pip_frame_height);
		printk("%d %d %d %d\n",de_ifo->crop_ori_x,de_ifo->crop_ori_y,de_ifo->crop_width,de_ifo->crop_height);
		printk("%d %d\n",de_ifo->screen_width,de_ifo->screen_height);
		break;	
	}
	if(status==1){
		/*
		if(de_ifo->input_width*outheight_ratio<de_ifo->input_height*outwidth_ratio){
			outheight=de_ifo->input_height+1;
			outwidth=outheight*outwidth_ratio/outheight_ratio;
		}
		else{
			outwidth=de_ifo->input_width+1;
			outheight=outwidth*outheight_ratio/outwidth_ratio;	
		}
		*/
		if(de_ifo->input_width*outheight_ratio<de_ifo->input_height*outwidth_ratio){
			outheight=de_ifo->input_height;
			outwidth=outheight*outwidth_ratio/outheight_ratio;
			if(outwidth>de_ifo->input_width)
			{
				outheight=de_ifo->input_height+1;
				outwidth=outheight*outwidth_ratio/outheight_ratio;
			}
		}
		else{
			outwidth=de_ifo->input_width;
			outheight=outwidth*outheight_ratio/outwidth_ratio;	
			if(outheight>de_ifo->input_height)
			{
				outwidth=de_ifo->input_width+1;
				outheight=outwidth*outheight_ratio/outwidth_ratio;
			}
		}
		if(outwidth<de_ifo->screen_width){
			de_ifo->pip_ori_x=(de_ifo->screen_width-outwidth)/2+1;
			de_ifo->pip_frame_width=outwidth;
			de_ifo->crop_ori_x=1;
			de_ifo->crop_width=de_ifo->input_width;
		}else{
			de_ifo->pip_ori_x=1;
			de_ifo->pip_frame_width=de_ifo->screen_width;
			temp=(de_ifo->screen_width)*(de_ifo->input_width)/outwidth;
			de_ifo->crop_ori_x=(de_ifo->input_width-temp)/2+1;
			de_ifo->crop_width=temp;				
		}
		if(outheight<de_ifo->screen_height){
			de_ifo->pip_ori_y=(de_ifo->screen_height-outheight)/2+1;
			de_ifo->pip_frame_height=outheight;
			de_ifo->crop_ori_y=1;
			de_ifo->crop_height=de_ifo->input_height;
		}else{
			de_ifo->pip_ori_y=1;
			de_ifo->pip_frame_height=de_ifo->screen_height;
			temp=(de_ifo->screen_height)*(de_ifo->input_height)/outheight;
			de_ifo->crop_ori_y=(de_ifo->input_height-temp)/2+1;
			de_ifo->crop_height=temp;				
		}										
	}
	
	//de_ifo->colorspace=CSC_NOCHANGE;
	retval=de_cfg_update(de_ifo);
	return retval;	
}


void flush_cache(void)
{
	int start = 0,size = 1920*1080*4; 
	if(RegBitRead(DE_Con,9,9)==1){  //flush de buffer if possible
		//flush main framebuffer 0
		start = act_readl(Frm_BA)&0x0fffffff;  //low 28bits
		size = 1920*1080*4;  //to max 
		dma_cache_wback_inv(start,size);
		if((act_readl(UV_BA)&0x0fffffff) !=start){  //flush  uv framebuffer if possible
			start = (act_readl(UV_BA)&0x0fffffff);	
			dma_cache_wback_inv(start,size);
		}
	}
	
	if(RegBitRead(OSD0con,19,19) == 1) //flush osd0 buffer if possible
	{
		start = act_readl(OSD0_BA);
		dma_cache_wback_inv(start,size);
	}
	
	if(RegBitRead(OSD1con,19,19) == 1) //flush osd1 buffer if possible
	{
		start = act_readl(OSD1_BA);
		dma_cache_wback_inv(start,size);
	}	

}
INT32S  de_set_cursor(DE_INFO *de_ifo)
{
	CursorAttribute cursor;
	
	cursor.SW=de_ifo->cursor_sw;
	cursor.x=de_ifo->cursor_x;	
	cursor.y=de_ifo->cursor_y;
	CursorSet(&cursor);
	return 0;
}

INT32S de_set(INT32S cmd,DE_INFO *de_ifo)
{
	INT32S ret=0;
	
//	dma_cache_wback_inv(0, 0x1fffffff);
	flush_cache();
	switch(cmd)
	{
	case DE_INIT:
		ret=de_init(de_ifo);
		break;
	case DE_RELEASE:
		ret=de_release(de_ifo);
		break;
	case UPDATE_DE_CONFIG:
		ret=de_cfg_update(de_ifo);
		break;
	case COLOR_ADJUST:
		ret=de_color_adjust(de_ifo);
		break;		
	case GAMMA_ADJUST:
		ret=de_gamma_adjust(de_ifo);
		break;
	case SET_DISPLAY_MODE:
		ret=de_set_display_mode(de_ifo);
		break;	
	case CHANGE_FRAME_ADDR:
		ret=de_change_frame_addr(de_ifo);
		break;
	case SET_OSD:
		ret=de_set_osd(de_ifo);
		break;
	case WRITE_OSDINDEX:
		de_write_osdIndex(de_ifo);
		break;	
	case SET_PALLET:
		de_set_pallet(de_ifo);
		break;
	case SET_PALLET256:
		//de_set_pallet256(de_ifo);
		break;
	case SHARPNESS_ADJUST:  //sharpness adjust
		de_sharpness_adjust(de_ifo);
		break;
	case SET_CURSOR:  //cursor set
		de_set_cursor(de_ifo);
		break;		
		
	}	
	return -ret;	
}
