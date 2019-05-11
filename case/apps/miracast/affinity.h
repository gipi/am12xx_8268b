/**
 * @file affinity.h
 *
 * @brief
 * Header file for affinity.c
 *
 * INTEL CONFIDENTIAL
 * Copyright 2010 Intel Corporation All Rights Reserved.
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

#ifndef _AFFINITY_H
#define _AFFINITY_H

#include "rtsp_abstraction.h"
#include "librtspsink.h"

/**
 * Holds the necessary variables for performing the affinity handshake as
 * defined in the Remote Display Specification 1.0
 */
typedef struct {
    uint8_t  Tn[32];                ///< Transmitter's nonce
    uint8_t  An[32];                ///< Receiver's nonce
    uint8_t  H1[32];                ///< First hash value
    uint8_t  H2[32];                ///< Second hash value
    uint8_t  SharedSecret[16];      ///< Affinity shared secret
    int      affinity_check_passed; ///< 1 if test has passed
} rtsp_affinity_t;

// @cond -- Don't run Doxygen on prototypes
long  rtsp_affinity_check( rtsp_handle hRtspLib, PARAMETER_STRUCT* p );
void  rtsp_affinity_test_vector( void );
char* rtsp_affinity_get_shared_secret( void );
// @endcond

#endif
