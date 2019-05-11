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
--  Description : System Wrapper Layer for Linux.
--                Polling version so the kernel driver is not needed.
--
------------------------------------------------------------------------------
--
--  Version control information, please leave untouched.
--
--  $RCSfile: dwl_x170_linux_no_drv.c,v $
--  $Revision: 1.5 $
--  $Date: 2007/04/02 06:51:19 $
--
------------------------------------------------------------------------------*/

#include "basetype.h"
#include "dwl.h"
#include "dwl_linux.h"

#include "hx170dec.h"
#include "memalloc.h"
#include "dwl_linux_lock.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/timeb.h>

#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#include <pthread.h>
#include <errno.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_EFENCE
#include "efence.h"
#endif

#define DEC_MODULE_PATH "/tmp/dev/hx170"

const char *dec_dev = DEC_MODULE_PATH;

/* a mutex protecting the wrapper init */
pthread_mutex_t x170_init_mutex = PTHREAD_MUTEX_INITIALIZER;

/*------------------------------------------------------------------------------
    Function name   : DWLInit
    Description     : Initialize a DWL instance

    Return type     : const void * - pointer to a DWL instance

    Argument        : DWLInitParam_t * param - initialization params
------------------------------------------------------------------------------*/
const void *DWLInit(DWLInitParam_t * param)
{
    hX170dwl_t *dec_dwl;
    unsigned long base;

    assert(param != NULL);

    dec_dwl = (hX170dwl_t *) malloc(sizeof(hX170dwl_t));

    if(dec_dwl == NULL)
    {
        DWL_DEBUG("Init: failed to allocate an instance\n");
        return NULL;
    }

    pthread_mutex_lock(&x170_init_mutex);
    dec_dwl->clientType = param->clientType;

    switch (dec_dwl->clientType)
    {
    case DWL_CLIENT_TYPE_H264_DEC:
    case DWL_CLIENT_TYPE_MPEG4_DEC:
    case DWL_CLIENT_TYPE_JPEG_DEC:
    case DWL_CLIENT_TYPE_PP:
    case DWL_CLIENT_TYPE_VC1_DEC:
    case DWL_CLIENT_TYPE_MPEG2_DEC:
        break;
    default:
        DWL_DEBUG("DWL: Unknown client type no. %d\n", dec_dwl->clientType);
        goto err;
    }

    dec_dwl->fd_mem = -1;
    dec_dwl->fd_memalloc = -1;
    dec_dwl->pRegBase = MAP_FAILED;
	dec_dwl->pCMURegs = MAP_FAILED;

    /* open mem device for memory mapping */
    dec_dwl->fd_mem = open("/dev/mem", O_RDWR | O_SYNC);

    if(dec_dwl->fd_mem == -1)
    {
        DWL_DEBUG("DWL: failed to open: %s\n", "/dev/mem");
        goto err;
    }

    dec_dwl->fd = open(dec_dev, O_RDWR);
    if(dec_dwl->fd == -1)
    {
        DWL_DEBUG("DWL: failed to open '%s'\n", dec_dev);
        goto err;
    }

    ioctl(dec_dwl->fd, HX170DEC_IOCGHWOFFSET, &base);
    /* map the hw registers to user space */
    dec_dwl->pRegBase = (u32*)mmap(0, HX170DEC_REG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dec_dwl->fd_mem, base);
    if(dec_dwl->pRegBase == MAP_FAILED)
    {
        DWL_DEBUG("DWL: Failed to mmap regs\n");
        goto err;
    }

	dec_dwl->pCMURegs = (u32*)mmap(0, 256, PROT_READ | PROT_WRITE, MAP_SHARED, dec_dwl->fd_mem, 0x10010000);
	if(dec_dwl->pCMURegs == MAP_FAILED)
	{
		DWL_DEBUG("DWL: Failed to mmap control regs\n");
		goto err;
	}

    dec_dwl->regSize = HX170DEC_REG_SIZE;

    DWL_DEBUG("DWL: regs size %d bytes, virt %08x\n", dec_dwl->regSize, (u32)dec_dwl->pRegBase);

	if(1)
	{
		/* use ASIC ID as the shared sem key */
	//	key_t key = DWLReadReg(dec_dwl, 0x00);
		key_t key = ftok(dec_dev, 1);
	//	printf("key: 0x%08x\n", key);
		if(key == -1)
		{
			perror("ftok");
			goto err;
		}
		int semid = binary_semaphore_allocation(key);
		if(semid == -1)
		{
			DWL_DEBUG("DWL: FAILED to get HW lock sem 0x%08x, PID %d\n", key, getpid());
			goto err;
		}
		DWL_DEBUG("DWL: HW lock sem 0x%08x aquired, PID %d\n", key, getpid());
		dec_dwl->semid = semid;
	}

    pthread_mutex_unlock(&x170_init_mutex);
    return dec_dwl;

  err:
    pthread_mutex_unlock(&x170_init_mutex);
    DWLRelease(dec_dwl);
    return NULL;
}

