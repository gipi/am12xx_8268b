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
--  Description : Locking semaphore for hardware sharing
--
------------------------------------------------------------------------------
--
--  Version control information, please leave untouched.
--
--  $RCSfile: dwl_linux_lock.c,v $
--  $Revision: 1.2 $
--  $Date: 2007/03/30 11:38:59 $
--
------------------------------------------------------------------------------*/

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "dwl_linux_lock.h"

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short int *array;
    struct seminfo *__buf;
};

/* Obtain a binary semaphore's ID, allocating if necessary.  */

int binary_semaphore_allocation(key_t key)
{
	int semid;
	semid = semget(key, 2, O_RDWR);//0666);//
	if(semid == -1)
	{
		int errsv = errno;
		printf("semget: errno:%d, %s\n", errsv, strerror(errsv));
	//	perror("semget");
		if(errsv == ENOENT || errsv == 0)
		{
			semid = semget(key, 2, IPC_CREAT | O_RDWR);//0666);//
			if(semid != -1)
			{
				union semun argument;
				unsigned short values[2];

				values[0] = 1;
				values[1] = 1;
				argument.array = values;
				if(semctl(semid, 0, SETALL, argument) == -1)
				{
					errsv = errno;
					printf("semctl: errno:%d, %s\n", errsv, strerror(errsv));
				//	perror("semctl");
					semid = -1;
				}
			}
			else
			{
				errsv = errno;
				printf("semget: errno:%d, %s\n", errsv, strerror(errsv));
			//	perror("semget");
			}
		}
		else
		{
			errsv = errno;
			printf("semget: errno:%d, %s\n", errsv, strerror(errsv));
		//	perror("semget");
		}
	}
    return semid;
}

/* Deallocate a binary semaphore.  All users must have finished their
   use.  Returns -1 on failure.  */

int binary_semaphore_deallocate(int semid)
{
    union semun ignored_argument;

    return semctl(semid, 1, IPC_RMID, ignored_argument);
}

/* Wait on a binary semaphore.  Block until the semaphore value is
   positive, then decrement it by one.  */

int binary_semaphore_wait(int semid, int sem_num)
{
    struct sembuf operations[1];
    int ret;
    
    /* Use 'sem_num' semaphore from the set.  */
    operations[0].sem_num = sem_num;
    /* Decrement by 1.  */
    operations[0].sem_op = -1;
    /* Permit undo'ing.  */
    operations[0].sem_flg = SEM_UNDO;
    
    /* signal safe */
#if 1	//////////////////////////////////////////////////////////////////////////
	for(;;)
	{
		ret = semop(semid, operations, 1);
		if(ret == -1)
		{
			int errsv = errno;
		//	printf("semop: errno:%d, %s\n", errsv, strerror(errsv));
			if(errsv == EINTR || errsv == 0)
				continue;
		}
		break;
	}
#else	//////////////////////////////////////////////////////////////////////////
	while((ret=semop(semid, operations, 1)) == -1 && (errno == EINTR || errno == 0))
	{/* if error is "error, interrupt", try again */
		printf("wait, errno:%d\n", errno);
		perror("semop");
		continue;
	}
#endif	//////////////////////////////////////////////////////////////////////////
	if(ret == -1)
	{
	//	printf("wait, errno:%d\n", errno);
		perror("semop");
	}

    return ret;
}

/* Post to a binary semaphore: increment its value by one.  This
   returns immediately.  */

int binary_semaphore_post(int semid, int sem_num)
{
    struct sembuf operations[1];
    int ret;
    
    /* Use 'sem_num' semaphore from the set.  */
    operations[0].sem_num = sem_num;
    /* Increment by 1.  */
    operations[0].sem_op = 1;
    /* Permit undo'ing.  */
    operations[0].sem_flg = SEM_UNDO;

    /* signal safe */
#if 1	//////////////////////////////////////////////////////////////////////////
	for(;;)
	{
		ret = semop(semid, operations, 1);
		if(ret == -1)
		{
			int errsv = errno;
		//	printf("semop: errno:%d, %s\n", errsv, strerror(errsv));
			if(errsv == EINTR || errsv == 0)
				continue;
		}
		break;
	}
#else	//////////////////////////////////////////////////////////////////////////
	while((ret=semop(semid, operations, 1)) == -1 && (errno == EINTR || errno == 0))
	{/* if error is "error, interrupt", try again */
		printf("post, errno:%d\n", errno);
		perror("semop");
		continue;
	}
#endif	//////////////////////////////////////////////////////////////////////////
	if(ret == -1)
	{
	//	printf("post, errno:%d\n", errno);
		perror("semop");
	}

    return ret;
}

/* Initialize a binary semaphore with a value of one.  */

int binary_semaphore_initialize(int semid)
{
    union semun argument;
    unsigned short values[2];

    values[0] = 1;
    values[1] = 1;
    argument.array = values;
    return semctl(semid, 0, SETALL, argument);
}
