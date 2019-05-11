#include "flickr_main.h"
#include "flickr_common.h"
#include "unistd.h"


//#define flickr_info(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
//#define flickr_err(fmt,arg...) printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

static char photo_url_str[FLICKR_TMP_BUF_LEN];

static char flickr_photoinfo_extra_str[][16]={
	"description,",
	"license,",
	"date_upload,",
	"date_taken,",
	"owner_name,",
	"icon_server,",
	"original_format,",
	"last_update,",
	"geo,",
	"tags,",
	"machine_tags,",
	"o_dims,",
	"views,",
	"media,",
	"path_alias,",
	"url_sq,",
	"url_t,",
	"url_s,",
	"url_m,",
	"url_z,",
	"url_l,",
	"url_o,"
};

static char flickr_contact_filter_str[][8]={
	"friends",
	"family", 
	"both",
	"neither"
};

#if 1
void *flickr_malloc(int size)
{
	return malloc(size);
}


char *flickr_realloc(char *ptr,unsigned int newsize)
{
	char *buf=(char*)realloc(ptr,newsize);
	if(buf==NULL){
		flickr_err("Realloc Failed!\n");
	}
	return buf;
}

char flickr_free(char* buffer)
{
	if(buffer!=NULL)
		free(buffer);
	return 0;
}

#endif

#if 1



#endif

/**
@brief copy the content to the atomic element, call the picasa_free_atomic() to free the space 
@param[in] content	: the content 
@param[in] atomic_name	: which atomic element will be filled
@return 
	- -1	: failed
	- 0	: success
@see flickr_free_atomic()
**/
int  flickr_fill_atomic(const char * content,flickr_data_atomic_t *atomic_name)
{
	char * buf=NULL;
	int str_len = 0;
	if(atomic_name==NULL)
		return 0;
	memset(atomic_name,0,sizeof(flickr_data_atomic_t));
	if(content==NULL){
		return -1;	
	}
	str_len = strlen((char*)content);
	if(str_len==0)
		return 0;
	buf = (char*)flickr_malloc(str_len+1);
	if(buf!=NULL){
		memcpy(buf,(char*)content,str_len);
		buf[str_len] = 0;
		atomic_name->data_len = str_len+1;
		atomic_name->data = buf;
	}
	else{
		flickr_err("Malloc Failed conent=%s\n",content);
		return -1;
	}
	//picasa_info("[ato_len=%d]=%s\n",atomic_name->data_len,atomic_name->data);
	return 0;
}


/**
@brief release the space which is occupied by calling picasa_fill_atomic()
@param[in] atimic_name	: where the content stored
@return 
	- -1	: failed
	- 0	: success
@see picasa_fill_atomic()
**/
int flickr_free_atomic(flickr_data_atomic_t *atomic_name)
{
	if(atomic_name==NULL)
		return 0;
	if(atomic_name->data && atomic_name->data_len!=0){
		flickr_free(atomic_name->data);
		atomic_name->data=NULL;
		atomic_name->data_len = 0;
		return 0;
	}
	else if(atomic_name->data==NULL && atomic_name->data_len==0){
		return 0;
	}
	else{
		flickr_err("Something must be wrong!\n");
		return -1;
	}
}

int flickr_data_write_init(flickr_data_write_t *data,FlickcurlDataWrite_e data_type,void* fp)
{
	memset(data,0,sizeof(flickr_data_write_t));
	if(data_type==flickcurl_data_write_file){
		data->file_handle = fp;
		data->data_type = flickcurl_data_write_file;
	}
	else
		data->data_type = flickcurl_data_write_buffer;
	return 0;
}


int flickr_data_write_free(flickr_data_write_t *data)
{
	if(data->data_head && data->data_type==flickcurl_data_write_buffer){
		flickr_free(data->data_head);
		data->data_head=NULL;
		data->data_cur=NULL;
		data->data_len = 0;
		data->data_used = 0;
	}
	return 0;
}


void * flickr_fopen(char *path, char *mode){
	return (void*)fopen(path, mode);
}


int flickr_fclose(void *fp){
	return fclose((FILE*)fp);
}

int flickr_fflush(void *fp)
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

int flickr_fremove(char* file)
{
	int err=0;
	if(file){
		err = unlink(file);
		if(err != 0){
			flickr_err("Remove Error file=%s\n",file);
			return -1;
		}
		return 0;
	}
	return -1;
}

long flickr_fwrite(void *fp, unsigned char *ptr, unsigned long nbytes){
	return fwrite(ptr, sizeof(unsigned char), nbytes,(FILE*)fp);
}

int _test_save_to_file(char* databuf,unsigned int datalen,char *filename)
{
#if 0
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
	fflush(fp);
	fclose(fp);
	return 1;
#endif
	return 0;
}
#if 1

