/*
**  UTF-8，Please open this file by UTF-8 NO BOM format.
*/

#ifndef __IMIRROR_H__
#define __IMIRROR_H__

/*
**  定义imirror_ValeriaStart()传入的callback函数的command。
*/
enum{
	/*
	** callback函数传入该消息时，则表示已经与iPhone/iPad连接并认证成功，此时需要准备mirroring.
	** 此时需要做的是释放一些资源，准备进入iPhone/iPad mirroring，如：stop wifi_display， SWF engine sleep 等；
	*/
	PLUGPLAY_CMD_IDEVICEREAD = 0,
	
	/*
	** 询问是否准备好可以mirroring，如果准备好则返回非0值，否则返回0；
	*/
	PLUGPLAY_CMD_ISMIRRORREAD,
	
	/*
	** iPhone/iPad mirroring结束，此时可以恢复之前被释放的资源，如：start wifi_display, SWF engine wakeup 等；
	** callback收到该消息时，将传入PLUGPLAY_CMD_INIT_DECODER时初始化的struct wireDecodeInfo_t类型结构体；
	*/
	PLUGPLAY_CMD_MIRRORSTOP,
	
	/*
	** 获取解码相关信息；
	** callback将传入一个struct wireDecodeInfo_t 结构体指针；
	** 该指针已经指向现有的struct wireDecodeInfo_t类型结构体，请勿重新malloc该结构体；
	** callback收到该消息时，需要对该结构体进行初始化。
	*/
	PLUGPLAY_CMD_INIT_DECODER
};

typedef enum {
	IMIRROR_MODE_CAMERA,
	IMIRROR_MODE_DOCKING
}imirror_mode_t;

typedef enum {
	PKGFORMAT_PCM = 5,
	PKGFORMAT_H264 = 7,
}pkg_format_t;

typedef struct {
	/*
	** user data, 根据实现情况定义。
	*/
	void *userData;
	double width;
	double height;
	imirror_mode_t mode;
	
	/*
	** malloc & free 函数，用于malloc video frame buffer，供解码器解码，解码结束后，libimirror将使用对应的free函数将其释放；
	** malloc & free 函数可以置NULL，此时将使用系统的malloc & free；
	** malloc & free 需要配套设置，当基本一个设置为NULL，另一个不为NULL时，将发生错误。
	*/
	void *(*malloc)(unsigned int);
	void (*free)(void *);
	
	/*
	** Audio解码相关函数；
	** initAudio 参数对应为：SampleRate, ChannelsPerFrame, BitsPerChannel；
	** sendAudioBuff 参数对应为：buffer, buffer_size；
	*/
	int (*initAudio)(int SampleRate, int ChannelsPerFrame, int BitsPerChannel);
	int (*sendAudioBuff)(const void*buffer, unsigned intbuffer_size);
	int (*sendAudioBuffWithPts)(const void*buffer, unsigned intbuffer_size, uint64_t pts);
	void (*uninitAudio)();
	
	/*
	** Video解码相关函数；
	** sendVideoCodec 参数对应：SPS/PPS buffer, buffer_size，用于传入SPS/PPS，每次SPS/PPS有变化时，执行该函数；
	** sendVideoBuff 参数对应为：buffer, buffer_size, width, height, rotate，该函数将传入一帧不带SPS/PPS的H264 buffer；
	*/
	int (*initVideo)();
	void (*uninitVideo)();
	int (*sendVideoBuff)(unsigned char *buffer, unsigned int buffer_size, unsigned long width, unsigned long height, unsigned int rotate);
	int (*sendVideoCodec)(unsigned char *SPSPPS, unsigned int SPSPPS_size);


	unsigned int limitResolutionProduct;
	int (*initStreaming)(int (*read)(void *handle, unsigned char *data, int size));
	void (*uninitStreaming)();
	int (*streamingSeek)(unsigned int millisec);
	int (*streamingSetAudioTime)(unsigned int millisec);
	int (*streamingPause)();
	int (*streamingResume)();
}wireDecodeInfo_t;

/*
** imirror_ValeriaStart 函数将创建一个新的线程，执行usbmuxd start, 检测认证，开始mirroring等；
** imirror_ValeriaStart 函数需要传入callback的函数指针，用于在不同阶段调用；
*/
void imirror_ValeriaStart(int (*callbackFun)(int, void *));
void imirror_ValeriaStop();

/*
** 检测当前是否处于Valeria模式，只有在Valeria模式下，才可以进行iPhone/iPad mirroring。
*/
int imirror_ValeriaCheck(libusb_device *dev);

/*
** 切换到Valeria模式
*/
int imirror_ValeriaSwitch(libusb_device *dev, int enable);

typedef struct amPkgHeader_t {
    int count;
    int totalSize;
	uint64_t pts;
    pkg_format_t format;
    int width;
    int height;
    int size;   
}amPkgHeader_t;

#endif
