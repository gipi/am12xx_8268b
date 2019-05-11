#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <wire_heap.h>

#include "lcm_op.h"
#include "wire_osd.h"
#include "wire_log.h"

#define LCM_DEV_NAME  "/dev/lcm"
#define __get_byte(which_pix,bpp)	((which_pix)/(bpp))

/**
* for 2bits osd the palette color in RGB565 formate
*/
#define OSD_2BITS_ALPHA_COLOR 0
#define OSD_2BITS_COLOR_1 0xffff
#define OSD_2BITS_COLOR_2 0x821
#define OSD_2BITS_COLOR_3 0xf800

struct osd_icon_header
{
	char	Identification[4];          // 始终为 "AIMG"
	int    dwSize;                      // 文件总长度
	unsigned short wWidth;
	unsigned short wHeight;
	char		 MainFmt;               //  0. YUV,  1. RGB
	char		 SubFmt;                //  YUV (0. yuv420,1. yuv422,2. yuv444 ; RGB 则表示位深度。
	unsigned short	Reserved;
};

struct osd_palette_header
{
	char Identification[4];  // 始终为 “APLT”
	int dwSize;     // 文件总长度
	char nBitCount;  // 位深度
	char nFormat ;  // 0. RGB565 (16 bit)   1. RGB888 (24bit)    2. ARGB (32 bit) 
	char Reserved[6];
};

struct osd_engine_info
{
	unsigned int stride;            /// stride for osd buffer in pixels.
	unsigned int formate;           /// formate for osd (rgb565/8 bits/4 bits etc.).
	int x;                          /// the x position for the osd to shown to the screen.
	int y;                          /// the y position for the osd to shown to the screen.
	int w;                          /// the width for the osd in pixels.
	int h;                          /// the height for the osd in pixels.
	unsigned int transcolor;        /// the transparent color of the osd. 
	int alpha;                      /// alpha for the osd.
	unsigned long v_render_addr;    /// the virtual address for the osd render buffer.
	unsigned long p_render_addr;    /// the physical address for the osd render buffer.
	unsigned long v_show_addr;      /// the virtual address for the osd show buffer.
	unsigned long p_show_addr;      /// the physical address for the osd show buffer.
	int valid;                      /// the info is valid or not.
	void *dev;                      /// the de inst.
	unsigned int palette[256];
};
struct osd_engine_info osdinfo;
static void *p_heap = NULL;

DE_result osd_de_init(void* *inst)
{
	DE_inst* handle;
	DE_config* conf;
	DE_INFO info;
	unsigned int i;


	handle = (DE_inst *)OSmalloc(sizeof(DE_inst));
	if(handle == NULL)
	{
		printf("de:can't get mem\n");
		return DE_MEMFAIL;
	}else{
		*inst=handle;
		conf=&(handle->config);
		memset(handle,0,sizeof(DE_inst));
		if((handle->fd=open(LCM_DEV_NAME,O_RDWR))<0)
			fprintf(stderr,"open %s error:%d\n",LCM_DEV_NAME,handle->fd);
#if 1
		lcm_get_config(*inst,&info);
		conf->output.display_mode=DE_DISPLAYMODE_LETTER_BOX;
		conf->input.pix_fmt=info.input_pix_fmt;
		conf->input.video_range=info.colorspace;
		conf->output.display_mode=info.display_mode;
		conf->input.width=info.input_width;
		conf->input.height=info.input_height;
		conf->input.height=info.input_height;
		conf->input.default_color = info.DefaultColor;
		conf->output.contrast=info.contrast;		
		conf->output.brightness=info.brightness;
		conf->output.saturation=info.saturation;		
		conf->output.hue=info.hue;
		conf->output.sharpness=info.sharpness;
		conf->cursor.enable=info.cursor_sw;
		conf->cursor.x=info.cursor_x;
		conf->cursor.y=info.cursor_y;
		if(info.DisplaySource==DE_IMAGE)
			conf->input.enable=1;
		else
			conf->input.enable=0;
		for(i=0;i<MAX_DEV_NUM;i++)
		{
			if(i==info.screen_type)
			{
				conf->dev_info[i].enable=1;
				conf->dev_info[i].width=info.screen_width;
				conf->dev_info[i].height=info.screen_height;
				conf->dev_info[i].par_width=info.par_width_screen;
				conf->dev_info[i].par_height=info.par_height_screen;
			}else
				memset(&(conf->dev_info[i]),0,sizeof(DE_dev_info));
		}
#endif


		return DE_OK;
	}

}

int lcm_get_config(DE_inst* inst, DE_INFO *cfg)
{
	int fd;
	
	fd=inst->fd;
	ioctl(fd,GET_DE_CONFIG,cfg);
	return 0;
}

static void de_get_display_status(DE_inst* inst ,DE_config *conf)
{
	DE_INFO info;
	unsigned int i;

	lcm_get_config(inst,&info);
	conf->input.bus_addr=info.input_luma_bus_addr;
	conf->input.bus_addr_uv=info.input_chroma_bus_addr;
	conf->input.width=info.input_width;
	conf->input.height=info.input_height;
	conf->input.video_range = info.colorspace;
	if(info.osd_no==0)
		conf->osd_input1.bus_addr=info.osd_addr;
	else if(info.osd_no==1)
		conf->osd_input2.bus_addr=info.osd_addr;	
	conf->de_status=info.updateFlag;
	for(i=0;i<MAX_DEV_NUM;i++)
	{
		if(i==info.screen_type)
		{
			conf->dev_info[i].enable=1;
			conf->dev_info[i].width=info.screen_width;
			conf->dev_info[i].height=info.screen_height;
			conf->dev_info[i].par_width=info.par_width_screen;
			conf->dev_info[i].par_height=info.par_height_screen;
		}else
			memset(&(conf->dev_info[i]),0,sizeof(DE_dev_info));
	}
}

