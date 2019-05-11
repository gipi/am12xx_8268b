#ifndef __WIRE_HEAP_H
#define __WIRE_HEAP_H

#ifndef __ACT_MODULE_CONFIG_H__ //For LinuxTool bug!! (i.e.; #if !define xxx)
#define __ACT_MODULE_CONFIG_H__
#include "../../../scripts/mconfig.h"
#endif

#if (MODULE_CONFIG_DDR_CAPACITY < 128)
#define ITS_SMALL_MEMORY	1
#else
#define ITS_SMALL_MEMORY	0
#endif

#if defined(MODULE_CONFIG_DISABLE_SWF_ENGINE) && MODULE_CONFIG_DISABLE_SWF_ENGINE!=0
	#define EZWIRE_SNOR_8M		1
	#define EZWIRE_SWF_MALLOC	0
#else
	#define EZWIRE_SNOR_8M		0
	#define EZWIRE_SWF_MALLOC	1
#endif


void *wire_memoryInit(unsigned int heap_size, int fromEnd, const char *func, unsigned int line);
void wire_memoryRelease(void *sh, const char *func, unsigned int line);
unsigned long wire_getBusAddress(unsigned long logicaddr);
void wire_Free(void *p);
void *wire_MallocWithPhysicAddress(unsigned int size, unsigned long *pbus);
void *wire_Malloc(unsigned int size);
int wire_set_context();
void wire_relesae_context();
unsigned int getStreamingDecodeMemorySize();

#define wire_MemoryInit(a) wire_memoryInit(a, 0, __func__, __LINE__)
#define wire_MemoryInit_r(a) wire_memoryInit(a, 1, __func__, __LINE__)
#define wire_MemoryRelease(a) wire_memoryRelease(a, __func__, __LINE__)

#endif
