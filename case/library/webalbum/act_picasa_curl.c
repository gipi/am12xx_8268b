#include "act_picasa_curl.h"
#include "act_picasa_common.h"
#include "act_picasa_ioctl.h"
#include <sys/vfs.h>

#include "am_types.h"
#define DATA_POST_BUF_LEN 512
#define LOGIN_DATA_LEN 128


extern picasaweb_schema_item_t schema_items[];

#define GET_XPATH(x) (schema_items[(x)].atom_pres)
static char picasaweb_login_url[LOGIN_DATA_LEN]="https://www.google.com/accounts/ClientLogin";
static char picasaweb_user_url[LOGIN_DATA_LEN]="https://picasaweb.google.com/data/feed/api/user/default";

//static char * data_post_buf=NULL;

static char picasaweb_login_email[LOGIN_DATA_LEN]="";
static char picasaweb_login_pwd[LOGIN_DATA_LEN]="";
extern char picasaweb_gdata_version[32];
extern char picasaweb_gdata_source[32];

extern picasa_ioctrl_t picasa_cmd_array[PICASA_IOARRAY_LEN];

static unsigned long picasa_timestamp=1;

///////functions definition

static xmlDocPtr picasa_get_xmlDoc(picasaweb_gdata_t *gdata,char* url,int isheaderneed);
static int picasa_free_xmlDoc(xmlDocPtr xmlDoc);

///////////////////////////
char * picasa_malloc(unsigned int size)
{
	char *buf=(char*)malloc(size);
	if(buf==NULL)
		picasa_err("%s,%d:Malloc Failed!\n");
	return buf;
}

char * picasa_realloc(char *ptr,unsigned int newsize)
{
	char *buf=(char*)realloc(ptr,newsize);
	if(buf==NULL){
		picasa_err("%s,%d:Realloc Failed!\n");
	}
	return buf;
}

char picasa_free(char* buffer)
{
	///picasa_info("Buffer=0x%X\n",(unsigned int)buffer);
	if(buffer!=NULL)
		free(buffer);
	return 0;
}

char * picasa_get_emailaddr()
{
	return picasaweb_login_email;
}

int picasa_set_emailaddr(const char* email)
{
	int str_len = strlen(email);
	if(str_len>=LOGIN_DATA_LEN){
		picasa_info("Error Email Lenth is to long, len=%d,MAX_LEN=%d\n",str_len,LOGIN_DATA_LEN);
		return -1;
	}
	strcpy(picasaweb_login_email,email);
	return 0;
	
}

char * picasa_get_pwd()
{
	return picasaweb_login_pwd;
}

int picasa_set_pwd(const char* password)
{
	int str_len = strlen(password);
	if(str_len>=LOGIN_DATA_LEN){
		picasa_info("Error PWD Lenth is to long, len=%d,MAX_LEN=%d\n",str_len,LOGIN_DATA_LEN);
		return -1;
	}
	strcpy(picasaweb_login_pwd,password);
	return 0;
}


static int __get_auth_errorcode(char* str)
{
	if(strstr(str,"BadAuthentication")!=NULL){
		return Picasaweb_Auth_BadAuthentication;
	}
	if(strstr(str,"CaptchaRequired")!=NULL){
		return Picasaweb_Auth_CaptchaRequired;
	}
	if(strstr(str,"NotVerified")!=NULL){
		return Picasaweb_Auth_NotVerified;
	}
	if(strstr(str,"TermsNotAgreed")!=NULL){
		return Picasaweb_Auth_TermsNotAgreed;
	}
	if(strstr(str,"AccountDeleted")!=NULL){
		return Picasaweb_Auth_AccountDeleted;
	}
	if(strstr(str,"AccountDisabled")!=NULL){
		return Picasaweb_Auth_AccountDisabled;
	}
	if(strstr(str,"ServiceDisabled")!=NULL){
		return Picasaweb_Auth_ServiceDisabled;
	}
	if(strstr(str,"ServiceUnavailable")!=NULL){
		return Picasaweb_Auth_ServiceUnavailable;
	}
	if(strstr(str,"Unknown")!=NULL){
		return PicasaWeb_Auth_Unknown;
	}
	return -1;
}

/**
@brief the callback function of retriving the data of the authentication
***/
static size_t pacasa_func_get_auth( void *ptr, size_t size, size_t nmemb, void *userdata)
{
	char *str_auth=NULL;
	int len_auth=0;
	picasaweb_gdata_t *gdata=(picasaweb_gdata_t *)userdata;

	picasa_info("size=%d,nmemb=%d\n",size,nmemb);
	picasa_info("ptr=%s\n",(char*)ptr);

	str_auth=strstr(ptr,"Auth=");	
	if(str_auth){
		len_auth=strlen(str_auth);
		gdata->auth=picasa_malloc(len_auth);
		if(gdata->auth){
			gdata->auth_len = len_auth;
			memcpy(gdata->auth,str_auth,gdata->auth_len);
			gdata->auth[gdata->auth_len-1]=0;
		}
		else
			gdata->auth_len = 0;
		gdata->auth_ok=Picasaweb_OK;
		picasa_info("{%s}\n",gdata->auth);
	}
	else{	
		gdata->auth_ok=__get_auth_errorcode(ptr);
		gdata->auth = NULL;
		gdata->auth_len = 0;
		picasa_err("Error Authentication Failed, AuthCode=%d\n",gdata->auth_ok);
	}
	return size*nmemb;
}


/**
@brief the callback function for uploading photo
**/
static size_t picasa_func_read_data( void *ptr, size_t size, size_t nmemb, void *userdata)
{	
	long bytesread=0;
	void *fp = NULL;
	photo_upload_info_t *  upload_info = (photo_upload_info_t *)userdata;

	if(upload_info==NULL)
		return 0;
	
	fp = upload_info->photo_data_send.file_handle;
	if(upload_info->photo_data_send.cancel_write){
		picasa_info("Upload Canceled!");
		upload_info->photo_data_send.cancel_write = 0;
		return 0;
	}
	if(fp==NULL || size*nmemb<1){
		picasa_info("Read Data ERROR\n");
		bytesread = 0;
	}
	else{
		bytesread =  picasa_fread(fp,ptr,size*nmemb);
		upload_info->uploaded_bytes += bytesread;
	}
	//picasa_info("READ DATA BYTES File=%d,uploadedbytes=%d",(int)upload_info->filesize_bytes,(int)upload_info->uploaded_bytes);
	return bytesread;
}


