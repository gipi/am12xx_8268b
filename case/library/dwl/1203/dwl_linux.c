/*------------------------------------------------------------------------------
--                                                                            --
--       This software is confidential and proprietary and may be used        --
--        only as expressly authorized by a licensing agreement from          --
--                                                                            --
--                            Hantro Products Oy.                             --
--                                                                            --
--                   (C) COPYRIGHT 2006 HANTRO PRODUCTS OY                    --
--                            ALL RIGHTS RESERVED                             --
--                                                                            --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
--                                                                            --
--------------------------------------------------------------------------------
--
--  Description :  dwl common part
--
------------------------------------------------------------------------------
--
--  Version control information, please leave untouched.
--
--  $RCSfile: dwl_linux.c,v $
--  $Revision: 1.7 $
--  $Date: 2007/04/03 17:06:35 $
--
------------------------------------------------------------------------------*/

#include "basetype.h"
#include "dwl.h"
#include "dwl_linux.h"

#include "memalloc.h"
#include "dwl_linux_lock.h"

#include "hx170dec.h"   /* This DWL uses the kernel module */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/timeb.h>
#include <fcntl.h>
#include <unistd.h>

#include <errno.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DWL_VC1_E   27
#define DWL_JPEG_E  26
#define DWL_MPEG4_E 25
#define DWL_H264_E  24
#define DWL_PP_E    16
#define DWL_BLEND_E 24

/*------------------------------------------------------------------------------
	Function name   : DWLReadAsicID
	Description     : Read the HW ID. Does not need a DWL instance to run

	Return type     : u32 - the HW ID
------------------------------------------------------------------------------*/
u32 DWLReadAsicID()
{
	u32 *io, id = ~0;
	int fd_dec = -1;
	u32 pgsize = getpagesize();
	unsigned long base = ~0;

	io = MAP_FAILED;

	int fd = open("/dev/mem", O_RDONLY);
	if(fd == -1)
	{
		DWL_DEBUG("DWLReadAsicID: failed to open /dev/mem\n");
		goto end;
	}

	fd_dec = open("/tmp/dev/hx170", O_RDONLY);
	if(fd_dec == -1)
	{
		DWL_DEBUG("DWLReadAsicID: failed to open /tmp/dev/hx170\n");
		goto end;
	}

	ioctl(fd_dec, HX170DEC_IOCGHWOFFSET, &base);
	
	io = (u32 *)mmap(0, 4096, PROT_READ, MAP_SHARED, fd, base);

	if(io == MAP_FAILED)
	{
		DWL_DEBUG("DWLReadAsicID: failed to mmap regs.");
		goto end;
	}

	id = io[0];
	
end:
	if(io != MAP_FAILED)
	   munmap(io, pgsize);
	if(fd != -1)
	   close(fd);
	if(fd_dec != -1)
	   close(fd_dec);
	
	return id;
}
/*------------------------------------------------------------------------------
	Function name   : DWLReadAsicConfig
	Description     : Read HW configuration. Does not need a DWL instance to run

	Return type     : DWLHwConfig_t - structure with HW configuration
------------------------------------------------------------------------------*/
DWLHwConfig_t DWLReadAsicConfig(void)
{
	u32 *io = 0;
	u32 configReg = 0;
	DWLHwConfig_t config;    
	unsigned long base;
	u32 pgsize = getpagesize();
	int fd=-1, fd_dec = -1;

	fd = open("/dev/mem", O_RDONLY);
	if(fd == -1)
	{
		DWL_DEBUG("DWLReadAsicConfig: failed to open /dev/mem\n");
	}

	memset(&config, 0, sizeof(DWLHwConfig_t));
		
	fd_dec = open("/tmp/dev/hx170", O_RDONLY);
	if(fd_dec == -1)
	{
		DWL_DEBUG("DWLReadAsicConfig: failed to open /tmp/dev/hx170\n");
	}

	ioctl(fd_dec, HX170DEC_IOCGHWOFFSET, &base);

	io = (u32 *) mmap(0, 4096, PROT_READ, MAP_SHARED, fd, base);
	if(io == MAP_FAILED)
	{ 
		 DWL_DEBUG("DWLReadAsicConfig: failed to mmap\n");
	}

	if(io != MAP_FAILED)
	{
		/* Decoder configuration */
		configReg = io[HX170DEC_SYNTH_CFG];

		config.h264Enabled = (configReg>>DWL_H264_E)&1;
		config.jpegEnabled = (configReg>>DWL_JPEG_E)&1;
		config.mpeg4Enabled = (configReg>>DWL_MPEG4_E)&1;
		config.vc1Enabled = (configReg>>DWL_VC1_E)&1;
		config.maxDecodingWidth = configReg&0x7FF;

		/* Pp configuration */
		configReg = io[HX170PP_SYNTH_CFG];

		if((configReg>>DWL_PP_E) &1)
		{
			config.maxPpOutWidth = configReg&0x7FF;
			config.ppBlendEnabled = (configReg>>DWL_BLEND_E)&1;
			config.ppEnabled = 1;
		}
		else
		{
			config.maxPpOutWidth = 0;
			config.ppBlendEnabled = 0;
			config.ppEnabled = 0;
		}
//		printf("maxDecodingWidth:%d, maxPpOutWidth:%d\n", config.maxDecodingWidth, config.maxPpOutWidth);

		munmap(io, pgsize);
	}

	close(fd);
	close(fd_dec);

	return config;
}
/*------------------------------------------------------------------------------
	Function name   : DWLMallocRefFrm
	Description     : Allocate a frame buffer (contiguous linear RAM memory)

	Return type     : i32 - 0 for success or a negative error code

	Argument        : const void * instance - DWL instance
	Argument        : u32 size - size in bytes of the requested memory
	Argument        : void *info - place where the allocated memory buffer
						parameters are returned
------------------------------------------------------------------------------*/
i32 DWLMallocRefFrm(const void *instance, u32 size, DWLLinearMem_t * info)
{
	return DWLMallocLinear(instance, size, info);
}

