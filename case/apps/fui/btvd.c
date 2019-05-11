#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include "btvd_api.h"
#include "swfdec.h"
#include "display.h"
//#include "sys_sn7328.h"
#include "sys_pmu.h"
#include "sys_cfg.h"
#include "sys_conf.h"
#include "hantro_misc.h"
#include "am7x_dac.h"


#define act_writel(val,reg)  (*(volatile int *)(reg) = (val))                
#define act_readl(port)  (*(volatile unsigned int *)(port))       

#define SIGNAL_GPIO 41
#define AVIN_DET	30
#define NTSC_HEIGHT	480

#define SUB_HEIGHT	0

#define PAL_RESIZE_W 		352//360
#define PAL_RESIZE_H		280//288

#define NTSC_RESIZE_W	352//360
#define NTSC_RESIZE_H	234//240

#define PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED				0x010001U

#if 1
#define btvd_info(fmt,arg...) 	printf("BTVD[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define btvd_err(fmt,arg...) 		printf("BTVD[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#else
#define btvd_info(fmt,arg...) do{}while(0);
#define btvd_err(fmt,arg...) do{}while(0);
#endif

static DE_config btvd_de_env_saved;
static char env_saved_ok = 0;
//static pthread_t btvd_thread_id = 0;

//static volatile char btvd_exit = 0; 

//extern int device_in;
BTVD_INFO info;


enum BTVD_SWF_CMD{
	BTVD_SWF_STOP = 0,
	BTVD_SWF_RUN
};

enum STATE{
	BTVD_STOP = 0,
	BTVD_RUN
};

enum {
	NOT = 0,
	AVIN,
	EAR_PHONE,
	SCAN_AVIN
};


volatile char check_avin = 0;
static struct __btvd_work_data{
	pthread_t display_thread_id;
	pthread_t ctl_thread_id;
	volatile char exit;
	volatile char swf_cmd;
	unsigned char signal;
	char state;
	sem_t sem;
}btvd_ctl;

static pthread_t sw_thread_id = 0;
static volatile char in = NOT;
char avin = 0;

#if 0
static int adc_base_addr = 0;
static int dac_base_addr = 0;

static unsigned int adc_ctl = 0;
static unsigned int dac_an1 = 0;
static unsigned int dac_an2 = 0;
#endif


static int volume_map[_MAX_VOLUME_LEVEL] = 
{
	_VOLUME_LEVEL_0,
	_VOLUME_LEVEL_1,
	_VOLUME_LEVEL_2,
	_VOLUME_LEVEL_3,
	_VOLUME_LEVEL_4,
	_VOLUME_LEVEL_5,
	_VOLUME_LEVEL_6,
	_VOLUME_LEVEL_7,
	_VOLUME_LEVEL_8,
	_VOLUME_LEVEL_9,
	_VOLUME_LEVEL_10
};


extern int get_subdisplay_state(void);

enum SUBDIS_STATE{
	SUBDIS_STOP = 0,
	SUBDIS_RUN
};
extern char is_in_stby;
//extern char screen_flag;


static int _btvd_sem_wait(sem_t * sem)
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
			return err;
		}
	}

	return err;
	
}

static int _btvd_sem_post(sem_t * sem)
{
	int err;
	err = sem_post(sem);
	return err;
}

static void RegBitSet(int val,int reg,int msb,int lsb)                                            
{                                             
	unsigned int mask = 0xFFFFFFFF;
	unsigned int old  = act_readl(reg);

	mask = mask << (31 - msb) >> (31 - msb + lsb) << lsb;
	act_writel((old & ~mask) | ((val << lsb) & mask), reg);	         
}                                                                                                                                                                                                                                                                               
static unsigned int RegBitRead(int reg,int msb,int lsb)
{                                                     
	unsigned int valr;
	
	valr=act_readl(reg);                                                                       
	return (valr<<(31-msb)>>(31-msb+lsb));                                                     
}				






