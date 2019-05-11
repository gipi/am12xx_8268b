#define _GNU_SOURCE
#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <dlfcn.h>
#include <memory.h>
#include "swf_ext.h"
#include "video_engine.h"
#include "fui_common.h"
#include "sys_cfg.h"
#include "display.h"
#include "apps_vram.h"
#include "lcm_op.h"
#include "system_info.h"
#include <errno.h>
#include "sys_buf.h"
#include <fcntl.h>
#include "stream_api.h"
#include "sys_gpio.h"
#include"ezcast_public.h"
//#define DIREC_IO_TEST
/** enable preview show or not */
#define VIDEO_PREVIEW_EN


#define SEEK_PLAY_THREAD //charles for 4x,8x....FB and FF
static inline void stop_seek_play_thread();


//#define  VIDEO_DEBUG
#ifdef  VIDEO_DEBUG
#define p_printf(format,args...)   printf(format, ##args)
#else
#define p_printf(format,args...)   do {} while (0)
#endif
int FF_step_form_SWF=1;
int FB_step_form_SWF=1;
static volatile int dmr_seek_allowed = 1;

extern int _set_hdmi_mode(int * hmode);

/** function pointers to video MMM APIs */
static videofunc_open vdec_open = NULL;
static videofunc_cmd vdec_cmd = NULL;
static videofunc_close vdec_close = NULL;
void *videohandle = NULL;

static unsigned char *vheap1 = NULL;
static long heap1size=0;
static unsigned char *prevheap = NULL;
static unsigned int prev_actual_w = 0;
static unsigned int prev_actual_h = 0;
sem_t v_info;

/** handle for video_player.so dynamic library */
void *dlhandle = NULL;

/** something error when play */
int video_error=0;
static int video_set_file_ok=0;

/** play status */
static int playstat=VE_IDLE;

/** 
* @brief semaphore to prevent concurrent access,
*    currently used for full screen play and preview.
*/
sem_t *vsem=NULL;

/** preview buffer address and size */
static unsigned char *vpreview_buffer=NULL;
static unsigned int vpreview_bufsize=0;
static int video_support_status=0;
sem_t *vsem_previewok=NULL;

#define PREVIEW_MAX 8		//video preview max number
static unsigned char *vpreview_buffer_array[PREVIEW_MAX];

/** current playback file */
static char vplay_file[256*3];

/*current file info struct*/

file_info_s media_file_info;

/** max frame width**/
static int max_frame_width = -1;
int is_video_stream=0;

static video_cmd_s videoPlayParaCmd;


long get_video_play_time_for_dmr();

void dump_prio_value(struct am_bus_priority prio_tmp_printf)
{
	
	
	fprintf(stderr,"function:%s,line:%d,prio_tmp_printf->special_dma=%d\n",__FUNCTION__,__LINE__,prio_tmp_printf.special_dma);
	
	fprintf(stderr,"function:%s,line:%d,prio_tmp_printf->mac=%d\n",__FUNCTION__,__LINE__,prio_tmp_printf.mac);
	fprintf(stderr,"function:%s,line:%d,prio_tmp_printf->ahb_bus=%d\n",__FUNCTION__,__LINE__,prio_tmp_printf.ahb_bus);
	fprintf(stderr,"function:%s,line:%d,prio_tmp_printf->graph_2d=%d\n",__FUNCTION__,__LINE__,prio_tmp_printf.graph_2d);
	fprintf(stderr,"function:%s,line:%d,prio_tmp_printf->de=%d\n",__FUNCTION__,__LINE__,prio_tmp_printf.de);
	fprintf(stderr,"function:%s,line:%d,prio_tmp_printf->dac=%d\n",__FUNCTION__,__LINE__,prio_tmp_printf.dac);
	fprintf(stderr,"function:%s,line:%d,prio_tmp_printf->vdc=%d\n",__FUNCTION__,__LINE__,prio_tmp_printf.vdc);
	fprintf(stderr,"function:%s,line:%d,prio_tmp_printf->axi_bus=%d\n",__FUNCTION__,__LINE__,prio_tmp_printf.axi_bus);
	fprintf(stderr,"function:%s,line:%d,prio_tmp_printf->vec=%d\n",__FUNCTION__,__LINE__,prio_tmp_printf.vec);
}

//#define TEST_MEM	//if use this macro,i will use the last 32M,you must have 64M ddr,and enable the last 2 blank swap
#ifdef TEST_MEM
static unsigned long heap_bus_addr = 0x02000000;
static int heap_size = (1L<<20)*32;
static void *heap_mem_handle = NULL;
static void *heap_mem_ptr = NULL;
static unsigned long *pRegDRAM = NULL;


static 
void REG_WRITE(unsigned long *p_reg_base, unsigned long reg_base, unsigned long offset, unsigned long val)
{
	p_reg_base[offset/4] = val;
}

static 
unsigned long REG_READ(unsigned long *p_reg_base, unsigned long reg_base, unsigned long offset)
{
	unsigned long val;
	val = p_reg_base[offset/4];
	return val;
}
#endif

static int __ve_pause();
static int __ve_resume();

extern void *deinst;
static DE_config ds_conf;
static DE_config saved_conf;


static int _get_fast_play_step(int totalTime)
{
	int sec;
	int step;

	sec = totalTime/1000;

	if(sec < 60){
		step = 2 * 1000;
	}
	else if(sec < 5*60){
		step = 5 *1000;
	}
	else{
		step = 10*1000;
	}
	return step;
	
}

int get_phy_viraddr(unsigned int* phy_addr,unsigned int* vir_addr)
{
	struct mem_dev basic_heap,share_heap;
	int memfd;
	int err;
	static char font0_path[64],font1_path[64];

	share_heap.request_size = _VIDEO_HEAP1_SIZE;
	share_heap.buf_attr = UNCACHE;

	memfd = open("/dev/sysbuf",O_RDWR);
	if(memfd == -1)
	{
		printf("open /dev/sysbuf error\n");
		return -1;
	}

	err = ioctl(memfd,MEM_GET,&share_heap);
	if(err == -1){
		printf("fui get basic heap error\n");
		close(memfd);
		return -1;
	}
	printf("phy = %x,vir = %x",share_heap.logic_address,share_heap.physic_address);
	*vir_addr = share_heap.logic_address;
	*phy_addr = share_heap.physic_address;
	close(memfd);
	return 0;
}

#ifdef DIREC_IO_TEST
//////////////////////////////////////
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define VIDEO_ALIGN	4096
#define DIO_BUFFER_SIZE		(VIDEO_ALIGN*8)

typedef struct _dio_info_
{
	unsigned char buffer[DIO_BUFFER_SIZE+VIDEO_ALIGN];
	unsigned char *buf;
	int data_size;
	int64 pos;
	int64 flen;
	int eof;
	int fd;
	void *sys_ptr;
}DIO;

static 
void dio_close(void *handle)
{
	if(handle)
	{
		DIO *pdio = (DIO*)handle;

		if(pdio->fd != -1)
			close(pdio->fd);
		if(pdio->sys_ptr != NULL)
		{
			void *sys_fb;
			void *io_buf = pdio->sys_ptr;
			
			sys_fb = open("/dev/sysbuf",O_RDWR);
			ioctl(sys_fb,MEM_PUT,io_buf);
			close(sys_fb);

			SWF_Free(io_buf);
		}
		//free(handle);
	}
}

static 
void *dio_open(char *fname, char *mode)
{
	unsigned long addr;
	int oflag;
	DIO *pdio;

	//pdio = SWF_Malloc(sizeof(DIO));
	//if(0)
	{
		struct mem_dev *io_buf = NULL;
		void *fd;
		
		io_buf=SWF_Malloc(sizeof(struct mem_dev));
		io_buf->buf_attr = MEM_FS;
		io_buf->request_size =sizeof(DIO);
		fd = open("/dev/sysbuf",O_RDWR);
		ioctl(fd,MEM_GET,io_buf);
		pdio= io_buf->logic_address;
		pdio->sys_ptr = io_buf;
		
		//OSprintf("DIO:0x%x,0x%x\n",io_buf.logic_address,io_buf.physic_address);
		close(fd);
	}
	if(!pdio)
	{
		printf("dio_open, malloc error\n");
		goto fail;
	}
	memset(pdio, 0, sizeof(DIO));
	pdio->fd = -1;
	oflag = O_RDONLY;
#ifdef WIN32
	oflag |= O_BINARY;
#else
	oflag |= O_DIRECT;
#endif
	pdio->fd = open(fname, oflag);
	if(pdio->fd == -1)
	{
		printf("dio_open, open error\n");
		goto fail;
	}
	
	pdio->flen = lseek64(pdio->fd, 0, SEEK_END);
	if(pdio->flen <= 0)
	{
		printf("dio_open, lseek64 error\n");
		goto fail;
	}
	if(lseek64(pdio->fd, 0, SEEK_SET) == -1)
	{
		printf("dio_open, lseek64 error\n");
		goto fail;
	}

	addr = (unsigned long)pdio->buffer;
	addr += VIDEO_ALIGN-1;
	addr -= addr&(VIDEO_ALIGN-1);
	pdio->buf = (unsigned char*)addr;
	//OSprintf("*************direct IO[0x%x]*************\n",pdio->buf);
	return (void*)pdio;

fail:
	dio_close(pdio);
	return NULL;
}

static 
int dio_read_packet(void *handle, void *buffer, int size)
{
	DIO *pdio = (DIO*)handle;
	unsigned char *buf = (unsigned char*)buffer;
	int offset;
	int left_size = size;

	{
	 	unsigned long cur_s;
		unsigned long cur_us;
		unsigned long new_s;
		unsigned long new_us;
		unsigned long rf_s;
		unsigned long rf_us;
		unsigned long rb_s;
		unsigned long rb_us;
		unsigned long v1=0,v2=0,v3=0;
		//OSGetTime(&cur_s, &cur_us);

	
	if(!handle || !buffer || size <= 0 || pdio->pos >= pdio->flen)
		return 0;

	offset = (int)(pdio->pos&(DIO_BUFFER_SIZE-1));
	if(pdio->data_size >= offset+size)
	{
		memcpy(buf, pdio->buf+offset, size);
		pdio->pos += size;
		if(pdio->data_size == offset+size)
			pdio->data_size = 0;
		return size;
	}
	else
	{
		int read_size;
		if(pdio->data_size==0)
		{
			int offset0 = (int)(pdio->pos&(DIO_BUFFER_SIZE-1));
			int64 offset1 = pdio->pos-offset0;
			if(lseek64(pdio->fd, offset1, SEEK_SET) == -1)
			{
				printf("dio_read_packet lseek64 error\n");
				return 0;
			}
			//OSprintf("<");
			read_size = read(pdio->fd, pdio->buf, DIO_BUFFER_SIZE);
			v1=((rb_s-rf_s)*1000*1000+(rb_us-rf_us))/1000;
			//OSprintf("%d>",v1);
			if(read_size < offset0)
			{
				printf("dio read error %d\n",__LINE__);
				return 0;
			}
				
			pdio->data_size = read_size;
		}
		if(pdio->data_size>0)
		{
			int copy_size = pdio->data_size-offset;
			if(copy_size > left_size){
				copy_size = left_size;
				left_size -= copy_size;
			}
			else
			{
				pdio->data_size = 0;
				left_size -= copy_size;
			}
			memcpy(buf, pdio->buf+offset, copy_size);
			buf += copy_size;
			pdio->pos += copy_size;
		}
		while(left_size >= DIO_BUFFER_SIZE)
		{
			//if((unsigned long)buf&(VIDEO_ALIGN-1))
			if(1)
			{
				read_size = read(pdio->fd, pdio->buf, DIO_BUFFER_SIZE);
				//v2+=((rb_s-rf_s)*1000*1000+(rb_us-rf_us))/1000;
				if(read_size == -1)
					read_size = 0;
				if(read_size)
				{
					memcpy(buf, pdio->buf, read_size);
					pdio->pos += read_size;
					buf += read_size;
					left_size -= read_size;
				}
				if(read_size < DIO_BUFFER_SIZE)
				{
					pdio->data_size = read_size;
					printf("dio read finish %d\n",__LINE__);
					goto end;
				}
				else
					pdio->data_size = 0;
			}
			else
			{
				read_size = read(pdio->fd, buf, DIO_BUFFER_SIZE);
				if(read_size == -1)
					read_size = 0;
				if(read_size)
				{
					buf += read_size;
					left_size -= read_size;
					pdio->pos += read_size;
				}
				if(read_size < DIO_BUFFER_SIZE)
				{
					printf("dio read finish %d\n",__LINE__);
					goto end;
				}
				else
					pdio->data_size = 0;
			}
		}
		if(left_size>0)
		{
			read_size = read(pdio->fd, pdio->buf, DIO_BUFFER_SIZE);
			if(read_size == -1)
				read_size = 0;
			pdio->data_size = read_size;
			if(read_size >= left_size)
			{
				memcpy(buf, pdio->buf, left_size);
				pdio->pos += left_size;
				buf += left_size;
				left_size -= left_size;
			}
			else if(read_size)
			{
				memcpy(buf, pdio->buf, read_size);
				pdio->pos += read_size;
				buf += read_size;
				left_size -= read_size;
			}
		}
	}
}

end:
	return size-left_size;
}

