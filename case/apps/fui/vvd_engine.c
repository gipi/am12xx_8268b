#ifdef MODULE_CONFIG_VVD
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vvd_engine.h"
#include "file_list.h"
#include "sys_rtc.h"
#include "fui_input.h"
#include "audio_midware.h"
#include "audio_engine.h"


/** current instance */
static VD2_INFO * curr_vd2 = NULL;
static pthread_t audio_thread_id;

/** default vividshare template */
static VT_INFO    s_vt;

extern void *audio_engine;
extern char cur_file[FULL_PATH_SIZE];

extern int sys_active_media;
extern char *disk_rootpath[];
extern file_iocontext_s file_iocontext;
extern music_status_t player_status;

static void vvd_current_path(char * path)
{
	int media;
	
	switch(sys_active_media)
	{
		case NAND_DISK:
		case CARD_DISK:
		case USB_DISK1:
		case HARD_DISK:
			media = sys_active_media;
			break;
		default:
			media = NAND_DISK;
			break;
	}
	sprintf(path,"%s",disk_rootpath[media]);
}

static void *vvd_create_audio_engine(void)
{
	/** FIXME: */
	return audio_play_open();
}

static void vvd_stop_audio_engine()
{
	if(audio_engine){
		_ae_engine_delete();
		_audio_clear_setfile_para(&file_iocontext);
		audio_engine = NULL;
	}
}

static void vvd_music_play_and_pause()
{
	VD2_INFO * vd2_info = curr_vd2;
	play_param_t play_param;
	if(audio_engine){
		if(curr_vd2->state == VVD_PLAY){
			play_param.mode=TAG_PLAY;
			play_param.param=0;
			audio_play_cmd(audio_engine,PLAY,(unsigned int)&play_param);
		}
		else if(curr_vd2->state == VVD_PAUSE){
			audio_play_cmd(audio_engine,pause,0);
		}
	}
}


static void *vvd_music_check()
{
	VD2_INFO * vd2_info = curr_vd2;
	VT_INFO * vt;
	int i = 0;
	play_param_t play_param;
	char * music_file = NULL;
	ITEM_INFO * item;
	int total=0,counter=0;
	int index;
	BLIST* bilist;
	a_set_file_s aSetfile;
	music_file_info_t media_info;

	if(curr_vd2 == NULL || curr_vd2->state == VVD_IDLE){
		printf("%s,%d:Music Thread exit,curr_vd2==NULL\n",__FILE__,__LINE__);
		pthread_exit((void*)1);
	}
	
	if(audio_engine == NULL){
		printf("%s,%d:Music Thread exit,audio_engine==NULL\n",__FILE__,__LINE__);
		pthread_exit((void*)1);
	}

	if(BLIST_IS_EMPTY(&vd2_info->music_list)){
		/** no music at all, do not play */
		vvd_stop_audio_engine();
		printf("%s,%d:Music Thread exit,no music file\n",__FILE__,__LINE__);
		pthread_exit((void*)1);
	}

	//ITERATE_OVER_LIST(&vd2_info->music_list, (BLIST*)item)
	ITERATE_OVER_LIST(&vd2_info->music_list, bilist){
		item = (ITEM_INFO *)bilist;
		total++;
	}

	while(1){
		if(vd2_info->state == VVD_IDLE){
			printf("%s,%d:Music Thread exit,state is IDLE\n",__FILE__,__LINE__);
			pthread_exit((void*)1);
		}
		
		index = vd2_info->music_index;
		counter=0;
		
		//printf("%s,%d:total music is %d,index is %d\n",__FILE__,__LINE__,total,index);
		
		if(total>0){
			//ITERATE_OVER_LIST(&vd2_info->music_list, (BLIST*)item)
			ITERATE_OVER_LIST(&vd2_info->music_list, bilist)
			{
				item = (ITEM_INFO *)bilist;
				counter++;
				if(index == (counter-1)){
					music_file = item->path;
					break;
				}
			}
		}
				
		audio_play_cmd(audio_engine,GET_PLAYER_STATUS,(unsigned int)&player_status);
		//printf("%s,%d:the status is %d\n",__FILE__,__LINE__,player_status.status);
		if(music_file && audio_engine && player_status.status==PLAYER_STOPPED)
		{
			/** FIXME: [set file and play]*/
			printf("%s,%d:music file is %s\n",__FILE__,__LINE__,music_file);
			_audio_set_setfile_para(&file_iocontext,music_file);
			aSetfile.f_io = &file_iocontext;
			aSetfile.pInfo = &media_info;
			audio_play_cmd(audio_engine,(unsigned int)SET_FILE,(unsigned int)&aSetfile);
			play_param.mode=NORMAL_PLAY;
			play_param.param=0;
			audio_play_cmd(audio_engine,PLAY,(unsigned int)&play_param);
			vd2_info->music_index++;
			if(vd2_info->music_index >= total){
				vd2_info->music_index=0;
			}
		}
	}
	
}

static void vvd_start_audio_engine(VD2_INFO * vd2_info)
{
	/** FIXME: [start but not play]*/
	int ret;
	if(audio_engine==NULL){
		audio_engine = vvd_create_audio_engine();
		if(audio_engine){
			printf("%s,%d:audio engine is 0x%x\n",__FILE__,__LINE__,audio_engine);
			ret = pthread_create(&audio_thread_id,NULL,vvd_music_check,NULL);
			if(ret==-1){
				printf("%s,%d:Create image thread error!\n",__FILE__,__LINE__);
			}
		}
		else{
			printf("%s,%d:vvd_create_audio_engine error!\n",__FILE__,__LINE__);
		}
	}
	else{
		printf("%s,%d:audio engine is 0x%x\n",__FILE__,__LINE__,audio_engine);
		ret = pthread_create(&audio_thread_id,NULL,vvd_music_check,NULL);
		if(ret==-1){
			printf("%s,%d:Create image thread error!\n",__FILE__,__LINE__);
		}
	}
}