int flickr_debug_callback (CURL* handle, curl_infotype info_type, char *ptr, size_t size, void *userdata)
{
#if 0
	char *locate = "http://www.flickr.com/signin/?";
	char *find = NULL;
	flickcurl_auth_info_t *auth_info=(flickcurl_auth_info_t*)userdata;
	int find_len = 0;
	int i = 0;
	//printf("<type=%d>%s",info_type,ptr);
	if(info_type==CURLINFO_HEADER_IN){
		find = strstr(ptr, locate);
		//flickr_info("info_type=%d",info_type);
		//flickr_info("Receive::=%s",ptr);
		if(find){
			int login_url_len=0;
			char * tmp=NULL;
			login_url_len = strlen(find);
			if(0){
				flickr_free(auth_info->login_url);
				auth_info->login_url=NULL;
			}
			//flickr_info("[len=%d]str=%s",login_url_len,find+10);
			tmp = (char*)flickr_malloc(login_url_len+32);
			if(tmp){
				memset(tmp,0,login_url_len);
				memcpy(tmp,find,login_url_len-3);
				auth_info->login_url = tmp;
				//flickr_info("find len=%d, location::=%s",login_url_len,auth_info->login_url);
			}
			else
				return -1;
		}
	}
#endif
	return 0;
}

size_t flickr_func_write_data( void *ptr, size_t size, size_t nmemb, void *userdata)
{
	flickr_data_write_t *data_write = (flickr_data_write_t*)userdata;
	unsigned int size_str=size*nmemb;
	unsigned int size_malloc=size_str+1;
	//printf("<receive size_str=%d>%s\n",size_str,ptr);	
	if(data_write->cancel_write){
		data_write->cancel_write = 0;
		flickr_info("Function Write Cancel!\n");
		return 0;
	}
	if(data_write->data_type==flickcurl_data_write_buffer){
		if(data_write->data_head==NULL){
			data_write->data_head=(char *)flickr_malloc(size_malloc);
			if(data_write->data_head==NULL){
				flickr_err("Malloc Failed\n");
				return 0;
			}
			data_write->data_cur = data_write->data_head;
			data_write->data_len = size_malloc;
			data_write->data_used = 0;
		}
		else{
			unsigned int newsize=data_write->data_used+size_malloc;
			if(newsize>=data_write->data_len){///need to realloc the buffer to store the data received
				char * buf=NULL;
				buf = (char*)flickr_realloc(data_write->data_head,newsize);
				if(buf==NULL){
					flickr_err("Realloc Failed\n");
					flickr_data_write_free(data_write);
					return 0;
				}
				else{
					data_write->data_head = buf;
					data_write->data_cur = data_write->data_head + data_write->data_used;
					data_write->data_len = newsize;
				}
			}
		}
		memcpy(data_write->data_cur,ptr,size_str);
		data_write->data_used +=size_str;
		data_write->data_cur = data_write->data_head + data_write->data_used;
		*(data_write->data_head+data_write->data_used)=0;//should end with \0	
		return size*nmemb;
	}
	else if(data_write->data_type==flickcurl_data_write_file){//write the data received to the file which had been opened
		void * fp = data_write->file_handle;
		unsigned int write_len = 0;
		if(fp==NULL){
			flickr_err("Error File Handle is NULL\n");
			return 0;
		}
		write_len =  flickr_fwrite(fp,ptr,size*nmemb);
		return write_len;
	}
	else{
		printf("Data Type Error!\n",data_write->data_type);
		return 0;
	}

}

size_t flickr_func_write_header( void *ptr, size_t size, size_t nmemb, void *userdata)
{
	size_t rev_data=size*nmemb;
	char * header_find=NULL;
	flickr_data_write_t *reloc_data = (flickr_data_write_t*)userdata;
	//flickr_info("<Head size=%d>%s",rev_data,(char*)ptr);
	header_find = strstr(ptr,"https://www.facebook.com/login.php?");
	if(header_find){
		reloc_data = flickr_str_append(reloc_data,header_find);
		return 0;
	}
	return rev_data;
}

int flickr_func_prog(void *p, double dltotal, double dlnow, double ult, double uln)
{
	flickr_download_info_t *down_info = (flickr_download_info_t*)p;
	if(dltotal!=0)
		down_info->prog_down= (int)(dlnow*100/dltotal);
	else 
		down_info->prog_down= 0;
	flickr_info("Progress: %d",down_info->prog_down);
	return 0;
}

flickr_data_write_t * flickr_str_append(flickr_data_write_t * xml_list,char* str_add)
{
	int len_str=0;
	if(str_add){
		len_str = strlen(str_add);
		if(len_str>0){
			flickr_func_write_data((void*)str_add,(size_t)1,len_str,(void*)xml_list);
		}
	}
	return xml_list;
}

flickr_data_write_t * flickr_strn_append(flickr_data_write_t * xml_list,char* str_add,int size)
{
	int len_str=0;
	if(str_add){
		len_str = size;
		if(len_str>0){
			flickr_func_write_data((void*)str_add,(size_t)1,len_str,(void*)xml_list);
		}
	}
	return xml_list;
}