static void __init_btvd_info(BTVD_INFO * pbtvd_info, int format)
{
	int height = 0;
	pbtvd_info->video_format = format;

	//btvd_ctl.exit = 0;
	btvd_ctl.state = BTVD_RUN;
	printf("%s %d format=%d\n",__FILE__,__LINE__,format);
	if(BT656_576I == info.video_format){
		height = 576;
		pbtvd_info->out_buf.logic_buf = (int)SWF_Malloc(PAL_RESIZE_H*PAL_RESIZE_W*2);
		
	}else{
		height = NTSC_HEIGHT;
		pbtvd_info->out_buf.logic_buf = (int)SWF_Malloc(NTSC_RESIZE_H*NTSC_RESIZE_W*2);
	}

	if(pbtvd_info->out_buf.logic_buf != 0){
		pbtvd_info->out_buf.phy_buf = (int)SWF_MemVir2Phy((void*)(pbtvd_info->out_buf.logic_buf));
	}else{
		btvd_info("malloc fail!!\n");
	}

	
	
#if 0
	pbtvd_info->buf1.logic_buf = (int)SWF_Malloc(720*height*2);
	pbtvd_info->buf1.phy_buf = (int)SWF_MemVir2Phy((void*)(pbtvd_info->buf1.logic_buf));
	pbtvd_info->buf2.logic_buf = (int)SWF_Malloc(720*height*2);
	pbtvd_info->buf2.phy_buf = (int)SWF_MemVir2Phy((void*)(pbtvd_info->buf2.logic_buf));
	

	btvd_info("buf1.phy_buf   == 0x%x\n", pbtvd_info->buf1.phy_buf);
	btvd_info("buf2.phy_buf   == 0x%x\n", pbtvd_info->buf2.phy_buf);
#endif 
}

static void __de_btvd_info(BTVD_INFO * pbtvd_info)
{
	btvd_ctl.exit = 0;
	btvd_ctl.state = BTVD_STOP;

	if(pbtvd_info->out_buf.logic_buf !=0){
		SWF_Free((void*)(pbtvd_info->out_buf.logic_buf));
		pbtvd_info->out_buf.logic_buf = 0;
	}
/*	SWF_Free((void*)(pbtvd_info->buf1.logic_buf));
	SWF_Free((void*)(pbtvd_info->buf2.logic_buf));


	pbtvd_info->buf1.logic_buf = 0;
	pbtvd_info->buf2.logic_buf = 0;*/
}




static int _btvd_save_env()
{
	void *deinst;
	de_init(&deinst);
	memset(&btvd_de_env_saved,0,sizeof(DE_config));
	if(de_get_config(deinst,&btvd_de_env_saved,DE_CFG_ALL)!=DE_OK){
		env_saved_ok =0;
		return -1;
	}
	else{
		env_saved_ok = 1;
		return 0;
	}
}


static void  _btvd_restore_env()
{
	void *deinst;
	de_init(&deinst);
	if(env_saved_ok)
		de_set_Config(deinst,&btvd_de_env_saved,DE_CFG_ALL);

#if 1
	int count=40;
	unsigned int vir_add,offset,vir_ori,size;
	int fd_mem;
	fd_mem = open("/dev/mem", O_RDWR | O_SYNC);	
	if(fd_mem<0)
		printf("open/dev/mem error\n");
	offset=0;
	size=offset+count*4;
	vir_ori=(int)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, 0xb0040000&0x1ffff000);	
	vir_add=vir_ori+offset;
	
	//RegBitSet(2,vir_add,8,7);//0:rgb;1:cpu;2:bt
	if (RegBitRead(vir_add,0,0) ==0)
		RegBitSet(1,vir_add,0,0);//1:interlace;0:progressive;
	else
		printf("@@@@@@@@@@interlace\n");
	munmap((void*)vir_ori,size);	
	close(fd_mem);
#endif
}
static int _btvd_switchto_btvd()
{
	int ret = 0;
	
	//_btvd_save_env();
	
	if(SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE){
		/** first set swf to sleep, and buffer will be released */
		SWF_SetState(SWF_GetActiveInst(), SWF_STATE_SLEEP);
	}
	
	return ret;
}

int _btvd_backfrom_btvd()
{
	int ret = 0;

	_btvd_restore_env();
	
	if((SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE) == 0){
		//btvd_info("SET SWF_STATE_ACTIVE \n");
			SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
	}
	
	return ret;
}

#if 0
static int __write_sn7328_reg(struct sn7328_data *pdata, char lmov, char flag)
{
	
	int fd = 0;
	int ret = 0;
	
	fd = open("/dev/sn7328", O_RDWR | O_NONBLOCK);
	if(fd < 0){
		printf("open /dev/sn7328 err \n");
		return -1;
	}
	

	pdata->val = 0;
	ret = ioctl(fd, READ_REG, pdata);

	if(0 == flag){
		pdata->val &= ~(1<<lmov);
	}else{
		pdata->val |= (1<<lmov);
	}
	
	ret = ioctl(fd, WRITE_REG, pdata);

	btvd_info("sn7328 reg 0x%x == 0x%x", pdata->reg, pdata->val);
	close(fd);

	return ret;
}
#endif

