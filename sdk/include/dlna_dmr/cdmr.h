#ifndef DLNA_DMR_CDMR_H
#define DLNA_DMR_CDMR_H

#include "DMR.h"
#include "DMRCommon.h"

struct CDMR
{
	BOOL  _setup ;
	void* _chain;
	DMR _dmr;
	BOOL _started ;
	//void (*Setup)(struct CDMR *cdmr,void* chain, int port, char* friendlyName, char* serialNumber, char* UDN, char* protocolInfo, ILibThreadPool threadPool,long (*get_player_total_time)(int),long (*get_player_play_time)(void));
	void(*Start)(struct CDMR *cdmr);
	void(*Stop)(struct CDMR *cdmr);
	void(*IsStarted)(struct CDMR *cdmr);
	void(*GetChain)(struct CDMR *cdmr);

	/* DMR Methods */
	void(*NotifyMicrostackOfIPAddressChange)(struct CDMR*cdmr);
	DMR_Error (*SetEventContextMask)(DMR_EventContextSwitch bitFlags,struct CDMR*cdmr);
	void(*ErrorEventResponse)(void* session, int errorCode, char* errorMessage);
	BOOL(*AddPresetNameToList)(const char* name,DMR dmr);
	DMR_Error (* ChangeSinkProtocolInfo)(char* info,DMR instance);
	DMR_Error (*ChangeTransportPlayState)(DMR_PlayState state,DMR instance);
	DMR_Error (*ChangeTransportPlaySpeed)(char* playSpeed,DMR instance);
	DMR_Error (*ChangeTransportStatus)(DMR_TransportStatus status,DMR instance);
	DMR_Error (*ChangeCurrentTransportActions)(unsigned short allowedActions,DMR instance);
	DMR_Error (*ChangeNumberOfTracks)(unsigned int maxNumberOfTracks,DMR instance);
	DMR_Error (*ChangeCurrentTrack)(unsigned int index,DMR instance);
	DMR_Error (*ChangeCurrentPlayMode)(DMR_MediaPlayMode mode,DMR instance);
	DMR_Error (*ChangeCurrentTrackURI)(char* trackURI,DMR instance);
	DMR_Error (*ChangeCurrentTrackMetaData)(struct CdsObject* trackMetadata,DMR instance);
	DMR_Error (*ChangeCurrentTrackDuration)(long duration,DMR instance);

#if defined(INCLUDE_FEATURE_VOLUME)
	DMR_Error (*ChangeVolume)(unsigned char volume,DMR instance);
	DMR_Error (*ChangeMute)(BOOL mute,DMR instance);
#endif /* INCLUDE_FEATURE_VOLUME */

#if defined(INCLUDE_FEATURE_DISPLAY)
	DMR_Error (*ChangeContrast)(unsigned char contrast,DMR instance);
	DMR_Error (*ChangeBrightness)(unsigned char brightness,DMR instance);
#endif /* INCLUDE_FEATURE_DISPLAY */

	DMR_Error (*ChangeAVTransportURI)(char* uri,DMR instance);
	DMR_Error (*ChangeAVTransportURIMetaData)(struct CdsObject* metadata,DMR instance);
	DMR_Error (*ChangeCurrentMediaDuration)(long duration,DMR instance);
	DMR_Error (*ChangeAbsoluteTimePosition)(long position,DMR instance);
	DMR_Error (*ChangeRelativeTimePosition)(long position,DMR instance);

};

void CdmrInit(struct CDMR*cdmr);
void Setup(struct CDMR *cdmr,void* chain, int port, char* friendlyName, char* serialNumber, char* UDN, char* protocolInfo, ILibThreadPool threadPool,long (*get_player_total_time)(int),long (*get_player_play_time)(void));
DMR_Error ChangeCurrentPlayMode(DMR_MediaPlayMode mode,DMR instance);

#endif