static 
int dio_seek_set(void *handle, int64 offset)
{
	DIO *pdio = (DIO*)handle;
	if(!handle ||  offset < 0 ||offset > pdio->flen)
		return -1;

	if((pdio->pos/DIO_BUFFER_SIZE) != (offset/DIO_BUFFER_SIZE))
		pdio->data_size = 0;

	pdio->pos = offset;

	return 0;
}

static 
int dio_seek_cur(void *handle, int64 offset)
{
	DIO *pdio = (DIO*)handle;
	if(!handle)
		return -1;
	return dio_seek_set(handle, pdio->pos+offset);
}

static 
int dio_seek_end(void *handle, int64 offset)
{
	DIO *pdio = (DIO*)handle;
	if(!handle)
		return -1;
	return dio_seek_set(handle, pdio->flen+offset);
}

static 
int64 dio_get_pos(void *handle)
{
	DIO *pdio = (DIO*)handle;
	if(!handle)
		return -1;

	return pdio->pos;
}

/**
* file operations for video.
*/
static void *_video_fopen(char *path, char *mode)
{
	return (void*)dio_open(path, mode);
}

static int _video_fclose(void *fp)
{
	dio_close(fp);
	return 0;
}
static int _video_read_packet(void *opaque, unsigned char *buf, long buf_size)
{
	
	return  dio_read_packet(opaque,buf,buf_size);
	
}

static int _video_write_packet(void *opaque, unsigned char *buf, long buf_size)
{
	//return (int)fwrite(buf, sizeof(char), buf_size, (FILE*)opaque);
}

static int _video_file_seek_set(void *opaque, int64 offset)
{
	return dio_seek_set(opaque,offset);
}

static int _video_file_seek_end(void *opaque, int64 offset)
{
	return dio_seek_end(opaque, offset);//dio_seek_cur
}

static int64 _video_get_pos(void *opaque)
{
	return dio_get_pos(opaque);
}

static int _video_get_seekable(void *opaque)
{
	return 1;
}

///////////////////////////////////////sdram test///////////////////////////////////////////
#elif defined( SDRAM_TEST)
int64 rptr,eptr;

static 
void *_memcpy_(void *d, const void *s, long n)
{
	unsigned char *dst = (unsigned char*)d;
	unsigned char *src = (unsigned char*)s;

	if(((unsigned long)dst&3) == ((unsigned long)src&3))
	{
		while(((unsigned long)dst&3) != 0 && n > 0)
		{
			*dst++ = *src++;
			n--;
		}
		while(n >= 16)
		{
			unsigned long t0, t1, t2, t3;

			t0 = ((unsigned long*)src)[0];
			t1 = ((unsigned long*)src)[1];
			t2 = ((unsigned long*)src)[2];
			t3 = ((unsigned long*)src)[3];
			((unsigned long*)dst)[0] = t0;
			((unsigned long*)dst)[1] = t1;
			((unsigned long*)dst)[2] = t2;
			((unsigned long*)dst)[3] = t3;
			n -= 16;
			src += 16;
			dst += 16;
		}
		while(n >= 4)
		{
			*(unsigned long*)dst = *(unsigned long*)src;
			n -= 4;
			src += 4;
			dst += 4;
		}
	}
//#ifdef MIPS
	else
	{
		while(((unsigned long)dst&3) != 0 && n > 0)
		{
			*dst++ = *src++;
			n--;
		}
		while(n >= 16)
		{
			unsigned long t0, t1, t2, t3;

			asm("lwr	%0, 0(%4)	\n\t"
				"lwl	%0, 3(%4)	\n\t"
				"lwr	%1, 4(%4)	\n\t"
				"lwl	%1, 7(%4)	\n\t"
				"lwr	%2, 8(%4)	\n\t"
				"lwl	%2, 11(%4)	\n\t"
				"lwr	%3, 12(%4)	\n\t"
				"lwl	%3, 15(%4)"
				: "=r" (t0), "=r" (t1), "=r" (t2), "=r" (t3)
				: "r" (src));
			((unsigned long*)dst)[0] = t0;
			((unsigned long*)dst)[1] = t1;
			((unsigned long*)dst)[2] = t2;
			((unsigned long*)dst)[3] = t3;
			n -= 16;
			src += 16;
			dst += 16;
		}
		while(n >= 4)
		{
			unsigned long t0;

			asm("lwr	%0, 0(%1)	\n\t"
				"lwl	%0, 3(%1)"
				: "=r" (t0)
				: "r" (src));
			((unsigned long*)dst)[0] = t0;
			n -= 4;
			src += 4;
			dst += 4;
		}
	}
//#endif
	while(n > 0)
	{
		n--;
		*dst++ = *src++;
	}

	return d;
}

static void *_ram_fopen(char *path, char *mode)
{
	void * handle;
	int ret;
	int64 filelen;
	struct mem_dev io_buf;
	
	handle =fopen(path, mode);
	
	//heap_mem_handle = OSmopen();
	//heap_mem_ptr = OSmmap(heap_mem_handle, heap_bus_addr, heap_size);
	{
		void *fd;
		io_buf.buf_attr = CACHE;
		io_buf.request_size =heap_size;
		fd = open("/dev/sysbuf",O_RDWR);
		ret = ioctl(fd,MEM_GET,&(io_buf));
		heap_mem_ptr = io_buf.logic_address;
		
		OSprintf("map:0x%x,0x%x\n",heap_mem_ptr,io_buf.physic_address);
		close(fd);
	} 
	
	fseek(handle, 0, SEEK_END);
	filelen=ftell(handle);
	fseek(handle, 0, SEEK_SET);
	
	ret=fread(heap_mem_ptr, sizeof(char), filelen, (FILE*)handle);
	
	fseek(handle, 0, SEEK_SET);
	rptr=0;
	eptr=filelen;
	
	return handle;
}

static int _ram_fclose(void *fp)
{
	OSmunmap(heap_mem_handle,heap_mem_ptr,heap_size);
	OSmclose(heap_mem_handle);
	return fclose((FILE*)fp);
}
static int _ram_read_packet(void *opaque, unsigned char *buf, long buf_size)
{
	unsigned int read_cnt;
	
	if(((int)rptr+buf_size)>(int)eptr)
		read_cnt = (int)eptr-(int)rptr;
	else
		read_cnt = buf_size;
	{
			unsigned long cur_s;
			unsigned long cur_us;
			unsigned long new_s;
			unsigned long new_us;
			OSGetTime(&cur_s, &cur_us);
			_memcpy_(buf,(void *)((int)heap_mem_ptr+(int)rptr),read_cnt);	
			OSGetTime(&new_s, &new_us);
			//OSprintf("c:%d-%d.%3d\n",read_cnt,((new_s-cur_s)*1000*1000+(new_us-cur_us))/1000,((new_s-cur_s)*1000*1000+(new_us-cur_us))%1000);
		}	
	
	rptr=rptr+read_cnt;
	return read_cnt;
}

static int _ram_write_packet(void *opaque, unsigned char *buf, long buf_size)
{
	return buf_size;
}

static int _ram_file_seek_set(void *opaque, int64 offset)
{
	rptr = offset;
	return 0;
}

static int _ram_file_seek_end(void *opaque, int64 offset)
{
	rptr = eptr-offset;
	return 0;
}

static int64 _ram_get_pos(void *opaque)
{
	return rptr;
}

static int _ram_get_seekable(void *opaque)
{
	return 1;
}

#else
/**
* file operations for video.
*/
static void *_video_fopen(char *path, char *mode)
{
	return (void*)fopen(path, mode);
}

static int _video_fclose(void *fp)
{
	return fclose((FILE*)fp);
}
static int _video_read_packet(void *opaque, unsigned char *buf, long buf_size)
{
	return (int)fread(buf, sizeof(char), buf_size, (FILE*)opaque);
}

static int _video_write_packet(void *opaque, unsigned char *buf, long buf_size)
{
	return (int)fwrite(buf, sizeof(char), buf_size, (FILE*)opaque);
}

static int _video_file_seek_set(void *opaque, int64 offset)
{
	return fseeko((FILE*)opaque, offset, SEEK_SET);
}

static int _video_file_seek_end(void *opaque, int64 offset)
{
	return fseeko((FILE*)opaque, offset, SEEK_END);
}

static int64 _video_get_pos(void *opaque)
{
	return ftello((FILE*)opaque);
}

static int _video_get_seekable(void *opaque)
{
	return 1;
}

#if 1 //[Sanders.130227] - EZ Stream porting
static void *stream_handle = NULL;
#endif
static void *_video_fopen_stream(char *path, char *mode)
{
	int file_format;
  #if 1 //[Sanders.130227] - EZ Stream porting
	stream_handle = am_stream_open(path, NULL, &file_format);
  	/**
	* for streaming, register the callback for pause and resume.
	*/
	am_stream_set_player_pause_cb(__ve_pause);
	am_stream_set_player_resume_cb(__ve_resume);
	return (void*)stream_handle;
  #else
	return (void*)am_stream_open(path, NULL, &file_format);
  #endif
}

static int _video_fclose_stream(void *fp)
{
	am_stream_close(fp);
  #if 1 //[Sanders.130227] - EZ Stream porting
	stream_handle = NULL;
  #endif
	return 0;
}

static int _video_read_packet_stream(void *opaque, unsigned char *buf, long buf_size)
{
	return (int)am_stream_read(opaque, buf, buf_size);
}

static int _video_write_packet_stream(void *opaque, unsigned char *buf, long buf_size)
{
	return buf_size;
}

static int _video_file_seek_set_stream(void *opaque, int64 offset)
{
	return am_stream_seek_set(opaque, offset);
}

static int _video_file_seek_end_stream(void *opaque, int64 offset)
{
	return am_stream_seek_end(opaque, offset);
}

static int64 _video_get_pos_stream(void *opaque)
{
	return am_stream_get_pos(opaque);
}

static int64 _video_get_filesize_stream(void *opaque)
{
	return am_stream_get_filesize(opaque);
}

static int _video_get_seekable_stream(void *fp)
{
	return am_stream_seekable(fp);
}

