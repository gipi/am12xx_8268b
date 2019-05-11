/*
 * debug.h - exFAT kernel debug support.
 *
 */
#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef DEBUG 
#define  exfat_debug(fmt,arg...)		printk("EXFAT-DEBUG[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#else
#define  exfat_debug(x...)
#endif

#define exfat_error(fmt,arg...)		printk("EXFAT-ERROR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define exfat_waring(fmt,arg...)		printk("EXFAT-WARING[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)


#endif 
