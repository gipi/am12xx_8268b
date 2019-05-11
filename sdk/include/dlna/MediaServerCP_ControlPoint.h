/*
 * INTEL CONFIDENTIAL
 * Copyright (c) 2002 - 2005 Intel Corporation.  All rights reserved.
 * 
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel
 * Corporation or its suppliers or licensors.  Title to the
 * Material remains with Intel Corporation or its suppliers and
 * licensors.  The Material contains trade secrets and proprietary
 * and confidential information of Intel or its suppliers and
 * licensors. The Material is protected by worldwide copyright and
 * trade secret laws and treaty provisions.  No part of the Material
 * may be used, copied, reproduced, modified, published, uploaded,
 * posted, transmitted, distributed, or disclosed in any way without
 * Intel's prior express written permission.
 
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you
 * by disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel or otherwise. Any license
 * under such intellectual property rights must be express and
 * approved by Intel in writing.
 * 
 * $Workfile: MediaServerCP_ControlPoint.h
 * $Revision: #1.0.2777.24812
 * $Author:   Intel Corporation, Intel Device Builder
 * $Date:     2011年10月26日
 *
 *
 *
 */
#ifndef __MediaServerCP_ControlPoint__
#define __MediaServerCP_ControlPoint__

#include "UPnPControlPointStructs.h"

/*! \file MediaServerCP_ControlPoint.h 
	\brief MicroStack APIs for Control Point Implementation
*/

/*! \defgroup ControlPoint Control Point Module
	\{
*/


/* Complex Type Parsers */


/* Complex Type Serializers */




/*! \defgroup CPReferenceCounter Reference Counter Methods
	\ingroup ControlPoint
	\brief Reference Counting for the UPnPDevice and UPnPService objects.
	\para
	Whenever a user application is going to keep the pointers to the UPnPDevice object that is obtained from
	the add sink (or any pointers inside them), the application <b>must</b> increment the reference counter. Failure to do so
	will lead to references to invalid pointers, when the device leaves the network.
	\{
*/
void MediaServerCP_AddRef(struct UPnPDevice *device);
void MediaServerCP_Release(struct UPnPDevice *device);
/*! \} */   



struct UPnPDevice* MediaServerCP_GetDevice1(struct UPnPDevice *device,int index);
int MediaServerCP_GetDeviceCount(struct UPnPDevice *device);
struct UPnPDevice* MediaServerCP_GetDeviceEx(struct UPnPDevice *device, char* DeviceType, int start,int number);
void PrintUPnPDevice(int indents, struct UPnPDevice *device);



/*! \defgroup CustomXMLTags Custom XML Tags
	\ingroup ControlPoint
	\brief Methods used to obtain metadata information from a specific UPnPDevice object.
	\{
*/
char *MediaServerCP_GetCustomTagFromDevice(struct UPnPDevice *d, char* FullNameSpace, char* Name);
/*! \def X_DLNACAP
\brief Custom XML Element: Local Name
*/
#define X_DLNACAP "X_DLNACAP"
/*! \def X_DLNACAP_NAMESPACE
\brief Custom XML Element: Fully Qualified Namespace
*/
#define X_DLNACAP_NAMESPACE "urn:schemas-dlna-org:device-1-0"
/*! \def X_DLNADOC
\brief Custom XML Element: Local Name
*/
#define X_DLNADOC "X_DLNADOC"
/*! \def X_DLNADOC_NAMESPACE
\brief Custom XML Element: Fully Qualified Namespace
*/
#define X_DLNADOC_NAMESPACE "urn:schemas-dlna-org:device-1-0"

char *MediaServerCP_GetCustomXML_X_DLNACAP(struct UPnPDevice *d);
char *MediaServerCP_GetCustomXML_X_DLNADOC(struct UPnPDevice *d);

/*! \} */