static int _video_stop_set_stream(void *fp)
{
	am_stream_stop(fp);
	return 0;
}


#endif

static int set_de_defaultcolor()
{
	de_get_config(deinst,&ds_conf,DE_CFG_ALL);
	ds_conf.input.enable=0;
	de_set_Config(deinst,&ds_conf,DE_CFG_ALL);
	
	return 0;
}

static int save_de_config()
{
	de_get_config(deinst,&saved_conf,DE_CFG_ALL);
	
	return 0;
}

static int restore_de_config(int setdefault)
{
	if(setdefault){
		saved_conf.input.enable=0;
	}
	de_set_Config(deinst,&saved_conf,DE_CFG_ALL);
	
	return 0;
}



static int free_video_heap()
{
	
	if(vheap1){
		printf("free_video_heap1 = %x,LINE=%d\n",vheap1,__LINE__);
		SWF_Free((void *)vheap1);
		vheap1 = NULL;
		
	}
	return 0;
}

static void _ve_sem_wait(sem_t * sem)
{
	int err;

__PEND_REWAIT:
	err = sem_wait(sem);
	if(err == -1){
		int errsv = errno;
		if(errsv == EINTR){
			//Restart if interrupted by handler
			goto __PEND_REWAIT;	
		}
		else{
			printf("work_sem_pend: errno:%d\n",errsv);
			return;
		}
	}

	return;
	
}
static void _ve_sem_post(sem_t * sem)
{
	int err;
	err = sem_post(sem);
}



#ifdef SEEK_PLAY_THREAD

//seek play thread struct
//for fast forward and back
struct seek_play{
	char stat;
	char exit;
	int sleep_time; //us
	int cur_seek_time; //ms
	int step;  
	pthread_t thread_id;
};

#define SEEK_THREAD_SLEEP_TIME (200*1000)//us
volatile struct seek_play seek_thread = {sleep_time: SEEK_THREAD_SLEEP_TIME};


static inline int get_cur_play_time()
{
	int playtime = -1;
	
	if(videohandle){
		video_cmd_s cmd;
		cmd.con  = V_GET_CUR_TIME;
		if(_video_dec_send_cmd(videohandle,&cmd)){
			playtime = -1;
		}else{
			playtime = cmd.par.cur_time;
		}
	}

	return playtime;//ms
}

static void * seek_play_thread_main(void * arg)
{
	int totaltime = -1;
	int step = 0;
	int i = 0;
	video_cmd_s cmd;
	unsigned int tcur_time = 0;

	pthread_detach(pthread_self());
	seek_thread.stat = 1; //run
	seek_thread.cur_seek_time = get_cur_play_time();
	totaltime = media_file_info.total_time;

	printf("%s start, step == %d\n", __func__, seek_thread.step);
	
	if(seek_thread.step != 0 && seek_thread.cur_seek_time >= 0){
		while(seek_thread.exit != 1 && videohandle != NULL && ((playstat == VE_FB)||(playstat == VE_FF)) ){
			seek_thread.cur_seek_time +=  seek_thread.step;
			printf("seek_time == %d\n", seek_thread.cur_seek_time);
			
			if(seek_thread.cur_seek_time >= totaltime){
				cmd.con  = V_STOP;
				_video_dec_send_cmd(videohandle,&cmd);
				printf("[%s] stop video seek\n", __func__);
				break;
			}else if(seek_thread.cur_seek_time < 0){
				dmr_seek_allowed = 0;
				cmd.par.seek_single_time = 0;
				cmd.con  = V_SEEK_SINGLE;
				_video_dec_send_cmd(videohandle,&cmd);
				playstat = VE_PAUSE;
				printf("[%s] replay video\n", __func__);
				break;
			}

			tcur_time = get_cur_play_time();
			printf("video play time=%d\n", tcur_time);
			if(playstat == VE_FF){
				if(tcur_time >= seek_thread.cur_seek_time){
				    goto seek_thread_sleep;
				}
			}else{
				if(tcur_time <= seek_thread.cur_seek_time){
				    goto seek_thread_sleep;
				}
			}

			if(!dmr_seek_allowed){
				printf("Error seek not end\n");
				goto seek_thread_sleep;
			}
			if(1 == seek_thread.exit){
				break;
			}
			dmr_seek_allowed = 0;
			cmd.par.seek_single_time = seek_thread.cur_seek_time;
			cmd.con  = V_SEEK_SINGLE;
			_video_dec_send_cmd(videohandle, &cmd);

seek_thread_sleep:
			if(1 == seek_thread.exit){
				break;
			}

			for(i=0; i<10; i++){
				usleep(seek_thread.sleep_time/10);
				if(1 == seek_thread.exit){
					goto END;
				}
			}
		}
	}else{
		printf("seek_thread.step == 0\n");
	}

END:
	printf("%s End, step == %d, exit=%d\n", __func__, seek_thread.step, seek_thread.exit);
	seek_thread.stat = 0; //stop
	pthread_exit(NULL);
	return NULL;
}


static inline void stop_seek_play_thread()
{
	if(1 == seek_thread.stat){
		seek_thread.exit = 1;

		while(seek_thread.stat){
			usleep(100*1000);
			printf("stop_seek_play_thread");
		}
	}
}

static inline void start_seek_thread(int speed)
{
	int err = -1;	

	printf("[%s]speed=%d\n", __func__, speed);
	seek_thread.step = speed * (SEEK_THREAD_SLEEP_TIME/1000);

	if(0 == seek_thread.stat){
		seek_thread.exit = 0;
		err = pthread_create(&seek_thread.thread_id, NULL, seek_play_thread_main, NULL);
		if(err < 0){
			printf("[video engine]create seek play thread err!\n");
		}
	}
}
#else
static inline void stop_seek_play_thread()
{
	return;
}
static inline void start_seek_thread(int speed)
{
	return;
}
#endif


void _video_get_player_msg(notify_msg_e msg)
{
	video_cmd_s cmd;
	int svalue=0;
	static int pre_msg = -1;
	int rtn = -1;
	//printf("-----get player msg:%d-----\n",msg);
	
	switch(msg){
		
		case NO_SUPPORT_MSG:
			printf("FUI/video_engie.c file is not support\n");
			#ifdef VIDEO_PREVIEW_EN
				p_printf("%s,%d:video_error is %d\n",__FILE__,__LINE__,video_error);
				if(video_error==0){
					video_error = 1;
					video_support_status = 3;
					if(vsem_previewok){
						_ve_sem_post(vsem_previewok);
					}
					
					sem_getvalue(&v_info,&svalue);
					if(svalue == 0){
						_ve_sem_post(&v_info);
					}
					p_printf("%s,%d,video_error is %d,video_support_status is %d\n",__FILE__,__LINE__,video_error,video_support_status);
				}
			#endif
			free_video_heap();
			playstat = VE_STOP;
			SWF_Message(NULL, SWF_MSG_KEY_NOT_SUPPORT, NULL);
			break;

		case FILE_INFO_READY_MSG:
			printf("file information ready\n");
			_ve_sem_post(&v_info);
			video_set_file_ok = 1;
			break;
			
		case NEED_STOP_MSG:
			{
				playstat = VE_READY_STOP;
				printf("need stop\n");
				set_de_defaultcolor();
				osdengine_disable_osd();
				stop_seek_play_thread();
				cmd.con = V_STOP;
				_video_dec_send_cmd(videohandle,&cmd);	
				break;
			}
		case STOP_OK_MSG:
			{
				
				playstat = VE_STOP;
				printf("stop ok video engine,playstat = %d\n",playstat);
				
				break;
			}
		case PREVIEW_OK_MSG:
			{
				printf("preview ok\n");
				if(vsem_previewok){
					_ve_sem_post(vsem_previewok);
				}
				break;
			}
		case CLEAR_FFB_FLAG:
			{
				printf("clear FF/FB flag,play again\n");
				//快退到头自动播放功能
				_ve_sem_wait(vsem);
				cmd.con = V_PLAY;
				_video_dec_send_cmd(videohandle, &cmd);
				cmd.con  = V_RESUME;
				_video_dec_send_cmd(videohandle,&cmd);	
				playstat = VE_PLAY;
				_ve_sem_post(vsem);
				break;
			}
  #if 1 //[Sanders.130227] - EZ Stream porting
		case VIDEO_READ_INDICATE_MSG:
			if (stream_handle != NULL)
			{
				int64 data_pos = _video_get_pos_stream(stream_handle);
				am_cache_setdatapos(data_pos);
			}
			break;
  #endif
  #if 1 //[Sanders.130425] - EZ Stream error handling
		case VIDEO_SEEK_START_MSG:
			am_cache_setavseeking(1);
			break;
		case VIDEO_SEEK_STOP_MSG:
			printf("[%s,%d] VIDEO_SEEK_STOP_MSG set dmr_seek_allowed = 1 \n",__FILE__,__LINE__);
			dmr_seek_allowed = 1;
			am_cache_setavseeking(0);
			#ifdef SEEK_PLAY_THREAD
			if((VE_FF!=playstat)&&(VE_FB!=playstat)){
				cmd.con  = V_RESUME;
				_video_dec_send_cmd(videohandle,&cmd);	
				playstat = VE_PLAY;
			}
			#else
			cmd.con  = V_RESUME;
			_video_dec_send_cmd(videohandle,&cmd);	
			playstat = VE_PLAY;
			#endif
			break;
  #endif
		default:
			printf("-----get unknown player msg:%d-----\n",msg);
			break;
	}

}


int _video_dec_send_cmd(void *handle,video_cmd_s *cmd)
{
	int rtn=-1;

	p_printf("+++++++++ send command:%d++++++++++\n",cmd->con);
	
	if (handle == NULL){
		printf("video dec handle NULL when send command\n");
		return -1;
	}

	if(vdec_cmd == NULL){
		printf("video cmd func NULL\n");
		return -1;
	}

	rtn = vdec_cmd(handle,cmd);

	return rtn;
}

int _video_dec_open()
{
	char mmmlib[64];
	
	/** 
	* open the MMM dynamic lib 
	*/
	sprintf(mmmlib,"%s%s",AM_DYNAMIC_LIB_DIR,"libvideo_player.so");
	//sprintf(mmmlib,"%s","/mnt/card/lib/video_player.so");
	printf("mmmlib == %s\n",mmmlib);
	if(dlhandle){
		dlclose(dlhandle);
	}
	dlhandle = dlopen(mmmlib,RTLD_LAZY|RTLD_GLOBAL);
	printf("handle == 0x%x\n",dlhandle);
	if(dlhandle==NULL){
		printf("dl open error\n");
		goto V_OPEN_ERROR1;
	}

	/**
	* resolve the MMM APIs.
	*/
	dlerror();
	vdec_open = dlsym(dlhandle,"video_dec_open");
	if(vdec_open == NULL){
		printf("vdec_open error:%s\n",dlerror());
		goto V_OPEN_ERROR2;
	}

	vdec_cmd = dlsym(dlhandle,"video_dec_cmd");
	if(vdec_cmd == NULL){
		printf("vdec_cmd error\n");
		vdec_open = NULL;
		goto V_OPEN_ERROR2;
	}

	vdec_close = dlsym(dlhandle,"video_dec_close");
	if(vdec_close == NULL){
		vdec_open = NULL;
		vdec_cmd = NULL;
		printf("vdec_close error\n");
		goto V_OPEN_ERROR2;
	}

	/**
	* open the video dec handle
	*/
	videohandle = vdec_open(NULL);
	if(videohandle == NULL){
		vdec_open = NULL;
		vdec_cmd = NULL;
		vdec_close = NULL;
		printf("videohandle error\n");
		goto V_OPEN_ERROR2;
	}

	/**
	* it seems everything init ok, just return
	*/

	return 0;


V_OPEN_ERROR2:
	
	if(dlhandle){
		dlclose(dlhandle);
		dlhandle = NULL;
	}
	
V_OPEN_ERROR1:
	
	return -1;
	

}

