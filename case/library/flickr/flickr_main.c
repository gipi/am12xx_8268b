#include "flickr_main.h"
#include "am_types.h"

//char flickr_api_key[]="28f8381457c65e99d900f9372edb5dba";//"0177dba27572959a7817987746a9bfca";//"28f8381457c65e99d900f9372edb5dba";
//char flickr_shared_secret[]="cf056181f32a2bfb";//"63d9160d9d332b4e";//"cf056181f32a2bfb";
static char account_photo_store_folder[]="my_photos";  ///< the photo under the account will be stored under this folder

char flickr_api_key[]="0177dba27572959a7817987746a9bfca";
char flickr_shared_secret[]="63d9160d9d332b4e";

char flickr_url_str[FLICKR_TMP_BUF_LEN]="";

flickr_task_info_t *flickr_task_info=NULL;
static unsigned long flickr_timestamp=1;
/////function definition////////
static flickcurl_person * flickr_get_user_info_addr(flickr_gdata_t*gdata,int which_contact);
flickcurl_contact *flickr_get_contact_addr(flickr_gdata_t *gdata,int which_contact);
static flickcurl_photoset* flickr_get_photoset_addr(flickr_gdata_t* gdata,int which_contact,int which_photoset);
static int flickr_get_photoset_count(flickr_gdata_t* gdata,int which_contact);
flickcurl_photos_list* flickr_get_photos_list_addr(flickr_gdata_t *gdata,int which_contact,int which_photoset);
flickcurl_photo * flickr_get_photo_addr(flickr_gdata_t *gdata,int which_contact,int which_photoset,int which_photo);
char * flickr_get_photoset_cover_url(flickcurl_photoset* photoset,flickr_photosize_e size);
char * flickr_get_photo_url(flickcurl_photo* photo,flickr_photosize_e size);
void flickr_free_photosets_info(flickr_gdata_t* gdata,int which_contact);

extern size_t flickr_func_write_data( void *ptr, size_t size, size_t nmemb, void *userdata);
extern int flickr_func_prog(void *p, double dltotal, double dlnow, double ult, double uln);
extern void * flickr_open_file(flickr_path_t *path,int *err_status);


static void flickr_free_taskinfo(flickr_task_info_t * task_info);
/////////////////////////////
 void flickr_msg_handler(void *user_data, const char *message)
{
	flickr_info("user_data=0x%x,message=%s\n",(int)user_data,message);
}

void  flickr_sem_wait(sem_t *sem)
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
			flickr_info("work_sem_pend: errno:%d\n",errsv);
			return;
		}
	}

	return ;
}

void  flickr_sem_post(sem_t *sem)
{
	int err;
	err = sem_post(sem);
	return;
}

unsigned long __flickr_get_timestamp(){
	struct timeval cur_time;
	if(gettimeofday(&cur_time,NULL)==0)	
		return cur_time.tv_sec*1000000L+cur_time.tv_usec;
	else{
		if(flickr_timestamp>=0xff)
			flickr_timestamp = 1;
		flickr_timestamp++;
		return flickr_timestamp;
	}
}

int  flickr_get_msg(flickr_task_info_t *task_info,flickr_ioctrl_t * req)
{
	int rtn=0;
	rtn = flickr_req_dequeue(task_info,req);
	return rtn;
}

void  *flickr_thread(void *arg)
{	
	int rtn = 0;
	flickr_task_info_t *task_info = (flickr_task_info_t *)arg;
	flickr_ioctrl_t req;
	if(task_info==NULL){
		flickr_err("Thread Gdata=NULL\n");
		return NULL;
	}
	while(1){
		flickr_info("Wait Semi Start----------\n");
		flickr_sem_wait(&task_info->syn_lock.semi_start);
		pthread_testcancel();
		rtn = flickr_get_msg(task_info,&req);
		flickr_info("Get Msg idx=%d\n",rtn);
		if(rtn>=0){
			switch(req.iocmd){
				case FLICKR_CMD_DOWNLOADPHOTO:
					if(req.para_size == sizeof(flickr_download_info_t)){
						flickr_download_info_t *down_info = (flickr_download_info_t *)req.para;
						flickr_download_photo(down_info->gdata,down_info);
					}
					break;
				case FLICKR_CMD_STOPDOWNLOAD:
					break;
				case FLICKR_CMD_AUTH:
					if(req.para_size == sizeof(flickr_gdata_t)){
						flickr_gdata_t *gdata = (flickr_gdata_t*)req.para;
						flickr_auth(gdata);
					}
					break;
				case FLICKR_CMD_STOPAUTH:
					break;
				default:
					flickr_err("Not Supprot yet CMD=%d\n",req.iocmd);
					break;
			}
			flickr_req_done(task_info,rtn);
		}	
	}
	pthread_exit((void*)rtn);
	return NULL;
}



int flickr_create_thread(flickr_task_info_t *task_info)
{
	int rtn = 0;
	pthread_t  tid;
	int arg=0;

	if(sem_init(&task_info->syn_lock.semi_start,0,0)==-1){
		flickr_err("Sem init error");
		goto CREATE_THREAD_END;
	}

	if(sem_init(&task_info->syn_lock.semi_end,0,0)==-1){
		sem_destroy(&task_info->syn_lock.semi_start);
		flickr_err("Sem init error");
		goto CREATE_THREAD_END;
	}

	if(sem_init(&task_info->syn_lock.semi_req,0,0)==-1){
		sem_destroy(&task_info->syn_lock.semi_start);
		sem_destroy(&task_info->syn_lock.semi_end);
		flickr_err("Sem init error");
		goto CREATE_THREAD_END;
	}

		
	rtn = pthread_create(&tid, NULL,flickr_thread,task_info);

	if(rtn){
		flickr_err("Create Flickr thread error!");
		sem_destroy(&task_info->syn_lock.semi_start);
		sem_destroy(&task_info->syn_lock.semi_end);
		sem_destroy(&task_info->syn_lock.semi_req);
		goto CREATE_THREAD_END;
	}

	flickr_sem_post(&task_info->syn_lock.semi_req);
	
	task_info->thread_id = tid;
	task_info->is_thread_run = 1;
	
CREATE_THREAD_END:
	return rtn;
}

int flickr_thread_exit(flickr_task_info_t *task_info)
{
	void * thread_ret=NULL;
	if(task_info && task_info->is_thread_run){
		pthread_cancel(task_info->thread_id);
		pthread_join(task_info->thread_id,&thread_ret);

		sem_destroy(&task_info->syn_lock.semi_start);
		sem_destroy(&task_info->syn_lock.semi_end);
		sem_destroy(&task_info->syn_lock.semi_req);
		task_info->is_thread_run = 0;
	}
	return 0;
}


static flickr_task_info_t * flickr_init_taskinfo()
{
	flickr_task_info_t * task_info=NULL;
	task_info = (flickr_task_info_t *)flickr_malloc();
	if(task_info){
		memset(task_info,0,sizeof(flickr_task_info_t));
		if(flickr_create_thread(task_info)!=0){
			flickr_free_taskinfo(task_info);
			task_info=NULL;
			goto INIT_TASK_INFO_END;
		}
		flickr_req_init_queue(task_info);
	}
INIT_TASK_INFO_END:
	return task_info;
}

void flickr_free_taskinfo(flickr_task_info_t * task_info)
{
	if(task_info){
		flickr_thread_exit(task_info);
		flickr_free((char*)task_info);
	}
}

flickr_task_info_t *flickr_get_task_info_global()
{
	if(flickr_task_info==NULL){
		flickr_task_info = flickr_init_taskinfo();
		if(!flickr_task_info)
			flickr_err("Get TaskInfo Error!");
	}
	return flickr_task_info;
}

void flickr_free_task_info_global()
{
	if(flickr_task_info){
		flickr_free_taskinfo(flickr_task_info);
		flickr_task_info = NULL;
	}
}

/**
@brief it is used when the authentication
@param[in] gdata : get from flickr_init_gdata()
@param[in] perms_type : see flickr_perms_type_e
@return NULL
**/
void flickr_set_perms(flickr_gdata_t *gdata,flickr_perms_type_e perms_type)
{
	if(gdata){
		switch(perms_type){
			case PERMS_READ:
				flickr_fill_atomic("read",&gdata->perms);
				break;
			case PERMS_WRITE:
				flickr_fill_atomic("write",&gdata->perms);
				break;
			case PERMS_DELETE:
				flickr_fill_atomic("delete",&gdata->perms);
				break;
			default: 
				flickr_fill_atomic("read",&gdata->perms);
		}
	}
}



