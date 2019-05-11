#include "album.h"
#include "stdio.h"
#include "string.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "act_media_info.h"
#include "fcntl.h"
#include "sys_resource.h"

#define db_info(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define db_err(fmt,arg...) printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

#define IDX_TABLE_LEN  16
#define INFO_BUFFER_LEN 256

#define CACHE_CHECK 0
#define CACHE_UPDATE 1
#define CACHE_FLUSH	  2
#define DB_CACHE_IDXBUF_LEN 16

#define DB_SUPPORT_MAX_FILE_NUM 	3000
#define DB_SUPPORT_MAX_DIR_NUM 	300

enum info_mode_e{
	//file basic
	INFO_GET_FILE_PATH,
	INFO_GET_FILE_NAME,
	INFO_GET_FILE_TIME,
	INFO_GET_FILE_SIZE,

	//photo
	INFO_GET_EXIF_WIDTH,
	INFO_GET_EXIF_HEIGHT,
	INFO_GET_EXIF_EXPTIME,

	//music
	INFO_GET_ID3_ARTISTS,
	INFO_GET_ID3_ALBUM,
	INFO_GET_ID3_GENRES,

	//user tag
	INFO_GET_USER_TAG1,
};

enum thread_status_e{
	THREAD_DB_CREATEING,
	THREAD_DB_CREATE_OK,
	THREAD_DB_CREATE_FAILED,
};

enum modify_mode_e{
	ML_MODIFY_CHANGE_TO=1,
	ML_MODIDY_SET_BITS,
	ML_MODIFY_CLR_BITS,
};

enum db_cache_value_type_e{
	TYPE_VALUE_UNKNOW,
	TYPE_VALUE_STRING,
	TYPE_VALUE_INT,
};

enum db_cache_action_type_e{
	ACTION_INVAID,
	ACTION_MODIFY,
	ACTION_DEL,
};


typedef struct time_info{
	unsigned int year;
	unsigned int month;
	unsigned int	day;
	unsigned int	hour;
	unsigned int	min;
	unsigned int	sec;
}time_info_t;

typedef struct album_info_s{	
	pthread_t thread_id; 				///< thread id which had been created
	int thread_status;				///< thread status
	enum storage_type s_type;		///< stored the storage type which is get from createdb
	medialib_t lasted_db_handle;		///< stored the lasting db handle which is returned from opendb
	struct medialib_info *ml_info;		///< store the information which is get from createdb,it will be free at correct time
}album_info_t;

typedef struct file_cache_s{
	medialib_view_t viewhandle;
	int fileidx;
	unsigned char infotype;
	////infos to be saved///
	char filepath[FULL_PATH_LEN];
	struct stat fileinfo;
	struct user_info usertag;
	union{
		IMG_INFO_S photo_info;  			
		IMG_INFO_USR_TAG_S photo_tag_info;
	};
	//
	M_INFO_INPUT_S  md_info;
}file_cache_t;

typedef struct db_cache_s{
	int action_type;				///< which action to be done, ACTION_MODIFY etc
	medialib_view_t viewhandle;  	///< the viewhandle
	int metadata;				///< the metadata
	modify_mode_t mode;			///< the modify mode 
	unsigned int * idxbuf;			///< the head of the idx buffer
	int idxbuf_len;				///< the length of idx buffer 
	int cur_idx;					///< the idx can be used in the idx buffer
	void* value;					///< the value to be change to, if the value_type==TYPE_VALUE_STRING, it is a buffer head, else it is an real value
	int value_type;				///< value type, TYPE_VALUE_UNKNOW etc
	int value_len;				///< if the value type ==	TYPE_VALUE_STRING, it is the length of the buffer used to store value		
}db_cache_t;


/////add for db storing in sdram////
enum sdram_db_cmp_e{
	DB_SDRAM_DB_CMP_SWFNAME,
	DB_SDRAM_DB_CMP_SDRAMNAME,
};

struct sdram_db_info_s{
	char*swf_db_name;
	struct sdram_dbname_s *sdram_db_name;
};

struct sdram_db_info_s sdram_db_info[SDRAM_DB_NUM]={
	{.swf_db_name=NULL,.sdram_db_name=NULL},
	{.swf_db_name=NULL,.sdram_db_name=NULL},
	{.swf_db_name=NULL,.sdram_db_name=NULL}
};

#define SDRAM_DB_SIZE (0x300000)    ///< size is 3M
static int  _db_get_storage_type(char* dbname);
static int _db_fill_medialib_info_name(char *db_name,struct medialib_info *medialib_info);
static void _db_free_medialib_info_name(struct medialib_info *medialib_info,int need_free_name);
//////////////////////

db_cache_t db_cache_info={
	.action_type= ACTION_INVAID,
	.viewhandle = NULL,
	.metadata = 0,
	.mode = 0,
	.idxbuf = NULL,
	.idxbuf_len = 0,
	.cur_idx = 0,
	.value = NULL,
	.value_len = 0,
	.value_type = 0	
};


album_info_t album_info={
	.thread_id =0,
	.thread_status = THREAD_DB_CREATEING,
	.s_type =VRAM_STORAGE,
	.lasted_db_handle=NULL,
	.ml_info=NULL,
};

file_cache_t fileinfo_cache={
	.viewhandle = 0,
	.fileidx=-1,
	.infotype=INFO_TYPE_INVAILD
};

char info_buffer[INFO_BUFFER_LEN];


////**********
static struct timeval start_time;
static struct timeval end_time;

///////////////
/******************extern******************/

/******IO Interface*******/
extern void *fui_os_fopen(char *path, char *mode);
extern int fui_os_fclose(void *fp);
extern long fui_os_fread(void *fp, unsigned char *buf, unsigned long nbytes);
extern long fui_os_fwrite(void *fp, unsigned char *ptr, unsigned long nbytes);
extern long fui_os_fseek_set(void *fp, long offset);
extern long fui_os_fseek_cur(void *fp, long offset);
extern long fui_os_fseek_end(void *fp, long offset);
extern long fui_os_ftell(void *fp);
extern int fui_os_fflush(void *fp);
extern void *fui_os_malloc(int size);
extern void fui_os_free(void * pfree);

/****Function Definition*******/
static unsigned int _map_attr_as2ml(unsigned int attrmask,enum minfo_type *mtype,char auto_add);
static int _free_db_info(int need_free_name);
void _ml_print_db_info(struct medialib_info *info);
void _ml_print_db_cache(db_cache_t *cache_info);
#if 1

#if 1
static int _medialib_check_photo_db_magic(PHOTO_DB_T *db)
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

static int _medialib_set_photo_db_magic(PHOTO_DB_T *db)
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
@brief read the tag infomation which is defined by actions-micro
@param[in] filename	: the file to be opened
@return
	- 0	: have not tag, just set default value 0
	- others : the tag value which had been stored
**/
static unsigned int medialib_read_photo_tag(char *filename)
{
	int rtn=0;
	FILE *fhandle=NULL;
	PHOTO_DB_T db;
	int rcount=0;
	long fpos=0;
	int tmp_len=sizeof(PHOTO_DB_T);
	memset(&db,0,tmp_len);
	fhandle = fui_os_fopen(filename, "rb");
	if(fhandle == NULL){
		db_err("can not open %s \n",filename);
		rtn = 0;
		goto READ_EXIF_END;
	}
	memset(&db,0,tmp_len);
	fui_os_fseek_end(fhandle,0-tmp_len);
	
	rcount = fui_os_fread(fhandle,(unsigned char*)&db, tmp_len);
	if(rcount != sizeof(PHOTO_DB_T)){
		rtn = 0;
		db_err("Read Tag Error!");
		goto READ_EXIF_END;
	}
	
	if(_medialib_check_photo_db_magic(&db) != 0){
		rtn = 0;
		goto READ_EXIF_END;
	}

	rtn = db.tag;
	
READ_EXIF_END:	
	if(fhandle)
		fui_os_fclose(fhandle);
	db_info("Tag Value====0x%x",rtn);
	return rtn;
}
 