/**
*@brief callback function for retriving data 
**/
static size_t picasa_func_write_data( void *ptr, size_t size, size_t nmemb, void *userdata)
{
	picasaweb_data_write_t *data_write = (picasaweb_data_write_t*)userdata;
	unsigned int size_str=size*nmemb;
	unsigned int size_malloc=size_str+1;
	//picasa_info("func size=%d,nmemb=%d\n",size,nmemb);
	//picasa_info("ptr=%s\n",ptr);	
	if(data_write->cancel_write){
		data_write->cancel_write = 0;
		picasa_info("%s,%d: Function Write Cancel!\n");
		return 0;
	}
	if(data_write->data_type==Picasaweb_data_write_buffer){
		if(data_write->data_head==NULL){
			data_write->data_head=(char *)picasa_malloc(size_malloc);
			if(data_write->data_head==NULL){
				picasa_err("Malloc Failed\n");
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
				buf = (char*)picasa_realloc(data_write->data_head,newsize);
				if(buf==NULL){
					picasa_err("Realloc Failed\n");
					__picasaweb_data_write_free(data_write);
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
	else if(data_write->data_type==Picasaweb_data_write_file){//write the data received to the file which had been opened
		void * fp = data_write->file_handle;
		unsigned int write_len = 0;
		if(fp==NULL){
			picasa_err("Error File Handle is NULL\n");
			return 0;
		}
		write_len =  picasa_fwrite(fp,ptr,size*nmemb);
		return write_len;
	}
	else{
		picasa_err("Data Type Error!\n",data_write->data_type);
		return 0;
	}

}

static int picasa_func_prog(void *p, double dltotal, double dlnow, double ult, double uln)
{
	photo_down_info_t *down_info = (photo_down_info_t*)p;
	if(dltotal!=0)
		down_info->prog_down= (int)(dlnow*100/dltotal);
	else 
		down_info->prog_down= 0;
	//picasa_info("Progress: %d\n",down_info->prog_down);
	return 0;
}




/**
@brief extracting an entry infomation to the structure of the picasaweb_entry_t
@param[in] entry	: where the info to be stored
@param[in] entrynode	: the resource of the infomation
@return
	- -1	: failed
	- 0	: Ok
**/
static int picasa_extract_entry_info(picasaweb_entry_t *entry,xmlNodePtr entrynode)
{
	xmlNodePtr childnode=NULL;
	int idx=0;
	if(entry==NULL)
		return -1;
	memset(entry,0,sizeof(picasaweb_entry_t));
	__picasa_fill_node_attr(entrynode,"etag",&entry->attr_etag);
	if(entrynode->children!=NULL){
		childnode = entrynode->children;
		while(childnode){
			picasa_fill_entry_element(entry,childnode);
			childnode = childnode->next;
		}
	}
	return 0;
	
}


/**
@brief extracting an feed infomation to the structure of the picasaweb_feed_t exclude the entries
@param[in] feed	: where the info to be stored
@param[in] feednode	: the resource of the infomation
@return
	- -1	: failed
	- 0	: Ok
**/
static int picasa_extract_feed_info(picasaweb_feed_t *feed,xmlNodePtr feednode)
{
	xmlNodePtr childnode=NULL;
	int idx=0;
	if(feed==NULL)
		return -1;
	__picasa_fill_node_attr(feednode,"etag",&feed->attr_etag);
	if(feednode->children!=NULL){
		childnode = feednode->children;
		while(childnode){
			picasa_fill_feed_element(feed,childnode);
			childnode = childnode->next;
		}
	}
	return 0;
}


static int picasa_free_feed_info(picasaweb_feed_t *feed)
{
	picasa_free_feed_element(feed);
	return 0;
}

/**
@brief extracting an feed infomation, remember to call picasa_free_feed() to release the resource
@param[in] feed_content	: the resource of the infomation
@return
	- NULL		: failed
	- others	: where the infomation be stored
@see picasa_free_feed()
**/
static picasaweb_feed_t * picasa_get_feed(xmlXPathContextPtr feed_content)
{
	picasaweb_feed_t *album_feed=NULL;
	int i=0;
	xmlChar *szXpath=NULL;
	xmlXPathObjectPtr result =NULL;
	xmlNodePtr curNode=NULL;
	xmlNodeSetPtr nodeset=NULL;
	album_feed  = (picasaweb_feed_t *)picasa_malloc(sizeof(picasaweb_feed_t));
	if(album_feed==NULL)
		goto EXTRACT_FEED_END;	
	memset(album_feed,0,sizeof(picasaweb_feed_t));
	

	if(picasa_extract_entrys_info(feed_content,album_feed)==-1){
		goto EXTRACT_FEED_ERROR;
	}

	///extract other information
	szXpath =(xmlChar*)GET_XPATH(FEED_ALL);
	result = xmlXPathEvalExpression(szXpath,feed_content);
	if(result){
		nodeset = result->nodesetval;
		if (xmlXPathNodeSetIsEmpty(result->nodesetval)){
			picasa_info("nodeset is empty\n");
			goto EXTRACT_FEED_ERROR;
		}
		picasa_info("NodeNr==%d\n",nodeset->nodeNr);
		for(i=0; i < nodeset->nodeNr; i++) {
			curNode = nodeset->nodeTab[i];
			if(curNode!=NULL){
				picasa_extract_feed_info(album_feed,curNode);
				__picasa_printf_feed(album_feed);
			}
		}
	}	

	goto EXTRACT_FEED_END;
	
EXTRACT_FEED_ERROR:
	if(album_feed){
		picasa_free_entrys_info(album_feed);
		picasa_free((char*)album_feed);
		album_feed = NULL;
	}
EXTRACT_FEED_END:
	if(result)
		xmlXPathFreeObject(result);
	return album_feed;

}

/**
@brief release the resource of the picasaweb feed which is get from picasa_get_feed()
@see picasa_get_feed()
**/
static int picasa_free_feed(picasaweb_feed_t *feed)
{
	if(feed!=NULL){
		picasa_free_entrys_info(feed);
		picasa_free_feed_info(feed);
		picasa_free((char*)feed);
		feed = NULL;
	}
	return 0;
}


/**
@brief call this function to extract the albums info in the albums content
@param[in] albums_content	: the content which the albums info are included
@param[in] album_feed	: where the info to be stored
@return 
	- 0	: succeed
	- -1	: failed
**/
int picasa_extract_entrys_info(xmlXPathContextPtr albums_content,picasaweb_feed_t *album_feed)
{
	int rtn=0;
	int bitmapbytes=0;
	unsigned char *bitmap=NULL;
	xmlXPathObjectPtr result=NULL;
	xmlChar *szXpath =(xmlChar*)GET_XPATH(ENTRY_ALL);
	picasa_info("szXPath=%s\n",(char*)szXpath);
	if(album_feed==NULL){
		rtn =0;
		goto EXTRACT_ALBUMS_END;
	}

	result = xmlXPathEvalExpression(szXpath,albums_content);
	if(result==NULL){
		picasa_info("PATH EVAL FAILE!\n");
	}
	else{
		xmlNodePtr curNode;
		int i = 0;
		xmlChar *szValue = NULL;
		xmlChar *attrContent=NULL;
		xmlNodeSetPtr nodeset = result->nodesetval;
		if (xmlXPathNodeSetIsEmpty(result->nodesetval)){
			picasa_info("nodeset is empty\n");
			rtn =0;
			goto EXTRACT_ALBUMS_END;
		}
		if(result->stringval){
			picasa_info("stringval=%s ",result->stringval);
		}
		picasa_info("nodeNr=%d\n",nodeset->nodeNr);
		album_feed->entry_num = nodeset->nodeNr;
		album_feed->entry = (picasaweb_entry_t*)picasa_malloc(sizeof(picasaweb_entry_t)*album_feed->entry_num);
		if(album_feed->entry==NULL){
			picasa_err("Error Malloc Entry Failed\n");
			rtn = -1;
			goto EXTRACT_ALBUMS_END;
		}
		
		bitmapbytes = (album_feed->entry_num+7)/8;
		bitmap = (unsigned char*)picasa_malloc(bitmapbytes*2);///malloc the space for bitmaps
		if(bitmap==NULL){
			picasa_err("Malloc BitMap Failed!\n");
			picasa_free((char*)album_feed->entry);
			rtn = -1;
			goto EXTRACT_ALBUMS_END;
		}
		memset(bitmap,0,bitmapbytes*2);
		album_feed->download_thumbnailbitmap = bitmap;
		album_feed->download_browsebitmap = bitmap+bitmapbytes;

		for(i=0; i < nodeset->nodeNr; i++) {
			curNode = nodeset->nodeTab[i];   /// it is an album entry 
			if(curNode!=NULL){
				picasa_info("@@@@@@@@ENTRY=%d@@@@@@@@@@\n",i);
				picasa_extract_entry_info(album_feed->entry+i,curNode);
				__picasa_printf_entry(album_feed->entry+i);
				//while(1);
			}
		}
	}
	
EXTRACT_ALBUMS_END:
	if(result)
		xmlXPathFreeObject(result);
	return rtn;
}


int picasa_free_entrys_info(picasaweb_feed_t *album_feed)
{
	int i=0;
	if(album_feed){
		if(album_feed->download_thumbnailbitmap){
			picasa_free((char*)album_feed->download_thumbnailbitmap);
			album_feed->download_thumbnailbitmap = NULL;
			album_feed->download_browsebitmap = NULL;
		}
		if(album_feed->entry){
			for(i=0;i<album_feed->entry_num;i++){
				picasa_free_entry_element(album_feed->entry+i);
			}
			picasa_free((char*)album_feed->entry);
		}
	}
	return 0;
}





/**
@brief call this function to retriving the xml from the server and parsing the memeory to getting the xml Document
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] url		: the url to connect
@param[in] isheaderneed	: whether the header is included, if the authentication is needed, this value should be 1
@return 
	- NULL		: failed to get the xml document
	- others	: the pointer to the xml document
@see call the picasa_free_xmlDoc() to free the space that this function got
**/
static xmlDocPtr picasa_get_xmlDoc(picasaweb_gdata_t *gdata,char* url,int isheaderneed)
{
	struct curl_slist *picasa_list_head=NULL;
	picasaweb_data_write_t data_write;
	xmlDocPtr doc_ptr=NULL;
	CURLcode curlcode=0;
	long m_http_code=0;
	static int index=0;
	char * tmp_str=NULL;
	char filename[64]="";
	
	
	__picasaweb_data_write_init(&data_write,Picasaweb_data_write_buffer,NULL);
	
	picasa_info("URL===%s\n",url);
	curl_easy_setopt(gdata->curl, CURLOPT_CUSTOMREQUEST,NULL);
	curl_easy_setopt(gdata->curl, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(gdata->curl, CURLOPT_URL,url);

	curl_easy_setopt(gdata->curl,CURLOPT_WRITEDATA,&data_write);
	curl_easy_setopt(gdata->curl,CURLOPT_WRITEFUNCTION,picasa_func_write_data);

	if(isheaderneed){
		tmp_str=(char*)picasa_malloc(gdata->auth_len+32);
		if(tmp_str==NULL){
			picasa_err("Malloc Failed\n");
			return doc_ptr;
		}
		else{
			memset(tmp_str,0,gdata->auth_len+32);
			sprintf(tmp_str,"Authorization: GoogleLogin %s",gdata->auth);
			picasa_info("tmp_str=%s\n",tmp_str);
		}
		picasa_list_head = curl_slist_append(picasa_list_head,"Content-Type: application/atom+xml");
		picasa_list_head = curl_slist_append(picasa_list_head,picasaweb_gdata_version);	
		picasa_list_head = curl_slist_append(picasa_list_head,tmp_str);	
		//picasa_list_head = curl_slist_append(picasa_list_head,gdata->auth);
		curl_easy_setopt(gdata->curl, CURLOPT_HTTPHEADER,picasa_list_head);


	}
	curlcode = curl_easy_perform(gdata->curl);
	curl_easy_getinfo(gdata->curl, CURLINFO_RESPONSE_CODE, &m_http_code);

	picasa_info("curlcode=%d,http_code=%d\n",(int)curlcode,(int)m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		//sprintf(filename,"XXX%x.xml",(unsigned int )gdata);
		//_test_save_to_file(data_write.data_head,data_write.data_used,filename);
		doc_ptr = xmlParseMemory(data_write.data_head,data_write.data_used);
	}
	else
		doc_ptr = NULL;
	
	////free the resource 	
	if(isheaderneed){
		if(tmp_str)
			picasa_free(tmp_str);
		if(picasa_list_head)
			curl_slist_free_all(picasa_list_head);	
	}	

	__picasaweb_data_write_free(&data_write);

	return doc_ptr;
}


/**
@brief call this function to free the space which is occupied when the picasa_get_xmlDoc() is called
@param[in] xmlDoc	: the pointer to the xml document which is gotten when the picasa_get_xmlDoc is called
@return 
	always 0
@see picasa_get_xmlDoc()
**/
static int picasa_free_xmlDoc(xmlDocPtr xmlDoc)
{
	if(xmlDoc)
		xmlFreeDoc(xmlDoc);
	return 0;
}



static picasaweb_data_write_t * picasa_str_append(picasaweb_data_write_t * xml_list,const char* str_add)
{
	int len_str=0;
	if(str_add){
		len_str = strlen(str_add);
		if(len_str>0){
			picasa_func_write_data((void*)str_add,(size_t)1,len_str,(void*)xml_list);
		}
	}
	return xml_list;
}

/**
@brief getting the field of new album for creating new album,call picasa_free_album_field() to free the space
**/
static picasaweb_data_write_t * picasa_get_album_field(album_info_t *album_info)
{	
	picasaweb_data_write_t *album_field=(picasaweb_data_write_t *)picasa_malloc(sizeof(picasaweb_data_write_t));
	const char entry_head[]="<entry xmlns='http://www.w3.org/2005/Atom' xmlns:media='http://search.yahoo.com/mrss/' xmlns:gphoto='http://schemas.google.com/photos/2007'>";
	if(album_field){
		__picasaweb_data_write_init(album_field,Picasaweb_data_write_buffer,NULL);
		album_field = picasa_str_append(album_field,entry_head);
		if(album_info->title.data){
			album_field = picasa_str_append(album_field,"<title type='text'>");
			album_field = picasa_str_append(album_field,album_info->title.data);
			album_field = picasa_str_append(album_field,"</title>");
		}
		if(album_info->summary.data){
			album_field = picasa_str_append(album_field,"<summary type='text'>");
			album_field = picasa_str_append(album_field,album_info->summary.data);
			album_field = picasa_str_append(album_field,"</summary>");
		}
		if(album_info->location.data){
			album_field = picasa_str_append(album_field,"<gphoto:location>");
			album_field = picasa_str_append(album_field,album_info->location.data);
			album_field = picasa_str_append(album_field,"</gphoto:location>");
		}
		if(album_info->access.data){
			album_field = picasa_str_append(album_field,"<gphoto:access>");
			album_field = picasa_str_append(album_field,album_info->access.data);
			album_field = picasa_str_append(album_field,"</gphoto:access>");
		}
		if(album_info->timestamp.data){
			album_field = picasa_str_append(album_field,"<gphoto:timestamp>");
			album_field = picasa_str_append(album_field,album_info->timestamp.data);
			album_field = picasa_str_append(album_field,"</gphoto:timestamp>");
		}
		if(album_info->gphoto_id.data){
			album_field = picasa_str_append(album_field,"<gphoto:id>");
			album_field = picasa_str_append(album_field,album_info->gphoto_id.data);
			album_field = picasa_str_append(album_field,"</gphoto:id>");
		}
		album_field = picasa_str_append(album_field,"<category scheme='http://schemas.google.com/g/2005#kind' term='http://schemas.google.com/photos/2007#album'></category>");
		album_field = picasa_str_append(album_field,"</entry>");
	}
	return album_field;
}

/**
@brief free the space which is occupied when the picasa_get_album_field() is called
@prarm[in] album_field	: the pointer retrived from picasa_get_album_field()
**/
static int picasa_free_album_field(picasaweb_data_write_t * album_field)
{
	if(album_field){
		__picasaweb_data_write_free(album_field);
		picasa_free((char*)album_field);
	}
	return 0;
}



static picasaweb_data_write_t * picasa_get_photo_field(photo_info_t *photo_info)
{	
	picasaweb_data_write_t *photo_field=(picasaweb_data_write_t *)picasa_malloc(sizeof(picasaweb_data_write_t));
	const char entry_head[]="<entry xmlns='http://www.w3.org/2005/Atom' xmlns:media='http://search.yahoo.com/mrss/' xmlns:gphoto='http://schemas.google.com/photos/2007'>";
	if(photo_field){
		__picasaweb_data_write_init(photo_field,Picasaweb_data_write_buffer,NULL);
		photo_field = picasa_str_append(photo_field,entry_head);
		if(photo_info->summary.data){
			photo_field = picasa_str_append(photo_field,"<summary type='text'>");
			photo_field = picasa_str_append(photo_field,photo_info->summary.data);
			photo_field = picasa_str_append(photo_field,"</summary>");
		}
		if(photo_info->description.data){
			photo_field = picasa_str_append(photo_field,"<media:group><media:description type='text'>");
			photo_field = picasa_str_append(photo_field,photo_info->description.data);
			photo_field = picasa_str_append(photo_field,"</media:description></media:group>");
		
		}
		photo_field = picasa_str_append(photo_field,"</entry>");
	}
	return photo_field;
}

static int picasa_free_photo_field(picasaweb_data_write_t * photo_field)
{
	if(photo_field){
		__picasaweb_data_write_free(photo_field);
		picasa_free((char*)photo_field);
	}
	return 0;
}

void  picasa_sem_wait(sem_t *sem)
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
			picasa_info("work_sem_pend: errno:%d\n",errsv);
			return;
		}
	}

	return ;
}

void  picasa_sem_post(sem_t *sem)
{
	int err;
	err = sem_post(sem);
	return;
}


unsigned long __picasa_get_timestamp(){
	struct timeval cur_time;
	if(gettimeofday(&cur_time,NULL)==0)	
		return cur_time.tv_sec*1000000L+cur_time.tv_usec;
	else{
		if(picasa_timestamp>=0xff)
			picasa_timestamp = 1;
		picasa_timestamp++;
		return picasa_timestamp;
	}
}


int  picasa_get_msg(picasaweb_gdata_t *gdata,picasa_ioctrl_t * req)
{
	int rtn=0;
	rtn = picasa_req_dequeue(gdata,req);
	return rtn;
}


void  *picasa_thread(void *arg)
{	
	int rtn = 0;
	picasaweb_gdata_t *gdata = (picasaweb_gdata_t *)arg;
	picasa_ioctrl_t req;
	if(gdata==NULL){
		picasa_err("Thread Gdata=NULL\n");
		return NULL;
	}
	while(1){
		picasa_info("Wait Semi Start----------\n");
		picasa_sem_wait(&gdata->syn_lock.semi_start);
		pthread_testcancel();
		rtn = picasa_get_msg(gdata,&req);
		picasa_info("Get Msg idx=%d\n",rtn);
		if(rtn>=0){
			switch(req.iocmd){
				case PICASA_CMD_DOWNLOADPHOTO:
					if(req.para_size == sizeof(photo_down_info_t))
						picasa_download_photo(gdata,(photo_down_info_t *)req.para);
					break;
				case PICASA_CMD_STOPDOWNLOAD:
					break;
				case PICASA_CMD_UPLOADPHOTO:
					if(req.para_size == sizeof(photo_upload_info_t))
						picasa_upload_photo(gdata,(photo_upload_info_t*)req.para);
					break;
				case PICASA_CMD_STOPUPLOAD:	
					break;
				default:
					picasa_info("Not Supprot yet CMD=%d\n",req.iocmd);
					break;
			}
			picasa_req_done(gdata,rtn);
		}	
	}
	pthread_exit((void*)rtn);
	return NULL;
}



int picasa_create_thread(picasaweb_gdata_t *gdata)
{
	int rtn = 0;
	pthread_t  tid;
	int arg=0;


	if(sem_init(&gdata->syn_lock.semi_start,0,0)==-1){
		picasa_err("Sem init error");
		goto CREATE_THREAD_END;
	}

	if(sem_init(&gdata->syn_lock.semi_end,0,0)==-1){
		sem_destroy(&gdata->syn_lock.semi_start);
		picasa_err("Sem init error");
		goto CREATE_THREAD_END;
	}

	if(sem_init(&gdata->syn_lock.semi_req,0,0)==-1){
		sem_destroy(&gdata->syn_lock.semi_start);
		sem_destroy(&gdata->syn_lock.semi_end);
		picasa_err("Sem init error");
		goto CREATE_THREAD_END;
	}

		
	rtn = pthread_create(&tid, NULL,picasa_thread,gdata);

	if(rtn){
		picasa_err("Create Picasa thread error!");
		sem_destroy(&gdata->syn_lock.semi_start);
		sem_destroy(&gdata->syn_lock.semi_end);
		sem_destroy(&gdata->syn_lock.semi_req);
		goto CREATE_THREAD_END;
	}

	picasa_sem_post(&gdata->syn_lock.semi_req);
	
	gdata->thread_id = tid;
	gdata->is_thread_run = 1;
	
CREATE_THREAD_END:
	return rtn;
}

int picasa_thread_exit(picasaweb_gdata_t *gdata)
{
	void * thread_ret=NULL;
	if(gdata && gdata->is_thread_run){
		pthread_cancel(gdata->thread_id);
		pthread_join(gdata->thread_id,&thread_ret);

		sem_destroy(&gdata->syn_lock.semi_start);
		sem_destroy(&gdata->syn_lock.semi_end);
		sem_destroy(&gdata->syn_lock.semi_req);
		gdata->is_thread_run = 0;
	}
	return 0;
}

/**
@brief call this function to fill the photo name in the download info, call picasa_download_free_photoname() to release
@param[in] down_info	: where the download information be stored
@param[in] feed		: it is the picasaweb_feed_t which get from picasa_get_albums_info() or picasa_get_album_info()
@param[in] which_entry 	: which entry that the photo url in
@return
	- 0 	: success
	- -1	: failed
**/
int picasa_download_fill_photoname(photo_down_info_t * down_info,picasaweb_feed_t *feed,int which_entry)
{
	char * namebuf=NULL;
	int photo_type=0;
	char photo_name[TMP_BUF_LEN]="";
	if(down_info && feed){
		photo_type=picasa_get_photo_type(down_info->photo_url.data,&namebuf);
		if(namebuf)
			picasa_free_photo_type(namebuf);
		picasa_info("Down Load Photo Photo Type==%d\n",photo_type);
		switch(photo_type){
			case PICASA_PHOTO_TYPE_JPEG:
				sprintf(photo_name,"%s%s",(feed->entry+which_entry)->id.data,".jpg");
				picasa_fill_atomic((xmlChar*)photo_name,&down_info->img_path.photo_name);//the album id
				break;
			case PICASA_PHOTO_TYPE_BMP:
				sprintf(photo_name,"%s%s",(feed->entry+which_entry)->id.data,".bmp");
				picasa_fill_atomic((xmlChar*)photo_name,&down_info->img_path.photo_name);//the album id
				break;
			case PICASA_PHOTO_TYPE_PNG:
				sprintf(photo_name,"%s%s",(feed->entry+which_entry)->id.data,".png");
				picasa_fill_atomic((xmlChar*)photo_name,&down_info->img_path.photo_name);//the album id
				break;
			case PICASA_PHOTO_TYPE_GIF:
				sprintf(photo_name,"%s%s",(feed->entry+which_entry)->id.data,".gif");
				picasa_fill_atomic((xmlChar*)photo_name,&down_info->img_path.photo_name);//the album id
				break;
			default:
				picasa_fill_atomic((xmlChar*)"",&down_info->img_path.photo_name);//the album id
				return -1;
				break;
		}
		if(feed->feed_kind==FEED_CONTACT){///这个地方需要做特殊处理，因为contact里面没有填充id
			if(down_info->img_path.photo_name.data!=NULL){
				picasa_free_atomic(&down_info->img_path.photo_name);
				if((feed->entry+which_entry)->author.name.data!=NULL){
					sprintf(photo_name,"%s%s",(feed->entry+which_entry)->user.data,".jpg");
					picasa_fill_atomic((xmlChar*)photo_name,&down_info->img_path.photo_name);
				}
			}
		}
		return 0;
	}
	return -1;
}

void picasa_download_free_photoname(photo_down_info_t * down_info)
{
	if(down_info)
		picasa_free_atomic(&down_info->img_path.photo_name);	
}

/**
@brief save the feed as ini file
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] feed	: the picasa feed get from picasa_get_albums_info(), picasa_get_album_info or picasa_get_contact()
@param[in] filename: the file which will be created to store to infomation
@return 
 	-1	: failed
 	0	: succeed
**/
int picasa_save_feed(picasaweb_gdata_t *gdata,picasaweb_feed_t * feed,char* filename)
{
	void *fhandle=NULL;
	fhandle = (void*)picasa_fopen(filename,"wb+");
	picasa_info("Save Feed ===========handle=0x%x",fhandle);
	if(fhandle && feed){
		picasa_write_feed(fhandle,0,feed);
		picasa_fflush(fhandle);
		picasa_fclose(fhandle);
	}
	else
		return -1;
	return 0;
}

/**
@brief load the feed which had been saved
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] filename : the file where the infomation be sored
@return 
	NULL : failed
	others: succeed
**/
picasaweb_feed_t * picasa_load_feed(picasaweb_gdata_t *gdata,char* filename)
{
	unsigned int entry_offset = 0;
	int rtn=0,i=0;
	void *fhandle=NULL;
	fhandle = (void*)picasa_fopen(filename,"rb");
	picasaweb_feed_t * feed=NULL;
	picasa_info("Read Albums ===========handle=0x%x",fhandle);
	feed = (picasaweb_feed_t*)picasa_malloc(sizeof(picasaweb_feed_t));
	if(feed==NULL){
		picasa_err("Read Album Malloc Feed Albums Failed!");
		rtn = -1;
		goto READ_ALBUMS_END;
	}
	memset(feed,0,sizeof(picasaweb_feed_t));
	if(fhandle){
		picasa_read_feed(fhandle,0,feed);
		__picasa_printf_feed(feed);
		picasa_info("ENTRY NUM======%d",feed->entry_num);
		if(feed->entry_num!=0){
			feed->entry = (picasaweb_entry_t*)picasa_malloc(feed->entry_num*sizeof(picasaweb_entry_t));
			if(feed->entry!=NULL){
				for(i=0;i<feed->entry_num;i++){
					entry_offset = picasa_ftell(fhandle);
					picasa_info("Entry Offset[%d]=0x%x",i,entry_offset);
					picasa_read_entry(fhandle,entry_offset,feed->entry+i);
					picasa_info("######Entry num=%d#########",i);
					__picasa_printf_entry(feed->entry+i);
				}
			}
		}
	}

READ_ALBUMS_END:
	if(rtn){
		if(feed){
			picasa_free_feed(feed);
			feed = NULL;
		}
	}
	if(fhandle)
		picasa_fclose(fhandle);
	return feed;
}


void picasa_free_cache_path(picasa_path_t *img_path)
{
	if(img_path){
		picasa_free_atomic(&img_path->cache_dir);
		picasa_free_atomic(&img_path->album_id);
		picasa_free_atomic(&img_path->photo_name);
	}
}

#if 1
/**##########################################################################################################################**/
EXPORT_SYMBOL
char picasa_debug(CURL *curl)
{
	curl_easy_setopt(curl,CURLOPT_VERBOSE,1L);
	return 0;
}

/**
@brief copy the content to the atomic element, call the picasa_free_atomic() to free the space 
@param[in] content	: the content 
@param[in] atomic_name	: which atomic element will be filled
@return 
	- -1	: failed
	- 0	: success
@see picasa_free_atomic()
**/
EXPORT_SYMBOL
int  picasa_fill_atomic(xmlChar * content,picasaweb_data_atomic_t *atomic_name)
{
	char * buf=NULL;
	int str_len = 0;
	if(atomic_name==NULL)
		return 0;
	memset(atomic_name,0,sizeof(picasaweb_data_atomic_t));
	if(content==NULL){
		return -1;	
	}
	str_len = strlen((char*)content);
	if(str_len==0)
		return 0;
	buf = (char*)picasa_malloc(str_len+1);
	if(buf!=NULL){
		memcpy(buf,(char*)content,str_len);
		buf[str_len] = 0;
		atomic_name->data_len = str_len+1;
		atomic_name->data = buf;
	}
	else{
		picasa_err("Malloc Failed conent=%s\n",content);
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
EXPORT_SYMBOL
int picasa_free_atomic(picasaweb_data_atomic_t *atomic_name)
{
	if(atomic_name==NULL)
		return 0;
	if(atomic_name->data && atomic_name->data_len!=0){
		picasa_free(atomic_name->data);
		atomic_name->data_len = 0;
		return 0;
	}
	else if(atomic_name->data==NULL && atomic_name->data_len==0){
		return 0;
	}
	else{
		picasa_err("Something must be wrong!\n");
		return -1;
	}
}

/**
@biref call this function when the app is started for initializing system resource, such as hash table
@param[in] NULL
@return 
	- 0	: succeed
	- 1	: failed
**/
EXPORT_SYMBOL
int picasa_init_resource()
{
	picasa_init_tag_table();
	return 0;
}

/**
@brief call this function to set the email addr and password
@param[in] email	: email addr
@param[in] pwd	: password
@return
	- 0 	: success
	- -1	: failed
**/
EXPORT_SYMBOL
int picasa_set_longin(const char* email, const char* pwd)
{
	if(picasa_set_emailaddr(email)!=0)
		return -1;
	if(picasa_set_pwd(pwd)!=0)
		return -1;
	return 0;
}

/**
@brief initializing the structure of picasaweb_gdata_t ,call the picasa_free_gdata() to free the space
remember to call this function first, for the reason that the other function will be used it
@param[in] gdata	: the pointer to the gdata which will be filled
@return 
	- -1	: failed
	- 0	: success
@see picasa_free_gdata()
**/
EXPORT_SYMBOL
int picasa_init_gdata(picasaweb_gdata_t *gdata)
{
	int rtn=0,tmpstr_len=0;
	char *tmpstr=NULL;
	memset(gdata,0,sizeof(picasaweb_gdata_t));

	tmpstr = picasa_get_emailaddr();
	tmpstr_len=strlen(tmpstr);
	gdata->login_name=picasa_malloc(tmpstr_len+1);
	if(gdata->login_name){
		strcpy(gdata->login_name,tmpstr);
		gdata->login_name[tmpstr_len]=0;
		gdata->login_name_len=tmpstr_len+1;
	}
	else{
		rtn = -1;
		goto INIT_DATA_END;
	}

	tmpstr = picasa_get_pwd();
	tmpstr_len=strlen(tmpstr);
	gdata->login_pwd=picasa_malloc(tmpstr_len+1);
	if(gdata->login_pwd){
		strcpy(gdata->login_pwd,tmpstr);
		gdata->login_pwd[tmpstr_len] = 0;
		gdata->login_pwd_len =tmpstr_len+1; 
	}
	else{
		rtn = -1;
		goto INIT_DATA_END;
	}

	if(picasa_create_thread(gdata)!=0){
		rtn = -1;
		goto INIT_DATA_END;
	}
	
	picasa_req_init_queue();
	
	gdata->curl = curl_easy_init();

	gdata->update_thread_id = -1;
INIT_DATA_END:
	picasa_info("addremail=0x%X,PWD=0x%X\n",(unsigned int)gdata->login_name,(unsigned int)gdata->login_pwd);
	picasa_info("Email=%s,Maillen=%d,PWD=%s,Pwdlen=%d\n",gdata->login_name,gdata->login_name_len,\
			gdata->login_pwd,gdata->login_pwd_len);

	if(rtn==-1){
		if(gdata->login_name){
			picasa_free(gdata->login_name);
			gdata->login_name_len=0;
			gdata->login_name = NULL;
		}
		if(gdata->login_pwd){
			picasa_free(gdata->login_pwd);
			gdata->login_pwd_len=0;
			gdata->login_pwd = NULL;
		}
	}
	return rtn;
}

/**
@brief free the space which had been malloc when the picasa_init_gdata() is called
@param[in] gdata	: the pointer to the gdata which had been filled
@see picasa_init_gdata()
**/
EXPORT_SYMBOL
int picasa_free_gdata(picasaweb_gdata_t *gdata)
{
	if(gdata->login_name){
		picasa_free(gdata->login_name);
		gdata->login_name_len=0;
		gdata->login_name = NULL;
	}
	if(gdata->login_pwd){
		picasa_free(gdata->login_pwd);
		gdata->login_pwd_len=0;
		gdata->login_pwd = NULL;
	}
	if(gdata->auth){
		picasa_free(gdata->auth);
		gdata->auth_len=0;
		gdata->auth = NULL;
	}
	if(gdata->curl)
		curl_easy_cleanup(gdata->curl);

	picasa_thread_exit(gdata);
	picasa_exit_update(gdata);
	return 0;
}

/**
@brief : get the contact list, call this function after the authentication, and call picasa_free_contact() to free the space
@param[in] gdata : initialize by calling the picasa_init_gdata()
@return
	- NULL		: get the infomation of the contacts failed
	- others	: the pointer to the picasaweb_feed_t which the information of the contacts are inclued 
**/
EXPORT_SYMBOL
picasaweb_feed_t* picasa_get_contact(picasaweb_gdata_t *gdata)
{
	char friends_url[TMP_BUF_LEN]="";
	xmlDocPtr doc_ptr=NULL;
	picasaweb_feed_t *contact_feed=NULL;
	sprintf(friends_url,"https://picasaweb.google.com/data/feed/api/user/%s/contacts",gdata->login_name);
	picasa_info("GET CONTACT Url=%s\n",friends_url);
	doc_ptr = picasa_get_xmlDoc(gdata,friends_url,1);
	if(doc_ptr){
		xmlXPathContextPtr contex= xmlXPathNewContext(doc_ptr);
		if(contex){
			if(picasa_regist_namespace(contex)==0){
				contact_feed = picasa_get_feed(contex);
			}
			xmlXPathFreeContext(contex);
		}
		else
 			goto PICASA_GET_CONTACT_END;
		if(contact_feed)
			contact_feed->feed_kind = FEED_CONTACT;
	}

PICASA_GET_CONTACT_END:
	if(doc_ptr)
		picasa_free_xmlDoc(doc_ptr);
	return contact_feed;
}

/**
@brief :  free the space which is occupied when the picasa_get_contact() is called 
@param[in] contact_feed :  the pointer to the picasaweb_feed_t which the information of the contacts are included when the picasa_get_contact() is called
@return 
	always return 0
**/
EXPORT_SYMBOL
int picasa_free_contact(picasaweb_feed_t *contact_feed)
{
	picasa_free_feed(contact_feed);
	return 0;
}




/**
@brief call this function after the authentication is ok for getting the albums in the account
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] feed_contact 	: get from picasa_get_cotact(), if this value is null, get albums info of the login account
@param[in] which_friend	: if the feed_contact is NULL, this value is useless, if the feed_contact is not NULL, specify which friend will be get, the range of this value is 0~ (feed_contact->entry_num-1)
@return 
	- NULL		: get the infomation of the albums failed
	- others	: the pointer to the picasaweb_feed_t which the information of the albums is inclued 
**/
EXPORT_SYMBOL
picasaweb_feed_t * picasa_get_albums_info(picasaweb_gdata_t *gdata,picasaweb_feed_t * feed_contact,int which_friend)
{
	xmlDocPtr doc_ptr=NULL;
	char tmp_user_url[TMP_BUF_LEN]="";
	picasaweb_feed_t * feed_albums = NULL;
	if(feed_contact!=NULL){
		if(which_friend<0 || which_friend >= feed_contact->entry_num){
			picasa_err("Friend Entry is out of range,MAX=%d,Your Value=%d!",feed_contact->entry_num,which_friend);
			goto GET_ALBUMS_INFO_END;
		}
		sprintf(tmp_user_url,"https://picasaweb.google.com/data/feed/api/user/%s",(feed_contact->entry+which_friend)->user.data);
	}
	else
		sprintf(tmp_user_url,"%s",picasaweb_user_url);
	doc_ptr = picasa_get_xmlDoc(gdata,tmp_user_url,1);
	if(doc_ptr){
		xmlXPathContextPtr contex = xmlXPathNewContext(doc_ptr);
		if(contex){
			if(picasa_regist_namespace(contex)==0)
				feed_albums = picasa_get_feed(contex);
			xmlXPathFreeContext(contex);	
		}
		else
			goto GET_ALBUMS_INFO_END;
		if(feed_albums){
			feed_albums->feed_kind = FEED_ALBUMS;
		}
		
	}	
GET_ALBUMS_INFO_END:	
	if(doc_ptr){
		picasa_free_xmlDoc(doc_ptr);
	}
	return feed_albums;
}


/**
@brief free the space which is occupied when the picasa_get_albums_info() is called 
@param[in] albums_info 	: the pointer to the picasaweb_feed_t which the information of the album is included when the picasa_get_albums_info() is called
@return always 0
@see picasa_get_albums_info()
**/
EXPORT_SYMBOL
int picasa_free_albums_info(picasaweb_feed_t * feed_albums)
{
	int i=0;
	if(feed_albums){
		for(i=0;i<feed_albums->entry_num;i++){ ///< free the feed_album frist
			picasa_free_feed((feed_albums->entry+i)->child_feed);
			(feed_albums->entry+i)->child_feed = NULL;
		}
		picasa_free_feed(feed_albums);
	}
	return 0;
}


/**
@brief getting the information of an album which is specified by the para used
@param[in] gdata	: initialize by calling the picasa_init_gdata();
@param[in] feed_albums	: the pointer to the picasaweb_feed_t which include all information of the albums in the account
@param[in] which_album	: which album will specified, the range of this value is 0~(feed_albums->entry_num-1)
@return
	NULL	: get the infomation of the album failed
	others	: the pointer to the picasaweb_feed_t which the information of the album 
@see call picasa_free_album_info() to free the space which is malloced
**/
EXPORT_SYMBOL
picasaweb_feed_t * picasa_get_album_info(picasaweb_gdata_t *gdata,picasaweb_feed_t *feed_albums,int which_album)
{
	char album_url[TMP_BUF_LEN]="";
	picasaweb_feed_t *feed_album=NULL;
	xmlDocPtr album_doc_ptr=NULL;
	picasa_info("GET ALBUM~~~AlbumsNum=%d~~~~\n",feed_albums->entry_num );
	if(feed_albums && feed_albums->feed_kind==FEED_ALBUMS){
		if(feed_albums->entry_num <=0 || which_album>=feed_albums->entry_num || which_album<0)
			goto GET_ALBUM_INFO_END;
		
		sprintf(album_url,"%s",(feed_albums->entry+which_album)->link_feed.href.data);

		picasa_info("album_url=%s\n",album_url);
		album_doc_ptr = picasa_get_xmlDoc(gdata,album_url,1);
		if(album_doc_ptr){
			xmlXPathContextPtr album_contex = xmlXPathNewContext(album_doc_ptr);
			if(album_contex){
				picasa_regist_namespace(album_contex);
				feed_album = picasa_get_feed(album_contex);
				xmlXPathFreeContext(album_contex);
			}	
		}
	}

GET_ALBUM_INFO_END:	
	if(album_doc_ptr)
		picasa_free_xmlDoc(album_doc_ptr);
	if(feed_album){	
		feed_album->feed_kind = FEED_PHOTOS;
		(feed_albums->entry+which_album)->child_feed = feed_album;
	}
	return feed_album;	
}

/**
@brief free the space which is occupied when the picasa_get_album_info() is called 
@param[in] album_info 	: the pointer to the picasaweb_feed_t which the information of the photos are included when the picasa_get_album_info() is called
@param[in] feed_albums : which feed_albums does this album belong to, get it from calling picasa_get_albums_info();
@param[in] which_album : the album index in the feed_albums,the range of this value is 0~(feed_albums->entry_num-1)
@return always 0
@see picasa_get_album_info()
**/
EXPORT_SYMBOL
int picasa_free_album_info(picasaweb_feed_t *feed_albums,int which_album)
{
	if(feed_albums){
		if(feed_albums->entry_num <=0 || which_album>=feed_albums->entry_num || which_album<0){
			picasa_err("Crazy Feed Albums Error!which_album=%d, entry_num=%d",which_album,feed_albums->entry_num);
			while(1);
		}
	}
	else{
		picasa_err("Free Album Info Error! Feed Albums=NULL");
		while(1);
	}
		
	picasa_free_feed((feed_albums->entry+which_album)->child_feed);
	(feed_albums->entry+which_album)->child_feed = NULL;
	
	return 0;
}

/**
@brief call this function for passing the authentication
@param[in] gdata	: call picasa_init_gdata() to initializing the gdata
@return
	the status of the processing, see macro get_status();
	- HttpStatus_OK : success
**/
EXPORT_SYMBOL
int picasa_authentication(picasaweb_gdata_t *gdata)
{
	int rtn=PICASA_RTN_OK;
	int status=0;
	char * post_buf=NULL;
	long m_http_code=0;
	CURLcode curlcode=0;
	struct curl_slist *picasa_list_head=NULL;


	curl_easy_setopt(gdata->curl,CURLOPT_SSL_VERIFYPEER,0);
	curl_easy_setopt(gdata->curl,CURLOPT_URL,picasaweb_login_url);

	picasa_list_head = curl_slist_append(picasa_list_head,picasaweb_gdata_version);
	curl_easy_setopt(gdata->curl, CURLOPT_HTTPHEADER, picasa_list_head);

	post_buf=(char*)picasa_malloc(DATA_POST_BUF_LEN);
	if(post_buf==NULL){
		rtn = PICASA_RTN_ERR_MEMORY;
		goto AUTHENTICATION_END;
	} 
	if(gdata->login_name!=NULL && gdata->login_pwd!=NULL){
		sprintf(post_buf,"accountType=HOSTED_OR_GOOGLE&Email=%s&Passwd=%s&service=lh2&source=%s",\
			gdata->login_name,gdata->login_pwd,picasaweb_gdata_source);
	}
	else{
		picasa_err("Email Or PWD Error!\n");
		rtn= PICASA_RTN_ERR_LOGIN;
		goto AUTHENTICATION_END;
	}	
	
	curl_easy_setopt(gdata->curl, CURLOPT_POST, 1L);//do a regular http post
	curl_easy_setopt(gdata->curl, CURLOPT_POSTFIELDS,post_buf);
	curl_easy_setopt(gdata->curl, CURLOPT_POSTFIELDSIZE,strlen(post_buf));

	curl_easy_setopt(gdata->curl,CURLOPT_WRITEDATA,gdata);
	curl_easy_setopt(gdata->curl,CURLOPT_WRITEFUNCTION,pacasa_func_get_auth);	

	curl_easy_setopt(gdata->curl,CURLOPT_TIMEOUT,CURL_OPT_TIMEOUT);	

	curlcode= curl_easy_perform(gdata->curl);	
	curl_easy_getinfo(gdata->curl, CURLINFO_RESPONSE_CODE, &m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
	}
	else{
		picasa_err("Authentication Error\n");
		rtn  = PICASA_RTN_ERR_LOGIN;
	}
	
 AUTHENTICATION_END:
	if(post_buf)
		picasa_free(post_buf);
	if(picasa_list_head)	
		curl_slist_free_all(picasa_list_head);
	picasa_info("curlcode=%d,m_http_code=%d\n",(int)curlcode,(int)m_http_code);
	status = get_status(rtn,curlcode,m_http_code);
	return status;
}

/**
@brief call this function for creating new albums
@param[in] album_info	: the initial info of the album which will be created
@param[in] gdata	: call picasa_init_gdata() to initializing the gdata
@return
	the status of the processing, see macro get_status();
	- HttpStatus_CREATED: success
**/
EXPORT_SYMBOL
int picasa_create_new_album(album_info_t *album_info,picasaweb_gdata_t *gdata)
{
	long m_http_code=0;
	int rtn=PICASA_RTN_OK;
	int status=0;
	CURLcode curlcode=0;
	struct curl_slist *picasa_list_head=NULL;
	char * tmp_str=NULL;
	picasaweb_data_write_t * album_field=NULL;
	picasaweb_data_write_t photo_data;
	__picasaweb_data_write_init(&photo_data,Picasaweb_data_write_buffer,NULL);

	picasa_info("Createing New Album url=%s\n",picasaweb_user_url);
	//curl_easy_setopt(gdata->curl,CURLOPT_FRESH_CONNECT,1);
	curl_easy_setopt(gdata->curl,CURLOPT_POST,1L);
	curl_easy_setopt(gdata->curl,CURLOPT_URL,picasaweb_user_url);

	tmp_str=(char*)picasa_malloc(gdata->auth_len+32);
	if(tmp_str==NULL){
		picasa_info("%s,%d:Malloc Failed\n",__FILE__,__LINE__);
		rtn=PICASA_RTN_ERR_MEMORY;
		goto PICASA_CREATE_ALBUM_END;
	}
	else{
		memset(tmp_str,0,gdata->auth_len+32);
		sprintf(tmp_str,"Authorization: GoogleLogin %s",gdata->auth);
		picasa_info("tmp_str=%s\n",tmp_str);
	}
	picasa_list_head = curl_slist_append(picasa_list_head,"Content-Type: application/atom+xml");
	picasa_list_head = curl_slist_append(picasa_list_head,picasaweb_gdata_version);	
	picasa_list_head = curl_slist_append(picasa_list_head,tmp_str);	

	curl_easy_setopt(gdata->curl, CURLOPT_HTTPHEADER,picasa_list_head);
		
	album_field = picasa_get_album_field(album_info);
	if(album_field==NULL){
		rtn=PICASA_RTN_ERR_CREATALBUM;
		goto PICASA_CREATE_ALBUM_END;
	}
	picasa_info("LENTH11=%d,ALBUM_FIELD=%s\n",strlen(album_field->data_head),album_field->data_head);
	curl_easy_setopt(gdata->curl, CURLOPT_POSTFIELDS,album_field->data_head);
	curl_easy_setopt(gdata->curl, CURLOPT_POSTFIELDSIZE,album_field->data_used);

	curl_easy_setopt(gdata->curl,CURLOPT_WRITEDATA,&photo_data);
	curl_easy_setopt(gdata->curl,CURLOPT_WRITEFUNCTION,picasa_func_write_data);

	curl_easy_setopt(gdata->curl,CURLOPT_TIMEOUT,CURL_OPT_TIMEOUT);	

	curlcode = curl_easy_perform(gdata->curl);
	curl_easy_getinfo(gdata->curl, CURLINFO_RESPONSE_CODE, &m_http_code);

	picasa_info("curlcode=%d,http_code=%d\n",(int)curlcode,(int)m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_CREATED){
		//if(photo_data.data_type==Picasaweb_data_write_buffer)
		//	_test_save_to_file(photo_data.data_head,photo_data.data_len,"newalbum.txt");
	}
	else{
		picasa_info("%s,%d:Create New Album Error\n",__FILE__,__LINE__);
		rtn  = PICASA_RTN_ERR_CREATALBUM;
	}

PICASA_CREATE_ALBUM_END:
	if(album_field)
		picasa_free_album_field(album_field);
	if(tmp_str)
		picasa_free(tmp_str);
	if(picasa_list_head)
		curl_slist_free_all(picasa_list_head);	
	__picasaweb_data_write_free(&photo_data);
	
	status = get_status(rtn,curlcode,m_http_code);
	return status;	
}

/**
@brief call this function for deleting albums
@param[in] gdata	: call picasa_init_gdata() to initializing the gdata
@param[in] feed_albums	: the album information of the albums, see picasa_get_albums_info()
@param[in] which_album	: which album to be choosen
@return
	the status of the processing, see macro get_status();
	- HttpStatus_OK : success
**/
EXPORT_SYMBOL
int picasa_delete_album(picasaweb_gdata_t *gdata,picasaweb_feed_t *feed_albums,int which_album)
{
	int status=PICASA_RTN_OK;
	int rtn=0;
	CURLcode curlcode=0;
	long m_http_code=0;
	char etag_str[64]="";
	struct curl_slist *picasa_list_head=NULL;
	char * tmp_str=NULL;

	picasaweb_data_write_t photo_data;
	__picasaweb_data_write_init(&photo_data,Picasaweb_data_write_buffer,NULL);

	
	if(feed_albums && feed_albums->feed_kind==FEED_ALBUMS){
		if(feed_albums->entry_num <=0 || which_album>=feed_albums->entry_num || which_album<0){
			rtn = PICASA_RTN_ERR_DELETEALBUM;
			goto DELETE_ALBUM_END;
		}
		
		picasa_info("Delete Album==%s\n",(feed_albums->entry+which_album)->link_edit.href.data);
		//curl_easy_setopt(gdata->curl,CURLOPT_FRESH_CONNECT,1);
		curl_easy_setopt(gdata->curl,CURLOPT_CUSTOMREQUEST,"DELETE"); 
		curl_easy_setopt(gdata->curl,CURLOPT_URL,(feed_albums->entry+which_album)->link_edit.href.data);

		tmp_str=(char*)picasa_malloc(gdata->auth_len+32);
		if(tmp_str==NULL){
			picasa_info("%s,%d:Malloc Failed\n",__FILE__,__LINE__);
			rtn = PICASA_RTN_ERR_MEMORY;
			goto DELETE_ALBUM_END;
		}
		else{
			memset(tmp_str,0,gdata->auth_len+32);
			sprintf(tmp_str,"Authorization: GoogleLogin %s",gdata->auth);
			picasa_info("tmp_str=%s\n",tmp_str);
		}
		picasa_list_head = curl_slist_append(picasa_list_head,"Content-Type: application/atom+xml");
		picasa_list_head = curl_slist_append(picasa_list_head,picasaweb_gdata_version);

		sprintf(etag_str,"%s%s","If-Match: ",(feed_albums->entry+which_album)->attr_etag.data);
		picasa_info("ETAG==%s\n",etag_str);

		picasa_list_head = curl_slist_append(picasa_list_head,etag_str);
		picasa_list_head = curl_slist_append(picasa_list_head,tmp_str);	

		curl_easy_setopt(gdata->curl, CURLOPT_HTTPHEADER,picasa_list_head);

		curl_easy_setopt(gdata->curl,CURLOPT_WRITEDATA,&photo_data);
		curl_easy_setopt(gdata->curl,CURLOPT_WRITEFUNCTION,picasa_func_write_data);

		curl_easy_setopt(gdata->curl,CURLOPT_TIMEOUT,CURL_OPT_TIMEOUT);	

		curlcode = curl_easy_perform(gdata->curl);
		curl_easy_getinfo(gdata->curl, CURLINFO_RESPONSE_CODE, &m_http_code);
		picasa_info("curlcode=%d,http_code=%d\n",(int)curlcode,(int)m_http_code);
		if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
			//if(photo_data.data_type==Picasaweb_data_write_buffer)
			//	_test_save_to_file(photo_data.data_head,photo_data.data_len,"delalbum.txt");
		}
		else{
			picasa_info("%s,%d:Delete Album Error\n",__FILE__,__LINE__);
			rtn  = PICASA_RTN_ERR_MEMORY;
		}
	}
	else 
		rtn = PICASA_RTN_ERR_PARA;

DELETE_ALBUM_END:
	if(tmp_str)
		picasa_free(tmp_str);
	if(picasa_list_head)
		curl_slist_free_all(picasa_list_head);	

	__picasaweb_data_write_free(&photo_data);

	status = get_status(rtn,curlcode,m_http_code);
	return status;

}

/**
*@brief call this function to update some info of the album
*@param[in] album_info	: change the info of the album if the element of album_info is not NULL, the gphoto_id member should be the value that retrived from the server.
*@param[in] gdata	: initialize by calling the picasa_init_gdata()
*@param[in] feed_albums	: the pointer to the picasaweb_feed_t where the info of the albums stored, see picasa_get_albums_info()
*@param[in] which_album	: which album to be changed
@return
	the status of the processing, see macro get_status();
	- HttpStatus_OK : success
**/
EXPORT_SYMBOL
int picasa_update_album_info(album_info_t * album_info,picasaweb_gdata_t *gdata,picasaweb_feed_t *feed_albums,int which_album)
{
	int status = 0;
	int rtn= PICASA_RTN_OK;
	char etag_str[32]="";
	char *tmp_str=NULL;
	picasaweb_data_write_t *album_field=NULL;
	CURLcode curlcode=0;
	long m_http_code=0;
	struct curl_slist *picasa_list_head=NULL;
	picasaweb_data_write_t album_data;

	if(feed_albums && feed_albums->feed_kind==FEED_ALBUMS && album_info){
		if(feed_albums->entry_num <=0 || which_album>=feed_albums->entry_num || which_album<0){
			picasa_err("MAX_ALBUM_NUM=%d,you alubm num=%d\n",feed_albums->entry_num,which_album);
			rtn = PICASA_RTN_ERR_PARA;
			goto UPDATE_ALBUM_END;
		}
		picasa_info("UPDATE_ALBUM url=%s\n",(feed_albums->entry+which_album)->link_edit.href.data);	
	
		//curl_easy_setopt(gdata->curl,CURLOPT_FRESH_CONNECT,1);
		curl_easy_setopt(gdata->curl,CURLOPT_CUSTOMREQUEST,"PUT"); 
		curl_easy_setopt(gdata->curl,CURLOPT_URL,(feed_albums->entry+which_album)->link_edit.href.data);

		tmp_str=(char*)picasa_malloc(gdata->auth_len+32);
		if(tmp_str==NULL){
			picasa_info("%s,%d:Malloc Failed\n",__FILE__,__LINE__);
			rtn = PICASA_RTN_ERR_MEMORY;
			goto UPDATE_ALBUM_END;
		}
		else{
			memset(tmp_str,0,gdata->auth_len+32);
			sprintf(tmp_str,"Authorization: GoogleLogin %s",gdata->auth);
			picasa_info("tmp_str=%s\n",tmp_str);
		}
		picasa_list_head = curl_slist_append(picasa_list_head,tmp_str);	
		picasa_list_head = curl_slist_append(picasa_list_head,"Content-Type: application/atom+xml");
		//picasa_list_head = curl_slist_append(picasa_list_head,"X-HTTP-Method-Override: PATCH");
		picasa_list_head = curl_slist_append(picasa_list_head,picasaweb_gdata_version);
		//sprintf(etag_str,"%s%s","If-Match: ",(feed_albums->entry+which_album)->attr_etag.data);
		sprintf(etag_str,"%s%s","If-Match: ","*");
		picasa_info("ETAG==%s\n",etag_str);
		picasa_list_head = curl_slist_append(picasa_list_head,etag_str);

		curl_easy_setopt(gdata->curl, CURLOPT_HTTPHEADER,picasa_list_head);

		if(album_info->gphoto_id.data!=NULL){
			picasa_free_atomic(&album_info->gphoto_id);//free the old first
		}
		picasa_fill_atomic((xmlChar*)(feed_albums->entry+which_album)->id.data,&album_info->gphoto_id);//fill the gphoto_id correctly

		album_field = picasa_get_album_field(album_info);
		if(album_field==NULL){
			rtn = PICASA_RTN_ERR_MEMORY;
			goto UPDATE_ALBUM_END;
		}
		picasa_info("ALBUM_field=%s\n",album_field->data_head);
		curl_easy_setopt(gdata->curl, CURLOPT_POSTFIELDS,album_field->data_head);
		curl_easy_setopt(gdata->curl, CURLOPT_POSTFIELDSIZE,album_field->data_used);

		__picasaweb_data_write_init(&album_data,Picasaweb_data_write_buffer,NULL);
		
		curl_easy_setopt(gdata->curl,CURLOPT_WRITEDATA,&album_data);
		curl_easy_setopt(gdata->curl,CURLOPT_WRITEFUNCTION,picasa_func_write_data);

		curl_easy_setopt(gdata->curl,CURLOPT_TIMEOUT,CURL_OPT_TIMEOUT);	

		curlcode = curl_easy_perform(gdata->curl);
		curl_easy_getinfo(gdata->curl, CURLINFO_RESPONSE_CODE, &m_http_code);
		picasa_info("curlcode=%d,http_code=%d\n",(int)curlcode,(int)m_http_code);
		if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
			//if(album_data.data_type==Picasaweb_data_write_buffer)
			//	_test_save_to_file(album_data.data_head,album_data.data_len,"updatealbum.txt");
		}
		else{
			picasa_info("%s,%d:Update Album Error\n",__FILE__,__LINE__);
			rtn  = PICASA_RTN_ERR_UPDATEALBUM;
		}	
		__picasaweb_data_write_free(&album_data);
	}
	else 
 		rtn = PICASA_RTN_ERR_PARA;

UPDATE_ALBUM_END:
	if(tmp_str)
		picasa_free(tmp_str);
	if(album_field)
		picasa_free_album_field(album_field);
	if(picasa_list_head)
		curl_slist_free_all(picasa_list_head);	
	
	picasa_free_atomic(&album_info->gphoto_id);
	
	status = get_status(rtn,curlcode,m_http_code);
	return status;
}


/**
@brief call this function to initializing the photo_upload_info_t which will be used in picasa_upload_photo(), call picasa_free_upload_info()
to release the resource
@param[in]	photo_fullpath	: the full path of the photo which will be uploaded
@param[in] feed		: it is the picasaweb_feed_t which get from picasa_get_albums_info()
@param[in] which_entry 	: which album does the photo to be added
@return 
	- NULL	: failed
	- others	: the pointer to the photo_upload_info_t
@see picasa_free_upload_info()
**/
EXPORT_SYMBOL
photo_upload_info_t *  picasa_init_upload_info(char* photo_fullpath,picasaweb_feed_t *feed,int which_entry)
{
	void * fp=NULL;
	photo_upload_info_t * upload_info=NULL;
	fp = (void*)picasa_fopen(photo_fullpath,(char*)"rb");
	if(fp==NULL){
		picasa_info("Open File Error=%s\n",photo_fullpath);
		return NULL;
	}
	upload_info = (photo_upload_info_t *)picasa_malloc(sizeof(photo_upload_info_t));
	if(upload_info){
		memset(upload_info,0,sizeof(photo_upload_info_t));
		upload_info->filesize_bytes = picasa_get_file_size(photo_fullpath);
		upload_info->photo_type = picasa_get_photo_type(photo_fullpath,&upload_info->photo_name);
		picasa_info("FileSize=%d,phototype=%d\n",(int)upload_info->filesize_bytes,(int)upload_info->photo_type);
		__picasaweb_data_write_init(&upload_info->photo_data_send,Picasaweb_data_write_file,fp);
		__picasaweb_data_write_init(&upload_info->photo_data_get,Picasaweb_data_write_buffer,NULL);
		upload_info->feed = feed;
		upload_info->which_entry = which_entry;
	}
	return upload_info;
}


/**
@brief call this function to to release the resource which had been occupied by calling picasa_init_upload_info()
@param[in] upload_info	: the pointer to where the information for uploading stored, get it from calling picasa_init_upload_info()
@return 
	always 0
**/
EXPORT_SYMBOL
int picasa_free_upload_info(photo_upload_info_t * upload_info)
{
	void *fhandle=NULL;
	if(upload_info){
		fhandle = upload_info->photo_data_send.file_handle;
		if(fhandle!=NULL)
			picasa_fclose(fhandle);
		if(upload_info->photo_name){
			picasa_free_photo_type(upload_info->photo_name);
			picasa_info("##########PhotoName===0x%x",upload_info->photo_name);
			upload_info->photo_name = NULL;
		}
		__picasaweb_data_write_free(&upload_info->photo_data_get);
		__picasaweb_data_write_free(&upload_info->photo_data_send);
		picasa_free((char*)upload_info);
		upload_info=NULL;
	}
	return 0;
}

/**
@brief call this function to upload a photo
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] upload_info	: the pointer to where the information for uploading stored, get it from calling picasa_init_upload_info()
@return 
	the status of the processing, see macro get_status();
	- HttpStatus_CREATED	: success
**/
EXPORT_SYMBOL
int picasa_upload_photo(picasaweb_gdata_t *gdata,photo_upload_info_t* upload_info)
{
	void * fp=NULL;
	int status=PICASA_RTN_OK;
	int rtn = 0;
	CURLcode curlcode=0;
	long m_http_code=0;
	char *tmp_str=NULL;
	char album_url[TMP_BUF_LEN]="";
	char content_send[32]="";
	char *slug_send=NULL;
	CURL * tmp_curl=NULL;
	struct curl_slist *picasa_list_head=NULL;
	tmp_curl = curl_easy_init();
	if(upload_info==NULL || gdata==NULL || tmp_curl==NULL){
			picasa_err("Upload photo Para err!");
			rtn = PICASA_RTN_ERR_PARA;
			goto  PICASA_UPLOAD_END;
	}

	if(upload_info->feed && upload_info->feed->feed_kind==FEED_ALBUMS && upload_info->photo_type>=0){
		if(upload_info->feed->entry_num <=0 || upload_info->which_entry >=upload_info->feed->entry_num || upload_info->which_entry <0){
			rtn = PICASA_RTN_ERR_PARA;
			goto  PICASA_UPLOAD_END;
		}
		sprintf(album_url,"%s/%s/%s",picasaweb_user_url,"albumid",(upload_info->feed->entry+upload_info->which_entry)->id.data);	
		
		picasa_info("UPLOAD PHOTO url=%s\n",album_url);

		curl_easy_setopt(tmp_curl,CURLOPT_SSL_VERIFYPEER,0);
		curl_easy_setopt(tmp_curl,CURLOPT_POST,1L);
		curl_easy_setopt(tmp_curl,CURLOPT_URL,album_url);

		tmp_str=(char*)picasa_malloc(gdata->auth_len+32);
		if(tmp_str==NULL){
			picasa_info("%s,%d:Malloc Failed\n",__FILE__,__LINE__);
			rtn = PICASA_RTN_ERR_MEMORY;
			goto PICASA_UPLOAD_END;
		}
		else{
			memset(tmp_str,0,gdata->auth_len+32);
			sprintf(tmp_str,"Authorization: GoogleLogin %s",gdata->auth);
			picasa_info("tmp_str=%s\n",tmp_str);
		}

				
		picasa_list_head = curl_slist_append(picasa_list_head,tmp_str);	
		picasa_list_head = curl_slist_append(picasa_list_head,gdata->auth);

		switch(upload_info->photo_type){
			case PICASA_PHOTO_TYPE_JPEG:
				picasa_list_head = curl_slist_append(picasa_list_head,"Content-Type: image/jpeg");
				break;
			case PICASA_PHOTO_TYPE_BMP:
				picasa_list_head = curl_slist_append(picasa_list_head,"Content-Type: image/bmp");
				break;
			case PICASA_PHOTO_TYPE_PNG:
				picasa_list_head = curl_slist_append(picasa_list_head,"Content-Type: image/png");
				break;
			case PICASA_PHOTO_TYPE_GIF:
				picasa_list_head = curl_slist_append(picasa_list_head,"Content-Type: image/gif");
				break;
		}
		sprintf(content_send,"%s%d","Content-Length: ",(int)upload_info->filesize_bytes);
		picasa_list_head = curl_slist_append(picasa_list_head,content_send);
		
		slug_send=(char*)picasa_malloc(strlen(upload_info->photo_name)+6);
		if(slug_send==NULL){
			rtn = PICASA_RTN_ERR_MEMORY;
			goto PICASA_UPLOAD_END;
		}
		sprintf(slug_send,"%s%s","Slug: ",upload_info->photo_name);
		picasa_list_head = curl_slist_append(picasa_list_head,slug_send);
		curl_easy_setopt(tmp_curl, CURLOPT_HTTPHEADER,picasa_list_head);

		curl_easy_setopt(tmp_curl,CURLOPT_POSTFIELDS,NULL);
		curl_easy_setopt(tmp_curl,CURLOPT_POSTFIELDSIZE,upload_info->filesize_bytes);

		curl_easy_setopt(tmp_curl,CURLOPT_READDATA,upload_info);
		curl_easy_setopt(tmp_curl,CURLOPT_READFUNCTION,picasa_func_read_data);

		curl_easy_setopt(tmp_curl,CURLOPT_WRITEDATA,&upload_info->photo_data_get);
		curl_easy_setopt(tmp_curl,CURLOPT_WRITEFUNCTION,picasa_func_write_data);

		curl_easy_setopt(tmp_curl,CURLOPT_TIMEOUT,CURL_OPT_TIMEOUT);

		curlcode = curl_easy_perform(tmp_curl);
		curl_easy_getinfo(tmp_curl, CURLINFO_RESPONSE_CODE, &m_http_code);
		picasa_info("curlcode=%d,http_code=%d\n",(int)curlcode,(int)m_http_code);
		if(curlcode==CURLE_OK && m_http_code==HttpStatus_CREATED){
			//if(photo_data_get.data_type==Picasaweb_data_write_buffer)
			//	_test_save_to_file(photo_data_get.data_head,photo_data_get.data_len,"uploadphoto.txt");
		}
		else{
			picasa_info("%s,%d:UPLOAD Load Photo Error\n",__FILE__,__LINE__);
			rtn  = PICASA_RTN_ERR_UPLOADPHOTO;
		}

	}
	else 
		rtn = PICASA_RTN_ERR_PARA;

PICASA_UPLOAD_END:
	if(picasa_list_head)
		curl_slist_free_all(picasa_list_head);	
	if(tmp_str)
		picasa_free(tmp_str);
	if(slug_send)
		picasa_free(slug_send);

	status = get_status(rtn,curlcode,m_http_code);
	if(upload_info)
		upload_info->status = status;
	if(tmp_curl)
		curl_easy_cleanup(tmp_curl);
	return status;	
}

/**
@brief query the upload staus,  call this function after sending msg to the thread by calling picasa_send_msg()
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] para	: it is the pointer to the photo_down_info_t, call the picasa_init_download_info() to get it
@param[in] query_cmd: see PicasawebQueryCmd
@return 
	if query_cmd == QUERY_CMD_RESULT
	- -2		: the param is error
	- -1		: had not up yet
	- others	: had been done
	if query_cmd == QUREY_CMD_PROGRESS
	the progress of downloading 
**/
EXPORT_SYMBOL
int picasa_query_upload_status(picasaweb_gdata_t *gdata,photo_upload_info_t * upload_info,PicasawebQueryCmd query_cmd)
{
	int rtn = -1;
	rtn = picasa_req_query(gdata,PICASA_CMD_UPLOADPHOTO,upload_info);
	if(rtn>=0){
		if(upload_info){
			if(QUERY_CMD_RESULT==query_cmd)
				return upload_info->status;
			else if(QUREY_CMD_PROGRESS == query_cmd)
				return (int)(upload_info->uploaded_bytes*100/upload_info->filesize_bytes);
			else{ 
				picasa_err("Query CMD Error cmd=%d",query_cmd);
				return 0;
			}
		}
		else 
			return 0;
	}
	else{
		if(QUREY_CMD_PROGRESS == query_cmd){
			return (int)(upload_info->uploaded_bytes*100/upload_info->filesize_bytes);
		}
		return rtn;
	}
}




/**
@brief call this function to update the information of a photo
@param[in] photo_info 	: the info of the photo which will be replaced
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] feed_album	: the photo information in an album, see picasa_get_album_info()
@param[in] which_photo	: which photo to be choosen
@return 
	the status of the processing, see macro get_status();
	- HttpStatus_OK	: success
**/
EXPORT_SYMBOL
int picasa_update_photo_info(photo_info_t *photo_info,picasaweb_gdata_t *gdata,picasaweb_feed_t *feed_album,int which_photo)
{
	int status = PICASA_RTN_OK;
	int rtn= 0;
	char etag_str[32]="";
	char *tmp_str=NULL;
	picasaweb_data_write_t *photo_field=NULL;
	CURLcode curlcode=0;
	long m_http_code=0;
	struct curl_slist *picasa_list_head=NULL;
	picasaweb_data_write_t photo_data;
	if(feed_album && feed_album->feed_kind==FEED_PHOTOS){
		if(feed_album->entry_num <=0 || which_photo>=feed_album->entry_num || which_photo<0){
			picasa_err("MAX_PHOTO_NUM=%d,you photo num=%d\n",feed_album->entry_num,which_photo);
			rtn = PICASA_RTN_ERR_PARA;
			goto UPDATE_PHOTO_END;
		}

		picasa_info("UPDATE_POTO url=%s\n",(feed_album->entry+which_photo)->link_edit.href.data);	
	
		//curl_easy_setopt(gdata->curl,CURLOPT_FRESH_CONNECT,1);
		//curl_easy_setopt(gdata->curl,CURLOPT_CUSTOMREQUEST,"PATCH"); 
		curl_easy_setopt(gdata->curl,CURLOPT_CUSTOMREQUEST,"PUT"); 
		curl_easy_setopt(gdata->curl,CURLOPT_URL,(feed_album->entry+which_photo)->link_edit.href.data);

		tmp_str=(char*)picasa_malloc(gdata->auth_len+32);
		if(tmp_str==NULL){
			picasa_info("%s,%d:Malloc Failed\n",__FILE__,__LINE__);
			rtn = PICASA_RTN_ERR_MEMORY;
			goto UPDATE_PHOTO_END;
		}
		else{
			memset(tmp_str,0,gdata->auth_len+32);
			sprintf(tmp_str,"Authorization: GoogleLogin %s",gdata->auth);
			picasa_info("tmp_str=%s\n",tmp_str);
		}
		picasa_list_head = curl_slist_append(picasa_list_head,tmp_str);	
		picasa_list_head = curl_slist_append(picasa_list_head,"Content-Type: application/atom+xml");
		picasa_list_head = curl_slist_append(picasa_list_head,"X-HTTP-Method-Override: PATCH");
		picasa_list_head = curl_slist_append(picasa_list_head,picasaweb_gdata_version);

		//sprintf(etag_str,"%s%s","If-Match: ",(feed_album->entry+which_photo)->attr_etag.data);
		sprintf(etag_str,"%s%s","If-Match: ","*");
		picasa_info("ETAG==%s\n",etag_str);
		picasa_list_head = curl_slist_append(picasa_list_head,etag_str);

		curl_easy_setopt(gdata->curl, CURLOPT_HTTPHEADER,picasa_list_head);

		photo_field = picasa_get_photo_field(photo_info);
		if(photo_field==NULL){
			rtn = PICASA_RTN_ERR_MEMORY;
			goto UPDATE_PHOTO_END;
		}
		picasa_info("PHOTO_field=%s\n",photo_field->data_head);
		curl_easy_setopt(gdata->curl, CURLOPT_POSTFIELDS,photo_field->data_head);
		curl_easy_setopt(gdata->curl, CURLOPT_POSTFIELDSIZE,photo_field->data_used);

		__picasaweb_data_write_init(&photo_data,Picasaweb_data_write_buffer,NULL);
		curl_easy_setopt(gdata->curl,CURLOPT_WRITEDATA,&photo_data);
		curl_easy_setopt(gdata->curl,CURLOPT_WRITEFUNCTION,picasa_func_write_data);

		curl_easy_setopt(gdata->curl,CURLOPT_TIMEOUT,CURL_OPT_TIMEOUT);

		curlcode = curl_easy_perform(gdata->curl);
		curl_easy_getinfo(gdata->curl, CURLINFO_RESPONSE_CODE, &m_http_code);
		picasa_info("curlcode=%d,http_code=%d\n",(int)curlcode,(int)m_http_code);
		if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
			//if(photo_data.data_type==Picasaweb_data_write_buffer)
			//	_test_save_to_file(photo_data.data_head,photo_data.data_len,"updatephoto.txt");
		}
		else{
			picasa_info("%s,%d:Update Photo Error\n",__FILE__,__LINE__);
			rtn  = PICASA_RTN_ERR_UPDATEPHOTO;
		}
	}
UPDATE_PHOTO_END:
	if(tmp_str)
		picasa_free(tmp_str);
	if(photo_field)
		picasa_free_photo_field(photo_field);
	if(picasa_list_head)
		curl_slist_free_all(picasa_list_head);	
	__picasaweb_data_write_free(&photo_data);
	
	status = get_status(rtn,curlcode,m_http_code);
	return status;	
}


/**
@brief call this function to delete a photo
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] feed_album	: the photo information in an album, see picasa_get_album_info()
@param[in] which_photo	: which photo to be choosen
@return 
	the status of the processing, see macro get_status();
	- HttpStatus_OK	: success
**/
EXPORT_SYMBOL
int picasa_delete_photo(picasaweb_gdata_t *gdata,picasaweb_feed_t *feed_album,int which_photo)
{
	int status = 0;
	int rtn=PICASA_RTN_OK;
	CURLcode curlcode=0;
	long m_http_code=0;
	char etag_str[64]="";
	struct curl_slist *picasa_list_head=NULL;
	char * tmp_str=NULL;

	picasaweb_data_write_t photo_data;
	__picasaweb_data_write_init(&photo_data,Picasaweb_data_write_buffer,NULL);

	
	if(feed_album && feed_album->feed_kind==FEED_PHOTOS){
		if(feed_album->entry_num <=0 || which_photo>=feed_album->entry_num || which_photo<0){
			rtn = PICASA_RTN_ERR_PARA;
			goto DELETE_PHOTO_END;
		}	
		picasa_info("Delete Photo==%s\n",(feed_album->entry+which_photo)->link_edit.href.data);
		//curl_easy_setopt(gdata->curl,CURLOPT_FRESH_CONNECT,1);
		curl_easy_setopt(gdata->curl,CURLOPT_CUSTOMREQUEST,"DELETE"); 
		curl_easy_setopt(gdata->curl,CURLOPT_URL,(feed_album->entry+which_photo)->link_edit.href.data);

		tmp_str=(char*)picasa_malloc(gdata->auth_len+32);
		if(tmp_str==NULL){
			picasa_info("%s,%d:Malloc Failed\n",__FILE__,__LINE__);
			rtn =PICASA_RTN_ERR_MEMORY;
			goto DELETE_PHOTO_END;
		}
		else{
			memset(tmp_str,0,gdata->auth_len+32);
			sprintf(tmp_str,"Authorization: GoogleLogin %s",gdata->auth);
			picasa_info("tmp_str=%s\n",tmp_str);
		}
		picasa_list_head = curl_slist_append(picasa_list_head,"Content-Type: application/atom+xml");
		picasa_list_head = curl_slist_append(picasa_list_head,picasaweb_gdata_version);

		sprintf(etag_str,"%s%s","If-Match: ",(feed_album->entry+which_photo)->attr_etag.data);
		picasa_info("ETAG==%s\n",etag_str);

		picasa_list_head = curl_slist_append(picasa_list_head,etag_str);
		picasa_list_head = curl_slist_append(picasa_list_head,tmp_str);	
		//picasa_list_head = curl_slist_append(picasa_list_head,gdata->auth);
		curl_easy_setopt(gdata->curl, CURLOPT_HTTPHEADER,picasa_list_head);

		curl_easy_setopt(gdata->curl,CURLOPT_WRITEDATA,&photo_data);
		curl_easy_setopt(gdata->curl,CURLOPT_WRITEFUNCTION,picasa_func_write_data);

		curl_easy_setopt(gdata->curl,CURLOPT_TIMEOUT,CURL_OPT_TIMEOUT);

		curlcode = curl_easy_perform(gdata->curl);
		curl_easy_getinfo(gdata->curl, CURLINFO_RESPONSE_CODE, &m_http_code);
		picasa_info("curlcode=%d,http_code=%d\n",(int)curlcode,(int)m_http_code);
		if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
			//if(photo_data.data_type==Picasaweb_data_write_buffer)
			//	_test_save_to_file(photo_data.data_head,photo_data.data_len,"deletephoto.txt");
		}
		else{
			picasa_info("%s,%d:Delete Photo Error\n",__FILE__,__LINE__);
			rtn  = PICASA_RTN_ERR_DELETEPHOTO;
		}	
	}
	else
		rtn = PICASA_RTN_ERR_PARA;

DELETE_PHOTO_END:
	if(tmp_str)
		picasa_free(tmp_str);
	if(picasa_list_head)
		curl_slist_free_all(picasa_list_head);	

	__picasaweb_data_write_free(&photo_data);
	status = get_status(rtn,curlcode,m_http_code);
	return status;	
}