#if 0
static int strcasecmp(char * cs,char * ct)
{
    register signed char __res;
	
    while (1) {
        if (!*cs && !*ct)
            return 0;
        else if (!*cs)
            return -1;  
        else if (!*ct)
            return 1;  
        __res = *cs - *ct;
        if (__res != 0 )
        {
            if((*cs>='A')&&(*cs<='Z'))
                __res += 32;   //'a'-'A'=32
            else
                __res-=32;
            if (__res != 0 )
                break;
        }
        cs++;
        ct++;
    }
	return __res;
}

static char * strdup(char * s)
{
	char * str = (char*)malloc(strlen(s) + 1);
	strcpy(str, s);
	
	return str;
}


static int str2i(char * str)
{
	int num;
	for(num = 0; *str != 0; str++)
	{
		num = num * 10 + (*str - '0');
	}
	return num;
}

#endif

static int vvd_parse_value(void * h, stream_input_t * pb, int (*set_value)(VD2_INFO *,char*,char*))
{
	int len, i, n = 0, state = VVD_PARSER_NAME;
	char * buf, * name, * val;
	int ret = 0;
	
	buf  = (char*)malloc(BUF_LEN);
	name = (char*)malloc(50);
	val  = (char*)malloc(FULL_PATH_SIZE);

	while((len = pb->read(pb, (unsigned int)buf, BUF_LEN)) > 0)
	{
		for(i = 0; i < len; i++)
		{
			switch(buf[i])
			{
			case '\t':
			case '\r':
			//case ' ' :
			case '\0':
				continue;
			case '\n':
				if(state == VVD_PARSER_VALUE)
				{
					val[n] = 0;
					//printf("%s,%d:name is %s,val is %s\n",__FILE__,__LINE__,name,val);
					if(set_value(h, name, val))
					{
						ret = 1;
					}
				}
				n = 0;
				state = VVD_PARSER_NAME;
				break;
			case '=':
				if(state == VVD_PARSER_NAME)
				{
					name[n] = 0;
					state = VVD_PARSER_VALUE;
				}
				n = 0;
				break;
			default:
				if(state == VVD_PARSER_NAME)
				{
					name[n] = buf[i];
				}
				else if(state == VVD_PARSER_VALUE)
				{
					val[n] = buf[i];
				}
				n++;
				if(n>=FULL_PATH_SIZE){
					DBG_PRINT("%s,%d:name space is not enough\n",__FILE__,__LINE__);
					break;
				}
			}
		}
	}

	free(val);
	free(name);
	free(buf);
	
	return ret;
}

static VT_INFO * vvd_new_template(char * filename)
{
	char * suffix;
	VT_INFO * vt_info = (VT_INFO*)malloc(sizeof(VT_INFO));
	vt_info->swf_inst = NULL;
	strcpy(vt_info->flash, filename);
	strcpy(vt_info->thumb, vt_info->flash);
	suffix = strstr(vt_info->thumb, ".swf");
	if(suffix != NULL)
	{
		suffix[1] = 'j';
		suffix[2] = 'p';
		suffix[3] = 'g';
	}
	else
	{
		vt_info->thumb[0] = 0;
	}
	return vt_info;
}

static int vvd_parser(VD2_INFO * vd2_info, char * name, char * val)
{
	if(val[0] != 0 && name[0] != 0)
	{
		if(strcasecmp(name, "theme") == 0)
		{
			strcpy(vd2_info->themepath, val);
		}
		else if(strcasecmp(name, "template") == 0)
		{
			BListAdd(&vd2_info->templ_list, &vvd_new_template(val)->BList);
		}
		else if(strcasecmp(name, "pic") == 0)
		{
			ITEM_INFO * photo = (ITEM_INFO*)malloc(sizeof(ITEM_INFO));
			strcpy(photo->path, val);
			BListAdd(&vd2_info->photo_list, &photo->BList);
		}
		else if(strcasecmp(name, "music") == 0)
		{
			ITEM_INFO * music = (ITEM_INFO*)malloc(sizeof(ITEM_INFO));
			strcpy(music->path, val);
			BListAdd(&vd2_info->music_list, &music->BList);
		}
	}
	return 0;
}

static void vvd_new_name(char * path, char * filename)
{
	char * p0 = NULL, * p1 = NULL, * p;
	char prefix[MNAVI_MAX_NAME_LEN];
	rtc_date_t date;
	rtc_time_t time;

	/** remove space and tab */
	for(p = &path[strlen(path) - 1]; p > path; p--)
	{
		if(*p == ' ' || *p == '\t')
		{
			*p = 0;
		}
		else
		{
			break;
		}
	}
	/** search '/' */ 
	for(p = path; *p != 0; p++)
	{
		if(*p == '/')
		{
			p0 = p1;
			p1 = p;
		}
	}
	
	/** check last '/' */
	if(p > path && *(p - 1) != '/')
	{
		p0 = p1;
		p1 = p;
	}
	
	/** get filename */
	if(p0 != NULL && p1 != NULL)
	{
		int i;
		for(i = 0, p = p0 + 1; p < p1; p++, i++)
		{
			prefix[i] = *p;
		}
		prefix[i] = 0;
	}
	else
	{
		strcpy(prefix, "user");
	}

	tm_get_rtc(&date,&time);
	
	sprintf(filename, "%s%02d%02d%02d%02d%02d.vd2", prefix, date.year%100, date.month+1, date.day, time.hour, time.min);
}

static VD2_INFO * vvd_new(char * filename)
{
	VD2_INFO * vd2_info;
	vd2_info = (VD2_INFO*)malloc(sizeof(VD2_INFO));
	if(vd2_info==NULL){
		return NULL;
	}
	BLIST_INIT(&vd2_info->templ_list);
	BLIST_INIT(&vd2_info->music_list);
	BLIST_INIT(&vd2_info->photo_list);
	vd2_info->state  = VVD_IDLE;
	vd2_info->templ_index = 0;
	vd2_info->photo_index = 0;
	vd2_info->music_index = 0;
	vd2_info->templ_files = NULL;
	vd2_info->photo_files = NULL;
	vd2_info->music_files = NULL;
	vd2_info->photo_overoll= 0;
	vd2_info->filename[0] = 0;
	vd2_info->photodir[0] = 0;
	vd2_info->musicdir[0] = 0;
	strcpy(vd2_info->themepath, filename);
	
	return vd2_info;
}

