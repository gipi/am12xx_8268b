#ifdef MODULE_CONFIG_WEBMAIL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "swf_ext.h"
#include "libetpan/libetpan.h"
#include "pthread.h"


//#define  WEBMAIL_DEBUG
#ifdef  WEBMAIL_DEBUG
#define p_printf(format,args...)   printf(format, ##args)
#else
#define p_printf(format,args...)   do {} while (0)
#endif

#define IMAP_PORT 	143
#define IMAP_SSL_PORT 	993
#define MAX_ARRAY_LEN	512


typedef struct
{
	char username[MAX_ARRAY_LEN];
	char password[MAX_ARRAY_LEN];
	char server[MAX_ARRAY_LEN];
	int with_ssl;
	int port;
}IMAP_LOGIN_PARAM;


enum {

  NO_ERROR,

  ERROR_FILE = 1,
  ERROR_MEMORY = 2,
  ERROR_INVAL = 3,
  ERROR_FETCH = 4
};

#define DEST_CHARSET "UTF-8"

#define WEMAIL_CFG_FILE 	"/mnt/vram/webmail.ini"

enum
{
	STATUS_FETCH_FINISHED=0,
	STATUS_FETCHING = 1,
	STATUS_FETCH_ERROR = -1,
	STATUS_FETCH_STOP = -2
};


typedef struct 
{
	int start_index;
	int count;
	pthread_t update_thread_id;
//	char last_update_time[256];
//	char this_update_time[256];

}FETCH_PARAM;

enum 
{
	STATUS_CONNECT_OK = 0,
	STATUS_CONNECTING = 1,
	STATUS_CONNECT_ERROR= -1
};

static IMAP_LOGIN_PARAM g_login_param;

static char g_workpath[MAX_ARRAY_LEN] = "/mnt/udisk";//保存的位置
static char g_filter[MAX_ARRAY_LEN] = "";

static  struct mailstorage * storage = NULL;
static  struct mailfolder * folder = NULL;

static int connect_status;
static int fetch_status;
static pthread_t thread_id;
static FETCH_PARAM fetch_param;

uint32_t webmail_last_update_id= 0;
uint32_t webmail_this_update_id= 0;


static int webmail_fetch_message(mailmessage * msg_info,
   struct mailmime * mime_part,
    struct mailmime_single_fields * fields,
    char ** result, size_t * result_len);
static int save_mime_content(mailmessage * msg_info, struct mailmime * mime_part);
static int webmail_write_data(char * filename, char * data, size_t len);
static void *  webmail_save_attachment(void * arg);

static void to_upper_case(char * str)
{
	int i = 0;
	while(str[i] != '\0')
	{
		if(str[i] >= 'a' && str[i] <= 'z' )
		{
			str[i] = str[i] + 'A' - 'a';
		}
		i++;
	}
}

static int  is_filter_type(char * file,const char * ext)
{
	
	int len;
	len = strlen(file);

	int i; 
	for(i = len - 1;i >= 1;i--)
	{
		if(file[i] == '.')
		{
			char * pret = NULL;
			
			char  * pstr = file+i+1;
		
			to_upper_case(pstr);
		
			pret = strstr(ext,pstr);
		
			if(pret != NULL)
			{
				return 1;
			}
			
			return 0;
		}
	}
	return 0;
}

/* write content to the given filename */

 int webmail_write_data(char * filename, char * data, size_t len)
{
	size_t write_len;
	FILE * f;
	int res;
 
	mode_t old_umask;
  
	old_umask = umask(0077);
	f = fopen(filename, "wb");
	umask(old_umask);
 
	if (f == NULL) {
		printf("%s,%d,filename = %s error\n",__FILE__,__LINE__,filename);	
		res = ERROR_FILE;
		goto err;
  	}

	write_len = fwrite(data, 1, len, f);
	if (write_len < len) {
		printf("%s,%d,fwrite error,write_len = %d\n",__FILE__,__LINE__,write_len);	
		res = ERROR_FILE;
		goto close;
	}
	else
	{
		printf("writing %s, %d bytes\n", filename, (int) len);
	}

	fclose(f);

	return NO_ERROR;

 close:
  	fclose(f);
 err:
	return res;
}


