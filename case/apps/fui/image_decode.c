#include "image_decode.h"
#include "system_info.h"
#include <semaphore.h>
#include <time.h>
#include <errno.h>

#include "display.h"

#include "stream_api.h"

//#define IMAGE_DEC_DEBUG
#define IMG_ERR_MSG(fmt,arg...) printf("IMG_ERR_MSG[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

#ifdef IMAGE_DEC_DEBUG
#define imgtrd_info(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define imgtrd_err(fmt,arg...) printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

#else

#define imgtrd_info(fmt,arg...) do{}while(0);
#define imgtrd_err(fmt,arg...) do{}while(0);
#endif


#define DIGITAL_PANEL 1
#define ANALOG_PANEL 2
#define DEC_REQ_NUM 128


#define GUI_BLACK         0x80108010
#define GUI_SILVER	0x80b980b9
#define GUI_WHITE	0x80eb80eb
#define GUI_BLUE          0x6E29F029
#define GUI_GREEN         0x22913691
#define GUI_CYAN          0x10AAA6AA
#define GUI_RED           0xf0525a52
struct img_dec_res_s{
	sem_t img_sem;   ///<used for lock the img_dec_req
	sem_t img_wait_stop; ///<used for waiting decoding stop
	unsigned int is_thread_run;  ///<note whether the task is running
	//int thread_stop;
	int req_start_idx;
	void*filehandle_reserve;  ///<note the file handle which had not been closed
	int filehandle_reserve_idx; ///<note the file handle came from which idx in the array
};

/******IO Interface*******/
extern void *fui_os_fopen(char *path, char *mode);
extern int fui_os_fclose(void *fp);
extern long fui_os_fread(void *fp, unsigned char *buf, unsigned long nbytes);
extern long fui_os_fwrite( void *fp,void *ptr, int nbytes);
extern long fui_os_fseek_set(void *fp, long offset);
extern long fui_os_fseek_cur(void *fp, long offset);
extern long fui_os_fseek_end(void *fp, long offset);
extern long fui_os_ftell(void *fp);
extern int fui_os_fflush(void *fp);
/*************************/
extern unsigned long swf_heap_logic_start;
extern unsigned long swf_heap_physic_start;

bg_img_dec_t img_dec_req[DEC_REQ_NUM];
static unsigned char *tmpbuf_img_dec=NULL;
static unsigned int tmpbuf_size = 0;

static unsigned char panel=DIGITAL_PANEL;
static pthread_t img_thread_id;
static void *img_thread_ret;
static void *dlhandle=NULL;
static void *imghandle;
static unsigned long timestamp_ID=1;
struct img_dec_res_s img_res={
	.is_thread_run=0,
	//.thread_stop = 0,
	.req_start_idx = 0,
	.filehandle_reserve=NULL,
	.filehandle_reserve_idx = -1,
};
struct img_dec_res_s bmp_res={
	.is_thread_run=0,
	//.thread_stop = 0,
	.req_start_idx = 0,
	.filehandle_reserve=NULL,
	.filehandle_reserve_idx = -1,
};

void *(*img_dec_open)(void *param);
int (*img_dec_close)(void *);
int (*img_dec_send_msg)(void *, void *, int, int);
int (*img_dec_get_msg)(void *, void *, int, int);
int (*img_dec_stop)(void*);

char is_filehandle_fops=0;

//add for dlna

typedef struct photo_file_iocontext
{
	void *(*open)(char *path, char *mode);						/**< open file opertion func */	
	int (*read)(void *fp, unsigned char *buf, int buf_size);	/**< read file opertion func */
	int (*write)(void *fp, unsigned char *buf, int buf_size);	/**< write file opertion func */
	long (*seek_set)(void *fp, long offset);					/**< seek set opertion func */
	long (*seek_end)(void *fp, long offset);					/**< seek end opertion func */
	long (*tell)(void *fp);									/**< tell file opertion func */
	int (*close)(void *fp);										/**< close file opertion func */
}file_iocontext_t;

file_iocontext_t  photo_fops;


img_io_layer_t *image_decode_fops;

//end of add


static void _img_printf_queue();
#if 1   //relate to the application, changing the following functions
int _img_dec_open_file(bg_img_dec_t * param,char * file,void *type)
{
	param->img_dec_para_info.handle = fui_os_fopen(file,"rb");
	//imgtrd_info("Filehandle=0x%x,reservefilehandle=0x%x,re_idx=%d\n",
	//	param->img_dec_para_info.handle,img_res.filehandle_reserve,img_res.filehandle_reserve_idx);
	if(param->img_dec_para_info.handle==NULL){
		printf("%s,%d:Open File=%s failed\n",__FILE__,__LINE__,file);
		return 0;
	}
	param->type = type;
	return 1;
}

int _img_dec_close_file(bg_img_dec_t * param,int handleidx)
{
	//used the type 
	if(param==NULL){
		if(img_res.filehandle_reserve){
			imgtrd_info("Close File,idx=%d,Reserve fhandle=0x%x",img_res.filehandle_reserve_idx,img_res.filehandle_reserve);
			//fui_os_fclose(img_res.filehandle_reserve);
			photo_fops.close(img_res.filehandle_reserve);
			if(img_res.filehandle_reserve_idx>=0)
				img_dec_req[img_res.filehandle_reserve_idx].img_dec_para_info.handle=NULL;
			img_res.filehandle_reserve = NULL;
			img_res.filehandle_reserve_idx = -1;
		}
	}
	else{
		if(param->is_fhandle==0){
			//imgtrd_info("Close File,ParamHandle=0x%x,Prevfhandle=0x%x,idx=%d",
			//param->img_dec_para_info.handle,img_res.filehandle_reserve,handleidx);
			if(img_res.filehandle_reserve==param->img_dec_para_info.handle||img_res.filehandle_reserve_idx== handleidx){
				imgtrd_err("Err Some Thing May Be Wrong!");
			}
			if(img_res.filehandle_reserve){
				photo_fops.close(img_res.filehandle_reserve);
				if(img_res.filehandle_reserve_idx>=0 &&img_res.filehandle_reserve_idx!=handleidx)
					img_dec_req[img_res.filehandle_reserve_idx].img_dec_para_info.handle=NULL;
			}
			img_res.filehandle_reserve=param->img_dec_para_info.handle;
			img_res.filehandle_reserve_idx= handleidx;
		}
	}
	return 1;
}


//////////////////////////
/*add for dlna*/

static int _check_file_exist(const char *filename)
{
	return access(filename, 0); 
}


static void *_image_fopen_stream(char *path, char *mode)
{
	int file_format;
	
	return (void*)am_stream_open(path, NULL, &file_format);
}

static int _image_fclose_stream(void *fp)
{
	 am_stream_close(fp);

	 return 0;
}

static int _image_read_stream(void *opaque, unsigned char *buf, long buf_size)
{
	return (int)am_stream_read(opaque, buf, buf_size);
}

static int _image_write_stream(void *opaque, unsigned char *buf, long buf_size)
{
	return buf_size;
}

static int _image_seek_set_stream(void *opaque, long offset)
{
	return am_stream_seek_set(opaque, offset);
}


static int _image_seek_cur_stream(void *opaque, long offset)
{
	return am_stream_seek_cur(opaque, offset);
}


static int _image_seek_end_stream(void *opaque, long offset)
{
	return am_stream_seek_end(opaque, offset);
}

static long _image_tell_stream(void *opaque)
{
	return am_stream_get_pos(opaque);
}

int _set_fs_ops(file_iocontext_t * param,int type)
{
	if(0 == type){
		
		param->open=(void*)fui_os_fopen;
		param->close= fui_os_fclose;
		param->read=fui_os_fread;
		param->seek_set=fui_os_fseek_set;
		param->seek_end=fui_os_fseek_end;
		param->tell =fui_os_ftell;
		//param->write=am_stream_(void * s)
		
	}
	else{

		param->open=(void*)am_stream_open;
		param->close= am_stream_close;
		param->read=am_stream_read;
		param->seek_set=am_stream_seek_set;
		param->seek_end=am_stream_seek_end;
		param->tell =am_stream_get_pos;
		
	}
	return 0;
}

//end of end

//////////////////////////




int _img_dec_set_fs_ops(IMG_DECODE_PARAM_S * param,img_io_layer_t *img_io_layer,void* type)
{
	if(img_io_layer==NULL){
		param->img_fread = fui_os_fread;
		param->img_fseek_set = fui_os_fseek_set;
		param->img_fseek_cur = fui_os_fseek_cur;
		param->img_fseek_end = fui_os_fseek_end;
		param->img_ftell = fui_os_ftell;

		_set_fs_ops(&photo_fops, 0);
	}
	else{
		param->img_fread =_image_read_stream;
		param->img_fseek_set = _image_seek_set_stream;
		param->img_fseek_cur = _image_seek_cur_stream ;
		param->img_fseek_end = _image_seek_end_stream;
		param->img_ftell = _image_tell_stream ;
		_set_fs_ops(&photo_fops, 1);
	}
		
	return 1;
}
#endif
#if 1
static int _img_check_photo_db_magic(PHOTO_DB_T *db)
{
	if( db->magic[0] == 'A' && \
		db->magic[1] == 'C' && \
		db->magic[2] == 'T' && \
		db->magic[3] == '-' && \
		db->magic[4] == 'M' && \
		db->magic[5] == 'I' && \
		db->magic[6] == 'C' && \
		db->magic[7] == 'R' && \
		db->magic[8] == 'O' ){

		return 0;
	}
	else{
		return -1;
	}
}