DE_result osd_de_get_config(void* inst,DE_config *conf, int flg)
{
	int i;
	DE_inst * p_inst=(DE_inst *)inst;

	//update_config(&((DE_inst *)inst)->config);
	de_get_display_status(p_inst,&(p_inst->config));

	switch(flg)
	{
	case DE_CFG_IN:
		memcpy(&(conf->input),&(p_inst->config.input),sizeof(DE_input));
		break;
	case DE_CFG_OUT:
			memcpy(&(conf->output),&(p_inst->config.output),sizeof(DE_output)-sizeof(conf->output.gamma));
		break;		
	case DE_CFG_DEVIFO:
		for(i=0;i<MAX_DEV_NUM;i++)
			memcpy(&(conf->dev_info[i]),&(p_inst->config.dev_info[i]),sizeof(DE_dev_info));
		break;
	case DE_CFG_OSD1:
			memcpy(&(conf->osd_input1),&(p_inst->config.osd_input1),sizeof(DE_OSD_input)-sizeof(conf->osd_input1.index));
			memcpy(&(conf->osd_output1),&(p_inst->config.osd_output1),sizeof(DE_OSD_output));
		break;
	case DE_CFG_OSD2:
			memcpy(&(conf->osd_input2),&(p_inst->config.osd_input2),sizeof(DE_OSD_input)-sizeof(conf->osd_input1.index));
			memcpy(&(conf->osd_output2),&(p_inst->config.osd_output2),sizeof(DE_OSD_output));
		break;
	case DE_CFG_OSD_IDX:
		memcpy(&(conf->osd_input1.index),&(p_inst->config.osd_input1.index),sizeof(conf->osd_input1.index));
		memcpy(&(conf->osd_input2.index),&(p_inst->config.osd_input2.index),sizeof(conf->osd_input1.index));
		break;
	case DE_CFG_GAMMA:
		memcpy(&(conf->output.gamma),&(p_inst->config.output.gamma),sizeof(conf->output.gamma));
		break;	
	case DE_CFG_ALL:
		memcpy(conf,&(p_inst->config),sizeof(DE_config));
		break;	
	case DE_CFG_CURSOR:
		memcpy(&(conf->cursor),&(p_inst->config.cursor),sizeof(DE_CURSOR));
		break;				
	}

	return  DE_OK;
}

static void  pallet565topallet888(unsigned int* pallet888,unsigned int pallet565,unsigned int transcolor)
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

static int get_osd_status(DE_inst* inst)
{
	DE_INFO info;
	int status_osd0,status_osd1;
	int ret;
	
	info.osd_no=0;
	lcm_get_config(inst,&info);
	status_osd0=info.osd_sw;
	info.osd_no=1;
	lcm_get_config(inst,&info);
	status_osd1=info.osd_sw;
	
	if((status_osd0==1)||(status_osd1==1))	
		ret=1;
	else
		ret=0;
	return ret;
}

