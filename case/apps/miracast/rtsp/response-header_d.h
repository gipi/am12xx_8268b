/**
 * @file: response-header.d
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

HeaderDecl(RespTokens)
	ItemDecl2(rAllow,Allow)                           // Section 12.4
	ItemDecl2(rLocation,Location)                     // Section 12.25
	ItemDecl2(rProxy_Authenticate,Proxy-Authenticate) // Section 12.26
	ItemDecl2(rPublic,Public)                         // Section 12.28
	ItemDecl2(rRange,Range)                           // Section 12.29
	ItemDecl2(rRetry_After,Retry-After)               // Section 12.31
	ItemDecl2(rRTP_Info,RTP-Info)                     // Section 12.33
	ItemDecl2(rScale,Scale)                           // Section 12.34
	ItemDecl2(rSession,Session)                       // Section 12.37
	ItemDecl2(rServer,Server)                         // Section 12.36
	ItemDecl2(rSpeed,Speed)                           // Section 12.35
	ItemDecl2(rTransport,Transport)                   // Section 12.39
	ItemDecl2(rUnsupported,Unsupported)               // Section 12.40
	ItemDecl2(rVary,Vary)                             // Section 12.42
	ItemDecl2(rWWW_Authenticate,WWW-Authenticate)     // Section 12.44
TailDecl(RespTokens)