int _video_dec_close()
{
	if(videohandle && vdec_close){
		vdec_close(videohandle,NULL);
	}
	
	vdec_open = NULL;
	vdec_cmd = NULL;
	vdec_close = NULL;

	if(dlhandle){
		dlclose(dlhandle);
		dlhandle = NULL;
	}
	/**/
	return 0;
}


static int calc_fastplay_step(int totaltime)
{
	int sec;
	int step;

	sec = totaltime/1000;

	if(sec < 60){
		step = 2 * 1000;
	}
	else if(sec < 5*60){
		step = 5 *1000;
	}
	else{
		step = 10*1000;
	}
	return step;
	
}

#ifdef MODULE_CONFIG_VIDEO

static int ve_open(void * handle)
{
	char * file;
	int index;
	int err;
	char mmmlib[64];
	video_cmd_s cmd;
	INT32U i;
	int ret=1;
	struct am_bus_priority prio_tmp;

	
	SWFEXT_FUNC_BEGIN(handle);
	video_set_file_ok = 0;
	playstat=VE_IDLE;

	if(videohandle){
		printf("videohandle has opened !\n");
		
		goto	_VE_OPEN_OUT;
	}

	save_de_config();

	/** FIXME: [release image and audio handle] */

	
	err = _video_dec_open();

	if(err == -1){
		printf("video engine open error\n");
		video_error = 1;
		ret=0;
		goto _VE_OPEN_OUT;
	}
	else{
		video_error = 0;
	}

	/**
	* register video callback function.
	*/
	cmd.con = V_SET_NOTIFY_FUNC;
	cmd.par.notify_func =_video_get_player_msg;	
	err = _video_dec_send_cmd(videohandle,&cmd);
	if(err == -1){
		video_error = 1;
		ret=0;
	}
	else{
		video_error = 0;
	}
	
//#if AM_CHIP_ID == 1213
#if 0
	am_soc_get_bus_priority(&prio_tmp);
	prio_tmp.graph_2d=0;
	prio_tmp.mac=4;
	prio_tmp.axi_bus=5;
	//dump_prio_value(prio_tmp);
	am_soc_set_bus_priority(&prio_tmp);
	//fprintf(stderr,"function:%s,line:%d\n",__FUNCTION__,__LINE__);
#endif

	/**
	* init mutual semaphore
	*/
#ifdef VIDEO_PREVIEW_EN
	vsem = (sem_t *)malloc(sizeof(sem_t));
	if(vsem){
		sem_init(vsem, 0, 1);
	}
	
	for(i=0;i<PREVIEW_MAX;i++)
	{
		vpreview_buffer_array[i] = NULL;
	}
#endif

_VE_OPEN_OUT:	
	
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}

static int ve_close(void * handle)
{

	INT32U i;
	
	struct am_bus_priority prio_tmp;
	
	SWFEXT_FUNC_BEGIN(handle);
	
//#if AM_CHIP_ID == 1213
#if 0
	am_soc_get_bus_priority(&prio_tmp);
	/**
	* reset to default 0x30248651.
	*/
	prio_tmp.special_dma = 3;
	prio_tmp.mac=0;
	prio_tmp.ahb_bus = 2;
	prio_tmp.graph_2d=4;
	prio_tmp.de = 8;
	prio_tmp.dac=6;
	prio_tmp.vdc=5;
	prio_tmp.axi_bus = 1;
	//dump_prio_value(prio_tmp);
	am_soc_set_bus_priority(&prio_tmp);
#endif
	
	fprintf(stderr,"function:%s,line:%d\n",__FUNCTION__,__LINE__);

	
	stop_seek_play_thread();
	if(videohandle)
	{
		video_cmd_s cmd;
		if(playstat!=VE_STOP&&playstat!=VE_IDLE)
		{
			printf("stop first when exit\n");
			cmd.con = V_STOP;
			_video_dec_send_cmd(videohandle,&cmd);
			playstat = VE_READY_STOP;
			set_de_defaultcolor();
			while(playstat != VE_STOP)
			{
				OSSleep(1);
			}
			free_video_heap();
			if((SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE) == 0)
			{
				SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
			}
		}
		cmd.con  = V_TERMINAL;
		_video_dec_send_cmd(videohandle,&cmd);	
		_video_dec_close();	
		videohandle = NULL;
	}
	
	
#ifdef VIDEO_PREVIEW_EN
	if(vsem){
		_ve_sem_wait(vsem);
		sem_destroy(vsem);
		free(vsem);
		vsem = NULL;
	}
	if(vsem_previewok){
		sem_destroy(vsem_previewok);
		free(vsem_previewok);
		vsem_previewok = NULL;
	}

	if(vpreview_buffer){
		SWF_Free(vpreview_buffer);
		vpreview_buffer=NULL;
	}	
	for(i=0;i<PREVIEW_MAX;i++)
	{
		if(vpreview_buffer_array[i]){
			SWF_Free(vpreview_buffer_array[i]);
			vpreview_buffer_array[i]=NULL;
		}
	}
	if(prevheap){
		printf("ve_close swf free ok\n");
		SWF_Free(prevheap);
		prevheap=NULL;
	}
#endif

	playstat=VE_IDLE;

	/** FIXME: [re-setup image and audio decoder]*/
	
	
	SWFEXT_FUNC_END();	
}



	
	
static int ve_play(void * handle)
{
	int n;
	video_cmd_s cmd;
	unsigned char *heap1=NULL;
	long heapsize = 0;
	INT32U i;

	
	SWFEXT_FUNC_BEGIN(handle);

#if defined(MODULE_CONFIG_FLASH_TYPE) && MODULE_CONFIG_FLASH_TYPE != 0
	int fdw = open("/proc/sys/vm/drop_caches", O_RDWR);
	if(fdw >= 0)
	{
		char val[2] = "1";
		write(fdw, val, sizeof(val));
		close(fdw);
	}
#endif
	
#ifdef VIDEO_PREVIEW_EN
	_ve_sem_wait(vsem);
	if(vpreview_buffer){
		SWF_Free(vpreview_buffer);
		vpreview_buffer=NULL;
	}
	for(i=0;i<PREVIEW_MAX;i++)
	{
		if(vpreview_buffer_array[i]){
			SWF_Free(vpreview_buffer_array[i]);
			vpreview_buffer_array[i]=NULL;
		}
	}
	if(prevheap){
		printf("ve_play swf free ok\n");
		SWF_Free(prevheap);
		prevheap=NULL;
	}
	
#endif



	switch(playstat){
		case VE_STOP:
		case VE_IDLE:
		case VE_ERROR:
			goto _DO_PLAY;
			break;
		case VE_FF:
			
			goto __PLAY__OUT;
			break;
		case VE_PAUSE:
			
			goto __PLAY__OUT;
			break;
		case VE_FB:
			
			goto __PLAY__OUT;
			break;
		default:
			printf("Play error state %d\n",playstat);
			goto __PLAY__OUT;
			break;

	}


_DO_PLAY:

	if(vheap1 != NULL)
	{
		printf("Memory is ready,just send play cmd\n");
		heap1 = vheap1;
		heapsize = heap1size;
		goto __SET_PARAM;
	}

	
	
	set_de_defaultcolor();
	
	/**
	* set fui to sleep.
	*/
	if(SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE)
	{
		/** first set swf to sleep, and buffer will be released */
		SWF_SetState(SWF_GetActiveInst(), SWF_STATE_SLEEP);
	}


	heap1 = (unsigned char *)SWF_Malloc(_VIDEO_HEAP1_SIZE);
	if(heap1 != NULL){
		heap1size = heapsize = _VIDEO_HEAP1_SIZE;
		goto __SET_PARAM;
	}

	heap1 = (unsigned char *)SWF_Malloc(_VIDEO_HEAP2_SIZE);
	if(heap1 != NULL){
		heap1size = heapsize = _VIDEO_HEAP2_SIZE;
		goto __SET_PARAM;
	}

	heap1 = (unsigned char *)SWF_Malloc(_VIDEO_HEAP3_SIZE);
	if(heap1 != NULL){
		heap1size = heapsize = _VIDEO_HEAP3_SIZE;
		goto __SET_PARAM;
	}
	
	/**
	* cannot malloc memory for video, just go out.
	*/
	if((SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE) == 0){
		SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
	}
	vheap1 = NULL;
	goto __PLAY__OUT;
	
	/**
	* set play parameters.
	*/
__SET_PARAM:
	cmd.con = V_SET_PARAM;
	cmd.par.set_par.is_preview = 0;
	if(max_frame_width > 0)
	{
		cmd.par.set_par.frame_w = max_frame_width;
		cmd.par.set_par.frame_h = cmd.par.set_par.frame_w*
			screen_output_data.screen_output_height/screen_output_data.screen_output_width;
	}
	else
	{
		cmd.par.set_par.frame_w = screen_output_data.screen_output_width;
		cmd.par.set_par.frame_h = screen_output_data.screen_output_height;	
	}

#if AM_CHIP_ID != 1213
	/**
	* The 1213 supports 1080P input and output.
	*/
	if(screen_output_data.screen_output_mode!=16 && cmd.par.set_par.frame_w > 1280 && cmd.par.set_par.frame_h > 720){
		printf("%s,%d:cmd.par.set_par.frame_w > 1280\n",__FILE__,__LINE__);
		cmd.par.set_par.frame_w = 1280;
		cmd.par.set_par.frame_h = 720;
	}
#endif

	if(screen_output_data.screen_output_true == 0)
	{
		_set_hdmi_mode(&screen_output_data.screen_output_mode);
		printf("set hdmi mode is %d\n",screen_output_data.screen_output_mode);
	}
	printf("screen real width is %d,height is %d\n",cmd.par.set_par.frame_w,cmd.par.set_par.frame_h);
	
	cmd.par.set_par.display_mode = get_video_disp_ratio();
#ifndef TEST_MEM	
	cmd.par.set_par.pix_fmt = VIDEO_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR;//VIDEO_PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
	cmd.par.set_par.p_heap.h_b_size = 1;
	
	//get_phy_viraddr(&cmd.par.set_par.p_heap.bus_addr[0],&cmd.par.set_par.p_heap.vir_addr[0]);
	vheap1 = heap1;
	p_printf("vheap1 = %x,LINE=%d\n",vheap1,__LINE__);
	cmd.par.set_par.p_heap.vir_addr[0] = heap1;
	cmd.par.set_par.p_heap.bus_addr[0] =((unsigned int)fui_get_bus_address((unsigned long)heap1));
	cmd.par.set_par.p_heap.size[0] = heapsize;
#else
	cmd.par.set_par.pix_fmt = VIDEO_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR;
	vheap1 = heap1;
	p_printf("vheap1 = %x,LINE=%d\n",vheap1,__LINE__);
	
	if(!heap_mem_handle)
		heap_mem_handle = OSmopen();
	if(!heap_mem_ptr)
		heap_mem_ptr = OSmmap(heap_mem_handle, heap_bus_addr, heap_size);
	if(!pRegDRAM)
		pRegDRAM = (unsigned long*)OSmmap(heap_mem_handle, 0x10070000, 4096);
	REG_WRITE(pRegDRAM, 0xB0070000, 0x4c, 0x000000c3);
	REG_READ(pRegDRAM, 0xB0070000, 0x4c);
	cmd.par.set_par.p_heap.vir_addr[0] = heap1;
	cmd.par.set_par.p_heap.bus_addr[0] =((unsigned int)fui_get_bus_address((unsigned long)heap1));
	cmd.par.set_par.p_heap.size[0] = _VIDEO_HEAP1_SIZE;
	cmd.par.set_par.p_heap.h_b_size = 2;	
	cmd.par.set_par.p_heap.vir_addr[1] = heap_mem_ptr;
	cmd.par.set_par.p_heap.bus_addr[1] = heap_bus_addr;
	cmd.par.set_par.p_heap.size[1] = heap_size;
#endif
	memcpy(&videoPlayParaCmd, &cmd, sizeof(cmd));
	p_printf("vir==0x%x,phy==0x%x,size==0x%x\n",cmd.par.set_par.p_heap.vir_addr[0],cmd.par.set_par.p_heap.bus_addr[0],cmd.par.set_par.p_heap.size[0]);
	_video_dec_send_cmd(videohandle,&cmd);
	
	/**
	* set play
	*/
	playstat=VE_PLAY;
	cmd.con = V_PLAY;
	_video_dec_send_cmd(videohandle,&cmd);
	
	
__PLAY__OUT:

#ifdef VIDEO_PREVIEW_EN
	_ve_sem_post(vsem);
#endif

	SWFEXT_FUNC_END();	
}


