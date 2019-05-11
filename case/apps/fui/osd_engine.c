#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <errno.h>
#include "fui_common.h"
#include "swf_ext.h"
#include "display.h"
#include "swf_types.h"

#ifdef MODULE_CONFIG_VIDEO_OSD_NEW


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



struct osd_engine_info osdinfo;

#define GET_OSD_INFO_WIDTH 0
#define GET_OSD_INFO_HEIGHT 1

/**
* for 2bits osd the palette color in RGB565 formate
*/
#define OSD_2BITS_ALPHA_COLOR 0
#define OSD_2BITS_COLOR_1 0xffff
#define OSD_2BITS_COLOR_2 0x821
#define OSD_2BITS_COLOR_3 0xf800

int osdengine_enable_osd()
{
	DE_config ds_conf;

	if(!osdinfo.valid){
		return -1;
	}

	de_get_config(osdinfo.dev,&ds_conf,DE_CFG_OSD1);
	
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
	de_set_Config(osdinfo.dev,&ds_conf,DE_CFG_OSD1);

	return 0;
	
}

int osdengine_disable_osd()
{
	DE_config ds_conf;

	if(!osdinfo.valid){
		return -1;
	}
	
	de_get_config(osdinfo.dev,&ds_conf,DE_CFG_OSD1);
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


	de_set_Config(osdinfo.dev,&ds_conf,DE_CFG_OSD1);	

	return 0;
}