/*------------------------------------------------------------------------------
	Function name   : DWLFreeRefFrm
	Description     : Release a frame buffer previously allocated with
						DWLMallocRefFrm.

	Return type     : void

	Argument        : const void * instance - DWL instance
	Argument        : void *info - frame buffer memory information
------------------------------------------------------------------------------*/
void DWLFreeRefFrm(const void *instance, DWLLinearMem_t * info)
{
	DWLFreeLinear(instance, info);
}
/*------------------------------------------------------------------------------
	Function name   : DWLMallocLinear
	Description     : Allocate a contiguous, linear RAM  memory buffer

	Return type     : i32 - 0 for success or a negative error code

	Argument        : const void * instance - DWL instance
	Argument        : u32 size - size in bytes of the requested memory
	Argument        : void *info - place where the allocated memory buffer
						parameters are returned
------------------------------------------------------------------------------*/
i32 DWLMallocLinear(const void *instance, u32 size, DWLLinearMem_t * info)
{
	instance = instance;
	return DWLSysbufMalloc(size,info);
}

/*------------------------------------------------------------------------------
	Function name   : DWLFreeLinear
	Description     : Release a linera memory buffer, previously allocated with
						DWLMallocLinear.

	Return type     : void

	Argument        : const void * instance - DWL instance
	Argument        : void *info - linear buffer memory information
------------------------------------------------------------------------------*/
void DWLFreeLinear(const void *instance, DWLLinearMem_t * info)
{
	instance = instance;
	DWLSysbufFree(info);
}

/*------------------------------------------------------------------------------
	Function name   : DWLWriteReg
	Description     : Write a value to a hardware IO register

	Return type     : void

	Argument        : const void * instance - DWL instance
	Argument        : u32 offset - byte offset of the register to be written
	Argument        : u32 value - value to be written out
------------------------------------------------------------------------------*/
void DWLWriteReg(const void *instance, u32 offset, u32 value)
{
	hX170dwl_t *dec_dwl = (hX170dwl_t*)instance;

	offset /= 4;
	*(dec_dwl->pRegBase + offset) = value;
	
	if(1)
	{
#ifdef REG_RW_TRACE
		u32 val = *(dec_dwl->pRegBase + offset);
		printf("W,%3d: 0x%08x <= 0x%08x", offset, HX170DEC_IO_BASE+offset*4, value);
		if(val == value)
			printf("\n");
		else
			printf(", 0x%08x\n", val);
#endif
	}
}