DE_result osd_de_set_Config(void* inst,DE_config *conf, int flg)
{
	DE_INFO info;
	int ret=0;
	int i;
	DE_inst * p_inst=(DE_inst *)inst;
	DE_OSD_input* osd_input[2]={&(conf->osd_input1),&(conf->osd_input2)};
	DE_OSD_output* osd_output[2]={&(conf->osd_output1),&(conf->osd_output2)};
    int osd_status;
	static int initial=0;
	int fd;
	
	fd=p_inst->fd;		
	
	#if 0
	printf("conf->input.width=%x\n",conf->input.width);	
	printf("conf->input.height=%x\n",conf->input.height);	
	printf("conf->input.video_range=%x\n",conf->input.video_range);
	printf("conf->output.display_mode=%x\n",conf->output.display_mode);
	i=0;
	printf("conf->dev_info[0].width=%x\n",conf->dev_info[i].width);	
	printf("conf->dev_info[0].height=%x\n",conf->dev_info[i].height);			
	#endif



	lcm_get_config(p_inst,&info);
	switch(flg)
	{
	case DE_CFG_IN:
		info.input_pix_fmt=conf->input.pix_fmt;
		if ((info.screen_type == DE_OUT_DEV_ID_LCD) || (info.screen_type == DE_OUT_DEV_ID_HDMI) || (info.screen_type == DE_OUT_DEV_ID_VGA))
		{
			switch (info.input_pix_fmt)
			{
				case PIX_FMT_RGB32:
				case PIX_FMT_BGR32:
				case PIX_FMT_RGB16_5_5_5:
				case PIX_FMT_BGR16_5_5_5:
				case PIX_FMT_RGB16_5_6_5:
				case PIX_FMT_BGR16_5_6_5:
				case PIX_FMT_RGB16_CUSTOM:
				case PIX_FMT_RGB32_CUSTOM:
					info.colorspace = CSC_OFF;
					break;
				case PIX_FMT_YCBCR_4_0_0:
				case PIX_FMT_YCBCR_4_2_0_PLANAR:
				case PIX_FMT_YCBCR_4_2_0_SEMIPLANAR:
				case PIX_FMT_YCBCR_4_2_0_TILED:
				case PIX_FMT_YCBCR_4_2_2_INTERLEAVED:
				case PIX_FMT_YCBCR_4_2_2_SEMIPLANAR:
				case PIX_FMT_YCBCR_4_4_0_SEMIPLANAR:
				case PIX_FMT_YCBCR_4_4_4_SEMIPLANAR:	
					info.colorspace = SDTV_SRGB;
					break;
				default:
					info.colorspace = SDTV_SRGB;
					break;
			}
		}
		else if ((info.screen_type == DE_OUT_DEV_ID_YPBPR) || (info.screen_type == DE_OUT_DEV_ID_CVBS))
		{
			info.colorspace = CSC_OFF;
		}
		info.display_mode=conf->output.display_mode;
		info.input_width=conf->input.width/8*8;
		info.input_height=conf->input.height;
		if(conf->input.enable==1)
			info.DisplaySource=DE_IMAGE;
		else
			info.DisplaySource=DE_DEFAULTCOLOR;
		info.DefaultColor=conf->input.default_color;
		info.input_luma_bus_addr=conf->input.bus_addr;
		info.input_rgb_bus_addr=conf->input.bus_addr;
		info.input_chroma_bus_addr=conf->input.bus_addr_uv;
		ioctl(fd,SET_DISPLAY_MODE,&info);
		memcpy(&(p_inst->config.input),&(conf->input),sizeof(DE_input));	
		break;
	case DE_CFG_OUT:
		break;		
	case DE_CFG_DEVIFO:
		break;
	case DE_CFG_OSD1:
	case DE_CFG_OSD2:
		 if(flg==DE_CFG_OSD1)
		 {
		 	i=0;
	 		memcpy(&(p_inst->config.osd_input1),&(conf->osd_input1),sizeof(DE_OSD_input));
			memcpy(&(p_inst->config.osd_output1),&(conf->osd_output1),sizeof(DE_OSD_output));	
		 }else
		 {
		 	i=1;
			memcpy(&(p_inst->config.osd_input2),&(conf->osd_input2),sizeof(DE_OSD_input));
			memcpy(&(p_inst->config.osd_output2),&(conf->osd_output2),sizeof(DE_OSD_output));
		 }	
		#ifdef OSD16BIT_2_OSD8BIT

		if(initial==0)
		{
			int width=1920;
			int height=200;
			buffer_1.size=width*height;
			OSBufMalloc(&buffer_1);
			buffer_2.size=width*height;
			OSBufMalloc(&buffer_2);
			initial=1;
		}
		if((osd_output[i]->width>128)&&(osd_input[i]->pix_fmt=DE_PIX_FMT_OSDBIT16MODE))
		{
			DE_config conf_t;
			OS_BUF* idle_buf;
			
			OSmemcpy(&conf_t,conf,sizeof(DE_config));
			if(i==0)
				osd_input[i]=&(conf_t.osd_input1);
			else
				osd_input[i]=&(conf_t.osd_input2);
			if(i==0)
				osd_output[i]=&(conf_t.osd_output1);
			else
				osd_output[i]=&(conf_t.osd_output2);
				
			idle_buf=select_idle_buffer();
			if(idle_buf->vir_addr!=0)
			{
				rgb565_to_idx8_2((unsigned char *)(osd_input[i]->img), 
								osd_input[i]->stride*2, 
								osd_output[i]->width, 
								osd_output[i]->height, 
								(unsigned char *)(idle_buf->vir_addr), 
								osd_output[i]->width,
								(unsigned short*)(osd_input[i]->index),
								osd_input[i]->tparent_color);
				osd_input[i]->pix_fmt=DE_PIX_FMT_OSDBIT8MODE;
				osd_input[i]->bus_addr=idle_buf->phy_addr;
				osd_input[i]->idx_fmt=DE_IDX_FMT_RGB16;
				//OSprintf("OSD16BIT_2_OSD8BIT\n");
			}else
				OSprintf("OSD16BIT_2_OSD8BIT mem malloc error\n");
			
		}
		#endif	 			
		info.osd_no=i;
		info.osd_sw=osd_input[i]->enable;
		if(osd_input[i]->pix_fmt==DE_PIX_FMT_OSDBIT1MODE)
			info.osd_bitmode=OSDBIT1MODE;
		else if(osd_input[i]->pix_fmt==DE_PIX_FMT_OSDBIT2MODE)
			info.osd_bitmode=OSDBIT2MODE;
		else if(osd_input[i]->pix_fmt==DE_PIX_FMT_OSDBIT4MODE)
			info.osd_bitmode=OSDBIT4MODE;
		else if(osd_input[i]->pix_fmt==DE_PIX_FMT_OSDBIT8MODE)
			info.osd_bitmode=OSDBIT8MODE;
		else if(osd_input[i]->pix_fmt==DE_PIX_FMT_OSDBIT16MODE)	
			info.osd_bitmode=OSDBIT16MODE;
		else if(osd_input[i]->pix_fmt==DE_PIX_FMT_OSDBIT32MODE)
			info.osd_bitmode=OSDBIT32MODE;
		info.TransColor=osd_input[i]->tparent_color;
		info.osd_alpha=osd_output[i]->alpha;
		info.osd_addr=osd_input[i]->bus_addr;
		info.osd_stride=osd_input[i]->stride;
		info.osd_xstart=osd_output[i]->pip_x;
		info.osd_ystart=osd_output[i]->pip_y;
		if(info.osd_xstart==0)
			info.osd_xstart=1;
		if(info.osd_ystart==0)
			info.osd_ystart=1;
		info.osd_width=osd_output[i]->width;
		info.osd_height=osd_output[i]->height;
		if((osd_input[i]->pix_fmt==DE_PIX_FMT_OSDBIT1MODE)\
			||(osd_input[i]->pix_fmt==DE_PIX_FMT_OSDBIT2MODE)\
			||(osd_input[i]->pix_fmt==DE_PIX_FMT_OSDBIT4MODE)\
			||(osd_input[i]->pix_fmt==DE_PIX_FMT_OSDBIT8MODE))
		{
			if(osd_input[i]->idx_fmt==DE_IDX_FMT_RGB32){
				info.Pallet=osd_input[i]->index;				
			}else if(osd_input[i]->idx_fmt==DE_IDX_FMT_RGB16)
			{
				unsigned int pallet888[256];
				int j;
				unsigned int pallet565;
				
				for(j=0;j<256;j++)
				{
					pallet565=(*(((INT16U*)(osd_input[i]->index))+j))&0xffff;
					pallet565topallet888(&(pallet888[j]),pallet565,osd_input[i]->tparent_color);
				}	
				info.Pallet=pallet888;	
			}
			ioctl(fd,SET_PALLET,&info);
		}			
		osd_status=get_osd_status(p_inst);
		if((osd_status==0)&&(info.osd_sw==1))
		{		
			//enable_2d_task(1);
		}
		ioctl(fd,SET_OSD,&info);
		if((osd_status==1)&&(info.osd_sw==0))
		{
			//enable_2d_task(0);
		}
		
		break;		
	case DE_CFG_OSD_IDX:
		break;
	case DE_CFG_GAMMA:
		break;	
	case DE_CFG_ALL:
	//	ds_change_devide(p_inst,conf);   // fix 
		memcpy((p_inst->config).dev_info,(conf->dev_info),sizeof(DE_dev_info)*MAX_DEV_NUM);
		info.input_pix_fmt=conf->input.pix_fmt;
		if(info.colorspace!=0)
			info.colorspace=conf->input.video_range;
		info.display_mode=conf->output.display_mode;
		info.input_width=conf->input.width/8*8;
		info.input_height=conf->input.height;
		//info.dar_width_image=conf->output.dar_width;
		//info.dar_height_image=conf->output.dar_height;
		//info.par_width_image=conf->input.dar_width;
		//info.par_height_image=conf->output.dar_height;
		info.dar_width_screen=conf->output.dar_width;
		info.dar_height_screen=conf->output.dar_height;
#if 0
		for(i=0;i<MAX_DEV_NUM;i++)
		{
			if(conf->dev_info[i].enable==1)
			{
				//info.screen_width=conf->dev_info[i].width;
				//info.screen_height=conf->dev_info[i].height;
				//info.par_width_screen=conf->dev_info[i].par_width;
				//info.par_height_screen=conf->dev_info[i].par_height;
				if(i==DE_OUT_DEV_ID_CVBS)
					info.colorspace=CSC_OFF;
				else 
				{
					if(conf->input.video_range!=0)
						info.colorspace=conf->input.video_range;
					else
						info.colorspace=DE_SDTV_SRGB;
				}
				break;
			}
		}
#endif
		if(conf->input.enable==1)
			info.DisplaySource=DE_IMAGE;
		else
			info.DisplaySource=DE_DEFAULTCOLOR;
		info.DefaultColor=conf->input.default_color;
		//info.Mode=;
		//info.DitherMode=
		//info.GammaMode=0;
		//info.Gamma=GAMMA_OFF
		info.input_luma_bus_addr=conf->input.bus_addr;
		info.input_rgb_bus_addr=conf->input.bus_addr;
		info.input_chroma_bus_addr=conf->input.bus_addr_uv;
		ioctl(fd,SET_DISPLAY_MODE,&info);
		info.contrast=conf->output.contrast;		
		info.brightness=conf->output.brightness;
		info.saturation=conf->output.saturation;		
		info.hue=conf->output.hue;
		info.sharpness=conf->output.sharpness;
		if(info.colorspace!=CSC_OFF)
		{
			lcm_get_config(p_inst,&info);
			ioctl(fd,COLOR_ADJUST,&info);
		}
	//	info.GammaMode=conf->output.gamma_enable;		
	//	info.Gamma=conf->output.gamma;
	//	ioctl(fd,GAMMA_ADJUST,&info);
		memcpy(&(p_inst->config.input),&(conf->input),sizeof(DE_input));
		memcpy(&(p_inst->config.output),&(conf->output),sizeof(DE_output));

		break;			
	case DE_CFG_INPUT_ADD_UNBLOCK:
		info.input_luma_bus_addr=conf->input.bus_addr;
		info.input_chroma_bus_addr=conf->input.bus_addr_uv;
		info.input_rgb_bus_addr=conf->input.bus_addr;
		ioctl(fd,CHANGE_FRAME_ADDR,&info);
		break;
	case DE_CFG_CURSOR:
		info.cursor_sw=conf->cursor.enable;
		info.cursor_x=conf->cursor.x;
		info.cursor_y=conf->cursor.y;
		ioctl(fd,SET_CURSOR,&info);
		break;			
	}

	return  DE_OK;
}


