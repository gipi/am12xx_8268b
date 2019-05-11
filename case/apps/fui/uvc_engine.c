/*
* V4L2 video capture functions for actions micro
* date:2014/04/28
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h> /* getopt_long() */
#include <fcntl.h> /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h> /* for videodev2.h */
#include <linux/videodev2.h>
#include<pthread.h> 

#define 	UVC_DEVICE_DEBUG
#define UvcInfo(fmt, arg...) printf("[%s %d]:"fmt"\n",__func__,__LINE__,##arg)
#define UvcErr(fmt, arg...) printf("[%s %d]:"fmt"\n",__func__,__LINE__,##arg)
	
#ifdef UVC_DEVICE_DEBUG
	#define UvcDbg(fmt, arg...) printf("[%s %d]:"fmt"\n",__func__,__LINE__,##arg)
#else
	#define UvcDbg(fmt, arg...)
#endif

struct _UvcBuffer {
		void * start;
		size_t length;
};

enum {
		IO_METHOD_READ,
		IO_METHOD_MMAP,
		IO_METHOD_USERPTR,
};
struct _UvcDataStruct{
	char dev_name[16];  //device name
	int UvcFd;              //file descriptor
	int NumUvcBuf;  //request buffer number
	int isExit;            //the flags  of thread exit or not 
	int UvcIoMethod;   //read data method
	pthread_t UvcPthread; 
	//pthread_mutex_t UvcPmt;
	struct _UvcBuffer *UvcBuffers;
};

static struct  _UvcDataStruct UvcDS;
void  _ProcessUvcFrame(void *arg);
static int  UvcReadFrame (void);

int OpenUvcDevice()
{
	struct stat st;
	UvcDS.UvcIoMethod=IO_METHOD_MMAP;
	if(strncpy(UvcDS.dev_name,"/dev/video0",strlen("/dev/video0"))==NULL){
		UvcErr("copy errer:%d\n",errno);
		return -1;
	}
	if (-1 == stat (UvcDS.dev_name, &st)) {
		UvcErr("stat dev error:%d",errno);
		return -1;
	}
	if (!S_ISCHR (st.st_mode)) {
		UvcErr("%s is no device\n",UvcDS.dev_name);
		return -1;
	}
	UvcDS.UvcFd = open (UvcDS.dev_name, O_RDWR | O_NONBLOCK, 0);
	if (-1 == UvcDS.UvcFd) {
		UvcErr("Cannot open %s errno: %d\n",UvcDS.dev_name, errno);
		return -1;
	}

	return 0;
}

int CloseUvcDevice(void)
{
	if (-1 == close (UvcDS.UvcFd))
		return -1;
	UvcDS.UvcFd = -1;
	return 0;
}

void  _ProcessUvcFrame(void *arg)
{
	int uvc_fd=*(int*)arg;
	fd_set fds;
	struct timeval tv;
	int rtn =-1;
	UvcDbg();
	pthread_detach((unsigned int)pthread_self());
	while (!UvcDS.isExit) {
		FD_ZERO (&fds);
		FD_SET (uvc_fd, &fds);
		/* Timeout. */
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		rtn = select (uvc_fd + 1, &fds, NULL, NULL, &tv);
		if (-1 == rtn) {
			if (EINTR == errno)
				continue;
		}
		if (0 == rtn) {
			UvcInfo("select timeout\n");
			continue;
		}
		/*process uvc frame information*/
		UvcReadFrame();
	}
	  pthread_exit(NULL);
}