/*! \defgroup CPAdministration Administrative Methods
	\ingroup ControlPoint
	\brief Basic administrative functions, used to setup/configure the control point application
	\{
*/
void *MediaServerCP_CreateControlPoint(void *Chain, void(*A)(struct UPnPDevice*),void(*R)(struct UPnPDevice*));
void MediaServerCP_ControlPoint_AddDiscoveryErrorHandler(void *cpToken, UPnPDeviceDiscoveryErrorHandler callback);
struct UPnPDevice* MediaServerCP_GetDeviceAtUDN(void *v_CP,char* UDN);
void MediaServerCP__CP_IPAddressListChanged(void *CPToken);
int MediaServerCP_HasAction(struct UPnPService *s, char* action);
void MediaServerCP_UnSubscribeUPnPEvents(struct UPnPService *service);
void MediaServerCP_SubscribeForUPnPEvents(struct UPnPService *service, void(*callbackPtr)(struct UPnPService* service,int OK));
struct UPnPService *MediaServerCP_GetService(struct UPnPDevice *device, char* ServiceName, int length);

void MediaServerCP_SetUser(void *token, void *user);
void* MediaServerCP_GetUser(void *token);

struct UPnPService *MediaServerCP_GetService_ConnectionManager(struct UPnPDevice *device);
struct UPnPService *MediaServerCP_GetService_ContentDirectory(struct UPnPDevice *device);

/*! \} */


/*! \defgroup InvocationEventingMethods Invocation/Eventing Methods
	\ingroup ControlPoint
	\brief Methods used to invoke actions and receive events from a UPnPService
	\{
*/
extern void (*MediaServerCP_EventCallback_ConnectionManager_SourceProtocolInfo)(struct UPnPService* Service,char* SourceProtocolInfo);
extern void (*MediaServerCP_EventCallback_ConnectionManager_SinkProtocolInfo)(struct UPnPService* Service,char* SinkProtocolInfo);
extern void (*MediaServerCP_EventCallback_ConnectionManager_CurrentConnectionIDs)(struct UPnPService* Service,char* CurrentConnectionIDs);
extern void (*MediaServerCP_EventCallback_ContentDirectory_ContainerUpdateIDs)(struct UPnPService* Service,char* ContainerUpdateIDs);
extern void (*MediaServerCP_EventCallback_ContentDirectory_SystemUpdateID)(struct UPnPService* Service,unsigned int SystemUpdateID);

void MediaServerCP_Invoke_ConnectionManager_GetCurrentConnectionIDs(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user,char* ConnectionIDs),void* _user);
void MediaServerCP_Invoke_ConnectionManager_GetCurrentConnectionInfo(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user,int RcsID,int AVTransportID,char* ProtocolInfo,char* PeerConnectionManager,int PeerConnectionID,char* Direction,char* Status),void* _user, int ConnectionID);
void MediaServerCP_Invoke_ConnectionManager_GetProtocolInfo(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user,char* Source,char* Sink),void* _user);
void MediaServerCP_Invoke_ContentDirectory_Browse(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user,char* Result,unsigned int NumberReturned,unsigned int TotalMatches,unsigned int UpdateID),void* _user, char* ObjectID, char* BrowseFlag, char* Filter, unsigned int StartingIndex, unsigned int RequestedCount, char* SortCriteria);
void MediaServerCP_Invoke_ContentDirectory_CreateObject(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user,char* ObjectID,char* Result),void* _user, char* ContainerID, char* Elements);
void MediaServerCP_Invoke_ContentDirectory_DestroyObject(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user),void* _user, char* unescaped_ObjectID);
void MediaServerCP_Invoke_ContentDirectory_GetSearchCapabilities(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user,char* SearchCaps),void* _user);
void MediaServerCP_Invoke_ContentDirectory_GetSortCapabilities(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user,char* SortCaps),void* _user);
void MediaServerCP_Invoke_ContentDirectory_GetSystemUpdateID(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user,unsigned int Id),void* _user);
void MediaServerCP_Invoke_ContentDirectory_Search(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user,char* Result,unsigned int NumberReturned,unsigned int TotalMatches,unsigned int UpdateID),void* _user, char* ContainerID, char* SearchCriteria, char* Filter, unsigned int StartingIndex, unsigned int RequestedCount, char* SortCriteria);

/* The string parameters for the following actions MUST be MANUALLY escaped */
/* 	void MediaServerCP_Invoke_ContentDirectory_Browse */
/* 	void MediaServerCP_Invoke_ContentDirectory_CreateObject */
/* 	void MediaServerCP_Invoke_ContentDirectory_Search */


/*! \} */


/*! \} */
#endif