int osdengine_enable_osd()
{
	DE_config ds_conf;

	if(!osdinfo.valid){
		return -1;
	}

	osd_de_get_config(osdinfo.dev,&ds_conf,DE_CFG_OSD1);
	
	ds_conf.osd_input1.enable = 1;
	ds_conf.osd_input1.stride = osdinfo.stride;
	ds_conf.osd_input1.pix_fmt= osdinfo.formate;
	ds_conf.osd_input1.img = (unsigned long *)osdinfo.v_show_addr;
	ds_conf.osd_input1.bus_addr = osdinfo.p_show_addr;
	ds_conf.osd_input1.tparent_color = osdinfo.transcolor;
	ds_conf.osd_input1.idx_fmt = DE_IDX_FMT_RGB16;
	memcpy((void *)ds_conf.osd_input1.index,(void *)osdinfo.palette,sizeof(unsigned int)*256);

	ds_conf.osd_output1.alpha = osdinfo.alpha;
	ds_conf.osd_output1.pip_x = osdinfo.x+1;
	ds_conf.osd_output1.pip_y = osdinfo.y+1;
	ds_conf.osd_output1.width = osdinfo.w;
	ds_conf.osd_output1.height = osdinfo.h;
	osd_de_set_Config(osdinfo.dev,&ds_conf,DE_CFG_OSD1);

	return 0;
	
}