/**
@brief store the exif infomation which is defined by actions-micro to the file
@param[in] filename	: the file to be opened
@param[in] tagvalue	: the tagvalue to be stored
@return
	- 0	: succeed
	- -1	: failed
**/
static int medialib_store_photo_tag(char *filename,unsigned int tagvalue)
{
	int rtn=0;
	FILE *fhandle=NULL;
	PHOTO_DB_T tail;
	int wcount=0;
	int len_db_t=sizeof(PHOTO_DB_T);
	fhandle = fui_os_fopen(filename, "ab");
	if(fhandle == NULL){
		db_err("can not open %s with append mode\n");
		rtn = -1;
		goto SET_EXIF_END;
	}
	// check if there is a valid tail
	fui_os_fseek_end(fhandle,0-len_db_t);
	wcount = fui_os_fread(fhandle,(unsigned char*)&tail, len_db_t);

	if(_medialib_check_photo_db_magic(&tail) == 0){
		// already valid tail
		fui_os_fseek_end(fhandle,0-len_db_t);
	}
	else{
		// should write new tail
		fui_os_fseek_end(fhandle,0);
		memset(&tail,0x7f,len_db_t);
		tail.version = 0x1;
		_medialib_set_photo_db_magic(&tail);
	}
	
	tail.tag = tagvalue;
	
	wcount=fui_os_fwrite(fhandle,(unsigned char*)&tail,(unsigned long)len_db_t);
	if(wcount != len_db_t){	
		rtn = -1;
		goto SET_EXIF_END;
	}

	fui_os_fflush(fhandle);
	
	db_info("Attach Tag to the file tagvalue=0x%x",tagvalue);
SET_EXIF_END:
	if(fhandle)
		fui_os_fclose(fhandle);
	return rtn;
}


#endif


/**
@brief check whether it is necessay to make the DB dirty when DPF is disconnected with PC
**/
static char _check_nand_db_dirty(int mediatype){
	return 0;
}
#endif

static unsigned int
_mk_exif_time(const unsigned int year0, const unsigned int mon0,
       const unsigned int day, const unsigned int hour,
       const unsigned int min, const unsigned int sec)
{
	unsigned int mon = mon0, year = year0;

	/* 1..12 -> 11,12,1..10 */
	if (0 >= (int) (mon -= 2)) {
		mon += 12;	/* Puts Feb last since it has leap day */
		year -= 1;
	}

	return ((((unsigned long)
		  (year/4 - year/100 + year/400 + 367*mon/12 + day) +
		  year*365 - 719499
	    )*24 + hour /* now have hours */
	  )*60 + min /* now have minutes */
	)*60 + sec; /* finally seconds */
}

static void time_info_2_tm(time_info_t *timeinfo, struct tm *time_tm)
{
	memset(time_tm,0,sizeof(struct tm));
	time_tm->tm_year = timeinfo->year-1990;
	time_tm->tm_mon = timeinfo->month-1;
	time_tm->tm_mday = timeinfo->day;
	time_tm->tm_hour = timeinfo->hour;
	time_tm->tm_min = timeinfo->min;
	time_tm->tm_sec = timeinfo->sec;
}

static  int  str2num(const char* str){
	char isminus=0,startidx=0;
	int c,i,num=0;
	int strlength=(int)strlen(str);
	if(str[0]=='-'){
		isminus=1;
		startidx = 1;
	}
	else{
		isminus=0;
		startidx = 0;

	}
	for(i=startidx;i<strlength;i++){
		c = str[i];
		if(c=='.')
			break;
		if(c<48||c>57)
			return 0;
		num = num*10+ (c-48);
	}
	if(isminus)
		num = 0-num;
	return num;
}

static char str2time(char *str,time_info_t *ptime){ //20101231
	char tmp[5]="";
	int strlength=strlen(str);
	memset(ptime,0,sizeof(time_info_t));
	if(strlength<4){
		db_err("Time Format is Error");
		return -1;
	}
	memcpy(tmp,str,4);
	ptime->year = str2num(tmp);

	if(strlength>=6){
		memset(tmp,0,5);
		memcpy(tmp,str+4,2);
		ptime->month = str2num(tmp);
	}
	if(strlength>=8){
		memset(tmp,0,5);
		memcpy(tmp,str+6,2);
		ptime->day = str2num(tmp);
	}
	if(strlength>=10){
		memset(tmp,0,5);
		memcpy(tmp,str+8,2);
		ptime->hour = str2num(tmp);
	}
	if(strlength>=12){
		memset(tmp,0,5);
		memcpy(tmp,str+10,2);
		ptime->min = str2num(tmp);
	}
	if(strlength>=14){
		memset(tmp,0,5);
		memcpy(tmp,str+12,2);
		ptime->sec= str2num(tmp);
	}
	return 0;
}


static int _init_db_info(char *rootpath,char*file_format,char*db_name,int attrmask,int mtype){
	int ret=-1;
	int str_len=0;
	if(album_info.ml_info==NULL){
		album_info.ml_info = SWF_Malloc(sizeof(struct medialib_info));
		if(album_info.ml_info==NULL){
			db_err("Malloc Space Failed!");
			ret = -1;
			goto INIT_DB_INFO_ERROR;
		}
		memset(album_info.ml_info,0,sizeof(struct medialib_info));
	}
	album_info.ml_info->max_support_dir_num = DB_SUPPORT_MAX_DIR_NUM;
	album_info.ml_info->max_support_file_num = DB_SUPPORT_MAX_FILE_NUM;
	album_info.ml_info->s_type = _db_get_storage_type(db_name);
	//album_info.ml_info->create_should_stop = 0;

	str_len = strlen(rootpath);
	album_info.ml_info->rootpath = SWF_Malloc(str_len+1);
	if(album_info.ml_info->rootpath==NULL){
		db_err("Malloc rootpath Failed!");
		ret = -1;
		goto INIT_DB_INFO_ERROR;
	}
	memset(album_info.ml_info->rootpath,0,str_len+1);
	memcpy(album_info.ml_info->rootpath,rootpath,str_len);

	str_len = strlen(file_format);
	album_info.ml_info->file_format= SWF_Malloc(str_len+1);
	if(album_info.ml_info->file_format==NULL){
		db_err("Malloc rootpath Failed!");
		ret = -1;
		goto INIT_DB_INFO_ERROR;
	}
	memset(album_info.ml_info->file_format,0,str_len+1);
	memcpy(album_info.ml_info->file_format,file_format,str_len);


	if(_db_fill_medialib_info_name(db_name,album_info.ml_info)==-1){
		ret = -1;
		goto INIT_DB_INFO_ERROR;
	}
		


	album_info.ml_info->attr_mask = _map_attr_as2ml((unsigned int)attrmask,&(album_info.ml_info->m_type),1);
	if(album_info.ml_info->m_type!=mtype){
		db_err("Crazy:the m_type is not match the attrs,check it ");
		ret = -1;
		goto INIT_DB_INFO_ERROR;
	}

	_ml_print_db_info(album_info.ml_info);
	ret = 0;
	
INIT_DB_INFO_ERROR:
	if(ret==-1){
		_free_db_info(1);
	}
	return ret;
}