/**
@brief download photo
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@parma[in] down_info	: choose the method to be download, cache or uncache ,see __picasa_init_download_info()
@return 
	the status of the processing, see macro get_status();
	- HttpStatus_OK	: success
**/
EXPORT_SYMBOL
int picasa_download_photo(picasaweb_gdata_t *gdata,photo_down_info_t *down_info)
{
	int status = 0;
	long m_http_code=0;
	int rtn=PICASA_RTN_OK;
	CURL *tmp_curl=NULL;
	CURLcode curlcode=0;
	struct curl_slist *picasa_list_head=NULL;
	char * tmp_str=NULL;
	tmp_curl = curl_easy_init();
	if(down_info==NULL || tmp_curl==NULL){
		picasa_err("DOWN info ==NULL\n");
		return 0;
	}
	
	picasaweb_data_write_t *photo_data = &down_info->photo_data;
	
	if(down_info->photo_url.data==NULL){
		picasa_err("Photo Url Error!\n");
		rtn  = PICASA_RTN_ERR_DOWNLOADPHOTO;
		goto DOWNLOAD_PHOTO_END;
	}
		
	picasa_info("DOWN LOAD Photo URL=%s\n",down_info->photo_url.data);

	curl_easy_setopt(tmp_curl,CURLOPT_SSL_VERIFYPEER,0);
	curl_easy_setopt(tmp_curl, CURLOPT_CUSTOMREQUEST,NULL);
	curl_easy_setopt(tmp_curl,CURLOPT_HTTPGET,1L);
	curl_easy_setopt(tmp_curl,CURLOPT_URL,down_info->photo_url.data);

	curl_easy_setopt(tmp_curl,CURLOPT_WRITEDATA,photo_data);
	curl_easy_setopt(tmp_curl,CURLOPT_WRITEFUNCTION,picasa_func_write_data);


	tmp_str=(char*)picasa_malloc(gdata->auth_len+32);
	if(tmp_str==NULL){
		picasa_info("%s,%d:Malloc Failed\n",__FILE__,__LINE__);
		rtn=PICASA_RTN_ERR_MEMORY;
		goto DOWNLOAD_PHOTO_END;
	}
	else{
		memset(tmp_str,0,gdata->auth_len+32);
		sprintf(tmp_str,"Authorization: GoogleLogin %s",gdata->auth);
		picasa_info("tmp_str=%s\n",tmp_str);
	}
	picasa_list_head = curl_slist_append(picasa_list_head,"Content-Type: application/atom+xml");
	picasa_list_head = curl_slist_append(picasa_list_head,picasaweb_gdata_version);	
	picasa_list_head = curl_slist_append(picasa_list_head,tmp_str);	
	curl_easy_setopt(tmp_curl, CURLOPT_HTTPHEADER,picasa_list_head);

	curl_easy_setopt(tmp_curl,CURLOPT_NOPROGRESS,0L);
	curl_easy_setopt(tmp_curl,CURLOPT_PROGRESSFUNCTION,picasa_func_prog);
	curl_easy_setopt(tmp_curl,CURLOPT_PROGRESSDATA,down_info);

	curl_easy_setopt(tmp_curl,CURLOPT_TIMEOUT,CURL_OPT_TIMEOUT);	

	curlcode = curl_easy_perform(tmp_curl);
	curl_easy_getinfo(tmp_curl, CURLINFO_RESPONSE_CODE, &m_http_code);
	down_info->workdone = 1;
	picasa_info("curlcode=%d,http_code=%d\n",(int)curlcode,(int)m_http_code);
	if(down_info->func_download_callback){
		down_info->func_download_callback(down_info->func_para);
	}	

	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		picasa_set_downloadbit(down_info->feed,down_info->which_entry,down_info->isthumbnail); ///< set the download bit if success
	}
	else{
		picasa_info("%s,%d:Down Load Photo Error\n",__FILE__,__LINE__);
		rtn  = PICASA_RTN_ERR_DOWNLOADPHOTO;
	}