static int _img_set_photo_db_magic(PHOTO_DB_T *db)
{
	db->magic[0] = 'A';
	db->magic[1] = 'C'; 
	db->magic[2] = 'T'; 
	db->magic[3] = '-'; 
	db->magic[4] = 'M'; 
	db->magic[5] = 'I'; 
	db->magic[6] = 'C'; 
	db->magic[7] = 'R'; 
	db->magic[8] = 'O'; 

	return 0;
}

/**
@brief do the effect of old photo
@param[in] pixel	: the buffer where the original data stored
@param[in] stride	: the stride of the original data, if it is YUV422, the stride equal to w*2
@param[in] w		: the width of the photo
@param[in] h		: the height of the photo
@return none
**/
static int _img_effect_oldphoto(char *pixel,int stride,int w,int h)
{
	int i,j;
	unsigned int addr;
	for(j=0;j<h;j++){	
		addr= (unsigned int)(pixel + stride*j);
		for(i=0;i<w/2;i++){
			*((	int *)addr)=(((*((	int *)addr))&0x00ff00ff)|0x94004800);
			addr=addr+4;
		}
	}
	return 0;
}

/**
@brief do the effect of black-white
@param[in] pixel	: the buffer where the original data stored
@param[in] stride	: the stride of the original data, if it is YUV422, the stride equal to w*2
@param[in] w		: the width of the photo
@param[in] h		: the height of the photo
@return none
**/
static int _img_effect_blackwhite(char *pixel,int stride,int w,int h)
{
	int i,j;
	unsigned int addr;
	
	for(j=0;j<h;j++){	
		addr= (unsigned int)(pixel + stride*j);
		for(i=0;i<w/2;i++){
			*((int*)addr)=(((*((int*)addr))&0x00ff00ff)|0x80008000);
			addr=addr+4;
		}
	}
	return 0;
}


static int _img_do_photo_effect(char effect, char*pixel,int stride, int w, int h)
{
	int rtn=0;
	imgtrd_info("Do Photo Effect!effect==%d",effect);
	switch(effect){
		case PHOTO_OLDPHOTO:
			_img_effect_oldphoto(pixel,stride,w,h);
			break;
		case PHOTO_BW:
			_img_effect_blackwhite(pixel,stride,w,h);
			break;
		default:
			imgtrd_err("Sorry the effect=%d can't support yet!",effect);
			break;
	}
	return rtn;
}


#endif

#if 1


int file_fflush(void *fp)
{
	int fd;
	fflush((FILE *)fp);
	fd = fileno((FILE *)fp);
	if(fsync(fd)==-1){
		printf("%s,%d: Fflush Error!\n",__FILE__,__LINE__);
		return -1;
	}
	return 0;
}

int _img_test_save_to_file(char* databuf,unsigned int datalen,char *filename)
{
	FILE *fp=NULL;
	unsigned int len_write=0;
	if(filename==NULL)
		return 0;
	fp = fopen(filename,"wb+");
	if(fp==NULL){
		printf("%s,%d:Open File Error=%s\n",__FILE__,__LINE__,filename);
		return 0;
	}
	len_write=fwrite(databuf,sizeof(char),datalen,fp);
	if(len_write!=datalen)
		printf("%s,%d:Write File Error!\n",__FILE__,__LINE__);
	else
		printf("Save File OK filename=%s\n",filename);
	printf("Len_Write=%d,data_len=%d\n",len_write,datalen);
	file_fflush(fp);
	fclose(fp);
	return 1;
}

static void _img_sem_wait(sem_t * sem)
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
static void _img_sem_post(sem_t * sem)
{
	int err;
	err = sem_post(sem);
}

void _init_img_dec_req(){
	_img_sem_wait(&img_res.img_sem);
	memset(img_dec_req,0,sizeof(bg_img_dec_t)*DEC_REQ_NUM);
	_img_sem_post(&img_res.img_sem);
}

int _img_dec_req_enqueue(bg_img_dec_t * req){
	int i=0,rtn,tmp_idx;
	_img_sem_wait(&img_res.img_sem);
	for(i=0;i<DEC_REQ_NUM;i++){
		/**search the positon to insert the request,**/
		tmp_idx = (img_res.req_start_idx+i)%DEC_REQ_NUM;
		if(img_dec_req[tmp_idx].active==REQ_INVALID|| img_dec_req[tmp_idx].active==REQ_DECODED){
			if(img_dec_req[tmp_idx].active==REQ_INVALID)
				break;
			if(img_dec_req[tmp_idx].active==REQ_DECODED&& tmp_idx!=img_res.filehandle_reserve_idx)
				break;
		}
	}
	if(i>=DEC_REQ_NUM){
		imgtrd_err("Sorry,the Queue is full");
		_img_printf_queue();
		rtn = -1;
	}
	else{
		memcpy(img_dec_req+tmp_idx,req,sizeof(bg_img_dec_t));
		imgtrd_info("Enque[%d],handle=0x%x,pic_ID=0x%x,reserve_handle=0x%x,rev_idx=%d",tmp_idx,img_dec_req[tmp_idx].img_dec_para_info.handle,
			img_dec_req[tmp_idx].img_dec_para_info.pic_ID,img_res.filehandle_reserve,img_res.filehandle_reserve_idx);
		img_dec_req[tmp_idx].active = REQ_ENQUEUE;
		//save the position where the request inserted for the next turn of searching//
		img_res.req_start_idx = tmp_idx;
		rtn  = tmp_idx;
	}
	_img_sem_post(&img_res.img_sem);
	return rtn;
}

int _img_dec_req_dequeue(bg_img_dec_t * req){
	int i=0,rtn;
	_img_sem_wait(&img_res.img_sem);
	for(i=0;i<DEC_REQ_NUM;i++){
		if(img_dec_req[i].active==REQ_ENQUEUE){
			break;
		}
	}
	if(i>=DEC_REQ_NUM){
		rtn = -1;
	}
	else{
		memcpy(req,img_dec_req+i,sizeof(bg_img_dec_t));
		img_dec_req[i].active = REQ_DEQUEUE;
		//imgtrd_info("Get Index=%d",i);
		rtn  = i;
	}
	_img_sem_post(&img_res.img_sem);
	return rtn;
}


inline void  _img_post_stop()
{
	imgtrd_info("Post Stop");
	_img_sem_post(&img_res.img_wait_stop);
}

inline void _img_wait_stop()
{
	imgtrd_info("Wait Stop 111");
	_img_sem_wait(&img_res.img_wait_stop);
	imgtrd_info("Wait Stop 222");
}

int _img_dec_releasehandle(int idx)
{
	if(img_dec_req[idx].active!=REQ_INVALID){
	//imgtrd_info("[i=%d] reserve_idx=%d\n",i,img_res.filehandle_reserve_idx);
		if(idx!=img_res.filehandle_reserve_idx){
			imgtrd_info("<Release [%d]>active=%d Close File Handle=0x%x",idx,img_dec_req[idx].active,img_dec_req[idx].img_dec_para_info.handle);
			if(img_dec_req[idx].img_dec_para_info.handle && img_dec_req[idx].img_dec_para_info.handle!=img_res.filehandle_reserve)
				photo_fops.close(img_dec_req[idx].img_dec_para_info.handle);
			img_dec_req[idx].img_dec_para_info.handle=NULL;
			img_dec_req[idx].active = REQ_INVALID;
			//memset(img_dec_req+idx,0,sizeof(bg_img_dec_t));
		}
	}
	return 0;
}
//if the dec stop is called the requests of decoding will be canceled
int _img_dec_stop(){
	int i=0,rtn=0;
	_img_sem_wait(&img_res.img_sem);
	for(i=0;i<DEC_REQ_NUM;i++){
		_img_dec_releasehandle(i);
	}
	_img_sem_post(&img_res.img_sem);
	return 0;
}