/**
@brief get userinfo
**/
static char * flickr_getinfo_userinfo(flickr_gdata_t *gdata,flickr_info_get_cmd_e cmd,flickr_info_query_t *infor_query)
{
	char * value = NULL;
	flickcurl_person * user_info=NULL;
	user_info = flickr_get_user_info_addr(gdata,infor_query->which_contact);
	if(user_info){
		switch(cmd){
			case GET_USER_REALNAME:				///< user's realname
				if(user_info->fields[PERSON_FIELD_realname].type != VALUE_TYPE_NONE)
					value = user_info->fields[PERSON_FIELD_realname].string;
				break;
			case GET_USER_USERNAME:			///< user's username
				if(user_info->fields[PERSON_FIELD_username].type != VALUE_TYPE_NONE)
					value = user_info->fields[PERSON_FIELD_username].string;
				break;
			default : 
				infor_query->err_status = -1;
		}
	}
	else
		infor_query->err_status = -1;
	return value;
}

static char * flickr_getinfo_getcontactinfo(flickr_gdata_t *gdata,flickr_info_get_cmd_e cmd,flickr_info_query_t *infor_query)
{
	flickcurl_contact * contact = NULL;
	char * value=NULL;
	contact = flickr_get_contact_addr(gdata,infor_query->which_contact);
	if(contact){
		switch(cmd){
			case GET_CONTACT_REALNAME:			///< contact's realname
				value = contact->realname;
				break;
			case GET_CONTACT_USERNAME:			///< contact's username
				value = contact->username;
				break;
			default:
				infor_query->err_status = -1;
		}
	}
	else
		infor_query->err_status = -1;
	return value;
}

static char *flickr_getinfo_getphotosetinfo(flickr_gdata_t *gdata,flickr_info_get_cmd_e cmd,flickr_info_query_t *infor_query)
{
	flickcurl_photoset* photoset=NULL;
	char * value = NULL;
	if(cmd==GET_PHOTOSET_COUNT)///< how many photosets in the account, should change to int by the caller
		value = (char*)flickr_get_photoset_count(gdata,infor_query->which_contact);
	else{
		photoset = flickr_get_photoset_addr(gdata,infor_query->which_contact,infor_query->which_photoset);
		if(photoset){
			switch(cmd){
				case GET_PHOTOSET_ID:					///< photoset id
					value = photoset->id;
					break;
				case GET_PHOTOSET_PHOTOS_COUNT: 		///< photos' num in a photoset
					value = (char*)photoset->photos_count; 	///< should change to int by the caller
					break;
				case GET_PHOTOSET_TITLE:				///< title of the photoset		
					value = photoset->title;
					break;
				case GET_PHOTOSET_DESCRIPTION:			///< description of the photoset
					value = photoset->description;
					break;
				default:
					infor_query->err_status = -1;	
			}
		}
		else
			infor_query->err_status = -1;
	}
	return value;
}

static char * flickr_getinfo_photoinfo(flickr_gdata_t *gdata,flickr_info_get_cmd_e cmd,flickr_info_query_t *infor_query)
{
	flickcurl_photo *photo=NULL;
	char *value = NULL;
	photo = flickr_get_photo_addr(gdata,infor_query->which_contact,infor_query->which_photoset,infor_query->which_photo);
	if(photo){
		switch(cmd){
			case GET_PHOTO_TITLE:					///< photo title
				if(photo->fields[PHOTO_FIELD_title].type != VALUE_TYPE_NONE)
					value = photo->fields[PHOTO_FIELD_title].string;
				break;
			case GET_PHOTO_DESCRIPTION:				///< photo description
				if(photo->fields[PHOTO_FIELD_description].type != VALUE_TYPE_NONE)
					value = photo->fields[PHOTO_FIELD_description].string;
				break;
			case GET_PHOTO_ID:						///< photo id
				value = photo->id;
				break;
			case GET_PHOTO_DATESPOSTED:				///< photo dates posted
				if(photo->fields[PHOTO_FIELD_dates_posted].type != VALUE_TYPE_NONE)
					value = photo->fields[PHOTO_FIELD_dates_posted].string;
				break;
			case GET_PHOTO_DATESTAKEN:				///< photo dates taken
				if(photo->fields[PHOTO_FIELD_dates_taken].type != VALUE_TYPE_NONE)
					value = photo->fields[PHOTO_FIELD_dates_taken].string;
				break;
			case GET_PHOTO_DATESLASTUPDATE:			///< photo dates last update
				if(photo->fields[PHOTO_FIELD_dates_lastupdate].type != VALUE_TYPE_NONE)
					value = photo->fields[PHOTO_FIELD_dates_lastupdate].string;
				break;
			default:
				infor_query->err_status = -1;
		}
	}
	else
		infor_query->err_status = -1;
	return value;
	
}

static char * flickr_getinfo_geturl(flickr_gdata_t *gdata,flickr_info_get_cmd_e cmd,flickr_info_query_t *infor_query)
{
	char *value=NULL;
	char *url=NULL;
	switch(cmd){
		case GET_URL_COVER_SMALL:				///< get photoset cover url, small picture
		case GET_URL_COVER_HUGE:				///< get photoset cover url, huge picture
			{	
				flickcurl_photoset* photoset=flickr_get_photoset_addr(gdata,infor_query->which_contact,infor_query->which_photoset);
				if(photoset){
					if(cmd==GET_URL_COVER_SMALL)
						url = flickr_get_photoset_cover_url(photoset,FLICKR_PHOTOSIZE_T);
					else
						url = flickr_get_photoset_cover_url(photoset,FLICKR_PHOTOSIZE_Z);
				}
			}
			break;
		case GET_URL_PHOTO_SMALL:				///< get photo url, small picture
		case GET_URL_PHOTO_HUGE:				///< get photo url, huge picture
			{
				flickcurl_photo * photo = flickr_get_photo_addr(gdata,infor_query->which_contact,infor_query->which_photoset,infor_query->which_photo);
				if(photo){
					if(cmd==GET_URL_PHOTO_SMALL)
						url = flickr_get_photo_url(photo,FLICKR_PHOTOSIZE_T);
					else
						url = flickr_get_photo_url(photo,FLICKR_PHOTOSIZE_Z);
				}
			}
			break;
		default:
			break;
	}
	if(url){
		memcpy(flickr_url_str,url,FLICKR_TMP_BUF_LEN);
		flickr_free(url);
		value = flickr_url_str;
	}
	return value;
}


/**
@brief get user nsid of the account see flickr.people.findByEmail 
@param[in] gdata: get from flickr_init_gdata(), need authentication
**/
static int flickr_get_usernsid_me(flickr_gdata_t *gdata)
{
	char *user_nsid;
	int rtn=FLICKR_RTN_OK;
	flickcurl *fc = gdata->fc;
	if(gdata->user_nsid.data!=NULL)
		return rtn;
	if(gdata->user_email.data==NULL)
		return FLICKR_GET_USERNSID_ERR;
	user_nsid = flickcurl_people_findByEmail(fc,gdata->user_email.data);
	flickr_info("user_nsid=%s",user_nsid);
	if(user_nsid){
		flickr_fill_atomic(user_nsid,&gdata->user_nsid);
		free(user_nsid);
		rtn = 0;
	}
	else
		rtn = FLICKR_GET_USERNSID_ERR;
	return rtn;
}

/**
@brief get user nisd
@param[in] gdata: get from flickr_init_gdata(), need authentication
@param[in] which_contact: -1, will get the calling account itself
		others:  the friends add by the calling account, should less than gdata->contacts.counts
**/
static char * flickr_get_usernsid(flickr_gdata_t *gdata,int which_contact)
{
	int rtn = FLICKR_RTN_OK;
	char *nsid=NULL;
	if(which_contact<0){
		rtn = flickr_get_usernsid_me(gdata);
		if(rtn==FLICKR_RTN_OK)
			nsid = gdata->user_nsid.data;
	}
	else{
		if(which_contact>=gdata->contacts.counts){
			flickr_err("Contact Index is Error!");
			goto GET_USERNSID_END;
		}
		nsid =(char*)(gdata->contacts.pcontacts[which_contact]->nsid);
	}
GET_USERNSID_END:
	return nsid;
}


/**
@brief get the address of the user_info
@param[in] gdata: get from flickr_init_gdata(), need authentication
@param[in] which_contact : specified which contact will be get
@return: the pointer to the flickcurl_person
@warning : call flickr_get_user_info() first
**/
static flickcurl_person * flickr_get_user_info_addr(flickr_gdata_t*gdata,int which_contact)
{
	flickcurl_person * user_info=NULL;
	if(which_contact<0){
		if(gdata->infors.user_info)
			user_info = gdata->infors.user_info;
		else
			flickr_err("Call flicrk_get_user_info first!");
	}
	else{
		if(which_contact<gdata->contacts.counts){
			if(gdata->contacts.infors_array[which_contact].user_info)
				user_info = gdata->contacts.infors_array[which_contact].user_info;
			else
				flickr_err("Call flicrk_get_user_info first!");
		}
	}
	return user_info;
}