static unsigned int _map_attr_as2ml(unsigned int attrmask,enum minfo_type *mtype,char auto_add)
{
	unsigned int ret=0;
	char mediatype=-1;
	//basic attr//
	if(attrmask&ML_ATTR_FILE_PATH)
		ret = ret | DB_FILE_PATH;
	else{
		if(auto_add){
			db_err("Sorry FILE_PATH must be contained, it will be added automatically");
			ret = ret | DB_FILE_PATH;
		}
	}
	if(attrmask&ML_ATTR_FILE_NAME)
		ret = ret | DB_FILE_NAME;
	else{
		if(auto_add){
			db_err("Sorry FILE_NAME must be contained, it will be added automatically");
			ret = ret | DB_FILE_NAME;
		}
	}
	if(attrmask&ML_ATTR_FILE_SIZE)
		ret = ret | DB_FILE_SIZE;

	if(attrmask&ML_ATTR_FILE_TIME)
		ret = ret | DB_FILE_MTIME;

	
	////photo attr//
	if(attrmask&ML_ATTR_EXIF_TIME){
		ret = ret | DB_EXIF_TIME;
		mediatype = 0;
	}
	if(attrmask&ML_ATTR_EXIF_HEIGHT){
		ret = ret | DB_EXIF_HEIGHT;
		mediatype = MINFO_TYPE_PHOTO;
	}
	if(attrmask&ML_ATTR_EXIF_WIDTH){
		ret = ret | DB_EXIF_WIDTH;
		mediatype = MINFO_TYPE_PHOTO;
	}

	///music attr///
	if(attrmask&ML_ATTR_ID3_ARTISTS){
		ret = ret | DB_ID3_ARTISTS;
		mediatype = MINFO_TYPE_MUSIC;
	}
	if(attrmask&ML_ATTR_ID3_ALBUM){
		ret = ret | DB_ID3_ALBUM;
		mediatype = MINFO_TYPE_MUSIC;
	}
	if(attrmask&ML_ATTR_ID3_GENRES){
		ret = ret | DB_ID3_GENRES;
		mediatype = MINFO_TYPE_MUSIC;
	}
	if(attrmask&ML_ATTR_ID3_COMPOSER){
		ret = ret | DB_ID3_COMPOSER;
		mediatype = MINFO_TYPE_MUSIC;
	}
	if(attrmask&ML_ATTR_ID3_TIME){
		ret = ret | DB_ID3_TIME;
		mediatype = MINFO_TYPE_MUSIC;
	}



	//user tag//
	if(attrmask&ML_ATTR_USER_TAG1)
		ret = ret | DB_USER_TAG1;
	if(attrmask&ML_ATTR_USER_TAG2)
		ret = ret | DB_USER_TAG2;
	if(attrmask&ML_ATTR_USER_TAG3)
		ret = ret | DB_USER_TAG3;
	if(attrmask&ML_ATTR_USER_TAG4)
		ret = ret | DB_USER_TAG4;
	if(attrmask&ML_ATTR_USER_TAG5)
		ret = ret | DB_USER_TAG5;

	if(mediatype ==-1){
		if(mtype!=NULL){
			db_err("Crazy the minfo_type is undefined, set it as MINFO_TYPE_PHOTO");
			*mtype = MINFO_TYPE_PHOTO;
		}
	}
	else{
		if(mtype!=NULL)
			*mtype = mediatype;
	}
		
	return ret ;
}

static void _start_point_time()
{
	if(gettimeofday(&start_time,NULL)==-1){
		start_time.tv_sec = 0;
		start_time.tv_usec = 0;
	}
}

static void  _end_point_time()
{
	if(gettimeofday(&end_time,NULL)==0){
		if(start_time.tv_sec ==0 && start_time.tv_usec==0)
			return;
		else{
			db_info("Last time start_time sec=0x%x, usec=0x%x, end_time sec=0x%x,usec=0x%x",
				start_time.tv_sec,start_time.tv_usec,end_time.tv_sec,end_time.tv_usec);
		}
	}
}


#ifdef MODULE_CONFIG_DB_ALBUM
void *_db_build_thread(void *arg){
	int ret;

	struct medialib_info *ml_info;
	ml_info = (struct medialib_info*)arg;
	album_info.thread_status=THREAD_DB_CREATEING;

	_start_point_time();
	ret = medialib_create(ml_info);
	if(ret==0)
		album_info.thread_status = THREAD_DB_CREATE_OK;
	else
		album_info.thread_status = THREAD_DB_CREATE_FAILED;

	db_info("Haha CreateDB return status=%d",album_info.thread_status);
	_end_point_time();
	_free_db_info(0);
	
	pthread_exit(0);
	return NULL;
}

#else
void *_db_build_thread(void *arg){
	return NULL;
}

#endif

int _change_value(int metadata,char * value,void *value_real){
	int type = -1;
	switch(metadata){
		case ML_ATTR_FILE_NAME:
		case ML_ATTR_FILE_PATH:
			type = 0;
			break;
		case ML_ATTR_ID3_ARTISTS:
		case ML_ATTR_ID3_ALBUM:
		case ML_ATTR_ID3_GENRES:
		case ML_ATTR_ID3_COMPOSER:
		case ML_ATTR_ID3_TIME:
			db_info("The Value is String which is formatted as Unicode");
			type=4;
			break;
		case ML_ATTR_FILE_SIZE:
		case ML_ATTR_EXIF_HEIGHT:
		case ML_ATTR_EXIF_WIDTH:
			*(unsigned short*)value_real = (unsigned short)str2num(value);
			type = 1;
			break;
		case ML_ATTR_FILE_TIME:
		case ML_ATTR_EXIF_TIME:
			{
				time_info_t tmptimeinfo;
				if(str2time(value,&tmptimeinfo)==-1)
					*(unsigned int*)value_real = 0;
				else{
					if(metadata == ML_ATTR_EXIF_TIME)
						*(unsigned int*)value_real = _mk_exif_time(tmptimeinfo.year,tmptimeinfo.month,tmptimeinfo.day,\
															tmptimeinfo.hour,tmptimeinfo.min,tmptimeinfo.day);
					else{
						struct tm filetime_tm;
						time_info_2_tm(&tmptimeinfo,&filetime_tm);
						*(unsigned int*)value_real = (unsigned int)mktime(&filetime_tm);
					}
				}
				type = 2;
			}
			break;
		case ML_ATTR_USER_TAG1:
		case ML_ATTR_USER_TAG2:
		case ML_ATTR_USER_TAG3:
		case ML_ATTR_USER_TAG4:
		case ML_ATTR_USER_TAG5:
			*(unsigned int*)value_real = (unsigned int)str2num(value);
			type = 3;
			break;
		default:
			db_err("Crazy: the type of metadata is not supported");
			type = -1;
			break;
	}
	return type;
}

static int _file_info_cache(unsigned char cmd,medialib_view_t viewhandle,int idx,int infotype){
	if(cmd==CACHE_CHECK){
		if(viewhandle!=fileinfo_cache.viewhandle || idx!= fileinfo_cache.fileidx || infotype!=fileinfo_cache.infotype){
			return 0;
		}
		else
			return 1;
	}
	else if(cmd == CACHE_UPDATE){
		fileinfo_cache.viewhandle = viewhandle;
		fileinfo_cache.fileidx=idx;
		fileinfo_cache.infotype =infotype;
		return 1;
	}
	else if(cmd==CACHE_FLUSH){
		fileinfo_cache.viewhandle = 0;
		fileinfo_cache.fileidx=-1;
		fileinfo_cache.infotype =INFO_TYPE_INVAILD;
		return 1;
	}
	else{
		db_err("Sorry Not Support CMD==%d",cmd);
		return 0;
	}
}

static void _init_media_info_input(M_INFO_INPUT_S *input){
	input->p_read = (void*)fui_os_fread;
	input->p_write = (void*)fui_os_fwrite;
	input->p_seek_cur = (void*)fui_os_fseek_cur;
	input->p_seek_set = (void*)fui_os_fseek_set;
	input->p_seek_end = (void*)fui_os_fseek_end;
	input->p_tell = (void*)fui_os_ftell;
	input->p_free = (void*)fui_os_free;
	input->p_malloc = (void*)fui_os_malloc;
}
static int _get_filename(char* fullpath,char *namebuf){
	char *tmp=fullpath;
	tmp=(strrchr(fullpath,'/')+1);
	strcpy(namebuf,tmp);
	//printf("fullpath==%s\n",fullpath);
	//printf("filename==%s\n",namebuf);
	return 1;
}

static int _free_db_info(int need_free_name)
{
	if(album_info.ml_info){
		if(album_info.ml_info->rootpath)
			SWF_Free(album_info.ml_info->rootpath);
		if(album_info.ml_info->file_format)
			SWF_Free(album_info.ml_info->file_format);
		_db_free_medialib_info_name(album_info.ml_info,need_free_name);
		SWF_Free(album_info.ml_info);
		album_info.ml_info=NULL;
	}
	return 0;
}

/**
@brief init the db_cache
**/
static int _db_cache_init(db_cache_t *cache_info)
{	
	if(cache_info->viewhandle==NULL){
		cache_info->cur_idx = 0;
		cache_info->idxbuf_len = DB_CACHE_IDXBUF_LEN;
		cache_info->idxbuf=(unsigned int*)SWF_Malloc(sizeof(unsigned int)*cache_info->idxbuf_len);
		if(cache_info->idxbuf==NULL){
			db_err("Sorry Malloc Failed!");
			return -1;
		}
		cache_info->value=NULL;
		cache_info->value_type=TYPE_VALUE_UNKNOW;
		cache_info->value_len = 0;
		cache_info->mode = 0;
		cache_info->metadata = 0;
		cache_info->action_type = ACTION_INVAID;
	}
	return 0;
}