static VD2_INFO * vvd_open(char * filename)
{
	VD2_INFO * vd2_info = NULL;
	stream_input_t * pb;
	int listtotal=0;
	char * path=NULL;
	if(strstr(filename, ".") == NULL)
	{
		//This is a theme directory
		UI_FILELIST tmpl_files;
		vd2_info = vvd_new(filename);
		vvd_new_name(vd2_info->themepath, vd2_info->filename);
		ui_filelist_init(&tmpl_files, filename, "swf", 0, FLIST_MODE_BROWSE_FILE);
		listtotal = ui_filelist_get_filetotal_num(&tmpl_files);
		if(listtotal > 0)
		{
			int i, templ_index = -1;
			printf("%s,%d:total is %d\n",__FILE__,__LINE__,listtotal);
			for(i = 0; i < listtotal; i++)
			{
				path = ui_filelist_get_cur_filepath(&tmpl_files, i);
				//printf("%s,%d:i is %d,path is %s\n",__FILE__,__LINE__,i,path);
				BListAdd(&vd2_info->templ_list, &vvd_new_template(path)->BList);
			}
		}
		ui_filelist_exit(&tmpl_files);
	}
	else
	{
		// if select a theme file
		int len;
		
		len = strlen(filename);
		if(len<=0){
			return NULL;
		}

		if( ((filename[len-1] == 'F') || (filename[len-1] == 'f')) && \
			((filename[len-2] == 'W') || (filename[len-2] == 'w')) && \
			((filename[len-3] == 'S') || (filename[len-3] == 's'))){
				VT_INFO *temp;
				vd2_info = vvd_new(filename);
				if(vd2_info==NULL){
					return NULL;
				}
				temp=vvd_new_template(filename);
				if(temp==NULL){
					free(vd2_info);
					return NULL;
				}
				BListAdd(&vd2_info->templ_list, &temp->BList);
				vd2_info->templ_index = 1;
				
		}
		else if(((filename[len-1] == '2') || (filename[len-1] == '2')) && \
			((filename[len-2] == 'D') || (filename[len-2] == 'd')) && \
			((filename[len-3] == 'V') || (filename[len-3] == 'v'))){
			//This is a normal file
			pb = create_fui_input(filename);
			if(pb != NULL)
			{
				vd2_info = vvd_new(filename);
				vvd_parse_value(vd2_info, pb, vvd_parser);
				pb->dispose(pb);
			}
		}
		else{
			return NULL;
		}
	}
	return vd2_info;
}

static int vvd_save(VD2_INFO * vd2, char * path)
{
	VT_INFO * vt;
	ITEM_INFO * item;
	stream_input_t * pb = NULL;
	char *buf;
	BLIST* bilist;
	
	
	if(vd2 == NULL){ 
		return 0;
	}
	
	buf = (char*)malloc(FULL_PATH_SIZE);
	if(path != NULL)
	{
		memset(buf,0,FULL_PATH_SIZE);
		strcpy(buf, path);
		if(path[strlen(path)-1] != '/')
		{
			strcat(buf, "/");
		}
		strcat(buf, vd2->filename);
	}
	else
	{
		memset(buf,0,FULL_PATH_SIZE);
		strcpy(buf, disk_rootpath[NAND_DISK]);
	}
	
	pb = create_fui_output(buf);
	if(pb != NULL)
	{
		memset(buf,0,FULL_PATH_SIZE);
		sprintf(buf, "theme=%s\n", vd2->themepath);
		pb->write(pb, (int)buf, strlen(buf));
		
		//ITERATE_OVER_LIST(&vd2->templ_list, (BLIST*)vt)
		ITERATE_OVER_LIST(&vd2->templ_list, bilist)
		{
			vt = (VT_INFO *)bilist;
			memset(buf,0,FULL_PATH_SIZE);
			sprintf(buf, "template=%s\n", vt->flash);
			pb->write(pb, (int)buf, strlen(buf));
		}
		
		//ITERATE_OVER_LIST(&vd2->photo_list, (BLIST*)item)
		ITERATE_OVER_LIST(&vd2->photo_list, bilist)
		{
			item = (ITEM_INFO *)bilist;
			memset(buf,0,FULL_PATH_SIZE);
			sprintf(buf, "pic=%s\n", item->path);
			pb->write(pb, (int)buf, strlen(buf));
		}
		
		//ITERATE_OVER_LIST(&vd2->music_list, (BLIST*)item)
		ITERATE_OVER_LIST(&vd2->music_list, bilist)
		{
			item = (ITEM_INFO *)bilist;
			memset(buf,0,FULL_PATH_SIZE);
			sprintf(buf, "music=%s\n", item->path);
			pb->write(pb, (int)buf, strlen(buf));
		}
		pb->dispose(pb);
		free(buf);
		return 1;
	}
	return 0;
}

static void vvd_close(VD2_INFO * vd2)
{
	VT_INFO * vt;
	ITEM_INFO * item;
	stream_input_t * pb = NULL;
	if(vd2 != NULL)
	{
		while((vt = (VT_INFO*)BListRemoveItemFromHead(&vd2->templ_list)) != 0)
		{
			free(vt);
		}
		while((item = (ITEM_INFO*)BListRemoveItemFromHead(&vd2->music_list)) != 0)
		{
			free(item);
		}
		while((item = (ITEM_INFO*)BListRemoveItemFromHead(&vd2->photo_list)) != 0)
		{
			free(item);
		}
		if(vd2->photo_files != NULL)
		{
			ui_filelist_exit(vd2->photo_files);
			free(vd2->photo_files);
		}
		if(vd2->music_files != NULL)
		{
			ui_filelist_exit(vd2->music_files);
			free(vd2->music_files);
		}
		if(vd2->templ_files != NULL)
		{
			ui_filelist_exit(vd2->templ_files);
			free(vd2->templ_files);
		}
		free(vd2);
		vvd_stop_audio_engine();
	}
}

