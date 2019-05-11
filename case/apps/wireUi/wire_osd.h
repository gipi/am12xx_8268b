#ifndef __WIRE_OSD_H
#define __WIRE_OSD_H


#include "display.h"

typedef struct _de_instance_
{
	DE_config config;
	int fd; //lcm driver handle
}DE_inst;

#define OSD_ALIGN_H_LEFT 		0x01
#define OSD_ALIGN_H_RIGHT 		0x02
#define OSD_ALIGN_H_CENTER 		0x04
#define OSD_ALIGN_V_TOP			0x10
#define OSD_ALIGN_V_BOTTOM		0x20
#define OSD_ALIGN_V_CENTER		0x40

DE_result osd_de_init(void* *inst);

DE_result osd_de_get_config(void* inst,DE_config *conf, int flg);

DE_result osd_de_set_Config(void* inst,DE_config *conf, int flg);

DE_result osd_de_release (void* inst);

int osdengine_init_osd(int x,int y,int w,int h,int mode,char *palettefile);

int osdengine_enable_osd();

int osdengine_disable_osd();

int osdengine_fill_osdrect(int x,int y,int w,int h,int color);

int osdengine_clear_osdrect(int x,int y,int w,int h);

int osdengine_attach_img_align(void *img_buf,int img_width,int img_height,int OSD_ALIGN);

int osdengine_update_osdrect(int x,int y,int w,int h);

int osdengine_show_icon(int x,int y,char *path,int update_now);

int osdengine_release_osd();

#endif
