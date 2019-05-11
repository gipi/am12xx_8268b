/**
 * Hardware specific defines
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

#ifndef HDCP2_HAL_H
#define HDCP2_HAL_H

#include <linux/kernel.h>
#include "sha.h"

#define H2uint8  unsigned char
#define H2uint16 unsigned short
#define H2uint32 unsigned int
#define H2uint64 unsigned long long

/* Define boolean types */
#define H2bool   bool

#ifndef TRUE
#define TRUE     true
#define FALSE    false
#endif

/* Define status codes */
typedef enum
{
   H2_OK = 0,
   H2_ERROR

} H2status;

typedef struct H2Sha256Ctx H2Sha256Ctx_t;
/* Define the H2Sha256Ctx structure. */
struct H2Sha256Ctx
{
   SHA256_CTX c;
};

#define SESSIONKEY_SIZE ( 128/8 )
#define MASTERKEY_SIZE  ( 128/8 )
#define KD_SIZE         ( 256/8 )
#define KH_SIZE         ( 128/8 )
#define CTR_SIZE        (  64/8 )
#define DKEY_SIZE       ( 128/8 )
#define STREAM_CTR_SIZE (  32/8 )
#define AES_BLK_SIZE    ( 256/8 )
#define KPUBRX_SIZE (1048/8)
#define KPUBRX_P    (512/8)
#define KPUBRX_Q    (512/8)
#define KPUBRX_dP   (512/8)
#define KPUBRX_dQ   (512/8)
#define KPUBRX_qInv (512/8)

extern H2bool crypto_sha256Init( H2Sha256Ctx_t* pCtx );
extern H2bool crypto_sha256Update( H2Sha256Ctx_t* pCtx, H2uint8 const * pBuf, H2uint32 len );
extern H2bool crypto_sha256Final( H2Sha256Ctx_t* pCtx, H2uint8* pHash );

#endif
