/**
 * SPU specific crypto code for HDCP2 XTASK.
 *
 * INTEL CONFIDENTIAL
 * Copyright 2010-2011 Intel Corporation All Rights Reserved.
 *
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation or its
 * suppliers or licensors.  Title to the Material remains with Intel
 * Corporation or its suppliers and licensors.  The Material contains trade
 * secrets and proprietary and confidential information of Intel or its
 * suppliers and licensors.  The Material is protected by worldwide copyright
 * and trade secret laws and treaty provisions. No part of the Material may
 * be used, copied, reproduced, modified, published, uploaded, posted,
 * transmitted, distributed, or disclosed in any way without Intel's prior
 * express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or
 * delivery of the Materials,  either expressly, by implication, inducement,
 * estoppel or otherwise.  Any license under such intellectual property
 * rights must be express and approved by Intel in writing.
 */


/**
 * This file contains source code that interacts with SPU,
 * and hardware ciphers for implementing the HDCP2
 * state machine.
 */

/**
 * Includes
 */
#include "hdcp2_hal.h"

/**
 * For openssl SHA-256 and AES
 */
#include "sha.h"
#include <linux/random.h>



/**
 * Generate 32 bits of random values
 *
 * @retval  Random 32 bit value
 *
 */
H2uint32 crypto_random32( )
{
	H2uint32 res;

	get_random_bytes(&res,sizeof(res));
	
	return (res);
}


/**
 * Initialize a SHA-256 context for use to calculate a SHA-256
 * hash. This must be called first before calling update and final.
 *
 * @param[in]  pCtx  Pointer to SHA-256 context. The contents to the
 *                   context are defined in hdcp2_hal.h
 *
 * @retval  True if successful, False if an error occurred.
 *
 */
H2bool crypto_sha256Init( H2Sha256Ctx_t* pCtx )
{
   bool bStatus = false;

   do
   {
      if( !SHA256_Init( &pCtx->c ) )
      {
		 printk("Failed to init SHA-256 context.\n");
         break;
      }

      bStatus = true;

   } while(0);

   return( bStatus );


   return( TRUE );
}

/**
 * Calculate the SHA-256 hash of the supplied memory buffer. This
 * function can be called multiple times to extend the hash
 * calculation accross multiple buffers. To get the hash, the final
 * function must be called to indicate no more buffers will be supplied.
 *
 * @param[in]  pCtx  Pointer to SHA-256 context. The contents to the
 *                   context are defined in hdcp2_hal.h
 * @param[in]  pBuf  Pointer to memory buffer to calculate hash on.
 * @param[in]  len   Length in bytes of the memory buffer.
 *
 * @retval  True if successful, False if an error occurred.
 *
 */
H2bool crypto_sha256Update( H2Sha256Ctx_t* pCtx, H2uint8 const * pBuf, H2uint32 len )
{
   bool bStatus = false;

   do
   {
      if( !SHA256_Update(&pCtx->c, pBuf, len) )
      {
		 printk("Failed to update SHA-256 hash.\n");
         break;
      }

      bStatus = true;

   } while(0);

   return( bStatus );
}

/**
 * Finalize the SHA-256 hash calculation and retrieve the hash. This
 * function also frees any resources associated with the supplied
 * context. The context must be initialized again before reusing it.
 *
 * @param[in]  pCtx  Pointer to SHA-256 context. The contents to the
 *                   context are defined in hdcp2_hal.h
 * @param[out] pHash Pointer to buffer to copy hash into. The buffer must
 *                   be SHA256_DIGEST_SIZE bytes long.
 *
 * @retval  True if successful, False if an error occurred.
 *
 */
H2bool crypto_sha256Final( H2Sha256Ctx_t* pCtx, H2uint8* pHash )
{
   bool bStatus = false;

   do
   {
      if( !SHA256_Final(pHash, &pCtx->c) )
      {
         printk("Failed to finalize SHA-256 hash.\n");
         break;
      }

      bStatus = true;

   } while(0);

   return( bStatus );
}


/**
 * Writeback a range of cache to RAM. This can be used
 * when we need to destroy local secrets in RAM.
 *
 * @param[in]  pAddr  Starting address of data to writeback
 * @param[in]  size   Number of bytes to write back
 *
 * @return NONE
 *
 */
void crypto_DCacheFlush( void* pAddr, int size )
{
   // This function is stubbed out and optional.

   return;
}