/*mmap*/
static int  InitUvcBuffer (void)
{
	struct v4l2_requestbuffers req;
	memset(&req,0x00,sizeof(req));
	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	
	if (-1 == ioctl (UvcDS.UvcFd, VIDIOC_REQBUFS, &req)) {	
		UvcErr("Unable to allocate buffers:%d\n", errno);
		return -1;
	}

	UvcInfo("request buffers number:%d\n",req.count);
	UvcDS.UvcBuffers = calloc (req.count, sizeof (struct _UvcBuffer));
	if (!UvcDS.UvcBuffers) {
		UvcErr("Out of memory\n");
		return -1;
	}
	
	for (UvcDS.NumUvcBuf = 0; UvcDS.NumUvcBuf < req.count; ++UvcDS.NumUvcBuf) {
		struct v4l2_buffer buf;
		memset(&buf,0x00,sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = UvcDS.NumUvcBuf;
		/*Data memery transformed to physical addresses*/
		if (-1 == ioctl (UvcDS.UvcFd, VIDIOC_QUERYBUF, &buf)){   
			UvcErr("request mmap memory error:%d\n",errno);
			return -1;
		}
		
		UvcDS.UvcBuffers[UvcDS.NumUvcBuf].length = buf.length;
		UvcDS.UvcBuffers[UvcDS.NumUvcBuf].start = mmap (NULL ,buf.length,PROT_READ | PROT_WRITE ,MAP_SHARED ,UvcDS.UvcFd, buf.m.offset);
		if (MAP_FAILED == UvcDS.UvcBuffers[UvcDS.NumUvcBuf].start){
			UvcErr("mmap failed\n");
			return -1;
		}
	}

	return 0;
}
/*
*obtain  uvc device capability(v4l2_cap_video_capture)
*
*/
 int InitUvcDevice (void)
{
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	struct v4l2_fmtdesc fmtdesc;  //
	unsigned int min;
	
	if (-1 == ioctl (UvcDS.UvcFd, VIDIOC_QUERYCAP, &cap)) {
		UvcInfo("query capbilities V4L2 device error:%d\n",errno);
		return -1;
	}
	
	UvcDbg("uvc_capabilites:%0x\n",cap.capabilities);
	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		UvcErr("%s is no video capture device\n",UvcDS.dev_name);
		return -1;
	}
	
	switch (UvcDS.UvcIoMethod) {
		case IO_METHOD_READ:
		if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
			UvcErr("%s does not support read i/o\n",UvcDS.dev_name);
			return -1;
		}
		break;
		case IO_METHOD_MMAP:
		case IO_METHOD_USERPTR:
		if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
			UvcErr("%s does not support streaming i/o\n",UvcDS.dev_name);
			return -1;
		}
		break;
	}
#if 1
	fmtdesc.index=0;
	fmtdesc.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	/*For uvc device equipment support format*/
	UvcDbg("Supportformat:/n");
	while(ioctl(UvcDS.UvcFd,VIDIOC_ENUM_FMT,&fmtdesc)!=-1){
		printf("/t%d.%s/n",fmtdesc.index+1,fmtdesc.description);
		fmtdesc.index++;
	}
#endif
	/* Select video input, video standard and tune here. */
	memset(&cropcap,0x00,sizeof(cropcap));
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (0 == ioctl (UvcDS.UvcFd, VIDIOC_CROPCAP, &cropcap)) {
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect; /* reset to default */
		if (-1 == ioctl (UvcDS.UvcFd, VIDIOC_S_CROP, &crop)) {
			switch (errno) {
			case EINVAL:
			default:
			/* Errors ignored. */
			break;
			}
		}
	}
	
	memset(&fmt,0x0,sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = 640;
	fmt.fmt.pix.height = 480;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	if (-1 == ioctl (UvcDS.UvcFd, VIDIOC_S_FMT, &fmt))
		UvcErr("VIDIOC_S_FMT\n");
	
	/* Note VIDIOC_S_FMT may change width and height. */
	/* Buggy driver paranoia. */
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
	fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
	fmt.fmt.pix.sizeimage = min;
	
	switch (UvcDS.UvcIoMethod) {
		case IO_METHOD_READ:
		case IO_METHOD_USERPTR:
		break;
		case IO_METHOD_MMAP:
			InitUvcBuffer();
		break;
	}

	return 0;
}

/*request video buffer*/
int StartUvcCapturing (void)
{
	unsigned int i;
	enum v4l2_buf_type type;
	switch (UvcDS.UvcIoMethod) {
	case IO_METHOD_READ:
	/* Nothing to do. */
	break;
	case IO_METHOD_MMAP:
		for (i = 0; i < UvcDS.NumUvcBuf; ++i) {
			struct v4l2_buffer buf;
			memset (&buf,0x00,sizeof(buf));
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = i;
			/*read data from memory*/
			UvcDbg();
			if (-1 == ioctl (UvcDS.UvcFd, VIDIOC_QBUF, &buf))
				UvcErr("VIDIOC_QBUF\n");
		}
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		/*start data capture*/
		if (-1 == ioctl (UvcDS.UvcFd, VIDIOC_STREAMON, &type))
			UvcErr("VIDIOC_STREAMON\n");
	break;
	case IO_METHOD_USERPTR:
		for (i = 0; i < UvcDS.NumUvcBuf; ++i) {
			struct v4l2_buffer buf;
			memset(&buf,0x00,sizeof(struct v4l2_buffer));
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_USERPTR;
			buf.index = i;
			buf.m.userptr = (unsigned long) UvcDS.UvcBuffers[i].start;
			buf.length = UvcDS.UvcBuffers[i].length;
			/*read data from memory*/
			if (-1 == ioctl (UvcDS.UvcFd, VIDIOC_QBUF, &buf))
				UvcErr("VIDIOC_QBUF\n");
		}
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == ioctl (UvcDS.UvcFd, VIDIOC_STREAMON, &type))
			UvcErr("VIDIOC_STREAMON\n");
	break;
	}
	return 0;
}