DOWNLOAD_PHOTO_END:
	if(tmp_str)
		picasa_free(tmp_str);
	if(picasa_list_head)
		curl_slist_free_all(picasa_list_head);
	status = get_status(rtn,curlcode,m_http_code);	
		down_info->status = status;

	if(tmp_curl)
		curl_easy_cleanup(tmp_curl);
	return status;
}

/**
@brief call this function to initialize the download info ,call __picasa_free_download_info() to free the space 
@param[in] iscache	: whether it is cached
@param[in] cache_dir	: if iscache==1, it is the path of cache dir, such as /mnt/udisk/
@param[in] feed		: it is the picasaweb_feed_t which get from picasa_get_albums_info() or picasa_get_album_info()
@param[in] which_entry 	: which entry that the photo url in
@param[in] isthumbnail	: whether it is a thumbnail, if it is it will be cached in the thumbnail folder
@param[out] err_status : when the return value is NULL, the reason of err is stored in here,see struct of picasa_file_check_err_e
@return 
	- NULL 	: failed
	- others : success
@see __picasa_free_download_info()
**/
EXPORT_SYMBOL
photo_down_info_t *picasa_init_download_info(int iscache,char* cache_dir,picasaweb_feed_t *feed,int which_entry,int isthumbnail,int *err_status)
{
	char photo_name[TMP_BUF_LEN]="";
	int rtn=0,photo_type=0;
	int errstatus=0;
	void *fhandle=NULL;
	photo_down_info_t * down_info=NULL;
	char * namebuf=NULL;
	down_info = (photo_down_info_t *)picasa_malloc(sizeof(photo_down_info_t));
	if(down_info==NULL)
		return NULL;
	memset(down_info,0,sizeof(photo_down_info_t));
	down_info->workdone=0;

	if(iscache){
		down_info->needcache = 1;
		memset(&down_info->img_path,0,sizeof(picasa_path_t));
		picasa_fill_atomic((xmlChar*)cache_dir,&down_info->img_path.cache_dir);
		/**需要填充img_path 的album_id 作为Photo 存储的文件夹**/
		if(feed->feed_kind==FEED_ALBUMS){//if it contains the albums the album_id==NULL
			if(isthumbnail){//if it is thumbnail,stored in the thumbnail floder
				picasa_fill_atomic((xmlChar*)(feed->entry+which_entry)->group.thumbnail.url.data,&down_info->photo_url);
				picasa_fill_atomic((xmlChar*)"thumbnail",&down_info->img_path.album_id);
				down_info->isthumbnail = 1;
			}
			else{//stored under the cache dir
				picasa_fill_atomic((xmlChar*)(feed->entry+which_entry)->group.content.url.data,&down_info->photo_url);
				picasa_fill_atomic((xmlChar*)"",&down_info->img_path.album_id);
				down_info->isthumbnail = 0;
			}
			
		}
		else if(feed->feed_kind==FEED_PHOTOS){//if it contains photos, fill the album_id
			if(isthumbnail){
				picasa_fill_atomic((xmlChar*)(feed->entry+which_entry)->group.thumbnail.url.data,&down_info->photo_url);
				sprintf(photo_name,"%s/%s",(feed->entry+which_entry)->albumid.data,"thumbnail");
				picasa_fill_atomic((xmlChar*)photo_name,&down_info->img_path.album_id);
				down_info->isthumbnail = 1;
			}
			else{
				picasa_fill_atomic((xmlChar*)(feed->entry+which_entry)->group.content.url.data,&down_info->photo_url);
				picasa_fill_atomic((xmlChar*)(feed->entry+which_entry)->albumid.data,&down_info->img_path.album_id);
				down_info->isthumbnail = 0;
			}
		}
		if(feed->feed_kind==FEED_CONTACT){//if it contains photos, fill the album_id
			picasa_fill_atomic((xmlChar*)(feed->entry+which_entry)->thumbnail.data,&down_info->photo_url);
			picasa_fill_atomic((xmlChar*)"user_profile",&down_info->img_path.album_id);
			down_info->isthumbnail = 0;
			isthumbnail=0;
		}
		if(down_info->photo_url.data==NULL){//may be some type of the file had not photo_url,such as the png had not thumbnail url
			rtn = -1;
			goto INIT_DOWNLOAD_INFO_END;
		}
		if(picasa_download_fill_photoname(down_info,feed,which_entry)==-1){//fill the photo name in the download info
			rtn = -1;
			goto INIT_DOWNLOAD_INFO_END;
		}
	
		rtn = picasa_check_path(&down_info->img_path,isthumbnail); 		
		if(rtn!=0){
			goto INIT_DOWNLOAD_INFO_END;
		}
		
		fhandle = (void*)picasa_open_file(&(down_info->img_path),&errstatus);
		picasa_info("fhandle=0x%x,status=%d\n",fhandle,errstatus);
		if(fhandle==NULL){
			rtn = errstatus;
			goto INIT_DOWNLOAD_INFO_END;
		}
		__picasaweb_data_write_init(&down_info->photo_data,Picasaweb_data_write_file,fhandle);
	}
	else{
		down_info->needcache = 0;
		__picasaweb_data_write_init(&down_info->photo_data,Picasaweb_data_write_buffer,NULL);
	}

	down_info->feed = feed;
	down_info->which_entry = which_entry;
	

INIT_DOWNLOAD_INFO_END:
	if(rtn!=0){
		if(fhandle!=NULL){
			picasa_close_file(fhandle);
		}
		picasa_free_atomic(&down_info->photo_url);
		picasa_free_atomic(&down_info->img_path.cache_dir);
		picasa_free_atomic(&down_info->img_path.album_id);
		picasa_download_free_photoname(down_info);
		picasa_free((char*)down_info);
		down_info = NULL;
	}
	picasa_info("%s,%d: rtn = %d\n",__FILE__,__LINE__,rtn);
	*err_status = rtn;
	return down_info;
};