/**
@brief release the resource which the db_cache occupied
**/
static int _db_cache_release(db_cache_t *cache_info)
{
	if(cache_info->viewhandle){
		if(cache_info->idxbuf){
			SWF_Free(cache_info->idxbuf);
			cache_info->idxbuf=NULL;
			cache_info->idxbuf_len = 0;
			cache_info->cur_idx = 0;
		}
		if(cache_info->value_type==TYPE_VALUE_STRING){
			if(cache_info->value!=NULL){
				SWF_Free(cache_info->value);
			}
			cache_info->value_type = TYPE_VALUE_UNKNOW;
			cache_info->value = NULL;
			cache_info->value_len = 0;
		}
		else{
			cache_info->value=NULL;
			cache_info->value_type=TYPE_VALUE_UNKNOW;
			cache_info->value_len = 0;
		}	
		cache_info->mode =0;
		cache_info->metadata = 0;
		cache_info->action_type = ACTION_INVAID;
		cache_info->viewhandle = NULL;

	}
	return 0;
}

/**
@brief realloc the idx buffer
**/
static int _db_realloc_idxbuf(db_cache_t *cache_info)
{
	unsigned int * tmpbuf;
	int tmpbuf_len = cache_info->idxbuf_len*2*sizeof(unsigned int);
	tmpbuf = SWF_Malloc(tmpbuf_len);
	if(tmpbuf==NULL){
		db_info("Re Alloc Failed!");
		return -1;
	}
	else{
		memcpy(tmpbuf,cache_info->idxbuf,cache_info->idxbuf_len*sizeof(unsigned int));
		SWF_Free(cache_info->idxbuf);
		cache_info->idxbuf = tmpbuf;
		cache_info->idxbuf_len = cache_info->idxbuf_len*2;
	}
	return 0;	
}

/**
@brief add the item into specified db_cache
@warning normally, the view, metadata, value, value_len,valuetype, mode, action_type will not be changed \n
	 in the period of a sequence of calling this function
**/
static int _db_cache_add_item(medialib_view_t view,int metadata,int idx,void* value,int value_len,
								int valuetype,modify_mode_t mode,db_cache_t *cache_info,int action_type)
{
	int rtn=0;
	if(_db_cache_init(cache_info)==-1){
		rtn = -1;
		goto ADD_ITEM_END;
	}

	cache_info->viewhandle = view;
	cache_info->action_type = action_type;
	if(action_type==ACTION_MODIFY){
		cache_info->metadata = metadata;
		cache_info->mode= mode;
		cache_info->value_type = valuetype;
		if(cache_info->value_type==TYPE_VALUE_STRING){
			if(cache_info->value==NULL){
				cache_info->value = SWF_Malloc(value_len+1);
				if(cache_info->value==NULL){
					cache_info->value_len = 0;
					rtn = -1;
					goto ADD_ITEM_END;
				}
				cache_info->value_len = value_len+1;
			}
		}
		else if(cache_info->value_type==TYPE_VALUE_INT){
			if(cache_info->value_len!=0){ // if it is an int, the value_len must be 0, must free the value when the value_len is not equal to 0
				//if the value_type is not changed, this situation will not happen in the period of a sequence of calling _db_cache_add_item
				db_err("Carzy: the value type is changed!");
				SWF_Free(cache_info->value);
				cache_info->value_len = 0;
			}
			db_info("");
			cache_info->value = (void*)(*(unsigned int*)value);
		}
		else
			db_err("Value type is Error!");
	}
	else{
		
	}
	db_info("");
	if(cache_info->idxbuf!=NULL){
		if(cache_info->cur_idx>=cache_info->idxbuf_len){
			if(_db_realloc_idxbuf(cache_info)==-1){
				rtn = -1;
				goto ADD_ITEM_END;
			}
		}	
		*(cache_info->idxbuf+cache_info->cur_idx) = idx;
		cache_info->cur_idx++;
	}
	db_info("");
ADD_ITEM_END:
	return rtn;
}


/**
@brief do the action according to the action_type and release the resource
**/
static int _db_cache_do_action(db_cache_t *cache_info)
{
	int rtn=0;
	if(cache_info->viewhandle!=NULL){
		_ml_print_db_cache(cache_info);
		if(cache_info->action_type==ACTION_MODIFY){
			 rtn =medialib_modify(cache_info->viewhandle,cache_info->metadata,cache_info->idxbuf,cache_info->cur_idx,\
			 	&(cache_info->value),cache_info->mode);
		}
		else if(cache_info->action_type==ACTION_DEL){
			rtn = medialib_delete(cache_info->viewhandle,cache_info->idxbuf,cache_info->cur_idx);
		}
		else{
			db_err("Sorry the Action type is Error!");
		}
		db_info("");
		_db_cache_release(cache_info);
	}
	if(rtn==-1)
		db_err("DO ACTION ERROR!");
	return rtn;
}


static int _db_change_buf_format(char *srcbuf,int *srclen,int srcformat,char *destbuf,int *destlen)
{
	int is_change_ok=0;
	charsetchg_info_t change_info;
	change_info.src_charset = srcformat;
	change_info.dest_charset = LAN_UTF16LE;
	change_info.inputbuf = srcbuf;
	change_info.inputbuflen = (size_t*)srclen;
	change_info.outputbuf = destbuf;
	change_info.outputbuflen = (size_t*)destlen;
	//print_test(StrTempBuf,TmpLineInfo.len,16);
	is_change_ok = change_chardec(&change_info);
	return is_change_ok;
}

int  _db_get_storage_type(char* dbname)
{
	if(dbname){
		if(dbname[0]=='s' && dbname[1]=='d' && dbname[2]=='r' && dbname[3]=='a' && dbname[4]=='m'){
			return SDRAM_STORAGE;
		}
		else
			return VRAM_STORAGE;
	}
	else
		return VRAM_STORAGE;
}

static struct sdram_dbname_s * __get_sdram_db_name()
{
	struct sdram_dbname_s * sdram_dbname=(struct sdram_dbname_s *)SWF_Malloc(sizeof(struct sdram_dbname_s));
	if(sdram_dbname){
		sdram_dbname->start_addr = (unsigned int)SWF_Malloc(SDRAM_DB_SIZE);
		sdram_dbname->mem_size = SDRAM_DB_SIZE;
		db_info("SDRAM DB Addr=0x%x,Size=0x%x",sdram_dbname->start_addr,sdram_dbname->mem_size);
		if(sdram_dbname->start_addr==0){
			SWF_Free((void*)sdram_dbname);
			sdram_dbname = NULL;
		}
		db_info("SDRAM DB MAlloc Handle==0x%x",sdram_dbname);
	}
	return sdram_dbname;
}

static void __free_sdram_db_name(struct sdram_dbname_s * sdram_dbname)
{
	if(sdram_dbname){
		db_info("SDRAM DB Addr FREE=0x%x,Size=0x%x",sdram_dbname->start_addr,sdram_dbname->mem_size);
		if(sdram_dbname->start_addr){
			SWF_Free((void*)sdram_dbname->start_addr);
			sdram_dbname->start_addr = 0;
			sdram_dbname->mem_size = 0;
		}
		SWF_Free(sdram_dbname);
	}
}

static int __get_db_sdram_info_index(void *dbname,enum sdram_db_cmp_e cmd_cmp)
{
	int i=0;
	for(i=0;i<SDRAM_DB_NUM;i++){
		if(cmd_cmp==DB_SDRAM_DB_CMP_SWFNAME){
			if(sdram_db_info[i].swf_db_name!=NULL){
				if(strcmp((char*)dbname,sdram_db_info[i].swf_db_name)==0){
					return i;
				}
			}
		}
		else{
			if(sdram_db_info[i].sdram_db_name!=NULL){
				if(sdram_db_info[i].sdram_db_name == (struct sdram_dbname_s *)dbname){
					return i;
				}
			}
		}
	}
	return -1;
}

static struct sdram_dbname_s * __swf_dbname2_sdram_dbname(char* dbname,int *index)
{
	*index = __get_db_sdram_info_index(dbname,DB_SDRAM_DB_CMP_SWFNAME);
	if((*index)>=0)
		return sdram_db_info[*index].sdram_db_name;
	else 
		return NULL;
}