int osdengine_disable_osd()
{
	DE_config ds_conf;

	if(!osdinfo.valid){
		return -1;
	}
	
	osd_de_get_config(osdinfo.dev,&ds_conf,DE_CFG_OSD1);
#if 1
	ds_conf.osd_input1.enable = 0;
	ds_conf.osd_input1.stride = osdinfo.stride;
	ds_conf.osd_input1.pix_fmt= osdinfo.formate;
	ds_conf.osd_input1.img = (unsigned long *)osdinfo.v_show_addr;
	ds_conf.osd_input1.bus_addr = osdinfo.p_show_addr;
	ds_conf.osd_input1.tparent_color = osdinfo.transcolor;
	ds_conf.osd_input1.idx_fmt = DE_IDX_FMT_RGB16;
	memcpy((void *)ds_conf.osd_input1.index,(void *)osdinfo.palette,sizeof(unsigned int)*256);

	ds_conf.osd_output1.alpha = osdinfo.alpha;
	ds_conf.osd_output1.pip_x = osdinfo.x+1;
	ds_conf.osd_output1.pip_y = osdinfo.y+1;
	ds_conf.osd_output1.width = osdinfo.w;
	ds_conf.osd_output1.height = osdinfo.h;
#else	
	ds_conf.osd_input1.enable = 0;
#endif


	osd_de_set_Config(osdinfo.dev,&ds_conf,DE_CFG_OSD1);	

	return 0;
}

int osdengine_init_osd(int x,int y,int w,int h,int mode,char *palettefile)
{
	int buffersize;
	int bpp;
	FILE *fp;
	struct osd_palette_header ph;
	unsigned short *p;
	//EZCASTLOG("palettefile: %s, [%d:%d:%d:%d], mode: %d\n", palettefile, x, y, w, h, mode);

	memset(&osdinfo,0,sizeof(struct osd_engine_info));
	
	/// We only support 4 bit mode osd currently.
	if(mode == DE_PIX_FMT_OSDBIT4MODE){
		if((w/2)%32 != 0){
			printf("[video_osd failed]:width in bytes must be aligned with 32\n");
			return -1;
		}
		bpp = 2;
		buffersize = w*h/bpp;

		/// read the palette.
		fp = fopen(palettefile,"rb");
		if(fp){
			fread(&ph, sizeof(struct osd_palette_header), 1, fp);
			fread(osdinfo.palette, ph.dwSize-sizeof(struct osd_palette_header), 1, fp);
			fclose(fp);
		}
		else{
			printf("[video_osd failed]:palette init error,%s\n",palettefile);
			return -1;
		}
		
	}
	else if(mode == DE_PIX_FMT_OSDBIT2MODE){
		if((w/4)%16 != 0){
			printf("[video_osd failed]: for 2bits osd width in bytes must be aligned with 64\n");
			return -1;
		}
		bpp = 4;
		buffersize = w*h/bpp;
		
		memset(osdinfo.palette,0,sizeof(osdinfo.palette));
		p = (unsigned short *)(&osdinfo.palette[0]);
		*p = OSD_2BITS_ALPHA_COLOR;
		*(p+1) = OSD_2BITS_COLOR_1;
		*(p+2) = OSD_2BITS_COLOR_2;
		*(p+3) = OSD_2BITS_COLOR_3;
	}
	else if(mode == DE_PIX_FMT_OSDBIT8MODE){
		bpp = 1;
		buffersize = w*h/bpp;
		
		/// read the palette.
		fp = fopen(palettefile,"rb");
		if(fp){
			fread(&ph, sizeof(struct osd_palette_header), 1, fp);
			fread(osdinfo.palette, ph.dwSize-sizeof(struct osd_palette_header), 1, fp);
			fclose(fp);
		}
		else{
			printf("[video_osd failed]:palette init error,%s\n",palettefile);
			return -1;
		}
	}
	else{
		printf("[video_osd failed]:error osd mode[2,4,8 bits valid only].\n");
		return -1;
	}

	if(x<0 || y<0){
		printf("[video_osd failed]:position error\n");
		return -1;
	}

	p_heap = (void *)wire_MemoryInit_r(4*buffersize);
	if(!p_heap){
		printf("[%s]%d, swf_malloc fail\n", __func__, __LINE__);
		return -1;
	}

	osdinfo.v_render_addr = (unsigned long)OSHmalloc(p_heap, buffersize, &(osdinfo.p_render_addr));
	if(osdinfo.v_render_addr == 0){
		printf("[video_osd failed]:render buffer alloc error\n");
		return -1;
	}

	osdinfo.v_show_addr = (unsigned long)(unsigned long)OSHmalloc(p_heap, buffersize, &(osdinfo.p_show_addr));
	if(osdinfo.v_show_addr == 0){
		osdinfo.v_show_addr = osdinfo.v_render_addr;
	}

	//osdinfo.p_render_addr = fui_get_bus_address(osdinfo.v_render_addr);
	//osdinfo.p_show_addr = fui_get_bus_address(osdinfo.v_show_addr);

	/// init the show instance.
	osd_de_init(&osdinfo.dev);

	osdinfo.alpha = 4;
	osdinfo.formate = mode;
	osdinfo.stride = w;
	osdinfo.transcolor = *((unsigned short *)(&osdinfo.palette[0]));
	osdinfo.w = w;
	osdinfo.h = h;
	osdinfo.x = x;
	osdinfo.y = y;
	memset((void *)osdinfo.v_render_addr,0,osdinfo.w*osdinfo.h/bpp);
	memset((void *)osdinfo.v_show_addr,0,osdinfo.w*osdinfo.h/bpp);
	osdinfo.valid = 1;

	osdengine_enable_osd();
	
	return 0;
}

int osdengine_set_alpha(int num)
{
	if(num > 8)
		num = 8;
	
	if(num < 0)
		num = 0;
	
	osdinfo.alpha = num;

	return 0;
}

