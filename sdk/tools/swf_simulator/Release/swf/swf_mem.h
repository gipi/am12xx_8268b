#ifndef SWF_MEM_H
#define SWF_MEM_H

#include "swf_types.h"

void swf_mem_init(SWF_HEAPINFO * info);
void*swf_malloc_basic(unsigned int size);
void*swf_malloc_share(unsigned int size);
void swf_free(void * p);
int  swf_mem_basic();
int  swf_mem_share();
int  swf_mem_stat(void);
int  swf_mem_max(void);
void swf_mem_destory(void);
int  swf_mem_get_heap_type(void * p);
int  swf_mem_share_total();
int  swf_dump_share(int dump);
void*swf_dbg_malloc(void * (*malloc_func)(unsigned int), unsigned int size, char * file, int line);
void swf_dbg_free(void * p);
void swf_dbg_mdump(int heap_type, int frm_no);
void *swf_mem_vir2phy(void * p);
int swf_mem_share_total();
int swf_mem_basic_total();

#endif