char * flickr_extract_str(const char* str_start,const char* str_end,char* str_src,char**nextpos)
{
	char *str_start_pos=NULL;
	char *str_end_pos=NULL;
	int str_start_offset=0;
	char *value=NULL;
	int str_len=0;
	*nextpos=NULL;
	str_start_pos = strstr(str_src,str_start);
	//flickr_info("<str_start=%s> {str_end=%s}",str_start,str_end);
	if(str_start_pos){
		str_start_offset = strlen(str_start);
		str_end_pos =strstr(str_start_pos,str_end);
		if(str_end_pos){
			*nextpos = str_end_pos;
			str_len = str_end_pos-str_start_pos-str_start_offset;
			if(str_len){
				value = (char*)flickr_malloc(str_len+1);
				if(value){
					memset(value,0,str_len+1);
					memcpy(value,str_start_pos+str_start_offset,str_len);
					//flickr_info("<len=%d>value=%s",str_len,value);
				}
				
			}
		}
	}
	return value;
}


char * flickr_get_URL_encoded(char* src,unsigned char c_convert)
{
	int len_str=strlen(src);
	int i=0,j=0;
	char *des_str=NULL;
	char *str_converted=NULL;
	unsigned char tmp=0;
	des_str = (char*)flickr_malloc(len_str*3);
	//flickr_info("<Conver=%c,0x%x,len=%d>",c_convert,c_convert,len_str);
	if(des_str){
		memset(des_str,0,len_str*3);
		for(i=0;i<len_str;i++){
			if(src[i]!=c_convert)
				des_str[j]=src[i];
			else{
				des_str[j++]='%';
				des_str[j++]=((c_convert&0xf0)>>4)+0x30;
				tmp = c_convert&0x0f;
				des_str[j]=(tmp<=0x09)?(tmp+0x30):(tmp-0x0A+0x41);
			}
			j++;
		}
	}
	if(j!=0){
		str_converted = (char*)flickr_malloc(j+1);
		if(str_converted){
			memset(str_converted,0,j+1);
			memcpy(str_converted,des_str,j);
			flickr_free(des_str);
		}
	}
	if(str_converted!=NULL)
		return str_converted;
	else
		return des_str;
}

void flickr_free_URL_encoded(char* url_encoded)
{
	if(url_encoded)
		flickr_free(url_encoded);
}


char * flickr_get_URLEncode_str(char * src,char* conv)
{
	char * tmp_str=NULL;
	char * next_tmp_str=NULL;
	unsigned char ch=0;
	int i=0;
	tmp_str = src;
	while((ch=(unsigned char)conv[i])!=0){
		next_tmp_str = flickr_get_URL_encoded(tmp_str,ch);
		//flickr_info("<tmpstr>%s",next_tmp_str);
		if(tmp_str!=src){
			flickr_free(tmp_str);
			tmp_str = NULL;
		}
		if(next_tmp_str!=NULL){
			tmp_str = next_tmp_str;
		}
		else{
			flickr_err("ULEEnocde Erro!");
			break;
		}
		i++;
	}
	return tmp_str;
}

/**
@brief get the flickr login url
@param[in] fc	: which the api_key and the secret is included
@param[in] perms : write , read or delete
@param[in] frob : get from the flickculr function flickcurl_auth_getFrob()
@return: the url of the flickr login web
**/
static char* flickr_get_yahoo_webpage_url(flickcurl *fc,const char*perms,const char*frob)
{
	char *buf=NULL;
	const char *api_key=NULL;
	const char *secret=NULL;
	char *md5_string = NULL;
	buf  = (char*)flickr_malloc(512);
	api_key = flickcurl_get_api_key(fc);
	secret = flickcurl_get_shared_secret(fc);
	if(buf){
		strcpy(buf, secret);
		strcat(buf,"api_key");
		strcat(buf,api_key);
		strcat(buf,"frob");
		strcat(buf,frob);
		strcat(buf,"perms");
		strcat(buf,perms);
		md5_string = (char*)MD5_string(buf);
		if(md5_string){
			memset(buf,0,512);
			strcpy(buf,yahoo_service_auth);
			strcat(buf,"api_key=");
			strcat(buf,api_key);
			strcat(buf,"&");
			strcat(buf,"perms=");
			strcat(buf,perms);
			strcat(buf,"&");
			strcat(buf,"frob=");
			strcat(buf,frob);
			strcat(buf,"&");
			strcat(buf,"api_sig=");
			strcat(buf,md5_string);
		}
	}
	else
		flickr_err("Malloc Failed!");
	if(md5_string)
		free(md5_string);
	return buf;
}


/**
@brief when the account of google or facebook is chosen, get the link 

**/
static char * flickr_get_login_link(flickr_gdata_t *gdata,char* yahoo_webpage)
{
	char *nextpos=NULL;
	char *value=NULL;
	if(gdata->account_type==ACCOUNT_TYPE_GOOGLE){
		value = flickr_extract_str("var googSigninLnk = \"","\";",yahoo_webpage,&nextpos);
	}
	else if(gdata->account_type==ACCOUNT_TYPE_FACEBOOK){
		value = flickr_extract_str("var fbSigninLnk = \"","\";",yahoo_webpage,&nextpos);
	}
	else{
	}
	return value;
}