int osdengine_fill_osdrect(int x,int y,int w,int h,int color)
{
	int bpp;
	char *sptr,*dptr;
	int i;
	char fillcolor=0;

	if(!osdinfo.valid){
		return -1;
	}

	if(w<=0 || h<=0){
		return -1;
	}

	if(x<0 || x>=osdinfo.w){
		return -1;
	}

	if(y<0 || y>=osdinfo.h){
		return -1;
	}

	if((x+w > osdinfo.w) || (y+h > osdinfo.h)){
		return -1;
	}

	if(osdinfo.formate == DE_PIX_FMT_OSDBIT4MODE){
		if(x%2){
			x = (x/2)*2;
		}
		bpp = 2;
		fillcolor = (((color&0xf)<<4)|(color&0xf))&0xff;
	}
	else if(osdinfo.formate == DE_PIX_FMT_OSDBIT2MODE){
		unsigned long head_start,tail_start,mid_start,line_start;
		int head_size,tail_size,mid_size;
		char *ptr;

		bpp=4;

		fillcolor = ((color&0x3)<<6) | ((color&0x3)<<4) | ((color&0x3)<<2) | (color&0x3);

		head_start = x/4;
		head_size = 4-x%4;
		if(head_size>w){
			head_size = w;
		}

		mid_size = w - head_size;

		if(mid_size > 0){
			mid_size /= 4;
		}
		else{
			mid_size = 0;
		}
		mid_start = head_start+1;

		tail_size = w - head_size - mid_size*4;
		if(tail_size <= 0){
			tail_size = 0;
		}
		tail_start = mid_start + mid_size;

		for(i=0;i<h;i++){

			line_start = osdinfo.v_render_addr+ (y+i)*osdinfo.w/bpp;

			/// fill head part.
			ptr = (char *)(line_start + head_start);
			if(head_size == 1){
				*ptr &= 0x3f;	
				*ptr |= (fillcolor&0xc0);
			}
			else if(head_size == 2){
				*ptr &= 0xf;
				*ptr |= (fillcolor&0xf0);
			}
			else if(head_size == 3){
				*ptr &= 0x3;
				*ptr |= (fillcolor&0xfc);
			}
			else{
				*ptr = (fillcolor&0xff);
			}

			/// fill mid part
			if(mid_size > 0){
				ptr = (char *)(line_start + mid_start);
				memset(ptr,fillcolor,mid_size);
			}

			/// fill tail part
			if(tail_size > 0){
				ptr = (char *)(line_start + tail_start);
				if(tail_size == 1){
					*ptr &= 0xfc;
					*ptr |= (fillcolor&0x3);
				}
				else if(tail_size == 2){
					*ptr &= 0xf0;
					*ptr |= (fillcolor&0xf);
				}
				else if(tail_size == 3){
					*ptr &= 0xc0;
					*ptr |= (fillcolor&0x3f);
				}
				else{
					*ptr = (fillcolor&0xff);
				}
			}
		}
		return 0;
		
	}
	else if(osdinfo.formate == DE_PIX_FMT_OSDBIT8MODE){
		bpp = 1;
		fillcolor = color&0xff;
	}
	else{
		return -1;
	}
	
	sptr = (char *)(osdinfo.v_render_addr+ y*osdinfo.w/bpp + x/bpp);
	for(i=0;i<h;i++){
		memset(sptr,fillcolor,w/bpp);
		sptr += osdinfo.w/bpp;
	}
	
	return 0;
}

static int osdengine_attach_img(void *img_buf,int img_width,int img_height,int img_pos_x,int img_pos_y)
{
	int bpp=0;
	int i=0;
	int start_addr;
	int line_bytes;
	int addr_head,addr_tail;
	int line_head,line_tail;
	unsigned char * osd_render_buf=(unsigned char *)osdinfo.v_render_addr;
	unsigned char * line_buf=NULL;
	int osd_width,osd_height;
	int real_width,real_height;
	if(osdinfo.formate==DE_PIX_FMT_OSDBIT4MODE){
		bpp = 2;
	}
	else if(osdinfo.formate==DE_PIX_FMT_OSDBIT2MODE){
		bpp = 4;
	}
	else if(osdinfo.formate==DE_PIX_FMT_OSDBIT8MODE){
		bpp = 1;
	}

	osd_width = osdinfo.w;
	osd_height = osdinfo.h;

	real_width = (img_pos_x+img_width)<=osd_width?(img_pos_x+img_width):osd_width;
	real_height =  (img_pos_y+img_height)<=osd_height?(img_pos_y+img_height):osd_height;

	real_width = real_width-img_pos_x;
	real_height = real_height-img_pos_y;

	if(real_width<0 || real_height <0){
		printf("%s,%d: image  attach error!, img_pos_x=%d,img_pos_y=%d,img_width=%d,img_height=%d,osd_w=%d,osd_h=%d\n",\
			__FILE__,__LINE__,img_pos_x,img_pos_y,img_width,img_height,osd_width,osd_height);
		return -1;
	}

	start_addr = (img_pos_y*osd_width + img_pos_x)/bpp;
	addr_head = (img_pos_y*osd_width + img_pos_x)%bpp;

	if(real_width%bpp==0)
		line_bytes = real_width/bpp;
	else
		line_bytes = real_width/bpp+1;

	line_buf  = (unsigned char*)malloc(line_bytes+1);///need one more byte

	if(line_buf==NULL){
		printf("%s,%d:Malloc LINE BUF Err size=%d!",__FILE__,__LINE__,line_bytes+1);
		return -1;
	}

	//printf("OSD Width=0x%x,OSD_Height=0x%x,img_width=%d,img_height=%d,line_bytes=0x%x\n",osd_width,osd_height,img_width,img_height,line_bytes);
	for(i=0;i<real_height;i++){
		line_head = __get_byte(i*img_width,bpp);
		line_tail = __get_byte(i*img_width+real_width,bpp);
		memset(line_buf,0,line_bytes+1);
		memcpy(line_buf,img_buf+line_head,line_tail-line_head);
		start_addr = __get_byte((i+img_pos_y)*osd_width+img_pos_x,bpp);
		//printf("[%d]line_head=0x%x,line_tail=0x%x,start_addr=0x%x\n",i,line_head,line_tail,start_addr);
		memcpy(start_addr+osd_render_buf,line_buf,line_tail-line_head);
	}


	if(line_buf)
		free(line_buf);

	return 0;
}