enum {
	TVP5150_STBY = 0,
	TVP5150_RUN
};

static char tvp5150_state = TVP5150_STBY;

int set_tvp5150_power (char  onoff)
{
	int fd =0;
	int err = 0;
	
#if 1//P30PLUS == 1


	struct btvd_data data;
	
	fd = open("/dev/btvd", O_RDWR);
	if(fd < 0)
		printf("open/dev/btvd error\n");

	data.reg = 0x02;
	data.val = 0x00;
	err = ioctl(fd,BTVD_IO_R_REG, &data);

	if(1 == onoff){
		data.val &= ~0x01;
	}else{
		data.val |= 0x01;
	}


	if(0 == onoff){
		tvp5150_state = TVP5150_STBY;
	}else{
		tvp5150_state = TVP5150_RUN;
	}

	
	err = ioctl(fd,BTVD_IO_W_REG, &data);

	close(fd);
#else
	btvd_info("this is no P30+ please check sys_cfg.h\n ");
#endif
	
	return err;
}

//  1 is to stby   0 is out stby


inline  int __set_tvp5150_stby(char flag)
{
	int ret = 0;
/*	struct sn7328_data data;
	
	data.reg = output_port_b_reg;
	ret = __write_sn7328_reg(&data, 7, !flag);*/

	
	ret = set_tvp5150_power(!flag);
	return ret;
}


#if 0
//flag - 0 : not earphone
int __audio_sw(char flag)
{
	int ret = 0;
	struct sn7328_data data;
	
	data.reg = output_port_b_reg;
	ret = __write_sn7328_reg(&data, 4, !flag);// AMP_EN  sn7328 pp0
	//usleep(300*1000);
	//ret = __write_sn7328_reg(&data, 6, flag);// audio_sw sn7328 pp2

#if 0
#ifdef P30_V03
	data.reg = output_port_a_reg;
	ret = __write_sn7328_reg(&data, 7, !flag);//AMP_EN  sn7328 od3
#endif
#endif

	return ret;
}

//0-avin 1-earphone
int __audio_sw_avin(char flag)
{
	int ret = 0;
	struct sn7328_data data;
	
	data.reg = output_port_b_reg;
	ret = __write_sn7328_reg(&data, 6, flag);// audio_sw sn7328 pp2

	return ret;
}

static inline void __switch_to_av_audio()
{
	
	adc_ctl = RegBitRead(adc_base_addr+0x00, 31, 0);
	dac_an1 = RegBitRead(dac_base_addr+0x0c, 31, 0);
	dac_an2 = RegBitRead(dac_base_addr+0x28, 31, 0);
	
//	btvd_info("111reg an1 == 0x%x\n", RegBitRead(dac_base_addr+0x0c, 31, 0));
	
	RegBitSet(3, adc_base_addr+0x00, 8, 7);//FM enable
	RegBitSet(1, adc_base_addr+0x00, 13, 12);//ADC input select

	RegBitSet(2, dac_base_addr+0x0c, 9, 8);
	RegBitSet(3, dac_base_addr+0x0c, 1, 0);
	
	RegBitSet(1, dac_base_addr+0x28, 8, 8);
	
	RegBitSet(volume_map[sys_get_volume()], dac_base_addr+0x0c, 7, 2);
}