//save the decode result to the queue
//req: the img_dec_ret_info of req must get from middle ware
//return: 0 success, -1 failed
int _img_save_dec_result(bg_img_dec_t * req){
	int rtn=0;
	int i=0;
	int dec_cmd=-1;
	_img_sem_wait(&img_res.img_sem);
	for(i=0;i<DEC_REQ_NUM;i++){
		if(img_dec_req[i].active!=REQ_DEQUEUE)
			continue;
		//printf("ID 1 =%d,ID 2=%d  ",img_dec_req[i].img_dec_para_info.pic_ID,req->img_dec_ret_info.pic_ID);
		//printf("Tiem ID 1=0x%x, Time ID 2=0x%x\n",img_dec_req[i].timestamp_ID,req->timestamp_ID);
		if(img_dec_req[i].timestamp_ID== req->img_dec_ret_info.pic_ID){
			memcpy(&img_dec_req[i].img_dec_ret_info,&req->img_dec_ret_info,sizeof(IMG_DECODE_INFO_S));
			img_dec_req[i].active = REQ_DECODED;
			imgtrd_info("cmd=%d,pic_ID=0x%x,save oK,req[%d].active=%d,Result=%d,handle=0x%x", img_dec_req[i].dec_cmd ,req->img_dec_ret_info.pic_ID,i,img_dec_req[i].active,\
				img_dec_req[i].img_dec_ret_info.result,img_dec_req[i].img_dec_para_info.handle);
			rtn =0;

			imgtrd_info("DEBUG:image file name=%s\n",img_dec_req[i].file_name);

			if( img_dec_req[i].dec_cmd==BG_IMG_DECODE || img_dec_req[i].dec_cmd ==BG_IMG_SET_ROTATION){
				if(_check_file_exist(img_dec_req[i].file_name) != -1){
					_img_dec_close_file(&img_dec_req[i],i);
				}
				else{
					_img_dec_close_file_stream(&img_dec_req[i],i);
				}
			}
			if((img_dec_req[i].dec_cmd==BG_IMG_DECODE||img_dec_req[i].dec_cmd ==BG_IMG_SET_ROTATION) \
				&&  img_dec_req[i].img_dec_para_info.thumb_browse_mode==0){
				int background_effect=0;
				background_effect = get_photo_background_effect();
				image_radiant(img_dec_req+i,background_effect);
			}
			/**do photo effect**/
			#if 0
			{///just for test 
				char* pixel =(char*)img_dec_req[i].img_dec_para_info.out_buf_y.buf;
				int w = img_dec_req[i].img_dec_para_info.output_width;
				int h =  img_dec_req[i].img_dec_para_info.output_height;
				char filename[64]="";
				static int start_index=0;
				sprintf(filename,"/mnt/udisk/0x%x_%d.dat",req->img_dec_ret_info.pic_ID,start_index);
				start_index++;
				imgtrd_info("Save Buffer name=%s,w=%d,h=%d",filename,w,h);
				_img_test_save_to_file(pixel,w*h*2,filename);
			}
			#endif
			if(img_dec_req[i].photo_effect!=0){
				char effect =img_dec_req[i].photo_effect;
				char* pixel =(char*)img_dec_req[i].img_dec_para_info.out_buf_y.buf;
				int w = img_dec_req[i].img_dec_para_info.output_width;
				int h =  img_dec_req[i].img_dec_para_info.output_height;
				int stride = 0;
				if( img_dec_req[i].img_dec_para_info.formate == IMG_FMT_YCBCR_4_2_2_INTERLEAVED){
					stride = w*2;
					_img_do_photo_effect(effect,pixel,stride,w,h);
				}
				else
					imgtrd_err("Do Photo Effect, formate=%d is not support!",img_dec_req[i].img_dec_para_info.formate);
					
			}
			
			break;
		}
	}
	if(i>=DEC_REQ_NUM){
		imgtrd_err("ID didn't match:can't find pic_ID=0x%x",req->img_dec_ret_info.pic_ID);
		rtn = -1;
	}
	_img_sem_post(&img_res.img_sem);
	return rtn;
}

unsigned long _img_get_timestamp_ID(){
	struct timeval cur_time;
	if(gettimeofday(&cur_time,NULL)==0)	
		return cur_time.tv_sec*1000000L+cur_time.tv_usec;
	else{
		if(timestamp_ID>=0xff)
			timestamp_ID = 1;
		timestamp_ID++;
		return timestamp_ID;
	}
}



int _img_dec_open(){
	#if 1  // using dynamic loading
	char mmmlib[64];
	memset((void*)mmmlib,0,64);
	sprintf(mmmlib,"%s%s", AM_DYNAMIC_LIB_DIR,"lib_image.so");
	//sprintf(mmmlib,"%s%s", AM_DYNAMIC_LIB_DIR,"libphoto.so");
	
	if(dlhandle){
		dlclose(dlhandle);
	}

	dlhandle = dlopen(mmmlib, RTLD_LAZY);
	if(dlhandle==NULL){
		goto P_OPEN_ERROR1;
	}
	img_dec_open = (void *(*)(void *))dlsym(dlhandle,"image_dec_open");
	if(img_dec_open == NULL){
		goto P_OPEN_ERROR2;
	}

	img_dec_close = dlsym(dlhandle,"image_dec_close");
	if(img_dec_close == NULL){
		goto P_OPEN_ERROR2;
	}
	
	img_dec_send_msg = dlsym(dlhandle,"image_dec_send_msg");
	if(img_dec_send_msg == NULL){
		goto P_OPEN_ERROR2;
	}

	img_dec_get_msg = dlsym(dlhandle,"image_dec_get_msg");
	if(img_dec_get_msg == NULL){
		goto P_OPEN_ERROR2;
	}

	img_dec_stop=dlsym(dlhandle,"image_dec_stop");
	if(img_dec_stop==NULL)
		goto P_OPEN_ERROR2;
	#else
	dlhandle =NULL;
	img_dec_open = image_dec_open;
	img_dec_close = image_dec_close;
	img_dec_send_msg = image_dec_send_msg;
	img_dec_get_msg = image_dec_get_msg;
	img_dec_stop = image_dec_stop;
	#endif
	imghandle = img_dec_open(NULL);
	is_filehandle_fops =0;
	if(imghandle==NULL){
		imgtrd_err("Image dec Open Error\n");
		goto P_OPEN_ERROR2;
	}
	imgtrd_info("~~~~~~~~DEC Open SUCCESS~~~~~~~~");
	return 0;
P_OPEN_ERROR2:
	if(dlhandle){
		dlclose(dlhandle);
		dlhandle = NULL;
	}
P_OPEN_ERROR1:
	imgtrd_err("Get Func in SO error!");
	img_dec_get_msg =NULL;
	img_dec_send_msg = NULL;
	img_dec_close = NULL;
	img_dec_stop = NULL;
	img_dec_open = NULL;

	imghandle=NULL;
	return -1;	
}
	


int _img_dec_close(){
	if(img_dec_close && imghandle){
		imgtrd_info("Image DEC Close");
		if(img_dec_close!=NULL)
			return img_dec_close(imghandle);
		img_dec_get_msg =NULL;
		img_dec_send_msg = NULL;
		img_dec_close = NULL;
		img_dec_stop = NULL;
		img_dec_open = NULL;
		return 0;
	}
	return -1;
}

unsigned int _img_get_tmpbuf_size()
{
	//int lcm_width = system_get_screen_para(CMD_GET_SCREEN_WIDTH);
	//int lcm_height = system_get_screen_para(CMD_GET_SCREEN_HEIGHT);
	//printf("lcm_width=========%d\n",lcm_width);
	
	//printf("lcm_height=========%d\n",lcm_height);
#if AM_CHIP_ID == 1213
	return 1920*1080*3/2+1024*512+512+100*1024;
#else
	return 1280*800*3/2+1024*512+512+100*1024;
#endif
}

unsigned char * _img_get_dec_tmpbuf(){
	unsigned int new_size = _img_get_tmpbuf_size();
	
DEC_TMPBUF_MALLOC_AGAIN:
	if(tmpbuf_img_dec==NULL){
		tmpbuf_img_dec =(unsigned char*) SWF_Malloc(new_size);
		if(tmpbuf_img_dec==NULL){
			imgtrd_err("Sorry Mallloc TemBuf for IMG_DEC Error");
			return NULL;
		}
		else
			tmpbuf_size = new_size;
	}
	else{
		if(tmpbuf_size!=0 && tmpbuf_size!= new_size){
			SWF_Free(tmpbuf_img_dec);
			tmpbuf_img_dec==NULL;
			tmpbuf_size = 0;
			goto DEC_TMPBUF_MALLOC_AGAIN;
		}
	}
	return tmpbuf_img_dec;
}
void _img_free_dec_tmpbuf(){
	if(tmpbuf_img_dec!=NULL){
		SWF_Free(tmpbuf_img_dec);
		tmpbuf_img_dec = NULL;
		tmpbuf_size = 0;
	}
}