static int  UvcReadFrame (void)
{
	struct v4l2_buffer buf;
	unsigned int i,num;
	switch (UvcDS.UvcIoMethod) {
	case IO_METHOD_READ:
		if (-1 == read (UvcDS.UvcFd, UvcDS.UvcBuffers[0].start, UvcDS.UvcBuffers[0].length)) {
		switch (errno) {
			case EAGAIN:
			case EIO:
			default:
				break;
			}
		}
		break;
	case IO_METHOD_MMAP:
		memset (&buf,0x00,sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		if (-1 == ioctl (UvcDS.UvcFd, VIDIOC_DQBUF, &buf)) {
			UvcErr("dqbuf error:%d\n",errno);
		}
		/********/
		//UvcInfo("buffers[%d]=%p buf_byteused:%d buf_len:%d\n",buf.index,UvcDS.UvcBuffers[buf.index].start,buf.bytesused,buf.length);
	/*
		for(num=0;num<buf.bytesused;++num){
			printf(" %0x",*((int *)buffers[buf.index].start));
			buffers[buf.index].start++;
			if(num%20==0)
				printf("\n");
		}
		printf("\n");
	*/
		/********/
		if (-1 == ioctl (UvcDS.UvcFd, VIDIOC_QBUF, &buf)){
			UvcErr("vidioc qubuf:%d\n",errno);
		}
		break;
	case IO_METHOD_USERPTR:
		memset(&buf,0x00,sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_USERPTR;
		if (-1 == ioctl (UvcDS.UvcFd, VIDIOC_DQBUF, &buf)) {
			switch (errno) {
				case EAGAIN:
				return 0;
				case EIO:
				/* Could ignore EIO, see spec. */
				/* fall through */
				default:
					break;
			}
		}
		for (i = 0; i < UvcDS.NumUvcBuf; ++i){
			if (buf.m.userptr == (unsigned long) UvcDS.UvcBuffers[i].start&& buf.length == UvcDS.UvcBuffers[i].length)
				break;
		}
		if (-1 == ioctl (UvcDS.UvcFd, VIDIOC_QBUF, &buf))
			return -1;
		
		break;
	}
	return 0;
}

static void UninitUvcDevice (void)
{
	unsigned int i;
	switch (UvcDS.UvcIoMethod) {
		case IO_METHOD_READ:
			free (UvcDS.UvcBuffers[0].start);
		break;
		case IO_METHOD_MMAP:
			for (i = 0; i < UvcDS.NumUvcBuf; ++i)
			if (-1 == munmap (UvcDS.UvcBuffers[i].start, UvcDS.UvcBuffers[i].length))
				printf("munmap\n");
		break;
		case IO_METHOD_USERPTR:
			for (i = 0; i < UvcDS.NumUvcBuf; ++i)
				free (UvcDS.UvcBuffers[i].start);
		break;
	}
	if(UvcDS.UvcBuffers!=NULL)
		free (UvcDS.UvcBuffers);
}

static void StopUvcCapturing(void)
{
	enum v4l2_buf_type type;
	switch (UvcDS.UvcIoMethod) {
		case IO_METHOD_READ:
		/* Nothing to do. */
			break;

		case IO_METHOD_MMAP:
		case IO_METHOD_USERPTR:
			UvcInfo("streamoff\n");
			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			if (-1 == ioctl (UvcDS.UvcFd, VIDIOC_STREAMOFF, &type))
				printf("VIDIOC_STREAMOFF\n");
			break;
	}
}

int _UvcStart()
{
	//pthread_mutex_init(&UvcDS.UvcLock, NULL);
	memset(&UvcDS,0x00,sizeof(struct _UvcDataStruct));
	if(0 != OpenUvcDevice()){
		UvcErr("open device exceptions\n");
		return -1;
	}
	InitUvcDevice();
	StartUvcCapturing();
	UvcDS.isExit=0;
	/*create a thread to process uvc data*/
	if(pthread_create(&UvcDS.UvcPthread, NULL, (void *(*)(void *))_ProcessUvcFrame, &UvcDS.UvcFd)!=0){
		UvcErr("create thread error:%d\n",errno);
		return -1;
	}
	return 0;
}

int _UvcStop()
{
	UvcDS.isExit=1;
	UvcInfo();
	pthread_cancel(UvcDS.UvcPthread);
	StopUvcCapturing();
	UninitUvcDevice();
	CloseUvcDevice();
	return 0;
}