static int ve_stop(void * handle)
{
	video_cmd_s cmd;
	int hdmi_mode;

	SWFEXT_FUNC_BEGIN(handle);
	p_printf("ve_stop\n");
	if(is_video_stream==1){
		am_stream_stop(NULL);
		is_video_stream = 0;

	}
	
	stop_seek_play_thread();
	if(videohandle)
	{
		switch(playstat){
			case VE_FF:
				
				goto _DO_STOP;
				break;
				
			case VE_PAUSE:
				
				goto _DO_STOP;
				break;
				
			case VE_FB:
				
				goto _DO_STOP;
				break;
				
			case VE_PLAY:
	
				goto _DO_STOP;
				break;
				
			case VE_STOP:
			case VE_IDLE:
			case VE_ERROR:
			default:
				p_printf("stop error state %d\n",playstat);
				goto _V_STOP_OUT;
				break;

		}

_DO_STOP:	
	
		printf("stop video\n");
		set_de_defaultcolor();
		playstat = VE_READY_STOP;
		
		cmd.con = V_STOP;
		_video_dec_send_cmd(videohandle,&cmd);
		
		
		while(playstat != VE_STOP)
		{
			OSSleep(1);
		}
		
	}
	
	
_V_STOP_OUT:
	if(screen_output_data.screen_output_true == 0)
	{
		hdmi_mode = FMT_1650x750_1280x720_60P;
		_set_hdmi_mode(&hdmi_mode);
		printf("stop--set hdmi mode is %d\n",hdmi_mode);
	}
	
	free_video_heap();
	
	if((SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE) == 0)
	{
		SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
	}

	restore_de_config(1);
	
	SWFEXT_FUNC_END();	
}

static int __ve_pause()
{
	video_cmd_s cmd;
	
	p_printf("ve_pause\n");	
	
	if(videohandle)
	{
		printf("playstat_pause=========%d\n",playstat);
		
		if(playstat==VE_PLAY||playstat==VE_FB||playstat==VE_FF){
			cmd.con  = V_PAUSE;
			_video_dec_send_cmd(videohandle,&cmd);
			playstat = VE_PAUSE;
		}
		else{
			printf("pause error state %d\n",playstat);
		}	
	}

	return 0;
}

static int ve_pause(void * handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	if(is_video_stream && get_video_play_time_for_dmr() == 0)
		goto __ve_pause_end___;
	__ve_pause();
__ve_pause_end___:	
	SWFEXT_FUNC_END();	
}

static int __ve_resume()
{
	video_cmd_s cmd;
	
	p_printf("ve_resume\n");
	
	if(videohandle)
	{
		switch(playstat)
		{
			case VE_FF:
			case VE_FB:
			{
				#ifndef SEEK_PLAY_THREAD
				cmd.con = V_PLAY;//cancel FF/FB
				_video_dec_send_cmd(videohandle, &cmd);
				OSSleep(500);
				#else
				stop_seek_play_thread();
				#endif
				if ((playstat == VE_FF) || (playstat == VE_FB))
				{
					cmd.con  = V_RESUME;
					_video_dec_send_cmd(videohandle,&cmd);	
					playstat = VE_PLAY;
				}
				break;
		
			}
				
			case VE_PAUSE:
			{
				cmd.con = V_RESUME;
				_video_dec_send_cmd(videohandle, &cmd);
				playstat = VE_PLAY;
				break;
			}
			case VE_STOP:
			case VE_IDLE:
			case VE_PLAY:
			case VE_ERROR:
			default:
				p_printf("resume error state %d\n",playstat);
				break;
		}
	}

	return 0;
}


static int ve_resume(void * handle)
{

	SWFEXT_FUNC_BEGIN(handle);

	__ve_resume();

	SWFEXT_FUNC_END();	
}


static int ve_fast_forward(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	p_printf("ve_fast_forward\n");
	video_cmd_s cmd;
	
	int step_from_swf=Swfext_GetNumber();

	
	switch(playstat){
		case VE_PLAY:
		case VE_FB:		
		case VE_PAUSE:
		case VE_FF:
			break;		
		case VE_STOP:
		case VE_IDLE:
		case VE_ERROR:
		default:
			goto _FF_OUT;
	}
	if(playstat== VE_PLAY)
	{	

			
		cmd.con  = V_PAUSE;
		if(_video_dec_send_cmd(videohandle,&cmd))
			goto _FF_OUT;
	}
	
	#ifndef SEEK_PLAY_THREAD
	fprintf(stdout,"function:%s,line:%d,step_from_swf=%d\n",__FUNCTION__,__LINE__,step_from_swf);
	if(step_from_swf<=0)
		cmd.par.seek_continue_step = calc_fastplay_step(media_file_info.total_time);
	else
		cmd.par.seek_continue_step=step_from_swf*calc_fastplay_step(media_file_info.total_time);
	cmd.con  = V_SEEK_CONTINUE;
	_video_dec_send_cmd(videohandle, &cmd);
	playstat = VE_FF;
	#else
	printf("media_file_info.total_time == %d\n", media_file_info.total_time);
	if(media_file_info.total_time > 2000){
		playstat = VE_FF;
		start_seek_thread(step_from_swf);
	}else{
		fprintf(stdout,"function:%s,line:%d,step_from_swf=%d\n",__FUNCTION__,__LINE__,step_from_swf);
		cmd.par.seek_continue_step=step_from_swf*100;
		cmd.con  = V_SEEK_CONTINUE;
		_video_dec_send_cmd(videohandle, &cmd);
		playstat = VE_FF;
	}
	#endif
_FF_OUT:
	SWFEXT_FUNC_END();	
}


static int ve_fast_backward(void * handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	p_printf("ve_fast_backward\n");
	video_cmd_s cmd;
	int step_from_swf=Swfext_GetNumber();
	
	switch(playstat){
		case VE_PLAY:
		case VE_PAUSE:
		case VE_FF:
		case VE_FB:
			break;
		case VE_STOP:
		case VE_IDLE:
		case VE_ERROR:
		default:
			goto _FB_OUT;
	}
			
	fprintf(stderr,"function:%s,line:%d,step_from_swf=%d\n",__FUNCTION__,__LINE__,step_from_swf);
	
	if(step_from_swf<=0)
		cmd.par.seek_continue_step = 0 -calc_fastplay_step(media_file_info.total_time);
	else
		cmd.par.seek_continue_step=0-step_from_swf*calc_fastplay_step(media_file_info.total_time);

	_ve_sem_wait(vsem);
	if ((playstat == VE_PLAY))
	{
	
		cmd.con  = V_PAUSE;
		if(_video_dec_send_cmd(videohandle,&cmd))
			goto _FB_RELEASE_SEM;
	}	
	playstat = VE_FB;
	
	#ifndef SEEK_PLAY_THREAD
	fprintf(stdout,"function:%s,line:%d, playstat=%d\n",__FUNCTION__,__LINE__, playstat);
	
	cmd.par.seek_continue_step=0-step_from_swf*calc_fastplay_step(media_file_info.total_time);
	cmd.con  = V_SEEK_CONTINUE;
	_video_dec_send_cmd(videohandle,&cmd);
	#else
	if(media_file_info.total_time > 2000){
		start_seek_thread(0 - step_from_swf);
	}else{
		fprintf(stdout,"function:%s,line:%d, playstat=%d\n",__FUNCTION__,__LINE__, playstat);
		cmd.par.seek_continue_step=-step_from_swf*100;
		cmd.con  = V_SEEK_CONTINUE;
		_video_dec_send_cmd(videohandle,&cmd);
	}
	#endif
	
_FB_RELEASE_SEM:
	_ve_sem_post(vsem);
_FB_OUT:
	SWFEXT_FUNC_END();	
}

static int ve_seek_play(void* handle)
{
	int seek_time;
	int totaltime;
	int ret;
	SWFEXT_FUNC_BEGIN(handle);
	seek_time = Swfext_GetNumber();
	p_printf("ve_seek_play\n");
	video_cmd_s cmd;
	if(videohandle){

		switch(playstat){
			case VE_STOP:
			case VE_IDLE:
			case VE_ERROR:
				ret = -1;
				goto _SEEK_OUT;
				break;
				
			case VE_FB:
			case VE_FF:
			case VE_PLAY:
				cmd.con  = V_PAUSE;
				_video_dec_send_cmd(videohandle,&cmd);
				break;
				
			case VE_PAUSE:
				goto _DO_SEEKPLAY;
				break;

			default:
				printf("error state %d\n",playstat);
				ret =-1;
				goto _SEEK_OUT;
				break;
		}

	}
	else
	{
		ret =-1;
		goto _SEEK_OUT;
	}
	
_DO_SEEKPLAY:
  #if 1 //[Sanders.130227] - EZ Stream porting
	if (stream_handle != NULL)
	{
		am_cache_avseekplay();
	}
  #endif

	totaltime = media_file_info.total_time/1000;
	if(seek_time>=totaltime)
		seek_time = totaltime;
	else if(seek_time<0)
		seek_time = 0;
	cmd.par.seek_single_time= seek_time*1000;
	cmd.con  = V_SEEK_SINGLE;
	_video_dec_send_cmd(videohandle,&cmd);
	playstat = VE_PAUSE;
	ret =0;
_SEEK_OUT:

	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}

#if 1
//add new
static int _check_file_exist(const char *filename)
{
	return access(filename, 0); 
}
#endif