/**
@brief get the pointer of the photoset which is specified
@param[in] gdata: get from flickr_init_gdata(), need authentication
@param[in] which_contact: -1, will get the calling account itself
		others:  the friends add by the calling account, should less than gdata->contacts.counts
@param[in] which_photoset: specified which photoset will be get
	range from [0~ photosets.counts)
@warning : calling the flickr_get_photosets_info() first
**/
static flickcurl_photoset* flickr_get_photoset_addr(flickr_gdata_t* gdata,int which_contact,int which_photoset)
{
	flickcurl_photoset* photoset=NULL;
	flickr_photosets_t* photosets=NULL;
	if(which_contact<0){
		if(which_photoset<gdata->infors.photosets.counts){
			photoset = gdata->infors.photosets.photoset_list[which_photoset];
		}
		else
			flickr_info("Photoset Index is out of range,max=%d!",gdata->infors.photosets.counts);
	}
	else{
		if(which_contact<gdata->contacts.counts){
			photosets = &(gdata->contacts.infors_array[which_contact].photosets);
			if(which_photoset<photosets->counts)
				if(photosets->photoset_list==NULL)
					flickr_info("Please Call get flickr_get_photosets_info first!");
				else
					photoset = photosets->photoset_list[which_photoset];
			else
				flickr_info("Photoset Index is out of range,max=%d!",photosets->counts);
		}
		else
			flickr_info("Contact Index is out of range,max=%d!",gdata->contacts.counts);
	}
	return photoset;
}

/**
@brief : get the photoset number in specified account
@param[in] gdata: get from flickr_init_gdata(), need authentication
@param[in] which_contact: -1, will get the calling account itself
		others:  the friends add by the calling account, should less than gdata->contacts.counts
@return :
 	the number of photosets in the account
**/
static int flickr_get_photoset_count(flickr_gdata_t* gdata,int which_contact)
{
	int photoset_count=0;
	flickr_photosets_t* photosets=NULL;
	if(which_contact<0){
		photoset_count = gdata->infors.photosets.counts;
	}
	else{
		if(which_contact<gdata->contacts.counts){
			photosets = &(gdata->contacts.infors_array[which_contact].photosets);
			photoset_count = photosets->counts;
		}
		else
			flickr_info("Contact Index is out of range,max=%d!",gdata->contacts.counts);
	}
	return photoset_count;
}

/**
@brief get the photoset id
@param[in] gdata: get from flickr_init_gdata(), need authentication
@param[in] which_contact: -1, will get the calling account itself
		others:  the friends add by the calling account, should less than gdata->contacts.counts
@param[in] which_photoset: specified which photoset will be get
	range from [0~ photosets.counts)
**/
static char* flickr_get_photoset_id(flickr_gdata_t *gdata,int which_contact,int which_photoset)
{
	char * photoset_id=NULL;
	flickcurl_photoset* photoset=NULL;
	photoset = flickr_get_photoset_addr(gdata,which_contact,which_photoset);
	if(photoset)
		photoset_id = photoset->id;
	return photoset_id;
}




/**
@brief get the pointer to the photos of a photoset
@param[in] gdata: get from flickr_init_gdata(), need authentication
@param[in] which_contact: -1, will get the calling account itself
		others:  the friends add by the calling account, should less than gdata->contacts.counts
@param[in] which_photoset: specified which photoset will be get
	range from [0~ photosets.counts)
**/
static flickcurl_photos_list* flickr_get_photoset_photos_addr(flickr_gdata_t *gdata,int which_contact,int which_photoset)
{
	flickcurl_photos_list* photos_list = NULL;
	if(which_contact<0){
		if(which_photoset< gdata->infors.photosets.counts){
			photos_list = gdata->infors.photosets.photos_list_array[which_photoset];
		}
		else
			flickr_info("Photoset Index is out of range,max=%d!",gdata->infors.photosets.counts);
	}
	else{
		if(which_contact<gdata->contacts.counts){
			if(which_photoset<gdata->contacts.infors_array[which_contact].photosets.counts){
				photos_list = gdata->contacts.infors_array[which_contact].photosets.photos_list_array[which_photoset];
			}
			else
				flickr_info("Photoset Index is out of range,max=%d!",gdata->infors.photosets.counts);
		}
		else
			flickr_info("Contact Index is out of range,max=%d!",gdata->contacts.counts);
	}
	return photos_list;
}

/**
@brief get the photo list addr
**/
flickcurl_photos_list* flickr_get_photos_list_addr(flickr_gdata_t *gdata,int which_contact,int which_photoset)
{
	flickcurl_photos_list* photos_list=NULL;
	if(which_contact<0){///< get the photo of the owner
		if(which_photoset<0){///< get the photo in the photos_info
			photos_list = gdata->infors.photos_list;
			if(photos_list==NULL)
				flickr_err("Call flickr_get_photos_info first!");
		}
		else{///< get the photo in the photoset
			if(gdata->infors.photosets.photos_list_array){	
				if(gdata->infors.photosets.photos_list_array[which_photoset]){
					photos_list = gdata->infors.photosets.photos_list_array[which_photoset];
					if(photos_list==NULL)
						flickr_err("Call flickr_get_photos_info first!");
				}
				else
					flickr_err("Call flickr_get_photosets_photos_info first!");
			}
			else
				flickr_err("Call flickr_get_photosets_info first!");
		}
	}
	else{///< get the photo of the contact
		if(which_photoset<0){
			photos_list = gdata->contacts.infors_array[which_contact].photos_list;
			if(photos_list==NULL)
				flickr_err("Call flickr_get_photos_info first!");
		}
		else{
			if(gdata->contacts.infors_array){
				if(gdata->contacts.infors_array[which_contact].photosets.photos_list_array){
					photos_list = gdata->contacts.infors_array[which_contact].photosets.photos_list_array[which_photoset];
					if(photos_list==NULL)
						flickr_err("Call flickr_get_photos_info first!");
				}
				else
					flickr_err("Call flickr_get_photosets_photos_info first!");
			}
			else
				flickr_err("Call flickr_get_contacts first!");
		}
	}
	return photos_list;
}

/**
@brief get the photo addr
@param[in] gdata: get from flickr_init_gdata(), need authentication
@param[in] which_contact: -1, will get the calling account itself
		others:  the friends add by the calling account, should less than gdata->contacts.counts
@param[in] which_photoset: if which_photoset>=0 specified which photoset will be get
	range from [0~ photosets.counts), if which_photoset=-1, it will get the photo in the account
@param[in] which_photo : which photo to be gotten, range from [0,photos_num), photos_num is the value returned from
	flickr_get_info when cmd is GET_PHOTOS_COUNT
**/
flickcurl_photo * flickr_get_photo_addr(flickr_gdata_t *gdata,int which_contact,int which_photoset,int which_photo)
{
	flickcurl_photos_list* photos_list=NULL;
	flickcurl_photo *photo=NULL;
	photos_list = flickr_get_photos_list_addr(gdata,which_contact,which_photoset);
	if(photos_list){
		if(which_photo< photos_list->total_count)
			photo = photos_list->photos[which_photo];
	}
	return photo;
}

/**
@brief get the url of a  photo ,use flickr_free() to release the space
@param[in] photo: a pointer to a photo 
@param[in] size : see flickr_photosize_e
@return 
	the photo url
@warning remember to call flickr_free to free the photo url
**/
char * flickr_get_photo_url(flickcurl_photo* photo,flickr_photosize_e size)
{
	char * url_str=NULL;
	char *tmp_str=NULL;
	int len_str=0;
	if(photo){
		tmp_str = (char*)flickr_photo_url_append(photo->fields[PHOTO_FIELD_farm].string,photo->fields[PHOTO_FIELD_server].string,
			photo->id,photo->fields[PHOTO_FIELD_secret].string,size);
		if(tmp_str){
			len_str = strlen(tmp_str);
			url_str = (char*)flickr_malloc(len_str+1);
			if(url_str){
				memset(url_str,0,len_str+1);
				strcpy(url_str,tmp_str);
				flickr_info("<photo Url>%s",url_str);
			}
			else
				flickr_err("Malloc PhotoUrl Error!");
		}
	}
	return url_str;
}

/**
@brief get the cover of the photoset
@param[in] photoset : the photoset which the cover included
@param[in] size : which size does the cover has
**/
char * flickr_get_photoset_cover_url(flickcurl_photoset* photoset,flickr_photosize_e size)
{
	char * url_str=NULL;
	url_str = (char*)flickr_malloc(FLICKR_TMP_BUF_LEN);
	if(url_str){
		memset(url_str,0,FLICKR_TMP_BUF_LEN);
	 	sprintf(url_str,"http://farm%d.static.flickr.com/%d/%s_%s.jpg",photoset->farm,\
		photoset->server,photoset->primary,photoset->secret);
		flickr_info("<cover_url>%s",url_str);
	}
	return url_str;
}	