static BLIST * vvd_get_item(BLIST * list, int index)
{
	BLIST* bilist;
	
	//ITERATE_OVER_LIST(list, (BLIST*)item)
	ITERATE_OVER_LIST(list, bilist)
	{
		if(index-- == 0)
		{
			return bilist;
		}
	}
	return NULL;
}

static int vvd_check_file_exist(const char *filename)
{
	return access(filename, 0); 
}
static int vvd_decode_bitmap(SWF_IMAGE_INFO * info)
{
	int w=0,h=0;
	int file_total_num=0;
	if(SWF_GetInstInfo(info->swf_inst,SWF_GETINFO_WIDTH,(void *)&w) == -1){
		return 0;
	}
	
	if(SWF_GetInstInfo(info->swf_inst,SWF_GETINFO_HEIGHT,(void *)&h) == -1){
		return 0;
	}

VVD_SEARCH_PHOTO:
	
	if(curr_vd2 != NULL)
	{
		//if((info->flag & 0xF) == SWF_BMP_FMT_YUV422 && (info->width * info->height == 800 * 600))
		if((info->flag & 0xF) == SWF_BMP_FMT_YUV422 && (info->width ==w ) && ( info->height == h))
		{
			if(BLIST_IS_EMPTY(&curr_vd2->photo_list))
			{
				#if 0
				if(curr_vd2->photo_files == NULL)
				{
					char path[FULL_PATH_SIZE];
					if(strlen(curr_vd2->photodir)==0){
						vvd_current_path(path);
					}
					else{
						strcpy(path,curr_vd2->photodir);
					}
					curr_vd2->photo_files = (UI_FILELIST*)malloc(sizeof(UI_FILELIST));					
					ui_filelist_init(curr_vd2->photo_files, path, "jpg bmp jpe", 0, FILES_BROWSE);
					file_total_num = ui_filelist_get_filetotal_num(curr_vd2->photo_files);
				}
				else
					file_total_num = ui_filelist_get_filetotal_num(curr_vd2->photo_files);
				#endif
				if(curr_vd2->photo_files == NULL)
					file_total_num = 0;
				else
					file_total_num = ui_filelist_get_filetotal_num(curr_vd2->photo_files);
				if(file_total_num > 0)
				{
					//curr_vd2->photo_index = rand() % currVd2->photo_files->list.total;
					curr_vd2->photo_index ++;
					if(curr_vd2->photo_index >= file_total_num){
						curr_vd2->photo_index = 0;
						curr_vd2->photo_overoll = 1;
					}
					info->jpeg_data = (unsigned char*)ui_filelist_get_cur_filepath(curr_vd2->photo_files, curr_vd2->photo_index);
					info->jpeg_size = 0;
					DBG_PRINT("vvd show %d, %s\n",__LINE__, info->jpeg_data);
				}
			}
			else
			{
				ITEM_INFO * item;
				ITEM_INFO * photo;				
				int index = curr_vd2->photo_index++;
				int total_photo = 0,cnt =0;
				BLIST* bilist;

				//ITERATE_OVER_LIST(&curr_vd2->photo_list, (BLIST*)item)
				ITERATE_OVER_LIST(&curr_vd2->photo_list, bilist)
				{
					total_photo++;
				}

				if(curr_vd2->photo_index >= total_photo){
					curr_vd2->photo_index = 0;
					curr_vd2->photo_overoll = 1;
				}

				if(index >= total_photo){
					index = total_photo-1;
				}

				//ITERATE_OVER_LIST(&curr_vd2->photo_list, (BLIST*)item)
				ITERATE_OVER_LIST(&curr_vd2->photo_list, bilist)
				{
					item = (ITEM_INFO *)bilist;
					cnt ++;
					if(cnt == (index+1))
					{
						if(-1 == vvd_check_file_exist(item->path)){
							DBG_PRINT("photo file not exist\n");
							photo = (ITEM_INFO*)vvd_get_item(&curr_vd2->photo_list, index);
							if(photo != NULL)
							{
								BListRemove(&photo->BList);
								free(photo);
								 curr_vd2->photo_index++;
								index ++;
								 goto VVD_SEARCH_PHOTO;
							}
						}
						info->jpeg_data = (unsigned char*)item->path;
						info->jpeg_size = 0;
						DBG_PRINT("vvd show %d, %s\n",__LINE__, info->jpeg_data);
						break;
					}
				}
				
			}
		}
		fui_decode_bitmap(info);
	}
	return 0;
}

static int vvd_release(SWF_DEC * s);
static int vvd_add_playlist(VT_INFO * vt)
{
	SWF_DEV_FUNC dev_func;
	SWF_INSTINFO info;
	SWF_RECT window = {0, 0, IMAGE_WIDTH_E-1, IMAGE_HEIGHT_E-1};

	//printf("---inst is %s---\n",vt->flash);
	vt->swf_inst = SWF_AddInst(vt->flash, &window, SWF_DEFAULT_FLAG | SWF_STANDALONE | SWF_NON_RELOAD, &info);
	if(vt->swf_inst != NULL)
	{
		SWF_GetDeviceFunction(vt->swf_inst, &dev_func);
		dev_func.decode_bitmap = vvd_decode_bitmap;  
		dev_func.release = vvd_release;
		SWF_SetDeviceFunction(vt->swf_inst, &dev_func);
		
		return 1;
	}
	
	return 0;
}