//map the dec_cmd to the commond used in photo midware
int _img_dec_req_map(bg_img_dec_t * req){
	switch(req->dec_cmd){
			case BG_IMG_ROT_RIGHT_90:
				req->img_dec_para_info.cmd = IMG_ROT_RIGHT_90;
				imgtrd_info("Rot right 90");
				break;
			case BG_IMG_ROT_LEFT_90:
				req->img_dec_para_info.cmd = IMG_ROT_LEFT_90;
				break;
			case BG_IMG_ROT_HOR_FLIP:
				req->img_dec_para_info.cmd = IMG_ROT_HOR_FLIP;
				break;
			case BG_IMG_ROT_VER_FLIP:
				req->img_dec_para_info.cmd = IMG_ROT_VER_FLIP;
				break;
			case BG_IMG_ROT_180:
				req->img_dec_para_info.cmd = IMG_ROT_180;
				break;
			case BG_IMG_ZOOM_IN:
				req->img_dec_para_info.cmd = IMG_ZOOM_IN;
				break;
			case BG_IMG_ZOOM_OUT:
				req->img_dec_para_info.cmd = IMG_ZOOM_OUT;
				break;
			case BG_IMG_MOVE_UP:
				req->img_dec_para_info.cmd = IMG_MOVE_UP;
				break;
			case BG_IMG_MOVE_DOWN:
				req->img_dec_para_info.cmd = IMG_MOVE_DOWN;
				break;
			case BG_IMG_MOVE_LEFT:
				req->img_dec_para_info.cmd = IMG_MOVE_LEFT;
				break;
			case BG_IMG_MOVE_RIGHT:
				req->img_dec_para_info.cmd = IMG_MOVE_RIGHT;
				break;
			case BG_IMG_ZOOM_RESET:
				req->img_dec_para_info.cmd = IMG_ZOOM_RESET;
				break;
			case BG_IMG_SET_SCALE_RATE:
				break;
			case BG_IMG_SET_ROTATION:
				req->img_dec_para_info.cmd = IMG_SET_ROTATION;
				break;
			case BG_IMG_GET_PHOTO_INFO:
				break;
			case BG_IMG_DEC_STOP:
				req->img_dec_para_info.cmd = IMG_DEC_STOP;
				break;
			case BG_IMG_DECODE:
				req->img_dec_para_info.cmd = IMG_DECODE;
				break;
			case BG_IMG_DEC_EXIT:
				return -1;
		}
	return 0;
}

void _img_print_addr(IMG_LINEAR_BUFFER *buffer){
	imgtrd_info("Buf=0x%x,bus_addr=0x%x,size=0x%x",buffer->buf,buffer->bus_addr,buffer->size);
}