/*------------------------------------------------------------------------------
	Function name   : DWLReadReg
	Description     : Read the value of a hardware IO register

	Return type     : u32 - the value stored in the register

	Argument        : const void * instance - DWL instance
	Argument        : u32 offset - byte offset of the register to be read
------------------------------------------------------------------------------*/
u32 DWLReadReg(const void *instance, u32 offset)
{
	hX170dwl_t *dec_dwl = (hX170dwl_t *) instance;
	u32 val;

	offset /= 4;
	val = *(dec_dwl->pRegBase + offset);
	
#ifdef REG_RW_TRACE
	printf("R,%3d: 0x%08x => 0x%08x\n", offset, HX170DEC_IO_BASE+offset*4, val);
#endif
	return val;
}

/*------------------------------------------------------------------------------
	Function name   : DWLmalloc
	Description     : Allocate a memory block. Same functionality as
					  the ANSI C malloc()

	Return type     : void pointer to the allocated space, or NULL if there
					  is insufficient memory available

	Argument        : u32 n - Bytes to allocate
------------------------------------------------------------------------------*/
void *DWLmalloc(u32 n)
{
	void *p = malloc((size_t) n);
//	printf("DWLmalloc(), size:%8d, addr:0x%08x\n", n, (u32)p);
	return p;
}

/*------------------------------------------------------------------------------
	Function name   : DWLfree
	Description     : Deallocates or frees a memory block. Same functionality as
					  the ANSI C free()

	Return type     : void

	Argument        : void *p - Previously allocated memory block to be freed
------------------------------------------------------------------------------*/
void DWLfree(void *p)
{
//	printf("DWLfree(), addr:0x%08x\n", (u32)p);
	if(p != NULL)
		free(p);
}

/*------------------------------------------------------------------------------
	Function name   : DWLcalloc
	Description     : Allocates an array in memory with elements initialized
					  to 0. Same functionality as the ANSI C calloc()

	Return type     : void pointer to the allocated space, or NULL if there
					  is insufficient memory available

	Argument        : u32 n - Number of elements
	Argument        : u32 s - Length in bytes of each element.
------------------------------------------------------------------------------*/
void *DWLcalloc(u32 n, u32 s)
{
	void *p = calloc((size_t) n, (size_t) s);
//	printf("DWLcalloc(), n:%d, s:%d, size:%d, addr:0x%08x", n, s, n * s, (u32)p);
	return p;
}

/*------------------------------------------------------------------------------
	Function name   : DWLmemcpy
	Description     : Copies characters between buffers. Same functionality as
					  the ANSI C memcpy()

	Return type     : The value of destination d

	Argument        : void *d - Destination buffer
	Argument        : const void *s - Buffer to copy from
	Argument        : u32 n - Number of bytes to copy
------------------------------------------------------------------------------*/
void *DWLmemcpy(void *d, const void *s, u32 n)
{
	return memcpy(d, s, (size_t) n);
}

/*------------------------------------------------------------------------------
	Function name   : DWLmemset
	Description     : Sets buffers to a specified character. Same functionality
					  as the ANSI C memset()

	Return type     : The value of destination d

	Argument        : void *d - Pointer to destination
	Argument        : i32 c - Character to set
	Argument        : u32 n - Number of characters
------------------------------------------------------------------------------*/
void *DWLmemset(void *d, i32 c, u32 n)
{
	return memset(d, (int) c, (size_t) n);
}

/*------------------------------------------------------------------------------
	Function name   : DWLFakeTimeout
	Description     : Testing help function that changes HW stream errors info
						HW timeouts. You can check how the SW behaves or not.
	Return type     : void
	Argument        : void
------------------------------------------------------------------------------*/

void DWLFlushCache(const void *instance)
{
	hX170dwl_t *dec_dwl = (hX170dwl_t*)instance;
	if(dec_dwl && dec_dwl->fd != -1)
	{
		int ret;
		ret = ioctl(dec_dwl->fd, HX170DEC_FLUSH_CACHE);
		if(ret == -1)
		{
			int errsv = errno;
			printf("ioctl: errno:%d, %s\n", errsv, strerror(errsv));
		}
	}
}