static int vvd_next(VD2_INFO * vd2)
{
	int file_total_num=0;	
	if(BLIST_IS_EMPTY(&vd2->templ_list))
	{
		if(vd2->templ_files == NULL)
		{
			vd2->templ_files = (UI_FILELIST*)malloc(sizeof(UI_FILELIST));					
			ui_filelist_init(vd2->templ_files, vd2->themepath, "swf", 0, FLIST_MODE_BROWSE_FILE);
			file_total_num = ui_filelist_get_filetotal_num(vd2->templ_files);
		}
		else
			file_total_num = ui_filelist_get_filetotal_num(vd2->templ_files);
		
		if(file_total_num > 0)
		{
			vd2->templ_index = rand() % file_total_num;
			strcpy(s_vt.flash, ui_filelist_get_cur_filepath(vd2->templ_files, vd2->templ_index));
			return vvd_add_playlist(&s_vt);
		}
	}
	else
	{
		VT_INFO * vt;
		int i = 0;
		int total=0;
		BLIST* bilist;
		
		//ITERATE_OVER_LIST(&vd2->templ_list, (BLIST*)vt)
		ITERATE_OVER_LIST(&vd2->templ_list, bilist)
		{
			total++;
		}
		
		if(total<=0){
			return 0;
		}


		if(vd2->templ_index>=total-1){
			vd2->templ_index=0;
			//return 0;
		}
		else{
			vd2->templ_index++;
		}
		
		//ITERATE_OVER_LIST(&vd2->templ_list, (BLIST*)vt)
		ITERATE_OVER_LIST(&vd2->templ_list, bilist)
		{
			vt = (VT_INFO *)bilist;
			if(i == vd2->templ_index)
			{
				return vvd_add_playlist(vt);
			}
			i++;
		}
		vd2->templ_index = 0;
	}
	return 0;
}


static void vvd_play(VD2_INFO * vd2)
{
	if(vd2 != NULL)
	{
		if(vd2->state == VVD_IDLE)
		{
			vd2->templ_index--;
			if(vvd_next(vd2))
			{
				vd2->state = VVD_PLAY;
				vvd_start_audio_engine(vd2);
				
				return;
			}
			DBG_PRINT("vvd no valid template\n");
		}
		else
		{
			DBG_PRINT("vvd not idle : %d\n", vd2->state);
		}
	}
}
static void vvd_pause(VD2_INFO * vd2)
{
	VT_INFO * vt;
	BLIST* bilist;
	
	if(vd2->state == VVD_PLAY)
	{
		int i = 0;
		//ITERATE_OVER_LIST(&vd2->templ_list, (BLIST*)vt)
		ITERATE_OVER_LIST(&vd2->templ_list, bilist)
		{
			vt = (VT_INFO *)bilist;
			if(i == vd2->templ_index)
			{
				
				SWF_Message(vt->swf_inst, SWF_MSG_PAUSE, NULL);
			}
			i++;
		}
		vd2->state = VVD_PAUSE;
		vvd_music_play_and_pause();
	}
}
static void vvd_resume(VD2_INFO * vd2)
{
	VT_INFO * vt;
	BLIST* bilist;
	
	if(vd2->state == VVD_PAUSE)
	{
		int i = 0;
		//ITERATE_OVER_LIST(&vd2->templ_list, (BLIST*)vt)
		ITERATE_OVER_LIST(&vd2->templ_list, bilist)
		{
			vt = (VT_INFO *)bilist;
			if(i == vd2->templ_index)
			{
				SWF_Message(vt->swf_inst, SWF_MSG_ACTIVE, NULL);
			}
			i++;
		}
		vd2->state = VVD_PLAY;
		vvd_music_play_and_pause();
	}
}
static void vvd_stop(VD2_INFO * vd2)
{
	if(vd2->state==VVD_IDLE){
		return;
	}
	
	if(BLIST_IS_EMPTY(&vd2->templ_list))
	{
		
		if(s_vt.swf_inst)
		{
			SWF_DEV_FUNC dev_func;
			dev_func.release = NULL;
			SWF_SetDeviceFunction(s_vt.swf_inst, &dev_func);
			SWF_RemoveInst(s_vt.swf_inst);
			SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
		}
		else{
			SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
		}
	}
	else
	{
		VT_INFO * vt;
		int i = 0;
		int find=0;
		BLIST* bilist;
		
		//ITERATE_OVER_LIST(&vd2->templ_list, (BLIST*)vt)
		ITERATE_OVER_LIST(&vd2->templ_list, bilist)
		{
			vt = (VT_INFO *)bilist;
			if(i  == vd2->templ_index)
			{
				SWF_DEV_FUNC dev_func;
				dev_func.release = NULL;
				SWF_SetDeviceFunction(vt->swf_inst, &dev_func);
				SWF_RemoveInst(vt->swf_inst);
				SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
				find=1;
				break;
			}
			i++;
		}

		
		if(find==0){
			SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
		}
	}
	vd2->state = VVD_IDLE;
	vvd_stop_audio_engine();
}

static int vvd_release(SWF_DEC * s)
{
	if(curr_vd2 != NULL)
	{
		//printf("vvd ----- change theme\n");
		if(curr_vd2->state == VVD_PLAY)
		{
			if(vvd_next(curr_vd2))
			{
				return 1;
			}
			
			SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
			vvd_stop_audio_engine();
			curr_vd2->state = VVD_IDLE;
		}	
	
			
	}
	return 0;
}

/************************************************************************/

static int vvd_engine_open(void * handle)
{
	char * path = NULL;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	if(curr_vd2 != NULL)
	{
		vvd_close(curr_vd2);
		curr_vd2 = NULL;
	}
	if(Swfext_GetParamType() == SWFDEC_AS_TYPE_STRING)
	{
		path = Swfext_GetString();
		//printf("@@@@path=%s\n",path);
		curr_vd2 = vvd_open(path);
	}
	if(curr_vd2 != NULL)
	{
		Swfext_PutNumber(1);
	}
	else
	{
		DBG_PRINT("cannot open vvd file %s\n", path);
		Swfext_PutNumber(0);
	}

	SWFEXT_FUNC_END();	
}

static int vvd_engine_save(void * handle)
{
	int type;
	char * path = NULL;
	
	SWFEXT_FUNC_BEGIN(handle);
	if(curr_vd2 != NULL)
	{
		type = Swfext_GetParamType();
		if(type == SWFDEC_AS_TYPE_STRING)
		{
			path = Swfext_GetString();
		}
		if(vvd_save(curr_vd2, path))
		{
			Swfext_PutNumber(1);
			SWFEXT_FUNC_END();
		}
	}
	Swfext_PutNumber(0);
	
	SWFEXT_FUNC_END();	
}