static char* __sdram_dbname2_swf_dbname(struct sdram_dbname_s *dbname,int *index)
{
	*index = __get_db_sdram_info_index(dbname,DB_SDRAM_DB_CMP_SDRAMNAME);
	if((*index)>=0)
		return sdram_db_info[*index].swf_db_name;
	else
		return NULL;
}

static int __get_invaild_sdram_db_info()
{
	int i=0;
	for(i=0;i<SDRAM_DB_NUM;i++){
		if(sdram_db_info[i].swf_db_name==NULL && sdram_db_info[i].sdram_db_name==NULL)
			return i;
	}
	db_err("Can't  finde Invaild Sdram DB Info");
	return -1;
}

static int __fill_sdram_db_info(char* dbname,int index)
{
	int str_len =0;
	int rtn =0;
	if(index<SDRAM_DB_NUM){
		str_len = strlen(dbname);
		sdram_db_info[index].swf_db_name= SWF_Malloc(str_len+1);
		if(sdram_db_info[index].swf_db_name){
			memset(sdram_db_info[index].swf_db_name,0,str_len+1);
			memcpy(sdram_db_info[index].swf_db_name,dbname,str_len);
			sdram_db_info[index].sdram_db_name = __get_sdram_db_name();
			if(sdram_db_info[index].sdram_db_name==NULL){
				SWF_Free(sdram_db_info[index].swf_db_name);
				sdram_db_info[index].swf_db_name = NULL;
				rtn  = -1;
			}
		}
	}
	else
		rtn = -1;
	return rtn;
}

static int __free_sdram_db_info(char *dbname)
{
	int index=0;
	if(__swf_dbname2_sdram_dbname(dbname,&index)!=NULL){
		if(sdram_db_info[index].swf_db_name && sdram_db_info[index].sdram_db_name){
			SWF_Free(sdram_db_info[index].swf_db_name);
			__free_sdram_db_name(sdram_db_info[index].sdram_db_name);
			sdram_db_info[index].swf_db_name = NULL;
			sdram_db_info[index].sdram_db_name = NULL;
			db_info("SDRAM DB FREE======================");
		}
		else{
			db_err("Carzy DB Info Error! Something must be wrong, swf_db_name==0x%x,sdram_db_name=0x%x",
				sdram_db_info[index].swf_db_name && sdram_db_info[index].sdram_db_name);
			return -1;
		}
	}
	else{
		db_info("DB name can't be find in sdram dbname=%s",dbname);
	}
	return 0;
}

int _db_fill_medialib_info_name(char *db_name,struct medialib_info *medialib_info)
{
	int str_len=0;
	int ret = 0;
	if(db_name && medialib_info){
		if(medialib_info->s_type == VRAM_STORAGE){
			str_len = strlen(db_name);
			medialib_info->name= SWF_Malloc(str_len+1);
			if(medialib_info->name==NULL){
				db_err("Malloc rootpath Failed!");
				ret = -1;
				goto FILL_DB_NAME_END;
			}
			memset(medialib_info->name,0,str_len+1);
			memcpy(medialib_info->name,db_name,str_len);	
		}
		else if(medialib_info->s_type == SDRAM_STORAGE){
			int index=0;
			index = __get_invaild_sdram_db_info();
			if(index>=0){
				ret = __fill_sdram_db_info(db_name,index);
				if(ret==0)
					medialib_info->name = (char*)__swf_dbname2_sdram_dbname(db_name,&index);
			}
			else
				ret = -1;
		}
		else{
			db_err("Not support DB Storage Type!");
			ret  = -1;
		}
			
	}
FILL_DB_NAME_END:
	if(ret==-1)
		medialib_info->name=NULL;
	return ret;
}

void _db_free_medialib_info_name(struct medialib_info *medialib_info,int need_free_name)
{
	if(medialib_info==NULL)
		return ;
	if(medialib_info->s_type == VRAM_STORAGE){
		if(medialib_info->name){
			SWF_Free(medialib_info->name);
			medialib_info->name = NULL;
		}
	}
	else if(medialib_info->s_type == SDRAM_STORAGE){
		if(need_free_name && medialib_info->name){ ///如果在创建时失败就需要去真正free
			int index=0;
			char *swf_dbname= __sdram_dbname2_swf_dbname((struct sdram_dbname_s *)(medialib_info->name),&index);
			if(swf_dbname)
				__free_sdram_db_info(swf_dbname);
			medialib_info->name = NULL;
		}
		else  ///<简单的将name与真是的存储断开
			medialib_info->name = NULL;
		
	}
}

static void * _get_medialib_db_real_name(char *dbname)
{
	int s_type=0;
	int index=0;
	void *db_realname=NULL;
	s_type= _db_get_storage_type(dbname);
	if(s_type == VRAM_STORAGE){
		db_realname = (void*)dbname;
		db_info("DBname in vram=%s",(char*)db_realname);
	}
	else if(s_type==SDRAM_STORAGE){
		db_realname = (void*)__swf_dbname2_sdram_dbname(dbname,&index);
		db_info("DBname in sdram=0x%x",db_realname);
	}
	db_info("DB Store ini %d(0==vram,1=sdram)",s_type);
	return db_realname;
}

static void _db_destroy_sdram_db(char *dbname)
{
	int s_type=0;
	s_type= _db_get_storage_type(dbname);
	if(s_type==SDRAM_STORAGE)
		__free_sdram_db_info(dbname);
}

#if 1
void _ml_print_db_cache(db_cache_t *cache_info)
{
	int i=0;
	db_info("@@@@@@@@@@@START PRINT DB CACHE@@@@@@@@@@@");
	db_info("viewhandle=0x%x, metadata=0x%x, mode=0x%x,type=%d",cache_info->viewhandle,cache_info->metadata,cache_info->mode,cache_info->action_type);
	db_info("Valuetype=%d",cache_info->value_type);
	if(cache_info->value_type==TYPE_VALUE_STRING){
		db_info("len=%d,value=%s",cache_info->value_len,cache_info->value);
	}
	else
		db_info("value=0x%x",cache_info->value);
	for(i=0;i<cache_info->cur_idx;i++){
		printf("idx[%d]=%d ",i,*(cache_info->idxbuf+i));
	}
	db_info("\n@@@@@@@@@@@END PRINT DB CACHE@@@@@@@@@@@");
}

void _ml_print_db_info(struct medialib_info *info)
{
	db_info("attr_mask=0x%x,max_file_num=%d,max_dir_num=%d,m_type=%d,s_type=%d",info->attr_mask,info->max_support_file_num,\
		info->max_support_dir_num,info->m_type,info->s_type);
	db_info("fileformat=%s,name=0x%x,rootpath=%s",info->file_format,info->name,info->rootpath);
}

void _ml_print_view_info(struct medialib_view_info *info)
{
	db_info("dbhandle=0x%x,metadata=0x%x,value=0x%x,mode=%d,view=0x%x",info->medialib,info->db_metadata,\
		info->value,info->mode,info->view);
}
#endif

#if 1
#ifdef MODULE_CONFIG_DB_ALBUM
static int  medialib_opendb(void *handle){
	int rtn;
	medialib_t ret=NULL;
	int m_type=0;
	void *db_realname=NULL;
	char *db_name=NULL;
	int s_type=0;
	SWFEXT_FUNC_BEGIN(handle);
	
	m_type = Swfext_GetNumber();
	db_name = Swfext_GetString();

	s_type = _db_get_storage_type(db_name);
	db_info("m_type=%d,s_type=%d,db_name=%s",m_type,s_type,db_name);
	
	db_realname = _get_medialib_db_real_name(db_name);
	if(db_realname)
		ret = medialib_open(m_type,s_type,db_realname);	
	if(ret==NULL)
		rtn = 0;
	else{
		db_info("DB Handle Opened=0x%x",ret);
		album_info.lasted_db_handle = ret;
		rtn = (int)ret;
	}
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}

static int medialib_closedb(void *handle){
	int dbhandle;
	int rtn;
	SWFEXT_FUNC_BEGIN(handle);
	
	dbhandle = Swfext_GetNumber();
	
	db_info("DBhandl=0x%x will be close",dbhandle);
	if(dbhandle!=0)
		rtn = medialib_close((medialib_t)dbhandle);
	else
		rtn = 0;
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}	