/**
@brief release the space which is malloc when the __picasa_init_download_info() is called
@param[in] down_info 	: get from __picasa_init_download_info()
@return
	always return 0
**/
EXPORT_SYMBOL
int picasa_free_download_info(photo_down_info_t* down_info)
{
	void * fhandle = NULL;
	char tmpbuf[TMP_BUF_LEN]="";
	if(down_info){
		fhandle = down_info->photo_data.file_handle;
		if(fhandle){
			picasa_fflush(fhandle);
			picasa_fclose(fhandle);
			if(down_info->status!=HttpStatus_OK){///down load failed, need remove the file which had been created
				if(picasa_get_file_path(&down_info->img_path,&tmpbuf)!=0){
					picasa_fremove(tmpbuf);
					
				}
				/// clear the download bit
				picasa_clear_downloadbit(down_info->feed,down_info->which_entry,down_info->isthumbnail);
			}
		}
		picasa_free_atomic(&down_info->photo_url);
		__picasaweb_data_write_free(&down_info->photo_data);
		picasa_free_atomic(&down_info->img_path.cache_dir);
		picasa_free_atomic(&down_info->img_path.album_id);
		picasa_download_free_photoname(down_info);
		picasa_free((char*)down_info);
		down_info = NULL;
	}
	return 0;
}


