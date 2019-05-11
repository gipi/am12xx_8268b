/**
 * @file: entity-header.d
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

HeaderDecl(WFD_EntTokens)
	ItemDecl2(EContent_Base,Content-Base)         // Section 12.11
	ItemDecl2(EContent_Encoding,Content-Encoding) // Section 12.12
	ItemDecl2(EContent_Language,Content-Language) // Section 12.13
	ItemDecl2(EContent_Length,Content-Length)     // Section 12.14
	ItemDecl2(EContent_Location,Content-Location) // Section 12.15
	ItemDecl2(EContent_Type,Content-Type)         // Section 12.16
	ItemDecl2(EExpires,Expires)                   // Section 12.19
	ItemDecl2(ELast_Modified,Last-Modified)       // Section 12.24
TailDecl(WFD_EntTokens)