static int __videoSetFile()
{
	video_cmd_s cmd;
	int err = 0;


	video_error = 0;
	video_support_status = 0;
	
	memset(&cmd, 0, sizeof(video_cmd_s));
	
	cmd.con = V_SET_FILE;
	cmd.par.set_file.fileflag =0;
	cmd.par.set_file.demuxname=NULL;
	cmd.par.set_file.av_fifo_enable=1;
	strcpy(cmd.par.set_file.filename,vplay_file);	
	dmr_seek_allowed = 1;

	if(_check_file_exist(cmd.par.set_file.filename)!=-1){	
		//printf("local filesystem file\n");
		cmd.par.set_file.f_io.open = _video_fopen;
		cmd.par.set_file.f_io.read = (void*)_video_read_packet;
		cmd.par.set_file.f_io.write = (void*)_video_write_packet;
		cmd.par.set_file.f_io.seek_set = (void*)_video_file_seek_set;
		cmd.par.set_file.f_io.seek_end = (void*)_video_file_seek_end;
		cmd.par.set_file.f_io.tell = _video_get_pos;
		cmd.par.set_file.f_io.close = _video_fclose;
		cmd.par.set_file.f_io.get_length = NULL;
		cmd.par.set_file.f_io.get_seekable = _video_get_seekable;
		cmd.par.set_file.f_io.set_stop = NULL;
		is_video_stream = 0;
	}
	else{
		printf("not local filesystem file\n");
		cmd.par.set_file.f_io.open = _video_fopen_stream;
		cmd.par.set_file.f_io.read = (void*)_video_read_packet_stream;
		//cmd.par.set_file.f_io.write = (void*)_video_write_packet;
		cmd.par.set_file.f_io.seek_set = (void*)_video_file_seek_set_stream;
		cmd.par.set_file.f_io.seek_end = (void*)_video_file_seek_end_stream;
		cmd.par.set_file.f_io.tell = _video_get_pos_stream;
		cmd.par.set_file.f_io.close = _video_fclose_stream;
		cmd.par.set_file.f_io.get_length = _video_get_filesize_stream;
		cmd.par.set_file.f_io.get_seekable = _video_get_seekable_stream;
		cmd.par.set_file.f_io.set_stop = _video_stop_set_stream;
		cmd.par.set_file.fileflag=AVFMT_STREAM;	
		is_video_stream = 1;
	}
	memset(&media_file_info,0,sizeof(file_info_s));
	cmd.par.set_file.pInfo = &media_file_info;
	printf("++++++set file : %s++++++++\n",cmd.par.set_file.filename);
	
	sem_init(&v_info, 0, 0);
	err = _video_dec_send_cmd(videohandle,&cmd);
	if(is_video_stream != 1){
		_ve_sem_wait(&v_info);
	}
	
	sem_destroy(&v_info);

	return err;

}


static int ve_set_file(void * handle)
{
	char * file;
	int n;
	int err;
	
	p_printf("ve_set_file\n");
	
	SWFEXT_FUNC_BEGIN(handle);

	int set_file_ret = 1;
	n = Swfext_GetParamNum();
	if(n == 0){
		//file = "c:\\";
		printf("no file, swf set video file error\n");
		vplay_file[0] = 0;
		set_file_ret = 0;
		goto _SET_FILE_END_;
	}
	else{
		file = Swfext_GetString();
	}

	strncpy(vplay_file,file, sizeof(vplay_file));
	vplay_file[sizeof(vplay_file)-1] = '\0';
	err = __videoSetFile();

#ifdef  MODULE_CONFIG_DLNA	
	#if 0
	int rtn = 0;
	if (err == -1 || video_error == 1)
	{
		rtn = 6;
		printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
		dlna_dmr_cb(&rtn);
	}
	#endif		
#endif		

	set_file_ret = !video_error ;
	
_SET_FILE_END_:
	Swfext_PutNumber((int)set_file_ret);
	SWFEXT_FUNC_END();	

}



static void do_get_one_frame(char * file,unsigned char *buf,int x,int y,int w,int h)
{
	
#ifdef VIDEO_PREVIEW_EN
	int rtn;
	int i;
	video_cmd_s cmd;
	unsigned char *prev_heap1=NULL;
	prev_entry p_e;
	
/** FIXME: [add get frame function]*/
	
	if(video_error==0){
		p_printf("%s,%d:prevheap is 0x%x\n",__FILE__,__LINE__,prevheap);
		if(prevheap==NULL){
			prev_heap1 = (unsigned char *)SWF_Malloc(_VIDEO_PREV_SIZE);
			if(prev_heap1 == NULL){
				prevheap = NULL;
				printf("%s,%d:swf malloc is error!\n",__FILE__,__LINE__);
			}
		}
		else{
			prev_heap1 = prevheap;
		}
		
		cmd.con = V_SET_PARAM;
		cmd.par.set_par.frame_w = w;
		cmd.par.set_par.frame_h = h;
		cmd.par.set_par.display_mode = VIDEO_DISPLAY_MODE_LETTER_BOX;
		cmd.par.set_par.pix_fmt = VIDEO_PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
		cmd.par.set_par.is_preview=1;
		cmd.par.set_par.p_heap.h_b_size = 1;
		prevheap = prev_heap1;
		cmd.par.set_par.p_heap.vir_addr[0] = prev_heap1;
		cmd.par.set_par.p_heap.bus_addr[0] =((unsigned int)fui_get_bus_address((unsigned long)prev_heap1));
		cmd.par.set_par.p_heap.size[0] = _VIDEO_PREV_SIZE;
		rtn = _video_dec_send_cmd(videohandle,&cmd);
		p_printf("%s,%d:V_SET_PARAM,rtn is %d\n",__FILE__,__LINE__,rtn);
		if(rtn == -1){
			printf("%s,%d:video set param error\n",__FILE__,__LINE__);
			//video_error = 1;
		}
		else{
			//video_error = 0;
		}
	}
	
	
	prev_actual_w = 0;
	prev_actual_h = 0;
	
	if(video_error==0){
		vsem_previewok = (sem_t *)malloc(sizeof(sem_t));
		if(vsem_previewok){
			sem_init(vsem_previewok, 0, 0);
		}
		cmd.con  = V_GET_PERVIEW;
		cmd.par.prev_par.prev_num = 1;
		if(media_file_info.total_time>20*1000){
			p_e.pre_time = 15*1000;
		}
		else{
			p_e.pre_time = media_file_info.total_time/2;
		}
		p_e.prev_w = w;
		p_e.prev_h = h;
		p_e.vir_addr = buf;
		p_e.actual_pre_h = 0;
		p_e.actual_pre_w = 0;
		for(i=0;i<w*h/2;i++){
			*(INT32S *)(buf+i*4)=0x80108010;
		}
		p_printf("total time is %d,preview time is %d\n",media_file_info.total_time,p_e.pre_time);
		cmd.par.prev_par.pprev[0] = &p_e;
		rtn = _video_dec_send_cmd(videohandle,&cmd);
		p_printf("%s,%d:V_GET_PERVIEW,rtn is %d\n",__FILE__,__LINE__,rtn);
		if(rtn == -1){
			printf("%s,%d:video get perview error\n",__FILE__,__LINE__);
			//video_error = 1;
		}
		else{
			//video_error = 0;
		}			
			
		if(vsem_previewok){
			_ve_sem_wait(vsem_previewok);
		}
		p_printf("%s,%d:pre w:%d h:%d\n",__FILE__,__LINE__,w,h);
		p_printf("%s,%d:pre w:%d h:%d\n",__FILE__,__LINE__,cmd.par.prev_par.pprev[0]->actual_pre_w,cmd.par.prev_par.pprev[0]->actual_pre_h);
		prev_actual_w = cmd.par.prev_par.pprev[0]->actual_pre_w;
		prev_actual_h = cmd.par.prev_par.pprev[0]->actual_pre_h;

		if(vsem_previewok){
			sem_destroy(vsem_previewok);
			free(vsem_previewok);
			vsem_previewok = NULL;
		}
	}
	
	if(vsem == NULL){
		return;
	}
	_ve_sem_wait(vsem);
	
	if(buf==NULL){
		_ve_sem_post(vsem);
		return;
	}

	_ve_sem_post(vsem);

	prev_heap1=NULL;
	
#endif

}

static void do_copy_buffer(char *sour_buf,int s_w,int s_h,char *des_buf,int d_w,int d_h)
{
	int source_locate = 0;
	int destination_locate = 0;
#ifdef VIDEO_PREVIEW_EN
	p_printf("s_w is %d,s_h is %d\n",s_w,s_h);
	p_printf("d_w is %d,d_h is %d\n",d_w,d_h);
	//if(s_w<d_w && s_h<d_h)
	if((s_w<d_w && s_h<d_h) || (s_w<d_w && s_h==d_h) || (s_w==d_w && s_h<d_h)){
		destination_locate = (d_h-s_h)*d_w + (d_w-s_w);
		while(source_locate<s_w*s_h*2){
			p_printf("%s,%d:source locate is %d,destination locate is %d\n",__FILE__,__LINE__,source_locate,destination_locate);
			memcpy(des_buf+destination_locate,sour_buf+source_locate,s_w*2);
			source_locate += s_w*2;
			destination_locate += d_w*2;
		}
	}
	else if(s_w==d_w && s_h==d_h){
		memcpy(des_buf,sour_buf,d_w*d_h*2);
	}
	
#endif

}

static int ve_get_preview_status(int id)
{
	return 1;
}


static int ve_attach_picture(void * handle)
{
	void *  target=NULL;
	unsigned int w,h;
	INT32U i;
	INT32U index;

	SWFEXT_FUNC_BEGIN(handle);

	target = Swfext_GetObject();
	w = Swfext_GetNumber();
	h = Swfext_GetNumber();
	index = Swfext_GetNumber();
#ifdef VIDEO_PREVIEW_EN
	printf("%s,%d:attach_picture index is %d,w is %d,h is %d,actual_w is %d,actual_h is %d\n",__FILE__,__LINE__,index,w,h,prev_actual_w,prev_actual_h);
	if(prev_actual_w>0 && prev_actual_h>0)
	{
		if(index==0)
		{
			if(vpreview_buffer)
				SWF_AttachBitmap(target,(unsigned int*)vpreview_buffer,w,h,prev_actual_w,prev_actual_h,prev_actual_w,SWF_BMP_FMT_YUV422,ve_get_preview_status);
		}
		else
		{
			index--;
			if(vpreview_buffer_array[index])
				SWF_AttachBitmap(target,(unsigned int*)vpreview_buffer_array[index],w,h,prev_actual_w,prev_actual_h,prev_actual_w,SWF_BMP_FMT_YUV422,ve_get_preview_status);
		}
	}
	
#endif

	Swfext_PutNumber(1);

	SWFEXT_FUNC_END();

}

static int ve_detach_picture(void * handle)
{
	int clone;
	void *target=NULL;
	INT32U index;
	
	SWFEXT_FUNC_BEGIN(handle);

	target = Swfext_GetObject();
	clone = Swfext_GetNumber();
	index = Swfext_GetNumber();
#ifdef VIDEO_PREVIEW_EN
	if(target != NULL){
		SWF_DetachBitmap(target,clone);
		printf("%s,%d:clone is %d,detach_picture release buffer index is %d\n",__FILE__,__LINE__,clone,index);

		if(index)
		{
			index--;
			if(index >= PREVIEW_MAX)
				goto ve_detach_picture_end;
			if(vpreview_buffer_array[index])
			{
				SWF_Free(vpreview_buffer_array[index]);
				vpreview_buffer_array[index] = NULL;
			}
		}
		else
		{
			if(vpreview_buffer)
			{
				SWF_Free(vpreview_buffer);
				vpreview_buffer = NULL;
			}
		}	
	}
#endif
ve_detach_picture_end:
	Swfext_PutNumber(1);

	SWFEXT_FUNC_END();
}