/**
@brief free photosets
@param[in] gdata: get from flickr_init_gdata(), need authentication
@param[in] which_contact: -1, will get the calling account itself
		others:  the friends add by the calling account, should less than gdata->contacts.counts
**/
static void flickr_free_contact_photosets(flickr_gdata_t *gdata,int which_contact)
{
	int i=0;
	int photoset_num=0;	
	if(which_contact<0){
		photoset_num = gdata->infors.photosets.counts;
		for(i=0;i<photoset_num;i++)
			flickr_free_photosets_photos_info(gdata,which_contact,i);
		flickr_free_photosets_info(gdata,which_contact);
		if(gdata->infors.photosets.photos_list_array){
			flickr_free(gdata->infors.photosets.photos_list_array);
			gdata->infors.photosets.photos_list_array = NULL;
		}
		gdata->infors.photosets.counts = 0;
	}
	else{
		if(which_contact<gdata->contacts.counts){
			photoset_num = gdata->contacts.infors_array[which_contact].photosets.counts;
			flickr_free_photosets_info(gdata,which_contact);
			for(i=0;i<photoset_num;i++)
				flickr_free_photosets_photos_info(gdata,which_contact,i);
			if(gdata->contacts.infors_array && gdata->contacts.infors_array[which_contact].photosets.photos_list_array){
				flickr_free(gdata->contacts.infors_array[which_contact].photosets.photos_list_array);
				gdata->contacts.infors_array[which_contact].photosets.photos_list_array = NULL;
			}
			gdata->contacts.infors_array[which_contact].photosets.counts = 0;
		}
	}
	
}


/**
@brief free infors
@param[in] gdata: get from flickr_init_gdata(), need authentication
@param[in] which_contact: -1, will get the calling account itself
		others:  the friends add by the calling account, should less than gdata->contacts.counts
**/
static void flickr_free_infors(flickr_gdata_t *gdata,int which_contact)
{
	flickr_free_user_info(gdata,which_contact);
	flickr_free_photos_info(gdata,which_contact);
	flickr_free_contact_photosets(gdata,which_contact);
}

/**
@brief free infors of the contacts
@param[in] gdata: get from flickr_init_gdata(), need authentication
**/
static void flikcr_free_contact_infors(flickr_gdata_t *gdata)
{
	int i=0;
	if(gdata->contacts.infors_array){
		for(i=0;i<gdata->contacts.counts;i++){
			flickr_free_infors(gdata,i);
		}
		flickr_free((char*)gdata->contacts.infors_array);
		gdata->contacts.infors_array=NULL;
	}
}


/**
@brief the the address of a specified contact
@param[in] gdata : get from flickr_init_gdata(), need authentication
@param[in] which_contact : specified which contact to get
@return 
	the pointer to the specifed contact 
@warning : call the flickr_get_contacts() first
**/
flickcurl_contact *flickr_get_contact_addr(flickr_gdata_t *gdata,int which_contact)
{
	flickcurl_contact *contact=NULL;
	if(gdata){
		if(gdata->contacts.pcontacts){
			if(which_contact<gdata->contacts.counts){
				contact = gdata->contacts.pcontacts[which_contact];
			}
			else
				flickr_err("Index Error which_contact=%d, max=%d",which_contact,gdata->contacts.counts);
		}
		else
			flickr_err("Call flickr_get_contacts first!");
	}
	return contact;
}



#if 1

/**
@brief init the gdata, remember to call the flickr_free_gdata() to free the space
@param[in] NULL
@return: the pointer to the gdata 
**/
EXPORT_SYMBOL
flickr_gdata_t *flickr_init_gdata()
{
	flickcurl *fc=NULL;
	flickr_gdata_t *gdata=NULL;
	flickr_task_info_t *task_info=NULL;
	gdata = (flickr_gdata_t *)flickr_malloc(sizeof(flickr_gdata_t));
	if(gdata){
		memset(gdata,0,sizeof(flickr_gdata_t));
		task_info = flickr_get_task_info_global(); /// get the task_info, all the flickr instance has the same task_info
		if(task_info){
			task_info->cnt_flickr_instance++;
		}
		gdata->task_info = task_info;
		flickcurl_init(); /* optional static initialising of resources */
		fc=flickcurl_new();
		if(fc){
			flickcurl_set_api_key(fc,flickr_api_key);
			flickcurl_set_shared_secret(fc,flickr_shared_secret);
			flickcurl_set_error_handler(fc,flickr_msg_handler,NULL);
		}
		gdata->fc = fc;
		
	}
	return gdata;
}

/**
@brief release the space which is malloced when calling the flickr_init_gdata()
@param[in]  gdata:	the pointer got from the flickr_init_gdata()
**/
EXPORT_SYMBOL
void flickr_free_gdata(flickr_gdata_t *gdata)
{
	if(gdata){
		if(gdata->task_info){
			if(gdata->task_info->cnt_flickr_instance)
				gdata->task_info->cnt_flickr_instance--;
			if(gdata->task_info->cnt_flickr_instance==0)
				flickr_free_task_info_global(); ///< if it is the last instance, free the global task info
			gdata->task_info=NULL;
		}
		if(gdata->user_email.data)
			flickr_free_atomic(&gdata->user_email);
		if(gdata->user_pwd.data)
			flickr_free_atomic(&gdata->user_pwd);	
		if(gdata->user_nsid.data)
			flickr_free_atomic(&gdata->user_nsid);
		if(gdata->frob.data)
			flickr_free_atomic(&gdata->frob);
		if(gdata->perms.data)
			flickr_free_atomic(&gdata->perms);
		if(gdata->auth_frob.data)
			flickr_free_atomic(&gdata->auth_frob);
		if(gdata->auth_token.data)
			flickr_free_atomic(&gdata->auth_token);
		if(gdata->fc){
			flickcurl_free(gdata->fc);	
			gdata->fc = NULL;
		}
		flickr_free_infors(gdata,-1);
		flickr_free_contacts(gdata);
		flickr_free((char*)gdata);
	}
}

/**
@brief authentication, call flickr_set_login_info() before calling this function
@param[in] gdata: get from flickr_init_gdata(), need authentication
**/
EXPORT_SYMBOL
int  flickr_auth(flickr_gdata_t *gdata)
{
	char * frob=NULL;
	char * token=NULL;
	int rtn=FLICKR_RTN_OK;
	flickcurl * fc= gdata->fc;
	frob = flickcurl_auth_getFrob(fc);
	flickr_info("frob=%s",frob);
	if(frob){
		flickr_fill_atomic(frob,&gdata->frob);
		free(frob);
		flickr_set_perms(gdata,PERMS_WRITE);
		rtn = flickr_desktop_auth(gdata);
		if(rtn == FLICKR_RTN_OK){
			flickr_info("<Auth Frob>%s",gdata->auth_frob.data);
			token = flickcurl_auth_getToken(fc,gdata->auth_frob.data);
			flickr_info("Token=%s",token);
			if(token){
				flickr_fill_atomic(token,&gdata->auth_token);
				flickcurl_set_auth_token(fc,token);
				free(token);
			}
			else
				rtn = FLICKR_RTN_TOKEN_ERR;
		}
	}
	else
		rtn = FLICKR_RTN_FROB_ERR;

	if(rtn==FLICKR_RTN_OK)
		rtn = flickr_get_status(rtn,0,FLICKR_HttpStatus_OK);
	gdata->is_auth_cancel = 0;
	gdata->err_status = rtn;
	return rtn;
}


/**
@brief set the login info
@param[in] gdata : get from flickr_init_gdata()
@param[in] user_email : the account name
@param[in] user_pwd: the password
@param[in] act_type : account type , the flickr support three types of account to login see flickr_account_type_e
@return NULL
**/
EXPORT_SYMBOL
void flickr_set_login_info(flickr_gdata_t *gdata,const char*user_email,const char* user_pwd,flickr_account_type_e act_type)
{
	if(gdata==NULL)
		return;
	if(user_email)
		flickr_fill_atomic(user_email,&gdata->user_email);
	if(user_pwd)
		flickr_fill_atomic(user_pwd,&gdata->user_pwd);
	gdata->account_type = act_type;
}


/**
@brief get the information of the user which is specified by user_nsid
@param[in] gdata: get from flickr_init_gdata(), need authentication
@param[in] whichi_contact: -1, will get the calling account itself
		others:  the friends add by the calling account, should less than gdata->contacts.counts
**/
EXPORT_SYMBOL
int flickr_get_user_info(flickr_gdata_t *gdata,int which_contact)
{
	int rtn=FLICKR_RTN_OK;
	flickcurl_person* person=NULL;
	char * nsid = NULL;
	nsid = flickr_get_usernsid(gdata,which_contact);
	if(nsid){
		person = flickcurl_people_getInfo(gdata->fc,nsid);	
		if(person){
			if(which_contact<0)
				gdata->infors.user_info =person;
			else
				gdata->contacts.infors_array[which_contact].user_info = person;
			flickr_print_user_info(person);
		}
		else{
			flickr_err("Get User Info Err!");
			rtn = FLICKR_GET_USERINFO_ERR;
		}	
	}
	else
		rtn = FLICKR_GET_USERNSID_ERR;
GET_USER_INFO_END:
	return rtn;
}