int osdengine_attach_img_align(void *img_buf,int img_width,int img_height,int OSD_ALIGN)
{
	int pos_x,pos_y;
	if((OSD_ALIGN & 0x0F) == OSD_ALIGN_H_LEFT)
		pos_x = 0;
	else if((OSD_ALIGN &0x0F) == OSD_ALIGN_H_RIGHT)
		pos_x = osdinfo.w-img_width;
	else if((OSD_ALIGN &0x0F) == OSD_ALIGN_H_CENTER)
		pos_x = (osdinfo.w-img_width)/2;
	else
		pos_x =0;

	if(pos_x <=0)
		pos_x =0;


	if((OSD_ALIGN &0xF0)== OSD_ALIGN_V_TOP)
		pos_y = 0;
	else if((OSD_ALIGN &0xF0) == OSD_ALIGN_V_BOTTOM)
		pos_y = osdinfo.h-img_height;
	else if((OSD_ALIGN &0xF0) == OSD_ALIGN_V_CENTER)
		pos_y = (osdinfo.h-img_height)/2;
	else
		pos_y =0;

	if(pos_y<=0)
		pos_y =0;

	
	osdengine_attach_img(img_buf,img_width,img_height,pos_x,pos_y);

	return 1;
}

int osdengine_update_osdrect(int x,int y,int w,int h)
{
	int bpp;
	char *sptr,*dptr;
	int i;
	char *ptr;

	if(!osdinfo.valid){
		return -1;
	}

	if(w<=0 || h<=0){
		return -1;
	}

	if(x<0 || x>=osdinfo.w){
		return -1;
	}

	if(y<0 || y>=osdinfo.h){
		return -1;
	}

	if((x+w > osdinfo.w) || (y+h > osdinfo.h)){
		return -1;
	}

	if(osdinfo.formate == DE_PIX_FMT_OSDBIT4MODE){
		if(x%2){
			x = (x/2)*2;
		}
		bpp = 2;
	}
	else if(osdinfo.formate == DE_PIX_FMT_OSDBIT2MODE){
		unsigned long head_start,tail_start,mid_start,line_start;
		int head_size,tail_size,mid_size;
		unsigned long dst_line_start,src_line_start;

		bpp=4;

		head_start = x/4;
		head_size = 4-x%4;
		if(head_size>w){
			head_size = w;
		}

		mid_size = w - head_size;

		if(mid_size > 0){
			mid_size /= 4;
		}
		else{
			mid_size = 0;
		}
		mid_start = head_start+1;

		tail_size = w - head_size - mid_size*4;
		if(tail_size <= 0){
			tail_size = 0;
		}
		tail_start = mid_start + mid_size;

		for(i=0;i<h;i++){

			src_line_start = osdinfo.v_render_addr+ (y+i)*osdinfo.w/bpp;
			dst_line_start = osdinfo.v_show_addr+ (y+i)*osdinfo.w/bpp;

			sptr = (char *)(src_line_start + head_start);
			dptr = (char *)(dst_line_start + head_start);
			if(head_size == 1){
				*dptr &= 0x3f;
				*sptr &= 0xc0;
				*dptr |= *sptr;
			}
			else if(head_size == 2){
				*dptr &= 0xf;
				*sptr &= 0xf0;
				*dptr |= *sptr;
			}
			else if(head_size == 3){
				*dptr &= 0x3;
				*sptr &= 0xfc;
				*dptr |= *sptr;
			}
			else{
				*dptr = *sptr;
			}

			if(mid_size > 0){
				sptr = (char *)(src_line_start + mid_start);
				dptr = (char *)(dst_line_start + mid_start);
				memcpy(dptr,sptr,mid_size);
			}

			if(tail_size > 0){
				sptr = (char *)(src_line_start + tail_start);
				dptr = (char *)(dst_line_start + tail_start);
				if(tail_size == 1){
					*dptr &= 0xfc;
					*sptr &= 0x3;
					*dptr |= *sptr;
				}
				else if(tail_size == 2){
					*dptr &= 0xf0;
					*sptr &= 0xf;
					*dptr |= *sptr;
				}
				else if(tail_size == 3){
					*dptr &= 0xc0;
					*sptr &= 0x3f;
					*dptr |= *sptr;
				}
				else{
					*dptr = *sptr;
				}
			}
		}
		
		return 0;
		
	}
	else if(osdinfo.formate == DE_PIX_FMT_OSDBIT8MODE){
		bpp = 1;
	}
	else{
		return -1;
	}

	/// update the rect area.
	sptr = (char *)(osdinfo.v_render_addr+ y*osdinfo.w/bpp + x/bpp);
	dptr = (char *)(osdinfo.v_show_addr+ y*osdinfo.w/bpp + x/bpp);
	if(sptr != dptr){
		for(i=0;i<h;i++){
			memcpy(dptr,sptr,w/bpp);
			sptr += osdinfo.w/bpp;
			dptr += osdinfo.w/bpp;
		}
	}
	return 0;
}