int flickr_auth_login(CURL* curl_handle,flickr_gdata_t *gdata,char* link_url)
{
	int rtn=FLICKR_RTN_OK;
	
	if(gdata->account_type==ACCOUNT_TYPE_GOOGLE){
		rtn = flickr_connect_google(curl_handle,gdata,link_url);
	}
	else if(gdata->account_type==ACCOUNT_TYPE_FACEBOOK){
		rtn = flickr_connect_facebook(curl_handle,gdata,link_url);
	}
	else{
		flickr_err("Account Type Error!");
		rtn = FLICKR_RTN_ACCOUTNTYPE_ERR;
	}
	flickr_info("rtn===%d",rtn);
	return rtn;
}


#endif

/**
@brief : connet to the flickr login web, choose the login type
@param[in] gdata: global data which the information is included
@return 
	see flickr_rtn_e
**/
int flickr_connect_yahoo_webpage(flickr_gdata_t *gdata)	
{
	char *yahoo_url=NULL;
	char *login_link=NULL;
	CURLcode curlcode=0;
	long m_http_code=0;
	int status=0;
	CURL* curl_handle =NULL;
	int rtn=FLICKR_RTN_OK;
	flickr_data_write_t receive_data;
	struct curl_slist *flickr_list_head=NULL;
	flickr_yahoo_login_para_t * yahoo_paras=NULL;
	if(gdata==NULL || gdata->perms.data==NULL||gdata->frob.data==NULL)
		goto YAHOO_WEBPAGE_END;
	
	yahoo_url = flickr_get_yahoo_webpage_url(gdata->fc,gdata->perms.data,gdata->frob.data);
	if(yahoo_url==NULL){
		flickr_err("Get Yahoo Webpage Error!");
		goto YAHOO_WEBPAGE_END;
	}
	
	curl_handle = curl_easy_init();//fc->curl_handle;
	if(curl_handle==NULL){
		flickr_err("Init Curl Error!");
		flickr_free(yahoo_url);
		goto YAHOO_WEBPAGE_END;
	}
	
	flickr_data_write_init(&receive_data,flickcurl_data_write_buffer,NULL);	
	curl_easy_setopt(curl_handle,CURLOPT_SSL_VERIFYPEER,0);
	//curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl_handle, CURLOPT_COOKIEJAR,flickr_cookie_addr);
	
	printf("%s,%d:Auth Url=%s\n",__FILE__,__LINE__,yahoo_url);
	curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(curl_handle, CURLOPT_URL,yahoo_url);


	receive_data.cancel_write = gdata->is_auth_cancel; ///< check whether the auth should be cancelled
	curl_easy_setopt(curl_handle,CURLOPT_WRITEDATA,&receive_data);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,flickr_func_write_data);

	flickr_list_head = curl_slist_append(flickr_list_head,"Cookie: BX=5lj4lkp6ress8&b=3&s=io");
	//flickr_list_head = curl_slist_append(flickr_list_head,"Accept-Language: en_US");
	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER,flickr_list_head);
	
	curlcode = curl_easy_perform(curl_handle);

	_test_save_to_file(receive_data.data_head,receive_data.data_used,"/mnt/udisk/yahoo_login.html");

	if(receive_data.data_used!=0){
		if(gdata->account_type==ACCOUNT_TYPE_YAHOO)
			yahoo_paras = flickr_extract_yahoo_paras(gdata,receive_data.data_head);
		else
			login_link = flickr_get_login_link(gdata,receive_data.data_head);
	}
	flickr_data_write_free(&receive_data);	

	if(gdata->account_type!=ACCOUNT_TYPE_YAHOO){
		if(login_link){
			rtn = flickr_auth_login(curl_handle,gdata,login_link);
			flickr_free(login_link);
		}
		else
			rtn = FLICKR_RTN_AUTH_ERR;
	}
	else{
		if(yahoo_paras){
			rtn = flickr_connect_yahoo(curl_handle,gdata,yahoo_paras);
			flickr_free_yahoo_paras(yahoo_paras);
		}
		else
			rtn = FLICKR_RTN_AUTH_ERR;
	}

	if(flickr_list_head)
		curl_slist_free_all(flickr_list_head);
	
	flickr_info("rtn===%d",rtn);
	flickr_free(yahoo_url);

YAHOO_WEBPAGE_END:
	status = flickr_get_status(rtn,curlcode,m_http_code);
	curl_easy_cleanup(curl_handle) ;
	return rtn;
}


/**
@brief : authetic the flickr
@param[in] gdata : the global data which is gotten from flickr_init_gdata();
@return 
	see flickr_rtn_e
**/
int flickr_desktop_auth(flickr_gdata_t *gdata)
{
	return flickr_connect_yahoo_webpage(gdata);
}


