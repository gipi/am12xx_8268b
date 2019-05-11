/**
 * @file: general-header.d
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

HeaderDecl(WFD_GenTokens)
	ItemDecl2(GCache_Control,Cache-Control)      // Section 12.8
	ItemDecl2(GConnection,Connection)            // Section 12.10
	ItemDecl2(GCSeq,CSeq)                        // Section 12.17
	ItemDecl2(GDate,Date)                        // Section 12.18
	ItemDecl2(GVia,Via)                          // Section 12.43
TailDecl(WFD_GenTokens)