int osdengine_clear_osdrect(int x,int y,int w,int h)
{
	int bpp;
	char *ptr;
	int i;

	if(!osdinfo.valid){
		return -1;
	}

	if(w<=0 || h<=0){
		return -1;
	}

	if(x<0 || x>=osdinfo.w){
		return -1;
	}

	if(y<0 || y>=osdinfo.h){
		return -1;
	}

	if((x+w > osdinfo.w) || (y+h > osdinfo.h)){
		return -1;
	}

	if(osdinfo.formate == DE_PIX_FMT_OSDBIT4MODE){
		if(x%2){
			x = (x/2)*2;
		}
		bpp = 2;
	}
	else if(osdinfo.formate == DE_PIX_FMT_OSDBIT2MODE){
		unsigned long head_start,tail_start,mid_start,line_start;
		int head_size,tail_size,mid_size;

		bpp=4;

		head_start = x/4;
		head_size = 4-x%4;
		if(head_size>w){
			head_size = w;
		}

		mid_size = w - head_size;

		if(mid_size > 0){
			mid_size /= 4;
		}
		else{
			mid_size = 0;
		}
		mid_start = head_start+1;

		tail_size = w - head_size - mid_size*4;
		if(tail_size <= 0){
			tail_size = 0;
		}
		tail_start = mid_start + mid_size;

		for(i=0;i<h;i++){

			line_start = osdinfo.v_render_addr+ (y+i)*osdinfo.w/bpp;

			/// fill head part.
			ptr = (char *)(line_start + head_start);
			if(head_size == 1){
				*ptr &= 0x3f;
			}
			else if(head_size == 2){
				*ptr &= 0xf;
			}
			else if(head_size == 3){
				*ptr &= 0x3;
			}
			else{
				*ptr = 0;
			}

			/// fill mid part
			if(mid_size > 0){
				ptr = (char *)(line_start + mid_start);
				memset(ptr,0,mid_size);
			}

			/// fill tail part
			if(tail_size > 0){
				ptr = (char *)(line_start + tail_start);
				if(tail_size == 1){
					*ptr &= 0xfc;
				}
				else if(tail_size == 2){
					*ptr &= 0xf0;
				}
				else if(tail_size == 3){
					*ptr &= 0xc0;
				}
				else{
					*ptr = 0;
				}
			}
		}

		
		return 0;
		
	}
	else if(osdinfo.formate == DE_PIX_FMT_OSDBIT8MODE){
		bpp = 1;
	}
	else{
		return -1;
	}

	/// clear the rect area.
	ptr = (char *)(osdinfo.v_render_addr+ y*osdinfo.w/bpp + x/bpp);
	for(i=0;i<h;i++){
		memset(ptr,0,w/bpp);
		ptr += osdinfo.w/bpp;
	}
	
	return 0;
}

int osdengine_show_icon(int x,int y,char *path,int update_now)
{
	struct osd_icon_header header;
	FILE *fp;
	char *ptr;
	int i;
	int bpp;

	if(!osdinfo.valid){
		return -1;
	}
	
	if(!path){
		return -1;
	}

	if(osdinfo.formate == DE_PIX_FMT_OSDBIT4MODE){
		bpp = 2;
	}
	else if(osdinfo.formate == DE_PIX_FMT_OSDBIT8MODE){
		bpp = 1;
	}
	else if(osdinfo.formate == DE_PIX_FMT_OSDBIT2MODE){
		/// for 2bits osd, we do not support show icon.
		return -1;
	}
	else{
		printf("osd formate not supported\n");
		return -1;
	}

	memset(&header,0,sizeof(struct osd_icon_header));
	
	fp = fopen(path,"rb");
	if(!fp){
		return -1;
	}

	fread(&header, sizeof(struct osd_icon_header), 1, fp);
	printf("%d\n",x);
	x = (x/2)*2;
	printf("%d %d %d\n", x,x+header.wWidth,osdinfo.w);
	if(x<0 || (x+header.wWidth)>osdinfo.w){
		printf("show icon x exceed\n");
		fclose(fp);
		return -1;
	}
	printf("%d %d %d\n", y,y+header.wHeight,osdinfo.h);
	if(y<0 || (y+header.wHeight)>osdinfo.h){
		printf("show icon y exceed\n");
		fclose(fp);
		return -1;
	}

	ptr = (char *)(osdinfo.v_render_addr + y*osdinfo.w/bpp + x/bpp);
	for(i=0;i<header.wHeight;i++){
		fread(ptr, sizeof(char), header.wWidth/bpp,fp);
		///memset(ptr,((i%16 << 4)|(i%16))&0xff,header.wWidth/bpp);
		ptr += osdinfo.w/bpp;
	}

	fclose(fp);

	if(update_now){
		memcpy((void *)osdinfo.v_show_addr,(void *)osdinfo.v_render_addr,osdinfo.w*osdinfo.h/bpp);
	}
	
	///printf("show icon [%s] at x=%d/y=%d\n",path,x,y);
	
	return 0;
}

DE_result osd_de_release (void* inst)
{
	 int fd;
	 
	 fd=((DE_inst*)inst)->fd;
	 OSfree(inst);
	 if(fd>=0)
		 close(fd);
	 return DE_OK;
};

int osdengine_release_osd()
{
	if(osdinfo.valid){
		
		printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
		//OSSleep(5000);
		printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
		osdengine_disable_osd();
		printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
		//OSSleep(5000);
		printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);	
		if(osdinfo.v_render_addr){
			OSHfree(p_heap, (void *)osdinfo.v_render_addr);
			osdinfo.v_render_addr = 0;
		}
		printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
		//OSSleep(5000);
		printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);

		if(osdinfo.v_show_addr && (osdinfo.v_render_addr != osdinfo.v_show_addr)){
			OSHfree(p_heap, (void *)osdinfo.v_show_addr);
			osdinfo.v_show_addr = 0;
		}
		printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
		//OSSleep(5000);
		printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
		if(p_heap){
			wire_MemoryRelease(p_heap);
			p_heap = NULL;
		}
		osd_de_release(osdinfo.dev);
		printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
		//OSSleep(5000);
		printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
		memset(&osdinfo,0,sizeof(struct osd_engine_info));
		printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
		//OSSleep(5000);
		printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
	}
	
	return 0;
}