flickr_data_write_t * flickr_get_extrainfo_para(unsigned int opts)
{
	int i=0;
	flickr_data_write_t * parafield=(flickr_data_write_t *)flickr_malloc(sizeof(flickr_data_write_t));
	if(parafield){
		flickr_data_write_init(parafield,flickcurl_data_write_buffer,NULL);
		for(i=0;i<FLICKR_EXTRA_MAX_PARA;i++){
			if(opts&(1<<i)){
				parafield = flickr_str_append(parafield,flickr_photoinfo_extra_str[i]);
			}
		}
		parafield->data_head[parafield->data_used-1]=0; //should delete the last comma
	}
	return parafield;
}

void flickr_free_extrainfo_para(flickr_data_write_t * parafield)
{
	if(parafield){
		flickr_data_write_free(parafield);
		flickr_free((char*)parafield);
	}
}

char * flickr_get_contact_fliter(flickr_contact_filter_e filter)
{	
	if(filter>=FLICKR_CONTACT_FRIENDS || filter <= FLICKR_CONTACT_NEITHER)
		return flickr_contact_filter_str[filter];
	else
		return NULL;
}

char * flickr_photo_url_append(char* farm_id,char * server_id,char* id,char *secret,flickr_photosize_e size)
{
	memset(photo_url_str,0,FLICKR_TMP_BUF_LEN);
	//sprintf("http://farm%s.static.flickr.com/%s/%s_%s",farm_id,server_id,id,secret);
	if(farm_id==NULL || server_id==NULL ||id==NULL || secret==NULL)
		return NULL;
	strcat(photo_url_str,"http://farm");
	strcat(photo_url_str,farm_id);
	strcat(photo_url_str,".static.flickr.com/");
	strcat(photo_url_str,server_id);
	strcat(photo_url_str,"/");
	strcat(photo_url_str,id);
	strcat(photo_url_str,"_");
	strcat(photo_url_str,secret);
	switch(size){
		case FLCKR_PHOTOSIZE_NONE:
			strcat(photo_url_str,".jpg");
			break;
		case FLICKR_PHOTOSIZE_S: 			///< little square 75*75
			strcat(photo_url_str,"_s.jpg");
			break;
		case FLICKR_PHOTOSIZE_T:			///< thumbnail max side 100
			strcat(photo_url_str,"_t.jpg");
			break;
		case FLICKR_PHOTOSIZE_M:			///< little pic max side 240
			strcat(photo_url_str,"_m.jpg");
			break;
		case FLICKR_PHOTOSIZE_Z:			///< max side 640
			strcat(photo_url_str,"_z.jpg");
			break;
		case FLICKR_PHOTOSIZE_B:			///< big size max 1024
			strcat(photo_url_str,"_b.jpg");
			break;
		case FLICKR_PHOTOSIZE_O:			///< original size 
			strcat(photo_url_str,".jpg");
			break;
		default:
			flickr_err("Not support URL size yet!");
			break;
	}
	flickr_info("<Photo Url>%s",photo_url_str);
	return photo_url_str;
}


/**
@brief get the full path of the photo in the structure flickr_path_t
@param[in] path 	: the picasa_path_t which include the path
@param[in] fullpathbuf	: where to store the path
@return 
	- 0 failed
	- others : the length of the buffer had been used
***/
int flickr_get_file_path(flickr_path_t *path,char* fullpathbuf)
{
	int buf_used_len=0;
	if(path){
		if(path->cache_dir.data[path->cache_dir.data_len-2]=='/'){
			if(path->album_id.data)
				sprintf(fullpathbuf,"%s%s",path->cache_dir.data,path->album_id.data);
			else
				sprintf(fullpathbuf,"%s%s",path->cache_dir.data,path->photo_name.data);//it is feed_albums and not thumbnial
		}
		else{
			if(path->album_id.data)
				sprintf(fullpathbuf,"%s/%s",path->cache_dir.data,path->album_id.data);
			else
				sprintf(fullpathbuf,"%s/s%s",path->cache_dir.data,path->photo_name.data);//it is feed_albums and not thumbnial
		}

		if(path->album_id.data){
			if(path->album_id.data[path->album_id.data_len-2]=='/')
				sprintf(fullpathbuf,"%s%s",fullpathbuf,path->photo_name.data);
			else
				sprintf(fullpathbuf,"%s/%s",fullpathbuf,path->photo_name.data);
		}
		flickr_info("File Path=%s\n",fullpathbuf);
		buf_used_len = strlen(fullpathbuf);
	}
	if(buf_used_len==0){
		flickr_err("Get File Path Error Something may be wrong!");
	}
	return buf_used_len;
}

