#ifndef DLNA_DMR_RENDERER_H
#define DLNA_DMR_RENDERER_H

#include "DlnaHttp.h"
#include "DlnaHttpClient.h"
#include "DMRCommon.h"
#include "DMR.h"
#define MAX_DEFAULT_MS	10000L // 10 seconds


typedef enum {
	RENDERER_STREAMING = 4,
	RENDERER_INTERACTIVE = 2,
	RENDERER_BACKGROUND = 1,
	RENDERER_UNKNOWN = 0
} RendererTransferMode;


struct  Renderer
{
	ILibThreadPool					_pool;
	ILibWebClient_RequestManager	_request_manager;
	void*							_user;

	DH_TransferStatus				_status;
	BOOL							_dontFireEvent;
	sem_t	 						_timerAbortMRE;	
	sem_t							_timerState;
	pthread_mutex_t                 mutex;

	BOOL							_finishConsumingBytes;
	BOOL							_timeExpired;
	BOOL							_bytesConsumed;
	long							_currentTime;
	long							_duration;
	//char*							_uri;
	BOOL							_paused;
	BOOL							_playing;
	BOOL							_seeking;
	
	void (*LpStateChange)(DMR instance,DMR_PlayState newState, long currentDuration, long maxDuration, BOOL needNextTrack, void* user);
	void(* Play)(DMR instance,char* uri, long duration, RendererTransferMode mode);
	void(*Pause)(DMR instance);
	void(*Stop)(DMR instance);
	BOOL(* IsPlaying)(struct Renderer *renderer);
	BOOL(* getFinishConsumingBytes)(struct Renderer*renderer);
	void(*setFinishConsumingBytes)(BOOL val/*,struct Renderer*renderer*/);
	void(* ReflectOnResponseDone)(DH_TransferStatus transfer_status_handle, enum DHC_Errors dhc_error_code, int http_error_code, const char* error_message, void *user_obj);
	void(*TimerThread)(DMR instance);
	void(*OnTimer)(DMR instance);
	void(* OnResponseDone)(struct Renderer *renderer,DH_TransferStatus transfer_status_handle, enum DHC_Errors dhc_error_code, int http_error_code, const char* error_message);
	void(* SetupTimer)(DMR instance);
	void(*DestroyTimer)(struct Renderer *renderer);
	void(* StartTimer)(struct Renderer *renderer);
	void(* StopTimer)(struct Renderer *renderer);
	char*(* GetUriClone)(struct Renderer *renderer);
	void(*FireCallback)(DMR instance,DMR_PlayState state, BOOL needNextTrack,struct Renderer*renderer);
	void(*Lock)(struct Renderer *renderer);
	void(* Unlock)(struct Renderer *renderer);
	void(*ConsumeBytes)(char* uri, /*RendererTransferMode mode,struct Renderer*renderer*/enum DH_TransferModes mode);
	void (* GotoNextTrack)(DMR instance);
	void(*Seek)(DMR instance, int (*seek_play_in_rending)(long),long position);

};
/*
extern BOOL IsPlaying(struct Renderer *renderer);
extern BOOL getFinishConsumingBytes();
extern void setFinishConsumingBytes(BOOL val);
extern void ReflectOnResponseDone(DH_TransferStatus transfer_status_handle, enum DHC_Errors dhc_error_code, int http_error_code, const char* error_message, void *user_obj);
extern void TimerThread();
extern void OnTimer();
extern void OnResponseDone(struct Renderer*renderer,DH_TransferStatus transfer_status_handle, enum DHC_Errors dhc_error_code, int http_error_code, const char* error_message);
extern void SetupTimer(DMR instance);
extern void DestroyTimer(struct Renderer*renderer);
extern void StartTimer();
extern char* GetUriClone(struct Renderer*renderer);
extern void FireCallback(DMR instance,DMR_PlayState state, BOOL needNextTrack,struct Renderer *renderer);
extern void Lock(struct Renderer*renderer);
extern void Unlock(struct Renderer*renderer);
extern void ConsumeBytes(char* uri, enum DH_TransferModes mode);
extern void GotoNextTrack(DMR instance);
*/
BOOL IsPlaying(struct Renderer *renderer);
void GotoNextTrack(DMR instance);
void DestructRenderer();
void RendererInit(struct Renderer*renderer,void* stateChangeCallback, ILibWebClient_RequestManager manager, ILibThreadPool pool, void* user);

#endif

