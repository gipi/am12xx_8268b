/**
 * @file: method.d
 * @brief Part of the RDS library.
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

#include "common_d.h"

HeaderDecl(WFD_MethodTokens)
	ItemDecl2(MDESCRIBE,DESCRIBE)           // Section 10.2
	ItemDecl2(MANNOUNCE,ANNOUNCE)           // Section 10.3
	ItemDecl2(MGET_PARAMETER,GET_PARAMETER) // Section 10.8
	ItemDecl2(MOPTIONS,OPTIONS)             // Section 10.1
	ItemDecl2(MPAUSE,PAUSE)                 // Section 10.6
	ItemDecl2(MPLAY,PLAY)                   // Section 10.5
	ItemDecl2(MRECORD,RECORD)               // Section 10.11
	ItemDecl2(MREDIRECT,REDIRECT)           // Section 10.10
	ItemDecl2(MSETUP,SETUP)                 // Section 10.4
	ItemDecl2(MSET_PARAMETER,SET_PARAMETER) // Section 10.9
	ItemDecl2(MTEARDOWN,TEARDOWN)           // Section 10.7
TailDecl(WFD_MethodTokens)
