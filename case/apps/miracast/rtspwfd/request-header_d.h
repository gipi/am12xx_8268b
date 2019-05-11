/**
 * @file: request-header.d
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

HeaderDecl(WFD_ReqTokens)
	ItemDecl2(RAccept,Accept)                       // Section 12.1
	ItemDecl2(RAccept_Encoding,Accept-Encoding)     // Section 12.2
	ItemDecl2(RAccept_Language,Accept-Language)     // Section 12.3
	ItemDecl2(RAuthorization,Authorization)         // Section 12.5
	ItemDecl2(RBandwidth,Bandwidth)                 // Section 12.6
	ItemDecl2(RBlocksize,Blocksize)                 // Section 12.7
	ItemDecl2(RConference,Conference)               // Section 12.9
	ItemDecl2(RFrom,From)                           // Section 12.20
	ItemDecl2(RIf_Modified_Since,If-Modified-Since) // Section 12.23
	ItemDecl2(RProxy_Require,Proxy-Require)         // Section 12.27
	ItemDecl2(RRange,Range)                         // Section 12.29
	ItemDecl2(RReferer,Referer)                     // Section 12.30
	ItemDecl2(RRequire,Require)                     // Section 12.32
	ItemDecl2(RScale,Scale)                         // Section 12.34
	ItemDecl2(RSession,Session)                     // Section 12.37
	ItemDecl2(RSpeed,Speed)                         // Section 12.35
	ItemDecl2(RTransport,Transport)                 // Section 12.39
	ItemDecl2(RUser_Agent,User-Agent)               // Section 12.41
TailDecl(WFD_ReqTokens)

