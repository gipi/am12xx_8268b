/*------------------------------------------------------------------------------
--																			  --
--		 This software is confidential and proprietary and may be used		  --
--		  only as expressly authorized by a licensing agreement from		  --
--																			  --
--							  Hantro Products Oy.							  --
--																			  --
--					 (C) COPYRIGHT 2006 HANTRO PRODUCTS OY					  --
--							  ALL RIGHTS RESERVED							  --
--																			  --
--				   The entire notice above must be reproduced				  --
--					on all copies and should not be removed.				  --
--																			  --
--------------------------------------------------------------------------------
--
--	Description : Sytem Wrapper Layer
--
------------------------------------------------------------------------------
--
--	Version control information, please leave untouched.
--
--	$RCSfile: dwl.h,v $
--	$Revision: 1.5 $
--	$Date: 2007/04/03 17:09:33 $
--
------------------------------------------------------------------------------*/
#ifndef __DWL_H__
#define __DWL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "basetype.h"

#define DWL_OK						0
#define DWL_ERROR				   -1

#define DWL_HW_WAIT_OK				DWL_OK
#define DWL_HW_WAIT_ERROR			DWL_ERROR
#define DWL_HW_WAIT_TIMEOUT 		1

#define DWL_CLIENT_TYPE_H264_DEC		1U
#define DWL_CLIENT_TYPE_MPEG4_DEC		2U
#define DWL_CLIENT_TYPE_JPEG_DEC		3U
#define DWL_CLIENT_TYPE_PP				4U
#define DWL_CLIENT_TYPE_VC1_DEC 		5U
#define DWL_CLIENT_TYPE_MPEG2_DEC		7U
#define DWL_CLIENT_TYPE_RV_DEC			8U

	/* Linear memory area descriptor */
	typedef struct DWLLinearMem
	{
		u32 *virtualAddress;
		u32 busAddress;
		u32 size;
	} DWLLinearMem_t;

	/* DWLInitParam is used to pass parameters when initializing the DWL */
	typedef struct DWLInitParam
	{
		u32 clientType;
		void *pHeap;
		u32 reg_rw_trace;
		u32 mem_usage_trace;
		u32 hw_reset_enable;
	} DWLInitParam_t;

	/* Hardware configuration description */

	typedef struct DWLHwConfig
	{
		u32 maxDecodingWidth; /* Maximum video decoding width supported  */
		u32 maxPpOutWidth; /* Maximum output width of Post-Processor */
		u32 h264Enabled;  /* HW supports h.264 */
		u32 jpegEnabled;  /* HW supports JPEG */
		u32 mpeg4Enabled;  /* HW supports MPEG-4 */
		u32 mpeg1Enabled;  /* HW supports MPEG-1 *///by zhangshj
		u32 mpeg2Enabled;  /* HW supports MPEG-2 */ //by zhangshj
		u32 vc1Enabled;  /* HW supports VC-1 */
		u32 ppEnabled;	/* HW supports post-processor */
		u32 ppBlendEnabled; /* HW supports alpha blending in post-processor */
	}DWLHwConfig_t;

/* HW ID retriving, static implementation */
	u32 DWLReadAsicID(void);

/* HW configuration retrieving, static implementation */
	DWLHwConfig_t DWLReadAsicConfig(void);

/* DWL initilaization and release */
	const void *DWLInit(DWLInitParam_t * param);
	i32 DWLRelease(const void *instance);

/* HW sharing */
	i32 DWLReserveHw(const void *instance);
	void DWLReleaseHw(const void *instance);

/* Frame buffers memory */
	i32 DWLMallocRefFrm(const void *instance, u32 size, DWLLinearMem_t * info);
	void DWLFreeRefFrm(const void *instance, DWLLinearMem_t * info);

/* SW/HW shared memory */
	i32 DWLMallocLinear(const void *instance, u32 size, DWLLinearMem_t * info);
	void DWLFreeLinear(const void *instance, DWLLinearMem_t * info);

/* D-Cache coherence */
	void DWLDCacheRangeFlush(const void *instance, DWLLinearMem_t * info); /* NOT in use */
	void DWLDCacheRangeRefresh(const void *instance, DWLLinearMem_t * info); /* NOT in use */

/* Register access */
	void DWLWriteReg(const void *instance, u32 offset, u32 value);
	u32 DWLReadReg(const void *instance, u32 offset);

	void DWLWriteRegAll(const void *instance, const u32 * table, u32 size); /* NOT in use */
	void DWLReadRegAll(const void *instance, u32 * table, u32 size); /* NOT in use */

/* HW starting */
	void DWLEnableHW(const void *instance);

/* HW synchronization, only pp interrupt is accepted */
	i32 DWLWaitPpHwReady(const void *instance, u32 timeout);
/* HW synchronization, only decoder interrupt is accepted */
	i32 DWLWaitDecHwReady(const void *instance, u32 timeout);

/* SW/SW shared memory */
	void *DWLmalloc(u32 n);
	void DWLfree(void *p);
	void *DWLcalloc(u32 n, u32 s);
	void *DWLmemcpy(void *d, const void *s, u32 n);
	void *DWLmemset(void *d, i32 c, u32 n);

/* Set PP Standalone flag*/
	void DWLSetPpStandalone(const void *instance, int enable);

	void DWLCacheWBInv(const void *instance, u32 bus_addr, u32 size);

	void DWLCacheWBInvAll(const void *instance);

#ifdef __cplusplus
}
#endif

#endif /* __DWL_H__ */
