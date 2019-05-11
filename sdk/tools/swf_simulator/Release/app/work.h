#ifndef SWF_WORK_H
#define SWF_WORK_H

#include "swf_render.h"

#ifdef MIPS_VERSION
	#if AM_CHIP_ID==1203 || AM_CHIP_ID==1207 || AM_CHIP_ID == 1211
		#define USE_2D_GRAPHIC
		//#define LINUX_2D_POLL_MODE
	#endif

	#if AM_CHIP_ID == 1211
		#ifdef USE_2D_GRAPHIC
			#define ZBUFFER_INVERT_MODE
		#endif
	#endif
#endif

extern void  open_2d_workflow(void);
extern int   put_2d_workflow(RENDER_INFO * ri);
extern int   get_2d_workflow();
extern int   wait_2d_workflow();
extern void  isr_2d_workflow(unsigned int irq, void *dev_id);
extern void  close_2d_workflow();
extern void  run_2d_fillwork(FILLINFO * fillinfo);

/**
* @brief thread sleep function in milliseconds.
*/
extern void swf_thread_sleep(int msec);
#endif