/**
@brief get information of the user
@param[in] gdata: get from flickr_init_gdata(), need authentication
@param[in] whichi_contact: -1, will get the calling account itself
		others:  the friends add by the calling account, should less than gdata->contacts.counts
**/
EXPORT_SYMBOL
void flickr_free_user_info(flickr_gdata_t*gdata,int which_contact)
{
	if(which_contact<0){
		if(gdata->infors.user_info){
			flickcurl_free_person(gdata->infors.user_info);
			gdata->infors.user_info = NULL;
		}
	}
	else{
		if(which_contact<gdata->contacts.counts){
			if(gdata->contacts.infors_array[which_contact].user_info)
				flickcurl_free_person(gdata->contacts.infors_array[which_contact].user_info);
			gdata->contacts.infors_array[which_contact].user_info = NULL;
		}
	}
}

/**
@brief get photos information of the specified user, all the photos under the user account will be gotten
@param[in] gdata: get from flickr_init_gdata(), need authentication
@param[in] which_contact: -1, will get the calling account itself
		others:  the friends add by the calling account, should less than gdata->contacts.counts
@param[in] priv_lever : see the flickr_privacy_fliter_e,
	if the which_contact == -1, the priv_level will take effect
	else it will consider as the FLICKR_PUBLIC
**/
EXPORT_SYMBOL
int flickr_get_photos_info(flickr_gdata_t *gdata,int which_contact,flickr_privacy_fliter_e priv_level)
{
	int rtn = FLICKR_RTN_OK;
	flickcurl_photos_list* photos_list=NULL;
	char *nsid=NULL;
	nsid =  flickr_get_usernsid(gdata,which_contact);
	if(nsid){
		photos_list = flickcurl_people_getPhotos_params(gdata->fc,nsid,FLICKR_SEARCE_RESTRICTED,NULL,NULL,NULL,NULL,\
			FLICKR_ALL,priv_level,NULL);
		if(photos_list){
			if(which_contact<0)
				gdata->infors.photos_list = photos_list;
			else
				gdata->contacts.infors_array[which_contact].photos_list = photos_list;
			flickr_print_photos(photos_list);
		}
		else
			rtn = FLICKR_GET_PHOTOS_ERR;
	}
	gdata->err_status = rtn;
	return rtn;
}

/**
@brief free the photos information
@param[in] gdata: get from flickr_init_gdata(), need authentication
@param[in] which_contact: -1, will get the calling account itself
		others:  the friends add by the calling account, should less than gdata->contacts.counts
**/
EXPORT_SYMBOL
void flickr_free_photos_info(flickr_gdata_t *gdata,int which_contact)
{
	if(which_contact<0){
		if(gdata->infors.photos_list){
			flickcurl_free_photos_list(gdata->infors.photos_list);
			gdata->infors.photos_list = NULL;
		}
	}
	else{
		if(which_contact<gdata->contacts.counts){
			if(gdata->contacts.infors_array[which_contact].photos_list)
				flickcurl_free_photos_list(gdata->contacts.infors_array[which_contact].photos_list);
			gdata->contacts.infors_array[which_contact].photos_list = NULL;
		}
	}	
	
}


/**
@brief get the photosets of the specified user
@param[in] gdata: get from flickr_init_gdata(), need authentication
@param[in] which_contact: -1, will get the calling account itself
		others:  the friends add by the calling account, should less than gdata->contacts.counts
**/
EXPORT_SYMBOL
int flickr_get_photosets_info(flickr_gdata_t *gdata,int which_contact)
{
	int rtn = FLICKR_RTN_OK;
	int i=0;
	char * nsid=NULL;
	flickcurl_photoset** photoset_list = NULL;
	flickcurl_photos_list** photos_list_array=NULL;
	nsid = flickr_get_usernsid(gdata,which_contact);
	if(nsid){
		photoset_list = flickcurl_photosets_getList(gdata->fc,gdata->user_nsid.data);
		if(photoset_list){
			for(i = 0; photoset_list[i]; i++) {
				fprintf(stderr, "%s: Photoset %d\n", __FILE__, i);
				flickr_print_photoset(photoset_list[i]);
			}
			if(which_contact<0){
				gdata->infors.photosets.counts = i;
				gdata->infors.photosets.photoset_list = photoset_list;
			}
			else{
				gdata->contacts.infors_array[which_contact].photosets.counts = i;
				gdata->contacts.infors_array[which_contact].photosets.photoset_list = photoset_list;
			}
			if(i!=0){
				photos_list_array=(flickcurl_photos_list**)flickr_malloc(sizeof(flickcurl_photos_list*)*i);
				if(photos_list_array==NULL){
					flickr_free_photosets_info(gdata,which_contact);
					rtn = FLICKR_RTN_MALLOC_FAIL;
				}
				else{
					memset(photos_list_array,0,sizeof(flickcurl_photos_list*)*i);
					if(which_contact<0)
						gdata->infors.photosets.photos_list_array = photos_list_array;
					else
						gdata->contacts.infors_array[which_contact].photosets.photos_list_array=photos_list_array;
				}
			}
		}
		else
			rtn = FLICKR_GET_PHOHOTSETS_ERR;
	}
	else
		rtn = FLICKR_GET_USERNSID_ERR;
	return rtn;
}

/**
@brief free the photosets information
@param[in] gdata: get from flickr_init_gdata(), need authentication
@param[in] which_contact: -1, will get the calling account itself
		others:  the friends add by the calling account, should less than gdata->contacts.counts
**/
EXPORT_SYMBOL
void flickr_free_photosets_info(flickr_gdata_t* gdata,int which_contact)
{
	if(which_contact){
		if(gdata->infors.photosets.photoset_list){
			flickcurl_free_photosets(gdata->infors.photosets.photoset_list);
			gdata->infors.photosets.photoset_list = NULL;
		}
	}
	else{
		if(which_contact<gdata->contacts.counts){
			if(gdata->contacts.infors_array[which_contact].photosets.photoset_list)
				flickcurl_free_photosets(gdata->contacts.infors_array[which_contact].photosets.photoset_list);
			gdata->contacts.infors_array[which_contact].photosets.photoset_list = NULL;
		}
	}
}


/**
@brief get the photos in a photoset
@param[in] gdata: get from flickr_init_gdata(), need authentication
@param[in] which_contact: -1, will get the calling account itself
		others:  the friends add by the calling account, should less than gdata->contacts.counts
@param[in] which_photoset: specified which photoset will be get
	range from [0~ photosets.counts)
@param[in] priv_level: see flickr_privacy_fliter_e
@return 
	see flickr_rtn_e
@warning: call the flickr_get_photosets_info befor calling this function to fetch the photoset_id information
**/
EXPORT_SYMBOL
int flickr_get_photosets_photos_info(flickr_gdata_t *gdata,int which_contact,int which_photoset,flickr_privacy_fliter_e priv_level)
{
	int rtn= FLICKR_RTN_OK;
	flickcurl_photos_list_params list_params;
	char * photoset_id = NULL;
	flickcurl_photos_list* photos_list = NULL;
	photoset_id = flickr_get_photoset_id(gdata,which_contact,which_photoset);
	if(photoset_id){
		flickcurl_photos_list_params_init(&list_params);
		photos_list = flickcurl_photosets_getPhotos_params(gdata->fc,photoset_id,priv_level,&list_params);
		if(photos_list){
			if(which_contact<0){
				gdata->infors.photosets.photos_list_array[which_photoset] = photos_list;
			}
			else{
				gdata->contacts.infors_array[which_contact].photosets.photos_list_array[which_photoset] = photos_list;
			}
			flickr_print_photos(photos_list);
		}
		else
			rtn = FLICKR_GET_PHOTOSET_PHOTOS_ERR;
	}
	else
		rtn = FLICKR_GET_PHOTOSETID_ERR;
	return rtn;
}