/* save attachment */

int save_mime_content(mailmessage * msg_info, struct mailmime * mime_part)
{
	char * body = NULL;
	size_t body_len = 0;
	int r;
	char * filename;
	struct mailmime_single_fields fields;
	int res = -1;

	memset(&fields, 0, sizeof(struct mailmime_single_fields));
	if (mime_part->mm_mime_fields != NULL)
		mailmime_single_fields_init(&fields, mime_part->mm_mime_fields,
			mime_part->mm_content_type);

	filename = fields.fld_disposition_filename;

	if (filename == NULL)
	filename = fields.fld_content_name;

	if (filename == NULL)
	{
		printf("no attachment\n");
		return ERROR_INVAL;
	}

	//date = get_mail_date(msg_info);
	//printf("mail date ===%ld\n",date);
	//timestr = ctime(&date);
	//printf("mail created at %s\n",timestr);	
	//decode 文件名
	size_t cur_token = 0;
	char * decoded_filename = NULL;
	p_printf("%s,%d\n",__FILE__,__LINE__);
	r = mailmime_encoded_phrase_parse(DEST_CHARSET,
	   filename, strlen(filename),
	   &cur_token, DEST_CHARSET,
	   &decoded_filename);
	p_printf("%s,%d\n",__FILE__,__LINE__);
	if (r != MAILIMF_NO_ERROR) 
	{
		printf("decode filename error,r= %d\n",r);
	}
	else
	{
		printf("decode filename = %s\n",decoded_filename);
	}

	//查询后辍名
	if(g_filter[0] == '\0' || is_filter_type(decoded_filename, g_filter))
	{

		r = webmail_fetch_message(msg_info,mime_part, &fields, &body, &body_len);
		if (r != NO_ERROR) 
		{
			printf("webmail_fetch_message  error \n");
			res = r;
			goto err;
		}
#if 0
		if(strcmp(webmail_last_update_time,fields.fld_disposition_creation_date)>0){
			printf("had been updated\n");
			goto free;
		}
		if(webmail_last_update_id >= msg_info->msg_index ){
			p_printf("sorry:this attachment had been updated \n");
			goto free;
		}
#endif
		char path[MAX_ARRAY_LEN*2];
		sprintf(path,"%s/%d_%s",g_workpath,msg_info->msg_index,decoded_filename);
		
		printf("attachment %s is downloading...\n",path);

		r = webmail_write_data(path, body, body_len);
		p_printf("%s,%d\n",__FILE__,__LINE__);
		if (decoded_filename != NULL)
	   		 free(decoded_filename);
		if (r != NO_ERROR) 
		{
			printf("webmail_write_data  error \n");
			res = r;
			goto free;
		}
#if 0
		//完成更新，记录更新时间(记录最晚的时间)
		if(strcmp(webmail_this_update_time,fields.fld_disposition_creation_date)<0)
			strcpy(webmail_this_update_time,fields.fld_disposition_creation_date);
		//更新完成 记录ID号
		if(webmail_this_update_id <msg_info->msg_index )
			webmail_this_update_id  = msg_info->msg_index;	
#endif

	}
	else
	{
		printf("omit file :%s\n",decoded_filename);
	}

	mailmime_decoded_part_free(body);
	p_printf("%s,%d\n",__FILE__,__LINE__);
	return NO_ERROR;

free:
	mailmime_decoded_part_free(body);
	p_printf("%s,%d\n",__FILE__,__LINE__);
err:
	return res;
}



/* fetch attachments */

static int webmail_fetch_mime(mailmessage * msg_info,
     struct mailmime * mime)
{
	int r;
	clistiter * cur;
	struct mailmime_single_fields fields;
	int res;

	memset(&fields, 0, sizeof(struct mailmime_single_fields));
	if (mime->mm_mime_fields != NULL)
		mailmime_single_fields_init(&fields, mime->mm_mime_fields,
		mime->mm_content_type);