/*------------------------------------------------------------------------------
    Function name   : DWLRelease
    Description     : Release a DWl instance

    Return type     : i32 - 0 for success or a negative error code

    Argument        : const void * instance - instance to be released
------------------------------------------------------------------------------*/
i32 DWLRelease(const void *instance)
{
    hX170dwl_t *dec_dwl = (hX170dwl_t *) instance;

    assert(dec_dwl != NULL);

    pthread_mutex_lock(&x170_init_mutex);
    if(dec_dwl == NULL)
        return DWL_ERROR;

	if(dec_dwl->pRegBase != MAP_FAILED)
	{
		munmap((void*)dec_dwl->pRegBase, HX170DEC_REG_SIZE);
		dec_dwl->pRegBase = MAP_FAILED;
	}

	if(dec_dwl->pCMURegs != MAP_FAILED)
	{
		munmap((void*)dec_dwl->pCMURegs, 256);
		dec_dwl->pCMURegs = MAP_FAILED;
	}

    if(dec_dwl->fd_mem != -1)
        close(dec_dwl->fd_mem);

    if(dec_dwl->fd != -1)
        close(dec_dwl->fd);

    /* linear memory allocator */
    if(dec_dwl->fd_memalloc != -1)
        close(dec_dwl->fd_memalloc);

    free(dec_dwl);

    pthread_mutex_unlock(&x170_init_mutex);

    return (DWL_OK);
}

/*------------------------------------------------------------------------------
    Function name   : DWLWaitDecHwReady
    Description     : Wait until hardware has stopped running.
                      Used for synchronizing software runs with the hardware.
                      The wait can succeed or timeout.

    Return type     : i32 - one of the values DWL_HW_WAIT_OK
                                              DWL_HW_WAIT_TIMEOUT

    Argument        : const void * instance - DWL instance
                      u32 timeout - timeout period for the wait specified in
                                milliseconds; 0 will perform a poll of the
                                hardware status and -1 means an infinit wait
------------------------------------------------------------------------------*/

i32 DWLWaitDecHwReady(const void *instance, u32 timeout)
{
    return DWLWaitPpHwReady(instance, timeout);
}

/*------------------------------------------------------------------------------
    Function name   : DWLWaitHwReady
    Description     : Wait until hardware has stopped running.
                      Used for synchronizing software runs with the hardware.
                      The wait can succeed or timeout.

    Return type     : i32 - one of the values DWL_HW_WAIT_OK
                                              DWL_HW_WAIT_TIMEOUT

    Argument        : const void * instance - DWL instance
                      u32 timeout - timeout period for the wait specified in
                                milliseconds; 0 will perform a poll of the
                                hardware status and -1 means an infinit wait
------------------------------------------------------------------------------*/

