#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <semaphore.h>
#include <errno.h>
#include <pthread.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <linux/string.h>

#include "sys_buf.h"
#include "display.h"
#include "sys_cfg.h"
#include "usb_subdisp.h"

#define DEBUG_SUBDISP

#ifdef  DEBUG_SUBDISP
#define DBG(fmt,stuff...)   printf(fmt,##stuff)
#else
#define DBG(fmt,stuff...)   do {} while (0)
#endif


#define NUM_BUFFERS		2
enum buffer_state {
	BUF_STATE_EMPTY = 0,
	BUF_STATE_FULL,
	BUF_STATE_BUSY,
};

struct buf_head{
	pthread_mutex_t  lock;
	struct{
		unsigned long phy_buf;
		unsigned long size;
	}head;
	unsigned long logic_buf;
	enum buffer_state state;
	struct buf_head *next;
};


struct global_info{
	/*we used multibuffer */
	struct buf_head bh[NUM_BUFFERS];
	struct buf_head *next_bufhd_to_fill;
	struct buf_head *next_bufhd_to_drain;
	
	/*handle for device subdisp*/
	int 		fd_subdisp;
	/*sem to indicate buf resource*/
	sem_t  sem_product;
	sem_t  sem_consume;
	
	/*thread sync sem*/
	sem_t sem_dec;
	sem_t sem_stream;
	
	int 	stream_thread_exit:1,
		dec_thread_exit:1,
		subdisp_disconn:1,
		subdisp_switch:1;
};

struct global_info g_info;
const char subdisp_name[] = "/dev/usb_subdisp";
const char sysbuf_name[]   = "/dev/sysbuf";

static int init_subdisp(int width,
int height,
int chip_id){
	int result;
	struct dev_info pinfo;
	int fd = open(subdisp_name,O_RDWR);
	if(fd < 0)
		return -1;

	/*set devinfo*/
	memset(&pinfo,0,sizeof(struct dev_info));
	pinfo.width		= width;
	pinfo.height		= height;
	pinfo.chip_id	= chip_id;
	
	result = subdisp_set_devinfo(fd,&pinfo);
	if(result < 0){
		DBG("set devinfo failed\n",result);	
		return -1;
	}
	
	DBG("set devinfo ok\n");
	return fd;
}


/*------------------------------------------------------------------------------*/
static void * stream_thread_proc(void *param)
{
	struct global_info *g_info = (struct global_info *)param;
	struct buf_head *bh;
	enum errtype err;
	int rc;
	
	sem_wait(&g_info->sem_stream);
	DBG("stream thread run\n");
	
	while(!g_info->stream_thread_exit){
		/*wait buffer usefull*/
		bh = g_info->next_bufhd_to_fill;	
		rc = sem_wait(&g_info->sem_product);
		if (rc) 
			return NULL;
		
		/*check if exceptions happen*/			
		if(g_info->subdisp_disconn 
			||g_info->subdisp_switch){
			DBG("exceptions happend\n");
			break;
		}
		
		/*now we can requet image data*/
		pthread_mutex_lock(&bh->lock);
		if(bh->state != BUF_STATE_EMPTY)
			DBG("buf state odd\n");	
		bh->state = BUF_STATE_BUSY;
		pthread_mutex_unlock(&bh->lock);
		
		rc = subdisp_get_image(g_info->fd_subdisp,
			(void *)&bh->head);		
		if(rc<0){
			rc = subdisp_get_lasterr(g_info->fd_subdisp,
				&err);
			if(rc!=0)
				DBG("get last err failed\n");
			
			if(err != ERR_NONE){
				DBG("err happend during get image,err:%d\n",err);
				/*know which exception happend*/	
			}else{			
				pthread_mutex_lock(&bh->lock);
				bh->state = BUF_STATE_EMPTY;
				pthread_mutex_unlock(&bh->lock);
				continue;
			}
		}
		
		g_info->next_bufhd_to_fill = bh->next;
		pthread_mutex_lock(&bh->lock);
		bh->state = BUF_STATE_FULL;
		pthread_mutex_unlock(&bh->lock);
		sem_post(&g_info->sem_consume);		
	}
	pthread_exit(NULL);
}