	switch(mime->mm_type) 
	{
		case MAILMIME_SINGLE:
  			printf("MAILMIME_SINGLE \n ");
			save_mime_content(msg_info, mime);
	
			break;
    
		case MAILMIME_MULTIPLE:
			printf("MAILMIME_MULTIPLE \n ");
   			 for(cur = clist_begin(mime->mm_data.mm_multipart.mm_mp_list) ;
        			cur != NULL ; cur = clist_next(cur)) 
			{
      
				r = webmail_fetch_mime(msg_info, clist_content(cur));
				if (r != NO_ERROR) 
				{
					printf("webmail_fetch_mime  error ,r = %d\n",r);
					res = r;
					goto err;
				}
			}

			break;
      
		case MAILMIME_MESSAGE:
			printf("MAILMIME_MESSAGE \n ");
			if (mime->mm_data.mm_message.mm_msg_mime != NULL) 
			{
				r = webmail_fetch_mime( msg_info, mime->mm_data.mm_message.mm_msg_mime);
				if (r != NO_ERROR) 
				{
					printf("webmail_fetch_mime  error ,r = %d\n",r);
					res = r;
					goto err;
				}
			}

			break;
	}

	return NO_ERROR;

 err:
	return res;
}


 int webmail_fetch_message(mailmessage * msg_info,
    struct mailmime * mime_part,
    struct mailmime_single_fields * fields,
    char ** result, size_t * result_len)
{
	char * data;
	size_t len;
	int r;
	int encoding;
	char * decoded;
	size_t decoded_len;
	size_t cur_token;
	int res;
	int encoded;

	encoded = 0;

	r = mailmessage_fetch_section(msg_info,
			mime_part, &data, &len);
	if (r != MAIL_NO_ERROR) {
		res = ERROR_FETCH;
		goto err;
	}

	encoded = 1;

  	printf("mailmessage_fetch_section,len = %d\n",len);
  	/* decode message */

	if (encoded) {
		if (fields->fld_encoding != NULL)
      			encoding = fields->fld_encoding->enc_type;
    		else 
			encoding = MAILMIME_MECHANISM_8BIT;
	}
	else {
		encoding = MAILMIME_MECHANISM_8BIT;
	}

	cur_token = 0;
	r = mailmime_part_parse(data, len, &cur_token,
			  encoding, &decoded, &decoded_len);
	if (r != MAILIMF_NO_ERROR) {
		printf("%s ,%d,r= %d\n",__FILE__,__LINE__,r);
		res = ERROR_FETCH;
		goto free; 
	}

	printf("mailmime_part_parse,decoded_len = %d\n",decoded_len);

	mailmessage_fetch_result_free(msg_info, data);
  
	* result = decoded;
	* result_len = decoded_len;
  
	return NO_ERROR;
  
 free:
	mailmessage_fetch_result_free(msg_info, data);
 err:
  	return res;
}

	  

static int webmail_set_login(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("enter %s\n",__FUNCTION__);
	int ret = 0;
	if( Swfext_GetParamNum() != 5)
	{
		printf("%s parameter wrong\n",__FUNCTION__);
		ret = -1;
		goto END;	
	}

	
	strcpy(g_login_param.username, Swfext_GetString());	
	strcpy(g_login_param.password, Swfext_GetString());	
	strcpy(g_login_param.server, Swfext_GetString());
	
	g_login_param.with_ssl = Swfext_GetNumber();
	g_login_param.port = Swfext_GetNumber();
	p_printf("usrname = %s,password = %s,server = %s,with_ssl = %d,port=%d\n",
		g_login_param.username,g_login_param.password,g_login_param.server,
		g_login_param.with_ssl,g_login_param.port);


	FILE * fp = fopen(WEMAIL_CFG_FILE,"w");

	char tmpstr[100];
	sprintf(tmpstr,"&web_username=%s\n",g_login_param.username);

	fwrite(tmpstr,1,strlen(tmpstr),fp);

	sprintf(tmpstr,"&web_password=%s\n",g_login_param.password);

	fwrite(tmpstr,1,strlen(tmpstr),fp);

	sprintf(tmpstr,"&web_server=%s\n",g_login_param.server);

       fwrite(tmpstr,1,strlen(tmpstr),fp);   

	sprintf(tmpstr,"&web_ssl=%d\n",g_login_param.with_ssl);

	fwrite(tmpstr,1,strlen(tmpstr),fp);

	sprintf(tmpstr,"&web_port=%d\n",g_login_param.port);

	fwrite(tmpstr,1,strlen(tmpstr),fp);

	int fd;

	fflush((FILE *)fp);

	fd = fileno((FILE *)fp);

	if(fsync(fd)==-1)
	{
		printf("%s,%d: Fflush Error!\n",__FILE__,__LINE__);
             fclose(fp);
	}


END:
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}