i32 DWLWaitPpHwReady(const void *instance, u32 timeout)
{
    hX170dwl_t *dec_dwl = (hX170dwl_t *) instance;
    volatile u32 irq_stats;

    u32 irqRegOffset;

    assert(dec_dwl != NULL);

    if(dec_dwl->clientType == DWL_CLIENT_TYPE_PP)
        irqRegOffset = HX170PP_REG_START;   /* pp ctrl reg offset */
    else
        irqRegOffset = HX170DEC_REG_START;  /* decoder ctrl reg offset */

    /* wait for decoder */
    irq_stats = DWLReadReg(dec_dwl, irqRegOffset);
    irq_stats = (irq_stats >> 12) & 0xFF;

    if(irq_stats != 0)
    {
        return DWL_HW_WAIT_OK;
    }
    else if(timeout)
    {
        struct timespec polling_time;
        struct timespec remaining_time;
        u32 sleep_time = 0;
        int forever = 0;
        int loop = 1;
        i32 ret = DWL_HW_WAIT_TIMEOUT;
        int sleep_interrupted = 0;
        int sleep_error = 0;
        int sleep_ret = 0;
        int polling_interval = 1;   /* 1ms polling interval */

        if(timeout == (u32) (-1))
        {
            forever = 1;    /* wait forever */
        }

        polling_time.tv_sec = polling_interval / 1000;
        polling_time.tv_nsec = polling_interval - polling_time.tv_sec * 1000;
        polling_time.tv_nsec *= (1000 * 1000);

        do
        {
            time_t actual_sleep_time;
            struct timeb start;
            struct timeb end;

            ftime(&start);
            sleep_ret = nanosleep(&polling_time, &remaining_time);
            ftime(&end);
            actual_sleep_time =
                (end.time * 1000 + end.millitm) - (start.time * 1000 +
                                                   start.millitm);

            DWL_DEBUG("Polling time in milli seconds: %d\n",
                      (u32) polling_time.tv_nsec / (1000 * 1000));
            DWL_DEBUG("Remaining time in milli seconds: %d\n",
                      (u32) remaining_time.tv_nsec / (1000 * 1000));
            DWL_DEBUG("Sleep time in milli seconds: %d\n",
                      (u32) actual_sleep_time);

            if(sleep_ret == -1 && errno == EINTR)
            {
                DWL_DEBUG("Sleep interrupted!\n");
                sleep_interrupted = 1;
            }
            else if(sleep_ret == -1)
            {
                DWL_DEBUG("Sleep error!\n");
                sleep_error = 1;
            }
            else
            {
                DWL_DEBUG("Sleep OK!\n");
                sleep_interrupted = 0;
                sleep_error = 0;
            }

            irq_stats = DWLReadReg(dec_dwl, irqRegOffset);
            irq_stats = (irq_stats >> 12) & 0xFF;

            if(irq_stats != 0)
            {
                ret = DWL_HW_WAIT_OK;
                loop = 0;   /* end the polling loop */
            }
            else if(sleep_error)
            {
                ret = DWL_HW_WAIT_TIMEOUT;
                loop = 0;   /* end the polling loop */
            }
            /* if not a forever wait and sleep was interrupted */
            else if(!forever && sleep_interrupted)
            {

                sleep_time +=
                    (polling_time.tv_sec - remaining_time.tv_sec) * 1000 +
                    (polling_time.tv_nsec -
                     remaining_time.tv_nsec) / (1000 * 1000);

                if(sleep_time >= timeout)
                {
                    ret = DWL_HW_WAIT_TIMEOUT;
                    loop = 0;
                }
                sleep_interrupted = 0;
            }
            /* not a forever wait */
            else if(!forever)
            {
                /* We need to calculate the actual sleep time since
                 * 1) remaining_time does not contain correct value
                 * in case of no signal interrupt
                 * 2) due to system timer resolution the actual sleep
                 * time can be greater than the polling_time
                 */
                /*sleep_time += (polling_time.tv_sec - remaining_time.tv_sec) * 1000
                 * + (polling_time.tv_nsec - remaining_time.tv_nsec) / (1000*1000); */
                /*sleep_time += polling_interval; */
                sleep_time += actual_sleep_time;

                if(sleep_time >= timeout)
                {
                    ret = DWL_HW_WAIT_TIMEOUT;
                    loop = 0;
                }
            }
            DWL_DEBUG("Loop for HW timeout. Total sleep: %dms\n", sleep_time);
//			printf("Loop for HW timeout. Total sleep: %dms\n", sleep_time);
        }
        while(loop);

        return ret;
    }
    else
    {
        return DWL_HW_WAIT_TIMEOUT;
    }
}
/*------------------------------------------------------------------------------
    Function name   : DWLReserveHw
    Description     :
    Return type     : i32
    Argument        : const void *instance
------------------------------------------------------------------------------*/
i32 DWLReserveHw(const void *instance)
{
    i32 ret;
    hX170dwl_t *dec_dwl = (hX170dwl_t*)instance;

	if(dec_dwl)
	{
		/* select which semaphore to use */
		if(dec_dwl->clientType == DWL_CLIENT_TYPE_PP)
		{
			DWL_DEBUG("DWL: PP trying to lock by PID %d\n", getpid());
			ret = binary_semaphore_wait(dec_dwl->semid, 1);
			if(!ret)
			{
				DWL_DEBUG("DWL: PP lock success, PID:%d\n", getpid());
			}
		}
		else
		{
			DWL_DEBUG("DWL: Dec trying to lock by PID %d\n", getpid());
			ret = binary_semaphore_wait(dec_dwl->semid, 0);
			if(!ret)
			{
				DWL_DEBUG("DWL: Dec lock success, PID:%d\n", getpid());
			}
		}
		if(ret)
			return DWL_ERROR;
	}
	
	if(0)
	{
		if(dec_dwl->pCMURegs != MAP_FAILED)
		{
			volatile u32 *regs = dec_dwl->pCMURegs;
			u32 val;
			//clock
			val = regs[0x80/4];
			val &= ~(1UL<<15);
			regs[0x80/4] = val;
			val |= 1UL<<15;
			regs[0x80/4] = val;
			//reset
			val = regs[0x84/4];
			val &= ~(1UL<<10);
			regs[0x84/4] = val;
			val |= 1UL<<10;
			regs[0x84/4] = val;
		}
	}

	return DWL_OK;
}

