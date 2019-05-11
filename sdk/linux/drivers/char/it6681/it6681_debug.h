///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <it6681_debug.h>
//   @author Kenneth.Hung@ite.com.tw
//   @date   2013/12/06
//   @fileversion: ITE_IT6682_1.01
//******************************************/
#ifndef _IT6681_DEBUG_H_
#define _IT6681_DEBUG_H_

///////////////////////////////////////////////////////////////
// IT6811 debug define
//
///////////////////////////////////////////////////////////////
#define IT6681_DEBUG_PRINTF(x)           	debug_6681 x
#define IT6681_DEBUG_INT_PRINTF(x)       	debug_6681 x
#define IT6681_DEBUG_CAP_INFO(x)			debug_6681 x
#define MHL_MSC_DEBUG_PRINTF(x)	      	    debug_6681 x

#define HDMITX_MHL_DEBUG_PRINTF(x)	     	debug_6681 x
#define HDMITX_DEBUG_HDCP_PRINTF(x)			debug_6681 x
#define HDMITX_DEBUG_HDCP_INT_PRINTF(x)		debug_6681 x

#ifdef ENV_ANDROID
    #define debug_6681	pr_err
#else
    //#define debug_6681	printf
#endif

//#define debug_6811(x,...) dev_err(it6811dev,x,##__VA_ARGS__)


#define _SHOW_VID_INFO_	    FALSE
#define _SHOW_HDCP_INFO_	TRUE
#define _IT6681_DUMP_REGISTER_	    TRUE



#endif