static int webmail_set_workpath(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("enter %s\n",__FUNCTION__);

	int ret = 0;
	if( Swfext_GetParamNum() != 1)
	{
		printf("%s parameter wrong\n",__FUNCTION__);
		ret = -1;
		goto END;	
	}
	strcpy(g_workpath, Swfext_GetString());
	
	p_printf("g_workpath = %s\n",g_workpath);
END:
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}

static int webmail_set_attachment_filter(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("enter %s\n",__FUNCTION__);

	int ret = 0;
	if( Swfext_GetParamNum() != 1)
	{
		printf("%s parameter wrong\n",__FUNCTION__);
		ret = -1;
		goto END;	
	}
	strcpy(g_filter, Swfext_GetString());
	
	to_upper_case(g_filter);
	
	p_printf("g_filter = %s\n",g_filter);
END:
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}

static void * webmail_connect(void *arg)
{

	printf("enter %s\n",__FUNCTION__);
  /* build the storage structure */

	storage = mailstorage_new(NULL);
	if (storage == NULL) 
	{
	    	printf("mailstorage_new error\n");
		connect_status = STATUS_CONNECT_ERROR;
	    	goto END;
	}

	
	int connection_type = g_login_param.with_ssl ? CONNECTION_TYPE_TLS :CONNECTION_TYPE_PLAIN;


	int  r = imap_mailstorage_init(storage, g_login_param.server, g_login_param.port,
	  	NULL, connection_type,IMAP_AUTH_TYPE_PLAIN, 
	  	g_login_param.username, g_login_param.password, 0, NULL);

	if (r != MAIL_NO_ERROR) 
	{
		printf("imap_mailstorage_init error\n");
	
		mailstorage_free(storage);
		storage = NULL;
	 	connect_status = STATUS_CONNECT_ERROR;
	      goto END;
	}

	/* get the folder structure */
  	folder = mailfolder_new(storage, "INBOX", NULL);
	if (folder == NULL) 
	{
	    	printf("mailfolder_new error\n");
		connect_status = STATUS_CONNECT_ERROR;
	    	goto END;
	}
	printf("mailfolder  INBOX connect...\n");
	r = mailfolder_connect(folder);
	p_printf("%s,%d\n",__FILE__,__LINE__);
	if (r != MAIL_NO_ERROR) 
	{
		printf("error mailfolder_connect, r = %d\n",r);
		connect_status = STATUS_CONNECT_ERROR;
		mailstorage_free(storage);
		storage = NULL;
		mailfolder_free(folder);
		folder = NULL;
	}
	else
	{
		connect_status = STATUS_CONNECT_OK;
	}
	
END:
	return NULL;
}


static int webmail_start_connect(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	connect_status = STATUS_CONNECTING;
	int ret = pthread_create (&thread_id,NULL,webmail_connect,NULL);
	if(ret)
	{
		connect_status = STATUS_CONNECT_ERROR;
		printf("%s error,ret = %d\n",__FUNCTION__,ret);
	}
	
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}
static int webmail_start_fetch_attachment(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	int ret = -1;
	if(connect_status == STATUS_CONNECT_OK)
	{
		fetch_param.start_index = Swfext_GetNumber();
		fetch_param.count = Swfext_GetNumber();
		fetch_status = STATUS_FETCHING;
		ret = pthread_create (&thread_id,NULL,webmail_save_attachment,(void *)&fetch_param);
		if(ret)
		{
			printf("%s error,ret = %d\n",__FUNCTION__,ret);
		}
	}

	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}

static int webmail_get_connect_status(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(connect_status);
	SWFEXT_FUNC_END();	
}


static int webmail_get_fetch_status(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(fetch_status);
	SWFEXT_FUNC_END();	
}