void _img_printf_queue()
{
	int i=0;
	_img_sem_wait(&img_res.img_sem);
	for(i=0;i<DEC_REQ_NUM;i++){
		imgtrd_info("Idx<%d>.active=%d,handle=0x%x",i,img_dec_req[i].active,img_dec_req[i].img_dec_para_info.handle);
	}
	_img_sem_post(&img_res.img_sem);
}
void _img_printf_de(){
	void *test_deinst=NULL;
	DE_config test_ds_conf;
	de_init(&test_deinst);
	de_get_config(test_deinst,&test_ds_conf,DE_CFG_ALL);
	imgtrd_info("~~~~~~~~~~~~~~~~~de out~~~~~~~~~~~~~~~~~~~~~");
	printf("dev_info.h=%d,dev_info.w=%d",test_ds_conf.dev_info[DE_OUT_DEV_ID_LCD].height,test_ds_conf.dev_info[DE_OUT_DEV_ID_LCD].width);
	printf("cropw=%d,croph=%d,cropx=%d,cropy=%d\n",test_ds_conf.input.crop_width,test_ds_conf.input.crop_height,test_ds_conf.input.crop_x,test_ds_conf.input.crop_y);
	imgtrd_info("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
}

void _img_print_para_info(bg_img_dec_t *para){
	imgtrd_info("******************para info***********************");
	imgtrd_info("ID=%d,timeID=0x%x,active=%d,cmd=%d,mode=%d",para->pic_ID,para->timestamp_ID,para->active,para->dec_cmd,para->dec_mode); 
	imgtrd_info("filehandle=%s",para->img_dec_para_info.handle);
	imgtrd_info("picID=0x%x,out_w=%d,out_h=%d",para->img_dec_para_info.pic_ID,para->img_dec_para_info.output_width,para->img_dec_para_info.output_height);
	_img_print_addr(&para->img_dec_para_info.out_buf_y);
	imgtrd_info("lcd_width=%d,lcd_height=%d,dec_cmd=%d",para->img_dec_para_info.lcd_width,para->img_dec_para_info.lcd_height,para->img_dec_para_info.cmd);
	imgtrd_info("browse_mode=%d,pip_enable=%d",para->img_dec_para_info.thumb_browse_mode,para->img_dec_para_info.pip_enable);
	imgtrd_info("dec_mode=%d,start_x=%d,start_y=%d",para->img_dec_para_info.pic_dec_mode,para->img_dec_para_info.pip_start_x,para->img_dec_para_info.pip_start_y);
	imgtrd_info("frame_width=%d,frame_height=%d",para->img_dec_para_info.pip_frame_width,para->img_dec_para_info.pip_frame_height);
	imgtrd_info("usr_rotated=%d",para->img_dec_para_info.usr_rotated);
	imgtrd_info("*******************************************************");
}

void _img_print_dec_info(IMG_DECODE_INFO_S *dec_info)
{
	if (dec_info)
	{
		imgtrd_info("*******************************************************");
		imgtrd_info("dec_info.pic_ID == %u", dec_info->pic_ID);
		imgtrd_info("dec_info.result == %d", dec_info->result);
		imgtrd_info("dec_info.img_ready == %d", dec_info->img_ready);
		imgtrd_info("dec_info.src_width == %u", dec_info->src_width);
		imgtrd_info("dec_info.src_height == %u", dec_info->src_height);
		imgtrd_info("dec_info.img_start_x == %d", dec_info->img_start_x);
		imgtrd_info("dec_info.img_start_y == %d", dec_info->img_start_y);
		imgtrd_info("dec_info.img_actual_wid == %u", dec_info->img_actual_wid);
		imgtrd_info("dec_info.img_actual_hei == %u", dec_info->img_actual_hei);
		imgtrd_info("dec_info.cur_scale_rate == %u", dec_info->cur_scale_rate);
		imgtrd_info("dec_info.date.year == %u", dec_info->date.year);
		imgtrd_info("dec_info.date.month == %u", dec_info->date.month);
		imgtrd_info("dec_info.date.day == %u", dec_info->date.day);
		imgtrd_info("*******************************************************");
	}
}


//set the buffer with the specified color, used in the YUV422 format
//buffer: buffer address
//color: see GUI_BLACK and others
//count: the length of the buffer 
void _img_memset4(unsigned int * buffer,unsigned int color, int count){
 	int i; 
	if(((int)buffer&0x3)) 
		imgtrd_err("memset4 error %x %x\n",buffer); 
	for(i=0;i<count;i++) 
		*(buffer+i)=color; 
}

//make the thread sleep, the unit is millseconds
void _img_thread_sleep(unsigned int millsec)
{
	struct timespec time_escape;
	time_escape.tv_sec = millsec/1000;
	time_escape.tv_nsec = (millsec-time_escape.tv_sec*1000)*1000000L;
	nanosleep(&time_escape,NULL);
}

int _img_change_req_status(int dec_cmd,int index,int newstatus)
{
	_img_sem_wait(&img_res.img_sem);
	
	if(dec_cmd==BG_IMG_DEC_STOP){
		if(index>=0 && index <DEC_REQ_NUM)
			img_dec_req[index].active = newstatus;
	}
	_img_sem_post(&img_res.img_sem);

	return 0;
}

int _img_send_msg()
{
	int img_dec_idx=0;
	bg_img_dec_t tmp_req;
	img_dec_idx = _img_dec_req_dequeue(&tmp_req);
	if(img_dec_idx>=0){
		if(_img_dec_req_map(&tmp_req)==-1){
			return -1;
		}
		//_img_print_para_info(&tmp_req);
		if(img_dec_send_msg!=NULL){
			img_dec_send_msg(imghandle, (void *)(&tmp_req.img_dec_para_info), sizeof(IMG_DECODE_PARAM_S), DECODE_PARAM);
			imgtrd_info("Send OK idx==%d,handle=0x%x,pid_ID=0x%x",img_dec_idx,tmp_req.img_dec_para_info.handle,tmp_req.img_dec_para_info.pic_ID);
			return 0;
		}
	}
	else{
		imgtrd_info("there are not msg need to be send in the queue");
	}
	return -1;
}
static pthread_t bmp_thread_id;
extern int bmp_dec_finished;
int exit_bmp()
{
	//printf("%s,%d:~~~~~~~~~~Close IMAGE DEC~~~~~~",__func__,__LINE__);
	if(bmp_res.is_thread_run==1)
	{
		printf("%s,%d:~~~~~~~~~~Close IMAGE DEC~~~~~~\n",__func__,__LINE__);
		//  _img_sem_post(&bmp_res.img_wait_stop);
		bmp_dec_finished=1;
		//printf("%s,%d:~~~~~~~~~~Close IMAGE DEC~~~~~~",__func__,__LINE__);
		_img_sem_wait(&bmp_res.img_wait_stop);	
		//printf("%s,%d:~~~~~~~~~~Close IMAGE DEC~~~~~~",__func__,__LINE__);
		bmp_dec_finished=0;
		bmp_res.is_thread_run=0;
        		pthread_cancel(bmp_thread_id);
		
	}
	printf("%s,%d:~~~~~~~~~~Close IMAGE DEC~~~~~~\n",__func__,__LINE__);
	
}
/**
* the thread which interact with photo middle ware
*/
void  bmp_thread_exit(void)
{

	_img_sem_post(&bmp_res.img_wait_stop);	
	
	printf("~~~~~~~~~~Image Thread exit~~~~~~~~~~\n");
	//img_res.is_thread_run = 0;
	pthread_exit((void*)1);
}
extern void  *bmp_dec(void *arg);
extern char path[512];
extern int bmp_height;
extern int bmp_width;
void creat_bmp_thread()
{
  int ret;
   printf("Create image thread!");
   ret = pthread_create(&bmp_thread_id,NULL,bmp_dec,NULL);
	if(ret==-1){
		printf("Create image thread error!");
		sem_destroy(&bmp_res.img_sem);
		sem_destroy(&bmp_res.img_wait_stop);
	}
	else{
		bmp_res.is_thread_run=1;
            printf("Create image thread right!");
		}
}
int start_bmp()
	{
	int ret;
	
	imgtrd_info("~~~~~IMAGE Thread Create~~~~~~\n");
	
	if(bmp_res.is_thread_run==1){
		imgtrd_info("IMAGE Thread Had Been Created");
		return 0;
	}

	if(sem_init(&bmp_res.img_wait_stop,0,0)==-1){
		sem_destroy(&bmp_res.img_wait_stop);
		imgtrd_info("Sem init error");
		goto THREAD_BMP_ERROR;
	}
	//_img_sem_post(&bmp_res.img_sem);
	goto THREAD_BMP_OK;
THREAD_BMP_ERROR:

	return -1;
	
THREAD_BMP_OK:
	return 0;
}
int _img_reset_img_res(struct img_dec_res_s *res)
{
	imgtrd_info("Call ImgRes reset!");
	memset(res,0,sizeof(struct img_dec_res_s));
	res->filehandle_reserve_idx = -1;
	return 0;
}


unsigned int _img_get_backgroud_color()
{
	unsigned int color=0;
	photo_background_effect_e background_effect=0;
	background_effect = get_photo_background_effect();
	switch(background_effect){
		case PHOTO_BKG_BLACK:
			color = GUI_BLACK;
			break;
		case PHOTO_BKG_WHIET:
			color = GUI_WHITE;
			break;
		case PHOTO_BKG_SILVER:
			color = GUI_SILVER;
			break;
		default:
			color = GUI_BLACK;
	}
	return color;

}

void _img_print_db_t(PHOTO_DB_T *db)
{
	int tmp_len=sizeof(PHOTO_DB_T);
	int i=0;
	unsigned char * tmp_buf=(unsigned char*)db;
	imgtrd_info("########start######");
	for(i=0;i<tmp_len;i++){
		printf("0x%2x ",tmp_buf[i]);
	}
	imgtrd_info("\n########end######");
}

#endif 

#if  1

#if 1

/**
@brief read the exif infomation which is defined by actions-micro
@param[in] filename	: the file to be opened
@param[out] db		: where to store the information
@return
	- 0	: succeed
	- -1	: failed
**/
int img_read_photo_exif(char *filename,PHOTO_DB_T *db)
{
	int rtn=0;
	FILE *fhandle=NULL;
	int rcount=0;
	long fpos=0;
	int tmp_len=sizeof(PHOTO_DB_T);
	memset(db,0,tmp_len);
	fhandle = fui_os_fopen(filename, "rb");
	if(fhandle == NULL){
		imgtrd_err("can not open %s \n",filename);
		rtn = -1;
		goto READ_EXIF_END;
	}
	memset(db,0,sizeof(PHOTO_DB_T));
	fui_os_fseek_end(fhandle,0-tmp_len);
	
	rcount = fui_os_fread(fhandle,(unsigned char*)db, sizeof(PHOTO_DB_T));
	//_img_print_db_t(db);
	if(rcount != sizeof(PHOTO_DB_T)){
		rtn = -1;
		goto READ_EXIF_END;
	}
	
	if(_img_check_photo_db_magic(db) != 0){
		rtn = -1;
		imgtrd_info("");
		goto READ_EXIF_END;
	}
	
READ_EXIF_END:	
	if(fhandle)
		fui_os_fclose(fhandle);
	imgtrd_info("rtn=%d,effect=%d",rtn,db->effect);
	return rtn;
}

//add new

//add for dlna

int img_read_photo_exif_stream(char *filename,PHOTO_DB_T *db)
{
	int rtn=0;
	FILE *fhandle=NULL;
	int rcount=0;
	long fpos=0;
	int tmp_len=sizeof(PHOTO_DB_T);
	memset(db,0,tmp_len);
	
	fhandle = _image_fopen_stream(filename, "rb");
	if(fhandle == NULL){
		imgtrd_err("can not open %s \n",filename);
		rtn = -1;
		goto READ_EXIF_END;
	}
	memset(db,0,sizeof(PHOTO_DB_T));
	_image_seek_end_stream(fhandle,0-tmp_len);
	
	rcount = _image_read_stream(fhandle,(unsigned char*)db, sizeof(PHOTO_DB_T));
	//_img_print_db_t(db);
	if(rcount != sizeof(PHOTO_DB_T)){
		rtn = -1;
		goto READ_EXIF_END;
	}
	
	if(_img_check_photo_db_magic(db) != 0){
		rtn = -1;
		imgtrd_info("");
		goto READ_EXIF_END;
	}
	
READ_EXIF_END:	
	if(fhandle)
		_image_fclose_stream(fhandle);
	imgtrd_info("rtn=%d,effect=%d",rtn,db->effect);
	return rtn;
}



int _img_dec_open_file_stream(bg_img_dec_t * param,char * file,void *type)
{
	param->img_dec_para_info.handle = _image_fopen_stream(file,"rb");
	//imgtrd_info("Filehandle=0x%x,reservefilehandle=0x%x,re_idx=%d\n",
	//	param->img_dec_para_info.handle,img_res.filehandle_reserve,img_res.filehandle_reserve_idx);
	if(param->img_dec_para_info.handle==NULL){
		printf("%s,%d:Open File=%s failed\n",__FILE__,__LINE__,file);
		return 0;
	}
	param->type = type;
	return 1;
}

int _img_dec_close_file_stream(bg_img_dec_t * param,int handleidx)
{
	//used the type 
	if(param==NULL){
		if(img_res.filehandle_reserve){
			imgtrd_info("Close File,idx=%d,Reserve fhandle=0x%x",img_res.filehandle_reserve_idx,img_res.filehandle_reserve);
			_image_fclose_stream(img_res.filehandle_reserve);
			if(img_res.filehandle_reserve_idx>=0)
				img_dec_req[img_res.filehandle_reserve_idx].img_dec_para_info.handle=NULL;
			img_res.filehandle_reserve = NULL;
			img_res.filehandle_reserve_idx = -1;
		}
	}
	else{
		if(param->is_fhandle==0){
			//imgtrd_info("Close File,ParamHandle=0x%x,Prevfhandle=0x%x,idx=%d",
			//param->img_dec_para_info.handle,img_res.filehandle_reserve,handleidx);
			if(img_res.filehandle_reserve==param->img_dec_para_info.handle||img_res.filehandle_reserve_idx== handleidx){
				imgtrd_err("Err Some Thing May Be Wrong!");
			}
			if(img_res.filehandle_reserve){
				_image_fclose_stream(img_res.filehandle_reserve);
				if(img_res.filehandle_reserve_idx>=0 &&img_res.filehandle_reserve_idx!=handleidx)
					img_dec_req[img_res.filehandle_reserve_idx].img_dec_para_info.handle=NULL;
			}
			img_res.filehandle_reserve=param->img_dec_para_info.handle;
			img_res.filehandle_reserve_idx= handleidx;
		}
	}
	return 1;
}
//end of add

//////////

/**
@brief store the exif infomation which is defined by actions-micro to the file
@param[in] filename	: the file to be opened
@param[in] cmd		: see photo_exif_setcmd_e
@param[in] param		: the value to be stored
	- if cmd==SET_EXIF_ROTATION, the param is the value of photo_rotation_e
	- if cmd==SET_EXIF_EFFECT, the param is the value of photo_effect_e
@return
	- 0	: succeed
	- -1	: failed
**/
int img_store_photo_exif(char *filename,int cmd,char param)
{
	int rtn=0;
	FILE *fhandle=NULL;
	PHOTO_DB_T tail;
	int wcount=0;
	long seek_pos=0;
	int len_db_t=sizeof(PHOTO_DB_T);
	memset(&tail,0,len_db_t);
	fhandle = fui_os_fopen(filename, "rb+");
	if(fhandle == NULL){
		imgtrd_err("can not open %s with append mode\n");
		rtn = -1;
		goto SET_EXIF_END;
	}
	imgtrd_info("filename=%s,pos=0x%x",filename,fui_os_ftell(fhandle));
	
	// check if there is a valid tail
	seek_pos = fui_os_fseek_end(fhandle,0-len_db_t);
	wcount = fui_os_fread(fhandle,(unsigned char*)&tail, len_db_t);
	imgtrd_info("cmd=%d,param=%d, datasize=%d,wcount=%d,pos=%d",cmd,param,len_db_t,wcount,seek_pos);
	//_img_print_db_t(&tail);
	if(_img_check_photo_db_magic(&tail) == 0){
		// already valid tail
		fui_os_fseek_end(fhandle,0-len_db_t);
	}
	else{
		// should write new tail
		fui_os_fseek_end(fhandle,0);
		memset(&tail,0x7f,len_db_t);
		tail.tag = 0;
		tail.version = 0x1;
		_img_set_photo_db_magic(&tail);
	}

	if(cmd == SET_EXIF_ROTATION){
		// for rotation
		tail.rotation = param;
	}
	else if(cmd == SET_EXIF_EFFECT){
		// for effect
		tail.effect= param;
	}
	else{
		// invalid
		imgtrd_err("cmd==%d is invaild\n",cmd);
		rtn = -1;
		goto SET_EXIF_END;
	}

	wcount=fui_os_fwrite(fhandle,&tail,len_db_t);
	if(wcount != len_db_t){	
		rtn = -1;
		goto SET_EXIF_END;
	}

	fui_os_fflush(fhandle);
	
SET_EXIF_END:
	if(fhandle)
		fui_os_fclose(fhandle);
	imgtrd_info("rtn=%d",rtn);
	return rtn;
}


#endif



/**
*@brief set default file operations
*
*@param[out] io_layer:	the pointer to the structure of img_io_layer_t
*@return always return 1
*/
int img_dec_set_ops_default(img_io_layer_t *io_layer)
{
	io_layer->img_fread = fui_os_fread;
	io_layer->img_fseek_set = fui_os_fseek_set;
	io_layer->img_fseek_cur = fui_os_fseek_cur;
	io_layer->img_fseek_end = fui_os_fseek_end;
	io_layer->img_ftell = fui_os_ftell;
	return 1;
}


/**
*@brief reset the command which is in the queue
*
*@param[in] newcmd		:the cmd to be executed,see the enum of req_query_cmd_e
*@param[in] index			:the index of the command which will be replaced in the queue	
*@param[in] param		: if the newcmd == BG_IMG_SET_ROTATION it is the angle to be rotated, see the enum of pe_rotate_e	
*@return NULL
*/
void img_dec_req_reset_deccmd(int newcmd,int index,void *param)
{
	bg_img_dec_t bg_img_dec;
	unsigned int bk_color=0;
	memset(&bg_img_dec,0,sizeof(bg_img_dec_t));
	memcpy(&bg_img_dec,&img_dec_req[index],sizeof(bg_img_dec_t));
	bg_img_dec.dec_cmd = newcmd;
	bg_img_dec.timestamp_ID = _img_get_timestamp_ID();
	bg_img_dec.img_dec_para_info.pic_ID = bg_img_dec.timestamp_ID;

	bk_color = _img_get_backgroud_color();
	_img_memset4((unsigned int*)bg_img_dec.img_dec_para_info.out_buf_y.buf,bk_color,\
		bg_img_dec.img_dec_para_info.out_buf_y.size/4);

	if(newcmd == BG_IMG_SET_ROTATION){
		bg_img_dec.img_dec_para_info.usr_rotated = (int)param;
	}
	
	img_dec_io_ctrl(newcmd,&bg_img_dec);
}


/**
* @brief search the queue to find the idx which is matched the condition
*
*param[in] que_cmd: the cmd to be executed,see the enum of req_query_cmd_e
*param[in] para: if the que_cmd==QUE_CMD_FILENAME , it is the pointer to file name
*return  -1:can find, others the index which is matched the condition
*/
void * img_dec_req_query(int que_cmd,void * para)
{
	int i,rtn,req_idx=-1;
	unsigned long timestamp=0;
	
	_img_sem_wait(&img_res.img_sem);
	
	for(i=0;i<DEC_REQ_NUM;i++){
		if(que_cmd==QUE_CMD_FILENAME){
			//the para is the pointer to the file name
			if(!strcmp((char*)para,img_dec_req[i].file_name)){
				imgtrd_info("idx[%d].timeID=0x%x,tmptimestamp=0x%x",i,img_dec_req[i].timestamp_ID,timestamp);
				if(img_dec_req[i].timestamp_ID>timestamp){
					timestamp = img_dec_req[i].timestamp_ID;
					req_idx = i;
				}
			}
		}
	}
	
	if(req_idx==-1){
		if(que_cmd==QUE_CMD_FILENAME){
			imgtrd_err("Can't find filename=%s",(char*)para);
		}
		//imgtrd_info("ID didn't match:can't find pic_ID=%d",req->img_dec_ret_info.pic_ID);
		rtn = -1;
	}
	else{
		rtn = req_idx;
	}
	
	_img_sem_post(&img_res.img_sem);
	
	return (void*)rtn;
}

/**
* @brief check the decoding result of the specified file
*
* @param[in] userid	: the userid which is received from fui
* @param[out] info		: store the result if the returned value is 1;
* @retval -1 			:failed,can't find the specified file
* @retval  0			: the file is decoding or just in the  queue
* @retval  1			: the file is decoded and the result will be stored in the para of info
*/
int img_get_dec_result_by_userid(int userid,IMG_DECODE_INFO_S *info)
{
	int i=0,req_idx=-1,rtn=0;
	int is_find=0;
	unsigned long timestamp=0;

	if(img_res.is_thread_run==0){
		IMG_ERR_MSG("Sorry The Image thread is not run");
		return -1;
	}
	_img_sem_wait(&img_res.img_sem);
	for(i=0;i<DEC_REQ_NUM;i++){
		if(img_dec_req[i].pic_ID == userid){
			if(i<=img_res.req_start_idx){//查询最近送进去解码的idx之前的idx
				is_find = 1;
				req_idx = i;
			}
			else{
				if(req_idx!=-1 && is_find){//找到最新送去解码的idx
					break;
				}
				else{//查询后续的idx，需要的是最后一个满足文件名匹配的idx。
					is_find=0;
					req_idx = i;
				}
			}
		}
	}
	if(req_idx==-1){
		//imgtrd_err("Can't find userid=%d\n",userid);
		rtn = 1;
	}
	else{
		if(img_dec_req[req_idx].active==REQ_DECODED){
			//imgtrd_info("Get Result req_idx=%d",req_idx);
			memcpy(info,&img_dec_req[req_idx].img_dec_ret_info,sizeof(IMG_DECODE_INFO_S));
			//_img_dec_releasehandle(req_idx);
			rtn =1;
		}
		else if(img_dec_req[req_idx].active==REQ_INVALID)
			rtn = 1;
		else
			rtn =0;
	}
	//imgtrd_info("Get Result rtn=%d,req_idx=%d,active=%d",rtn,req_idx,img_dec_req[req_idx].active);
	_img_sem_post(&img_res.img_sem);
	return rtn;
	
}

/**
* @brief check the decoding result of the specified file
*
* @param[in] file		: the filename which will be queried
* @param[out] info		: store the result if the returned value is 1;
* @retval -1 			:failed,can't find the specified file
* @retval  0			: the file is decoding or just in the  queue
* @retval  1			: the file is decoded and the result will be stored in the para of info
*/


int  img_get_dec_result(char * file,IMG_DECODE_INFO_S *info)
{
	int i=0,req_idx=-1,rtn=0;
	int is_find=0;
	unsigned long timestamp=0;
	_img_sem_wait(&img_res.img_sem);
	info->result = -2;
	for(i=0;i<DEC_REQ_NUM;i++){
		if(!strcmp(file,img_dec_req[i].file_name)){
			if(i<=img_res.req_start_idx){//查询最近送进去解码的idx之前的idx
				is_find = 1;
				req_idx = i;
			}
			else{
				if(req_idx!=-1 && is_find){//找到最新送去解码的idx
					break;
				}
				else{//查询后续的idx，需要的是最后一个满足文件名匹配的idx。
					is_find=0;
					req_idx = i;
				}
			}
		}
	}
	
	if(req_idx==-1){
		//imgtrd_err("Can't find file=%s\n",file);
		rtn = -1;
	}
	else{
		if(img_dec_req[req_idx].active==REQ_DECODED){
			//imgtrd_info("Find Index=%d",req_idx);
			memcpy(info,&img_dec_req[req_idx].img_dec_ret_info,sizeof(IMG_DECODE_INFO_S));
			//_img_dec_releasehandle(req_idx);
			rtn =1;
		}
		else if(img_dec_req[req_idx].active==REQ_INVALID){
			//imgtrd_err("error: REQ_INVALID,req_index=%d\n",req_idx);
			rtn = 2;//(-1->2):@request cancelled
		}
		else if(img_dec_req[req_idx].active==REQ_DEQUEUE){
			
			rtn =1;
		}
		else{
			rtn =0;
		}
	}
	_img_sem_post(&img_res.img_sem);
	return rtn;
}

/**
*@brief  send the command and the parameter to the image thread
*
*@param[in] cmd:	see the enum of bg_img_dec_cmd_e
*@param[in] param: if the cmd is one of the following command,it is a pointer to the structure of bg_img_dec_t,otherwise it is a NULL
*		BG_IMG_DECODE,
*		BG_IMG_ZOOM_IN,
*		BG_IMG_ZOOM_OUT,
*		BG_IMG_ZOOM_RESET,		
*		BG_IMG_MOVE_LEFT,
*		BG_IMG_MOVE_RIGHT,
*		BG_IMG_MOVE_UP,
*		BG_IMG_MOVE_DOWN,
*		BG_IMG_SET_ROTATION,
*		BG_IMG_ROT_NONE,
*		BG_IMG_ROT_LEFT_90,
*		BG_IMG_ROT_180,
*		BG_IMG_ROT_RIGHT_90
*		
*@retval 0: success
*@retval -1: error
*/
int img_dec_io_ctrl(unsigned int cmd, void *param)
{
	int rtn=0;
	bg_img_dec_t bg_img;
	imgtrd_info("Call IO Contral cmd=%d",cmd);
	switch(cmd){
		case BG_IMG_DECODE:

		case BG_IMG_ZOOM_IN:
		case BG_IMG_ZOOM_OUT:
		case BG_IMG_ZOOM_RESET:
			
		case BG_IMG_MOVE_LEFT:
		case BG_IMG_MOVE_RIGHT:
		case BG_IMG_MOVE_UP:
		case BG_IMG_MOVE_DOWN:
		case BG_IMG_SET_ROTATION:
		case BG_IMG_ROT_NONE:
		case BG_IMG_ROT_LEFT_90:
		case BG_IMG_ROT_180:
		case BG_IMG_ROT_RIGHT_90:
			if(img_res.is_thread_run==0){
				IMG_ERR_MSG("####Thread is not running####");
				break;
			}
			((bg_img_dec_t *)param)->dec_cmd = cmd;
			rtn = _img_dec_req_enqueue((bg_img_dec_t *)param);
			_img_send_msg();
			if(rtn >=0)
				rtn = 0;
			break;
		case BG_IMG_DEC_CLEAR_QUEUE:
			_init_img_dec_req();
			rtn = 0;
			break;
		case BG_IMG_DEC_STOP:
			if(img_res.is_thread_run==0){
				IMG_ERR_MSG("####Thread is not running####");
				break;
			}
			if(img_dec_stop&&imghandle){
				img_dec_stop(imghandle);
			}
			else{
				imgtrd_err("Crazy Stop Error");
				break;
			}
			memset(&bg_img,0,sizeof(bg_img_dec_t));
			bg_img.dec_cmd = cmd;
			rtn = _img_dec_req_enqueue(&bg_img);
			_img_send_msg();
			if(rtn>=0){// wait the semi at the condition of  the DEC_STOP cmd was sent successfully
				_img_wait_stop();//waiting the semi which is posted from decoder thread
				_img_dec_stop(); //release the fhanle in the queue
				_img_dec_close_file(NULL,-1);
				_img_free_dec_tmpbuf();
			}
			break;
		case BG_IMG_DEC_EXIT:
		/*	if(img_res.is_thread_run==1){
				//img_res.thread_stop=1;
				img_res.is_thread_run = 0;
			}
		*/
			break;
	}
	return rtn;
}


/**
* @brief used this function to send a decoded command to the request queue, it will fill the necessary parameters before sending  command
*
* @param[in] dec_cmd		: the decoded command, see the bg_img_dec_cmd_e;
* @param[in] dec_mode[in]	: the decoded mode,see the IMG_DEC_MODE
* @param[in] para			: if the dec_cmd==BG_IMG_DEC_HANDLE, it is a pointer to the structure of img_io_layer_t,
*							else it is pointer to a file name
* @param[in] buffer		: the output buffer where the image will be decoded
* @param[in] w			: the width of image in output buffer 
* @param[in] h			: the height of image in output buffer
* @param[in] id			: the unique id which will be identify by middleware
* @return: -1 failed, 0 success
*/
int img_dec_send_cmd(bg_img_dec_cmd_e dec_cmd,IMG_DEC_MODE dec_mode,void * para,unsigned char *buffer,int w,int h,INT32S id)
{
	bg_img_dec_t img_dec_req;
	PHOTO_DB_T exif_info;
	unsigned int bk_color=0;
	int img_dec_cmd = BG_IMG_DECODE;
	memset(&img_dec_req,0,sizeof(bg_img_dec_t));
	//printf("cmd=%d,mode=%d,pic_id=%d,w=%d,h=%d\n",dec_cmd,dec_mode,id,w,h);
	bk_color = _img_get_backgroud_color();
	if(dec_cmd==BG_IMG_DEC_HANDLE){
		img_io_layer_t *io_layer = (img_io_layer_t *)para;
		img_dec_req.img_dec_para_info.handle = io_layer->handle;
		img_dec_req.is_fhandle = 1;
		img_dec_req.img_dec_para_info.thumb_browse_mode = 0;
		img_dec_req.img_dec_para_info.pip_enable = 0;
		img_dec_req.img_dec_para_info.cmd = IMG_DECODE;
		
		_img_memset4((unsigned int*)buffer,bk_color,w*h/2);
		_img_dec_set_fs_ops(&img_dec_req.img_dec_para_info,io_layer,NULL);
	}
	else{
		memset(img_dec_req.file_name,0,sizeof(img_dec_req.file_name));
		strcpy(img_dec_req.file_name,(char*)para);
		imgtrd_info("name=%s\n",img_dec_req.file_name);

		//change log:
		if(_check_file_exist(img_dec_req.file_name)!=-1){	
			
			//printf("file in local filesystem\n");
			
			/** add for photo effect and rotation**/
			if(img_read_photo_exif(img_dec_req.file_name,&exif_info)==0){
				img_dec_req.photo_effect = exif_info.effect;
				img_dec_req.photo_rotation = exif_info.rotation;
			}
			else
				img_dec_req.photo_effect = 0;
			//imgtrd_info("exif_info effect=%d",exif_info.effect);
			/** open the file **/
			if(_img_dec_open_file(&img_dec_req,(char*)para,NULL)==0)
				return -1;
			img_dec_req.is_fhandle = 0; //it is opened by img_decoder,you should close it when it is proper
			img_dec_req.dec_cmd = dec_cmd;
			if(dec_cmd==BG_IMG_DEC_PREV || dec_cmd==BG_IMG_DEC_FULL2BUF){
				if(dec_cmd==BG_IMG_DEC_PREV){
					img_dec_req.img_dec_para_info.thumb_browse_mode = 1;
				}
				else{
					img_dec_req.img_dec_para_info.thumb_browse_mode = 0;
				}
				img_dec_req.img_dec_para_info.pip_enable = 0;
				img_dec_req.img_dec_para_info.cmd = IMG_DECODE;
				_img_memset4((unsigned int*)buffer,bk_color,w*h/2);
			}
			else{
				img_dec_req.img_dec_para_info.cmd= dec_cmd;
			}
			
			/** set the default operation **/
			_img_dec_set_fs_ops(&img_dec_req.img_dec_para_info,NULL,NULL);
		}
		else{
			//printf("not local filesystem file\n");
			
			//if(img_read_photo_exif_stream(img_dec_req.file_name,&exif_info)==0){
			if(0){
				// for stream photo, do not do this.
				img_dec_req.photo_effect = exif_info.effect;
				img_dec_req.photo_rotation = exif_info.rotation;
			}
			else{
				img_dec_req.photo_effect = 0;
			}
			//imgtrd_info("exif_info effect=%d",exif_info.effect);
			/** open the file **/
			if(_img_dec_open_file_stream(&img_dec_req,(char*)para,NULL)==0){
				_img_memset4((unsigned int*)buffer,bk_color,w*h/2);
				return -1;
			}
			img_dec_req.is_fhandle = 0; //it is opened by img_decoder,you should close it when it is proper
			img_dec_req.dec_cmd = dec_cmd;
			if(dec_cmd==BG_IMG_DEC_PREV || dec_cmd==BG_IMG_DEC_FULL2BUF){
				if(dec_cmd==BG_IMG_DEC_PREV){
					img_dec_req.img_dec_para_info.thumb_browse_mode = 1;
				}
				else{
					img_dec_req.img_dec_para_info.thumb_browse_mode = 0;
				}
				img_dec_req.img_dec_para_info.pip_enable = 0;
				img_dec_req.img_dec_para_info.cmd = IMG_DECODE;
				_img_memset4((unsigned int*)buffer,bk_color,w*h/2);
			}
			else{
				img_dec_req.img_dec_para_info.cmd= dec_cmd;
			}
			
			/** set the default operation **/
			img_io_layer_t *stream_ops;
			_img_dec_set_fs_ops(&img_dec_req.img_dec_para_info,&stream_ops,NULL);
		}
//end of add
	}
	
	
	
	/** set the id,which is the unique to the middle ware */
	img_dec_req.pic_ID = id;
	img_dec_req.timestamp_ID = _img_get_timestamp_ID();
	img_dec_req.img_dec_para_info.pic_ID = img_dec_req.timestamp_ID;
	
	img_dec_req.img_dec_para_info.formate=IMG_FMT_YCBCR_4_2_2_INTERLEAVED;
	
	img_dec_req.img_dec_para_info.out_buf_y.buf =(unsigned char *)buffer;
	img_dec_req.img_dec_para_info.out_buf_y.bus_addr	=swf_heap_physic_start + ((unsigned long)buffer-swf_heap_logic_start);
	img_dec_req.img_dec_para_info.out_buf_y.size = w*h*2;
	
	img_dec_req.img_dec_para_info.tmp_buffer.buf = _img_get_dec_tmpbuf();
	if(img_dec_req.img_dec_para_info.tmp_buffer.buf==NULL){
		return -1;
	}
	img_dec_req.img_dec_para_info.tmp_buffer.bus_addr = swf_heap_physic_start + ((unsigned long)img_dec_req.img_dec_para_info.tmp_buffer.buf-swf_heap_logic_start);
	img_dec_req.img_dec_para_info.tmp_buffer.size = tmpbuf_size;
	
	img_dec_req.img_dec_para_info.is_high_prio_cmd = 0;
	img_dec_req.img_dec_para_info.output_width = w;
	img_dec_req.img_dec_para_info.output_height = h;
	img_dec_req.img_dec_para_info.lcd_width = system_get_screen_para(CMD_GET_SCREEN_WIDTH);
	img_dec_req.img_dec_para_info.lcd_height = system_get_screen_para(CMD_GET_SCREEN_HEIGHT);
	if(img_dec_req.img_dec_para_info.lcd_width < 1280){
		img_dec_req.img_dec_para_info.lcd_width = 1280;
	}
	
	if(img_dec_req.img_dec_para_info.lcd_height < 800){
		img_dec_req.img_dec_para_info.lcd_height = 800;
	}
	
	img_dec_req.img_dec_para_info.pic_dec_mode = dec_mode;
	img_dec_req.img_dec_para_info.auto_rotation_by_exif_en = get_photo_autorotation_exif_en();

	if(img_dec_req.photo_rotation!=0 && get_photo_autorotation_adhere_en()){//if the rotation adhere is enable  
		img_dec_req.img_dec_para_info.usr_rotated = img_dec_req.photo_rotation;
		img_dec_cmd = BG_IMG_SET_ROTATION;
		imgtrd_info("Photo Rotation===%d",img_dec_req.img_dec_para_info.usr_rotated);
	}
		
	
	if(panel==DIGITAL_PANEL){
		img_dec_req.img_dec_para_info.num = 1; 
		img_dec_req.img_dec_para_info.den = 1; 
	}
	else if(panel==ANALOG_PANEL){
		img_dec_req.img_dec_para_info.num = 13; 
		img_dec_req.img_dec_para_info.den = 15; 
	}

	return img_dec_io_ctrl(img_dec_cmd,(void*)&img_dec_req);
}

/**
* the thread which interact with photo middle ware
*/
void  *image_thread(void *arg)
{
	int img_dec_idx=0;
	int msg_type=0;
	int ret_get_msg;
	bg_img_dec_t tmp_req;
	int i=0;
	IMG_ERR_MSG("WA HAHA imge_Thread Start Work==%d\n",getpid());
//	img_res.is_thread_run = 1;
	while(1){
		//IMG_ERR_MSG();
		if(img_dec_get_msg!=NULL){
			memset(&tmp_req,0,sizeof(bg_img_dec_t));
			ret_get_msg = img_dec_get_msg(imghandle, (void *)(&tmp_req.img_dec_ret_info), sizeof(IMG_DECODE_INFO_S), DECODE_INFO);
			#if 0
			i++;
			if(i>=35){
				i=0;
				imgtrd_info("------Sleep!!!!!!");
			}
			#endif
			//IMG_ERR_MSG("ret_get_msg ======= %d\n",ret_get_msg);
			//IMG_ERR_MSG("img_res.is_thread_run ===== %d\n",img_res.is_thread_run);
			if(ret_get_msg==0){
				/** get a message */
				_img_save_dec_result(&tmp_req);
				imgtrd_info("IS Ap stop=%d",tmp_req.img_dec_ret_info.ap_stop);
				if(tmp_req.img_dec_ret_info.ap_stop==1){//release the file handle in the queue
					_img_post_stop();
				}
			}
			else if(ret_get_msg==-1){
				/** get message error */
			}
			else if(ret_get_msg==-2){
				/** there is not message in the queue */
				//if(img_res.thread_stop==1)
				//	break;
				_img_thread_sleep(30);
			}
		}else{
			printf("img_dec_get_msg ===== NULL");

		}
		//IMG_ERR_MSG();
	}
	
	IMG_ERR_MSG("image_thread exit!!");
//	img_res.is_thread_run = 0;
	IMG_ERR_MSG("img_res.is_thread_run ====== %d\n",img_res.is_thread_run);

	pthread_exit((void*)1);
}

/**
*@brief 	create a thread used for image decoding 
*
*@param[in] NULL
*@retval 0	:success
*@retval -1	:failed
*/
int image_thread_create()
{
	int ret;
	
	imgtrd_info("~~~~~IMAGE Thread Create~~~~~~");
	
	if(img_res.is_thread_run==1){
		IMG_ERR_MSG("IMAGE Thread Had Been Created");
		return 0;
	}
#if 1		
	if(_img_dec_open()==-1)
		goto THREAD_CREATE_ERROR;
#else
	img_dec_get_msg =NULL;
	img_dec_send_msg = NULL;
	img_dec_close = NULL;
	img_dec_open = NULL;
	imghandle=NULL;
#endif	

	if(sem_init(&img_res.img_sem,0,0)==-1){
		imgtrd_info("Sem init error");
		goto THREAD_CREATE_ERROR;
	}

	if(sem_init(&img_res.img_wait_stop,0,0)==-1){
		sem_destroy(&img_res.img_wait_stop);
		imgtrd_info("Sem init error");
		goto THREAD_CREATE_ERROR;
	}
	
	ret = pthread_create(&img_thread_id,NULL,image_thread,NULL);
	if(ret==-1){
		imgtrd_err("Create image thread error!");
		sem_destroy(&img_res.img_sem);
		sem_destroy(&img_res.img_wait_stop);
		goto THREAD_CREATE_ERROR;
	}
	
	img_res.is_thread_run = 1;
	img_res.req_start_idx = 0;
	//img_res.thread_stop = 0;
	_img_sem_post(&img_res.img_sem);
	img_dec_io_ctrl(BG_IMG_DEC_CLEAR_QUEUE,NULL);
	goto THREAD_CREATE_OK;
THREAD_CREATE_ERROR:
	_img_dec_close();
	return -1;
	
THREAD_CREATE_OK:
	return 0;
}

/**
*@brief  delete the image decoding thread 
*
*@param[in] NULL
*@return always return 0
*/
int image_thread_exit()
{
	//imgtrd_info("IMAGE THREAD EXIT!");
	if(img_res.is_thread_run==1){
		img_dec_io_ctrl(BG_IMG_DEC_STOP,NULL);
		img_dec_io_ctrl(BG_IMG_DEC_EXIT,NULL);
	}
	if(_img_dec_close()==-1)
		return 0;
	pthread_cancel(img_thread_id);
	pthread_join(img_thread_id,&img_thread_ret);
	img_res.is_thread_run = 0;
	_img_free_dec_tmpbuf();
	sem_destroy(&img_res.img_sem);
	sem_destroy(&img_res.img_wait_stop);
	_img_dec_close_file(NULL,-1);
	_img_reset_img_res(&img_res);
	return 0;
}
#endif