static inline void __switch_from_av_audio()
{
	
	#if 0
	RegBitSet((adc_ctl&0x00000180)>>7, adc_base_addr+0x00, 8, 7);//FM enable
	RegBitSet((adc_ctl&0x00003000)>>12, adc_base_addr+0x00, 13, 12);//ADC input select

	RegBitSet((dac_an1&0x00000700)>>8, dac_base_addr+0x0c, 10, 8);
	RegBitSet((dac_an1&0x00000003), dac_base_addr+0x0c, 1, 0);
	
	RegBitSet((dac_an2&0x00000100)>>8, dac_base_addr+0x28, 8, 8);
	
	#else
	int tmp = 0;
	
	tmp = RegBitRead(dac_base_addr+0x0c, 10, 2);
	
	RegBitSet(adc_ctl, adc_base_addr+0x00, 31, 0);

	dac_an1 = (dac_an1 & 0x0fc) | 0x3; //|(tmp<<2) 
	RegBitSet(dac_an1, dac_base_addr+0x0c, 31, 0);
	
	RegBitSet(dac_an2, dac_base_addr+0x28, 31, 0);
	#endif

}
#endif
extern void ae_paude_c();
static void*  __display_thread_proc(void * arg)
{
	int fd = 0;
	int err = 0; 
	int height;
	DE_config ds_conf;
	void *deinst = NULL;
	int format = *(int *)arg;
	struct btvd_data data;
	int out_w = 0;
	int out_h = 0;
	char btvd_stat = 0;
	int i = 0;
	char ratio = 0;
	
	btvd_info("av display thread start......\n");
	
	avin = 1;
	__init_btvd_info(&info,  format);
	
	
	if(BT656_576I == format){
		height = 576;
		out_w = PAL_RESIZE_W;
		out_h = PAL_RESIZE_H;
	}else{
		height = NTSC_HEIGHT;
		out_w = NTSC_RESIZE_W;
		out_h = NTSC_RESIZE_H;
	}
	_btvd_save_env();
	
	de_init(&deinst);
	de_get_config(deinst,&ds_conf, DE_CFG_ALL);
	
#if 	 1  //no PP
	ds_conf.input.width = 720;
	ds_conf.input.height = height-SUB_HEIGHT;
	ds_conf.input.pix_fmt = PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
	ds_conf.input.enable = 1;
	ds_conf.output.display_mode = DE_DISPLAYMODE_FULL_SCREEN;

	fd = open("/dev/btvd", O_RDWR);
	if(fd < 0)
		printf("open/dev/btvd error\n");
	

	ds_conf.input.img = (unsigned long *)(info.buf1.logic_buf + (SUB_HEIGHT * ds_conf.input.width *2));
	ds_conf.input.bus_addr = info.buf1.phy_buf + (SUB_HEIGHT * ds_conf.input.width *2);	
	printf("ds_conf.input.img=0x%x ds_conf.input.bus_addr=0x%x\n",ds_conf.input.img,ds_conf.input.bus_addr);
	_btvd_switchto_btvd();
	OSSleep(3000);
	de_set_Config(deinst,&ds_conf,DE_CFG_ALL);
	
#if 1
	int count=64;
	unsigned int vir_add,offset,vir_ori,size;
	int fd_mem;
	fd_mem = open("/dev/mem", O_RDWR | O_SYNC);	
	if(fd_mem<0)
		printf("open/dev/mem error\n");
	offset=0;
	size=offset+count*4;
	vir_ori=(int)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, 0xb0040000&0x1ffff000);	
	vir_add=vir_ori+offset;
	
	//RegBitSet(2,vir_add,8,7);//0:rgb;1:cpu;2:bt
	if (RegBitRead(vir_add,0,0) ==1)
	{
		RegBitSet(0,vir_add,0,0);//1:interlace;0:progressive;
		RegBitSet(1,(vir_ori+0xe4),0,0);//DB_WR
	}
	else
		printf("@@@@@@@@@@progressive\n");
	if (RegBitRead(vir_ori+0x10,31,0) != ds_conf.input.bus_addr)
	{
		printf("error frm_BA\n");
		RegBitSet(ds_conf.input.bus_addr,vir_ori+0x10,31,0);//frm_BA
		RegBitSet(1,(vir_ori+0xe4),0,0);//DB_WR
	}	
	munmap((void*)vir_ori,size);	
	close(fd_mem);
#endif	
	
	//__switch_to_av_audio();
	ioctl(fd,BTVD_IOCSOPEN,&info);	
	
	while(1){
		if(1 ==  btvd_ctl.exit){
			goto btvd_exit;
		}
		usleep(50000);
	}
#else
	ratio = get_photo_disp_ratio();

	printf("ratio == %d\n", ratio);
	if(0 == ratio){
		ds_conf.output.display_mode = DE_DISPLAYMODE_LETTER_BOX;//DE_DISPLAYMODE_FULL_SCREEN;//DE_DISPLAYMODE_ACTUAL_SIZE;
	}else if(1 == ratio){
		ds_conf.output.display_mode = DE_DISPLAYMODE_LETTER_BOX;
	}else{
		ds_conf.output.display_mode = DE_DISPLAYMODE_FULL_SCREEN;
	}
	
	ds_conf.input.width = out_w;
	ds_conf.input.height = out_h  - SUB_HEIGHT; 
	ds_conf.input.pix_fmt = PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
	ds_conf.input.enable = 1;
	ds_conf.input.img = (unsigned long *)(info.out_buf.logic_buf + (SUB_HEIGHT * ds_conf.input.width *2));
	ds_conf.input.bus_addr = info.out_buf.phy_buf + (SUB_HEIGHT * ds_conf.input.width *2);
	
	fd = open("/dev/btvd", O_RDWR);
	if(fd < 0)
		printf("open/dev/btvd error\n");

	_btvd_switchto_btvd();
	
	
	btvd_stat = 1;

	info.buf1.phy_buf &= 0x1FFFFFFF;
	info.out_buf.phy_buf &= 0x1FFFFFFF;
	
