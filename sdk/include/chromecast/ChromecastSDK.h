#ifndef __CHROMECAST_SDK_H__
#define __CHROMECAST_SDK_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*! \brief Codec type
 */
typedef enum {
	CastSDK_CODEC_UNKNOWN 		= 0, //!< no codec.
	CastSDK_CODEC_AUDIO_OPUS 	= 1, //!< opus audio, from Chrome Browser.
	CastSDK_CODEC_AUDIO_PCM16 	= 2, //!< PCM16 audio, only for test.
	CastSDK_CODEC_AUDIO_AAC		= 3, //!< aac audio, from Android Phone/ChromeBook.
	CastSDK_CODEC_VIDEO_FAKE	= 4, //!< fake video, only for test.
	CastSDK_CODEC_VIDEO_VP8		= 5, //!< vp8 video, from Chrome Browser.
	CastSDK_CODEC_VIDEO_H264	= 6, //!< h264 video, from Android Phone/ChromeBook.
	CastSDK_CODEC_LAST			= CastSDK_CODEC_VIDEO_H264
} CastSDKCodec;

typedef struct {
	CastSDKCodec codec;		//!< codec, unknow means there is no audio/video in cast session.
	/*for video*/
	uint32_t video_width;	//!< video width.
	uint32_t video_height;	//!< video height.
	/*for audio*/
	uint32_t channels;		//!< audio channels.
	uint32_t sample_rate;	//!< audio sample rate.
} CastSDKCodecConfig;

typedef struct {
	void (*CastPlatformSetVolume)(float volume); 	//!< callback to set volume.
	void (*CastPlatformSetMute)(int mute);			//!< callback to mute.
	int (*CastMirrorAppLaunched)(const char* clientName); //!< callback when cast session started.
	void (*CastMirrorCodecInfo)(const CastSDKCodecConfig* videoCodec, const CastSDKCodecConfig* audioCodec); //!< callback for video/audio info in cast session.
	void (*CastMirrorVideoEncodedFrame)(const uint8_t* encodedFrame, uint32_t len, int64_t ts); //!< callback for encoded video data.
	void (*CastMirrorAudioEncodedFrame)(const uint8_t* encodedFrame, uint32_t len, int64_t ts); //!< callback for encoded audio data.
	void (*CastMirrorAppStop)();	//!< callback when cast session stoped.
} CastSDKCallback;

/**
 *	Init the CastSDK.
 *  @param[in] callback the User Callback.
 *  @param[in] mac the MAC address of host.
 *  @param[in] model the model name.
 *	@return 0 for no error, other for error.
 */
int CastSDK_Init(const CastSDKCallback* callback, const uint8_t mac[6], const char* model);

/**
 *	Start the Cast Service.
 *  @param[in] friendlyName the name.
 *  @param[in] udn the UDN like in DMR, User should generate it one time and store in preference.
 *  @param[in] dial_port the port used for dial, default should be 8008.
 *  @param[in] port the port used for Cast, default should be 8009.
 *	@return 0 for no error, other for error.
 */
int CastSDK_StartService(const char* friendlyName, const char* udn, const int dial_port, const int port);

/**
 *	Stop the Cast Session.
 *	@return 0 for no error, other for error.
 */
int CastSDK_StopCastMirrorApp();

/**
 *	Stop the Cast Service.
 *	@return 0 for no error, other for error.
 */
int CastSDK_StopService();

#ifdef __cplusplus
}
#endif

#endif//__CHROMECAST_SDK_H__