/**
@brief check whether the file is exist, remember to call flickr_close_file() to release the resource that this function occupied
@param[in] path : the pointer to the picasa_path_t where the path of the file stored
@param[out] status : where the status returned, 0: the if is not exist, and had been created, 
				FLICKR_RTN_DOWNLOAD_FILE_EXIST: the file is exist,
				FLICKR_RTN_DOWNLOAD_FILE_CREATE_FAIL:the file is not exist, created failed,
				FLICKR_RTN_DOWNLOAD_FILEBUF_ERR: the buffer for the fullpath is short
@return
 	- NULL: the file is exist or create failed
	- others: the handle of the file which had been created  
@see flickr_close_file()
**/
void * flickr_open_file(flickr_path_t *path,int *err_status)
{
	char tmpbuf[FLICKR_TMP_BUF_LEN]="";
	void * fhandle=NULL;
	if(flickr_get_file_path(path,tmpbuf)==0){
		*err_status = FLICKR_RTN_DOWNLOAD_FILEBUF_ERR;
		return NULL;
	}
	fhandle = flickr_fopen(tmpbuf,"rb");
	if(fhandle){//the file is exit,do not need download
		*err_status = FLICKR_RTN_DOWNLOAD_FILE_EXIST;
		flickr_fclose(fhandle);
		return NULL;
	}
	else{
		fhandle = flickr_fopen(tmpbuf,"wb+");
		if(fhandle==NULL){
			*err_status = FLICKR_RTN_DOWNLOAD_FILE_CREATE_FAIL;
			return NULL;
		}
	}
	*err_status = FLICKR_RTN_OK;
	return fhandle;
	
}

void flickr_close_file(void * fp)
{
	if(fp){
		flickr_fclose(fp);
	}
}

/**
@brief check the cache_dir, if the dir is not exist, try to create
@param[in] dir_path : the path of the dir
@return 
	0 : succeed
	others : failed
**/
int __flickr_create_dir(char * dir_path)
{
	int rtn=0;
	rtn = access(dir_path,W_OK);
	if(rtn!=0){
		rtn = mkdir(dir_path,0777);
		if(rtn!=0){
			flickr_err("Make Dir Failed=%s",dir_path);
		}
	}
	return rtn;	
}


/**
@biref checking the path ,if it is not exit, make it
**/
int flickr_check_path(flickr_path_t *path,int isthumbnail)
{	
	int rtn=FLICKR_RTN_OK;
	char tmpbuf[FLICKR_TMP_BUF_LEN]="";
	char tmpalbumid[64]="";	
	char *tmpstr=NULL;
	if(path==NULL){
		rtn =  FLICKR_RTN_PATH_CACHEDIR_ERR;
		goto FLICKR_PATH_CHECK_END;
	}
	rtn = __flickr_create_dir(path->cache_dir.data);
	if(rtn!=0){
		rtn =  FLICKR_RTN_PATH_CACHEDIR_ERR;
		goto FLICKR_PATH_CHECK_END;
	}
	if(isthumbnail){//should check the album_id first
		tmpstr = strstr(path->album_id.data,"/");
		if(tmpstr){
			memcpy(tmpalbumid,path->album_id.data,tmpstr-path->album_id.data+1);
		}
		else{
			if(strcmp(path->album_id.data,"thumbnail")!=0){// is thumbnail
				rtn =  FLICKR_RTN_PATH_ALBUMID_ERR;
				goto FLICKR_PATH_CHECK_END;
			}
		}
		sprintf(tmpbuf,"%s%s",path->cache_dir.data,tmpalbumid);
		flickr_info("PATH THUMBNAIL=%s\n",tmpbuf);
		rtn = __flickr_create_dir(tmpbuf);
		if(rtn!=0){
			rtn =  FLICKR_RTN_PATH_ALBUMID_ERR;
			goto FLICKR_PATH_CHECK_END;
		}
	
	}
	if(path->album_id.data){
		if(path->cache_dir.data[path->cache_dir.data_len-2]=='/'){
			sprintf(tmpbuf,"%s%s",path->cache_dir.data,path->album_id.data);
		}
		else
			sprintf(tmpbuf,"%s/%s",path->cache_dir.data,path->album_id.data);
		flickr_info("PATH=====%s\n",tmpbuf);
		rtn = __flickr_create_dir(tmpbuf);
	}
	if(rtn!=0){
		rtn =  FLICKR_RTN_PATH_ALBUMID_ERR;
		goto FLICKR_PATH_CHECK_END;
	}	

FLICKR_PATH_CHECK_END:
	flickr_info("%s,%d:Rtn=%d\n",__FILE__,__LINE__,rtn);
	return rtn;
}



void flickr_free_cache_path(flickr_path_t *img_path)
{
	if(img_path){
		flickr_free_atomic(&img_path->cache_dir);
		flickr_free_atomic(&img_path->album_id);
		flickr_free_atomic(&img_path->photo_name);
	}
}