//	btvd_info("info.buf1.logic_buf == 0x%x,  info.buf1.phy_buf == 0x%x, height== %d\n", info.buf1.logic_buf, info.buf1.phy_buf, height);
//	btvd_info("info.out_buf.logic_buf == 0x%x,  info.out_buf.phy_buf == 0x%x,w ==%d, height== %d\n", info.out_buf.logic_buf, info.out_buf.phy_buf, out_w, out_h);

	__audio_sw_avin(0);
	__switch_to_av_audio();
	ioctl(fd, BTVD_IOCSOPEN,&info);
	while(1){
		if(1 ==  btvd_ctl.exit || 1 == device_in){
			goto btvd_exit;
		}
		//usleep(500 *1000);
		#if 1
		if(1 == is_in_stby && 1 == btvd_stat){  //close btvd
			ioctl(fd, BTVD_IOCTCLOSE,&info);
			btvd_info("close btvd\n");
			btvd_stat = 0;
		}else if(0 == is_in_stby && 0 == btvd_stat){
			btvd_info("open btvd\n");
			ioctl(fd, BTVD_IOCSOPEN,&info);
			btvd_stat = 1;
		}
		
		
		if(1 == btvd_stat){
			ioctl(fd,BTVD_GET_IMG, &info);
			if( BUF_FULL == info.buf1.state){
				err = hm_func.pp_blit((void *) info.buf1.logic_buf,  info.buf1.phy_buf, 0, 0, 0, 720, height/2, PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED, \
				info.out_buf.logic_buf, info.out_buf.phy_buf, out_w, 0, 0, out_w, out_h, PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED);
			}else if( BUF_FULL == info.buf2.state){
				err = hm_func.pp_blit((void *) info.buf2.logic_buf,  info.buf2.phy_buf, 0, 0, 0, 720, height/2, PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED, \
				info.out_buf.logic_buf, info.out_buf.phy_buf, out_w, 0, 0, out_w, out_h, PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED);
			}else{
				btvd_err("get img err!!");
			}

			if(err < 0){
				btvd_info("pp resize err ==  %d\n", err);
			}

			if(1 ==  btvd_ctl.exit || 1 == device_in){
				goto btvd_exit;
			}
			de_set_Config(deinst,&ds_conf,DE_CFG_ALL);
			
		//	btvd_info("addr == %p\n",  &__display_thread_proc);
		}else{
			usleep(10000);
		}
		#endif

	}
#endif
	
btvd_exit:
	
	
	ioctl(fd,BTVD_IOCTCLOSE,&info);	
	de_release(deinst);
	close(fd);
	
	//__switch_from_av_audio();
	//__audio_sw_avin(1);
	_btvd_backfrom_btvd();

	__de_btvd_info(&info);
	
	//if(0 == device_in)
	{
		SWF_Message(NULL, SWF_MSG_KEY_F1, NULL);
		SWF_Message(NULL, SWF_MSG_PLAY, NULL);
	}

	avin = 0;
	btvd_info("av display thread end!!\n");
	pthread_exit(NULL);

	return NULL;
}



static void __stop_disply_thread()
{
	int fd = 0;
	struct btvd_data data;

//	__switch_from_av_audio();
	if(BTVD_RUN != btvd_ctl.state){
		btvd_info("Avin display is no run!!");
		return;
	}
	
	_btvd_sem_wait(&btvd_ctl.sem);
	
	if(btvd_ctl.display_thread_id == -1){
		_btvd_sem_post(&btvd_ctl.sem);
		return ;
	}
	
	//__audio_sw(1);
	fd = open("/dev/btvd", O_RDWR);
	if(fd < 0)
		printf("open/dev/btvd error\n");

	btvd_ctl.exit = 1;
	pthread_join(btvd_ctl.display_thread_id, NULL);
	btvd_ctl.display_thread_id = -1;
	
	
	data.reg = 0x28;
	data.val = 0x00;
	ioctl(fd,BTVD_IO_W_REG, &data);
	
	close(fd);
	
	//__audio_sw(0);
	_btvd_sem_post(&btvd_ctl.sem);
	
}