static int vvd_engine_close(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	
	if(curr_vd2 != NULL)
	{
		vvd_close(curr_vd2);
		curr_vd2 = NULL;
	}

	SWFEXT_FUNC_END();	
}

static int vvd_engine_set_photo_dir(void * handle)
{
	char *path;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	if(curr_vd2 != NULL)
	{
		if(Swfext_GetParamType() == SWFDEC_AS_TYPE_STRING)
		{
			path = Swfext_GetString();
			strcpy(curr_vd2->photodir,path);
		}
	}

	Swfext_PutNumber(1);
	
	SWFEXT_FUNC_END();	
}

static int vvd_engine_photo_rollover(void * handle)
{
	int overoll=0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	if(curr_vd2 != NULL)
	{
		if(curr_vd2->photo_overoll){
			overoll = 1;
		}
	}
	Swfext_PutNumber(overoll);
	
	SWFEXT_FUNC_END();	
}

static int vvd_engine_reset_photo_index(void * handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	
	if(curr_vd2 != NULL){
		curr_vd2->photo_index = 0;
		curr_vd2->photo_overoll = 0;
	}
	Swfext_PutNumber(1);
	
	SWFEXT_FUNC_END();	
}

static int vvd_engine_get_first_template(void * handle)
{
	VT_INFO * vt;
	BLIST* blist;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	if(curr_vd2 != NULL){

		if(BLIST_IS_EMPTY(&curr_vd2->templ_list)){
			Swfext_PutString(NULL);
		}
		else{
			
			//ITERATE_OVER_LIST(&curr_vd2->templ_list, (BLIST*)vt)
			ITERATE_OVER_LIST(&curr_vd2->templ_list, blist)
			{
				vt = (VT_INFO *)blist;
				Swfext_PutString(vt->flash);
				printf("first template is %s\n",vt->flash);
				break;
			}
		}
	}
	else{
		Swfext_PutString(NULL);
	}
	
	SWFEXT_FUNC_END();	
}

static int vvd_engine_play(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(curr_vd2 != NULL)
	{
		vvd_play(curr_vd2);
	}
	SWFEXT_FUNC_END();	
}

static int vvd_engine_stop(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(curr_vd2 != NULL)
	{
		vvd_stop(curr_vd2);
	}
	SWFEXT_FUNC_END();	
}

static int vvd_engine_pause(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(curr_vd2 != NULL)
	{
		vvd_pause(curr_vd2);
	}
	SWFEXT_FUNC_END();	
}

static int vvd_engine_resume(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(curr_vd2 != NULL)
	{
		vvd_resume(curr_vd2);
	}
	SWFEXT_FUNC_END();	
}

static int vvd_engine_get_state(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	
	if(curr_vd2 != NULL)
	{
		Swfext_PutNumber(curr_vd2->state);
	}
	else{
		
		Swfext_PutNumber(VVD_IDLE);
	}
	
	SWFEXT_FUNC_END();	
}

static int vvd_engine_get_play_mode(void * handle)
{
	int playmode=0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	Swfext_PutNumber(playmode);

	SWFEXT_FUNC_END();
}

static int vvd_engine_set_play_mode(void * handle)
{
	int playmode=0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	playmode = Swfext_GetNumber();
	
	SWFEXT_FUNC_END();
}

/************************************************************************/

static int vvd_engine_get_template_num(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	
	if(curr_vd2 != NULL)
	{
		int n = 0;
		BLIST * list;
		ITERATE_OVER_LIST(&curr_vd2->templ_list, list)
		{
			n++;
		}
		Swfext_PutNumber(n);
	}
	
	SWFEXT_FUNC_END();	
}

static int vvd_engine_insert_template(void * handle)
{
	int file_total_num=0;
	SWFEXT_FUNC_BEGIN(handle);
	char * path =NULL;
	if(curr_vd2 != NULL)
	{
		int i = 0, index = Swfext_GetNumber();
		UI_FILELIST tmpl_files;
		ui_filelist_init(&tmpl_files, curr_vd2->themepath, "swf", 0, FLIST_MODE_BROWSE_FILE);
		file_total_num = ui_filelist_get_filetotal_num(&tmpl_files);
		if(file_total_num > 0)
		{
			int tmpl_index = rand() % file_total_num;
			path = ui_filelist_get_cur_filepath(&tmpl_files, tmpl_index);
			VT_INFO * vt = vvd_new_template(path), * vt2;
			BLIST *blist;
			
			if(vt != NULL)
			{
				if(index > 0)
				{
					BLIST * after;
					ITERATE_OVER_LIST(&curr_vd2->templ_list, after)
					{
						if(index-- <= 0) break;
					}
					BListInsertTail(after, &vt->BList);
				}
				else if(index == 0)
				{
					BListInsertHead(&curr_vd2->templ_list, &vt->BList);
				}
				else
				{
					BListAdd(&curr_vd2->templ_list, &vt->BList);
				}
				
				//ITERATE_OVER_LIST(&curr_vd2->templ_list, (BLIST*)vt2)
				ITERATE_OVER_LIST(&curr_vd2->templ_list, blist)
				{
					vt2 = (VT_INFO *)blist;
					if(vt2 == vt)
					{
						break;
					}
					i++;
				}
				Swfext_PutNumber(i);
				SWFEXT_FUNC_END();	
			}
		}
		ui_filelist_exit(&tmpl_files);
	}
	Swfext_PutNumber(-1);
	
	SWFEXT_FUNC_END();	
}

static int vvd_engine_delete_template(void * handle)
{
	VT_INFO * vt;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	if(curr_vd2 != NULL)
	{
		vt = (VT_INFO*)vvd_get_item(&curr_vd2->templ_list, Swfext_GetNumber());
		if(vt != NULL)
		{
			BListRemove(&vt->BList);
			free(vt);
			Swfext_PutNumber(1);
			SWFEXT_FUNC_END();
		}
	}
	Swfext_PutNumber(0);
	
	SWFEXT_FUNC_END();	
}