/**
@brief free the photos information in a photoset
@param[in] gdata: get from flickr_init_gdata(), need authentication
@param[in] which_contact: -1, will get the calling account itself
		others:  the friends add by the calling account, should less than gdata->contacts.counts
@param[in] which_photoset: specified which photoset will be get
	range from [0~ photosets.counts)
**/
EXPORT_SYMBOL
void flickr_free_photosets_photos_info(flickr_gdata_t *gdata,int which_contact,int which_photoset)
{
	flickcurl_photos_list* photos_list = NULL;
	if(which_contact<0){
		flickr_info("whichPhotoset=%d,counts=%d",which_photoset,gdata->infors.photosets.counts);
		if(which_photoset< gdata->infors.photosets.counts){
			photos_list = gdata->infors.photosets.photos_list_array[which_photoset];
			flickr_info("photos_list===0x%x",(unsigned int)photos_list);
			if(photos_list)
				flickcurl_free_photos_list(photos_list);
			gdata->infors.photosets.photos_list_array[which_photoset] = NULL;
		}
	}
	else{
		if(which_contact<gdata->contacts.counts){
			if(which_photoset<gdata->contacts.infors_array[which_contact].photosets.counts){
				photos_list = gdata->contacts.infors_array[which_contact].photosets.photos_list_array[which_photoset];
				if(photos_list)
					flickcurl_free_photos_list(photos_list);
				 gdata->contacts.infors_array[which_contact].photosets.photos_list_array[which_photoset] = NULL;
			}
			else
				flickr_info("Photoset Index is out of range,max=%d!",gdata->infors.photosets.counts);
		}
		else
			flickr_info("Contact Index is out of range,max=%d!",gdata->contacts.counts);
	}
}


/**
@brief free the space of the contact occupied
@param[in] gdata : get from flickr_init_gdata(), need authentication
**/
EXPORT_SYMBOL
void flickr_free_contacts(flickr_gdata_t *gdata)
{
	if(gdata){
		if(gdata->contacts.pcontacts){
			flickcurl_free_contacts(gdata->contacts.pcontacts);
			gdata->contacts.pcontacts = NULL;
		}
		flikcr_free_contact_infors(gdata);
		gdata->contacts.counts = 0;
	}
	
}

/**
@brief get the contacts of the login account
@param[in] gdata : get from flickr_init_gdata(), need authentication
**/
EXPORT_SYMBOL
int flickr_get_contacts(flickr_gdata_t *gdata)
{
	int rtn = FLICKR_RTN_OK;
	flickcurl_contact **contacts = NULL;
	flickr_info("");
	if(gdata==NULL || gdata->fc==NULL){
		flickr_err("call flickr_init_gdata first!");
		rtn= FLICKR_GET_CONTACT_ERR;
		goto GET_CONTACTS_END;
	}
	contacts = flickcurl_contacts_getList(gdata->fc,(const char*)flickr_get_contact_fliter(FLICKR_CONTACT_ALL),0,0);
	flickr_info("");
	if(contacts) {
		int i;
		for(i = 0; contacts[i]; i++)
			flickr_print_contact(contacts[i], i);
		gdata->contacts.counts = i;
		gdata->contacts.pcontacts= contacts;
		if(i!=0){
			gdata->contacts.infors_array= (flickr_infors_t *)flickr_malloc(sizeof(flickr_infors_t)*i);
			if(gdata->contacts.infors_array==NULL){
				rtn = FLICKR_RTN_MALLOC_FAIL;
				flickr_err("Malloc Contacts infors Space Error!");
				flickr_free_contacts(gdata);
			}
			else
				memset(gdata->contacts.infors_array,0,sizeof(flickr_infors_t)*i);
		}
	}
	else
		rtn = FLICKR_GET_CONTACT_ERR;
GET_CONTACTS_END:
	return rtn;
}

/**
@brief get the information of the photos, photosets, user, contacts
@param[in] gdata : 
**/
EXPORT_SYMBOL
char * flickr_get_info(flickr_gdata_t *gdata,flickr_info_get_cmd_e cmd,flickr_info_query_t *infor_query)
{
	char * value = NULL;
	if(gdata==NULL){
		infor_query->err_status= -1;
		return NULL;
	}
	infor_query->err_status = 0;
	switch(cmd){
		case GET_ACCOUNT_TYPE:
			value = (char*)gdata->account_type;
			break;
		case GET_LOGIN_EMAIL:
			value = gdata->user_email.data;
			break;
		case GET_LOGIN_PWD:
			value = gdata->user_pwd.data;
			break;
		case GET_PERMS:
			value = gdata->perms.data;
			break;
		case GET_FROB:
			value = gdata->frob.data;
			break;
		case GET_USER_REALNAME:			///< user's realname
		case GET_USER_USERNAME:			///< user's username
			value = flickr_getinfo_userinfo(gdata,cmd,infor_query);
			break;
		case GET_CONTACT_COUNT:			///< how many contacts does the login account has, should change to int by the caller
			//flickr_info("Contact count==%d",gdata->contacts.counts);
			value = (char*)(gdata->contacts.counts);
			break;
		case GET_CONTACT_REALNAME:			///< contact's realname
		case GET_CONTACT_USERNAME:		///< contact's username
			value = flickr_getinfo_getcontactinfo(gdata,cmd,infor_query);
			break;
		case GET_PHOTOSET_COUNT:					///< how many photosets in the account
		case GET_PHOTOSET_ID:					///< photoset id
		case GET_PHOTOSET_PHOTOS_COUNT: 		///< photos' num in a photoset
		case GET_PHOTOSET_TITLE:				///< title of the photoset	
		case GET_PHOTOSET_DESCRIPTION:			///< description of the photoset
			value = flickr_getinfo_getphotosetinfo(gdata,cmd,infor_query);
			break;
		case GET_PHOTOS_COUNT:///< photos num in the account,  should change to int by the caller
			{
				flickcurl_photos_list* photos_list=NULL;
				photos_list = flickr_get_photos_list_addr(gdata,infor_query->which_contact,-1);
				if(photos_list)
					value = (char*)photos_list->total_count; ///< should change to int by the caller
			}
			break;
		case GET_PHOTO_DATESPOSTED:				///< photo dates posted
		case GET_PHOTO_DATESTAKEN:				///< photo dates taken
		case GET_PHOTO_DATESLASTUPDATE:			///< photo dates last update
		case GET_PHOTO_TITLE:					///< photo title
		case GET_PHOTO_DESCRIPTION:			///< photo description
		case GET_PHOTO_ID:					///< photo id
			value = flickr_getinfo_photoinfo(gdata,cmd,infor_query);
			break;
		case GET_URL_COVER_SMALL:				///< get photoset cover url, small picture
		case GET_URL_COVER_HUGE:				///< get photoset cover url, huge picture
		case GET_URL_PHOTO_SMALL:				///< get photo url, small picture
		case GET_URL_PHOTO_HUGE:				///< get photo url, huge picture
			value = flickr_getinfo_geturl(gdata,cmd,infor_query);
			break;
		default:
			flickr_err("Sorry CMD ERROR!");
	}
	return value;
}


