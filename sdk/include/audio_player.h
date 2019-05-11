/*************************************************************************
Description: Audio player API. The default libaudio_player.so is implemented
			 by OSS. If your platform doesn't support ALSA/OSS, you can implement
			 your own audio player API and replace libaudio_player.so with yours.

Author:	Goody Wang. ¤ý¿³´é
 
Date created: 02/10/2009

All rights are reserved by AWIND Inc.
*************************************************************************/
#ifndef _AUDIO_PLAYER_H_
#define _AUDIO_PLAYER_H_

#ifdef __cplusplus
extern "C" {
#endif


#ifdef  _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT 
#endif

/**
 * Initialize audio player.
 * @param nSamplesPerSec Number of samples per second, ex. 44100.
 * @param nChannels Number of channels, ex. 2.
 * @param nBitsPerSample Number of bits per sample, ex. 16.
 * @return negative value if failed.
 */
EXPORT int InitAudio(int nSamplesPerSec, int nChannels, int nBitsPerSample);
EXPORT int InitAudioWithBufferSize(int nSamplesPerSec, int nChannels, int nBitsPerSample, int bBufferSize);

/**
 * Send audio data to playback buffer. The default OSS write() implementation
 *   will maintain an underlying FIFO buffer. If FIFO buffer space is enough for
 *   the new data block, this function simply copy data to FIFO buffer and then
 *   returns immediately. If FIFO buffer is not enough, this function will wait
 *   until FIFO buffer is available.
 * @param pAudioData Audio data. 
 * @nSize Size of audio data.
 * @return number of bytes sent. MirrorOp doesn't handle the case if sent size < nSize.
 */
EXPORT int SendAudioBuf(const void *pAudioData, unsigned int nSize);

/**
 * Uninitialize audio player.
 */
EXPORT void UninitAudio();

#if 1 //[Sanders.120628] - Add nonblocking audio play mode

/**
 * Initialize audio player.
 * @param nSamplesPerSec Number of samples per second, ex. 44100.
 * @param nChannels Number of channels, ex. 2.
 * @param nBitsPerSample Number of bits per sample, ex. 16.
 * @return negative value if failed.
 */
EXPORT int InitAudioNonBlock(int nSamplesPerSec, int nChannels, int nBitsPerSample);

/**
 * Send audio data to playback buffer. The default OSS write() implementation
 *   will maintain an underlying FIFO buffer. If FIFO buffer space is enough for
 *   the new data block, this function simply copy data to FIFO buffer and then
 *   returns immediately. If FIFO buffer is not enough, this function will wait
 *   until FIFO buffer is available.
 * @param pAudioData Audio data. 
 * @nSize Size of audio data.
 * @return number of bytes sent. MirrorOp doesn't handle the case if sent size < nSize.
 */
EXPORT int SendAudioBufNonBlock(const void *pAudioData, unsigned int nSize);

/**
 * Uninitialize audio player.
 */
EXPORT void UninitAudioNonBlock();

#endif

#ifdef __cplusplus
}
#endif

#endif	// _AUDIO_PLAYER_H_
