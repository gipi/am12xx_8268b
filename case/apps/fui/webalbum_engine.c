#ifdef MODULE_CONFIG_WEBALBUM
#include <webalbum_api.h>
#include <flickr_main.h>
#include <webalbum_engine.h>
#include <swf_ext.h>

#ifdef PICASA_WEBALBUM_DEBUG
#define web_info(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define web_err(fmt,arg...)  printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

#else

#define web_info(fmt,arg...) do{}while(0);
#define web_err(fmt,arg...) do{}while(0);
#endif

static Webalbum_type login_type=0;
facebook_data facebook_data_s;

#define TMPSTRBUF_LEN 256
picasaweb_gdata_t web_gdata;
int download_info_err_status=0;
char  web_tmpstrbuf[TMPSTRBUF_LEN];

inline int rtt_isleap(unsigned short year)
{
        return (((year%4==0)&&(year%100!=0))||(year%400==0));
}

#if 1
#define FLICKR_LOGIN_DATA_LEN 64
typedef struct flicrk_user_info_s
{
	char email[FLICKR_LOGIN_DATA_LEN];
	char passwd[FLICKR_LOGIN_DATA_LEN];
}flickr_user_info_t;
flickr_user_info_t flickr_user;

//@brief use this function to cache the account information of flickr
static int flickr_save_tmp_user_info(char *email, char* passwd)   
{
	memset(&flickr_user,0,sizeof(flickr_user_info_t));
	memcpy(flickr_user.email,email,FLICKR_LOGIN_DATA_LEN-1);
	memcpy(flickr_user.passwd,passwd,FLICKR_LOGIN_DATA_LEN-1);
	return 0;
}

#ifdef MODULE_CONFIG_FLICKR
static void __webalbum_flickr_free_album_info(eg_flickr_feed_info_t *albums_info,int which_photoset);
static void flickr_fill_gdata_user_info(flickr_gdata_t *gdata)
{
	flickr_set_login_info(gdata,flickr_user.email,flickr_user.passwd,ACCOUNT_TYPE_YAHOO);
}

static eg_flickr_feed_info_t * __webalbum_flickr_get_albums_info(flickr_gdata_t * gdata,int which_contact)
{
	int rtn = FLICKR_RTN_OK;
	int photosets_num=0;
	eg_flickr_feed_info_t * albums_info=NULL;
	int feed_info_size =sizeof(eg_flickr_feed_info_t);
	rtn = flickr_get_photosets_info(gdata,which_contact);
	if(rtn==FLICKR_RTN_OK){
		albums_info =(eg_flickr_feed_info_t*)SWF_Malloc(feed_info_size);
		if(albums_info){
			flickr_info_query_t info_query;	
			memset(&info_query,0,sizeof(flickr_info_query_t));
			memset(albums_info,0,feed_info_size);
			info_query.which_contact = which_contact;
			photosets_num = (int)flickr_get_info(gdata,GET_PHOTOSET_COUNT,&info_query);
			if(photosets_num){
				albums_info->child_feed_array = (eg_flickr_feed_info_t**)SWF_Malloc(sizeof(eg_flickr_feed_info_t*)*photosets_num);
				if(albums_info->child_feed_array){
					memset(albums_info->child_feed_array,0,feed_info_size*photosets_num);
					albums_info->entry_num = photosets_num;
				}
				else{
					flickr_free_photosets_info(gdata,which_contact);
					SWF_Free(albums_info);
					albums_info =NULL;
					web_err("Malloc Album_info Array Error!");
					goto GET_ALBUMS_INFO_END;
				}
			}
			
			albums_info->feed_type = FLICKR_FEED_ALBUMS;
			albums_info->gdata = gdata;
			albums_info->which_contact = which_contact;
		}
		else{
			flickr_free_photosets_info(gdata,which_contact);
		}	
	}
GET_ALBUMS_INFO_END:
	return albums_info;
}

static void __webalbum_flickr_free_albums_info(eg_flickr_feed_info_t *albums_info)
{
	int i=0;
	if(albums_info){
		if(albums_info->gdata && albums_info->feed_type==FLICKR_FEED_ALBUMS)
			flickr_free_photosets_info(albums_info->gdata,albums_info->which_contact);
		else
			web_err("Carzy Something must be wrong!");
		albums_info->gdata = NULL;
		if(albums_info->child_feed_array){
			for(i=0;i<albums_info->entry_num;i++){
				__webalbum_flickr_free_album_info(albums_info,i);
			}
			SWF_Free(albums_info->child_feed_array);
			albums_info->entry_num = 0;
		}
		SWF_Free(albums_info);
	}
}


static eg_flickr_feed_info_t * __webalbum_flickr_get_album_info(eg_flickr_feed_info_t *albums_info,int which_photoset)
{
	int rtn = FLICKR_RTN_OK;
	eg_flickr_feed_info_t * album_info=NULL;
	if(albums_info==NULL || albums_info->gdata==NULL)
		return albums_info;
	web_info("Flcikr_get Album_info!");
	rtn = flickr_get_photosets_photos_info(albums_info->gdata,albums_info->which_contact,which_photoset,FLICKR_PRI_NONE);
	if(rtn==FLICKR_RTN_OK){
		album_info =(eg_flickr_feed_info_t*)SWF_Malloc(sizeof(eg_flickr_feed_info_t));
		if(album_info){
			memset(album_info,0,sizeof(eg_flickr_feed_info_t));
			album_info->feed_type = FLICKR_FEED_ALBUM;
			album_info->gdata = albums_info->gdata;
			album_info->which_contact = albums_info->which_contact;
			album_info->which_photoset = which_photoset;
			if(which_photoset<albums_info->entry_num){
				albums_info->child_feed_array[which_photoset]= album_info;
			}
			else{
				web_err("Crazy: Get album Info Error!your index=%d,max index=%d",which_photoset,albums_info->entry_num);
				flickr_free_photosets_photos_info(albums_info->gdata,albums_info->which_contact,which_photoset);
				SWF_Free(album_info);
				album_info=NULL;
			}
		}
		else{
			flickr_free_photosets_photos_info(albums_info->gdata,albums_info->which_contact,which_photoset);
		}	
	}
	return album_info;
}

static void __webalbum_flickr_free_album_info(eg_flickr_feed_info_t *albums_info,int which_photoset)
{
	eg_flickr_feed_info_t * album_info=NULL;
	if(albums_info){
		if(albums_info->gdata && albums_info->feed_type==FLICKR_FEED_ALBUMS){
			if(which_photoset<albums_info->entry_num){
				album_info = albums_info->child_feed_array[which_photoset];
				if(album_info && album_info->feed_type==FLICKR_FEED_ALBUM){
					flickr_free_photosets_photos_info(album_info->gdata,album_info->which_contact,album_info->which_photoset);
				}
			}
			else{
				web_err("PhotosetError Num Error max=%d!",albums_info->entry_num);
			}
				
		}
		else
			web_err("Carzy Something must be wrong!");
		album_info->gdata = NULL;
		SWF_Free(album_info);
	}
}

static flickr_download_info_t * __webalbum_flickr_init_download_info(int iscache,char* cache_dir,eg_flickr_feed_info_t *album_info,int which_photo,int isthumbnail)
{
	flickr_download_info_t * flickr_download_info=NULL;
	if(album_info && album_info->gdata!=NULL ){
		if(album_info->feed_type==FLICKR_FEED_ALBUM)	///< download the photo in the specified photoset
			flickr_download_info = flickr_init_download_info(iscache,cache_dir,album_info->gdata,album_info->which_contact,album_info->which_photoset,which_photo,isthumbnail);
		else if(album_info->feed_type== FLICKR_FEED_ALBUMS) ///< down load cover of the photoset
			flickr_download_info = flickr_init_download_info(iscache,cache_dir,album_info->gdata,album_info->which_contact,album_info->which_photoset,-1,isthumbnail);
	}
	return flickr_download_info;
}

static void __webalbum_flickr_free_download_info(flickr_download_info_t *download_info)
{
	if(download_info){
		flickr_free_download_info(download_info);
	}
}

static void  __webalbum_flickr_get_info(void*para,int para_type,int info_type,int other_info,char* info_store_buff)
{
	flickr_gdata_t * flickr_gdata=NULL;
	flickr_info_query_t  info_query;
	int num_real=0;
	char *value_real=NULL;
	memset(&info_query,0,sizeof(flickr_info_query_t));
	if(para_type==PARA_TYPE_FEED){
		flickr_gdata = (flickr_gdata_t*)para;
		switch(info_type){
			case FLICKR_GET_ACCOUNT_TYPE:
			case FLICKR_GET_LOGIN_EMAIL:
			case FLICKR_GET_LOGIN_PWD:
			case FLICKR_GET_PERMS:
			case FLICKR_GET_FROB:
			case FLICKR_GET_CONTACT_COUNT:	
				value_real = flickr_get_info(flickr_gdata,info_type,&info_query);
				if(info_type==FLICKR_GET_ACCOUNT_TYPE || info_type==FLICKR_GET_CONTACT_COUNT){
					num_real =(int)value_real;
					sprintf(info_store_buff,"%d",num_real);
				}
				else
					sprintf(info_store_buff,"%s",value_real);
				break;
			default:
				web_err("The Info type is not match to the para_type");
				break;
		}
	}
	else if(para_type==PARA_TYPE_CONTACT){
		flickr_gdata = (flickr_gdata_t*)para;
		info_query.which_contact = other_info;
		switch(info_type){
			case FLICKR_GET_USER_REALNAME:
			case FLICKR_GET_USER_USERNAME:
			case FLICKR_GET_CONTACT_REALNAME:
			case FLICKR_GET_CONTACT_USERNAME:
			case FLICKR_GET_PHOTOSET_COUNT:
			case FLICKR_GET_PHOTOS_COUNT:
				value_real = flickr_get_info(flickr_gdata,info_type,&info_query);
				web_info("Value Real==0x%x",value_real);
				if(info_type==FLICKR_GET_PHOTOS_COUNT || info_type==FLICKR_GET_PHOTOSET_COUNT){
					num_real =(int)value_real;
					sprintf(info_store_buff,"%d",num_real);
				}
				else
					sprintf(info_store_buff,"%s",value_real);
				break;
			default:
				web_err("The Info type is not match to the para_type");
				break;
		}
	}
	else if(para_type==PARA_TYPE_PHOTOALBUMS){
		eg_flickr_feed_info_t *albums_info = (eg_flickr_feed_info_t*)para;
		if(albums_info && albums_info->feed_type==FLICKR_FEED_ALBUMS){
			flickr_gdata = albums_info->gdata;
			info_query.which_contact = albums_info->which_contact;
			info_query.which_photoset =  other_info;
			switch(info_type){
				case FLICKR_GET_PHOTOSET_ID:
				case FLICKR_GET_PHOTOSET_PHOTOS_COUNT:
				case FLICKR_GET_PHOTOSET_TITLE:
				case FLICKR_GET_PHOTOSET_DESCRIPTION:
				case FLICKR_GET_URL_COVER_SMALL:
				case FLICKR_GET_URL_COVER_HUGE:
					value_real = flickr_get_info(flickr_gdata,info_type,&info_query);
					if(info_type==FLICKR_GET_PHOTOSET_PHOTOS_COUNT){
						num_real =(int)value_real;
						sprintf(info_store_buff,"%d",num_real);
					}
					else
						sprintf(info_store_buff,"%s",value_real);
					break;
				default:
					web_err("The Info type is not match to the para_type");
					break;
			}
		}
	}
	else if(para_type==PARA_TYPE_SINGLEALBUM){
		eg_flickr_feed_info_t * album_info=(eg_flickr_feed_info_t*)para;
		if(album_info && album_info->feed_type==FLICKR_FEED_ALBUM){
			flickr_gdata= album_info->gdata;
			info_query.which_contact = album_info->which_contact;
			info_query.which_photoset = album_info->which_photoset;
			info_query.which_photo = other_info;
			switch(info_type){
				case FLICKR_GET_PHOTO_DATESPOSTED:
				case FLICKR_GET_PHOTO_DATESTAKEN:
				case FLICKR_GET_PHOTO_DATESLASTUPDATE:
				case FLICKR_GET_PHOTO_TITLE:
				case FLICKR_GET_PHOTO_DESCRIPTION:
				case FLICKR_GET_PHOTO_ID:
				case FLICKR_GET_URL_PHOTO_SMALL:
				case FLICKR_GET_URL_PHOTO_HUGE:
					value_real = flickr_get_info(flickr_gdata,info_type,&info_query);
					sprintf(info_store_buff,"%s",value_real);
					break;
				default:
					web_err("The Info type is not match to the para_type");
					break;
			}
		}
	}
	
}