#if 1
void flickr_print_notes(flickcurl_note** notes, const char* label,
                    const char* value)
{
#ifdef FLICKR_DEBUG_EN
  int i;

  if(!notes)
    return;
  
  if(label)
    fprintf(stderr, "%s: %s %s notes\n", __FILE__, label,
            (value ? value : "(none)"));
  else
    fprintf(stderr, "notes:\n");

  for(i = 0; notes[i]; i++) {
    flickcurl_note* note = notes[i];
    fprintf(stderr,
            "%d) id %d note: author ID %s name %s  x %d y %d w %d h %d text '%s'\n",
            i, note->id,
            note->author, (note->authorname ? note->authorname : "(Unknown)"),
            note->x, note->y, note->w, note->h,
            note->text);
  }
#endif
}

void flickr_print_tags(flickcurl_tag** tags, const char* label, const char* value)
{
#ifdef FLICKR_DEBUG_EN
  int i;
  if(!tags)
    return;
  
  if(label)
    fprintf(stderr, "%s: %s %s tags\n", __FILE__, label,
            (value ? value : "(none)"));
  else
    fprintf(stderr, "tags:\n");
  
  for(i = 0; tags[i]; i++) {
    flickcurl_tag* tag = tags[i];
    fprintf(stderr,
            "%d) %s tag: id %s author ID %s name %s raw '%s' cooked '%s' count %d\n",
            i, (tag->machine_tag ? "machine" : "regular"),
            tag->id, tag->author,
            (tag->authorname ? tag->authorname : "(Unknown)"),
            tag->raw, tag->cooked, tag->count);
  }
#endif
}


void flickr_print_shape(flickcurl_shapedata* shape)
{
#ifdef FLICKR_DEBUG_EN
  fprintf(stderr,
          "created %d  alpha %2.2f  #points %d  #edges %d\n"
          "  is donuthole: %d  has donuthole: %d\n",
          shape->created, shape->alpha, shape->points, shape->edges,
          shape->is_donuthole, shape->has_donuthole);

  if(shape->data_length > 0) {
    int s;
#define MAX_XML 70
    s = (shape->data_length > MAX_XML ? MAX_XML : shape->data_length);
    fprintf(stderr, "  Shapedata (%d bytes):\n    ",
            (int)shape->data_length);
    fwrite(shape->data, 1, s, stderr);
    fputs("...\n", stderr);
  }

  if(shape->file_urls_count > 0) {
    int j;
    fprintf(stderr, "  Shapefile URLs: %d\n", shape->file_urls_count);
    for(j = 0; j < shape->file_urls_count; j++) {
      fprintf(stderr,"    URL %d: %s\n", j, shape->file_urls[j]);
    }
  }
#endif
}
int flickr_print_location(flickcurl_location* location)
{
#ifdef FLICKR_DEBUG_EN
  const char* accuracy_label;
  accuracy_label = flickcurl_get_location_accuracy_label(location->accuracy);
  
  if(accuracy_label)
    fprintf(stderr, "latitude %f  longitude %f  accuracy %s(%d)\n",
            location->latitude, location->longitude, 
            accuracy_label, location->accuracy);
  else
    fprintf(stderr, "latitude %f  longitude %f  accuracy unknown\n",
            location->latitude, location->longitude);
#endif
  return 0;
}

void flickr_print_place(flickcurl_place* place,
                    const char* label, const char* value,
                    int print_locality)
{
#ifdef FLICKR_DEBUG_EN
  int i;
  if(label)
    fprintf(stderr, "%s: %s %s places\n", __FILE__, label,
            (value ? value : "(none)"));

  if(print_locality && place->type != FLICKCURL_PLACE_LOCATION)
    fprintf(stderr, "  Type %s (%d)\n",
            flickcurl_get_place_type_label(place->type), (int)place->type);
  
  if(place->location.accuracy != 0) {
    fputs("  Location: ", stderr);
    flickr_print_location(&place->location);
  }
  
  if(place->timezone)
    fprintf(stderr, "  Timezone: %s\n", place->timezone);

  if(place->shape)
    flickr_print_shape(place->shape);
  
  if(place->count >0)
    fprintf(stderr, "  Photos at Place: %d\n", place->count);
  
  for(i = (int)0; i <= (int)FLICKCURL_PLACE_LAST; i++) {
    char* name = place->names[i];
    char* id = place->ids[i];
    char* url = place->urls[i];
    char* woe_id = place->woe_ids[i];
    
    if(!name && !id && !url && !woe_id)
      continue;
    
    fprintf(stderr, "  %d) place %s:", i,
            flickcurl_get_place_type_label((flickcurl_place_type)i));
    if(name)
      fprintf(stderr," name '%s'", name);
    if(id)
      fprintf(stderr," id %s", id);
    if(woe_id)
      fprintf(stderr," woeid %s", woe_id);
    if(url)
      fprintf(stderr," url '%s'", url);
    fputc('\n', stderr);
  }
#endif
}

void flickr_print_video(flickcurl_video* v)
{
#ifdef FLICKR_DEBUG_EN
  fprintf(stderr,
          "video: ready %d  failed %d  pending %d  duration %d  width %d  height %d\n",
          v->ready, v->failed, v->pending, v->duration,
          v->width, v->height);
#endif
}