/**
@brief init the download information for the flickr_download_photo function
@param[in] iscache: whether the photo be cached,1 cached, 0 uncached
@param[in] cache_dir : where the photo be stored
@param[in] gdata: get from flickr_init_gdata(), need authentication
@param[in] which_contact: -1, will get the calling account itself
		others:  the friends add by the calling account, should less than gdata->contacts.counts
@param[in] which_photoset: if which_photoset>=0 specified which photoset will be get
	range from [0~ photosets.counts), if which_photoset=-1, it will get the photo in the account
@param[in] which_photo : which photo to be gotten,if range from [0,photos_num), photos_num is the value returned from
	flickr_get_info when cmd is GET_PHOTOS_COUNT, if the which_photo ==-1, download the cover
@param[in] is_thumbnail: whether the photo be download is thumbnail
**/
EXPORT_SYMBOL
flickr_download_info_t *flickr_init_download_info(int iscache,char* cache_dir,flickr_gdata_t * gdata,int which_contact,int which_photoset,int which_photo,int isthumbnail)
{
	flickr_download_info_t * down_info=NULL;
	char tmp_buf[FLICKR_TMP_BUF_LEN]="";
	flickcurl_photoset*photoset=NULL;
	flickcurl_photo * photo=NULL;
	int err_status=FLICKR_RTN_OK;
	void *fhandle=NULL;
	char *photo_url=NULL;
	down_info = (flickr_download_info_t *)flickr_malloc(sizeof(flickr_download_info_t));
	memset(tmp_buf,0,FLICKR_TMP_BUF_LEN);
	if(down_info){
		memset(down_info,0,sizeof(flickr_download_info_t));
		down_info->isthumbnail = !!isthumbnail;
		down_info->needcache = !!iscache;
		if(which_photoset>=0){///< get the photoset
			photoset = flickr_get_photoset_addr(gdata,which_contact,which_photoset);
			if(photoset){
				if(which_photo>=0){///get the photo
					if(isthumbnail)
						sprintf(tmp_buf,"%s/%s",photoset->id,"thumbnails");  ///< store under /cache_dir/photosetid/thumbnails/
					else
						sprintf(tmp_buf,"%s",photoset->id);	///< store under /cache_dir/photosetid/
				}
				else{///< get the cover
					if(isthumbnail) /// store under the /cache_dir/thumbnails/
						sprintf(tmp_buf,"thumbnails");
				}	
				flickr_fill_atomic(tmp_buf,&down_info->img_path.album_id);
			}
			else{
				err_status = FLICKR_RTN_DOWNLOAD_GETPHOTOSET_ERR;
			}
		}
		else{///< get the photos
			if(isthumbnail)	///< store under /cache_dir/my_photos/thumbnails/
				sprintf(tmp_buf,"%s/%s",account_photo_store_folder,"thumbnails");
			else
				sprintf(tmp_buf,"%s",account_photo_store_folder);	///< store under /cache_dir/my_photos/
			flickr_fill_atomic(tmp_buf,&down_info->img_path.album_id);///< fill the album_id addr
		}
			
		flickr_fill_atomic(cache_dir,&down_info->img_path.cache_dir);

		if(which_photo>=0){///< download photo
			photo = flickr_get_photo_addr(gdata,which_contact,which_photoset,which_photo);
			if(photo){
				memset(tmp_buf,0,FLICKR_TMP_BUF_LEN);
				sprintf(tmp_buf,"%s.jpg",photo->id);
				flickr_fill_atomic(tmp_buf,&down_info->img_path.photo_name);
				if(isthumbnail)
					photo_url = flickr_get_photo_url(photo,FLICKR_PHOTOSIZE_T);
				else
					photo_url = flickr_get_photo_url(photo,FLICKR_PHOTOSIZE_B);
				if(photo_url)
					flickr_fill_atomic(photo_url, &down_info->photo_url);
			}
			else{
				err_status = FLICKR_RTN_DOWNLOAD_GETPHOTO_ERR;
				goto INIT_DOWNLOAD_INFO_END;
			}
		}
		else{/// down load cover
			if(photoset){
				memset(tmp_buf,0,FLICKR_TMP_BUF_LEN);
				sprintf(tmp_buf,"%s.jpg",photoset->id);
				flickr_fill_atomic(tmp_buf,&down_info->img_path.photo_name);
				if(isthumbnail)
					photo_url = flickr_get_photoset_cover_url(photoset,FLICKR_PHOTOSIZE_T);
				else
					photo_url = flickr_get_photoset_cover_url(photoset,FLICKR_PHOTOSIZE_B);
				if(photo_url)
					flickr_fill_atomic(photo_url, &down_info->photo_url);
			}
		}

		err_status = flickr_check_path(&down_info->img_path,isthumbnail); 
		if(err_status){
			goto INIT_DOWNLOAD_INFO_END;
		}
			
		if(iscache){
			fhandle = flickr_open_file(&down_info->img_path,&err_status);
			if(fhandle==NULL){
				goto INIT_DOWNLOAD_INFO_END;
			}
			flickr_data_write_init(&down_info->photo_data,flickcurl_data_write_file,fhandle);
		}
		else{
			flickr_data_write_init(&down_info->photo_data,flickcurl_data_write_buffer,NULL);
		}
		down_info->gdata = gdata;
	}

INIT_DOWNLOAD_INFO_END:
	if(err_status){
		flickr_free(down_info);
		down_info = NULL;
	}
	gdata->err_status = flickr_get_status(err_status,0,0);
	flickr_info("ErrStatus=%d",err_status);
	return down_info;
}

EXPORT_SYMBOL
int flickr_free_download_info(flickr_download_info_t* down_info)
{
	void * fhandle = NULL;
	char tmpbuf[FLICKR_TMP_BUF_LEN]="";
	if(down_info){
		fhandle = down_info->photo_data.file_handle;
		if(fhandle){
			flickr_fflush(fhandle);
			flickr_fclose(fhandle);
			if(down_info->status!=FLICKR_HttpStatus_OK){///down load failed, need remove the file which had been created
				if(flickr_get_file_path(&down_info->img_path,&tmpbuf)!=0){
					flickr_fremove(tmpbuf);
					
				}
			}
		}
		flickr_free_atomic(&down_info->photo_url);
		flickr_data_write_free(&down_info->photo_data);
		flickr_free_atomic(&down_info->img_path.cache_dir);
		flickr_free_atomic(&down_info->img_path.album_id);
		flickr_free_atomic(&down_info->img_path.photo_name);	
		flickr_free((char*)down_info);
		down_info = NULL;
	}
	return 0;
}

/**
@brief download photo
@param[in] gdata: get from flickr_init_gdata(), need authentication
@parma[in] down_info	: choose the method to be download, cache or uncache ,see __picasa_init_download_info()
@return 
	the status of the processing, see macro get_status();
	- HttpStatus_OK	: success
**/
EXPORT_SYMBOL
int flickr_download_photo(flickr_gdata_t *gdata,flickr_download_info_t *down_info)
{
	int status = 0;
	long m_http_code=0;
	int rtn=FLICKR_RTN_OK;
	CURL *tmp_curl=NULL;
	CURLcode curlcode=0;

	tmp_curl = curl_easy_init();
	if(down_info==NULL || tmp_curl==NULL){
		flickr_err("DOWN info ==NULL\n");
		return 0;
	}
	
	flickr_data_write_t *photo_data = &down_info->photo_data;
	
	if(down_info->photo_url.data==NULL){
		flickr_err("Photo Url Error!\n");
		rtn  = FLICKR_RTN_ERR_DOWNLOADPHOTO;
		goto DOWNLOAD_PHOTO_END;
	}
		
	flickr_info("DOWN LOAD Photo URL=%s\n",down_info->photo_url.data);

	curl_easy_setopt(tmp_curl,CURLOPT_SSL_VERIFYPEER,0);
	//curl_easy_setopt(tmp_curl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(tmp_curl,CURLOPT_HTTPGET,1L);
	curl_easy_setopt(tmp_curl,CURLOPT_URL,down_info->photo_url.data);

	curl_easy_setopt(tmp_curl,CURLOPT_WRITEDATA,photo_data);
	curl_easy_setopt(tmp_curl,CURLOPT_WRITEFUNCTION,flickr_func_write_data);

	curl_easy_setopt(tmp_curl,CURLOPT_NOPROGRESS,0L);
	curl_easy_setopt(tmp_curl,CURLOPT_PROGRESSFUNCTION,flickr_func_prog);
	curl_easy_setopt(tmp_curl,CURLOPT_PROGRESSDATA,down_info);

	curl_easy_setopt(tmp_curl,CURLOPT_TIMEOUT,FLICKR_CURL_TIMEOUT);	

	curlcode = curl_easy_perform(tmp_curl);
	curl_easy_getinfo(tmp_curl, CURLINFO_RESPONSE_CODE, &m_http_code);
	down_info->workdone = 1;
	flickr_info("curlcode=%d,http_code=%d\n",(int)curlcode,(int)m_http_code);

	if(curlcode==CURLE_OK && m_http_code==FLICKR_HttpStatus_OK){
		
	}
	else{
		flickr_info("%s,%d:Down Load Photo Error\n",__FILE__,__LINE__);
		rtn  = FLICKR_RTN_ERR_DOWNLOADPHOTO;
	}
DOWNLOAD_PHOTO_END:
	
	status = flickr_get_status(rtn,curlcode,m_http_code);	
	down_info->status = status;

	if(tmp_curl)
		curl_easy_cleanup(tmp_curl);
	return status;
}