/**
@brief query the download staus, call this function after sending msg to the thread by calling picasa_send_msg()
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] down_info	: it is the pointer to the photo_down_info_t, call the picasa_init_download_info() to get it
@param[in] query_cmd: see PicasawebQueryCmd
@return 
	if query_cmd == QUERY_CMD_RESULT
	- -2		: the param is error
	- -1		: had not down yet
	- others	: had been done
	if query_cmd == QUREY_CMD_PROGRESS
	the progress of downloading 
**/
EXPORT_SYMBOL
int picasa_query_download_status(picasaweb_gdata_t *gdata,photo_down_info_t * down_info,PicasawebQueryCmd query_cmd)
{
	int rtn = -1;
	if(query_cmd==QUREY_CMD_PROGRESS){
		if(down_info)
			return down_info->prog_down;
		else
			return -2;
	}
	rtn = picasa_req_query(gdata,PICASA_CMD_DOWNLOADPHOTO,down_info);
	if(rtn>=0){
		if(down_info){
			if(query_cmd==QUERY_CMD_RESULT)
				return down_info->status;
			else if(query_cmd==QUREY_CMD_PROGRESS)
				return down_info->prog_down;
			else{
				picasa_err("Query Download Cmd Error cmd =%d",query_cmd);
				return 0;
			}
		}
		else 
			return 0;
	}
	else{
		if(query_cmd==QUREY_CMD_PROGRESS){
			return down_info->prog_down;
		}
		return rtn;
	}
}