void flickr_print_photo(flickcurl_photo* photo)
{
#ifdef FLICKR_DEBUG_EN
	int i;
	fprintf(stderr, "%s with URI %s ID %s and %d tags\n",
	      photo->media_type, 
	      (photo->uri ? photo->uri : "(Unknown)"),
	      photo->id, photo->tags_count);
	for(i = 0; i <= PHOTO_FIELD_LAST; i++) {
		flickcurl_photo_field_type field = (flickcurl_photo_field_type)i;
		flickcurl_field_value_type datatype = photo->fields[field].type;
		if(datatype == VALUE_TYPE_NONE)
		  	continue;
		fprintf(stderr, "    field %s (%d) with %s value: '%s' / %d\n", 
		        flickcurl_get_photo_field_label(field), field,
		        flickcurl_get_field_value_type_label(datatype),
		        photo->fields[field].string, photo->fields[field].integer);
	}
	flickr_photo_url_append(photo->fields[PHOTO_FIELD_farm].string,photo->fields[PHOTO_FIELD_server].string,
		photo->id,photo->fields[PHOTO_FIELD_secret].string,FLICKR_PHOTOSIZE_M);
	flickr_print_tags(photo->tags, NULL, NULL);
	if(photo->notes)
		flickr_print_notes(photo->notes, NULL, NULL);
	if(photo->place)
		flickr_print_place(photo->place, NULL, NULL, 1);
	if(photo->video)
		flickr_print_video(photo->video);
#endif
}

void flickr_print_photos(flickcurl_photos_list* photos_list)
{
#ifdef FLICKR_DEBUG_EN
	int i;
	flickr_info("<Photo total count=%d,photos_acount=%d>",photos_list->total_count,photos_list->photos_count);
	if(photos_list->photos){
		 for(i = 0; photos_list->photos[i]; i++) {
			flickr_info("photo<%d>",i);
			flickr_print_photo(photos_list->photos[i]);
		}
	}else if(photos_list->content) {
   		flickr_info("<Photos Content len=%d>%s",photos_list->content_length,photos_list->content);
  	} else {
   		flickr_info("returned neither photos nor raw content!");
 	}
#endif
}

void flickr_print_user_info(flickcurl_person * user_info)
{
#ifdef FLICKR_DEBUG_EN
	int field_type=0;
	flickr_info("nsid=%s",user_info->nsid);
	for(field_type=0; field_type <= PERSON_FIELD_LAST; field_type++) {
		flickcurl_field_value_type datatype=user_info->fields[field_type].type;
		if(datatype != VALUE_TYPE_NONE)
			fprintf(stderr, "field %s (%d) with %s value: '%s' / %d\n", 
			flickcurl_get_person_field_label(field_type), (int)field_type,
			flickcurl_get_field_value_type_label(datatype),
			user_info->fields[field_type].string,
			user_info->fields[field_type].integer);
	}
#endif
}

void flickr_print_gallery(flickcurl_gallery* g)
{
#ifdef FLICKR_DEBUG_EN
    fprintf(stderr,
            "id %s  url %s  owner %s\n"
            "  date create %d  date update %d\n"
            "  count of photos %d  count of videos %d\n"
            "  title '%s'\n"
            "  description '%s'\n"
            ,
            g->id, g->url, g->owner,
            g->date_create, g->date_update,
            g->count_photos, g->count_videos,
            g->title, g->description);
    fputs("  primary ", stderr);
    flickr_print_photo(g->primary_photo);
#endif
}

void flickr_print_photoset(flickcurl_photoset* photoset)
{
#ifdef FLICKR_DEBUG_EN
	int i;
  fprintf(stderr, 
          "%s: Found photoset with ID %s primary photo: '%s' secret: %s server: %d farm: %d photos count: %d title: '%s' description: '%s'\n",
          __FILE__,
          photoset->id, photoset->primary, photoset->secret,
          photoset->server, photoset->farm,
          photoset->photos_count,
          photoset->title, 
          (photoset->description ? photoset->description : "(No description)"));

  flickr_info("<photo url>http://farm%d.static.flickr.com/%d/%s_%s.jpg",photoset->farm,\
		photoset->server,photoset->primary,photoset->secret);
  	if(photoset->photos){
		flickr_info("The Photos are the following...");
	  	for(i=0;i<photoset->photos_count;i++){
			flickr_print_photo(photoset->photos[i]);
	  	}
		flickr_info("**************************");
  	}
#endif
}

void flickr_print_contact(flickcurl_contact* contact, int i)
{
#ifdef FLICKR_DEBUG_EN
  fprintf(stderr,
          "contact %d: NSID %s username %s iconserver %d realname %s friend %d family %d ignored %d upload count %d\n",
          i,
          contact->nsid, contact->username,
          contact->iconserver, contact->realname,
          contact->is_friend, contact->is_family,
          contact->ignored, contact->uploaded);
#endif
}

#endif