static int medialib_createdb(void *handle){
	int ret=-1;
	char * rootpath=NULL;
	int attrmask=0;
	char * fileformat=NULL;
	char * dbname=NULL;
	enum minfo_type m_type;
	struct medialib_info ml_info;
	SWFEXT_FUNC_BEGIN(handle);
	
	rootpath= Swfext_GetString();
	attrmask = Swfext_GetNumber();
	fileformat = Swfext_GetString();
	dbname = Swfext_GetString();
	m_type = Swfext_GetNumber();
	

	
	db_info("rootpath=%s,attrmask=0x%x,fileformat=%s,name=%s,m_type=%d",rootpath,attrmask,fileformat,dbname,m_type);


	if(_init_db_info(rootpath,fileformat,dbname,attrmask,m_type)==-1){
		ret = -1;
		goto CREATE_DB_ERROR;
	}

	
	album_info.thread_id = 0;
	album_info.thread_status=THREAD_DB_CREATEING;
	album_info.s_type = album_info.ml_info->s_type;
	
	ret = pthread_create(&album_info.thread_id,NULL,_db_build_thread,(void*)album_info.ml_info);
	if(ret==-1){
		db_err("Create DB Build Thread error!");
		goto CREATE_DB_ERROR;
	}
	else
		ret = -2;  // create thread succeed
CREATE_DB_ERROR:	
	if(ret==-1){
		_free_db_info(1);
	}
	Swfext_PutNumber(ret);
	
	SWFEXT_FUNC_END();
}

static int medialib_disabledb(void *handle){

	SWFEXT_FUNC_BEGIN(handle);
	db_info("Sorry disable DB do nothing, Call destoryDB");
	SWFEXT_FUNC_END();
}

static int medialib_destorydb(void *handle){
	int m_type=0;
	int s_type=0;
	char *db_name=NULL;
	void *db_realname=NULL;
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);
	
	m_type = Swfext_GetNumber();
	db_name = Swfext_GetString();

	s_type = _db_get_storage_type(db_name);
	db_info("m_type=%d,s_type=%d,db_name=%s",m_type,s_type,db_name);
	if(album_info.ml_info)
		medialib_stop_create(album_info.ml_info);

	db_realname = _get_medialib_db_real_name(db_name);
	s_type = _db_get_storage_type(db_name);
	if(db_realname){
		rtn = medialib_destroy(m_type,s_type,(char*)db_realname);
		_db_destroy_sdram_db(db_name);
	}
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}