static int vvd_engine_get_template_path(void * handle)
{
	VT_INFO * vt;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	if(curr_vd2 != NULL)
	{
		vt = (VT_INFO*)vvd_get_item(&curr_vd2->templ_list, Swfext_GetNumber());
		if(vt != NULL)
		{
			Swfext_PutString(vt->flash);
		}
	}
	
	SWFEXT_FUNC_END();	
}

static int vvd_engine_get_thumb_path(void * handle)
{
	VT_INFO * vt;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	if(curr_vd2 != NULL)
	{
		vt = (VT_INFO*)vvd_get_item(&curr_vd2->templ_list, Swfext_GetNumber());
		if(vt != NULL)
		{
			Swfext_PutString(vt->thumb);
		}
	}
	
	SWFEXT_FUNC_END();	
}

static int vvd_engine_get_photo_num(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	
	if(curr_vd2 != NULL)
	{
		int n = 0;
		BLIST * list;
		ITERATE_OVER_LIST(&curr_vd2->photo_list, list)
		{
			n++;
		}
		Swfext_PutNumber(n);
	}
	
	SWFEXT_FUNC_END();	
}

static int vvd_engine_insert_photo(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	
	if(curr_vd2 != NULL)
	{
		int    i = 0;
		int    index = Swfext_GetNumber();
		char * path  = Swfext_GetString();
		ITEM_INFO * photo2=NULL;
		ITEM_INFO *photo = (ITEM_INFO*)malloc(sizeof(ITEM_INFO));
		BLIST* blist;

		if(photo != NULL)
		{
			strcpy(photo->path, path);
			//printf("photo path is %s\n",photo->path);
			if(index > 0)
			{
				BLIST * after;
				ITERATE_OVER_LIST(&curr_vd2->photo_list, after)
				{
					if(index-- <= 0) break;
				}
				BListInsertTail(after, &photo->BList);
			}
			else if(index == 0)
			{
				BListInsertHead(&curr_vd2->photo_list, &photo->BList);
			}
			else
			{
				BListAdd(&curr_vd2->photo_list, &photo->BList);
			}
			
			//ITERATE_OVER_LIST(&curr_vd2->photo_list, (BLIST*)photo2)
			ITERATE_OVER_LIST(&curr_vd2->photo_list, blist)
			{
				photo2 = (ITEM_INFO *)blist;
				if(photo2 == photo)
				{
					break;
				}
				i++;
			}
			Swfext_PutNumber(i);
			SWFEXT_FUNC_END();
		}
	}
	Swfext_PutNumber(-1);
	
	SWFEXT_FUNC_END();	
}

static int vvd_engine_delete_photo(void * handle)
{
	ITEM_INFO * photo;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	if(curr_vd2 != NULL)
	{
		photo = (ITEM_INFO*)vvd_get_item(&curr_vd2->photo_list, Swfext_GetNumber());
		if(photo != NULL)
		{
			BListRemove(&photo->BList);
			free(photo);
			Swfext_PutNumber(1);
			SWFEXT_FUNC_END();
		}
	}
	Swfext_PutNumber(0);
	
	SWFEXT_FUNC_END();	
}

static int vvd_engine_get_photo_path(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	
	if(curr_vd2 != NULL)
	{
		ITEM_INFO * photo = (ITEM_INFO*)vvd_get_item(&curr_vd2->photo_list, Swfext_GetNumber());
		if(photo != NULL)
		{
			Swfext_PutString(photo->path);
		}
	}
	
	SWFEXT_FUNC_END();	
}

static int vvd_engine_get_music_num(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	
	if(curr_vd2 != NULL)
	{
		int n = 0;
		BLIST * list;
		ITERATE_OVER_LIST(&curr_vd2->music_list, list)
		{
			n++;
		}
		Swfext_PutNumber(n);
	}
	
	SWFEXT_FUNC_END();	
}

static int vvd_engine_insert_music(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	
	if(curr_vd2 != NULL)
	{
		int    i = 0;
		int    index = Swfext_GetNumber();
		char * path  = Swfext_GetString();
		ITEM_INFO * music = (ITEM_INFO*)malloc(sizeof(ITEM_INFO)), * music2;
		BLIST* blist;

		if(music != NULL)
		{
			strcpy(music->path, path);
			//printf("music path is %s\n",music->path);
			if(index > 0)
			{
				BLIST * after;
				ITERATE_OVER_LIST(&curr_vd2->music_list, after)
				{
					if(index-- <= 0) break;
				}
				BListInsertTail(after, &music->BList);
			}
			else if(index == 0)
			{
				BListInsertHead(&curr_vd2->music_list, &music->BList);
			}
			else
			{
				BListAdd(&curr_vd2->music_list, &music->BList);
			}
			
			//ITERATE_OVER_LIST(&curr_vd2->music_list, (BLIST*)music2)
			ITERATE_OVER_LIST(&curr_vd2->music_list, blist)
			{
				music2 = (ITEM_INFO *)blist;
				if(music2 == music)
				{
					break;
				}
				i++;
			}
			Swfext_PutNumber(i);
			SWFEXT_FUNC_END();
		}
	}
	Swfext_PutNumber(-1);
	
	SWFEXT_FUNC_END();	
}

static int vvd_engine_delete_music(void * handle)
{
	ITEM_INFO * music;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	if(curr_vd2 != NULL)
	{
		music = (ITEM_INFO*)vvd_get_item(&curr_vd2->music_list, Swfext_GetNumber());
		if(music != NULL)
		{
			BListRemove(&music->BList);
			free(music);
			Swfext_PutNumber(1);
			SWFEXT_FUNC_END();
		}
	}
	Swfext_PutNumber(0);
	
	SWFEXT_FUNC_END();	
}

static int vvd_engine_get_music_path(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	
	if(curr_vd2 != NULL)
	{
		ITEM_INFO * music = (ITEM_INFO*)vvd_get_item(&curr_vd2->music_list, Swfext_GetNumber());
		if(music != NULL)
		{
			Swfext_PutString(music->path);
		}
	}
	
	SWFEXT_FUNC_END();	
}

static int vvd_engine_get_save_path(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	
	if(curr_vd2 != NULL)
	{
		Swfext_PutString(curr_vd2->filename);
	}
	
	SWFEXT_FUNC_END();	
}