static inline int transfer_pix_fmt(int  rawfmt)
{
	if(rawfmt == OUT_PIX_FMT_YUV422 )
		return  DE_PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
	else
		return  DE_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR;
}


static void dump_pic_info(struct pic_fmt *fmt)
{
	DBG("===================================\n");
	switch (fmt->format)
	{
	case OUT_PIX_FMT_JPEG:
		DBG("fmt:out_jpeg\n");break;
	case OUT_PIX_FMT_YUV420:
		DBG("fmt:out yuv420\n");break;
	case OUT_PIX_FMT_YUV422:
		DBG("fmt:out yuv422\n");break;
	default:
		DBG("invalid fmt:%d\n",fmt->format);
	}

	DBG("width:%d\n",fmt->width);
	DBG("height:%d\n",fmt->height);
	DBG("size:%d\n",fmt->isize);
}


static void * decode_thread_proc(void *param)
{
	struct global_info *g_info = (struct global_info *)param;
	struct buf_head *bh;
	struct pic_fmt* picfmt;
	void *phy_addr;
	unsigned long size;
	int rc;
	void *deinst;
	DE_config ds_conf;
	static int cnt = 0;
	int tmp;
	char filename[20];
	
	DBG("decode thread start\n");
	de_init(&deinst);
	
	sem_wait(&g_info->sem_dec);
	DBG("decode thread ready to run\n");
	
	while(!g_info->dec_thread_exit){
		/*wait buffer usefull*/		
		bh = g_info->next_bufhd_to_drain;
		rc = sem_wait(&g_info->sem_consume);
		if (rc) 
			return NULL;

		/*check if exception happen*/
		if(g_info->subdisp_disconn
			||g_info->subdisp_switch){
			DBG("exceptions happend\n");
			break;
		}
		
		/*judge data we received */
		pthread_mutex_lock(&bh->lock);
		if(bh->state != BUF_STATE_FULL)
			DBG("dec buf state odd\n");
		bh->state = BUF_STATE_BUSY;
		pthread_mutex_unlock(&bh->lock);
		
		picfmt = (struct pic_fmt *)bh->logic_buf;
		size  = sizeof(struct pic_fmt);	
		dump_pic_info(picfmt);	
		switch(picfmt->format){
		case OUT_PIX_FMT_JPEG:
			printf("prepare to dec mjpeg frame");
			/*fix me : need mjpeg decode api*/
			break;
		case OUT_PIX_FMT_YUV422:
		case OUT_PIX_FMT_YUV420:
			/*raw yuv data*/
			phy_addr = (void *)bh->head.phy_buf;
			de_get_config(deinst,&ds_conf,DE_CFG_ALL);
			ds_conf.input.width = picfmt->width;
			ds_conf.input.height = picfmt->height;
			ds_conf.input.pix_fmt = transfer_pix_fmt(picfmt->format);
			ds_conf.input.img =(void *)(bh->logic_buf + size);
			ds_conf.input.bus_addr = (void *)(bh->head.phy_buf + size);
			ds_conf.input.enable = 1;
			de_set_Config(deinst,&ds_conf,DE_CFG_ALL);
			break;
		default:
			printf("invalid data fmt\n");
			break;
		}
		
		g_info->next_bufhd_to_drain = bh->next;			
		pthread_mutex_lock(&bh->lock);
		bh->state = BUF_STATE_EMPTY;
		pthread_mutex_unlock(&bh->lock);

		sem_post(&g_info->sem_product);		
	}	
	pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
	int parent_msg_id;
	struct pollfd fd;
	int rpipe;
	int flag;

	int i;
	int fd_sysbuf,fd_subdisp;
	int ret;
	struct mem_dev g_buf;
	pthread_t stream_thread,decode_thread;
	struct buf_head *pbuf;
	
	/*get 3M for stream buffer*/
	fd_sysbuf = open(sysbuf_name,O_RDWR);
	if(fd_sysbuf < 0)
		return -1;

	g_buf.request_size = (unsigned long )3<<20;
    	g_buf.buf_attr = UNCACHE;
	if(ioctl(fd_sysbuf,MEM_GET,&g_buf) < 0){
		printf("get sysbuffer failed\n");
		close(fd_sysbuf);
		return -1;
	}
	
	DBG("phy_buf:%x,logic_buf:%x\n",
		g_buf.physic_address,
		g_buf.logic_address);

	/*init multi buffer */
	memset(g_info.bh,0,sizeof(g_info.bh));
	for(i=0;i<NUM_BUFFERS;i++){
		pbuf = &g_info.bh[i];
		pbuf->next = pbuf + 1;
		pbuf->head.size = g_buf.request_size/NUM_BUFFERS;
		pbuf->head.phy_buf = g_buf.physic_address 
			+ i*pbuf->head.size;
		pbuf->logic_buf = g_buf.logic_address
			+ i*pbuf->head.size;
		pbuf->state = BUF_STATE_EMPTY;
		
		/*init mutex*/
		ret = pthread_mutex_init(&pbuf->lock,NULL);
		if(ret < 0){
			DBG("mutex create failed \n");
			goto err;
		}
	} 
	
	g_info.bh[NUM_BUFFERS-1].next = &g_info.bh[0];
	g_info.next_bufhd_to_fill = g_info.next_bufhd_to_drain = &g_info.bh[0];
	g_info.subdisp_disconn = g_info.subdisp_switch = 0;
	g_info.dec_thread_exit = g_info.stream_thread_exit= 0;
	
	ret = sem_init(&g_info.sem_product, 0, NUM_BUFFERS);
	if(ret < 0)
		DBG("create stream sem failed:%d",ret);

	ret = sem_init(&g_info.sem_consume, 0, 0);
	if(ret < 0)
		DBG("create stream sem failed:%d",ret);

	ret = pthread_create (&stream_thread,
			NULL,
			stream_thread_proc,
			&g_info);	
	if(ret < 0)
		DBG("create stream thread failed:%d",ret);
		
	ret = pthread_create (&decode_thread,
			NULL,
			decode_thread_proc,
			&g_info);	
	if(ret < 0)
		DBG("create decode thread failed:%d",ret);

	fd_subdisp = init_subdisp(800,600,0x7555);
	if(fd_subdisp < 0)	{
		DBG("init subdisp failed:%d",fd_subdisp);
		return -1;
	}	
	g_info.fd_subdisp = fd_subdisp;

	DBG("init subdisp ok:%d",fd_subdisp);
	/*let task run*/
	sem_post(&g_info.sem_dec);
	sem_post(&g_info.sem_stream);
	
	/*poll msg for quit or switch*/
	while(1){
		;
		/*if get message quit or disconnect,need 
			pipe to get msg from manager*/
	}

	ret = pthread_join(stream_thread, NULL);
	if(ret)
		DBG("stream thread join failed\n");
	
	ret = pthread_join(decode_thread,NULL);
	if(ret)
		DBG("decode thread join failed\n");
	
	sem_destroy(&g_info.sem_dec);
	sem_destroy(&g_info.sem_stream);
	
	for(i=0;i<NUM_BUFFERS;i++){
		pbuf = &g_info.bh[i];
		pthread_mutex_destroy(&pbuf->lock);	
	}
	
	ret = 0;	
err1:
	close(fd_subdisp);
err:
	ioctl(fd_sysbuf,MEM_PUT,&g_buf);
	close(fd_sysbuf);	
	return ret;
}