static int webmail_get_mailnumbers(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("enter %s\n",__FUNCTION__);
	unsigned int total = 0;
	if(folder == NULL)
		goto END;
	
	printf(" folder->fld_pathname = %s\n", folder->fld_pathname);
	int r =  mailsession_messages_number(folder->fld_session, folder->fld_pathname,&total);
	if (r != MAIL_NO_ERROR) 
	{
	    printf("error mailsession_messages_number, r = %d\n",r);
	    goto END;
	}
	
	p_printf("messages total = %d\n",total);
	
END:
	Swfext_PutNumber(total);
	SWFEXT_FUNC_END();	
}

static int webmail_get_mail_id(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("enter %s\n",__FUNCTION__);
	
	int  mail_id = -1;
	if( Swfext_GetParamNum() != 1)
	{
		printf("%s parameter wrong\n",__FUNCTION__);
		goto END;	
	}
	int index = Swfext_GetNumber();

	struct mailmessage_list * env_list;
	
	if(folder == NULL)
		goto END;
	
	/* get the list of messages numbers of the folder */

	int r = mailsession_get_messages_list(folder->fld_session, &env_list);
	
	if (r != MAIL_NO_ERROR) 
	{
	    printf("error message list,r=%d\n",r);
	    goto END;
	}

	if(index >= 0 && index < carray_count(env_list->msg_tab))
	{
		  mailmessage * msg = carray_get(env_list->msg_tab, index);
		  mail_id = msg->msg_index;
		  printf("mail_id = %d\n",mail_id);
	}
	else
	{
		printf("index = %d is out of range !!!!!\n",index);
	}
	
  	/* free structure */

	mailmessage_list_free(env_list);
	
END:
	Swfext_PutNumber(mail_id);
	SWFEXT_FUNC_END();	
}


static void thread_cleanup(void * args)
{
	struct mailmessage_list * env_list = (struct mailmessage_list * )args;
	fetch_status = STATUS_FETCH_STOP;
	mailmessage_list_free(env_list);
}
void *  webmail_save_attachment(void * arg)
{
	printf("enter %s\n",__FUNCTION__);
	
	FETCH_PARAM * fetch_param = (FETCH_PARAM *)arg;
	int mail_id;
	int start_index = fetch_param->start_index;
	int count = fetch_param->count;

	struct mailmessage_list * env_list;
	
	if(folder == NULL)
		goto END;
	
	/* get the list of messages numbers of the folder */
	
	int r = mailsession_get_messages_list(folder->fld_session, &env_list);
	
	if (r != MAIL_NO_ERROR) 
	{
	    printf("error message list,r=%d\n",r);
	    goto END;
	}

	pthread_cleanup_push(thread_cleanup,(void *)env_list);
	int i;
	for(i=start_index;i<start_index+count;i++)
	{
		pthread_testcancel();
		mailmessage * msg = carray_get(env_list->msg_tab, i);
		mail_id = msg->msg_index;
		printf("=========== index = %d,mail_id = %d ===============\n",i,mail_id);
	
		fetch_status = (i << 16) | STATUS_FETCHING;
		// mailmessage * msg = NULL;
		struct mailmime * mime = NULL;

		r = mailsession_get_message(folder->fld_session, mail_id, &msg);
		if (r != MAIL_NO_ERROR) 
		{
	     		 printf("mailsession_get_message error, r= %d \n", r);
			fetch_status = (i << 16) | STATUS_FETCH_ERROR;
			break;
		}
		

		r = mailmessage_get_bodystructure(msg, &mime);
		 
		if (r != MAIL_NO_ERROR) 
		{
	     		 printf("mailmessage_get_bodystructure error, r= %d \n", r);
			fetch_status =  (i << 16) |STATUS_FETCH_ERROR;
			break;
		}
		
		 r = webmail_fetch_mime(msg, mime);
		
		if (r != MAIL_NO_ERROR) 
		{
	     		 printf("webmail_fetch_mime error, r= %d \n", r);
			fetch_status = (i << 16) | STATUS_FETCH_ERROR;
			break;
		}
		
		if(msg != NULL)
	   	 	mailmessage_free(msg);
	}
	pthread_cleanup_pop(0);

	/* free structure */
	fetch_status = STATUS_FETCH_FINISHED;
	mailmessage_list_free(env_list);
END:
	return NULL;
}
static int webmail_stop_fetch(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	pthread_cancel(thread_id);
	pthread_join(thread_id,NULL);
	SWFEXT_FUNC_END();	
}