#endif

#endif
/**
@brief changing the seconds to the localtime from 1970,1,1
@param[in]
**/
void __webalbum_get_localtime(long time_sec, date_time_t *time,long timezone)
{
	const int monthLengths[2][13] = {
	{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
		{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366},
	};
	const int yearLengths[2] = { 365, 366 };
	int year;
	int month;
	int minMonth;
	int maxMonth;

	int days;
	int clock;
	int isLeap;

	time_sec += timezone*60*60;

	days = time_sec / 86400;
	clock = time_sec % 86400;
	if(clock < 0){
		clock += 86400;
		days -= 1;
	}

    /**/////////////////////////////////////////////////////////////////////////////
    // Calcaulate year, 11323=0x3A4FC880/86400; 13879=0x47798280/86400
    /**////////////////////////////////////////////////////////////////////////////   
	if(days >= 0){
		year = days/366;
		days -= year*365 + (year+1)/4 - (year+69)/100 + (year+369)/400;
		for(time->year = year + 1970; ; time->year++){
			isLeap = rtt_isleap(time->year);
			if(days < yearLengths[isLeap]){
			        break;
			}
			days -= yearLengths[isLeap];
		}
	}
    	else{
		year = days/366;
		days -= year*365 + (year-2)/4 - (year-30)/100 + (year-30)/400;

		for(time->year = year + 1970 - 1; ; time->year--){
		    isLeap = 0;//rtt_isleap(time->year);
		    days += yearLengths[isLeap];
		    if(days >= 0){
			break;
		    }
		}
	}    
    /**///////////////////////////////////////////////////////////////////////////
    // compute month and day, use the half search save time
    /**////////////////////////////////////////////////////////////////////////////
	minMonth = 0;
	maxMonth = 12;
	for(month = 5; month < 12 && month > 0; month = (minMonth + maxMonth) / 2){
	// days between monthLengths[month]<=days<monthLengths[month+1]
		if(days < monthLengths[isLeap][month]){    //too big
			maxMonth = month;
		}
		else if(days >= monthLengths[isLeap][month + 1]){    //too small
			minMonth = month;
		}
		else{    //so it is
	    		break;
		}
	}
	days -= monthLengths[isLeap][month];
	time->month = month + 1;
	time->day = days + 1;	 
	/**///////////////////////////////////////////////////////////////////////////
	// Calcaulate hour minute and second
	/**///////////////////////////////////////////////////////////////////////////
	time->hour = clock / 3600;        //3600s one hour
	clock = clock % 3600;
	time->minute = clock / 60;        //60s one minute
	time->second = clock % 60;        //ms
	web_info("time=%d-%d-%d-%d-%d-%d\n",time->year,time->month,time->day,time->hour,time->minute,time->second);
}

long str2long(char*str)
{
	long sum=0;
	char c=0;
	int i=0;
	while((c=*(str+i))!=0){
		if(c-'0'>10){
			web_info("Str2long err : str=%s",str);
			sum =0;
			break;
		}
		sum = (c-'0')*10;
		i++;
	}
	return sum;
}

int webalbum_timestamp2loacaltime(char* timestamp,char*buf)
{
	long sec=0;
	date_time_t time_local;
	sec = str2long(timestamp);
	web_info("Sec==%d",sec);
	memset(&time_local,0,sizeof(date_time_t));
	__webalbum_get_localtime(sec,&time_local,0);
	sprintf(buf,"%d-%02d-%02d-%02d-%02d-%02d",time_local.year,time_local.month,time_local.day,time_local.hour,\
		time_local.minute,time_local.second);
	return 0;
}

static picasaweb_gdata_t *__web_get_gdata()
{
	picasaweb_gdata_t * gdata=NULL;
	gdata = &web_gdata;
	return gdata;
}

/**
@brief call this function to set the login type(picasa/facebook/flickr)
@param[in] login_type : which type of the account, picasa or facebook or flickr
@return
	- 0 	: success
	- -1	: failed
**/
static int webalbum_set_logintype(void * handle)
{
	int rtn = 0;
	SWFEXT_FUNC_BEGIN(handle);
	login_type = Swfext_GetNumber();
	web_info("Login Type is %d",login_type);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

/**
@brief call this function to set the email addr and password
@param[in] email	: email addr
@param[in] pwd	: password
@return
	- 0 	: success
	- -1	: failed
**/
static int webalbum_set_login(void * handle)
{
	int rtn = 0;
	char *email=NULL;
	char *pwd = NULL;
	SWFEXT_FUNC_BEGIN(handle);
	email = Swfext_GetString();
	pwd = Swfext_GetString();
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA)
		rtn = picasa_set_longin(email,pwd);
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK)
		rtn = facebook_set_longin(email,pwd);
#endif
#ifdef MODULE_CONFIG_FLICKR
	if(login_type>=FLICKR_YAHOO)
		rtn = flickr_save_tmp_user_info(email,pwd);
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

/**
@brief initializing the structure of picasaweb_gdata_t ,call the webalbum_free_gdata() to free the space
@return 
	- 0 		: failed
	- others 	: pointer to the gdata
**/
static int webalbum_init_gdata(void * handle)
{
	int rtn = 0;
	void * webalbumdata=NULL;
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		picasa_init_resource();
		webalbumdata = __web_get_gdata();
		if(picasa_init_gdata(webalbumdata)==0)
			rtn = (int)webalbumdata;
		else
			rtn = 0;
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		webalbumdata = &facebook_data_s;
		if(facebook_init_fdata(webalbumdata)==0)
			rtn = (int)webalbumdata;
		else
			rtn = 0;
	}
#endif	

#ifdef MODULE_CONFIG_FLICKR
	if(login_type>=FLICKR_YAHOO){
		flickr_gdata_t *gdata=NULL;
		gdata =flickr_init_gdata();
		if(gdata)
			flickr_fill_gdata_user_info(gdata);  ///set login email and password
		rtn = (int)gdata;
		web_info("Flickr Gdata====0x%x",gdata);
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

/**
@brief free the space which had been malloc when the webalbum_init_gdata() is called
@param[in] gdata	: the pointer to the gdata which had been filled
@see webalbum_init_gdata()
**/
static int webalbum_free_gdata(void * handle)
{
	int rtn = 0;
	void * webalbumdata=NULL;
	SWFEXT_FUNC_BEGIN(handle);
	webalbumdata = (void*)Swfext_GetNumber();
	if(webalbumdata){
#ifdef MODULE_CONFIG_PICASA
		if(login_type==PICASA){
			picasa_free_gdata((picasaweb_gdata_t *)webalbumdata);
		}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
		if(login_type==FACEBOOK){
			facebook_free_fdata((facebook_data*)webalbumdata);
		}
#endif
#ifdef MODULE_CONFIG_FLICKR
		if(login_type>=FLICKR_YAHOO){
			flickr_free_gdata((flickr_gdata_t*)webalbumdata);
		}
#endif
	}
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

/**
@brief get the login user info
@param[in] gdata	: initialize by calling the webalbum_init_gdata()
@return :
	NULL	: get user info failed
	others	: the pointer to the facebook_user which include all information of the login user
**/
static int webalbum_get_userinfo(void * handle)
{
	int rtn = 0;
	void * webalbumdata=NULL;
	facebook_user * userinfo=NULL;
	SWFEXT_FUNC_BEGIN(handle);
	webalbumdata = (void*)Swfext_GetNumber();
	if(webalbumdata){
#ifdef MODULE_CONFIG_FACEBOOK
		if(login_type==FACEBOOK){
			userinfo = get_user_info((facebook_data*)webalbumdata);
			rtn = (int)userinfo;
		}
#endif
#ifdef MODULE_CONFIG_FLICKR
		if(login_type>=FLICKR_YAHOO){
			int flickr_rtn=FLICKR_RTN_OK;
			if(webalbumdata){
				flickr_rtn = flickr_get_user_info((flickr_gdata_t * )webalbumdata,-1);//get the login account's user info
				if(flickr_rtn==FLICKR_RTN_OK)
					rtn = (int)webalbumdata;
			}
		}
#endif
	}

	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

/**
@brief free the space which is occupied when the webalbum_get_contact() is called 
@param[in] feed_contact 	: the pointer to the picasaweb_feed_t which the information of the contacts are included when webalbum_get_contact() is called
@return always 0
@see webalbum_get_contact()
**/
static int webalbum_free_userinfo(void * handle)
{
	int rtn = 0;
	facebook_user * userinfo=NULL;
	flickr_gdata_t * flickr_gdata=NULL;
	SWFEXT_FUNC_BEGIN(handle);
	
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		userinfo = (facebook_user*)Swfext_GetNumber();
		if(userinfo)
			facebook_free_userinfo(userinfo);
	}
#endif
#ifdef MODULE_CONFIG_FLICKR
	if(login_type>=FLICKR_YAHOO){
		flickr_gdata = (flickr_gdata_t*)Swfext_GetNumber();
		if(flickr_gdata)
			flickr_free_user_info(flickr_gdata,-1);
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

/**
@brief call this function after the authentication is ok for getting the contact in the account
@param[in] gdata	: initialize by calling the webalbum_init_gdata()
@return 
	- NULL		: get the infomation of the contact failed
	- others	: the pointer to the picasaweb_feed_t which the information of the contact are inclued 
**/
static int webalbum_get_contact(void * handle)
{
	int rtn = 0;
	void * webalbumdata=NULL;
	picasaweb_gdata_t * gdata=NULL;
	picasaweb_feed_t * feed_contact=NULL;
	flickr_gdata_t *flickr_gdata=NULL;
	facebook_data * fdata=NULL;
	facebook_contact * contact=NULL;
	facebook_feed *f_feed = NULL;
	SWFEXT_FUNC_BEGIN(handle);
	
	webalbumdata = (void*)Swfext_GetNumber();
	if(webalbumdata){
#ifdef MODULE_CONFIG_PICASA
		if(login_type==PICASA){
			gdata = (picasaweb_gdata_t*)webalbumdata;
			feed_contact = picasa_get_contact(gdata);
			rtn = (int)feed_contact;
		}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
		if(login_type==FACEBOOK){
			fdata = (facebook_data*)webalbumdata;
			contact = get_contact(fdata);
			web_info("Get Contact info contact=0x%x",contact);
			f_feed = facebook_feed_type(CONTACT_FEED,(void *)contact);
			rtn = (int)f_feed;
		}
#endif

#ifdef MODULE_CONFIG_FLICKR
		if(login_type>=FLICKR_YAHOO){
			int flickr_rtn=-1;
			flickr_gdata = (flickr_gdata_t*)webalbumdata;
			if(flickr_gdata){
				flickr_rtn = flickr_get_contacts(flickr_gdata);
			}
			if(flickr_rtn!=FLICKR_RTN_OK)
				rtn = 0;
			else
				rtn = (int)flickr_gdata;
		}
#endif
	}
	web_info("Get Contact info feed_contact=0x%x",rtn);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}


/**
@brief free the space which is occupied when the webalbum_get_contact() is called 
@param[in] feed_contact 	: the pointer to the picasaweb_feed_t which the information of the contacts are included when webalbum_get_contact() is called
@return always 0
@see webalbum_get_contact()
**/
static int webalbum_free_contact(void * handle)
{
	int rtn = 0;
	picasaweb_feed_t * feed_contact=NULL;
	facebook_contact * contact=NULL;
	facebook_feed * f_feed=NULL;
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		feed_contact = (picasaweb_feed_t*)Swfext_GetNumber();
		if(feed_contact){
			picasa_free_contact(feed_contact);
		}
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
		if(login_type==FACEBOOK){
			f_feed = (facebook_feed *)Swfext_GetNumber();
			if(f_feed){
				facebook_free_contact((facebook_contact *)f_feed->feed_p);
				f_feed->feed_p = NULL;
				facebook_free_type(f_feed);
				f_feed = NULL;
			}
		}
#endif
#ifdef MODULE_CONFIG_FLICKR
	if(login_type>=FLICKR_YAHOO){
		flickr_gdata_t * gdata=(flickr_gdata_t*)Swfext_GetNumber();
		if(gdata)
			flickr_free_contacts(gdata);
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int webalbum_get_friendlists(void * handle)
{
	int rtn = 0;
	
	facebook_data * fdata=NULL;
	facebook_friendlists * friendlists=NULL;
	SWFEXT_FUNC_BEGIN(handle);
	
	fdata = (facebook_data*)Swfext_GetNumber();
	if(fdata){
#ifdef MODULE_CONFIG_FACEBOOK
		if(login_type==FACEBOOK){
			friendlists = get_friends_list(fdata);
			rtn = (int)friendlists;
		}
#endif
	}
	web_info("Get friendlist info friendlist=0x%x",rtn);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int webalbum_free_friendlists(void * handle)
{
	int rtn = 0;
	facebook_friendlists * friendlists=NULL;
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_FACEBOOK
		if(login_type==FACEBOOK){
			friendlists = (facebook_friendlists *)Swfext_GetNumber();
			if(friendlists){
				facebook_free_friendlists(friendlists);
			}
		}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int webalbum_get_member(void * handle)
{
	int rtn = 0;
	facebook_data * fdata=NULL;
	facebook_friendlists * friendlists=NULL;
	int friendlist_index = 0;
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_FACEBOOK
		if(login_type==FACEBOOK){
			fdata = (facebook_data*)Swfext_GetNumber();
			friendlists = (facebook_friendlists *)Swfext_GetNumber();
			friendlist_index = Swfext_GetNumber();
			if(fdata && friendlists){
				get_listmembers_info(fdata, friendlists->friendlist_entry+friendlist_index);
				rtn = (int)(friendlists->friendlist_entry+friendlist_index);
			}
		}
#endif
	web_info("Get Contact Member info feed_contact=0x%x",rtn);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int webalbum_free_member(void * handle)
{
	int rtn = 0;
	facebook_friendlist * friendlist=NULL;
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_FACEBOOK
		if(login_type==FACEBOOK){
			friendlist = (facebook_friendlist *)Swfext_GetNumber();
			if(friendlist){
				facebook_free_friendlist(friendlist);
			}
		}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

/**
@brief call this function after the authentication is ok for getting the albums in the account
@param[in] gdata	: initialize by calling the webalbum_init_gdata()
@param[in] feed_contact 	: get from webalbum_get_cotact(), if this value is null, get albums info of the login account
@param[in] which_friend	:  if the feed_contact is NULL, this value is useless,if the feed_contact is not NULL, specify which friend will be get, the range of this value is 0~ (feed_contact->entry_num-1)
@return 
	- NULL		: get the infomation of the albums failed
	- others	: the pointer to the picasaweb_feed_t which the information of the albums is inclued 
**/
static int webalbum_get_albums_info(void * handle)
{
	int rtn = 0;
	void * webalbumdata=NULL;
	picasaweb_gdata_t * gdata=NULL;
	picasaweb_feed_t * feed_contact=NULL;
	int which_friend = 0;
	picasaweb_feed_t * feed_albums=NULL;

	facebook_data * fdata=NULL;
	facebook_contact * contact=NULL;
	facebook_photoalbums * photoalbums = NULL;
	facebook_feed *f_feed = NULL;

	flickr_gdata_t * flickr_gdata=NULL;
	int flickr_contact=0;
	eg_flickr_feed_info_t *flickr_albums_feed=NULL;

	SWFEXT_FUNC_BEGIN(handle);
	webalbumdata = (void*)Swfext_GetNumber();
	if(webalbumdata){
#ifdef MODULE_CONFIG_PICASA
		if(login_type==PICASA){
			gdata = (picasaweb_gdata_t*)webalbumdata;
			feed_contact = (picasaweb_feed_t*)Swfext_GetNumber();
			which_friend = Swfext_GetNumber();
			
			if(gdata){
				feed_albums = picasa_get_albums_info(gdata,feed_contact,which_friend);
			}
			
			rtn = (int)feed_albums;
		}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
		if(login_type==FACEBOOK){
			fdata = (facebook_data*)webalbumdata;
			facebook_feed *f_feed = (facebook_feed *)Swfext_GetNumber();
			if(f_feed)
				contact = (facebook_contact*)f_feed->feed_p;
			else
				contact = NULL;
			which_friend = Swfext_GetNumber();
			photoalbums = facebook_get_albums_info(fdata, contact, which_friend);
			web_info("Get Albums info feed_albums=0x%x",photoalbums);
			f_feed = facebook_feed_type(ALBUM_FEED,(void *)photoalbums);
			rtn = (int)f_feed;
		}
#endif
#ifdef MODULE_CONFIG_FLICKR
	if(login_type>=FLICKR_YAHOO){
		flickr_gdata = (flickr_gdata_t*)webalbumdata;
		flickr_contact = Swfext_GetNumber(); 
		which_friend = Swfext_GetNumber();
		if(flickr_contact==0) ///< get the login account' info
			which_friend = -1;
		flickr_albums_feed = __webalbum_flickr_get_albums_info(flickr_gdata,which_friend);
		rtn = (int)flickr_albums_feed;
	}
#endif
	}
	web_info("Get Albums info feed_albums=0x%x",rtn);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}


/**
@brief free the space which is occupied when the webalbum_get_albums_info() is called 
@param[in] feed_albums 	: the pointer to the picasaweb_feed_t which the information of the album is included when webalbum_get_albums_info() is called
@return always 0
@see webalbum_get_albums_info()
**/
static int webalbum_free_albums_info(void * handle)
{
	int rtn = 0;
	void * feed_albums=NULL;
	facebook_feed *f_feed = NULL;
	SWFEXT_FUNC_BEGIN(handle);
	
	feed_albums = (void*)Swfext_GetNumber();
	if(feed_albums){
#ifdef MODULE_CONFIG_PICASA
		if(login_type==PICASA){
			picasa_free_albums_info((picasaweb_feed_t*)feed_albums);
		}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
		if(login_type==FACEBOOK){
			f_feed = (facebook_feed *)feed_albums;
			if(f_feed){
				facebook_free_photoalbums((facebook_photoalbums *)f_feed->feed_p);
				f_feed->feed_p = NULL;
				facebook_free_type(f_feed);
				f_feed = NULL;
			}
		}
#endif
#ifdef MODULE_CONFIG_FLICKR
		if(login_type>=FLICKR_YAHOO){
			__webalbum_flickr_free_albums_info((eg_flickr_feed_info_t *)feed_albums);
		}
#endif
	}

	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

/**
@brief getting the information of an album which is specified by the para used
@param[in] gdata	: initialize by calling the webalbum_init_gdata();
@param[in] feed_albums	: the pointer to the picasaweb_feed_t which include all information of the albums in the account
@param[in] which_album	: which album will specified, the range of this value is 0~(feed_albums->entry_num-1)
@return
	NULL	: get the infomation of the album failed
	others	: the pointer to the picasaweb_feed_t which the information of the album 
@see call webalbum_free_album_info() to free the space which is malloced
**/
static int webalbum_get_album_info(void * handle)
{
	int rtn = 0;
	void * webalbumdata=NULL;
	picasaweb_gdata_t * gdata=NULL;
	picasaweb_feed_t * feed_albums=NULL;
	int which_album=0;
	picasaweb_feed_t * feed_album=NULL;

	facebook_data * fdata=NULL;
	facebook_photoalbums * photoalbums = NULL;
	facebook_feed *f_feed = NULL;
	facebook_feed *photo_feed = NULL;
	
	SWFEXT_FUNC_BEGIN(handle);
	webalbumdata = (void*)Swfext_GetNumber();

	if(webalbumdata){
#ifdef MODULE_CONFIG_PICASA
		if(login_type==PICASA){
			gdata = (picasaweb_gdata_t*)webalbumdata;
			feed_albums = (picasaweb_feed_t*)Swfext_GetNumber();
			which_album = Swfext_GetNumber();
			web_info("Call Get Album info gdata=0x%x,albums=0x%x,which_album=0x%x");
			if(gdata && feed_albums)
				feed_album = picasa_get_album_info(gdata,feed_albums,which_album);
			
			rtn = (int)feed_album;
		}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
		if(login_type==FACEBOOK){
			fdata = (facebook_data*)webalbumdata;
			f_feed = (facebook_feed *)Swfext_GetNumber();
			which_album = Swfext_GetNumber();
			photoalbums = (facebook_photoalbums*)f_feed->feed_p;
			if(get_albumphoto_info(fdata, which_album, photoalbums) == 0){
				photo_feed = facebook_feed_type(PHOTO_FEED,(void *)(photoalbums->album_entry+which_album));
				rtn = (int)photo_feed;
			}
		}
#endif
#ifdef MODULE_CONFIG_FLICKR
		if(login_type>=FLICKR_YAHOO){
			eg_flickr_feed_info_t *album_info=NULL;
			flickr_gdata_t * flickr_gdata = (flickr_gdata_t*)webalbumdata;
			eg_flickr_feed_info_t *albums_info=(eg_flickr_feed_info_t*)Swfext_GetNumber();
			which_album = Swfext_GetNumber();
			web_info("albums_info=0x%x,which_album=%d",albums_info,which_album);
			if(albums_info && albums_info->feed_type==FLICKR_FEED_ALBUMS)
				album_info = __webalbum_flickr_get_album_info(albums_info,which_album);
			rtn = (int)album_info;
		}
#endif
	}
	web_info("Get Album info album_info==0x%x",rtn);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

/**
@brief free the space which is occupied when the webalbum_get_album_info() is called 
@param[in] feed_album : the pointer to the picasaweb_feed_t which the information of the photos are included when the webalbum_get_album_info() is called
@return always 0
@see webalbum_get_album_info()
**/
static int webalbum_free_album_info(void * handle)
{
	int rtn = 0;
	picasaweb_feed_t * feed_album=NULL;
	picasaweb_feed_t * feed_albums=NULL;
	int which_album =0;

	facebook_feed *photo_feed = NULL;
	SWFEXT_FUNC_BEGIN(handle);
	
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		feed_albums = (picasaweb_feed_t*)Swfext_GetNumber();
		which_album = Swfext_GetNumber();
		
		if(feed_albums){
			picasa_free_album_info(feed_albums,which_album);
		}
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		photo_feed = (facebook_feed*)Swfext_GetNumber();
		which_album = Swfext_GetNumber();
		if(photo_feed){
			facebook_free_album((facebook_album *)(photo_feed->feed_p));
			photo_feed->feed_p = NULL;
			facebook_free_type(photo_feed);
			photo_feed = NULL;
		}
	}
#endif
#ifdef MODULE_CONFIG_FLICKR
	if(login_type>=FLICKR_YAHOO){
		eg_flickr_feed_info_t *albums_info=(eg_flickr_feed_info_t*)Swfext_GetNumber();
		which_album = Swfext_GetNumber();
		if(albums_info && albums_info->feed_type == FLICKR_FEED_ALBUMS)
			__webalbum_flickr_free_album_info(albums_info,which_album);
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

/**
@brief call this function for passing the authentication
@param[in] gdata	: call webalbum_init_gdata() to initializing the gdata
@return
	the status of the processing, see macro get_status();
	- HttpStatus_OK : success
**/
static int webalbum_authentication(void * handle)
{
	int rtn = 0;
	void * webalbumdata=NULL;
	SWFEXT_FUNC_BEGIN(handle);
	webalbumdata = (void*)Swfext_GetNumber();
	if(webalbumdata){
#ifdef MODULE_CONFIG_PICASA
		if(login_type==PICASA){
			rtn = picasa_authentication((picasaweb_gdata_t*)webalbumdata);
		}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
		if(login_type==FACEBOOK){
			rtn = facebook_authentication((facebook_data*)webalbumdata);
		}
#endif
#ifdef MODULE_CONFIG_FLICKR
		if(login_type>=FLICKR_YAHOO){
			rtn = flickr_auth((flickr_gdata_t*)webalbumdata);
		}
#endif
	}
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

/**
@brief query the auth staus, call this function after sending msg to the thread by calling picasa_send_msg()
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] query_cmd: see PicasawebQueryCmd
@return 
	if query_cmd == FACEBOOK_QUERY_CMD_RESULT
	- -2		: the param is error
	- -1		: had not down yet
	- HttpStatus_FOUND	: had been done
	if query_cmd == others
	- -2		: the param is error
**/
static int webalbum_query_auth_status(void * handle)
{
	int rtn = -2;
	int query_cmd=0;

	facebook_data * f_data = NULL;
	flickr_gdata_t *flickr_gdata=NULL;
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		f_data = (facebook_data * )Swfext_GetNumber();
		query_cmd = Swfext_GetNumber();
		
		if(f_data && query_cmd==FACEBOOK_QUERY_CMD_RESULT){
			rtn = facebook_query_auth_status(f_data,query_cmd);
		}
	}
#endif
#ifdef MODULE_CONFIG_FLICKR
	if(login_type>=FLICKR_YAHOO){
		flickr_gdata = (flickr_gdata_t * )Swfext_GetNumber();
		query_cmd = Swfext_GetNumber();
		if(f_data && query_cmd==FACEBOOK_QUERY_CMD_RESULT)
			rtn = flickr_query_auth_status(flickr_gdata,query_cmd);
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

/**
@brief call this function for creating new albums
@param[in] album_info	: the initial info of the album which will be created
@param[in] gdata	: call webalbum_init_gdata() to initializing the gdata
@return
	the status of the processing, see macro get_status();
	- HttpStatus_CREATED: success
**/
static int webalbum_create_new_album(void * handle)
{
	int rtn = 0;
	picasaweb_gdata_t * gdata=NULL;
	album_info_t album_info;
	char *title=NULL,*summary=NULL,*location=NULL,*access=NULL,*timestamp=NULL;
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_PICASA	
	if(login_type==PICASA){
		gdata = (picasaweb_gdata_t*)Swfext_GetNumber();
		title = Swfext_GetString();
		summary = Swfext_GetString();	
		location = Swfext_GetString();
		access = Swfext_GetString();
		timestamp = Swfext_GetString();
		
		memset(&album_info,0,sizeof(album_info_t));
		picasa_fill_atomic((xmlChar*)title,&(album_info.title));
		picasa_fill_atomic((xmlChar*)summary,&(album_info.summary));
		picasa_fill_atomic((xmlChar*)location,&(album_info.location));
		if(access)
			picasa_fill_atomic((xmlChar*)access,&(album_info.access));
		else
			picasa_fill_atomic((xmlChar*)"public",&(album_info.access));
		if(timestamp)
			picasa_fill_atomic((xmlChar*)timestamp,&(album_info.timestamp));
		else
			picasa_fill_atomic(NULL,&(album_info.timestamp));
		
		rtn = picasa_create_new_album(&album_info,gdata);
		
		picasa_free_atomic(&album_info.title);
		picasa_free_atomic(&album_info.summary);
		picasa_free_atomic(&album_info.location);
		picasa_free_atomic(&album_info.access);
		picasa_free_atomic(&album_info.timestamp);
	}
#endif
#ifdef MODULE_CONFIG_FLICKR
	if(login_type>=FLICKR_YAHOO){
		web_info("Sorry Flickr haven't Supportted Creating New Album Yet");
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}


/**
@brief call this function for deleting albums
@param[in] gdata	: call webalbum_init_gdata() to initializing the gdata
@param[in] feed_albums	: the album information of the albums, see webalbum_get_albums_info()
@param[in] which_album	: which album to be choosen
@return
	the status of the processing, see macro get_status();
	- HttpStatus_OK : success
**/
static int webalbum_delete_album(void * handle)
{
	int rtn = 0;
	picasaweb_gdata_t * gdata=NULL;
	picasaweb_feed_t * feed_albums=NULL;
	int which_album=0;
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		gdata = (picasaweb_gdata_t*)Swfext_GetNumber();
		feed_albums = (picasaweb_feed_t*)Swfext_GetNumber();
		which_album = Swfext_GetNumber();
		
		rtn = picasa_delete_album(gdata,feed_albums,which_album);
	}
#endif
#ifdef MODULE_CONFIG_FLICKR
	if(login_type>=FLICKR_YAHOO){
		web_info("Sorry Flickr can't support delete album yet!");
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

/**
*@brief call this function to update some info of the album
*@param[in] album_info	: change the info of the album if the element of album_info is not NULL, the gphoto_id member should be the value that retrived from the server.
*@param[in] gdata	: initialize by calling the webalbum_init_gdata()
*@param[in] feed_albums	: the pointer to the picasaweb_feed_t where the info of the albums stored, see webalbum_get_albums_info()
*@param[in] which_album	: which album to be changed
@return
	the status of the processing, see macro get_status();
	- HttpStatus_OK : success
**/
static int webalbum_update_album_info(void * handle)
{
	int rtn = 0;
	picasaweb_gdata_t * gdata=NULL;
	picasaweb_feed_t * feed_albums=NULL;
	int which_album=0;
	album_info_t album_info;
	char *title=NULL,*summary=NULL,*location=NULL,*access=NULL,*timestamp=NULL;
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		gdata = (picasaweb_gdata_t*)Swfext_GetNumber();
		feed_albums = (picasaweb_feed_t*)Swfext_GetNumber();
		which_album = Swfext_GetNumber();
		
		title = Swfext_GetString();
		summary = Swfext_GetString();	
		location = Swfext_GetString();
		access = Swfext_GetString();
		timestamp = Swfext_GetString();
		
		memset(&album_info,0,sizeof(album_info_t));
		picasa_fill_atomic((xmlChar*)title,&(album_info.title));
		picasa_fill_atomic((xmlChar*)summary,&(album_info.summary));
		picasa_fill_atomic((xmlChar*)location,&(album_info.location));
		if(access)
			picasa_fill_atomic((xmlChar*)access,&(album_info.access));
		else
			picasa_fill_atomic((xmlChar*)"public",&(album_info.access));
		if(timestamp)
			picasa_fill_atomic((xmlChar*)timestamp,&(album_info.timestamp));
		else
			picasa_fill_atomic(NULL,&(album_info.timestamp));
		
		rtn = picasa_update_album_info(&album_info,gdata,feed_albums,which_album);
		
		picasa_free_atomic(&album_info.title);
		picasa_free_atomic(&album_info.summary);
		picasa_free_atomic(&album_info.location);
		picasa_free_atomic(&album_info.access);
		picasa_free_atomic(&album_info.timestamp);
	}
#endif
#ifdef MODULE_CONFIG_FLICKR
	if(login_type>=FLICKR_YAHOO){
		web_info("Sorry Flickr can't support update album info yet!");
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

/**
@brief call this function to initializing the photo_upload_info_t which will be used in picasa_upload_photo(), call webalbum_free_upload_info()
to release the resource
@param[in]	photo_fullpath	: the full path of the photo which will be uploaded
@param[in] feed		: it is the picasaweb_feed_t which get from webalbum_get_albums_info()
@param[in] which_entry 	: which album does the photo to be added
@return 
	- NULL	: failed
	- others	: the pointer to the photo_upload_info_t
@see webalbum_free_upload_info()
**/
static int webalbum_init_upload_info(void * handle)
{
	int rtn = 0;
	photo_upload_info_t * upload_info=NULL;
	picasaweb_feed_t * feed_albums=NULL;
	int which_album=0;
	char * photo_fullpath=NULL;
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		feed_albums = (picasaweb_feed_t*)Swfext_GetNumber();
		which_album = Swfext_GetNumber();
		photo_fullpath = Swfext_GetString();
		
		upload_info = picasa_init_upload_info(photo_fullpath,feed_albums,which_album);
		
		rtn = (int)upload_info;
	}
#endif
#ifdef MODULE_CONFIG_FLICKR
	if(login_type>=FLICKR_YAHOO){
		web_info("Sorry Flickr can't support upload photo yet!");
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

/**
@brief call this function to to release the resource which had been occupied by calling webalbum_init_upload_info()
@param[in] upload_info	: the pointer to where the information for uploading stored, get it from calling webalbum_init_upload_info()
@return 
	always 0
**/
static int webalbum_free_upload_info(void * handle)
{
	int rtn = 0;
	photo_upload_info_t * upload_info=NULL;
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		upload_info=(photo_upload_info_t*)Swfext_GetNumber();

		if(upload_info)
			picasa_free_upload_info(upload_info);
	}
#endif
#ifdef MODULE_CONFIG_FLICKR
	if(login_type>=FLICKR_YAHOO){
		web_info("Sorry Flickr can't support upload photo yet!");
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

/**
@brief call this function to upload a photo
@param[in] gdata	: initialize by calling the webalbum_init_gdata()
@param[in] upload_info	: the pointer to where the information for uploading stored, get it from calling webalbum_init_upload_info()
@return 
	the status of the processing, see macro get_status();
	- HttpStatus_CREATED	: success
**/
static int webalbum_upload_photo(void * handle)
{
	int rtn = 0;
	picasaweb_gdata_t * gdata=NULL;
	photo_upload_info_t * upload_info=NULL;

	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		gdata = (picasaweb_gdata_t*)Swfext_GetNumber();
		upload_info=(photo_upload_info_t*)Swfext_GetNumber();

		if(gdata && upload_info)
			rtn = picasa_upload_photo(gdata,upload_info);
	}
#endif
#ifdef MODULE_CONFIG_FLICKR
	if(login_type>=FLICKR_YAHOO){
		web_info("Sorry Flickr can't support upload photo yet!");
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

/**
@brief query the upload staus,  call this function after sending msg to the thread by calling webalbum_send_msg()
@param[in] gdata	: initialize by calling the webalbum_init_gdata()
@param[in] para	: it is the pointer to the photo_down_info_t, call the webalbum_init_upload_info() to get it
@param[in] query_cmd: see PicasawebQueryCmd
@return 
	if query_cmd == QUERY_CMD_RESULT
	- -2		: the param is error
	- -1		: had not up yet
	- others	: had been done
	if query_cmd == QUREY_CMD_PROGRESS
	the progress of downloading 
**/
static int webalbum_query_upload_status(void * handle)
{
	int rtn = 0;
	picasaweb_gdata_t * gdata=NULL;
	photo_upload_info_t * upload_info=NULL;
	int upload_status_cmd=0;
	int query_cmd =0;
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		gdata = (picasaweb_gdata_t*)Swfext_GetNumber();
		upload_info=(photo_upload_info_t*)Swfext_GetNumber();
		query_cmd = Swfext_GetNumber();
		if(gdata && upload_info)
			rtn = picasa_query_upload_status(gdata,upload_info,query_cmd);
	}
#endif
#ifdef MODULE_CONFIG_FLICKR
	if(login_type>=FLICKR_YAHOO){
		web_info("Sorry Flickr can't support upload photo yet!");
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}


/**
@brief call this function to update the information of a photo
@param[in] photo_info 	: the info of the photo which will be replaced
@param[in] gdata	: initialize by calling the webalbum_init_gdata()
@param[in] feed_album	: the photo information in an album, see webalbum_get_album_info()
@param[in] which_photo	: which photo to be choosen
@return 
	the status of the processing, see macro get_status();
	- HttpStatus_OK	: success
**/
static int webalbum_update_photo_info(void * handle)
{
	int rtn = 0;
	photo_info_t photo_info;
	picasaweb_gdata_t * gdata=NULL;
	picasaweb_feed_t * feed_album=NULL;
	int which_photo=0;
	char * summary=NULL,*description=NULL;
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		gdata = (picasaweb_gdata_t*)Swfext_GetNumber();
		feed_album = (picasaweb_feed_t *)Swfext_GetNumber();
		which_photo = Swfext_GetNumber();
		summary = Swfext_GetString();
		description = Swfext_GetString();

		memset(&photo_info,0,sizeof(photo_info_t));
		picasa_fill_atomic((xmlChar*)summary,&(photo_info.summary));
		picasa_fill_atomic((xmlChar*)description,&(photo_info.description));
		
		rtn = picasa_update_photo_info(&photo_info,gdata,feed_album,which_photo);
		
		picasa_free_atomic(&(photo_info.summary));
		picasa_free_atomic(&(photo_info.description));
	}
#endif
#ifdef MODULE_CONFIG_FLICKR
	if(login_type>=FLICKR_YAHOO){
		web_info("Sorry Flickr can't support upload photo yet!");
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}


/**
@brief call this function to delete a photo
@param[in] gdata	: initialize by calling the webalbum_init_gdata()
@param[in] feed_album	: the photo information in an album, see webalbum_get_album_info()
@param[in] which_photo	: which photo to be choosen
@return 
	the status of the processing, see macro get_status();
	- HttpStatus_OK	: success
**/
static int webalbum_delete_photo(void * handle)
{
	int rtn = 0;
	picasaweb_gdata_t * gdata=NULL;
	picasaweb_feed_t * feed_album=NULL;
	int which_photo=0;
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		gdata = (picasaweb_gdata_t*)Swfext_GetNumber();
		feed_album = (picasaweb_feed_t *)Swfext_GetNumber();
		which_photo = Swfext_GetNumber();

		rtn = picasa_delete_photo(gdata,feed_album,which_photo);
	}
#endif
#ifdef MODULE_CONFIG_FLICKR
	if(login_type>=FLICKR_YAHOO){
		web_info("Sorry Flickr can't support delete photo yet!");
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}


/**
@brief call this function to initialize the download info ,call webalbum_free_download_info() to free the space 
@param[in] iscache	: whether it is cached
@param[in] cache_dir	: if iscache==1, it is the path of cache dir, such as /mnt/udisk/
@param[in] feed		: it is the picasaweb_feed_t which get from webalbum_get_albums_info() or webalbum_get_album_info()
@param[in] which_entry 	: which entry that the photo url in
@param[in] isthumbnail	: whether it is a thumbnail, if it is it will be cached in the thumbnail folder
@return 
	- NULL 	: failed
	- others : the pointer to the photo_down_info_t
@see webalbum_free_download_info()
**/
static int webalbum_init_download_info(void * handle)
{
	int rtn = 0;
	int iscache=0;
	char* cache_dir=NULL;
	void *feed_album = NULL;
	picasaweb_feed_t * feed=NULL;
	int which_entry = 0;
	int isthumbnail = 0;
	int err_status = 0;
	photo_down_info_t * down_info=NULL;

	facebook_photo_down_info *f_down_info = NULL;
	facebook_photoalbums * photoalbums = NULL;
	facebook_feed *f_feed = NULL;

	eg_flickr_feed_info_t * album_info=NULL;
	 flickr_download_info_t * flickr_download_info=NULL;
	
	SWFEXT_FUNC_BEGIN(handle);
	iscache = Swfext_GetNumber();
	cache_dir = Swfext_GetString();
	feed_album = (void*) Swfext_GetNumber();
	which_entry=Swfext_GetNumber();
	isthumbnail=Swfext_GetNumber();
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		feed = (picasaweb_feed_t *)feed_album;
		down_info = picasa_init_download_info(iscache,cache_dir,feed,which_entry,isthumbnail,&err_status);
		download_info_err_status = err_status;
		rtn = (int)down_info;
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		f_feed = (facebook_feed *)feed_album;
		f_down_info = facebook_init_download_info(iscache,cache_dir,f_feed,which_entry,isthumbnail);
		rtn = (int)f_down_info;
	}
#endif
#ifdef MODULE_CONFIG_FLICKR
	if(login_type>=FLICKR_YAHOO){
		album_info = (eg_flickr_feed_info_t*)feed_album;
		flickr_download_info = __webalbum_flickr_init_download_info(iscache,cache_dir,album_info,which_entry,isthumbnail);
		rtn = (int)flickr_download_info;
	}
#endif
	web_info("Init DownInfo down_info=0x%x",rtn);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}


/**
@brief release the space which is malloc when the webalbum_init_download_info() is called
@param[in] down_info 	: get from webalbum_init_download_info()
@return
	always return 0
**/
static int webalbum_free_download_info(void * handle)
{
	int rtn = 0;
	void *down = NULL;
	photo_down_info_t * down_info=NULL;
	facebook_photo_down_info *f_down_info = NULL;
	SWFEXT_FUNC_BEGIN(handle);
	
	down = (void*)Swfext_GetNumber();
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		down_info = (photo_down_info_t*)down;
		if(down_info)
			picasa_free_download_info(down_info);
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		f_down_info = (facebook_photo_down_info*)down;
		if(f_down_info)
			facebook_free_download_info(f_down_info);
	}
#endif
#ifdef MODULE_CONFIG_FLICKR
	if(login_type==FLICKR_YAHOO){
		__webalbum_flickr_free_download_info((flickr_download_info_t*)down);
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}


/**
@brief download photo
@param[in] gdata	: initialize by calling the webalbum_init_gdata()
@parma[in] down_info	: choose the method to be download, cache or uncache ,see webalbum_init_download_info()
@return 
	the status of the processing, see macro get_status();
	- HttpStatus_OK	: success
**/
static int webalbum_download_photo(void * handle)
{
	int rtn = 0;
	void * webalbumdata = NULL;
	void * feed = NULL;
	
	picasaweb_gdata_t * gdata=NULL;
	photo_down_info_t * down_info=NULL;

	facebook_data * f_data = NULL;
	facebook_photo_down_info * f_down_info = NULL;

	flickr_gdata_t * flickr_gdata=NULL;
	flickr_download_info_t * flickr_download_info=NULL;
	
	SWFEXT_FUNC_BEGIN(handle);

	webalbumdata = (void * )Swfext_GetNumber();
	feed = (void*)Swfext_GetNumber();
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		gdata = (picasaweb_gdata_t * )webalbumdata;
		down_info = (photo_down_info_t*)feed;

		if(gdata && down_info){
			rtn = picasa_download_photo(gdata,down_info);
		}
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		f_data = (facebook_data * )webalbumdata;
		f_down_info = (facebook_photo_down_info*)feed;
		if(f_data && f_down_info)
			rtn = facebook_download_photo(f_data, f_down_info);
	}
#endif
#ifdef MODULE_CONFIG_FLICKR
	if(login_type>=FLICKR_YAHOO){
		flickr_gdata = (flickr_gdata_t*)webalbumdata;
		flickr_download_info = (flickr_download_info_t*)feed;
		if(flickr_gdata && flickr_download_info)
			rtn = flickr_download_photo(flickr_gdata,flickr_download_info);
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}


/**
@brief query the download staus, call this function after sending msg to the thread by calling webalbum_send_msg()
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] down_info	: it is the pointer to the photo_down_info_t, call the webalbum_init_download_info() to get it
@param[in] query_cmd: see PicasawebQueryCmd
@return 
	if query_cmd == QUERY_CMD_RESULT
	- -2		: the param is error
	- -1		: had not down yet
	- others	: had been done
	if query_cmd == QUREY_CMD_PROGRESS
	the progress of downloading 
**/
static int webalbum_query_download_status(void * handle)
{
	int rtn = -2;
	picasaweb_gdata_t * gdata=NULL;
	photo_down_info_t * down_info=NULL;
	int query_cmd=0;

	facebook_data * f_data = NULL;
	facebook_photo_down_info * f_down_info = NULL;
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		gdata = (picasaweb_gdata_t * )Swfext_GetNumber();
		down_info = (photo_down_info_t*)Swfext_GetNumber();
		query_cmd = Swfext_GetNumber();
		
		if(gdata && down_info){
			rtn = picasa_query_download_status(gdata,down_info,query_cmd);
		}
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		f_data = (facebook_data * )Swfext_GetNumber();
		f_down_info = (facebook_photo_down_info*)Swfext_GetNumber();
		query_cmd = Swfext_GetNumber();
		
		if(f_data && f_down_info){
			rtn = facebook_query_download_status(f_data,f_down_info,query_cmd);
		}
	}
#endif
#ifdef MODULE_CONFIG_FLICKR
	if(login_type>=FLICKR_YAHOO){
		flickr_gdata_t * flickr_gdata= (flickr_gdata_t*)Swfext_GetNumber();
		flickr_download_info_t *flickr_down_info = (flickr_download_info_t*)Swfext_GetNumber();
		query_cmd = Swfext_GetNumber();
		web_info("flikr_gdata=0x%x,down_info=0x%x,query_cmd=%d",flickr_gdata,flickr_down_info,query_cmd);
		if(flickr_gdata && flickr_down_info)
			rtn = flickr_query_download_status(flickr_gdata, flickr_down_info,query_cmd);
	}
#endif
	web_info("Query Rtn=%d",rtn);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}


/**
@brief send the message to the thread
@param[in] gdata	: initialize by calling the webalbum_init_gdata()
@param[in] cmd 	: see picasa_ioctl_cmd_e
@param[in] para	: 
			if cmd ==PICASA_CMD_DOWNLOADPHOTO
				it is the pointer to the photo_down_info_t, call the webalbum_init_download_info() to get it
			if cmd==PICASA_CMD_UPLOADPHOTO
				it is the pointer to the photo_upload_info_t, call the webalbum_init_upload_info() to get it
@return 
	 - -1	: failed
	 - 0 		: success
**/
static int webalbum_send_msg(void * handle)
{
	int rtn = -1;
	picasaweb_gdata_t * gdata=NULL;
	int cmd=0;
	void * para=NULL;

	facebook_data * fdata=NULL;
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		gdata = (picasaweb_gdata_t * )Swfext_GetNumber();
		cmd = Swfext_GetNumber();
		para = (void*)Swfext_GetNumber();

		if(gdata && para)
			rtn = picasa_send_msg(gdata,cmd,para);
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		fdata = (facebook_data * )Swfext_GetNumber();
		cmd = Swfext_GetNumber();
		para = (void*)Swfext_GetNumber();

		if(fdata && para)
			rtn = facebook_send_msg(fdata,cmd,para);
	}
#endif
#ifdef MODULE_CONFIG_FLICKR
	if(login_type>=FLICKR_YAHOO){
		flickr_gdata_t *flickr_gdata = (flickr_gdata_t * )Swfext_GetNumber();
		cmd = Swfext_GetNumber();
		para = (void*)Swfext_GetNumber();
		if(flickr_gdata && para)
			rtn = flickr_send_msg(flickr_gdata,cmd,para);
		web_info("Send Msg gdata=0x%x,cmd=%d,para=0x%x,rtn=%d",flickr_gdata,cmd,para,rtn);
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}


static int webalbum_get_entry(void*handle)
{
	int rtn = 0;
	void * feed_album = NULL;
	picasaweb_feed_t* feed = NULL; 
	int which_entry=0;
	picasaweb_entry_t * entry=NULL;

	facebook_feed *f_feed = NULL;
	facebook_photo * f_entry = NULL;
	SWFEXT_FUNC_BEGIN(handle);
	feed_album = (void *)Swfext_GetNumber();
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		feed = (picasaweb_feed_t*)feed_album;
		which_entry = Swfext_GetNumber();
		if(feed){
			if(which_entry<0 || which_entry >= feed->entry_num){
				web_err("Entry Num is out of range MAX=%d,Your Value=%d",feed->entry_num-1,which_entry);
				goto GET_ENTRY_END;
			}
			entry = feed->entry+which_entry;
			rtn = (int)entry;
		}
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		f_feed = (facebook_feed *)feed_album;
		which_entry = Swfext_GetNumber();
		if(f_feed){
			if(f_feed->type==PHOTO_FEED){
				facebook_album * album = (facebook_album *)f_feed->feed_p;
				if(which_entry<0 || which_entry >= album->count){
					web_err("Entry Num is out of range MAX=%d,Your Value=%d",album->count-1,which_entry);
					goto GET_ENTRY_END;
				}
				f_entry = album->photo_entry+which_entry;
				rtn = (int)f_entry;
			}
			else if(f_feed->type==CONTACT_FEED){
				facebook_contact * contact = (facebook_contact *)f_feed->feed_p;
				if(which_entry<0 || which_entry >= contact->contact_num){
					web_err("Entry Num is out of range MAX=%d,Your Value=%d",contact->contact_num-1,which_entry);
					goto GET_ENTRY_END;
				}
				facebook_member * c_entry = contact->member_entry+which_entry;
				rtn = (int)c_entry;
			}
		}
	}
#endif
GET_ENTRY_END:
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}


int __webalbum_get_feed_element(int cmd,picasaweb_feed_t * feed_albums,char* buffer)
{
	switch(cmd){
		case FEED_ENTRY_NUM:
			sprintf(buffer,"%d",feed_albums->entry_num);
			break;
		case FEED_ALBUMID:
			sprintf(buffer,"%s",feed_albums->albumid.data);
			break;
		case FEED_ID:
			sprintf(buffer,"%s",feed_albums->id.data);
			break;
		case FEED_UPDATED:
			webalbum_timestamp2loacaltime(feed_albums->updated.data,buffer);
			break;
		case FEED_TITLE:
			sprintf(buffer,"%s",feed_albums->title.data);
			break;
		case FEED_SUBTITLE:
			sprintf(buffer,"%s",feed_albums->subtitle.data);
			break;
		case FEED_AUTHOR_NAME:
			sprintf(buffer,"%s",feed_albums->author.name.data);
			break;
		case FEED_USER:
			sprintf(buffer,"%s",feed_albums->user.data);
			break;
		case FEED_ACCESS:
			sprintf(buffer,"%s",feed_albums->access.data);
			break;
		case FEED_BYTESUSED:
			sprintf(buffer,"%s",feed_albums->bytesUsed.data);
			break;
		case FEED_LOCATION:
			sprintf(buffer,"%s",feed_albums->location.data);
			break;
		case FEED_NUMPHOTOS:
			sprintf(buffer,"%s",feed_albums->updated.data);
			break;
		case FEED_HEIGHT:
			sprintf(buffer,"%s",feed_albums->height.data);
			break;
		case FEED_WIDTH:
			sprintf(buffer,"%s",feed_albums->width.data);
			break;
		case FEED_TIMESTAMP:
			webalbum_timestamp2loacaltime(feed_albums->timestamp.data,buffer);
			break;
		default:
			web_err("Have not support yet info_type=%d\n",cmd);
	}
	return 0;
}

int __webalbum_get_entry_element(int cmd,picasaweb_entry_t * entry,char* buffer)
{
	switch(cmd){
		case ENTRY_EXIF_DISTANCE:
			sprintf(buffer,"%s",entry->tags.distance.data);
			break;
		case ENTRY_EXIF_EXPOSURE:
			sprintf(buffer,"%s",entry->tags.exposure.data);
			break;
		case ENTRY_EXIF_FLASH:
			sprintf(buffer,"%s",entry->tags.flash.data);
			break;
		case ENTRY_EXIF_FOCALLENGTH:
			sprintf(buffer,"%s",entry->tags.focallength.data);
			break;
		case ENTRY_EXIF_FSTOP:
			sprintf(buffer,"%s",entry->tags.fstop.data);
			break;
		case ENTRY_EXIF_IMG_UNIQUEID:
			sprintf(buffer,"%s",entry->tags.img_uniqueID.data);
			break;
		case ENTRY_EXIF_ISO:
			sprintf(buffer,"%s",entry->tags.iso.data);
			break;
		case ENTRY_EXIF_MAKE:
			sprintf(buffer,"%s",entry->tags.make.data);
			break;
		case ENTRY_EXIF_MODEL:
			sprintf(buffer,"%s",entry->tags.model.data);
			break;
		case ENTRY_EXIF_TIME:
			webalbum_timestamp2loacaltime(entry->tags.time.data,buffer);
			break;
		case ENTRY_PUBLISHED:
			webalbum_timestamp2loacaltime(entry->published.data,buffer);
			break;
		case ENTRY_UPDATED:
			webalbum_timestamp2loacaltime(entry->updated.data,buffer);
			break;
		case ENTRY_EDITED:
			webalbum_timestamp2loacaltime(entry->edited.data,buffer);
			break;
		case ENTRY_RIGHT:
			sprintf(buffer,"%s",entry->rights.data);
			break;
		case ENTRY_SUMMARY:
			sprintf(buffer,"%s",entry->summary.data);
			break;
		case ENTRY_TITLE:
			sprintf(buffer,"%s",entry->title.data);
			break;
		case ENTRY_AUTHOR_NAME:
			sprintf(buffer,"%s",entry->author.name.data);
			break;
		case ENTRY_WHERE_POS:
			sprintf(buffer,"%s",entry->where.point.pos.data);
			break;
		case ENTRY_ACCESS:
			sprintf(buffer,"%s",entry->access.data);
			break;
		case ENTRY_BYTESUSED:
			sprintf(buffer,"%s",entry->bytesUsed.data);
			break;
		case ENTRY_LOCATION:
			sprintf(buffer,"%s",entry->location.data);
			break;
		case ENTRY_NUMPHOTOS:
			sprintf(buffer,"%s",entry->numphotos.data);
			break;
		case ENTRY_NUMPHOTOREMAINING:
			sprintf(buffer,"%s",entry->numphotosremaining.data);
			break;
		case ENTRY_WIDHT:
			sprintf(buffer,"%s",entry->width.data);
			break;
		case ENTRY_HEIGHT:
			sprintf(buffer,"%s",entry->height.data);
			break;
		case ENTRY_TIMESTAMP:
			webalbum_timestamp2loacaltime(entry->timestamp.data,buffer);
			break;
		case ENTRY_ALBUMTITLE:
			sprintf(buffer,"%s",entry->albumtitle.data);
			break;
		case ENTRY_USERID:
			sprintf(buffer,"%s",entry->user.data);
			break;
		default:
			web_err("Entry CMD Err =%d",cmd);
	}
	return 0;
}

int __webalbum_get_photoalbums_element(int cmd,facebook_photoalbums * photoalbums,char* buffer)
{
	switch(cmd){
		case ALBUM_NUM:
			sprintf(buffer,"%d",photoalbums->album_num);
			break;
		default:
			web_err("Have not support yet info_type=%d\n",cmd);
	}
	return 0;
}

int __webalbum_get_singlealbum_element(int cmd,facebook_album * album,char* buffer)
{
	switch(cmd){
		case ALBUM_ID:
			sprintf(buffer,"%s",album->id.data);
			break;
		case ALBUM_FROM_ID:
			sprintf(buffer,"%s",(album->from.id).data);
			break;
		case ALBUM_FROM_NAME:
			sprintf(buffer,"%s",(album->from.name).data);
			break;
		case ALBUM_NAME:
			sprintf(buffer,"%s",album->name.data);
			break;
		case ALBUM_DESCRIPTION:
			sprintf(buffer,"%s",album->description.data);
			break;
		case ALBUM_LINK:
			sprintf(buffer,"%s",album->link.data);
			break;
		case ALBUM_COVER_PHOTO:
			sprintf(buffer,"%s",album->cover_photo.data);
			break;
		case ALBUM_PRIVACY:
			sprintf(buffer,"%s",album->privacy.data);
			break;
		case ALBUM_PHOTOCOUNT:
			sprintf(buffer,"%d",album->count);
			break;
		case ALBUM_TYPE:
			sprintf(buffer,"%s",album->type.data);
			break;
		case ALBUM_CREATED_TIME:
			sprintf(buffer,"%s",album->created_time.data);
			break;
		case ALBUM_UPDATED_TIME:
			sprintf(buffer,"%s",album->updated_time.data);
			break;
		case ALBUM_COMMENTS:
			break;
		default:
			web_err("Entry CMD Err =%d",cmd);
	}
	return 0;
}

int __webalbum_get_singlephoto_element(int cmd,facebook_photo * photo,char* buffer)
{
	switch(cmd){
		case PHOTO_ID:
			sprintf(buffer,"%s",photo->id.data);
			break;
		case PHOTO_FROM_ID:
			sprintf(buffer,"%s",photo->from.id.data);
			break;
		case PHOTO_FROM_NAME:
			sprintf(buffer,"%s",photo->from.name.data);
			break;
		case PHOTO_NAME:
			sprintf(buffer,"%s",photo->name.data);
			break;
		case PHOTO_PICTURE:
			sprintf(buffer,"%s",photo->picture.data);
			break;
		case PHOTO_SOURCE:
			sprintf(buffer,"%s",photo->source.data);
			break;
		case PHOTO_HEIGHT:
			sprintf(buffer,"%d",photo->height);
			break;
		case PHOTO_WIDTH:
			sprintf(buffer,"%d",photo->width);
			break;
		case PHOTO_LINK:
			sprintf(buffer,"%s",photo->link.data);
			break;
		case PHOTO_ICON:
			sprintf(buffer,"%s",photo->icon.data);
			break;
		case PHOTO_CREATED_TIME:
			sprintf(buffer,"%s",photo->created_time.data);
			break;
		case PHOTO_UPDATED_TIME:
			sprintf(buffer,"%s",photo->updated_time.data);
			break;
		case PHOTO_COMMENTS:
			break;
		default:
			web_err("Entry CMD Err =%d",cmd);
	}
	return 0;
}

int __webalbum_get_contact_element(int cmd,facebook_contact * contact,char* buffer)
{
	switch(cmd){
		case FRIENDS_NUM:
			sprintf(buffer,"%d",contact->contact_num);
			break;
		default:
			web_err("Entry CMD Err =%d",cmd);
	}
	return 0;
}

int __webalbum_get_singlemember_element(int cmd,facebook_member * member,char* buffer)
{
	switch(cmd){
		case MEMBER_ID:
			sprintf(buffer,"%s",member->id.data);
			break;
		case MEMBER_NAME:
			sprintf(buffer,"%s",member->name.data);
			break;
		default:
			web_err("Entry CMD Err =%d",cmd);
	}
	return 0;
}

/**
@brief get the information of the albums or photos
@param[in] para	: the pointer where the information stored
@param[in] para_type : which type of the pointer
@param[in] info_type: which information will be gotten
@param[in] other_info: the other information which will be needed when getting content
@return 
	the string of the content
**/
static int webalbum_get_info(void * handle)
{
	void *para=NULL;
	char * str_rtn=NULL;
	int info_type=0,para_type,other_info=0;
	SWFEXT_FUNC_BEGIN(handle);
	para = (void*)Swfext_GetNumber();
	para_type = Swfext_GetNumber();
	info_type = Swfext_GetNumber();
	other_info = Swfext_GetNumber();
	web_info("Get Info para=0x%x,para_type=%d,info_type=%d,other_info=%d",para,para_type,info_type,other_info);
	memset(web_tmpstrbuf,0,TMPSTRBUF_LEN);
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		switch(para_type){
			case PARA_TYPE_FEED:
			{
				picasaweb_feed_t * feed = (picasaweb_feed_t*)para;
				if(info_type==	FEED_PHOTO_ADDR){
					picasa_get_cache_path(feed,other_info,1,web_tmpstrbuf,TMPSTRBUF_LEN);
				}
				else if(info_type==FEED_PTOTO_THUMB_ADDR){
					picasa_get_cache_path(feed,other_info,0,web_tmpstrbuf,TMPSTRBUF_LEN);
				}
				else
					__webalbum_get_feed_element(info_type,feed,web_tmpstrbuf);
				break;
			}
			case PARA_TYPE_ENTRY:
			{
				picasaweb_entry_t * entry = (picasaweb_entry_t*)para;
				__webalbum_get_entry_element(info_type,entry,web_tmpstrbuf);
				break;
			}
			case PARA_TYPE_ERR_INIT_DOWNLOAD:
				sprintf(web_tmpstrbuf,"%d",download_info_err_status);
				break;		
		}
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		switch(para_type){
			case PARA_TYPE_PHOTOALBUMS:
			{
				facebook_feed *f_feed = (facebook_feed *)para;
				if(info_type==	PHOTO_PATH){
					facebook_get_cache_path(f_feed,other_info,0,web_tmpstrbuf,TMPSTRBUF_LEN);
				}
				else if(info_type==PHOTO_THUMB_PATH){
					facebook_get_cache_path(f_feed,other_info,1,web_tmpstrbuf,TMPSTRBUF_LEN);
				}
				else{
					facebook_photoalbums *photoalbums = (facebook_photoalbums *)f_feed->feed_p;
					__webalbum_get_photoalbums_element(info_type,photoalbums,web_tmpstrbuf);
				}
				break;
			}
			case PARA_TYPE_SINGLEALBUM:
			{
				facebook_feed *f_feed = (facebook_feed *)para;
				if(info_type==	PHOTO_PATH){
					facebook_get_cache_path(f_feed,other_info,0,web_tmpstrbuf,TMPSTRBUF_LEN);
				}
				else if(info_type==PHOTO_THUMB_PATH){
					facebook_get_cache_path(f_feed,other_info,1,web_tmpstrbuf,TMPSTRBUF_LEN);
				}
				else{
					facebook_album * album = (facebook_album *)f_feed->feed_p;
					__webalbum_get_singlealbum_element(info_type,album,web_tmpstrbuf);
				}
				break;
			}
			case PARA_TYPE_SINGLEPHOTO:
			{
				facebook_photo *photo = (facebook_photo *)para;
				__webalbum_get_singlephoto_element(info_type,photo,web_tmpstrbuf);
				break;
			}
			case PARA_TYPE_CONTACT:
			{
				facebook_feed *f_feed = (facebook_feed *)para;
				facebook_contact * contact = (facebook_contact *)f_feed->feed_p;
				__webalbum_get_contact_element(info_type,contact,web_tmpstrbuf);
				break;
			}
			case PARA_TYPE_SINGLEMEMBER:
			{
				facebook_member * member = (facebook_member *)para;
				__webalbum_get_singlemember_element(info_type,member,web_tmpstrbuf);
				break;
			}
		}
	}
#endif
#ifdef MODULE_CONFIG_FLICKR
	if(login_type>=FLICKR_YAHOO){
		__webalbum_flickr_get_info(para,para_type,info_type,other_info,web_tmpstrbuf);
	}
#endif
	web_info("info_type=%d,str=%s\n",info_type,web_tmpstrbuf);
	Swfext_PutString(web_tmpstrbuf);
	SWFEXT_FUNC_END();
}

///the following functions are for local operation

/**
@brief save the albums info into a file
@param[in] gdata	: initialize by calling the webalbum_init_gdata()
@param[in] feed_albums	: get from webalbum_get_albums_info()
@param[in] cache_dir	: under which folder will the file be created
@param[in] filename	: the name of the file to be created,if it is null, the default filename is the login name
@return :
	-1 	: failed
	0	: succeed
**/
static int webalbum_save_albums_info(void * handle)
{
	picasaweb_gdata_t *gdata = NULL;
	picasaweb_feed_t * feed_albums=NULL;
	char * cache_dir = NULL;
	char *filename=NULL;
	int rtn = 0;

	facebook_data * fdata=NULL;
	facebook_feed *f_feed = NULL;
	
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		gdata = (picasaweb_gdata_t*)Swfext_GetNumber();
		feed_albums = (picasaweb_feed_t*)Swfext_GetNumber();
		cache_dir = Swfext_GetString();
		filename = Swfext_GetString();
		
		if(strlen(filename)==0)
			filename = NULL;
		rtn = picasa_save_albums_info(gdata,feed_albums,cache_dir,filename);
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		fdata = (facebook_data*)Swfext_GetNumber();
		f_feed = (facebook_feed*)Swfext_GetNumber();
		cache_dir = Swfext_GetString();
		filename = Swfext_GetString();
		
		if(strlen(filename)==0)
			filename = NULL;
		if(f_feed && f_feed->type==ALBUM_FEED)
			rtn = facebook_save_albums_info(fdata,(facebook_photoalbums *)f_feed->feed_p,cache_dir,filename);
	}
#endif

	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}


/**
@brief load albums info from the file
@param[in] gdata	: initialize by calling the webalbum_init_gdata()
@param[in] cache_dir	: under which folder will the file be stored
@param[in] filename	: the name of the file to be read
@return :
	NULL	: load albums info error
	others	: the pointer to the picasaweb_feed_t which include all information of the albums in the account
**/
static int webalbum_load_albums_info(void * handle)
{
	int rtn = 0;
	picasaweb_gdata_t *gdata = NULL;
	char * cache_dir = NULL;
	char *filename=NULL;
	picasaweb_feed_t * feed_albums=NULL;

	facebook_data * fdata=NULL;
	facebook_photoalbums * photoalbums = NULL;
	facebook_feed *f_feed = NULL;
	
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		gdata = (picasaweb_gdata_t*)Swfext_GetNumber();
		cache_dir = Swfext_GetString();
		filename = Swfext_GetString();

		if(strlen(filename)==0)
			filename = NULL;
		feed_albums = picasa_load_albums_info(gdata,cache_dir,filename);
		rtn = (int)feed_albums;
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		fdata = (facebook_data*)Swfext_GetNumber();
		cache_dir = Swfext_GetString();
		filename = Swfext_GetString();
		
		if(strlen(filename)==0)
			filename = NULL;
		photoalbums = facebook_load_albums_info(fdata,cache_dir,filename);
		web_info("Get Albums info feed_albums=0x%x",photoalbums);
		f_feed = facebook_feed_type(ALBUM_FEED,(void *)photoalbums);
		rtn = (int)f_feed;
	}
#endif

	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}


/**
@brief save the albums info into a file
@param[in] gdata	: initialize by calling the webalbum_init_gdata()
@param[in] feed_albums	: the pointer to the picasaweb_feed_t which include all information of the albums in the account
@param[in] which_album	: which album will specified, the range of this value is 0~(feed_albums->entry_num-1)
@param[in] cache_dir	: under which folder will the file be created
@return :
	-1 	: failed
	0	: succeed
**/
static int webalbum_save_album_info(void * handle)
{
	picasaweb_gdata_t *gdata = NULL;
	picasaweb_feed_t * feed_albums=NULL;
	int which_album = 0;
	char * cache_dir = NULL;
	int rtn = 0;

	facebook_data * fdata=NULL;
	facebook_feed *f_feed = NULL;
	
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		gdata = (picasaweb_gdata_t*)Swfext_GetNumber();
		feed_albums = (picasaweb_feed_t*)Swfext_GetNumber();
		which_album = Swfext_GetNumber();
		cache_dir = Swfext_GetString();
		
		rtn =  picasa_save_album_info(gdata,feed_albums,which_album,cache_dir);
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		fdata = (facebook_data*)Swfext_GetNumber();
		f_feed = (facebook_feed*)Swfext_GetNumber();
		which_album = Swfext_GetNumber();
		cache_dir = Swfext_GetString();
		
		if(f_feed && f_feed->type==ALBUM_FEED)
			rtn = facebook_save_album_info(fdata,(facebook_photoalbums *)f_feed->feed_p,which_album,cache_dir);
	}
#endif

	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}


/**
@brief load the album info into a file
@param[in] gdata	: initialize by calling the webalbum_init_gdata()
@param[in] feed_albums	: the pointer to the picasaweb_feed_t which include all information of the albums in the account
@param[in] which_album	: which album will specified, the range of this value is 0~(feed_albums->entry_num-1)
@param[in] cache_dir	: under which folder will the file be created
@return :
	NULL	: load album info error
	others	: the pointer to the picasaweb_feed_t which include all information of the album
**/
static int webalbum_load_album_info(void * handle)
{
	int rtn = 0;
	picasaweb_gdata_t *gdata = NULL;
	picasaweb_feed_t * feed_albums=NULL;
	int which_album = 0;
	char * cache_dir = NULL;
	picasaweb_feed_t * feed_album=NULL;

	facebook_data * fdata=NULL;
	facebook_feed *f_feed=NULL;
	facebook_feed *photo_feed = NULL;
	facebook_photoalbums *photoalbums=NULL;
	
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		gdata = (picasaweb_gdata_t*)Swfext_GetNumber();
		feed_albums = (picasaweb_feed_t*)Swfext_GetNumber();
		which_album = Swfext_GetNumber();
		cache_dir = Swfext_GetString();
		
		feed_album = picasa_load_album_info(gdata,feed_albums,which_album,cache_dir);
		rtn = (int)feed_album;
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		fdata = (facebook_data*)Swfext_GetNumber();
		f_feed = (facebook_feed*)Swfext_GetNumber();
		which_album = Swfext_GetNumber();
		cache_dir = Swfext_GetString();
		photoalbums = (facebook_photoalbums*)f_feed->feed_p;
		if(facebook_load_album_info(fdata,photoalbums,which_album,cache_dir)==0){
			web_info("Get Album info feed_albums=0x%x",photoalbums->album_entry+which_album);
			photo_feed = facebook_feed_type(PHOTO_FEED,(void *)(photoalbums->album_entry+which_album));
			rtn = (int)photo_feed;
		}
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}


/**
@brief save the contact info into a file
@param[in] gdata	: initialize by calling the webalbum_init_gdata()
@param[in] feed_contact	: get from webalbum_get_contact()
@param[in] cache_dir	: under which folder will the file be created
@return :
	-1 	: failed
	0	: succeed
**/
static int webalbum_save_contact_info(void * handle)
{
	picasaweb_gdata_t *gdata = NULL;
	picasaweb_feed_t * feed_contact=NULL;
	char * cache_dir = NULL;
	int rtn = 0;

	facebook_data * fdata=NULL;
	facebook_feed *f_feed = NULL;
	
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		gdata = (picasaweb_gdata_t*)Swfext_GetNumber();
		feed_contact = (picasaweb_feed_t*)Swfext_GetNumber();
		cache_dir = Swfext_GetString();
		
		rtn= picasa_save_contact_info(gdata,feed_contact,cache_dir);
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		fdata = (facebook_data*)Swfext_GetNumber();
		f_feed = (facebook_feed*)Swfext_GetNumber();
		cache_dir = Swfext_GetString();
		
		if(f_feed && f_feed->type==CONTACT_FEED)
			rtn = facebook_save_contact_info(fdata,(facebook_contact *)f_feed->feed_p,cache_dir);
	}
#endif

	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}


/**
@brief load contact info from the file
@param[in] gdata	: initialize by calling the webalbum_init_gdata()
@param[in] cache_dir	: under which folder will the file be stored
@return :
	NULL	: load albums info error
	others	: he pointer to the picasaweb_feed_t which include all information of the contact in the account
**/
static int webalbum_load_contact_info(void * handle)
{
	int rtn = 0;
	picasaweb_gdata_t *gdata = NULL;
	char * cache_dir = NULL;
	picasaweb_feed_t * feed_contact=NULL;

	facebook_data * fdata=NULL;
	facebook_feed *f_feed=NULL;
	facebook_contact *contact=NULL;
	
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		gdata = (picasaweb_gdata_t*)Swfext_GetNumber();
		cache_dir = Swfext_GetString();
		
		feed_contact = picasa_load_contact_info(gdata,cache_dir);
		rtn = (int)feed_contact;
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		fdata = (facebook_data*)Swfext_GetNumber();
		cache_dir = Swfext_GetString();
		
		contact = facebook_load_contact_info(fdata,cache_dir);
		web_info("Get Contact info contact=0x%x",contact);
		f_feed = facebook_feed_type(CONTACT_FEED,(void *)contact);
		rtn = (int)f_feed;
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int webalbum_save_userinfo(void * handle)
{
	int rtn = 0;
	picasaweb_gdata_t *gdata = NULL;

	facebook_data * fdata=NULL;
	
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		gdata = (picasaweb_gdata_t*)Swfext_GetNumber();
		if(gdata)
			rtn = picasa_save_userinfo(gdata);
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		fdata = (facebook_data*)Swfext_GetNumber();
		if(fdata)
			rtn = facebook_save_userinfo(fdata);
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int webalbum_check_user(void * handle)
{
	int rtn = 0;
	
	SWFEXT_FUNC_BEGIN(handle);
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		rtn = picasa_check_user();
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		rtn = facebook_check_user();
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int webalbum_load_useraccount(void * handle)
{
	int user_index = 0;
	char user_account[LOGIN_MAX_LEN] = {0};
	
	SWFEXT_FUNC_BEGIN(handle);
	user_index = Swfext_GetNumber();
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		picasa_load_useraccount(user_index,user_account);
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		facebook_load_useraccount(user_index,user_account);
	}
#endif
	web_info("Get User PWD %s",user_account);
	Swfext_PutString(user_account);
	SWFEXT_FUNC_END();
}

static int webalbum_load_userpwd(void * handle)
{
	int user_index = 0;
	char user_pwd[LOGIN_MAX_LEN] = {0};
	
	SWFEXT_FUNC_BEGIN(handle);
	user_index = Swfext_GetNumber();
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		picasa_load_userpwd(user_index,user_pwd);
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		facebook_load_userpwd(user_index,user_pwd);
	}
#endif
	web_info("Get User PWD %s",user_pwd);
	Swfext_PutString(user_pwd);
	SWFEXT_FUNC_END();
}

static int webalbum_create_update(void * handle)
{
	int rtn = -1;
	int iscache=0;
	char* cache_dir=NULL;
	int isthumbnail = 0;
	void * webalbumdata=NULL;
	picasaweb_gdata_t *gdata = NULL;

	facebook_data * fdata=NULL;
	
	SWFEXT_FUNC_BEGIN(handle);
	webalbumdata = (void*)Swfext_GetNumber();
	iscache = Swfext_GetNumber();
	cache_dir = Swfext_GetString();
	isthumbnail=Swfext_GetNumber();
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		gdata = (picasaweb_gdata_t*)webalbumdata;
		picasa_create_update(gdata,iscache,cache_dir,isthumbnail);
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		fdata = (facebook_data*)webalbumdata;
		rtn = facebook_create_update(fdata,iscache,cache_dir,isthumbnail);
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int webalbum_select_friend(void * handle)
{
	int rtn = -1;
	int friend_index = -1;
	char * friend_id=NULL;
	void * webalbumdata=NULL;
	
	SWFEXT_FUNC_BEGIN(handle);
	friend_index = Swfext_GetNumber();
	friend_id = Swfext_GetString();
#ifdef MODULE_CONFIG_PICASA
	if(login_type==PICASA){
		rtn = picasa_select_friend(friend_index, friend_id);
	}
#endif
#ifdef MODULE_CONFIG_FACEBOOK
	if(login_type==FACEBOOK){
		rtn = facebook_select_friend(friend_index, friend_id);
	}
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

int swfext_webalbum_register(void)
{
	SWFEXT_REGISTER("web_set_logintype", webalbum_set_logintype);
	SWFEXT_REGISTER("web_set_login", webalbum_set_login);
	
	SWFEXT_REGISTER("web_init_gdata", webalbum_init_gdata);
	SWFEXT_REGISTER("web_free_gdata", webalbum_free_gdata);

	SWFEXT_REGISTER("web_get_userinfo", webalbum_get_userinfo);
	SWFEXT_REGISTER("web_free_userinfo", webalbum_free_userinfo);

	SWFEXT_REGISTER("web_get_contact", webalbum_get_contact);
	SWFEXT_REGISTER("web_free_contact", webalbum_free_contact);
	
	SWFEXT_REGISTER("web_get_friendlists", webalbum_get_friendlists);
	SWFEXT_REGISTER("web_free_friendlists", webalbum_free_friendlists);
	SWFEXT_REGISTER("web_get_member", webalbum_get_member);
	SWFEXT_REGISTER("web_free_member", webalbum_free_member);
	
	SWFEXT_REGISTER("web_get_albums_info", webalbum_get_albums_info);
	SWFEXT_REGISTER("web_free_albums_info", webalbum_free_albums_info);
	
	SWFEXT_REGISTER("web_get_album_info", webalbum_get_album_info);
	SWFEXT_REGISTER("web_free_album_info", webalbum_free_album_info);
	
	SWFEXT_REGISTER("web_authentication", webalbum_authentication);
	SWFEXT_REGISTER("web_query_auth_status", webalbum_query_auth_status);
	SWFEXT_REGISTER("web_create_new_album", webalbum_create_new_album);
	SWFEXT_REGISTER("web_delete_album", webalbum_delete_album);
	SWFEXT_REGISTER("web_update_album_info", webalbum_update_album_info);
	
	SWFEXT_REGISTER("web_init_upload_info", webalbum_init_upload_info);
	SWFEXT_REGISTER("web_free_upload_info", webalbum_free_upload_info);
	SWFEXT_REGISTER("web_upload_photo", webalbum_upload_photo);
	SWFEXT_REGISTER("web_query_upload_status", webalbum_query_upload_status);
	

	SWFEXT_REGISTER("web_update_photo_info", webalbum_update_photo_info);
	SWFEXT_REGISTER("web_delete_photo", webalbum_delete_photo);
	
	SWFEXT_REGISTER("web_init_download_info", webalbum_init_download_info);
	SWFEXT_REGISTER("web_free_download_info", webalbum_free_download_info);
	SWFEXT_REGISTER("web_download_photo", webalbum_download_photo);
	SWFEXT_REGISTER("web_query_download_status", webalbum_query_download_status);
	
	SWFEXT_REGISTER("web_send_msg", webalbum_send_msg);
	SWFEXT_REGISTER("web_get_entry", webalbum_get_entry);	
	SWFEXT_REGISTER("web_get_info", webalbum_get_info);

	SWFEXT_REGISTER("web_save_albums_info", webalbum_save_albums_info);
	SWFEXT_REGISTER("web_load_albums_info", webalbum_load_albums_info);
	SWFEXT_REGISTER("web_save_album_info", webalbum_save_album_info);
	SWFEXT_REGISTER("web_load_album_info", webalbum_load_album_info);
	SWFEXT_REGISTER("web_save_contact_info", webalbum_save_contact_info);
	SWFEXT_REGISTER("web_load_contact_info", webalbum_load_contact_info);

	SWFEXT_REGISTER("web_save_usernfo", webalbum_save_userinfo);
	SWFEXT_REGISTER("web_check_user", webalbum_check_user);
	SWFEXT_REGISTER("web_load_useraccount", webalbum_load_useraccount);
	SWFEXT_REGISTER("web_load_userpwd", webalbum_load_userpwd);

	SWFEXT_REGISTER("web_create_update", webalbum_create_update);

	SWFEXT_REGISTER("web_select_friend", webalbum_select_friend);
	return 0;
}
#else

int swfext_webalbum_register(void)
{
	return 0;
}

#endif	/** MODULE_CONFIG_WEBALBUM */