static int __check_avin_signal()
{
	unsigned char signal = 0;
	char cnt = 0;
	int fd = 0;
	int err = 0;
	int i = 10;
	struct btvd_data data;
	
	fd = open("/dev/btvd", O_RDWR);
	if(fd < 0)
		btvd_err("open/dev/btvd error\n");

	while(i--){
		data.reg = 0xc0;
		data.val = 0x00;
		err = ioctl(fd,BTVD_IO_R_REG, &data);
				
		if(err < 0){
			btvd_info("read tvp5150 err\n");
			close(fd);
			return 0;
		}
		
		signal = (0x80 & data.val)>>7;
		if(1 == signal){
			cnt++;
		}
	}

	if(cnt > 5){
		signal = 1;
	}else{
		signal = 0;
	}
	
	close(fd);
	return signal;
}





//extern int __onoff_screen(int mode);
//extern int out_stby_mode(void);

#define TEST_LIB_DIR	"/mnt/udisk"
static int  __intall_btvd()
{
	char cmd[100];
	int err = 0;
	//struct sn7328_data data;

#if 0
	//reset tvp5150
	data.reg = output_port_a_reg;
	__write_sn7328_reg(&data, 4, 0);
	usleep(25*1000);
	data.reg = output_port_a_reg;
	__write_sn7328_reg(&data, 4, 1);
	usleep(300);
#endif
	printf("%s %d\n",__FILE__,__LINE__);
	sprintf(cmd,"%s%s/%s","insmod  ", AM_SYS_LIB_DIR, "btvd_i2c.ko");
	err = system(cmd);
	printf("%s %d\n",__FILE__,__LINE__);
	sprintf(cmd,"%s%s/%s","insmod  ", AM_SYS_LIB_DIR, "am7x_btvd.ko");
	err = system(cmd);
	printf("%s %d\n",__FILE__,__LINE__);
	sprintf(cmd,"%s","mknod /dev/btvd c 241 0");
	err = system(cmd);
	printf("%s %d\n",__FILE__,__LINE__);

	return err;
}

static void*  __ctl_thread_proc(void * arg)
{
	int fd = 0;
	struct btvd_data data;
	int format = 0;
	unsigned char reg = 0;
	char ival = 0;
	int err = 0;
	int i = 0;
	int  subdis_state = SUBDIS_STOP;
	static int nosignl_cnt = 0;
	
#if 0
	char cmd[100];
	
	sprintf(cmd,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"btvd_i2c.ko");
	err = system(cmd);
	sprintf(cmd,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"am7x_btvd.ko");
	err = system(cmd);
	sprintf(cmd,"%s","mknod /dev/btvd c 241 0");
	err = system(cmd);
#else
	printf("%s %d\n",__FILE__,__LINE__);
	if(__intall_btvd() >= 0){
		info.buf1.logic_buf = (int)SWF_Malloc(720*576*2);
		info.buf1.phy_buf = (int)SWF_MemVir2Phy((void*)(info.buf1.logic_buf));
		info.buf2.logic_buf = (int)SWF_Malloc(720*576*2);
		info.buf2.phy_buf = (int)SWF_MemVir2Phy((void*)(info.buf2.logic_buf));

		btvd_info("malloc mem for btvd!!");
	}else{
		btvd_info("Intall btvd driver is not ok!! Please check!Now  ctl thread out!");
		goto CTL_OUT;
	}
#endif
	
	sleep(12);
	
	btvd_info("av in ctl thread start\n");

	fd = open("/dev/btvd", O_RDWR);
	if(fd < 0)
		printf("open/dev/btvd error\n");

	
	while(1){
		/*extern char upgrade_now;
		if(1 == upgrade_now){
			goto CLOSE_OUT;
		}*/
		
		/*if(btvd_ctl.swf_cmd != BTVD_SWF_RUN ){
			goto SLEEP;
		}*/
				

		/*if(TVP5150_STBY ==  tvp5150_state){
			btvd_info("please check code ,   5150 is stby!!");
			sleep(5);
			goto SLEEP;
		}*/
		
		//subdis_state = get_subdisplay_state();
		
		/*if((SUBDIS_RUN == subdis_state)){
			if((BTVD_RUN == btvd_ctl.state)){
				__stop_disply_thread();
			}
		}else*/
		{
CHECK_SINGNL:
		
			i = 10;
			do{
				data.reg = 0xc0;
				data.val = 0x00;
				err = ioctl(fd,BTVD_IO_R_REG, &data);
				
				if(err < 0){
					btvd_info("read tvp5150 err\n");
					sleep(1);
					goto SLEEP;
				}
				
				btvd_ctl.signal = (0x80 & data.val)>>7;
				if(1 == btvd_ctl.signal){
					break;
				}
				
				usleep(6);
			}while(--i);
			
			
			if((0 == btvd_ctl.signal) && (BTVD_RUN == btvd_ctl.state)){
				if(nosignl_cnt > 5){
					nosignl_cnt = 0;
					__stop_disply_thread();
				}
				else{
					nosignl_cnt++;
					usleep(5);
					goto CHECK_SINGNL;
				}
			}else if((1 == btvd_ctl.signal) && (BTVD_STOP == btvd_ctl.state) ){
				/*if(0 == screen_flag){
					__onoff_screen(1);
				}*/
				/*if(1 == is_in_stby){
					out_stby_mode();
				}*/
				
				nosignl_cnt = 0;
				btvd_ctl.exit = 0;
				
				data.reg = 0x8c;
				data.val = 0x00;
				ioctl(fd,BTVD_IO_R_REG, &data);
				
				reg = data.val & 0x0F;

				/*if(btvd_ctl.swf_cmd == BTVD_SWF_STOP){
					goto SLEEP;
				}*/
			
				if((1 == reg ) || (9 == reg)){//NTSC
					format = BT656_480I;
				}else if((3 == reg ) || (5 == reg) || (7 == reg) || (0x0a == reg)){
					format = BT656_576I;
				}else{
					goto SLEEP;
				}
			
				data.reg = 0x28;//将tmp5150 锁定在某一种制式 防止掉线
				data.val = reg + 1;
				ioctl(fd,BTVD_IO_W_REG, &data);

				//SWF_Message(NULL, SWF_MSG_KEY_EXT_PLAYER, NULL);
				//SWF_Message(NULL, SWF_MSG_PLAY, NULL);
				
				
				//__audio_sw(0);
				//in = AVIN;
				
				btvd_ctl.state = BTVD_RUN;
				pthread_create(&btvd_ctl.display_thread_id, NULL, __display_thread_proc, &format);
				
				sleep(1);
			}else{
				nosignl_cnt = 0;
			}
		}
		
SLEEP:
		//sleep(1);
		usleep(600*1000);
		
	}

CLOSE_OUT:
	close(fd);
	SWF_Free((void*)(info.buf1.logic_buf));
	SWF_Free((void*)(info.buf2.logic_buf));

CTL_OUT:
	pthread_exit(NULL);
	return NULL;
	
}