/**
@brief get the full path of the photo
@param[in] feed		: it is the picasaweb_feed_t which get from picasa_get_albums_info() or picasa_get_album_info()
@param[in] which_entry 	: which entry that the photo url in
@param[in] isthumbnail	: whether it is a thumbnail, if it is it will be cached in the thumbnail folder
@param[out] pathbuf : where the path to be stored
@param[in] buf_len	: the length of the pathbuf
@return 
	- -1	: failed
	- 0	: succeed
**/
EXPORT_SYMBOL
int picasa_get_cache_path(picasaweb_feed_t *feed,int which_entry,int isthumbnail,char *pathbuf,int buf_len)
{
	picasa_path_t img_path;
	int buf_len_need=0;
	int photo_type=0;
	char * namebuf=NULL;
	char photo_name[TMP_BUF_LEN]="";
	char * photo_url = NULL;
	memset(&img_path,0,sizeof(picasa_path_t));
	if(feed==NULL || feed->entry_num < which_entry ||which_entry<0)
		return -1;
	if(feed->feed_kind==FEED_ALBUMS){//if it contains the albums the album_id==NULL
		if(isthumbnail){//if it is thumbnail,stored in the thumbnail floder
			photo_url = (feed->entry+which_entry)->group.thumbnail.url.data;
			picasa_fill_atomic((xmlChar*)"thumbnail",&img_path.album_id);
		}
		else{//stored under the cache dir
			photo_url = (feed->entry+which_entry)->group.content.url.data;
			picasa_fill_atomic((xmlChar*)"",&img_path.album_id);
		}
		
	}
	else if(feed->feed_kind==FEED_PHOTOS){//if it contains photos, fill the album_id
		if(isthumbnail){
			photo_url = (feed->entry+which_entry)->group.thumbnail.url.data;
			sprintf(photo_name,"%s/%s",(feed->entry+which_entry)->albumid.data,"thumbnail");
			picasa_fill_atomic((xmlChar*)photo_name,&img_path.album_id);
		}
		else{
			photo_url = (feed->entry+which_entry)->group.content.url.data;
			picasa_fill_atomic((xmlChar*)(feed->entry+which_entry)->albumid.data,&img_path.album_id);
		}
	}

	photo_type=picasa_get_photo_type(photo_url,&namebuf);
	if(namebuf)
		picasa_free_photo_type(namebuf);
	picasa_info("Get Cache Path Photo Type==%d\n",photo_type);
	switch(photo_type){
		case PICASA_PHOTO_TYPE_JPEG:
			sprintf(photo_name,"%s%s",(feed->entry+which_entry)->id.data,".jpg");
			picasa_fill_atomic((xmlChar*)photo_name,&img_path.photo_name);//the album id
			break;
		case PICASA_PHOTO_TYPE_BMP:
			sprintf(photo_name,"%s%s",(feed->entry+which_entry)->id.data,".bmp");
			picasa_fill_atomic((xmlChar*)photo_name,&img_path.photo_name);//the album id
			break;
		case PICASA_PHOTO_TYPE_PNG:
			sprintf(photo_name,"%s%s",(feed->entry+which_entry)->id.data,".png");
			picasa_fill_atomic((xmlChar*)photo_name,&img_path.photo_name);//the album id
			break;
		case PICASA_PHOTO_TYPE_GIF:
			sprintf(photo_name,"%s%s",(feed->entry+which_entry)->id.data,".gif");
			picasa_fill_atomic((xmlChar*)photo_name,&img_path.photo_name);//the album id
			break;
		default:
			picasa_fill_atomic((xmlChar*)"",&img_path.photo_name);//the album id
			return -1;
			break;
	}
	buf_len_need = img_path.album_id.data_len+img_path.photo_name.data_len+2;
	if(buf_len< buf_len_need){
		picasa_err("Get Cache Path Erro Buf Len is Short buflen=%d, need=%d",buf_len,buf_len_need);
		picasa_free_cache_path(&img_path);
		return -1;
	}
	else {
		if(img_path.album_id.data)
			sprintf(pathbuf,"/%s/%s",img_path.album_id.data,img_path.photo_name.data);
		else
			sprintf(pathbuf,"/%s",img_path.photo_name.data);
	}
	picasa_free_cache_path(&img_path);
	return 0;
}


void * func_download_callback(void* para)
{
	return NULL;
}

/**
@brief send the message to the thread
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] cmd 	: see picasa_ioctl_cmd_e
@param[in] para	: 
			if cmd ==PICASA_CMD_DOWNLOADPHOTO
				it is the pointer to the photo_down_info_t, call the picasa_init_download_info() to get it
			if cmd==PICASA_CMD_UPLOADPHOTO
				it is the pointer to the photo_upload_info_t, call the picasa_init_upload_info() to get it
@return 
	 - -1	: failed
	 - 0 		: success
**/
EXPORT_SYMBOL
int picasa_send_msg(picasaweb_gdata_t *gdata,picasa_ioctl_cmd_e cmd,void * para)
{
	int rtn = 0;
	int req_idx=0;
	picasa_ioctrl_t req;
	req.iocmd = cmd;
	req.para = para;
	switch(cmd){
		case PICASA_CMD_STOPDOWNLOAD:
			{
				photo_down_info_t * down_info = (photo_down_info_t *)req.para;
				if(down_info)
					down_info->photo_data.cancel_write = 1;
				req.para_size = sizeof(photo_down_info_t);
				break;
			}
		case PICASA_CMD_DOWNLOADPHOTO:
			req.para_size = sizeof(photo_down_info_t);
			break;
		case PICASA_CMD_STOPUPLOAD:
			{
				photo_upload_info_t * upload_info = (photo_upload_info_t *)req.para;
				upload_info->photo_data_send.cancel_write =1;
				req.para_size = sizeof(photo_upload_info_t);
				break;
			}
		case PICASA_CMD_UPLOADPHOTO:
			req.para_size = sizeof(photo_upload_info_t);
			break;
	}
	req.timestamp = __picasa_get_timestamp();
	req_idx = picasa_req_enqueue(gdata,&req);
	picasa_info("Send Msg reqidx=%d\n",req_idx);
	if(req_idx==-1)
		rtn = -1;
	else{
		if(gdata){
			picasa_info("Post Semi Start~~~~~~~~~~~~~~~~~~~~~~\n");
			picasa_sem_post(&gdata->syn_lock.semi_start);
		}
		else
			rtn = -1;
	}
	return rtn;
}

#if 1
/**
@brief save the albums info into a file
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] feed_albums	: get from picasa_get_albums_info()
@param[in] cache_dir	: under which folder will the file be created
@param[in] filename	: the name of the file to be created,if it is null, the default filename is the login name
@return :
	-1 	: failed
	0	: succeed
**/
EXPORT_SYMBOL
int picasa_save_albums_info(picasaweb_gdata_t *gdata,picasaweb_feed_t * feed_albums,char* cache_dir,char* filename)
{
	char namebuf[TMP_BUF_LEN]="";
	int rtn = 0;
	if(picasa_get_ini_albumsname(gdata,cache_dir,filename,namebuf,TMP_BUF_LEN)==-1){
		rtn = -1;
		goto SAVE_ALBUMS_INFO_END;
	}
	rtn = picasa_save_feed(gdata,feed_albums,namebuf);

SAVE_ALBUMS_INFO_END:
	return rtn;
}


/**
@brief load albums info from the file
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] cache_dir	: under which folder will the file be stored
@param[in] filename	: the name of the file to be read
@return :
	NULL	: load albums info error
	others	: he pointer to the picasaweb_feed_t which include all information of the albums in the account
**/
EXPORT_SYMBOL
picasaweb_feed_t * picasa_load_albums_info(picasaweb_gdata_t *gdata,char* cache_dir,char *filename)
{
	char namebuf[TMP_BUF_LEN]="";
	picasaweb_feed_t * feed_albums=NULL;
	int rtn = 0;
	if(picasa_get_ini_albumsname(gdata,cache_dir,filename,namebuf,TMP_BUF_LEN)==-1){
		goto LOAD_ALBUMS_INFO_END;
	}
	feed_albums = picasa_load_feed(gdata,namebuf);

LOAD_ALBUMS_INFO_END:
	return feed_albums;
}



/**
@brief save the albums info into a file
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] feed_albums	: the pointer to the picasaweb_feed_t which include all information of the albums in the account
@param[in] which_album	: which album will specified, the range of this value is 0~(feed_albums->entry_num-1)
@param[in] cache_dir	: under which folder will the file be created
@return :
	-1 	: failed
	0	: succeed
**/
EXPORT_SYMBOL
int picasa_save_album_info(picasaweb_gdata_t *gdata,picasaweb_feed_t *feed_albums,int which_album,char* cache_dir)
{
	int rtn = 0;
	char namebuf[TMP_BUF_LEN]="";
	picasaweb_feed_t * feed_album=NULL;
	if(gdata==NULL || feed_albums==NULL ||(which_album<0 || which_album>=feed_albums->entry_num)){
		rtn = -1;
		goto SAVE_ALBUM_INFO_END;
	}
	if(picasa_get_ini_albumname(feed_albums,which_album,cache_dir,namebuf,TMP_BUF_LEN)==-1){
		goto SAVE_ALBUM_INFO_END;
	}
	feed_album = (feed_albums->entry+which_album)->child_feed;
	rtn = picasa_save_feed(gdata,feed_album,namebuf);
	
SAVE_ALBUM_INFO_END:
	return rtn;
}


/**
@brief load the album info into a file
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] feed_albums	: the pointer to the picasaweb_feed_t which include all information of the albums in the account
@param[in] which_album	: which album will specified, the range of this value is 0~(feed_albums->entry_num-1)
@param[in] cache_dir	: under which folder will the file be created
@return :
	NULL	: load album info error
	others	: the pointer to the picasaweb_feed_t which include all information of the album
**/
EXPORT_SYMBOL
picasaweb_feed_t * picasa_load_album_info(picasaweb_gdata_t *gdata,picasaweb_feed_t *feed_albums,int which_album,char* cache_dir)
{
	char namebuf[TMP_BUF_LEN]="";
	picasaweb_feed_t * feed_album=NULL;
	int rtn = 0;
	if(gdata==NULL || feed_albums==NULL ||(which_album<0 || which_album>=feed_albums->entry_num)){
		rtn = -1;
		goto LOAD_ALBUM_INFO_END;
	}
	if(picasa_get_ini_albumname(feed_albums,which_album,cache_dir,namebuf,TMP_BUF_LEN)==-1){
		goto LOAD_ALBUM_INFO_END;
	}
	feed_album = picasa_load_feed(gdata,namebuf);
	(feed_albums->entry+which_album)->child_feed = feed_album;

LOAD_ALBUM_INFO_END:
	return feed_album;
}


/**
@brief save the contact info into a file
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] feed_contact	: get from picasa_get_contact()
@param[in] cache_dir	: under which folder will the file be created
@return :
	-1 	: failed
	0	: succeed
**/
EXPORT_SYMBOL
int picasa_save_contact_info(picasaweb_gdata_t *gdata,picasaweb_feed_t * feed_contact,char* cache_dir)
{
	char namebuf[TMP_BUF_LEN]="";
	int rtn = 0;
	if(picasa_get_ini_contactname(gdata,cache_dir,namebuf,TMP_BUF_LEN)==-1){
		rtn = -1;
		goto SAVE_CONTACT_INFO_END;
	}
	rtn = picasa_save_feed(gdata,feed_contact,namebuf);

SAVE_CONTACT_INFO_END:
	return rtn;
}


/**
@brief load contact info from the file
@param[in] gdata	: initialize by calling the picasa_init_gdata()
@param[in] cache_dir	: under which folder will the file be stored
@return :
	NULL	: load albums info error
	others	: he pointer to the picasaweb_feed_t which include all information of the contact in the account
**/
EXPORT_SYMBOL
picasaweb_feed_t * picasa_load_contact_info(picasaweb_gdata_t *gdata,char* cache_dir)
{
	char namebuf[TMP_BUF_LEN]="";
	picasaweb_feed_t * feed_contact=NULL;
	int rtn = 0;
	if(picasa_get_ini_contactname(gdata,cache_dir,namebuf,TMP_BUF_LEN)==-1){
		goto LOAD_CONTACT_INFO_END;
	}
	feed_contact = picasa_load_feed(gdata,namebuf);

LOAD_CONTACT_INFO_END:
	return feed_contact;
}

int picasa_write_userinfo(void *file_handle,picasaweb_gdata_t *gdata)
{
	char tmpbuf[LOGIN_MAX_LEN*2] = {0};
	char writebuf[LOGIN_MAX_LEN*2] = {0};
	
	fseek((FILE*)file_handle, 0, SEEK_SET);
	while(fgets(tmpbuf,LOGIN_MAX_LEN*2,(FILE*)file_handle)){
		//picasa_info("%s",tmpbuf);
		if(strncmp(tmpbuf,gdata->login_name,strlen(gdata->login_name))==0){
			picasa_info("user %s has exist",gdata->login_name);
			return 0;
		}
	}
	sprintf(writebuf,"%s\t%s\n",gdata->login_name,gdata->login_pwd);
	picasa_info("%s",writebuf);
	
	if(fwrite(writebuf, strlen(writebuf), 1,(FILE*)file_handle)!=1){
		picasa_info("Write User Infor Error!");
		return -1;
	}
	return 0;
}

EXPORT_SYMBOL
int picasa_save_userinfo(picasaweb_gdata_t *gdata)
{
	char namebuf[TMP_BUF_LEN]="/mnt/vram/picasa_user.ini";
	void *fhandle=NULL;
	fhandle = (void*)fopen(namebuf,"a+");
	picasa_info("Save User Info ===========handle=0x%x",fhandle);
	if(fhandle && gdata){
		picasa_write_userinfo(fhandle,gdata);
		picasa_fflush(fhandle);
		fclose(fhandle);
		fhandle = NULL;
	}
	else
		return -1;
	return 0;
}

int picasa_read_userinfo(void *file_handle)
{
	char tmpbuf[LOGIN_MAX_LEN*2] = {0};
	int user_num = 0;
	
	fseek((FILE*)file_handle, 0, SEEK_SET);
	while(fgets(tmpbuf,LOGIN_MAX_LEN*2,(FILE*)file_handle)){
		//picasa_info("%s",tmpbuf);
		user_num++;
	}
	picasa_info("%d",user_num);
	return user_num;
}

EXPORT_SYMBOL
int picasa_check_user()
{
	char namebuf[TMP_BUF_LEN]="/mnt/vram/picasa_user.ini";
	void *fhandle=NULL;
	int user_num = 0;
	fhandle = (void*)fopen(namebuf,"r");
	picasa_info("Check User Info ===========handle=0x%x",fhandle);
	if(fhandle){
		user_num = picasa_read_userinfo(fhandle);
		picasa_fflush(fhandle);
		fclose(fhandle);
		fhandle = NULL;
	}
	else{
		picasa_info("this is no ini file");
		return 0;
	}
	return user_num;
}