static int webmail_exit(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("enter %s\n",__FUNCTION__);

	pthread_cancel(thread_id);
	pthread_join(thread_id,NULL);
	
	if(folder != NULL)
	{
		mailfolder_free(folder);
		folder = NULL;
	}

	if(storage != NULL)
	{
		mailstorage_free(storage);
		storage = NULL;
	}
	SWFEXT_FUNC_END();	
}

#if 1
//newly added
static int webmail_get_unseen_mailnum(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("enter %s\n",__FUNCTION__);
	unsigned int total = 0;
	if(folder == NULL)
		goto END;
	
	printf(" folder->fld_pathname = %s\n", folder->fld_pathname);
	int r =  mailsession_unseen_number(folder->fld_session, folder->fld_pathname,&total);
	if (r != MAIL_NO_ERROR) 
	{
	    printf("error mailsession_messages_number, r = %d\n",r);
	    goto END;
	}
	
	p_printf("messages total = %d\n",total);
	
END:
	Swfext_PutNumber(total);
	SWFEXT_FUNC_END();	

}



static time_t get_mail_date(mailmessage * msg)
{
  struct tm tmval;
  time_t timeval;
  struct mailimf_date_time * date_time;

  if (msg->msg_single_fields.fld_orig_date == NULL)
    return (time_t) -1;
  
  date_time = msg->msg_single_fields.fld_orig_date->dt_date_time;
  
  tmval.tm_sec  = date_time->dt_sec;
  tmval.tm_min  = date_time->dt_min;
  tmval.tm_hour = date_time->dt_hour;
  tmval.tm_sec  = date_time->dt_sec;
  tmval.tm_mday = date_time->dt_day;
  tmval.tm_mon  = date_time->dt_month - 1;
  tmval.tm_year = date_time->dt_year - 1900;
  
  timeval = mail_mkgmtime(&tmval);
  
  timeval -= date_time->dt_zone * 36;
  
  return timeval;
}


void *webmail_update_thread(void *arg)
{
	p_printf("######start %s######\n",__FUNCTION__);

	uint32_t mail_total;
	int i;
	struct timeval start_time, end_time;
	
	FETCH_PARAM * fetch_param = (FETCH_PARAM *)arg;
	int mail_id;
	struct mailmessage_list * env_list;

	if(NULL == arg){
		printf("webmail update err:invalid arg\n");
		goto END;
	}
		
	if(NULL == folder)
		goto END;
	
	pthread_cleanup_push(thread_cleanup,(void *)env_list);
	
	while(1){
		pthread_testcancel();
		
		printf("++++++++++++++++++++++++++webmail update start++++++++++++++++++++++++++++\n");
	/* get the list of messages numbers of the folder */
	
	int r = mailsession_get_messages_list(folder->fld_session, &env_list);
	
	if (r != MAIL_NO_ERROR) 
	{
	    printf("error message list,r=%d\n",r);
	    goto END;
	}
	/*queries the number of messages in the folder*/
	r =  mailsession_messages_number(folder->fld_session, folder->fld_pathname,&mail_total);
			p_printf("%d mails total\n",mail_total);
	
	if (r != MAIL_NO_ERROR) 
	{
	    printf("error mailsession_messages_number, r = %d\n",r);
	    goto END;
	}
		for(i=0;i<mail_total;i++)
		{
			mailmessage * msg = carray_get(env_list->msg_tab, i);
			mail_id = msg->msg_index;
			printf("=========== index = %d,mail_id = %d ===============\n",i,mail_id);
		
			fetch_status = (i << 16) | STATUS_FETCHING;
			// mailmessage * msg = NULL;
			struct mailmime * mime = NULL;

			r = mailsession_get_message(folder->fld_session, mail_id, &msg);
			if (r != MAIL_NO_ERROR) 
			{
		     		 printf("mailsession_get_message error, r= %d \n", r);
				fetch_status = (i << 16) | STATUS_FETCH_ERROR;
				break;
			}
			

			r = mailmessage_get_bodystructure(msg, &mime);
			 
			if (r != MAIL_NO_ERROR) 
			{
		     		 printf("mailmessage_get_bodystructure error, r= %d \n", r);
				fetch_status =  (i << 16) |STATUS_FETCH_ERROR;
				break;
			}

			/*之前更新过直接跳过*/
			if(webmail_last_update_id >= msg->msg_index ){
				printf("sorry:this mail had been updated,mailID ==%ld\n",msg->msg_index);
				continue;
			}
			
			 r = webmail_fetch_mime(msg, mime);
			
			if (r != MAIL_NO_ERROR) 
			{
		     		 printf("webmail_fetch_mime error, r= %d \n", r);
				fetch_status = (i << 16) | STATUS_FETCH_ERROR;
				break;
			}

			//当前邮件更新完成记录ID号
			if(webmail_this_update_id <msg->msg_index )
				webmail_this_update_id  = msg->msg_index;			
			
			if(msg != NULL)
		   	 	mailmessage_free(msg);
		}
		
		//所有邮件update完成，更新mailID
		if(webmail_last_update_id <webmail_this_update_id )
			webmail_last_update_id = webmail_this_update_id ;

		printf("++++++++++++webmail update compeleted: lastID=%ld thisID=%ld++++++++++++\n",webmail_last_update_id,webmail_this_update_id);
			

		/* free structure */
		fetch_status = STATUS_FETCH_FINISHED;
		mailmessage_list_free(env_list);
		gettimeofday(&start_time,NULL);
	DELAY:
		sleep(100);
		gettimeofday(&end_time, NULL);
		p_printf("start time is %d, end time is %d",start_time.tv_sec,end_time.tv_sec);
		if(end_time.tv_sec-start_time.tv_sec < 3600)// 1hour
			goto DELAY;

}
	pthread_cleanup_pop(0);
END:
	pthread_exit((void*)NULL);
	return NULL;
}