#if 0
#if P30PLUS == 1
static void * __sw_audio_check_avin_proc(void *arg)
{
	char value = 0;
	char old_value = 0;
	int err = 0;
	int tvfd = 0;
	int i = 10;
	int snfd = 0;
	unsigned char signal;
	int count = 50;
	
	struct btvd_data tv_data;

	tvfd = open("/dev/btvd", O_RDWR);
	
	do{
		printf("open btvd err\n");
		tvfd = open("/dev/btvd", O_RDWR);

		if(tvfd > 0){
			break;
		}
		
		sleep(1);
		count--;
		
	}while(tvfd < 0 && count > 0);

	
	if(tvfd < 0){
		btvd_info("open btvd err end@@@@@@@@\n");
	}
	
	set_tvp5150_power(0);
	
	while(1){
		extern char upgrade_now;
		if(1 == upgrade_now){
			goto OUT;
		}
		
		get_gpio(AVIN_DET, &value);
		//btvd_info("AVIN_DET ==0x%x", value);
		
		if(value != old_value){
			old_value = value;
			
			if(0 == value){// not in
				avin = 0;
				if(AVIN == in){
					__stop_disply_thread();
					if(0 == check_avin){
						set_tvp5150_power(0);
					}
				}else{
					__audio_sw(0);
				}
				
				in = NOT;
				btvd_info("NOT");
			}else{//avin or earphone
				if(in != AVIN){
					__audio_sw(1);
				}else{
					avin = 1;
					btvd_info("AVIN");
					goto SLEEP;
				}
				
				if(tvfd >= 0){
					i = 10;
					set_tvp5150_power(1);
					sleep(1);
					usleep(200*1000);
					
					do{
						tv_data.reg = 0xc0;
						tv_data.val = 0x00;
						if(tvp5150_state == TVP5150_STBY){
							set_tvp5150_power(1);
							usleep(200*1000);
							btvd_info("5150 is stby!!");
						}
						
						err = ioctl(tvfd,BTVD_IO_R_REG, &tv_data);
						
						if(err < 0){
							btvd_info(" sw_audio read tvp5150 err\n");
							//__audio_sw(1);
							in = EAR_PHONE;
							if(0 == check_avin){
								set_tvp5150_power(0);
							}
							btvd_info("EAR_PHONE");
							goto SLEEP;
						}
					
						signal = (0x80 & tv_data.val)>>7;

						if(1 == signal){
							break;
						}
						usleep(6);
						
					}while(--i);
				}else{
					signal = 0;
				}
				
				
				if(0 == signal){//earphone
					//__audio_sw(1);
					in = EAR_PHONE;
					if(0 == check_avin){
						set_tvp5150_power(0);
					}
					btvd_info("EAR_PHONE");
				}else{//avin
					//__audio_sw(0);
					in = AVIN;
					avin = 1;

					if(0 == screen_flag){
						__onoff_screen(1);
					}
					
					SWF_Message(NULL, SWF_MSG_KEY_EXT_PLAYER, NULL);
					SWF_Message(NULL, SWF_MSG_PLAY, NULL);
					btvd_info("AVIN");
				}
			}

		}
		
SLEEP:	
		
		usleep(400*1000);//0.4s
		//sleep(4);
	}
OUT:
	close(tvfd);
	
	pthread_exit(NULL);
	return NULL;
}