int picasa_get_userinfo(void *file_handle, int user_index, char *buf, int cmd_type)
{
	char tmpbuf[LOGIN_MAX_LEN*2] = {0};
	int user_num = 0;
	char tmpbuf2[LOGIN_MAX_LEN] = {0};
	
	fseek((FILE*)file_handle, 0, SEEK_SET);
	while(fgets(tmpbuf,LOGIN_MAX_LEN*2,(FILE*)file_handle)){
		//picasa_info("%s",tmpbuf);
		user_num++;
		if(user_num==user_index)
			break;
	}
	picasa_info("%d---%s",user_num,tmpbuf);
	switch(cmd_type){
		case 0:
			sscanf(tmpbuf,"%s\t%s\n",buf,tmpbuf2);
			break;
		case 1:
			sscanf(tmpbuf,"%s\t%s\n",tmpbuf2,buf);
			break;
	}
	picasa_info("%s",buf);
	return 0;
}

EXPORT_SYMBOL
int picasa_load_useraccount(int user_index,char *user_account)
{
	char namebuf[TMP_BUF_LEN]="/mnt/vram/picasa_user.ini";
	void *fhandle=NULL;
	fhandle = (void*)fopen(namebuf,"r");
	picasa_info("Load User Info ===========handle=0x%x",fhandle);
	if(fhandle){
		picasa_get_userinfo(fhandle,user_index,user_account,0);
		picasa_fflush(fhandle);
		fclose(fhandle);
		fhandle = NULL;
	}
	else
		return -1;
	return 0;
}

EXPORT_SYMBOL
int picasa_load_userpwd(int user_index,char *user_account)
{
	char namebuf[TMP_BUF_LEN]="/mnt/vram/picasa_user.ini";
	void *fhandle=NULL;
	fhandle = (void*)fopen(namebuf,"r");
	picasa_info("Load User Info ===========handle=0x%x",fhandle);
	if(fhandle){
		picasa_get_userinfo(fhandle,user_index,user_account,1);
		picasa_fflush(fhandle);
		fclose(fhandle);
		fhandle = NULL;
	}
	else
		return -1;
	return 0;
}

int delete_oldalbums(char * cache_dir)
{
	int rtn = -1;
	char call_buf[TMP_BUF_LEN] = {0};
	picasa_info("delete dir is %s",cache_dir);
	sprintf(call_buf,"rm -rf %s",cache_dir);
	rtn = system(call_buf);
	picasa_info("rtn is %d",rtn);
	return 0;
}

static long get_free_space(char * path)
{
	struct statfs fsstatus;
	long blocks;
	long space;

	if(statfs(path, &fsstatus)==0){
		blocks = fsstatus.f_bfree;
		space = fsstatus.f_bsize*blocks/1024/1024;
	}
	else{
		facebook_dbg("get fs stat error==%d\n",errno);
		space = -1;
	}
	return space;
}

void *picasa_updatethread(void *arg)
{	
	int rtn = 0;
	int i=0;
	int j=0;
	struct timeval start_time, end_time;
	picasaweb_feed_t * feed_albums=NULL;
	picasaweb_feed_t * feed_album=NULL;
	photo_down_info_t * down_info=NULL;
	int err_status = 0;
	picasaweb_gdata_t *gdata = NULL;
	char * cache_dir = NULL;
	int iscache = 0;
	int isthumbnail = 0;
	picasa_update_t *update_arg = (picasa_update_t *)arg;
	static char last_update[] = "1970-01-01T00:00:00.000Z";
	static char this_update[] = "1970-01-01T00:00:00.000Z";
	char cache_dir_self[TMP_BUF_LEN] = {0};
	
	if(update_arg==NULL){
		picasa_err("Thread update_arg=NULL");
		rtn = -1;
		goto update_end;
	}
	gdata = update_arg->gdata;
	cache_dir = update_arg->cache_dir;
	iscache = update_arg->iscache;
	isthumbnail = update_arg->isthumbnail;
	
	if(gdata==NULL){
		picasa_err("Thread gdata=NULL");
		rtn = -1;
		goto update_end;
	}
	if(cache_dir==NULL){
		picasa_err("Thread cache dir=NULL");
		rtn = -1;
		goto update_end;
	}
	if(__picasa_create_dir(cache_dir) != 0){
		picasa_err("create dir error");
		rtn = -1;
		goto update_end;
	}
	sprintf(cache_dir_self,"%s/account/",cache_dir);
	if(__picasa_create_dir(cache_dir_self) != 0){
		picasa_err("create self dir error");
		rtn = -1;
		goto update_end;
	}
	while(1){
	update_try_again:
		pthread_testcancel();
		
		picasa_info("Update Start----------");
		feed_albums = picasa_get_albums_info(gdata,0,0);
		if(feed_albums == NULL){
			picasa_err("photoalbums is NULL");
			rtn = -1;
			goto update_try_again;
		}
		
		for(i=0;i<feed_albums->entry_num;i++){
			picasa_info("last update time is %s",last_update);
			picasa_info("album update time is %s",(feed_albums->entry+i)->updated.data);
			if(strcmp(last_update,(feed_albums->entry+i)->updated.data)>=0){
				picasa_info("this album has been download!");
				continue;
			}
			
			feed_album = picasa_get_album_info(gdata,feed_albums,i);
			if(feed_album==NULL){
				picasa_err("Get Album %d Info Error!",i);
				rtn = -1;
				continue;
			}
			for(j=0; j<feed_album->entry_num; j++){
				picasa_info("last update time is %s",last_update);
				picasa_info("photo update time is %s",(feed_album->entry+j)->updated.data);
				if(strcmp(last_update,(feed_album->entry+j)->updated.data)>=0){
					picasa_info("this photo has been download!");
					continue;
				}

				if(get_free_space(TARGET_DISK)<FREE_SPACE){
					picasa_err("there is no enough space!");
					rtn = -1;
					goto free_resource;
				}
				
				down_info = picasa_init_download_info(iscache,cache_dir_self,feed_album,j,isthumbnail,&err_status);
				if(down_info==NULL){
					picasa_err("Init Download Info Error, Album is %d, Photo is %d",i,j);
					rtn = -1;
					continue;
				}
				rtn = picasa_download_photo(gdata,down_info);
				picasa_free_download_info(down_info);
				down_info = NULL;
			}
			if(strcmp(this_update,feed_album->updated.data)<0){
				strcpy(this_update,feed_album->updated.data);
				picasa_info("this update time is %s",this_update);
			}
			
			if(feed_album){
				picasa_free_album_info(feed_albums,i);
				feed_album = NULL;
			}
		}
	free_resource:
		if(feed_album){
			picasa_free_album_info(feed_albums,i);
			feed_album = NULL;
		}
		if(feed_albums){
			picasa_free_albums_info(feed_albums);
			feed_albums = NULL;
		}

		picasa_update_friends(update_arg, last_update, this_update);

		if(strcmp(last_update,this_update)<0){
			strcpy(last_update,this_update);
			facebook_dbg("last update time is %s",last_update);
		}

		gettimeofday(&start_time,NULL);
	delay_time:
		sleep(100);
		gettimeofday(&end_time, NULL);
		picasa_info("start time is %d, end time is %d",start_time.tv_sec,end_time.tv_sec);
		if(end_time.tv_sec-start_time.tv_sec < UPDATE_INTERVAL)
			goto delay_time;
	}
update_end:
	if(update_arg){
		picasa_free((char *)update_arg);
		update_arg = NULL;
	}
	pthread_exit((void*)rtn);
	return NULL;
}

EXPORT_SYMBOL
int picasa_create_update(picasaweb_gdata_t *gdata, int iscache, char* cache_dir, int isthumbnail)
{
	int rtn = -1;
	pthread_t  tid;
	picasa_update_t *update_arg;

	if(gdata==NULL){
		picasa_err("gdata is NULL");
		return -1;
	}
	if(cache_dir==NULL){
		picasa_err("cache_dir is NULL");
		return -1;
	}
	update_arg = (picasa_update_t *)picasa_malloc(sizeof(picasa_update_t));
	if(update_arg==NULL){
		picasa_err("update arg is NULL");
		return -1;
	}
	update_arg->gdata = gdata;
	update_arg->iscache = iscache;
	update_arg->isthumbnail = isthumbnail;
	update_arg->cache_dir = cache_dir;
	
	rtn = pthread_create(&tid, NULL,picasa_updatethread,update_arg);
	if(rtn){
		picasa_err("Create Picasa update thread error!");
	}
	else
		gdata->update_thread_id = tid;
	return rtn;
}

int picasa_exit_update(picasaweb_gdata_t *gdata)
{
	void * thread_ret=NULL;
	picasa_info("exit update thread");
	if(gdata){
		if(gdata->update_thread_id == -1){
			picasa_info("update thread is not running");
			return 0;
		}
		pthread_cancel(gdata->update_thread_id);
		pthread_join(gdata->update_thread_id,&thread_ret);
	}
	return 0;
}

EXPORT_SYMBOL
int picasa_select_friend(int friend_index, char *friend_id)
{
	char namebuf[TMP_BUF_LEN]="/mnt/vram/picasa_friend.ini";
	void *fhandle=NULL;
	char writebuf[TMP_BUF_LEN]="";
	fhandle = (void*)fopen(namebuf,"a+");
	picasa_info("Save selected friend ===========handle=0x%x",fhandle);
	if(fhandle){
		sprintf(writebuf, "%d\t%s\n", friend_index, friend_id);
		if(fwrite(writebuf, strlen(writebuf), 1,(FILE*)fhandle)!=1){
			picasa_err("Write User Infor Error!");
			fclose(fhandle);
			fhandle = NULL;
			return -1;
		}
		picasa_fflush(fhandle);
		fclose(fhandle);
		fhandle = NULL;
	}
	else
		return -1;
	return 0;
}

int picasa_get_friendid(void *file_handle, int index, char *buf)
{
	char tmpbuf[LOGIN_MAX_LEN*2] = {0};
	int user_num = 0;
	int friend_index = -1;
	
	fseek((FILE*)file_handle, 0, SEEK_SET);
	while(fgets(tmpbuf,LOGIN_MAX_LEN*2,(FILE*)file_handle)){
		//picasa_info("%s",tmpbuf);
		if(user_num==index)
			break;
		user_num++;
	}
	sscanf(tmpbuf,"%d\t%s\n",&friend_index,buf);
	picasa_info("%d\t%s",friend_index,buf);
	return friend_index;
}

int picasa_load_friendid(int index,char *friend_id)
{
	char namebuf[TMP_BUF_LEN]="/mnt/vram/picasa_friend.ini";
	void *fhandle=NULL;
	int friend_index = -1;
	fhandle = (void*)fopen(namebuf,"r");
	picasa_info("Load selected friend ===========handle=0x%x",fhandle);
	if(fhandle){
		friend_index = picasa_get_friendid(fhandle,index,friend_id);
		fclose(fhandle);
		fhandle = NULL;
	}
	else
		return -1;
	return friend_index;
}

int picasa_check_friend()
{
	char namebuf[TMP_BUF_LEN]="/mnt/vram/picasa_friend.ini";
	void *fhandle=NULL;
	int user_num = 0;
	char tmpbuf[LOGIN_MAX_LEN*2] = {0};
	fhandle = (void*)fopen(namebuf,"r");
	picasa_info("Check selected friend ===========handle=0x%x",fhandle);
	if(fhandle){
		fseek((FILE*)fhandle, 0, SEEK_SET);
		while(fgets(tmpbuf,LOGIN_MAX_LEN*2,(FILE*)fhandle)){
			//picasa_info("%s",tmpbuf);
			user_num++;
		}
		picasa_info("%d",user_num);
		fclose(fhandle);
		fhandle = NULL;
	}
	else{
		picasa_info("this is no ini file");
		return 0;
	}
	return user_num;
}


int picasa_update_friends(picasa_update_t *update_arg, char *last_update, char *this_update)
{
	int rtn = 0;
	int err_status = 0;
	picasaweb_gdata_t *gdata = NULL;
	char * cache_dir = NULL;
	int iscache = 0;
	int isthumbnail = 0;
	int friend_num = 0;
	int i=0,j=0,k=0;
	int friend_index = -1;
	char friend_id[TMP_BUF_LEN]={0};
	
	picasaweb_feed_t * feed_albums=NULL;
	picasaweb_feed_t * feed_album=NULL;
	photo_down_info_t * down_info=NULL;
	picasaweb_feed_t * feed_contact=NULL;

	char cache_dir_friends[TMP_BUF_LEN] = {0};

	if(update_arg==NULL){
		picasa_err("Thread update_arg=NULL");
		return -1;
	}
	gdata = update_arg->gdata;
	cache_dir = update_arg->cache_dir;
	iscache = update_arg->iscache;
	isthumbnail = update_arg->isthumbnail;
	
	if(gdata==NULL){
		picasa_err("Thread g_data=NULL");
		return -1;
	}
	if(cache_dir==NULL){
		picasa_err("Thread cache_dir=NULL");
		return -1;
	}

	sprintf(cache_dir_friends,"%s/friends/",cache_dir);
	if(__picasa_create_dir(cache_dir_friends) != 0){
		picasa_err("create friends dir error");
		return -1;
	}

	friend_num = picasa_check_friend();
	if(friend_num>0){
		for(k=0;k<friend_num;i++){
			feed_contact = picasa_get_contact(gdata);
			if(feed_contact == NULL){
				picasa_err("get contact error");
				rtn = -1;
				goto free_resource;
			}
			
			friend_index = picasa_load_friendid(k,friend_id);

			memset(cache_dir_friends,0,sizeof(cache_dir_friends));
			sprintf(cache_dir_friends,"%s/friends/%s/",cache_dir,friend_id);
			if(__picasa_create_dir(cache_dir_friends) != 0){
				picasa_err("create friends dir error");
				return -1;
			}

			feed_albums = picasa_get_albums_info(gdata,feed_contact,friend_index);
			if(feed_albums == NULL){
				picasa_err("photoalbums is NULL");
				rtn = -1;
				continue;
			}
		
			for(i=0;i<feed_albums->entry_num;i++){
				picasa_info("last update time is %s",last_update);
				picasa_info("album update time is %s",(feed_albums->entry+i)->updated.data);
				if(strcmp(last_update,(feed_albums->entry+i)->updated.data)>=0){
					picasa_info("this album has been download!");
					continue;
				}
				
				feed_album = picasa_get_album_info(gdata,feed_albums,i);
				if(feed_album==NULL){
					picasa_err("Get Album %d Info Error!",i);
					rtn = -1;
					continue;
				}
				for(j=0; j<feed_album->entry_num; j++){
					picasa_info("last update time is %s",last_update);
					picasa_info("photo update time is %s",(feed_album->entry+j)->updated.data);
					if(strcmp(last_update,(feed_album->entry+j)->updated.data)>=0){
						picasa_info("this photo has been download!");
						continue;
					}

					if(get_free_space(TARGET_DISK)<FREE_SPACE){
						picasa_err("there is no enough space!");
						rtn = -1;
						goto free_resource;
					}
					
					down_info = picasa_init_download_info(iscache,cache_dir_friends,feed_album,j,isthumbnail,&err_status);
					if(down_info==NULL){
						picasa_err("Init Download Info Error, Album is %d, Photo is %d",i,j);
						rtn = -1;
						continue;
					}
					rtn = picasa_download_photo(gdata,down_info);
					picasa_free_download_info(down_info);
					down_info = NULL;
				}
				if(strcmp(this_update,feed_album->updated.data)<0){
					strcpy(this_update,feed_album->updated.data);
					picasa_info("this update time is %s",this_update);
				}
				
				if(feed_album){
					picasa_free_album_info(feed_albums,i);
					feed_album = NULL;
				}
			}
		free_resource:
			if(feed_album){
				picasa_free_album_info(feed_albums,i);
				feed_album = NULL;
			}
			if(feed_albums){
				picasa_free_albums_info(feed_albums);
				feed_albums = NULL;
			}
			if(feed_contact){
				picasa_free_contact(feed_contact);
				feed_contact = NULL;
			}
			
			if(rtn != 0){
				break;
			}
		}
	}
	return 0;
}
#endif

#endif