int osdengine_init_osd(int x,int y,int w,int h,int mode,char *palettefile)
{
	int buffersize;
	int bpp;
	FILE *fp;
	struct osd_palette_header ph;
	unsigned short *p;

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

	osdinfo.v_render_addr = (unsigned long)SWF_Malloc(buffersize);
	if(osdinfo.v_render_addr == 0){
		printf("[video_osd failed]:render buffer alloc error\n");
		return -1;
	}

	osdinfo.v_show_addr = (unsigned long)SWF_Malloc(buffersize);
	if(osdinfo.v_show_addr == 0){
		osdinfo.v_show_addr = osdinfo.v_render_addr;
	}

	osdinfo.p_render_addr = fui_get_bus_address(osdinfo.v_render_addr);
	osdinfo.p_show_addr = fui_get_bus_address(osdinfo.v_show_addr);

	/// init the show instance.
	de_init(&osdinfo.dev);

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
extern void *deinst;
static DE_config ds_conf;

static int set_de_defaultcolor()
{
	de_get_config(deinst,&ds_conf,DE_CFG_ALL);
	ds_conf.input.enable=0;
	de_set_Config(deinst,&ds_conf,DE_CFG_ALL);
	
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

 int osdengine_release_osd()
{
	if(osdinfo.valid){
		
		//printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
		//OSSleep(5000);
		//printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
		osdengine_disable_osd();
		printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
		//OSSleep(5000);
		//printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);	
		if(osdinfo.v_render_addr){
			SWF_Free((void *)osdinfo.v_render_addr);
			osdinfo.v_render_addr=0;
		}
		//printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
		//OSSleep(5000);
		printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);

		if(osdinfo.v_show_addr && (osdinfo.v_render_addr != osdinfo.v_show_addr)){
			SWF_Free((void *)osdinfo.v_show_addr);
			osdinfo.v_show_addr=0;
		}
		//printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
		//OSSleep(5000);
		printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);

		de_release(osdinfo.dev);
		printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
		//OSSleep(5000);
		//printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
		memset(&osdinfo,0,sizeof(struct osd_engine_info));
		printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
		//OSSleep(5000);
		//printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
	}
	
	return 0;
}
/*stop video_engine and release_osd*/
static int special_prepare_for_release_osd(void * handle){
	
	SWFEXT_FUNC_BEGIN(handle);
	printf("special_prepare_for_release_osd\n");
	set_de_defaultcolor();
	SWFEXT_FUNC_END();	
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
	printf("x=%d header.wWidth=%d osdinfo.w=%d\n",x,header.wWidth,osdinfo.w);
	x = (x/2)*2;

	if(x<0 || (x+header.wWidth)>osdinfo.w){
		printf("show icon x exceed\n");
		fclose(fp);
		return -1;
	}

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

static unsigned short get_utf16(char * str, int * index)
{
	int i = *index;
	
	if((str[i] & 0x80) == 0)
	{
		*index = i + 1;
		return str[i];
	}
	else if((str[i] & 0x20) == 0)
	{
		*index = i + 2;
		return ((str[i] << 6) & 0x07c0) | (str[i+1] & 0x003f);
	}
	else if((str[i] & 0x10) == 0)
	{
		*index = i + 3;
		return ((str[i] << 12) & 0xf000) | ((str[i+1] << 6) & 0x0fc0) | (str[i+2] & 0x003f);
	}

	return 0;
}

int osdengine_show_string(int x,int y,int xrange,int fontsize,char *str,int fontcolor,int sheight)
{
	if(!osdinfo.valid){
		return -1;
	}
	
	if(!str){
		return -1;
	}

	if(sheight == 0){
		sheight = fontsize;
	}

	if(fontsize > sheight){
		printf("fontsize too big\n");
		return -1;
	}

	if(osdinfo.y + sheight > osdinfo.y+osdinfo.h)
		return -1;
	
	//printf("show string:%s\n",str);	

	int i = 0;
	unsigned int previous = 0;
	unsigned int height = fontsize;
	unsigned int color = fontcolor;
	unsigned short c;	
	SWF_RECT rect;
	rect.Xmin = x;
	rect.Xmax = x + xrange;
	rect.Ymin = y;
	rect.Ymax = y + sheight;
	
	
	c = get_utf16(str, &i);
	while(c != 0)
	{
		if (c == '\n' || c == '\r'){	//unix格式
			break;	
		}
		else if ((x <= rect.Xmax) || c == ' '){
			if(osdinfo.formate == DE_PIX_FMT_OSDBIT4MODE){
				SWF_TtfDisplayHorz(3, osdinfo.v_render_addr, osdinfo.w, color, &x, &y, c, height, &rect, NULL, 0, &previous,0,NULL);
			}
			else if(osdinfo.formate == DE_PIX_FMT_OSDBIT2MODE){
				SWF_TtfDisplayHorz(4, osdinfo.v_render_addr, osdinfo.w, color, &x, &y, c, height, &rect, NULL, 0, &previous,0,NULL);
			}
			else if(osdinfo.formate == DE_PIX_FMT_OSDBIT8MODE){
				SWF_TtfDisplayHorz(5, osdinfo.v_render_addr, osdinfo.w, color, &x, &y, c, height, &rect, NULL, 0, &previous,0,NULL);
			}
		}

		c = get_utf16(str, &i);
	}		

	return -1;
}

static int osdengine_get_icon_info(char *icon,struct osd_icon_header *info)
{
	FILE *fp;
	int nr;
	
	if(!icon || !info){
		return -1;
	}

	fp = fopen(icon,"rb");
	if(!fp){
		printf("open file : %s error\n",icon);
		return -1;
	}

	nr = fread(info, 1,sizeof(struct osd_icon_header),fp);
	if(nr != sizeof(struct osd_icon_header)){
		fclose(fp);
		return -1;
	}

	fclose(fp);
	
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

#define __get_byte(which_pix,bpp)	((which_pix)/(bpp))
/**
* attach the image to the position which is specified by img_pos_x and img_pos_y
* @param[in] img_buf : the buf which contained the image data
* @param[in] img_width : the width of the image
* @param[in] img_height : the height of the image
* @param[in] img_pos_x : the x coordinate where the image will be attach
* @param[in] img_pos_y : the ycoordinate where the image will be attach
**/
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

#define OSD_ALIGN_H_LEFT 		0x01
#define OSD_ALIGN_H_RIGHT 		0x02
#define OSD_ALIGN_H_CENTER 		0x04
#define OSD_ALIGN_V_TOP			0x10
#define OSD_ALIGN_V_BOTTOM		0x20
#define OSD_ALIGN_V_CENTER		0x40

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

static int osdengine_enable(void * handle)
{
	int err;
	
	SWFEXT_FUNC_BEGIN(handle);

	err = osdengine_enable_osd();
	if(err<0){
		Swfext_PutNumber(0);
	}
	else{
		Swfext_PutNumber(1);
	}

	SWFEXT_FUNC_END();	
}

static int osdengine_disable(void * handle)
{
	int err;
	
	SWFEXT_FUNC_BEGIN(handle);

	err = osdengine_disable_osd();
	if(err<0){
		Swfext_PutNumber(0);
	}
	else{
		Swfext_PutNumber(1);
	}

	SWFEXT_FUNC_END();	
}

static int osdengine_init(void * handle)
{
	int x,y,w,h,mode;
	int err;
	char *palette;
	
	SWFEXT_FUNC_BEGIN(handle);

	x = Swfext_GetNumber();
	y = Swfext_GetNumber();
	w = Swfext_GetNumber();
	h = Swfext_GetNumber();
	mode = Swfext_GetNumber();
	palette = Swfext_GetString();

	err = osdengine_init_osd(x,y,w,h,mode,palette);
	if(err<0){
		Swfext_PutNumber(0);
	}
	else{
		Swfext_PutNumber(1);
	}

	SWFEXT_FUNC_END();	
}

static int osdengine_release(void * handle)
{
	int err;
	
	SWFEXT_FUNC_BEGIN(handle);
	printf("function:%s,line:%d,osdengine_release\n",__FUNCTION__,__LINE__);
	
	err = osdengine_release_osd();
	if(err<0){
		Swfext_PutNumber(0);
	}
	else{
		Swfext_PutNumber(1);
	}
	SWFEXT_FUNC_END();	
}

static int osdengine_update_rect(void * handle)
{	
	int x,y,w,h;
	
	SWFEXT_FUNC_BEGIN(handle);

	x = Swfext_GetNumber();
	y = Swfext_GetNumber();
	w = Swfext_GetNumber();
	h = Swfext_GetNumber();

	osdengine_update_osdrect(x,y,w,h);

	SWFEXT_FUNC_END();	
}


static int osdengine_update(void * handle)
{	
	SWFEXT_FUNC_BEGIN(handle);

	osdengine_update_osdrect(0,0,osdinfo.w,osdinfo.h);

	SWFEXT_FUNC_END();	
}

static int osdengine_clear_rect(void * handle)
{	
	int x,y,w,h;
	
	SWFEXT_FUNC_BEGIN(handle);

	x = Swfext_GetNumber();
	y = Swfext_GetNumber();
	w = Swfext_GetNumber();
	h = Swfext_GetNumber();
	
	osdengine_clear_osdrect(x,y,w,h);

	SWFEXT_FUNC_END();	
}

static int osdengine_clear(void * handle)
{	
	SWFEXT_FUNC_BEGIN(handle);

	osdengine_clear_osdrect(0,0,osdinfo.w,osdinfo.h);

	SWFEXT_FUNC_END();	
}

static int osdengine_fill_rect(void * handle)
{	
	int x,y,w,h,color;
	
	SWFEXT_FUNC_BEGIN(handle);

	x = Swfext_GetNumber();
	y = Swfext_GetNumber();
	w = Swfext_GetNumber();
	h = Swfext_GetNumber();
	color = Swfext_GetNumber();
	
	osdengine_fill_osdrect(x,y,w,h,color);

	SWFEXT_FUNC_END();	
}




static int osdengine_show_osdicon(void * handle)
{
	int x,y;
	char *path;
	int update_now;
	int err;
	
	SWFEXT_FUNC_BEGIN(handle);

	x = Swfext_GetNumber();
	y = Swfext_GetNumber();
	path = Swfext_GetString();
	update_now = Swfext_GetNumber();

	err = osdengine_show_icon(x,y,path,update_now);
	if(err<0){
		Swfext_PutNumber(0);
	}
	else{
		Swfext_PutNumber(1);
	}

	SWFEXT_FUNC_END();	
}

static int osdengine_show_osdstring(void * handle)
{
	int x,y;
	int xrange;
	int fontsize,fontcolor;
	char *str;
	int err;
	
	SWFEXT_FUNC_BEGIN(handle);

	x = Swfext_GetNumber();
	y = Swfext_GetNumber();
	xrange = Swfext_GetNumber();
	fontsize = Swfext_GetNumber();
	str = Swfext_GetString();
	fontcolor = Swfext_GetNumber();

	err = osdengine_show_string(x,y,xrange,fontsize,str,fontcolor,0);
	if(err<0){
		Swfext_PutNumber(0);
	}
	else{
		Swfext_PutNumber(1);
	}

	SWFEXT_FUNC_END();	
}

static int osdengine_show_osdstring2(void * handle)
{
	int x,y;
	int xrange;
	int fontsize,fontcolor,sheight;
	char *str;
	int err;
	
	SWFEXT_FUNC_BEGIN(handle);

	x = Swfext_GetNumber();
	y = Swfext_GetNumber();
	xrange = Swfext_GetNumber();
	fontsize = Swfext_GetNumber();
	sheight = Swfext_GetNumber();
	str = Swfext_GetString();
	fontcolor = Swfext_GetNumber();

	err = osdengine_show_string(x,y,xrange,fontsize,str,fontcolor,sheight);
	if(err<0){
		Swfext_PutNumber(0);
	}
	else{
		Swfext_PutNumber(1);
	}

	SWFEXT_FUNC_END();	
}

static int osdengine_get_iconinfo(void * handle)
{
	char *icon;
	int param=0;
	struct osd_icon_header info;
	int err;
	
	SWFEXT_FUNC_BEGIN(handle);

	icon = Swfext_GetString();
	param = Swfext_GetNumber();

	err = osdengine_get_icon_info(icon,&info);
	if(err != 0){
		Swfext_PutNumber(0);
	}
	else{
		if(param == GET_OSD_INFO_WIDTH){
			Swfext_PutNumber(info.wWidth);
		}
		else if(param == GET_OSD_INFO_HEIGHT){
			Swfext_PutNumber(info.wHeight);
		}
		else{
			Swfext_PutNumber(0);
		}
	}

	SWFEXT_FUNC_END();	
}

static int osdengine_get_xmouse(void * handle)
{
	int x;
	
	SWFEXT_FUNC_BEGIN(handle);

	SWF_GetMousePos(handle,&x,NULL);

	Swfext_PutNumber(x);

	SWFEXT_FUNC_END();	
}

static int osdengine_get_ymouse(void * handle)
{
	int y;
	
	SWFEXT_FUNC_BEGIN(handle);

	SWF_GetMousePos(handle,NULL,&y);

	Swfext_PutNumber(y);

	SWFEXT_FUNC_END();	
}

int swfext_osdengine_register(void)
{
	SWFEXT_REGISTER("osdengine_enable", osdengine_enable);
	SWFEXT_REGISTER("osdengine_disable", osdengine_disable);
	SWFEXT_REGISTER("osdengine_init", osdengine_init);
	SWFEXT_REGISTER("osdengine_release", osdengine_release);
	SWFEXT_REGISTER("osdengine_updateRect", osdengine_update_rect);
	SWFEXT_REGISTER("osdengine_update", osdengine_update);
	SWFEXT_REGISTER("osdengine_clearRect", osdengine_clear_rect);
	SWFEXT_REGISTER("osdengine_clear", osdengine_clear);
	SWFEXT_REGISTER("osdengine_showIcon", osdengine_show_osdicon);
	SWFEXT_REGISTER("osdengine_showString", osdengine_show_osdstring);
	SWFEXT_REGISTER("osdengine_showString2", osdengine_show_osdstring2);
	SWFEXT_REGISTER("osdengine_getIconInfo", osdengine_get_iconinfo);
	SWFEXT_REGISTER("osdengine_getXMouse", osdengine_get_xmouse);
	SWFEXT_REGISTER("osdengine_getYMouse", osdengine_get_ymouse);
	SWFEXT_REGISTER("osdengine_fillRect", osdengine_fill_rect);
	SWFEXT_REGISTER("osdengine_prepareforreleaseosd", special_prepare_for_release_osd);
	return 0;
}



#if 0
static int test_osd_attach_img()
{
	unsigned char *buf=malloc(6*1024);
	FILE * pfile=NULL;
	int bytes_read=0;
	pfile = fopen("/mnt/udisk/1.data","rb");
	if(pfile==NULL){
		printf("Open 1.data Fialed!\n");
		return -1;
	}

	
	bytes_read = fread(buf,1,0x1734,pfile);
	printf("Open File OK Malloc OK bytes-read=0x%x\n",bytes_read);

	osdengine_attach_img_align(buf,720,33,OSD_ALIGN_H_CENTER|OSD_ALIGN_V_CENTER);
	
	osdengine_attach_img(buf,720,33,300,200);

	osdengine_update_osdrect(0,0,osdinfo.w,osdinfo.h);
	fclose(pfile);
	free(buf);
	
}

int osdengine_test()
{
	struct osd_icon_header info;
	int cnt=0;
	int i;
	int start_x;

	printf("===\n\n");
	
	//osdengine_init_osd(0,0,768,300,DE_PIX_FMT_OSDBIT2MODE,"/am7x/case/data/osdicon/palette.plt");
	//osdengine_init_osd(0,0,768,300,DE_PIX_FMT_OSDBIT2MODE,"/am7x/case/data/osdicon/palette.plt");

	osdengine_init_osd(100,100,320,320,DE_PIX_FMT_OSDBIT4MODE,"/am7x/case/data/osdicon/palette.plt");
	//osdengine_show_icon(10,9,"/am7x/case/data/osdicon/menu.bin",0);
	start_x = 4;
	for(i=0;i<12;i++){
		osdengine_clear_osdrect(start_x,i*20,i+1,16);
		osdengine_fill_osdrect(start_x,i*20,i+1,16,0);
		osdengine_update_osdrect(start_x,i*20,i+1,16);
	}


	start_x = 21;
	for(i=0;i<12;i++){
		osdengine_clear_osdrect(start_x,i*20,i+1,16);
		osdengine_fill_osdrect(start_x,i*20,i+1,16,1);
		osdengine_update_osdrect(start_x,i*20,i+1,16);
	}

	start_x = 42;
	for(i=0;i<12;i++){
		osdengine_clear_osdrect(start_x,i*20,i+1,16);
		osdengine_fill_osdrect(start_x,i*20,i+1,16,2);
		osdengine_update_osdrect(start_x,i*20,i+1,16);
	}

	start_x = 63;
	for(i=0;i<12;i++){
		osdengine_clear_osdrect(start_x,i*20,i+1,16);
		osdengine_fill_osdrect(start_x,i*20,i+1,16,3);
		osdengine_update_osdrect(start_x,i*20,i+1,16);
	}
	
	
	//osdengine_get_icon_info("/am7x/case/data/osdicon/menu.bin",&info);
	//printf("osdicon:w=%d,h=%d\n",info.wWidth,info.wHeight);

	//osdengine_clear_osdrect(10,100,200,80);

	//osdengine_fill_osdrect(10,100,200,80,3);

	osdengine_show_string(0, 0, 260, 24, "hello! world.",1,0);
	
	osdengine_update_osdrect(0,0,osdinfo.w,osdinfo.h);
	test_osd_attach_img();
	sleep(20);
	osdengine_release_osd();

#if 0
	while(1){
		sleep(5);
	#if 0
		if(cnt==0){
			osdengine_disable_osd();
			cnt = 1;
		}
		else{
			osdengine_enable_osd();
			cnt = 0;
		}
	#endif
		
	}
#endif
	return 0;
}
#endif

#endif /** MODULE_CONFIG_VIDEO_OSD_NEW */