/**
@brief get the path where the photos be stored
@param[in] gdata : get from flickr_init_gdata(), need authentication
@param[in] which_contact : if -1, get the login account itself, others get the specified contact
@param[in] which_photoset : if -1, get the photos of all, others get the specified photoset
@param[in] which_photo : specified which photo
@param[in] isthumbnail : 1: thumbnail, 0: large picture
@param[in] pathbuf : the buffer where the path to be stored
@param[in] buf_len : the length of the pathbuf
**/
EXPORT_SYMBOL
int flickr_get_cache_path(flickr_gdata_t * gdata,int which_contact,int which_photoset,int which_photo,int isthumbnail,char *pathbuf,int buf_len)
{
	flickr_path_t img_path;
	int buf_len_need = 0;
	int err_status = FLICKR_RTN_OK;
	flickcurl_photoset* photoset=NULL;
	flickcurl_photo * photo=NULL;
	char tmp_buf[FLICKR_TMP_BUF_LEN]="";
	memset(&img_path,0,sizeof(flickr_path_t));
	if(which_photoset>=0){///< get the photoset
		photoset = flickr_get_photoset_addr(gdata,which_contact,which_photoset);
		if(photoset){
			if(which_photo>=0){///get the photo
				if(isthumbnail)
					sprintf(tmp_buf,"%s/%s",photoset->id,"thumbnails");  ///< store under /cache_dir/photosetid/thumbnails/
				else
					sprintf(tmp_buf,"%s",photoset->id);	///< store under /cache_dir/photosetid/
			}
			else{///< get the cover
				if(isthumbnail) /// store under the /cache_dir/thumbnails/
					sprintf(tmp_buf,"thumbnails");
			}	
			flickr_fill_atomic(tmp_buf,&img_path.album_id);
		}
		else{
			err_status = FLICKR_RTN_DOWNLOAD_GETPHOTOSET_ERR;
		}
	}
	else{///< get the photos
		if(isthumbnail)	///< store under /cache_dir/my_photos/thumbnails/
			sprintf(tmp_buf,"%s/%s",account_photo_store_folder,"thumbnails");
		else
			sprintf(tmp_buf,"%s",account_photo_store_folder);	///< store under /cache_dir/my_photos/
		flickr_fill_atomic(tmp_buf,&img_path.album_id);///< fill the album_id addr
	}
			
	if(which_photo>=0){///< download photo
		photo = flickr_get_photo_addr(gdata,which_contact,which_photoset,which_photo);
		if(photo){
			memset(tmp_buf,0,FLICKR_TMP_BUF_LEN);
			sprintf(tmp_buf,"%s.jpg",photo->id);
			flickr_fill_atomic(tmp_buf,&img_path.photo_name);
		}
		else{
			err_status = FLICKR_RTN_DOWNLOAD_GETPHOTO_ERR;
			goto GET_CACHE_PATH_END;
		}
	}
	else{/// down load cover
		if(photoset){
			memset(tmp_buf,0,FLICKR_TMP_BUF_LEN);
			sprintf(tmp_buf,"%s.jpg",photoset->id);
			flickr_fill_atomic(tmp_buf,&img_path.photo_name);
		}
	}

	buf_len_need = img_path.album_id.data_len+img_path.photo_name.data_len+2;
	if(buf_len_need>buf_len){
		flickr_err("The Buf len is short, need %d bytes!",buf_len_need);
		err_status = FLICKR_RTN_CACHE_BUF_ERR;
	}
	else{
		if(img_path.album_id.data)
			sprintf(pathbuf,"/%s/%s",img_path.album_id.data,img_path.photo_name.data);
		else
			sprintf(pathbuf,"/%s",img_path.photo_name.data);
		
	}
GET_CACHE_PATH_END:
	flickr_free_cache_path(&img_path);
	return err_status;
}


/**
@brief send the message to the thread
@param[in] gdata	: initialize by calling the flickr_init_gdata()
@param[in] cmd 	: see flickr_ioctl_cmd_e
@param[in] para	: 
			if cmd ==FLICKR_CMD_DOWNLOADPHOTO, OR FLICKR_CMD_DOWNLOADPHOTO
				it is the pointer to the flickr_download_info_t, call the flickr_init_download_info() to get it
			if cmd ==FLICKR_CMD_AUTH or FLICKR_CMD_STOPAUTH
				it is the pointer to the flickr_gdata_t, call flickr_init_gdata() to get it
@return 
	 - -1	: failed
	 - 0 		: success
**/
EXPORT_SYMBOL
int flickr_send_msg(flickr_gdata_t *gdata,flickr_ioctl_cmd_e cmd,void * para)
{
	int rtn = 0;
	int req_idx=0;
	flickr_ioctrl_t req;
	req.iocmd = cmd;
	req.para = para;
	switch(cmd){
		case FLICKR_CMD_STOPDOWNLOAD:
			{
				flickr_download_info_t * down_info = (flickr_download_info_t *)req.para;
				if(down_info)
					down_info->photo_data.cancel_write = 1;
				req.para_size = sizeof(flickr_download_info_t);
				break;
			}
		case FLICKR_CMD_DOWNLOADPHOTO:
			req.para_size = sizeof(flickr_download_info_t);
			break;
		case FLICKR_CMD_AUTH:
			req.para_size = sizeof(flickr_gdata_t);
			break;
		case FLICKR_CMD_STOPAUTH:
			{
				flickr_gdata_t * gdata = (flickr_gdata_t *)req.para;
				if(gdata)
					gdata->is_auth_cancel= 1;
				req.para_size = sizeof(flickr_gdata_t);
				break;
			}
			break;
		default:
			flickr_err("CMD Not Support yet cmd=%d",cmd);
	}
	req.timestamp = __flickr_get_timestamp();
	req_idx = flickr_req_enqueue(gdata->task_info,&req);
	flickr_info("Send Msg reqidx=%d\n",req_idx);
	if(req_idx==-1)
		rtn = -1;
	else{
		if(gdata->task_info){
			flickr_info("Post Semi Start~~~~~~~~~~~~~~~~~~~~~~\n");
			flickr_sem_post(&gdata->task_info->syn_lock.semi_start);
		}
		else
			rtn = -1;
	}
	return rtn;
}


/**
@brief query the download staus, call this function after sending msg to the thread by calling flickr_send_msg()
@param[in] gdata	: initialize by calling the flickr_init_gdata()
@param[in] down_info	: it is the pointer to the flickr_download_info_t, call the flickr_init_download_info() to get it
@param[in] query_cmd: see flickr_query_cmd
@return 
	if query_cmd == FLICKR_QUERY_CMD_RESULT
	- -2		: the param is error
	- -1		: had not down yet
	- others	: had been done
	if query_cmd == FLICKR_QUREY_CMD_PROGRESS
	the progress of downloading 
**/
EXPORT_SYMBOL
int flickr_query_download_status(flickr_gdata_t *gdata,flickr_download_info_t * down_info,flickr_query_cmd query_cmd)
{
	int rtn = -1;
	if(query_cmd ==FLICKR_QUREY_CMD_PROGRESS && down_info!=NULL)
		return down_info->prog_down;
	
	rtn = flickr_req_query(gdata->task_info,FLICKR_CMD_DOWNLOADPHOTO,down_info);
	if(rtn>=0){
		if(down_info){
			if(query_cmd==FLICKR_QUERY_CMD_RESULT)
				return down_info->status;
			else{
				flickr_err("Query Download Cmd Error cmd =%d",query_cmd);
				return 0;
			}
		}
		else 
			return 0;
	}
	else{
		return rtn;
	}
}

/**
@brief query the authstaus, call this function after sending msg to the thread by calling flickr_send_msg()
@param[in] gdata	: initialize by calling the flickr_init_gdata()
@param[in] query_cmd: see flickr_query_cmd, set the 
@return 
	if query_cmd == FLICKR_QUERY_CMD_RESULT
	- -2		: the param is error
	- -1		: had not down yet
	- others	: had been done, but it is unguaranteed that it is auth successfully
**/
EXPORT_SYMBOL
int flickr_query_auth_status(flickr_gdata_t *gdata,flickr_query_cmd query_cmd)
{
	int rtn = -1;
	rtn = flickr_req_query(gdata->task_info,FLICKR_CMD_DOWNLOADPHOTO,gdata);
	if(query_cmd == FLICKR_QUERY_CMD_RESULT){
		if(rtn>=0)
			return gdata->err_status;
		else
			return rtn;
	}
	else{
		flickr_err("Query CMD is error querycmd=%d!",query_cmd);
		return FLICKR_QUERY_STATUS_PARAERR;
	}
}


#endif

#if 0
int main()
{	
	int rtn=FLICKR_RTN_OK;
	flickr_gdata_t * gdata=NULL;
	gdata = flickr_init_gdata();

	//flickr_set_login_info(gdata,flickr_google_name,flickr_google_pwd,ACCOUNT_TYPE_GOOGLE);
	//flickr_set_login_info(gdata,flickr_fbook_name,flickr_fbook_pwd,ACCOUNT_TYPE_FACEBOOK);
	flickr_set_login_info(gdata,flickr_yahoo_name,flickr_yahoo_pwd,ACCOUNT_TYPE_YAHOO);
	flickr_auth(gdata);
	
	flickr_get_contacts(gdata);
	flickr_get_user_info(gdata,-1);
	
	flickr_get_photos_info(gdata,1,FLICKR_PRI_NONE);
	{
		flickcurl_photo *  photo=flickr_get_photo_addr(gdata,-1,-1,0);
		if(photo)
			flickr_get_photo_url(photo,FLICKR_PHOTOSIZE_M);
	}
	
	rtn = flickr_get_photosets_info(gdata,-1);
	if(!rtn){
		flickr_get_photosets_photos_info(gdata,-1,0,FLICKR_PRI_NONE);
	}
	
	{	
		char * value=NULL;
		flickr_info_query_t infor_query;
		infor_query.which_contact = -1;
		infor_query.which_photoset = 0;
		infor_query.which_photo = 0;
		value = flickr_get_info(gdata,GET_URL_PHOTO_SMALL,&infor_query);
		if(value)
			flickr_info("Value===<%s>",value);
	}
	{
		flickr_download_info_t * down_info=NULL;
		down_info = flickr_init_download_info(1,"/work/pictures/",gdata,-1,0,0,1);
		if(down_info)
			flickr_download_photo(gdata,down_info);
		flickr_free_download_info(down_info);
	}
	flickr_free_gdata(gdata);
	return 1;
}
#endif