#else

static void * __sw_audio_check_avin_proc(void *arg)
{
	char value = 0;
	char old_value = 0;

	btvd_info("P30 audio sw thread run ,if are P30+ ,Please cheak sys_cfg.h\n");
	while(1){
		extern char upgrade_now;
		if(1 == upgrade_now){
			goto OUT;
		}
		
		get_gpio(AVIN_DET, &value);

		if(value != old_value){
			old_value = value;
			
			if(0 == value){// not in
				__audio_sw(0);
				in = NOT;
			}else{//earphone
				__audio_sw(1);
				in = EAR_PHONE;
				btvd_info("EAR_PHONE");
			}
		}
SLEEP:	
		usleep(500*1000);//0.4s
		//sleep(4);
	}

OUT:
	btvd_info("audio sw thread exit!!!!! check check, this is for audio switch earphone or spreak\n");
	pthread_exit(NULL);
	return NULL;
}

#endif
#endif




int get_avin_state()
{
	return btvd_ctl.state;
}




int setAvinCheckState(char check)
{
	int rtn = 0;
	
	if(0 == check ){
		check_avin = 0;
		btvd_ctl.swf_cmd = BTVD_SWF_STOP;
		btvd_info("avin stop!!");
		if(BTVD_RUN == btvd_ctl.state){
			__stop_disply_thread();
			in = NOT;
		}

		usleep(300*1000);
		rtn = set_tvp5150_power(0);
	}else{
		check_avin = 1;
		in = SCAN_AVIN;
		rtn = set_tvp5150_power(1);
		btvd_ctl.swf_cmd = BTVD_SWF_RUN;
	}

	return rtn;
}


int btvd_work_start(void)
{
	int ret = 0;
	int fd = 0;
	
	sem_init(&btvd_ctl.sem, 0 ,1);
	btvd_ctl.state = BTVD_STOP;
	btvd_ctl.display_thread_id = -1;
	btvd_ctl.exit = 0;
#if 0	
#if P30PLUS == 1
	fd = open("/dev/mem", O_RDWR | O_SYNC);		

	if(fd<0){
		printf("open/dev/mem error\n");
	}else{
		adc_base_addr = (int)mmap(0, 0x08, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x10110000);
		dac_base_addr = (int)mmap(0, 0x2A, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x10100000);
	//	btvd_info("adc_base_addr = 0x%x\n ", adc_base_addr);
	//	RegBitSet(0, adc_base_addr+0x00, 15, 14); //ADC GC 0:-6db
	//	RegBitSet(7, adc_base_addr+0x00, 11, 9); // FMIN GC
		close(fd);
	}
	
	pthread_create(&btvd_ctl.ctl_thread_id, NULL, __ctl_thread_proc, NULL);
#endif
#endif
	char cmd[100];
	int err = 0;
	sprintf(cmd,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"i2c-am7x.ko");
	err = system(cmd);
	printf("%s %d err=%d\n",__FILE__,__LINE__,err);
	pthread_create(&btvd_ctl.ctl_thread_id, NULL, __ctl_thread_proc, NULL);
	
	//pthread_create(&sw_thread_id, NULL, __sw_audio_check_avin_proc, NULL);
	
	return ret;
}

int btvd_work_exit(void)
{
	int ret = 0;

	btvd_ctl.swf_cmd = BTVD_SWF_STOP;
	
	return ret;
}