static int webmail_creat_update(void * handle)
{
	int ret = -1;
	pthread_t  tid;
	
	SWFEXT_FUNC_BEGIN(handle);

	if(connect_status == STATUS_CONNECT_OK)
	{
		
		fetch_status = STATUS_FETCHING;
		ret = pthread_create (&tid,NULL,webmail_update_thread,(void *)&fetch_param);
		if(ret){
			printf("update err:create webmail update thread error!\n");
		}
		else
			fetch_param.update_thread_id = tid;
		

	}
	else{
		printf("update err:please check connect\n");
		}
	
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}

static int webmail_exit_update(void * handle)
{
	int ret = -1;
	SWFEXT_FUNC_BEGIN(handle);
	p_printf("exit webmail update thread");
	
	if(fetch_param.update_thread_id == -1){
		p_printf("webmail update thread is not running");
		ret = 0;
	}
	pthread_cancel(fetch_param.update_thread_id);
	pthread_join(fetch_param.update_thread_id,NULL);
	fetch_param.update_thread_id = -1;
	ret = 0;
	
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}



#endif

int swfext_webmail_register(void)
{
	SWFEXT_REGISTER("webmail_set_login", webmail_set_login);
	SWFEXT_REGISTER("webmail_set_workpath", webmail_set_workpath);
	SWFEXT_REGISTER("webmail_set_attachment_filter", webmail_set_attachment_filter);
	SWFEXT_REGISTER("webmail_start_connect", webmail_start_connect);
	SWFEXT_REGISTER("webmail_update", webmail_creat_update);	
	SWFEXT_REGISTER("webmail_get_mailnumbers", webmail_get_mailnumbers);
	SWFEXT_REGISTER("webmail_get_mail_id", webmail_get_mail_id);
	SWFEXT_REGISTER("webmail_get_connect_status", webmail_get_connect_status);
	SWFEXT_REGISTER("webmail_get_fetch_status", webmail_get_fetch_status);
	SWFEXT_REGISTER("webmail_stop_fetch", webmail_stop_fetch);	
	SWFEXT_REGISTER("webmail_exit", webmail_exit);
	SWFEXT_REGISTER("webmail_start_fetch_attachment", webmail_start_fetch_attachment);	

	return 0;
	
}
#else
int swfext_webmail_register(void)
{
	return 0;
}
#endif	/** MODULE_CONFIG_WEBMAIL */