static int  medialib_updatedb(void *handle){
	int dbhandle=0;
	int rtn = 0;
	SWFEXT_FUNC_BEGIN(handle);
	dbhandle = Swfext_GetNumber();

	db_info("dbhandle=0x%x\n",dbhandle);
	_file_info_cache(CACHE_FLUSH,0,0,0);
	rtn = _db_cache_do_action(&db_cache_info);
	rtn = medialib_update((medialib_t)dbhandle);
	
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int medialib_get_thread_status(void *handle){
	SWFEXT_FUNC_BEGIN(handle);
	
	Swfext_PutNumber(album_info.thread_status);
	
	SWFEXT_FUNC_END();
}


static int medialib_open_views(void *handle){
	struct medialib_view_info viewinfo;
	medialib_view_t viewhandle=NULL;
	int dbhandle=0;
	int viewbasedon=0;
	int metadata=0,metadata_as;
	int view_mode=0;
	char *value=NULL;
	unsigned int valuereal=0;
	SWFEXT_FUNC_BEGIN(handle);
	
	dbhandle = Swfext_GetNumber();
	metadata_as = Swfext_GetNumber();
	value = Swfext_GetString();
	view_mode = Swfext_GetNumber();
	viewbasedon=Swfext_GetNumber();

	db_info("dbhanlde=0x%x,metatdata=0x%x,value=%s,viewmode=%d,baseview=0x%x\n",dbhandle,metadata_as,value,view_mode,viewbasedon);

	if(dbhandle==0){
		db_info("Crazy: Dbhandle===0 is not allowed!");
		goto OPEN_VIEWS_END;
	}
	
	viewinfo.medialib = (medialib_t)dbhandle;
	metadata = _map_attr_as2ml((unsigned int) metadata_as,NULL,0);
	viewinfo.db_metadata = metadata;
	if(view_mode==VIEW_MODE_PURE_SORT)
		viewinfo.value = (void*)0;
	else{
		int ret=0;
		ret = _change_value(metadata_as,value,&valuereal);
		if(ret==0){
			viewinfo.value = value;
		}
		else if(ret==4){
			int strsrc_len=strlen(value);
			int desbuf_len=INFO_BUFFER_LEN;
			memset(info_buffer,0,INFO_BUFFER_LEN);
			if(_db_change_buf_format(value,&strsrc_len,LAN_UTF8,info_buffer,&desbuf_len)==-1){
				db_err("Change Str Format Error!\n");
			}
			else
				viewinfo.value=info_buffer;
		}
		else if(ret==1 || ret==2 || ret==3){
			viewinfo.value = &valuereal;
		}
		db_info("Valuereal=%d\n",valuereal);	
	}
	viewinfo.mode = view_mode;
	viewinfo.view = (medialib_view_t)viewbasedon;
	
	_ml_print_view_info(&viewinfo);
	viewhandle = medialib_open_view(&viewinfo);
	
OPEN_VIEWS_END:	
	if(viewhandle==NULL){
		db_err("Open View Error!");
	}	
	db_info("ViewHandle=0x%x",viewhandle);
	Swfext_PutNumber((int)viewhandle);
	SWFEXT_FUNC_END();
}



static int medialib_close_views(void *handle){
	int viewhandle=0;
	int rtn = 0;
	SWFEXT_FUNC_BEGIN(handle);

	viewhandle = Swfext_GetNumber();
	if(viewhandle!=0)
		rtn = medialib_close_view((medialib_view_t)viewhandle);
	else{
		db_err("Close View handle is zero Do nothing, Check it!");
		rtn = 0;
	}
	
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int medialib_get_num(void *handle){
	struct db_info  dbinfo;
	int viewhandle=0;
	int numtotal=0;
	SWFEXT_FUNC_BEGIN(handle);
	
	viewhandle = Swfext_GetNumber();
	db_info("Get Num ViewHandle=0x%x",viewhandle);


	//init the resouce which may be used in get info function///
	_file_info_cache(CACHE_FLUSH,0,0,0);
	_init_media_info_input(&fileinfo_cache.md_info);
	

	//get total num in the view
	if(viewhandle==0){
		if(album_info.lasted_db_handle==NULL){
			db_err("Crazy: DB had not been opened, Check openDB");
			numtotal = -1;
			goto GET_NUM_ERROR;
		}
		if(medialib_getdb_info(album_info.lasted_db_handle,&dbinfo)==0)
			numtotal = dbinfo.file_num;
		else{
			db_err("Sorry getDB info Error");
			numtotal = -1;
		}
	}
	else
		numtotal = medialib_getnum((medialib_view_t)viewhandle);
	if(numtotal==-1){
		db_err("Sorry Get Num==-1");
	}

GET_NUM_ERROR:
	Swfext_PutNumber(numtotal);
	SWFEXT_FUNC_END();
}


static int medialib_del_items(void *handle){
	int viewhandle=0;
	unsigned int idx;
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);

	viewhandle = Swfext_GetNumber();
	idx = Swfext_GetNumber();

	db_info("ViewHandle=0x%x,fileidx=%d",viewhandle,idx);
	//rtn = medialib_delete((medialib_view_t)viewhandle,idx,1);
	if(viewhandle!=0)
		rtn = _db_cache_add_item((medialib_view_t)viewhandle,0,idx,NULL,0,0,0,&db_cache_info,ACTION_DEL);
	else
		rtn = -1;
	
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int medialib_add_items(void *handle){
	int dbhandle=0;
	int tagvalue=0;
	char * filename=NULL;
	struct user_info userinfo;
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);
	
	dbhandle = Swfext_GetNumber();
	filename = Swfext_GetString();
	tagvalue = Swfext_GetNumber();

	db_info("dbhandle=0x%x,filename=%s,tagvalue=0x%x\n",dbhandle,filename,tagvalue);

	memset((void*)&userinfo,0,sizeof(struct user_info));
	userinfo.user_tag1 = tagvalue;
	if(dbhandle!=0)
		rtn = medialib_insert((medialib_t)dbhandle,filename,&userinfo);
	else
		rtn = -1;
	
	if(rtn==-1)
		db_err("Sorry insert file=%s, failed",filename);
	
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int medialib_find_item(void *handle){
	int rtn;
	int indxfind=-1;
	unsigned int valuereal=0;
	int metadata,metadata_as;
	int viewhandle=0;
	char *value=NULL;
	search_mode_t findmode;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	viewhandle = Swfext_GetNumber();
	metadata_as = Swfext_GetNumber();
	value = Swfext_GetString();
	findmode = Swfext_GetNumber();

	//db_info("viewhandle=0x%x,metadata=%d,value=%s,findmode=%d",viewhandle,metadata_as,value,findmode);

	if(viewhandle==0){
		db_err("Crazy Viewhandle==0 is not allowed!");
		rtn = -1;
		goto FIND_ITEM_END;
	}
	
	rtn = _change_value(metadata_as,value,&valuereal);
	
	metadata = _map_attr_as2ml((unsigned int) metadata_as,NULL,0);
	if(rtn==-1)
		Swfext_PutNumber(-1);
	else{
		if(rtn==0){
			indxfind= medialib_search((medialib_view_t)viewhandle,metadata,value, findmode);
		}
		else if(rtn==4){
			int strsrc_len=strlen(value);
			int desbuf_len=INFO_BUFFER_LEN;
			memset(info_buffer,0,INFO_BUFFER_LEN);
			if(_db_change_buf_format(value,&strsrc_len,LAN_UTF8,info_buffer,&desbuf_len)==-1){
				db_err("Change Str Format Error!\n");
			}
			else
				indxfind= medialib_search((medialib_view_t)viewhandle,metadata,info_buffer, findmode);
		}
		else if(rtn==1 || rtn==2 || rtn==3){
			indxfind= medialib_search((medialib_view_t)viewhandle,metadata,&valuereal, findmode);
		}
		else
			db_err("Crazy: rtn==%d",rtn);
		Swfext_PutNumber(indxfind);
	}
	
FIND_ITEM_END:
	if(rtn==-1)
		db_err("Sorry can find data");

	db_info("Find Idx==%d\n",indxfind);
	SWFEXT_FUNC_END();
}

static int medialib_modify_items(void *handle){
	int viewhandle=0;
	int metadata = 0,metadata_as;
	int ret=0;
	char *value=NULL;
	int modify_mode=0;
	unsigned int valuereal=0;
	unsigned int idx;
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);

	viewhandle = Swfext_GetNumber();
	idx= Swfext_GetNumber();//file index
	metadata_as = Swfext_GetNumber();
	value = Swfext_GetString();
	modify_mode = Swfext_GetNumber();


	if(viewhandle==0){
		db_err("Crazy Viewhandle==0 is not allowed!");
		rtn = -1;
		goto MODIFY_ITEMS_END;
	}

	ret = _change_value(metadata_as,value,&valuereal);

	metadata = _map_attr_as2ml((unsigned int) metadata_as,NULL,0);
	
	if(ret==-1)
		Swfext_PutNumber(-1);
	else{
		if(ret == 0){
			if(modify_mode!=ML_MODIFY_CHANGE_TO)
				db_err("Sorry the metadata=0x%x, the mode need use CHANGE_TO",metadata);
			rtn = _db_cache_add_item((medialib_view_t)viewhandle,metadata,idx,value,strlen(value),TYPE_VALUE_STRING,\
							MODIFY_MODE_CHANGETO,&db_cache_info,ACTION_MODIFY);	
		}
		else if(ret==4){
			int strsrc_len=strlen(value);
			int desbuf_len=INFO_BUFFER_LEN;
			memset(info_buffer,0,INFO_BUFFER_LEN);
			if(_db_change_buf_format(value,&strsrc_len,LAN_UTF8,info_buffer,&desbuf_len)==-1){
				db_err("Change Str Format Error!\n");
			}
			else
				rtn = _db_cache_add_item((medialib_view_t)viewhandle,metadata,idx,info_buffer,strlen(info_buffer),TYPE_VALUE_STRING,\
							MODIFY_MODE_CHANGETO,&db_cache_info,ACTION_MODIFY);	
		}
		else if(ret==1 ||ret==2){
			if(modify_mode!=ML_MODIFY_CHANGE_TO)
				db_err("Sorry the metadata=0x%x, the mode need use CHANGE_TO",metadata);
			rtn = _db_cache_add_item((medialib_view_t)viewhandle,metadata,idx,&valuereal,0,TYPE_VALUE_INT,\
							MODIFY_MODE_CHANGETO,&db_cache_info,ACTION_MODIFY);	
		}
		else if(ret==3){
			int m_mode=0;
			switch(modify_mode){
				case  ML_MODIFY_CHANGE_TO:
					m_mode = MODIFY_MODE_CHANGETO;
					break;
				case  ML_MODIDY_SET_BITS:
					m_mode = MODIFY_MODE_XOR;
					break;
				case  ML_MODIFY_CLR_BITS:
					m_mode = MODIFY_MODE_AND;
					valuereal = ~(valuereal);
					break;
			}
			db_info("metadata=0x%x,fileidx=%d,tagvalue=0x%x,mode=%d",metadata,idx,valuereal,m_mode);
			rtn = _db_cache_add_item((medialib_view_t)viewhandle,metadata,idx,&valuereal,0,TYPE_VALUE_INT,\
						m_mode,&db_cache_info,ACTION_MODIFY);	
		}
		else
			db_err("Crazy: rtn==%d",ret);
	}
	
MODIFY_ITEMS_END:
	if(rtn==-1)
		db_err("Sorry modify Failed");
	
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}



static int medialib_get_info(void *handle){
	char path[1][FULL_PATH_LEN];
	int infotype=0;
	unsigned int idx[1];
	int info_mode=0;
	int viewhandle=0;
	struct user_info usertag[1];
	SWFEXT_FUNC_BEGIN(handle);
	
	viewhandle = Swfext_GetNumber();
	idx[0]=Swfext_GetNumber();
	info_mode = Swfext_GetNumber();

	db_info("viewhandle=0x%x,idx=%d,info_mode=%d\n",viewhandle,idx[0],info_mode);
	memset(path[0],0,FULL_PATH_LEN);
	
	if(viewhandle==0){
		db_err("Crazy the ViewHandle==NULL");
		Swfext_PutString(NULL);
		goto GET_INFO_END;
	}

	infotype =INFO_TYPE_PHOTO;//this value should be changed when the video or the music is added in
	if(_file_info_cache(CACHE_CHECK,(medialib_view_t)viewhandle,idx[0],infotype)==0){
		if(medialib_select((medialib_view_t)viewhandle,idx,1,path,usertag)==-1){
			db_err("Sorry medialib_select Error");
		}
		//save file path 
		memset(fileinfo_cache.filepath,0,FULL_PATH_LEN);
		memcpy(fileinfo_cache.filepath,path[0],FULL_PATH_LEN);

		if(stat(fileinfo_cache.filepath,&fileinfo_cache.fileinfo)==-1){
			db_err("Get File Info Error");
			Swfext_PutString(NULL);
			goto GET_INFO_END;
		}

		//save user tag
		memcpy(&fileinfo_cache.usertag,usertag,sizeof(struct user_info));
		db_info("@@idx=%d@@@@@usertag==%d",idx[0],usertag[0].user_tag1);
		
		//save photo info
		if(infotype==INFO_TYPE_PHOTO){
			MEDIA_INFO_RET ret;
			fileinfo_cache.md_info.file_handle = fui_os_fopen(fileinfo_cache.filepath,"rb");
			if(fileinfo_cache.md_info.file_handle==NULL)
				db_err("Crazy:Open File Error %s",fileinfo_cache.filepath);
			//tell the middle ware which info we need
			fileinfo_cache.photo_info.date_time_org_enable= 1;
			fileinfo_cache.photo_info.img_hei_enable = 1;
			fileinfo_cache.photo_info.img_wid_enable = 1;
			
			ret= GetMediaInfo(&fileinfo_cache.md_info,M_PHOTO, &fileinfo_cache.photo_info);
			if(ret!=M_INFO_OK){
				db_err("Sorry GetMediaInfo Error");
			}
			if(fileinfo_cache.md_info.file_handle!=NULL)
				fui_os_fclose(fileinfo_cache.md_info.file_handle);
		}	
		else{
			db_info("Sorry the infotype is not support");
		}
		_file_info_cache(CACHE_UPDATE,(medialib_view_t)viewhandle,idx[0],infotype);
	}	

	memset(info_buffer,0,INFO_BUFFER_LEN);

	switch(info_mode){
		case INFO_GET_FILE_PATH:
			strcpy(info_buffer,fileinfo_cache.filepath);
			break;
		case INFO_GET_FILE_NAME:
			_get_filename(fileinfo_cache.filepath,info_buffer);
			break;
		case INFO_GET_FILE_TIME:
			{
				struct tm *filetime;
				filetime = localtime(&fileinfo_cache.fileinfo.st_mtime);
				sprintf(info_buffer,"%4d%02d%02d",filetime->tm_year+1900,filetime->tm_mon+1,filetime->tm_mday);
				break;
			}
		case INFO_GET_FILE_SIZE:
			sprintf(info_buffer,"%d",fileinfo_cache.fileinfo.st_size);
			break;
		case INFO_GET_EXIF_EXPTIME:
			sprintf(info_buffer,"%4d%02d%02d",fileinfo_cache.photo_info.iie_date_time_original.year,\
										fileinfo_cache.photo_info.iie_date_time_original.month,\
										fileinfo_cache.photo_info.iie_date_time_original.day);
			break;
		case INFO_GET_EXIF_WIDTH:
			sprintf(info_buffer,"%d",fileinfo_cache.photo_info.main_img_width);
			break;
		case INFO_GET_EXIF_HEIGHT:
			sprintf(info_buffer,"%d",fileinfo_cache.photo_info.main_img_height);
			break;
		case INFO_GET_USER_TAG1:
			sprintf(info_buffer,"%d",fileinfo_cache.usertag.user_tag1);
			break;
		default:
			db_err("Sorry info_mode not support now");
	}
	Swfext_PutString(info_buffer);
GET_INFO_END:
	
	SWFEXT_FUNC_END();
}
#else
static int  medialib_opendb(void *handle){
	int rtn=0;;
	
	SWFEXT_FUNC_BEGIN(handle);
	db_err("Open Config First Do nothing!");
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}

static int  medialib_closedb(void *handle){
	int rtn=0;;
	
	SWFEXT_FUNC_BEGIN(handle);
	db_err("Open Config First Do nothing!");
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}

static int  medialib_createdb(void *handle){
	int rtn=0;;
	
	SWFEXT_FUNC_BEGIN(handle);
	db_err("Open Config First Do nothing!");
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}
static int  medialib_disabledb(void *handle){
	int rtn=0;;
	
	SWFEXT_FUNC_BEGIN(handle);
	db_err("Open Config First Do nothing!");
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}
static int  medialib_destorydb(void *handle){
	int rtn=0;;
	
	SWFEXT_FUNC_BEGIN(handle);
	db_err("Open Config First Do nothing!");
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}
static int  medialib_updatedb(void *handle){
	int rtn=0;;
	
	SWFEXT_FUNC_BEGIN(handle);
	db_err("Open Config First Do nothing!");
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}
static int  medialib_get_thread_status(void *handle){
	int rtn=0;;
	
	SWFEXT_FUNC_BEGIN(handle);
	db_err("Open Config First Do nothing!");
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}
static int  medialib_open_views(void *handle){
	int rtn=0;;
	
	SWFEXT_FUNC_BEGIN(handle);
	db_err("Open Config First Do nothing!");
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}
static int  medialib_close_views(void *handle){
	int rtn=0;;
	
	SWFEXT_FUNC_BEGIN(handle);
	db_err("Open Config First Do nothing!");
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}
static int  medialib_get_num(void *handle){
	int rtn=0;;
	
	SWFEXT_FUNC_BEGIN(handle);
	db_err("Open Config First Do nothing!");
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}
static int  medialib_del_items(void *handle){
	int rtn=0;;
	
	SWFEXT_FUNC_BEGIN(handle);
	db_err("Open Config First Do nothing!");
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}
static int  medialib_add_items(void *handle){
	int rtn=0;;
	
	SWFEXT_FUNC_BEGIN(handle);
	db_err("Open Config First Do nothing!");
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}

static int  medialib_find_item(void *handle){
	int rtn=0;;
	
	SWFEXT_FUNC_BEGIN(handle);
	db_err("Open Config First Do nothing!");
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}

static int  medialib_modify_items(void *handle){
	int rtn=0;;
	
	SWFEXT_FUNC_BEGIN(handle);
	db_err("Open Config First Do nothing!");
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}

static int  medialib_get_info(void *handle){
	int rtn=0;;
	
	SWFEXT_FUNC_BEGIN(handle);
	db_err("Open Config First Do nothing!");
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}

#endif
static int medialib_check_udisk_dirty(void *handle)
{
	int rtn =0;
	SWFEXT_FUNC_BEGIN(handle);
	rtn = get_is_udisk_dirty();
	if(rtn==1)
		set_is_udisk_dirty(0); ///< clear the dirty flag
	Swfext_PutNumber(rtn);

	SWFEXT_FUNC_END();
}

static int medialib_get_tag_photo_attached(void *handle)
{
	unsigned int rtn=0;
	char * filepath=NULL;
	SWFEXT_FUNC_BEGIN(handle);
	filepath = Swfext_GetString();
	rtn = medialib_read_photo_tag(filepath);
	Swfext_PutNumber((int)rtn);
	SWFEXT_FUNC_END();
}

static int medialib_store_tag_photo_attached(void *handle)
{
	int rtn=0;
	char * filepath=NULL;
	unsigned int tagvalue=0;
	SWFEXT_FUNC_BEGIN(handle);
	filepath = Swfext_GetString();
	tagvalue = (unsigned int)Swfext_GetNumber();
	
	rtn = medialib_store_photo_tag(filepath,tagvalue);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

int medialib_check_udisk_written()
{

	char filename[64]="/sys/devices/gadget/gadget-lun0/iswritten";
	char iswritten[2]="";
	int rtn = -1;
	int fd=0;
	if(get_is_udisk_dirty()==1)
			return 1;
	fd = open(filename,O_RDONLY);
	if(fd==-1)
	{
		return rtn;
	}
	if(read(fd,iswritten,1)!=1){
		rtn = -1;
		goto CHECK_UDISK_END;
	}
	if(iswritten[0]=='0')
		rtn =0;
	else{
		rtn = 1;
		set_is_udisk_dirty(rtn);
	}
CHECK_UDISK_END:
	close(fd);
	return rtn;
}


int swfext_album_register(void)
{
	printf("swfext_album_register\n");
	SWFEXT_REGISTER("album_openDB", medialib_opendb);
	SWFEXT_REGISTER("album_closeDB", medialib_closedb);
	SWFEXT_REGISTER("album_createDB",medialib_createdb);
	SWFEXT_REGISTER("album_disableDB", medialib_disabledb);
	SWFEXT_REGISTER("album_destoryDB", medialib_destorydb);
	SWFEXT_REGISTER("album_updateDB", medialib_updatedb);
	SWFEXT_REGISTER("album_getDBCreateStatus",medialib_get_thread_status);
	SWFEXT_REGISTER("album_delItem",medialib_del_items);
	SWFEXT_REGISTER("album_addItem",medialib_add_items);
	SWFEXT_REGISTER("album_findItem",medialib_find_item);
	SWFEXT_REGISTER("album_modifyItem",medialib_modify_items);
	SWFEXT_REGISTER("album_getNum",medialib_get_num);
	SWFEXT_REGISTER("album_getInfo",medialib_get_info);
	SWFEXT_REGISTER("album_openView",medialib_open_views);
	SWFEXT_REGISTER("album_closeView",medialib_close_views);
	SWFEXT_REGISTER("album_checkudiskdirty",medialib_check_udisk_dirty);	
	SWFEXT_REGISTER("album_getTagPhotoAttached", medialib_get_tag_photo_attached);
	SWFEXT_REGISTER("album_storeTagPhotoAttached", medialib_store_tag_photo_attached);
	return 0;
}

#endif