static int vvd_engine_load_movie(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	
	if(curr_vd2 != NULL)
	{
		SWF_DEV_FUNC dev_func;
		VT_INFO * vt;
		BLIST* blist;
		
		//ITERATE_OVER_LIST(&curr_vd2->templ_list, (BLIST*)vt)
		ITERATE_OVER_LIST(&curr_vd2->templ_list, blist)
		{
			void * target = Swfext_GetObject();
			int width = Swfext_GetNumber() * 20 / 16;
			int height = Swfext_GetNumber() * 20 / 16;
			
			vt = (VT_INFO *)blist;
			dev_func.decode_bitmap = vvd_decode_bitmap;
			dev_func.play_audio = NULL;
			dev_func.release = NULL;
			dev_func.KeySound_Trigger = NULL;
			SWF_LoadMovie(target, vt->flash, width, height, &dev_func);
			break;
		}
	}
	
	SWFEXT_FUNC_END();	
}

static SWF_DEC * vvd_flash = NULL;

static int vvd_engine_play_flash(void * handle)
{
	SWF_RECT window = {0, 0, IMAGE_WIDTH_E-1, IMAGE_HEIGHT_E-1};
	SWF_INSTINFO info;
	SWF_DEV_FUNC dev_func;
	char * path;

	SWFEXT_FUNC_BEGIN(handle);
	path = Swfext_GetString();
	if(path)
	{
		char fullpath[FULL_PATH_SIZE];

		if(vvd_flash)
		{
			SWF_RemoveInst(vvd_flash);
		}
		
		//get full path
		if(path[0] != '/')
		{
			//relative path
			char * p, * q;
			
			strcpy(fullpath, SWF_GetFilePath(handle));
			
			for(p = fullpath, q = NULL; *p != 0; p++)
			{
				if(*p == '/')
				{
					q = p + 1;
				}
			}
			if(q)
			{
				strcpy(q, path);
			}
			else
			{
				strcpy(fullpath, path);
			}
		}
		else
		{
			strcpy(fullpath, path);
		}

		//insert instance
		vvd_flash = SWF_AddInst(fullpath, &window, SWF_LBOX_FLAG | SWF_STANDALONE, &info);
	}
	SWFEXT_FUNC_END();	
}

static int vvd_engine_stop_flash(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	
	if(vvd_flash)
	{
		SWF_RemoveInst(vvd_flash);
		vvd_flash = NULL;
		SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
	}
	
	SWFEXT_FUNC_END();	
}

void vvd_engine_exception_release()
{
	// for slider show
	if(vvd_flash)
	{
		SWF_RemoveInst(vvd_flash);
		vvd_flash = NULL;
		SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
	}

	if(curr_vd2 != NULL)
	{
		
		vvd_stop(curr_vd2);
		
		vvd_close(curr_vd2);
		
		curr_vd2 = NULL;
	}
}

int swfext_vvd_register(void)
{
	SWFEXT_REGISTER("vvd_Open", vvd_engine_open);
	SWFEXT_REGISTER("vvd_Save", vvd_engine_save);
	SWFEXT_REGISTER("vvd_Close", vvd_engine_close);
	SWFEXT_REGISTER("vvd_Play", vvd_engine_play);
	SWFEXT_REGISTER("vvd_Stop", vvd_engine_stop);
	SWFEXT_REGISTER("vvd_Pause", vvd_engine_pause);
	SWFEXT_REGISTER("vvd_Resume", vvd_engine_resume);
	
	SWFEXT_REGISTER("vvd_GetState", vvd_engine_get_state);
	SWFEXT_REGISTER("vvd_GetPlayMode", vvd_engine_get_play_mode);
	SWFEXT_REGISTER("vvd_SetPlayMode", vvd_engine_set_play_mode);

	SWFEXT_REGISTER("vvd_GetTemplateNum", vvd_engine_get_template_num);
	SWFEXT_REGISTER("vvd_InsertTemplate", vvd_engine_insert_template);
	SWFEXT_REGISTER("vvd_DeleteTemplate", vvd_engine_delete_template);
	SWFEXT_REGISTER("vvd_GetTemplatePath", vvd_engine_get_template_path);
	SWFEXT_REGISTER("vvd_GetThumbPath", vvd_engine_get_thumb_path);
	
	SWFEXT_REGISTER("vvd_GetPhotoNum", vvd_engine_get_photo_num);
	SWFEXT_REGISTER("vvd_InsertPhoto", vvd_engine_insert_photo);
	SWFEXT_REGISTER("vvd_DeletePhoto", vvd_engine_delete_photo);
	SWFEXT_REGISTER("vvd_GetPhotoPath", vvd_engine_get_photo_path);
	
	SWFEXT_REGISTER("vvd_GetMusicNum", vvd_engine_get_music_num);
	SWFEXT_REGISTER("vvd_InsertMusic", vvd_engine_insert_music);
	SWFEXT_REGISTER("vvd_DeleteMusic", vvd_engine_delete_music);
	SWFEXT_REGISTER("vvd_GetMusicPath", vvd_engine_get_music_path);

	SWFEXT_REGISTER("vvd_GetSavePath", vvd_engine_get_save_path);
	SWFEXT_REGISTER("vvd_LoadMovie", vvd_engine_load_movie);

	SWFEXT_REGISTER("vvd_PlayFlash", vvd_engine_play_flash);
	SWFEXT_REGISTER("vvd_StopFlash", vvd_engine_stop_flash);
	SWFEXT_REGISTER("vvd_GetFirstTemplate", vvd_engine_get_first_template);
	SWFEXT_REGISTER("vvd_SetPhotoDir", vvd_engine_set_photo_dir);
	SWFEXT_REGISTER("vvd_PhotoRollover", vvd_engine_photo_rollover);
	SWFEXT_REGISTER("vvd_ResetPhotoIndex", vvd_engine_reset_photo_index);
	
	return 0;
}
#else
int swfext_vvd_register(void)
{
	return 0;
}
#endif	/** MODULE_CONFIG_VVD */