static int ve_get_one_frame(void *handle)
{
	int w,h;
	INT32U index;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	w = Swfext_GetNumber();
	h = Swfext_GetNumber();
	index = Swfext_GetNumber();
	
#ifdef VIDEO_PREVIEW_EN
	printf("%s,%d:get_one_frame index is %d\n",__FILE__,__LINE__,index);
	if(index)
	{
		index--;
		if(index >= PREVIEW_MAX)
			goto ve_get_one_frame_end;
		
		if(vpreview_buffer_array[index])
		{
			SWF_Free(vpreview_buffer_array[index]);
			vpreview_buffer_array[index] = NULL;

			vpreview_buffer_array[index]=SWF_Malloc(w*h*2);
			if(vpreview_buffer_array[index]){
				p_printf("memory stat======(0x%x)/(0x%x)\n",SWF_MemCurrent(),SWF_MemTotal());
			}
			else{
				printf("%s,%d:small prev malloc failed\n",__FILE__,__LINE__);
			}
		}
		else
		{
			vpreview_buffer_array[index]=SWF_Malloc(w*h*2);
			if(vpreview_buffer_array[index]){
				p_printf("memory stat======(0x%x)/(0x%x)\n",SWF_MemCurrent(),SWF_MemTotal());
			}
			else{
				printf("%s,%d:small prev malloc failed\n",__FILE__,__LINE__);
			}
		}
		p_printf("%s,%d:small malloc %d ok,hd=0x%x\n",__FILE__,__LINE__,index,vpreview_buffer_array[index]);
		if(vpreview_buffer_array[index]){
			do_get_one_frame(vplay_file,vpreview_buffer_array[index],0,0,w,h);
		}
	}
	else{
		if(vpreview_buffer){
			if(vpreview_bufsize<w*h*2){
				SWF_Free(vpreview_buffer);
				vpreview_buffer=NULL;
				vpreview_buffer=SWF_Malloc(w*h*2);
				if(vpreview_buffer){
					vpreview_bufsize=w*h*2;
				}
				else{
					vpreview_bufsize=0;
				}
			}
		}
		else{
			vpreview_buffer=SWF_Malloc(w*h*2);
			if(vpreview_buffer){
				vpreview_bufsize=w*h*2;
			}
			else{
				vpreview_bufsize=0;
			}
		}
		p_printf("%s,%d:vpreview_buffer is 0x%x\n",__FILE__,__LINE__,vpreview_buffer);
		if(vpreview_buffer){
			do_get_one_frame(vplay_file,vpreview_buffer,0,0,w,h);
		}
	}
#endif
ve_get_one_frame_end:
	p_printf("%s,%d,video_error is %d,video_support_status is %d\n",__FILE__,__LINE__,video_error,video_support_status);
	Swfext_PutNumber(video_support_status);
	SWFEXT_FUNC_END();
}
int ezcast_ve_get_state()
{

	int rtn;
	static int pre_video_status=-1;
	if(videohandle){
		if(playstat==VE_STOP || playstat==VE_IDLE || playstat==VE_PAUSE){
			return playstat;
		}
		else{
			
			return playstat;
		}
		
	}
	else{
		/** if video engine handle null, just return idle */
		playstat=VE_IDLE;
		return playstat;
	}
#ifdef  MODULE_CONFIG_DLNA	

	rtn = playstat;
	if(pre_video_status!=playstat)
		dlna_dmr_cb(&rtn);
	

#endif	
	pre_video_status=playstat;
	
}
static int ve_get_state(void * handle)
{
	int rtn;
	static int pre_video_status=-1;
	SWFEXT_FUNC_BEGIN(handle);
	
	if(videohandle){
		if(playstat==VE_STOP || playstat==VE_IDLE || playstat==VE_PAUSE){
			Swfext_PutNumber(playstat);
		}
		else{
			
			Swfext_PutNumber(playstat);
		}
		
	}
	else{
		/** if video engine handle null, just return idle */
		playstat=VE_IDLE;
		Swfext_PutNumber(VE_IDLE);
	}
#ifdef  MODULE_CONFIG_DLNA	

	rtn = playstat;
	if(pre_video_status!=playstat)
		dlna_dmr_cb(&rtn);
	

#endif	
	pre_video_status=playstat;
	SWFEXT_FUNC_END();	
}

static int ve_get_total_time(void * handle)
{
	int totaltime = 0;
	
	SWFEXT_FUNC_BEGIN(handle);
	p_printf("ve_get_total_time\n");
	/** FIXME: [ please finish it ]*/
	
	if(videohandle){
		totaltime = media_file_info.total_time/1000;	
		if(totaltime < 0)
			totaltime = 0;
		Swfext_PutNumber(totaltime);
	}
	else{
		Swfext_PutNumber(0);
	}	

	
	SWFEXT_FUNC_END();	
}


long get_video_total_time_for_dmr()
{
	if(video_set_file_ok == 0)
		return 0;
	long totaltime = 0;
	//printf("*****************get_total_time_for_dmr********************************\n");
#if 1
	if(videohandle){
		totaltime = media_file_info.total_time;
	}
	
	//printf("[%s,%d]totaltime ===========%ld",__FUNCTION__,__LINE__,totaltime);
#endif
	return totaltime;
}


long get_video_play_time_for_dmr()
{
	long playtime = 0;
	if(video_set_file_ok == 0)
		return 0;
	//printf("*****************get_play_time_for_dmr********************************\n");
#if 1
	if(videohandle){
		video_cmd_s cmd;
		cmd.con  = V_GET_CUR_TIME;
		if(_video_dec_send_cmd(videohandle,&cmd)){
			if(playstat == VE_STOP)
				playtime = media_file_info.total_time;
			goto end;
		}
		playtime = cmd.par.cur_time;
	}
	else{
		p_printf("%s,%d:videohandle is NULL\n",__FILE__,__LINE__);
		playtime = 0;
	}
	end:
	//printf("[%s,%d]playtime ===========%ld",__FUNCTION__,__LINE__,playtime);
#endif
	return playtime;
}

int seek_video_play_for_dmr(long seek_time)
{
	int totaltime;
	int ret;
	video_cmd_s cmd;
	//printf("seek_time===========%ld\n",seek_time);
	printf("[%s,%d]dmr_seek_allowed===========%d\n",__FILE__,__LINE__,dmr_seek_allowed);
	if(video_set_file_ok == 0 || dmr_seek_allowed == 0 || get_video_play_time_for_dmr() == 0)
		return -1;
	if(videohandle){

		switch(playstat){
			case VE_STOP:
			case VE_IDLE:
			case VE_ERROR:
				ret = -1;
				goto _SEEK_OUT_FOR_DMR;
				break;
				
			case VE_FB:
			case VE_FF:
			case VE_PLAY:
				cmd.con  = V_PAUSE;
				_video_dec_send_cmd(videohandle,&cmd);
				break;
				
			case VE_PAUSE:
				goto _DO_SEEKPLAY;
				break;

			default:
				printf("error state %d\n",playstat);
				ret =-1;
				goto _SEEK_OUT_FOR_DMR;
				break;
		}

	}
	else
	{
		ret =-1;
		goto _SEEK_OUT_FOR_DMR;
	}
	
_DO_SEEKPLAY:
  #if 1 //[Sanders.130227] - EZ Stream porting
	if (stream_handle != NULL)
	{
		am_cache_avseekplay();
	}
  #endif

	totaltime = media_file_info.total_time;
	if(seek_time>=totaltime)
		seek_time = totaltime;
	else if(seek_time<0)
		seek_time = 0;
	cmd.par.seek_single_time= seek_time;
	cmd.con  = V_SEEK_SINGLE;
	dmr_seek_allowed = 0;
	_video_dec_send_cmd(videohandle,&cmd);
	playstat = VE_PAUSE;
	ret =0;
_SEEK_OUT_FOR_DMR:

	return 0;
}


static int ve_get_play_time(void * handle)
{
	int playtime = 0;
	
	SWFEXT_FUNC_BEGIN(handle);
	p_printf("ve_get_play_time\n");
	if(videohandle){
		video_cmd_s cmd;
		cmd.con  = V_GET_CUR_TIME;
		if(_video_dec_send_cmd(videohandle,&cmd)){
			if(playstat == VE_STOP)
				playtime = media_file_info.total_time/1000;
			goto end;
		}
		playtime = cmd.par.cur_time/1000;
	}
	else{
		p_printf("%s,%d:videohandle is NULL\n",__FILE__,__LINE__);
		playtime = 0;
	}
	end:	
		if(playtime < 0)
			playtime = 0;
		Swfext_PutNumber(playtime);
		SWFEXT_FUNC_END();		
}


static int ve_get_play_mode(void * handle)
{
	int playmode=0;
	
	SWFEXT_FUNC_BEGIN(handle);

	playmode = get_video_repeat_mode();
	Swfext_PutNumber(playmode);
	
	SWFEXT_FUNC_END();
}

static int ve_set_play_mode(void * handle)
{
	int playmode=0;

	SWFEXT_FUNC_BEGIN(handle);
	
	playmode = Swfext_GetNumber();
	set_video_repeat_mode(playmode);
	
	SWFEXT_FUNC_END();
}


/**
* the following functions are added for the channel 
* selection of TS PS
*/

/**************************BEGIN****************************/


static int ve_set_program(void *handle)
{
	int rtn=0;
	unsigned char *heap1=NULL;
	long heapsize = 0;
	
	video_cmd_s cmd;
	v_set_program_s prog_set;
	SWFEXT_FUNC_BEGIN(handle);
	
	prog_set.v_id = Swfext_GetNumber();
	prog_set.v_codecid = Swfext_GetNumber();
	prog_set.a_id = Swfext_GetNumber();
	prog_set.a_codecid=Swfext_GetNumber();
	prog_set.a_onlyswitch=Swfext_GetNumber();
	printf("%s,%d:%d,%d,%d,%d,%d\n",__func__,__LINE__,prog_set.v_id,prog_set.v_codecid,prog_set.a_id, prog_set.a_codecid,prog_set.a_onlyswitch);

	if(prog_set.a_onlyswitch==0){//video channel changed, need to replay
		if(videohandle){	
			cmd.con = V_STOP;
			_video_dec_send_cmd(videohandle, &cmd);
		}
	}	
	cmd.con = V_SET_PROGRAM;
	memcpy(&cmd.par.set_prog,&prog_set,sizeof(v_set_program_s));
	_video_dec_send_cmd(videohandle,&cmd);

	if(prog_set.a_onlyswitch==0){//video channel changed, need to replay
		__videoSetFile();
		if(video_error){
			playstat=VE_ERROR;
			rtn = -1;
			goto __PLAY__OUT;
		}
		/**
		* set play parameters.
		*/
		_video_dec_send_cmd(videohandle,&videoPlayParaCmd);
		playstat = VE_PLAY;
		cmd.con = V_PLAY;
		_video_dec_send_cmd(videohandle,&cmd);
		video_error=0;
	}
	
__PLAY__OUT:
	if(rtn==-1)
		printf(VIDEOINFO_ERR_SET);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();	
}

static int ve_get_file_info_string(void *handle)
{
	char strbuf[32]="";
	int rtn=0;
	file_info_s  file_info;
	int info_tag=0;
	int pro_tag=0;
	int prg_index=0;
	int stream_tag=0;
	int stream_index=0;

	/** FIXME: [ please finish it ]*/
	
	SWFEXT_FUNC_BEGIN(handle);
	info_tag = Swfext_GetNumber();
	pro_tag = Swfext_GetNumber();
	prg_index = Swfext_GetNumber();
	stream_tag = Swfext_GetNumber();
	stream_index = Swfext_GetNumber();
	if(videohandle){
		memcpy(&file_info,&media_file_info,sizeof(file_info_s));
		if(rtn==-1){
			printf(VIDEOINFO_ERR_MSG);
			goto END;
		}
		switch(info_tag){
			case VIDEOINFO_PRO_NUM:
				sprintf(strbuf,"%d",file_info.prg_nb);
				break;
			case VIDEOINFO_PRO_INFO:
				switch(pro_tag){
					case  PRO_NAME:
						strcpy(strbuf,(file_info.prg+prg_index)->prg_name);
						break;
					case PRO_V_NUM:
						sprintf(strbuf,"%d",(file_info.prg+prg_index)->prg_v_nb);
						break;
					case PRO_A_NUM:
						sprintf(strbuf,"%d",(file_info.prg+prg_index)->prg_a_nb);
						break;
					case VIDEO_INFO:
						switch(stream_tag){
							case STREAM_ID:
								sprintf(strbuf,"%d",(file_info.prg+prg_index)->video_info[stream_index].id);
								break;
							case STREAM_VALID:
								sprintf(strbuf,"%d",(file_info.prg+prg_index)->video_info[stream_index].valid);
								break;
							case STREAM_CODECID:
								sprintf(strbuf,"%d",(file_info.prg+prg_index)->video_info[stream_index].codecid);
								break;
							case STREAM_TYPE:
								strcpy(strbuf,(file_info.prg+prg_index)->video_info[stream_index].stream_type);
								break;
						}
						break;
					case AUDIO_INFO:
						switch(stream_tag){
							case STREAM_ID:
								sprintf(strbuf,"%d",(file_info.prg+prg_index)->audio_info[stream_index].id);
								break;
							case STREAM_VALID:
								sprintf(strbuf,"%d",(file_info.prg+prg_index)->audio_info[stream_index].valid);
								break;
							case STREAM_CODECID:
								sprintf(strbuf,"%d",(file_info.prg+prg_index)->audio_info[stream_index].codecid);
								break;
							case STREAM_TYPE:
								strcpy(strbuf,(file_info.prg+prg_index)->audio_info[stream_index].stream_type);
								break;
						}
						break;
				}
				break;
			default:
				printf(VIDEOINFO_ERR_MSG);
				break;
		}
		
	}
END:
	Swfext_PutString(strbuf);
	SWFEXT_FUNC_END();			
}