/*------------------------------------------------------------------------------
	Function name	: DWLReleaseHw
	Description 	:
	Return type 	: void
	Argument		: const void *instance
------------------------------------------------------------------------------*/
void DWLReleaseHw(const void *instance)
{
	i32 ret;
	hX170dwl_t *dec_dwl = (hX170dwl_t*)instance;

	if(dec_dwl)
	{
		if(dec_dwl->clientType == DWL_CLIENT_TYPE_PP)
		{
			DWL_DEBUG("DWL: PP trying to release by %d\n", getpid());
			ret = binary_semaphore_post(dec_dwl->semid, 1);
			if(!ret)
			{
				DWL_DEBUG("DWL: PP released by PID %d\n", getpid());
			}
		}
		else
		{
			DWL_DEBUG("DWL: Dec trying to release by %d\n", getpid());
			ret = binary_semaphore_post(dec_dwl->semid, 0);
			if(!ret)
			{
				DWL_DEBUG("DWL: Dec released by PID %d\n", getpid());
			}
		}
	}
}

/*------------------------------------------------------------------------------
    Function name   : DWLEnableHw
    Description     : Enable hw, here also enable polling (=disable irq)
    Return type     : void
    Argument        : const void *instance
------------------------------------------------------------------------------*/

void DWLEnableHW(const void *instance)
{
	hX170dwl_t *dec_dwl = (hX170dwl_t *) instance;
	u32 val;

	val = DWLReadReg(dec_dwl, HX170DEC_REG_START);
//	val = val | 0x1 | (1 << 4);
	val = val | 0x13;
	DWLWriteReg(dec_dwl, HX170DEC_REG_START, val);
}
