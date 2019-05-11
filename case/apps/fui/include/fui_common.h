#ifndef FUI_COMMON_H
#define FUI_COMMON_H
#include "swf_types.h"
#include "sys_msg.h"
#include "sys_cfg.h"
#include "ezcast_public.h"
#define IMAGE_WIDTH_E     (gui_get_image_width())
#define IMAGE_HEIGHT_E    (gui_get_image_height())

#if AM_CHIP_ID == 1213
// 128 DDR

#define HEAP_BASIC_ADDR   0
#if MODULE_CONFIG_DDR_CAPACITY==256
#define HEAP_BASIC_SIZE   (10*1024*1024+8*1024)
#elif MODULE_CONFIG_DDR_CAPACITY==64
#define HEAP_BASIC_SIZE   (5*1024*1024+8*1024)
#else
#define HEAP_BASIC_SIZE   (6*1024*1024+8*1024)
#endif
#define HEAP_SHARE_ADDR   0
#if !EZCAST_ENABLE
#define HEAP_SHARE_SIZE   (56*1024*1024+512*1024)
#else
#if MODULE_CONFIG_DDR_CAPACITY==256
#define HEAP_SHARE_SIZE   (108*1024*1024+512*1024)
#elif MODULE_CONFIG_EZMUSIC_ENABLE
#define HEAP_SHARE_SIZE   (12*1024*1024)
#else
#define HEAP_SHARE_SIZE   (54*1024*1024+512*1024)
#endif
#endif
#else
#define HEAP_BASIC_ADDR   0
#define HEAP_BASIC_SIZE   (5*1024*1024+8*1024)
#define HEAP_SHARE_ADDR   0
#define HEAP_SHARE_SIZE   (38*1024*1024+512*1024)
#endif

#define TSCAL_SHM_KEY			(APP_KEY_BASE + 0x10)


#define SYS_MSGQ_KEY_BASE		(APP_KEY_BASE)
#define FUI_MOUSE_MSGQ_KEY	(SYS_MSGQ_KEY_BASE+0x20)

#define USE_NEW_VIDEO_OSD

extern void fui_jpeg_dec(unsigned char * JPEGData, int JPEGSize, int mode, unsigned char ** Pixel, int * w, int * h, int * stride);
extern int  fui_decode_bitmap(SWF_IMAGE_INFO * info);
extern int  fui_play_audio(SWF_AUDIO_INFO * info);

extern int  fui_stop_audio(int s);
extern int  disable_swf_audio(void);
extern int  enable_swf_audio(void);

extern int  fui_interval();
extern void fui_show(int frame_addr, void * dev);
extern void fui_frame_ready(unsigned char * frame);

extern unsigned long fui_get_bus_address(unsigned long logicaddr);

extern void *sys_shmget(int size,int key);
extern int sys_shmrm(int key);

/**
* fui key mapping function
*/
extern int fui_read_key(struct am_sys_msg *msg);

extern void put_display_task(int address);

extern int get_current_display_addr(int flag);

extern int _wifi_direct_copy_file(char *from, char *to);

#endif