static int ve_get_file_info_number(void *handle)
{
	char isprint=0;
	
	int info_tag=0;
	int pro_tag=0;
	int prg_index=0;
	int stream_tag=0;
	int stream_index=0;

	char strbuf[32]="";
	int tmpnum=0;
	int rtn=0;
	file_info_s  file_info;
	SWFEXT_FUNC_BEGIN(handle);
	info_tag = Swfext_GetNumber();
	pro_tag = Swfext_GetNumber();
	prg_index = Swfext_GetNumber();
	stream_tag = Swfext_GetNumber();
	stream_index = Swfext_GetNumber();
	if(videohandle){
		memcpy(&file_info,&media_file_info,sizeof(file_info_s));
		if(rtn==-1){
			printf(VIDEOINFO_ERR_MSG);
			goto END;
		}
		switch(info_tag){
			case VIDEOINFO_PRO_NUM:
				sprintf(strbuf,"%d",file_info.prg_nb);
				tmpnum = file_info.prg_nb;
				break;
			case VIDEOINFO_PRO_INFO:
				switch(pro_tag){
					case  PRO_NAME:
						break;
					case PRO_V_NUM:
						tmpnum = (file_info.prg+prg_index)->prg_v_nb;
						break;
					case PRO_A_NUM:
						tmpnum = (file_info.prg+prg_index)->prg_a_nb;
						break;
					case VIDEO_INFO:
						switch(stream_tag){
							case STREAM_ID:
								tmpnum = (file_info.prg+prg_index)->video_info[stream_index].id;
								break;
							case STREAM_VALID:
								tmpnum = (file_info.prg+prg_index)->video_info[stream_index].valid;
								break;
							case STREAM_CODECID:
								tmpnum = (file_info.prg+prg_index)->video_info[stream_index].codecid;
								break;
							case STREAM_TYPE:
								break;
						}
						break;
					case AUDIO_INFO:
						switch(stream_tag){
							case STREAM_ID:
								tmpnum = (file_info.prg+prg_index)->audio_info[stream_index].id;
								break;
							case STREAM_VALID:
								tmpnum = (file_info.prg+prg_index)->audio_info[stream_index].valid;
								break;
							case STREAM_CODECID:
								tmpnum = (file_info.prg+prg_index)->audio_info[stream_index].codecid;
								break;
							case STREAM_TYPE:
								break;
						}
						break;
				}
				break;
			default:
				printf(VIDEOINFO_ERR_MSG);
				break;
		}
		#if 1
		if(isprint)
		{
			int i,j;
			printf("prog_nb=%-6d\n",file_info.prg_nb);
				for(j=0;j<file_info.prg_nb;j++){
				printf("prog_name[%d]=%s    ",j,(file_info.prg+j)->prg_name);
				printf("prg_id==%-6d",(file_info.prg+j)->prg_id);
				printf("prg_v_nb=%-6d",(file_info.prg+j)->prg_v_nb);
				printf("prg_a_nb=%-6d\n",(file_info.prg+j)->prg_a_nb);
				for (i=0;i<(file_info.prg+j)->prg_v_nb;i++){
					printf("video[%d].id=%-6d",i,(file_info.prg+j)->video_info[i].id);
					printf("video[%d].valid=%-6d",i,(file_info.prg+j)->video_info[i].valid);
					printf("video[%d].codecid=%-6d",i,(file_info.prg+j)->video_info[i].codecid);
					printf("video[%d].type=%s\n",i,(file_info.prg+j)->video_info[i].stream_type);	
				}
				for (i=0;i<(file_info.prg+j)->prg_a_nb;i++){
					printf("auido[%d].id=%-6d",i,(file_info.prg+j)->audio_info[i].id);
					printf("audio[%d].valid=%-6d",i,(file_info.prg+j)->audio_info[i].valid);
					printf("audio[%d].codecid=%-6d",i,(file_info.prg+j)->audio_info[i].codecid);
					printf("audio[%d].type=%s\n",i,(file_info.prg+j)->audio_info[i].stream_type);	
				}
				printf("*************\n");
			}
			printf("%s,%d\n",__FILE__,__LINE__);
			isprint = 0;
		}
		#endif
		
	}
END:
	Swfext_PutNumber(tmpnum);
	SWFEXT_FUNC_END();
}

/**************************END****************************/


static int ve_get_play_ratio(void * handle)
{
	int ratio = 0;

	
	/** FIXME: [ please finish it ]*/
	SWFEXT_FUNC_BEGIN(handle);

	ratio = get_video_disp_ratio();
	Swfext_PutNumber(ratio);
	
	SWFEXT_FUNC_END();
}

static int ve_set_play_ratio(void * handle)
{
	int ratio = 0;
	int err = 0;

	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [ please finish it ]*/
	ratio = Swfext_GetNumber();
	err = set_video_disp_ratio(ratio);

	if((0==err)&&videohandle){
		ratio = get_video_disp_ratio();
		printf("set video play ratio!ratio=%d\n", ratio);
		videoPlayParaCmd.con = V_CHANGE_OUTPUT;
		videoPlayParaCmd.par.set_par.display_mode = ratio;
		_video_dec_send_cmd(videohandle,&videoPlayParaCmd);	
	}
	
	SWFEXT_FUNC_END();
}


void ve_exception_out()
{
	/** FIXME: [please finish exception out]*/
	
	INT32U i;
#ifdef VIDEO_PREVIEW_EN	
	_ve_sem_wait(vsem);
#endif	

	if(videohandle){
		switch(playstat){
			case VE_FF:

				goto _EXP_STOP;
				break;
				
			case VE_PAUSE:

				goto _EXP_STOP;
				break;
				
			case VE_FB:

				goto _EXP_STOP;
				break;
				
			case VE_PLAY:
				goto _EXP_STOP;
				break;
				
			case VE_STOP:
			case VE_IDLE:
			case VE_ERROR:
				break;
				
			default:
				printf("exception error state %d\n",playstat);
				break;

		}

_EXP_STOP:	
		
		playstat=VE_IDLE;
		video_error=0;
		
	}
	
#ifdef VIDEO_PREVIEW_EN
	sem_destroy(vsem);
	free(vsem);
	vsem = NULL;
	if(vsem_previewok){
		sem_destroy(vsem_previewok);
		free(vsem_previewok);
		vsem_previewok = NULL;
	}

	if(vpreview_buffer){
		SWF_Free(vpreview_buffer);
		vpreview_buffer = NULL;
	}
	for(i=0;i<PREVIEW_MAX;i++)
	{
		if(vpreview_buffer_array[i]){
			SWF_Free(vpreview_buffer_array[i]);
			vpreview_buffer_array[i]=NULL;
		}
	}
	if(prevheap){
		printf("ve_exception_out swf free ok\n");
		SWF_Free(prevheap);
		prevheap=NULL;
	}
#endif
	
}

static int ve_set_max_frame_width(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	max_frame_width = Swfext_GetNumber();
	if(max_frame_width > screen_output_data.screen_output_width )
		max_frame_width = screen_output_data.screen_output_width ;
	
	SWFEXT_FUNC_END();
}
static int ve_video_seekable(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	int ret = 0; 
	if(media_file_info.seekable){
		ret = 1;
		printf("!!!!!!!!!!!!seekable\n");
	}
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}



static int ve_put_FF_step(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	FF_step_form_SWF=Swfext_GetNumber();
	SWFEXT_FUNC_END();
}
static int ve_put_FB_step(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	FB_step_form_SWF=Swfext_GetNumber();
	SWFEXT_FUNC_END();
}
static int ve_get_FF_step(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(FF_step_form_SWF);
	SWFEXT_FUNC_END();
}
static int ve_get_FB_step(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(FB_step_form_SWF);
	SWFEXT_FUNC_END();
}




int swfext_video_register(void)
{
	SWFEXT_REGISTER("ve_Open", ve_open);
	SWFEXT_REGISTER("ve_Close", ve_close);
	SWFEXT_REGISTER("ve_Set_max_frame_width", ve_set_max_frame_width);
	SWFEXT_REGISTER("ve_Play", ve_play);
	SWFEXT_REGISTER("ve_Stop", ve_stop);
	SWFEXT_REGISTER("ve_Pause", ve_pause);
	SWFEXT_REGISTER("ve_Resume", ve_resume);
	SWFEXT_REGISTER("ve_FF", ve_fast_forward);
	SWFEXT_REGISTER("ve_FB", ve_fast_backward);
	SWFEXT_REGISTER("ve_SeekPlay", ve_seek_play);
	SWFEXT_REGISTER("ve_State", ve_get_state);
	SWFEXT_REGISTER("ve_TotalTime", ve_get_total_time);
	SWFEXT_REGISTER("ve_CurTime", ve_get_play_time);
	SWFEXT_REGISTER("ve_GetPlayRatio", ve_get_play_ratio);
	SWFEXT_REGISTER("ve_SetPlayRatio", ve_set_play_ratio);
	SWFEXT_REGISTER("ve_GetPlayMode", ve_get_play_mode);
	SWFEXT_REGISTER("ve_SetPlayMode", ve_set_play_mode);
	SWFEXT_REGISTER("ve_SetFile", ve_set_file);
	SWFEXT_REGISTER("ve_Picture", ve_attach_picture);
	SWFEXT_REGISTER("ve_DetachPicture", ve_detach_picture);
	SWFEXT_REGISTER("ve_getOneFrame", ve_get_one_frame);
	SWFEXT_REGISTER("ve_GetVideoFileInfoStr", ve_get_file_info_string);
	SWFEXT_REGISTER("ve_GetVideoFileInfoNum", ve_get_file_info_number);
	SWFEXT_REGISTER("ve_SetVideoPro", ve_set_program);
	SWFEXT_REGISTER("ve_Seekable", ve_video_seekable);
/** used for store the step form SWF to display Multiple FF icon or FB icon by chenshouhui**/
	SWFEXT_REGISTER("put_videofastbackwardstep", ve_put_FB_step);
	SWFEXT_REGISTER("put_videofastforwardstep", ve_put_FF_step);
	SWFEXT_REGISTER("get_videofastbackwardstep", ve_get_FB_step);
	SWFEXT_REGISTER("get_videofastforwardstep", ve_get_FF_step);  
/** used for store the step form SWF to display Multiple FF icon or FB icon by chenshouhui**/
	return 0;
	
}
#else
int swfext_video_register(void)
{
	return 0;
}
#endif/*MODULE_CONFIG_VIDEO*/
