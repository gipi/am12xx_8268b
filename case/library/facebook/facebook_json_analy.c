#include "facebook_json_analy.h"
#include "facebook_ioctl.h"
#include <am_types.h>
#include <sys/vfs.h>

static char facebook_login_email[LOGIN_MAX_LEN]="";
static char facebook_login_pwd[LOGIN_MAX_LEN]="";
static unsigned long facebook_timestamp=1;
char * url_reload_p = NULL;

extern facebook_ioctrl facebook_cmd_array[FACEBOOK_IOARRAY_LEN];

char * delete_char(char * str)
{
	int i=0,j=0;
	int len = strlen(str);
	char * ret = facebook_malloc(len+1);
	if(ret==NULL){
		facebook_dbg("delete_char ret malloc failed");
		return NULL;
	}
	//facebook_dbg("str is %s,len is %d",str, len);
	while(i<len)
	{
		if(str[i]!='"' && str[i]!='\\'){
			ret[j++] = str[i++];
		}
		else
			i++;
	}
	ret[j]='\0';
	//facebook_dbg("j is %d,ret is %s",j,ret);
	return ret;
}

char * delete_quotation(char * str)
{
	int i=0;
	char * ret = facebook_malloc(strlen(str)-1);
	if(ret==NULL){
		facebook_dbg("delete_char ret malloc failed");
		return NULL;
	}
	//facebook_dbg("str is %s, len is %d",str,strlen(str));
	if(str[0]=='"'){
		for(i=0;i<strlen(str)-2;i++)
			ret[i] = str[i+1];
	}
	ret[i]='\0';
	//facebook_dbg("ret is %s",ret);
	return ret;
}

int facebook_get_keyindex(char * key)
{
	int index = -1;
	if(strcasecmp(key,"ID")==0)
		index = 0;
	else if(strcasecmp(key,"FROM")==0)
		index = 1;
	else if(strcasecmp(key,"NAME")==0)
		index = 2;
	else if(strcasecmp(key,"DESCRIPTION")==0)
		index = 3;
	else if(strcasecmp(key,"LINK")==0)
		index = 4;
	else if(strcasecmp(key,"COVER_PHOTO")==0)
		index = 5;
	else if(strcasecmp(key,"PRIVACY")==0)
		index = 6;
	else if(strcasecmp(key,"COUNT")==0)
		index = 7;
	else if(strcasecmp(key,"TYPE")==0)
		index = 8;
	else if(strcasecmp(key,"CREATED_TIME")==0)
		index = 9;
	else if(strcasecmp(key,"UPDATED_TIME")==0)
		index = 10;
	else if(strcasecmp(key,"COMMENTS")==0)
		index = 11;
	else if(strcasecmp(key,"PICTURE")==0)
		index = 12;
	else if(strcasecmp(key,"SOURCE")==0)
		index = 13;
	else if(strcasecmp(key,"HEIGHT")==0)
		index = 14;
	else if(strcasecmp(key,"WIDTH")==0)
		index = 15;
	else if(strcasecmp(key,"IMAGES")==0)
		index = 16;
	else if(strcasecmp(key,"ICON")==0)
		index = 17;
	else if(strcasecmp(key,"MESSAGE")==0)
		index = 18;
	else if(strcasecmp(key,"FIRST_NAME")==0)
		index = 19;
	else if(strcasecmp(key,"LAST_NAME")==0)
		index = 20;
	else if(strcasecmp(key,"GENDER")==0)
		index = 21;
	else if(strcasecmp(key,"THIRD_PARTY_ID")==0)
		index = 22;
	else if(strcasecmp(key,"EMAIL")==0)
		index = 23;
	else if(strcasecmp(key,"TIMEZONE")==0)
		index = 24;
	else if(strcasecmp(key,"LOCALE")==0)
		index = 25;
	else if(strcasecmp(key,"VERIFIED")==0)
		index = 26;
	else if(strcasecmp(key,"ABOUT")==0)
		index = 27;
	else if(strcasecmp(key,"BIO")==0)
		index = 28;
	else if(strcasecmp(key,"BIRTHDAY")==0)
		index = 29;
	//facebook_dbg("key is %s,index is %d",key,index);
	return index;
}

int facebook_func_prog(void *p, double dltotal, double dlnow, double ult, double uln)
{
	facebook_write_data *down_info = (facebook_write_data *)p;
	if((int)dltotal==0)
		down_info->download_process = 0;
	else
		down_info->download_process = (int)(dlnow*100/dltotal);
	//facebook_dbg("Progress: %d",down_info->download_process);
	return 0;
}

int get_picture(CURL *curl_p, char * location)
{
	CURLcode curlcode=0;
	long m_http_code=0;
	int rtn = 0;
	int status = 0;
	
	facebook_dbg("URL is %s, len is %d",location, strlen(location));
	curl_easy_setopt(curl_p, CURLOPT_URL,location);

	curlcode = curl_easy_perform(curl_p);
	curl_easy_getinfo(curl_p, CURLINFO_RESPONSE_CODE, &m_http_code);
	facebook_dbg("curlcode is %d",curlcode);
	facebook_dbg("m_http_code is %ld",m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		facebook_dbg("Connect Success");
	}
	else{
		facebook_dbg("Connect Fail");
		rtn = FACEBOOK_RTN_ERR_DOWNLOADPHOTO;
	}
	status= get_status(rtn,curlcode,m_http_code);
	return status;
}

int facebook_fill_atomic(const char * value,facebook_atomic * atomic_t)
{
	char *buf = NULL;
	int len = 0;
	if(value == NULL){
		facebook_dbg("fill value is null!");
		return -1;
	}
	memset(atomic_t,0,sizeof(facebook_atomic));
	len = strlen(value);
	buf = (char *)facebook_malloc(len+1);
	if(buf==NULL){
		facebook_dbg("fill atomic buf is null");
		return -1;
	}
	memcpy(buf,value,len);
	buf[len]=0;
	atomic_t->data = delete_char(buf);
	//atomic_t->data = delete_quotation(buf);
	atomic_t->data_len = strlen(atomic_t->data)+1;
	//facebook_dbg("atomic is %s,len is %d",atomic_t->data, atomic_t->data_len);
	free(buf);
	return 0;
}

int facebook_fill_fromelement(char * key, json_object * obj, facebook_from * from_t)
{
	int idx=-1;
	int rtn = 0;
	if(key){
		idx = facebook_get_keyindex(key);
	}
	if(idx!=-1){
		switch(idx){
			case ID:
				facebook_fill_atomic(json_object_to_json_string(obj),&(from_t->id));
				facebook_dbg("from->id is %s",from_t->id.data);
				break;
			case NAME:
				facebook_fill_atomic(json_object_to_json_string(obj),&(from_t->name));
				facebook_dbg("from->name is %s",from_t->name.data);
				break;
		}
	}
	else
		rtn = -1;
	return rtn;
}

int analy_each_from_info(json_object * obj, facebook_from * from_t)
{
	json_object_object_foreach(obj, key, val){
		//facebook_dbg("\t%s: %s\n", key, json_object_to_json_string(val));
		facebook_fill_fromelement(key,val,from_t);
        }
	return 0;
}

int facebook_fill_commentelement(char * key, json_object * obj, facebook_comment * comments)
{
	int idx=-1;
	int rtn = 0;
	if(key){
		idx = facebook_get_keyindex(key);
	}
	if(idx!=-1){
		switch(idx){
			case ID:
				facebook_fill_atomic(json_object_to_json_string(obj),&(comments->id));
				facebook_dbg("comments->id is %s",comments->id.data);
				break;
			case FROM:
				analy_each_from_info(obj, &(comments->from));
				break;
			case MESSAGE:
				facebook_fill_atomic(json_object_to_json_string(obj),&(comments->message));
				facebook_dbg("comments->message is %s",comments->message.data);
				break;
			case CREATED_TIME:
				facebook_fill_atomic(json_object_to_json_string(obj),&(comments->created_time));
				facebook_dbg("comments->created_time is %s",comments->created_time.data);
				break;
		}
	}
	else
		rtn = -1;
	return rtn;
}

int analy_each_comment_info(json_object * obj, facebook_comment * comments)
{
	json_object_object_foreach(obj, key, val){
		//facebook_dbg("\t%s: %s\n", key, json_object_to_json_string(val));
		facebook_fill_commentelement(key,val,comments);
        }
	return 0;
}

int analy_comment_info(json_object * obj,facebook_comment * comments)
{
	json_object * array_obj=NULL, *each_obj=NULL;
	array_obj = json_object_object_get(obj, "data");
	int i;
	for(i=0; i < json_object_array_length(array_obj); i++){
		each_obj = json_object_array_get_idx(array_obj, i);
		//facebook_dbg("\n[%d]=%s\n", i, json_object_to_json_string(each_obj));
		analy_each_comment_info(each_obj, comments);
	}
	if(array_obj){
		facebook_dbg("free json object");
		json_object_put(array_obj);
	}
	if(each_obj){
		facebook_dbg("free json object");
		json_object_put(each_obj);
	}
	return 0;
}

int facebook_fill_albumelement(char * key, json_object * obj,facebook_album * albuminfo)
{
	int idx=-1;
	int rtn = 0;
	if(key){
		idx = facebook_get_keyindex(key);
	}
	if(idx!=-1){
		switch(idx){
			case ID:
				facebook_fill_atomic(json_object_to_json_string(obj),&(albuminfo->id));
				facebook_dbg("albuminfo->id is %s",albuminfo->id.data);
				break;
			case FROM:
				analy_each_from_info(obj, &(albuminfo->from));
				break;
			case NAME:
				facebook_fill_atomic(json_object_to_json_string(obj),&(albuminfo->name));
				facebook_dbg("albuminfo->name is %s",albuminfo->name.data);
				break;
			case DESCRIPTION:
				facebook_fill_atomic(json_object_to_json_string(obj),&(albuminfo->description));
				facebook_dbg("albuminfo->description is %s",albuminfo->description.data);
				break;
			case LINK:
				facebook_fill_atomic(json_object_to_json_string(obj),&(albuminfo->link));
				facebook_dbg("albuminfo->link is %s",albuminfo->link.data);
				break;
			case COVER_PHOTO:
				facebook_fill_atomic(json_object_to_json_string(obj),&(albuminfo->cover_photo));
				facebook_dbg("albuminfo->cover_photo is %s",albuminfo->cover_photo.data);
				break;
			case PRIVACY:
				facebook_fill_atomic(json_object_to_json_string(obj),&(albuminfo->privacy));
				facebook_dbg("albuminfo->privacy is %s",albuminfo->privacy.data);
				break;
			case COUNT:
				albuminfo->count = json_object_get_int(obj);
				facebook_dbg("albuminfo->count is %d",albuminfo->count);
				break;
			case TYPE:
				facebook_fill_atomic(json_object_to_json_string(obj),&(albuminfo->type));
				facebook_dbg("albuminfo->type is %s",albuminfo->type.data);
				break;
			case CREATED_TIME:
				facebook_fill_atomic(json_object_to_json_string(obj),&(albuminfo->created_time));
				facebook_dbg("albuminfo->created_time is %s",albuminfo->created_time.data);
				break;
			case UPDATED_TIME:
				facebook_fill_atomic(json_object_to_json_string(obj),&(albuminfo->updated_time));
				facebook_dbg("albuminfo->updated_time is %s",albuminfo->updated_time.data);
				break;
			case COMMENTS:
				analy_comment_info(obj,&(albuminfo->comments));
				break;
		}
	}
	else
		rtn = -1;
	return rtn;
}

int analy_each_album_info(facebook_album * albuminfo, json_object * each_obj)
{
	json_object_object_foreach(each_obj, key, val){
		//facebook_dbg("\t%s: %s\n", key, json_object_to_json_string(val));
		facebook_fill_albumelement(key,val,albuminfo);
        }
	return 0;
}

facebook_photoalbums * analy_albums_info(char * str)
{
	json_object *new_obj=NULL,*array_obj=NULL,*each_obj=NULL;
	int i;
	int rtn = 0;

	//facebook_dbg("the string is \n%s\n", str);
	new_obj = json_tokener_parse(str);
	array_obj = json_object_object_get(new_obj, "data");

	facebook_photoalbums * photoalbums = NULL;
	photoalbums = (facebook_photoalbums *)facebook_malloc(sizeof(facebook_photoalbums));
	if(photoalbums==NULL){
		facebook_dbg("photoalbums malloc fail!");
		rtn = -1;
		goto analy_albums_info_out;
	}
	
	//facebook_dbg("new_obj.to_string()=\n%s\n", json_object_to_json_string(new_obj));
	//facebook_dbg("val_obj.to_string()=\n%s\n", json_object_to_json_string(array_obj));
	photoalbums->album_num = json_object_array_length(array_obj);
	facebook_dbg("this user has %d album(s)",photoalbums->album_num);
	if(photoalbums->album_num == 0){
		facebook_dbg("this user has no album");
	}
	else{
		photoalbums->album_entry = (facebook_album *)facebook_malloc(sizeof(facebook_album)*photoalbums->album_num);
		if(photoalbums->album_entry == NULL){
			facebook_dbg("photoalbums->album_entry malloc fail!");
			rtn = -1;
			goto analy_albums_info_out;
		}
		for(i=0; i < photoalbums->album_num; i++){
			each_obj = json_object_array_get_idx(array_obj, i);
			//facebook_dbg("\n[%d]=%s\n", i, json_object_to_json_string(each_obj));
			analy_each_album_info(photoalbums->album_entry+i,each_obj);
		}
	}
analy_albums_info_out:
	if(new_obj){
		facebook_dbg("free json object");
		json_object_put(new_obj);
	}
	if(array_obj){
		facebook_dbg("free json object");
		json_object_put(array_obj);
	}
	if(each_obj){
		facebook_dbg("free json object");
		json_object_put(each_obj);
	}
	if(rtn == -1){
		if(photoalbums){
			facebook_free_photoalbums(photoalbums);
		}
		return NULL;
	}
	return photoalbums;
}

facebook_photoalbums * get_albums_info(facebook_data * f_data, char * id)
{
	char album_info[200] = {0};
	CURLcode curlcode=0;
	long m_http_code=0;
	facebook_photoalbums * photoalbums = NULL;
	/*FILE * fp = NULL;

	fp = fopen("/mnt/udisk/album_info.txt","wb+");
	if(fp==NULL){
		facebook_dbg("open album_info.txt error!");
		return NULL;
	}*/
	facebook_write_data * albums_info_data = (facebook_write_data *)facebook_malloc(sizeof(facebook_write_data));
	if(albums_info_data==NULL){
		facebook_dbg("albums_info_data malloc failed");
		return NULL;
	}
	facebook_data_write_init(albums_info_data,Facebook_data_write_buffer,NULL);

	sprintf(album_info,"https://graph.facebook.com/%s/albums?%s",id,f_data->access_token);
	facebook_dbg("URL is %s, len is %d",album_info, strlen(album_info));
	
	curl_easy_setopt(f_data->curl, CURLOPT_URL,album_info);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEDATA,albums_info_data);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEFUNCTION,facebook_func_write_data);
	
	curlcode = curl_easy_perform (f_data->curl);
	curl_easy_getinfo(f_data->curl, CURLINFO_RESPONSE_CODE, &m_http_code);
	facebook_dbg("curlcode is %d",curlcode);
	facebook_dbg("m_http_code is %ld",m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		facebook_dbg("Connect Success");
	}
	else{
		facebook_dbg("Connect Error");
	}
	/*if(fp){
		fclose(fp);
		fp = NULL;
	}*/
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		photoalbums = analy_albums_info(albums_info_data->data_head);
	}
	if(albums_info_data){
		facebook_data_write_free(albums_info_data);
		facebook_free(albums_info_data);
		albums_info_data = NULL;
	}
	return photoalbums;
}

EXPORT_SYMBOL
facebook_photoalbums * facebook_get_albums_info(facebook_data *f_data, facebook_contact *contact, int which_friend)
{
	if(contact==NULL){
		return get_albums_info(f_data, "me");
	}
	else{
		return get_albums_info(f_data, (contact->member_entry+which_friend)->id.data);
	}
}

char * get_album_id(facebook_album * albuminfo)
{
	facebook_dbg("id is %s",albuminfo->id.data);
	return albuminfo->id.data;
}

int get_album_cover(CURL * curl_p, facebook_data * f_data, facebook_album * albuminfo)
{
	char album_cover[200] = {0};
	char cover_addr[200] = {0};
	CURLcode curlcode=0;
	long m_http_code=0;
	int rtn = 0;

	facebook_dbg("this album has %d photos",albuminfo->count);
	if(albuminfo->count==0){
		facebook_dbg("this album has no photo");
		return get_status(FACEBOOK_RTN_ERR_DOWNLOADPHOTO,0,0);
	}

	sprintf(album_cover,"https://graph.facebook.com/%s/picture?%s",albuminfo->id.data,f_data->access_token);
	facebook_dbg("URL is %s, len is %d",album_cover, strlen(album_cover));
	curl_easy_setopt(curl_p, CURLOPT_URL,album_cover);
		
	curlcode = curl_easy_perform(curl_p);
	curl_easy_getinfo(curl_p, CURLINFO_RESPONSE_CODE, &m_http_code);
	facebook_dbg("curlcode is %d",curlcode);
	facebook_dbg("m_http_code is %ld",m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		facebook_dbg("Connect Success");
	}
	else if(curlcode==CURLE_OK && m_http_code==HttpStatus_FOUND){
		memset(cover_addr,0,200);
		memcpy(cover_addr, url_reload_p, strlen(url_reload_p));
		rtn = get_picture(curl_p,cover_addr);
	}
	else{
		facebook_dbg("Connect Fail");
		rtn = get_status(FACEBOOK_RTN_ERR_DOWNLOADPHOTO,0,0);
	}
	return rtn;
}

int facebook_fill_imageelement(char * key, json_object * obj, facebook_picture * image)
{
	int idx=-1;
	int rtn = 0;
	if(key){
		idx = facebook_get_keyindex(key);
	}
	if(idx!=-1){
		switch(idx){
			case HEIGHT:
				image->height = json_object_get_int(obj);
				facebook_dbg("image->height is %d",image->height);
				break;
			case WIDTH:
				image->width = json_object_get_int(obj);
				facebook_dbg("image->width is %d",image->width);
				break;
			case SOURCE:
				facebook_fill_atomic(json_object_to_json_string(obj),&(image->source));
				facebook_dbg("image->source is %s",image->source.data);
				break;
		}
	}
	else
		rtn = -1;
	return rtn;
}

int analy_each_image_info(json_object * obj, facebook_picture * image)
{
	json_object_object_foreach(obj, key, val){
		//facebook_dbg("\t%s: %s\n", key, json_object_to_json_string(val));
		facebook_fill_imageelement(key,val,image);
    }
	return 0;
}

int analy_images_info(json_object * obj,facebook_picture * image)
{
	json_object * array_obj=NULL, *each_obj=NULL;
	int i;
	for(i=0; i < json_object_array_length(obj); i++){
		each_obj = json_object_array_get_idx(obj, i);
		//facebook_dbg("\n[%d]=%s\n", i, json_object_to_json_string(each_obj));
		analy_each_image_info(each_obj, image+i);
	}
	if(array_obj){
		facebook_dbg("free json object");
		json_object_put(array_obj);
	}
	if(each_obj){
		facebook_dbg("free json object");
		json_object_put(each_obj);
	}
	return 0;
}

int facebook_fill_photoelement(char * key, json_object * obj, facebook_photo * photoinfo)
{
	int idx=-1;
	int rtn = 0;
	if(key){
		idx = facebook_get_keyindex(key);
	}
	if(idx!=-1){
		switch(idx){
			case ID:
				facebook_fill_atomic(json_object_to_json_string(obj),&(photoinfo->id));
				facebook_dbg("photoinfo->id is %s",photoinfo->id.data);
				break;
			case FROM:
				analy_each_from_info(obj, &(photoinfo->from));
				break;
			case NAME:
				facebook_fill_atomic(json_object_to_json_string(obj),&(photoinfo->name));
				facebook_dbg("photoinfo->name is %s",photoinfo->name.data);
				break;
			case PICTURE:
				facebook_fill_atomic(json_object_to_json_string(obj),&(photoinfo->picture));
				facebook_dbg("photoinfo->picture is %s",photoinfo->picture.data);
				break;
			case SOURCE:
				facebook_fill_atomic(json_object_to_json_string(obj),&(photoinfo->source));
				facebook_dbg("photoinfo->source is %s",photoinfo->source.data);
				break;
			case HEIGHT:
				photoinfo->height = json_object_get_int(obj);
				facebook_dbg("photoinfo->height is %d",photoinfo->height);
				break;
			case WIDTH:
				photoinfo->width = json_object_get_int(obj);
				facebook_dbg("photoinfo->width is %d",photoinfo->width);
				break;
			case IMAGES:
				analy_images_info(obj,photoinfo->images);
				break;
			case LINK:
				facebook_fill_atomic(json_object_to_json_string(obj),&(photoinfo->link));
				facebook_dbg("photoinfo->link is %s",photoinfo->link.data);
				break;
			case ICON:
				facebook_fill_atomic(json_object_to_json_string(obj),&(photoinfo->icon));
				facebook_dbg("photoinfo->icon is %s",photoinfo->icon.data);
				break;
			case CREATED_TIME:
				facebook_fill_atomic(json_object_to_json_string(obj),&(photoinfo->created_time));
				facebook_dbg("photoinfo->created_time is %s",photoinfo->created_time.data);
				break;
			case UPDATED_TIME:
				facebook_fill_atomic(json_object_to_json_string(obj),&(photoinfo->updated_time));
				facebook_dbg("photoinfo->updated_time is %s",photoinfo->updated_time.data);
				break;
			case COMMENTS:
				analy_comment_info(obj,&(photoinfo->comments));
				break;
		}
	}
	else
		rtn = -1;
	return rtn;
}

int analy_each_photo_info(facebook_photo * photoinfo, json_object * each_obj)
{
	json_object_object_foreach(each_obj, key, val){
		//facebook_dbg("\t%s: %s\n", key, json_object_to_json_string(val));
		facebook_fill_photoelement(key,val,photoinfo);
        }
	return 0;
}

int analy_photo_info(facebook_photo * photo_entry, char * str)
{
	json_object *new_obj=NULL,*array_obj=NULL,*each_obj=NULL;
	int i;
	int array_length = 0;

	//facebook_dbg("the string is \n%s\n", str);
	new_obj = json_tokener_parse(str);
	array_obj = json_object_object_get(new_obj, "data");
	array_length = json_object_array_length(array_obj);
	facebook_dbg("the array length is %d",array_length);
	for(i=0; i < array_length; i++){
		each_obj = json_object_array_get_idx(array_obj, i);
		//facebook_dbg("\n[%d]=%s\n", i, json_object_to_json_string(each_obj));
		analy_each_photo_info(photo_entry+i,each_obj);
	}
	if(new_obj){
		facebook_dbg("free json object");
		json_object_put(new_obj);
	}
	if(array_obj){
		facebook_dbg("free json object");
		json_object_put(array_obj);
	}
	if(each_obj){
		facebook_dbg("free json object");
		json_object_put(each_obj);
	}
	return array_length;
}

EXPORT_SYMBOL
int get_albumphoto_info(facebook_data * f_data, int index, facebook_photoalbums * photoalbums)
{
	char albumphoto_info[200] = {0};
	CURLcode curlcode=0;
	long m_http_code=0;
	int rtn = 0;
	int photo_total = 0;
	
	facebook_write_data * albumphoto_info_data = (facebook_write_data *)facebook_malloc(sizeof(facebook_write_data));
	if(albumphoto_info_data==NULL){
		facebook_dbg("albumphoto_info_data malloc failed");
		return -1;
	}
	facebook_data_write_init(albumphoto_info_data,Facebook_data_write_buffer,NULL);

	char * album_id = get_album_id(photoalbums->album_entry+index);
	sprintf(albumphoto_info,"https://graph.facebook.com/%s/photos?%s&limit=%d&offset=0",album_id, f_data->access_token,(photoalbums->album_entry+index)->count);
	facebook_dbg("URL is %s, len is %d",albumphoto_info,strlen(albumphoto_info));
	
	curl_easy_setopt(f_data->curl, CURLOPT_URL,albumphoto_info);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEDATA,albumphoto_info_data);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEFUNCTION,facebook_func_write_data);
		
	curlcode = curl_easy_perform (f_data->curl);
	curl_easy_getinfo(f_data->curl, CURLINFO_RESPONSE_CODE, &m_http_code);
	facebook_dbg("curlcode is %d",curlcode);
	facebook_dbg("m_http_code is %ld",m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		facebook_dbg("Connect Success");
	}
	else{
		facebook_dbg("Connect Error");
		rtn = -1;
	}
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		facebook_dbg("this album has %d photos",(photoalbums->album_entry+index)->count);
		if((photoalbums->album_entry+index)->count== 0){
			facebook_dbg("this album has no photo");
		}
		else{
			(photoalbums->album_entry+index)->photo_entry = (facebook_photo *)facebook_malloc((photoalbums->album_entry+index)->count * sizeof(facebook_photo));
			if((photoalbums->album_entry+index)->photo_entry){
				photo_total = analy_photo_info((photoalbums->album_entry+index)->photo_entry, albumphoto_info_data->data_head);
				if(photo_total != (photoalbums->album_entry+index)->count){
					facebook_dbg("the photo actually num is %d",photo_total);
					(photoalbums->album_entry+index)->count = photo_total;
				}
			}
			else{
				facebook_dbg("(photoalbums->album_entry+index)->photo_entry is malloc error!");
				rtn = -1;
			}
		}
	}
	if(albumphoto_info_data){
		facebook_data_write_free(albumphoto_info_data);
		facebook_free(albumphoto_info_data);
		albumphoto_info_data= NULL;
	}
	return rtn;
}

int download_photo_big(CURL * curl_p, facebook_album * album_entry, int index)
{
	facebook_dbg("album entry is 0x%x,photo index is %d",album_entry,index);
	facebook_dbg("this album has %d photos",album_entry->count);
	if(album_entry->count==0){
		facebook_dbg("this album has no photo");
		return get_status(FACEBOOK_RTN_ERR_DOWNLOADPHOTO,0,0);
	}
	if(index<0 || index>(album_entry->count-1)){
		facebook_dbg("index should be in [0,%d]",album_entry->count-1);
		return get_status(FACEBOOK_RTN_ERR_DOWNLOADPHOTO,0,0);
	}
	facebook_dbg("download url is %s",(album_entry->photo_entry+index)->images->source.data);
	return get_picture(curl_p,(album_entry->photo_entry+index)->images->source.data);
}

int download_photo_small(CURL * curl_p, facebook_album * album_entry, int index)
{
	facebook_dbg("album entry is 0x%x,index is %d",album_entry,index);
	facebook_dbg("this album has %d photos",album_entry->count);
	if(album_entry->count==0){
		facebook_dbg("this album has no photo");
		return get_status(FACEBOOK_RTN_ERR_DOWNLOADPHOTO,0,0);
	}
	if(index<0 || index>album_entry->count-1){
		facebook_dbg("index should be in [0,%d]",album_entry->count-1);
		return get_status(FACEBOOK_RTN_ERR_DOWNLOADPHOTO,0,0);
	}
	facebook_dbg("download url is %s",((album_entry->photo_entry+index)->images+3)->source.data);
	return get_picture(curl_p,((album_entry->photo_entry+index)->images+3)->source.data);
}

int facebook_fill_userelement(char * key, json_object * obj,facebook_user * userinfo)
{
	int idx=-1;
	int rtn = 0;
	if(key){
		idx = facebook_get_keyindex(key);
	}
	if(idx!=-1){
		switch(idx){
			case ID:
				facebook_fill_atomic(json_object_to_json_string(obj),&(userinfo->id));
				facebook_dbg("userinfo->id is %s",userinfo->id.data);
				break;
			case NAME:
				facebook_fill_atomic(json_object_to_json_string(obj),&(userinfo->name));
				facebook_dbg("userinfo->name is %s",userinfo->name.data);
				break;
			case FIRST_NAME:
				facebook_fill_atomic(json_object_to_json_string(obj),&(userinfo->firstname));
				facebook_dbg("userinfo->firstname is %s",userinfo->firstname.data);
				break;
			case LAST_NAME:
				facebook_fill_atomic(json_object_to_json_string(obj),&(userinfo->lastname));
				facebook_dbg("userinfo->lastname is %s",userinfo->lastname.data);
				break;
			case LINK:
				facebook_fill_atomic(json_object_to_json_string(obj),&(userinfo->link));
				facebook_dbg("userinfo->link is %s",userinfo->link.data);
				break;
			case GENDER:
				facebook_fill_atomic(json_object_to_json_string(obj),&(userinfo->gender));
				facebook_dbg("userinfo->gender is %s",userinfo->gender.data);
				break;
			case THIRD_PARTY_ID:
				facebook_fill_atomic(json_object_to_json_string(obj),&(userinfo->third_party_id));
				facebook_dbg("userinfo->third_party_id is %s",userinfo->third_party_id.data);
				break;
			case EMAIL:
				facebook_fill_atomic(json_object_to_json_string(obj),&(userinfo->email));
				facebook_dbg("userinfo->email is %s",userinfo->email.data);
				break;
			case TIMEZONE:
				userinfo->timezone = json_object_get_int(obj);
				facebook_dbg("userinfo->timezone is %d",userinfo->timezone);
				break;
			case LOCALE:
				facebook_fill_atomic(json_object_to_json_string(obj),&(userinfo->locale));
				facebook_dbg("userinfo->locale is %s",userinfo->locale.data);
				break;
			case VERIFIED:
				userinfo->verified = json_object_get_boolean(obj);
				facebook_dbg("userinfo->verified is %d",userinfo->verified);
				break;
			case UPDATED_TIME:
				facebook_fill_atomic(json_object_to_json_string(obj),&(userinfo->updated_time));
				facebook_dbg("userinfo->updated_time is %s",userinfo->updated_time.data);
				break;
			case ABOUT:
				facebook_fill_atomic(json_object_to_json_string(obj),&(userinfo->about));
				facebook_dbg("userinfo->about is %s",userinfo->about.data);
				break;
			case BIO:
				facebook_fill_atomic(json_object_to_json_string(obj),&(userinfo->bio));
				facebook_dbg("userinfo->bio is %s",userinfo->bio.data);
				break;
			case BIRTHDAY:
				facebook_fill_atomic(json_object_to_json_string(obj),&(userinfo->birthday));
				facebook_dbg("userinfo->birthday is %s",userinfo->birthday.data);
				break;
		}
	}
	else
		rtn = -1;
	return rtn;
}

facebook_user * analy_user_info(char * str)
{
	json_object *new_obj=NULL;
	facebook_user * userinfo = (facebook_user *)facebook_malloc(sizeof(facebook_user));
	if(userinfo==NULL){
		facebook_dbg("userinfo malloc fail!");
		return NULL;
	}
	//facebook_dbg("the string is \n%s\n", str);
	new_obj = json_tokener_parse(str);
	//facebook_dbg("new_obj.to_string()=\n%s\n", json_object_to_json_string(new_obj));
	json_object_object_foreach(new_obj, key, val){
		//facebook_dbg("\t%s: %s\n", key, json_object_to_json_string(val));
		facebook_fill_userelement(key,val,userinfo);
        }
	if(new_obj){
		facebook_dbg("free json object");
		json_object_put(new_obj);
	}
	return userinfo;
}

EXPORT_SYMBOL
facebook_user * get_user_info(facebook_data * f_data)
{
	char user_info[200] = {0};
	CURLcode curlcode=0;
	long m_http_code=0;
	int rtn = 0;
	facebook_user * userinfo = NULL;
	/*FILE * fp = NULL;

	fp = fopen("/mnt/udisk/user_info.txt","wb+");
	if(fp==NULL){
		facebook_dbg("open user_info.txt error!");
		return NULL;
	}*/
	facebook_write_data * user_info_data = (facebook_write_data *)facebook_malloc(sizeof(facebook_write_data));
	if(user_info_data==NULL){
		facebook_dbg("user_info_data malloc failed");
		return NULL;
	}
	facebook_data_write_init(user_info_data,Facebook_data_write_buffer,NULL);
	
	sprintf(user_info,"https://graph.facebook.com/me?%s",f_data->access_token);
	facebook_dbg("URL is %s, len is %d",user_info, strlen(user_info));
	
	curl_easy_setopt(f_data->curl, CURLOPT_URL,user_info);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEDATA,user_info_data);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEFUNCTION,facebook_func_write_data);
		
	curlcode = curl_easy_perform (f_data->curl);
	curl_easy_getinfo(f_data->curl, CURLINFO_RESPONSE_CODE, &m_http_code);
	facebook_dbg("curlcode is %d",curlcode);
	facebook_dbg("m_http_code is %ld",m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		facebook_dbg("Connect Success");
	}
	else{
		facebook_dbg("Connect Error");
	}
	/*if(fp){
		fclose(fp);
		fp = NULL;
	}*/
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		userinfo = analy_user_info(user_info_data->data_head);
	}
	if(user_info_data){
		facebook_data_write_free(user_info_data);
		facebook_free(user_info_data);
		user_info_data = NULL;
	}
	return userinfo;
}
char * get_profile_type(char * str)
{
	char *pic_type = NULL;
	pic_type = strrchr(str,'.');
	if(pic_type==NULL){
		facebook_dbg("profile type is error");
		return NULL;
	}
	else{
		int len = strlen(pic_type);
		char *buf = (char*)facebook_malloc(len+1);
		if(buf == NULL){
			facebook_dbg("get_profile_type buf malloc fail!");
			return NULL;
		}
		memcpy(buf, pic_type, len);
		buf[len] = 0;
		return buf;
	}
}

int get_user_picture(CURL * curl_p, facebook_data * f_data, facebook_user * userinfo)
{
	char user_pic[160] = {0};
	char profile_addr[200] = {0};
	CURLcode curlcode=0;
	long m_http_code=0;
	int rtn = 0;

	char *pic_type = NULL;

	sprintf(user_pic,"https://graph.facebook.com/me/picture?%s",f_data->access_token);
	facebook_dbg("URL is %s, len is %d",user_pic, strlen(user_pic));
	curl_easy_setopt(curl_p, CURLOPT_URL,user_pic);
		
	curlcode = curl_easy_perform(curl_p);
	curl_easy_getinfo(curl_p, CURLINFO_RESPONSE_CODE, &m_http_code);
	facebook_dbg("curlcode is %d",curlcode);
	facebook_dbg("m_http_code is %ld",m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		facebook_dbg("Connect Success");
	}
	else if(curlcode==CURLE_OK && m_http_code==HttpStatus_FOUND){
		facebook_dbg("Need to relocate");
		memset(profile_addr,0,200);
		memcpy(profile_addr, url_reload_p, strlen(url_reload_p));
		pic_type = get_profile_type(profile_addr);
		if(pic_type){
			rtn = get_picture(curl_p, profile_addr);
		}
		else{
			rtn = -1;
		}
	}
	else{
		facebook_dbg("Connect Fail");
		rtn = -1;
	}
	if(pic_type){
		facebook_free(pic_type);
		pic_type=NULL;
	}
	return rtn;
}

int get_friend_profile(CURL * curl_p, facebook_data * f_data, char * id)
{
	char user_pic[160] = {0};
	char profile_addr[200] = {0};
	CURLcode curlcode=0;
	long m_http_code=0;
	int rtn = 0;

	char *pic_type = NULL;

	sprintf(user_pic,"https://graph.facebook.com/%s/picture?%s",id,f_data->access_token);
	facebook_dbg("URL is %s, len is %d",user_pic, strlen(user_pic));
	curl_easy_setopt(curl_p, CURLOPT_URL,user_pic);
		
	curlcode = curl_easy_perform(curl_p);
	curl_easy_getinfo(curl_p, CURLINFO_RESPONSE_CODE, &m_http_code);
	facebook_dbg("curlcode is %d",curlcode);
	facebook_dbg("m_http_code is %ld",m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		facebook_dbg("Connect Success");
	}
	else if(curlcode==CURLE_OK && m_http_code==HttpStatus_FOUND){
		facebook_dbg("Need to relocate");
		memset(profile_addr,0,200);
		memcpy(profile_addr, url_reload_p, strlen(url_reload_p));
		pic_type = get_profile_type(profile_addr);
		if(pic_type){
			rtn = get_picture(curl_p, profile_addr);
		}
		else{
			rtn = -1;
		}
	}
	else{
		facebook_dbg("Connect Fail");
		rtn = -1;
	}
	if(pic_type){
		facebook_free(pic_type);
		pic_type=NULL;
	}
	return rtn;
}

int facebook_fill_friendlistelement(char * key, json_object * obj, facebook_friendlist * friendlist)
{
	int idx=-1;
	int rtn = 0;
	if(key){
		idx = facebook_get_keyindex(key);
	}
	if(idx!=-1){
		switch(idx){
			case ID:
				facebook_fill_atomic(json_object_to_json_string(obj),&(friendlist->id));
				facebook_dbg("friendlist->id is %s",friendlist->id.data);
				break;
			case NAME:
				facebook_fill_atomic(json_object_to_json_string(obj),&(friendlist->name));
				facebook_dbg("friendlist->name is %s",friendlist->name.data);
				break;
		}
	}
	else
		rtn = -1;
	return rtn;
}

int analy_each_friendlist(facebook_friendlist * friendlist, json_object * obj)
{
	json_object_object_foreach(obj, key, val){
		//facebook_dbg("\t%s: %s\n", key, json_object_to_json_string(val));
		facebook_fill_friendlistelement(key,val,friendlist);
        }
	return 0;
}

facebook_friendlists * analy_friends_list(char * str)
{
	json_object *new_obj=NULL, *array_obj=NULL, *each_obj=NULL;
	int i;
	int rtn = 0;
	//facebook_dbg("the string is \n%s\n", str);
	new_obj = json_tokener_parse(str);
	//facebook_dbg("new_obj.to_string()=\n%s\n", json_object_to_json_string(new_obj));
	array_obj = json_object_object_get(new_obj, "data");
	//facebook_dbg("array_obj.to_string()=\n%s\n", json_object_to_json_string(array_obj));

	facebook_friendlists * friendlists = NULL;
	friendlists = (facebook_friendlists *)facebook_malloc(sizeof(facebook_friendlists));
	if(friendlists==NULL){
		facebook_dbg("friendlists malloc fail!");
		rtn = -1;
		goto analy_friends_list_out;
	}

	friendlists->friendlist_num = json_object_array_length(array_obj);
	facebook_dbg("this user has %d friendlist(s)",friendlists->friendlist_num);
	if(friendlists->friendlist_num == 0){
		facebook_dbg("this user has no friendlist");
	}
	else{
		friendlists->friendlist_entry = (facebook_friendlist *)facebook_malloc(sizeof(facebook_friendlist)*friendlists->friendlist_num);
		if(friendlists->friendlist_entry == NULL){
			facebook_dbg("friendlists->friendlist_entry malloc fail!");
			rtn = -1;
			goto analy_friends_list_out;
		}
		for(i=0; i < friendlists->friendlist_num; i++){
			each_obj = json_object_array_get_idx(array_obj, i);
			//facebook_dbg("\n[%d]=%s\n", i, json_object_to_json_string(each_obj));
			analy_each_friendlist(friendlists->friendlist_entry+i,each_obj);
		}
	}
analy_friends_list_out:
	if(new_obj){
		facebook_dbg("free json object");
		json_object_put(new_obj);
	}
	if(array_obj){
		facebook_dbg("free json object");
		json_object_put(array_obj);
	}
	if(each_obj){
		facebook_dbg("free json object");
		json_object_put(each_obj);
	}
	if(rtn == -1){
		if(friendlists){
			facebook_free_friendlists(friendlists);
			facebook_free(friendlists);
			friendlists = NULL;
			return NULL;
		}
	}
	return friendlists;
}

int facebook_fill_memberelement(char * key, json_object * obj, facebook_member * member)
{
	int idx=-1;
	int rtn = 0;
	if(key){
		idx = facebook_get_keyindex(key);
	}
	if(idx!=-1){
		switch(idx){
			case ID:
				facebook_fill_atomic(json_object_to_json_string(obj),&(member->id));
				facebook_dbg("member->id is %s",member->id.data);
				break;
			case NAME:
				facebook_fill_atomic(json_object_to_json_string(obj),&(member->name));
				facebook_dbg("member->name is %s",member->name.data);
				break;
		}
	}
	else
		rtn = -1;
	return rtn;
}

int analy_each_member(facebook_member * member, json_object *obj)
{
	json_object_object_foreach(obj, key, val){
		//facebook_dbg("\t%s: %s\n", key, json_object_to_json_string(val));
		facebook_fill_memberelement(key,val,member);
        }
	return 0;
}

int analy_listmembers(facebook_friendlist * friendlist_entry, char * str)
{
	json_object *new_obj=NULL, *array_obj=NULL, *each_obj=NULL;
	int i;
	//facebook_dbg("the string is \n%s\n", str);
	new_obj = json_tokener_parse(str);
	//facebook_dbg("new_obj.to_string()=\n%s\n", json_object_to_json_string(new_obj));
	array_obj = json_object_object_get(new_obj, "data");
	//facebook_dbg("array_obj.to_string()=\n%s\n", json_object_to_json_string(array_obj));

	friendlist_entry->member_num = json_object_array_length(array_obj);
	facebook_dbg("this friendlist has %d member(s)",friendlist_entry->member_num);
	if(friendlist_entry->member_num == 0){
		facebook_dbg("this friendlist has no member");
	}
	else{
		friendlist_entry->member_entry = (facebook_member *)facebook_malloc(sizeof(facebook_member)*friendlist_entry->member_num);
		if(friendlist_entry->member_entry == NULL){
			facebook_dbg("friendlist_entry->member_entry malloc fail!");
			return -1;
		}

		for(i=0; i < json_object_array_length(array_obj); i++){
			each_obj = json_object_array_get_idx(array_obj, i);
			//facebook_dbg("\n[%d]=%s\n", i, json_object_to_json_string(each_obj));
			analy_each_member(friendlist_entry->member_entry+i,each_obj);
		}
	}
	if(new_obj){
		facebook_dbg("free json object");
		json_object_put(new_obj);
	}
	if(array_obj){
		facebook_dbg("free json object");
		json_object_put(array_obj);
	}
	if(each_obj){
		facebook_dbg("free json object");
		json_object_put(each_obj);
	}
	return 0;
}

EXPORT_SYMBOL
int get_listmembers_info(facebook_data * f_data, facebook_friendlist * friendlist_entry)
{
	char list_members[200] = {0};
	CURLcode curlcode=0;
	long m_http_code=0;
	int rtn = 0;
	/*FILE * fp = NULL;

	fp = fopen("/mnt/udisk/friend_members.txt","wb+");
	if(fp==NULL){
		facebook_dbg("open friend_members.txt error!");
		return -1;
	}*/
	facebook_write_data * list_members_data = (facebook_write_data *)facebook_malloc(sizeof(facebook_write_data));
	if(list_members_data==NULL){
		facebook_dbg("list_members_data malloc failed");
		return -1;
	}
	facebook_data_write_init(list_members_data,Facebook_data_write_buffer,NULL);

	sprintf(list_members,"https://graph.facebook.com/%s/members?%s",friendlist_entry->id.data, f_data->access_token);
	facebook_dbg("URL is %s, len is %d",list_members, strlen(list_members));
	
	curl_easy_setopt(f_data->curl, CURLOPT_URL,list_members);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEDATA,list_members_data);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEFUNCTION,facebook_func_write_data);

	curlcode = curl_easy_perform (f_data->curl);
	curl_easy_getinfo(f_data->curl, CURLINFO_RESPONSE_CODE, &m_http_code);
	facebook_dbg("curlcode is %d",curlcode);
	facebook_dbg("m_http_code is %ld",m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		facebook_dbg("Connect Success");
	}
	else{
		facebook_dbg("Connect Fail");
		rtn = -1;
	}
	/*if(fp){
		fclose(fp);
		fp = NULL;
	}*/
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		analy_listmembers(friendlist_entry,list_members_data->data_head);
	}
	if(list_members_data){
		facebook_data_write_free(list_members_data);
		facebook_free(list_members_data);
		list_members_data = NULL;
	}
	return rtn;
}

EXPORT_SYMBOL
facebook_friendlists * get_friends_list(facebook_data * f_data)
{
	char friends_list[160] = {0};
	CURLcode curlcode=0;
	long m_http_code=0;
	int rtn = 0;
	int i;
	facebook_friendlists * friendlists = NULL;
	/*FILE * fp = NULL;

	fp = fopen("/mnt/udisk/friend_list.txt","wb+");
	if(fp==NULL){
		facebook_dbg("open friend_list.txt error!");
		return -1;
	}*/
	facebook_write_data * friendslist_data = (facebook_write_data *)facebook_malloc(sizeof(facebook_write_data));
	if(friendslist_data==NULL){
		facebook_dbg("friendslist_data malloc failed");
		return NULL;
	}
	facebook_data_write_init(friendslist_data,Facebook_data_write_buffer,NULL);

	sprintf(friends_list,"https://graph.facebook.com/me/friendlists?%s", f_data->access_token);
	facebook_dbg("URL is %s, len is %d",friends_list, strlen(friends_list));
	
	curl_easy_setopt(f_data->curl,CURLOPT_URL,friends_list);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEDATA,friendslist_data);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEFUNCTION,facebook_func_write_data);

	curlcode = curl_easy_perform (f_data->curl);
	curl_easy_getinfo(f_data->curl, CURLINFO_RESPONSE_CODE, &m_http_code);
	facebook_dbg("curlcode is %d",curlcode);
	facebook_dbg("m_http_code is %ld",m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		facebook_dbg("Connect Success");
	}
	else{
		facebook_dbg("Connect Fail");
	}
	/*if(fp){
		fclose(fp);
		fp = NULL;
	}*/
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		friendlists = analy_friends_list(friendslist_data->data_head);
	}
	if(friendslist_data){
		facebook_data_write_free(friendslist_data);
		facebook_free(friendslist_data);
		friendslist_data = NULL;
	}
	return friendlists;
}

facebook_contact * analy_contact(char * str)
{
	json_object *new_obj=NULL, *array_obj=NULL, *each_obj=NULL;
	int i;
	int rtn = 0;
	//facebook_dbg("the string is \n%s\n", str);
	new_obj = json_tokener_parse(str);
	array_obj = json_object_object_get(new_obj, "data");
	//facebook_dbg("array_obj.to_string()=\n%s\n", json_object_to_json_string(array_obj));

	facebook_contact * contact = NULL;
	contact = (facebook_contact *)facebook_malloc(sizeof(facebook_contact));
	if(contact==NULL){
		facebook_dbg("contact malloc fail!");
		rtn = -1;
		goto analy_friends_list_out;
	}

	contact->contact_num = json_object_array_length(array_obj);
	facebook_dbg("this user has %d friend(s)",contact->contact_num);
	if(contact->contact_num == 0){
		facebook_dbg("this user has no friend");
	}
	else{
		contact->member_entry = (facebook_member *)facebook_malloc(sizeof(facebook_member)*contact->contact_num);
		if(contact->member_entry == NULL){
			facebook_dbg("contact->member_entry malloc fail!");
			rtn = -1;
			goto analy_friends_list_out;
		}
		for(i=0; i < contact->contact_num; i++){
			each_obj = json_object_array_get_idx(array_obj, i);
			//facebook_dbg("\n[%d]=%s\n", i, json_object_to_json_string(each_obj));
			analy_each_member(contact->member_entry+i,each_obj);
		}
	}
analy_friends_list_out:
	if(new_obj){
		facebook_dbg("free json object");
		json_object_put(new_obj);
	}
	if(array_obj){
		facebook_dbg("free json object");
		json_object_put(array_obj);
	}
	if(each_obj){
		facebook_dbg("free json object");
		json_object_put(each_obj);
	}
	if(rtn == -1){
		if(contact){
			facebook_free_contact(contact);
			facebook_free(contact);
			contact = NULL;
			return NULL;
		}
	}
	return contact;
}

EXPORT_SYMBOL
facebook_contact * get_contact(facebook_data * f_data)
{
	char contact_url[200] = {0};
	CURLcode curlcode=0;
	long m_http_code=0;
	int rtn = 0;
	int i;
	facebook_contact * contact = NULL;
	/*FILE * fp = NULL;

	fp = fopen("/mnt/udisk/friend_list.txt","wb+");
	if(fp==NULL){
		facebook_dbg("open friend_list.txt error!");
		return -1;
	}*/
	facebook_write_data * contact_data = (facebook_write_data *)facebook_malloc(sizeof(facebook_write_data));
	if(contact_data==NULL){
		facebook_dbg("contact_data malloc failed");
		return NULL;
	}
	facebook_data_write_init(contact_data,Facebook_data_write_buffer,NULL);

	sprintf(contact_url,"https://graph.facebook.com/me/friends?%s", f_data->access_token);
	facebook_dbg("URL is %s, len is %d",contact_url, strlen(contact_url));
	
	curl_easy_setopt(f_data->curl,CURLOPT_URL,contact_url);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEDATA,contact_data);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEFUNCTION,facebook_func_write_data);

	curlcode = curl_easy_perform (f_data->curl);
	curl_easy_getinfo(f_data->curl, CURLINFO_RESPONSE_CODE, &m_http_code);
	facebook_dbg("curlcode is %d",curlcode);
	facebook_dbg("m_http_code is %ld",m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		facebook_dbg("Connect Success");
	}
	else{
		facebook_dbg("Connect Fail");
	}
	/*if(fp){
		fclose(fp);
		fp = NULL;
	}*/
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		contact = analy_contact(contact_data->data_head);
	}
	if(contact_data){
		facebook_data_write_free(contact_data);
		facebook_free(contact_data);
		contact_data = NULL;
	}
	return contact;
}

int facebook_free_atomic(facebook_atomic * atomic)
{
	if(atomic==NULL){
		facebook_dbg("this atomic is NULL, no need to free");
		return 0;
	}
	if(atomic->data && atomic->data_len!=0){
		facebook_free(atomic->data);
		atomic->data = NULL;
		atomic->data_len = 0;
		return 0;
	}
	else if(atomic->data==NULL && atomic->data_len==0){
		return 0;
	}
	else{
		facebook_dbg("There is something wrong");
		return -1;
	}
}

int facebook_free_from(facebook_from * from)
{
	if(from==NULL){
		facebook_dbg("this from is NULL, no need to free");
		return 0;
	}
	facebook_free_atomic(&from->name);
	facebook_free_atomic(&from->id);
	return 0;
}

int facebook_free_comments(facebook_comment *comments)
{
	if(comments==NULL){
		facebook_dbg("this comments is NULL, no need to free");
		return 0;
	}
	facebook_free_atomic(&comments->id);
	facebook_free_from(&comments->from);
	facebook_free_atomic(&comments->message);
	facebook_free_atomic(&comments->created_time);
	return 0;
}

int facebook_free_photoimage(facebook_picture * images)
{
	if(images==NULL){
		facebook_dbg("images is NULL, no need to free");
		return 0;
	}
	int i=0;
	for(i=0;i<5;i++){
		facebook_free_atomic(&(images+i)->source);
	}
	return 0;
}

int facebook_free_photo(facebook_photo * photo)
{
	if(photo==NULL){
		facebook_dbg("photo is NULL, no need to free");
		return 0;
	}
	facebook_free_atomic(&photo->id);
	facebook_free_from(&photo->from);
	facebook_free_atomic(&photo->name);
	facebook_free_atomic(&photo->picture);
	facebook_free_atomic(&photo->source);
	facebook_free_photoimage(photo->images);
	facebook_free_atomic(&photo->link);
	facebook_free_atomic(&photo->icon);
	facebook_free_atomic(&photo->created_time);
	facebook_free_atomic(&photo->updated_time);
	facebook_free_comments(&photo->comments);
	return 0;
}

int facebook_free_basicalbum(facebook_album * album)
{
	facebook_free_atomic(&album->id);
	facebook_free_from(&album->from);
	facebook_free_atomic(&album->name);
	facebook_free_atomic(&album->description);
	facebook_free_atomic(&album->link);
	facebook_free_atomic(&album->cover_photo);
	facebook_free_atomic(&album->privacy);
	facebook_free_atomic(&album->type);
	facebook_free_atomic(&album->created_time);
	facebook_free_atomic(&album->updated_time);
	facebook_free_comments(&album->comments);
	return 0;
}

EXPORT_SYMBOL
int facebook_free_album(facebook_album * album)
{
	int i = 0;
	if(album==NULL){
		facebook_dbg("album is NULL, no need to free");
		return 0;
	}
	if(album->photo_entry){
		for(i=0;i<album->count;i++){
			facebook_free_photo(album->photo_entry+i);
		}
		facebook_free(album->photo_entry);
		album->photo_entry = NULL;
	}
	return 0;
}

EXPORT_SYMBOL
int facebook_free_photoalbums(facebook_photoalbums *photoalbums)
{
	int i=0;
	if(photoalbums==NULL){
		facebook_dbg("photoalbums is NULL, no need to free");
		return 0;
	}
	if(photoalbums->album_entry){
		for(i=0;i<photoalbums->album_num;i++){
			facebook_free_basicalbum(photoalbums->album_entry+i);
			facebook_free_album(photoalbums->album_entry+i);
		}
		facebook_free(photoalbums->album_entry);
		photoalbums->album_entry = NULL;
	}
	return 0;
}

int facebook_free_memberlist(facebook_member *member)
{
	int i=0;
	if(member==NULL){
		facebook_dbg("member is NULL, no need to free");
		return 0;
	}
	facebook_free_atomic(&member->id);
	facebook_free_atomic(&member->name);
	return 0;
}

EXPORT_SYMBOL
int facebook_free_friendlist(facebook_friendlist *friendlist)
{
	int i=0;
	if(friendlist==NULL){
		facebook_dbg("friendlist is NULL, no need to free");
		return 0;
	}
	facebook_free_atomic(&friendlist->id);
	facebook_free_atomic(&friendlist->name);
	if(friendlist->member_entry){
		for(i=0;i<friendlist->member_num;i++){
			facebook_free_memberlist(friendlist->member_entry+i);
		}
		facebook_free(friendlist->member_entry);
		friendlist->member_entry = NULL;
	}
	return 0;
}

EXPORT_SYMBOL
int facebook_free_friendlists(facebook_friendlists *friendlists)
{
	int i=0;
	if(friendlists==NULL){
		facebook_dbg("friendlists is NULL, no need to free");
		return 0;
	}
	if(friendlists->friendlist_entry){
		for(i=0;i<friendlists->friendlist_num;i++){
			facebook_free_friendlist(friendlists->friendlist_entry+i);
		}
		facebook_free(friendlists->friendlist_entry);
		friendlists->friendlist_entry = NULL;
	}
	return 0;
}

EXPORT_SYMBOL
int facebook_free_contact(facebook_contact *contact)
{
	int i=0;
	if(contact==NULL){
		facebook_dbg("contact is NULL, no need to free");
		return 0;
	}
	if(contact->member_entry){
		for(i=0;i<contact->contact_num;i++){
			facebook_free_memberlist(contact->member_entry+i);
		}
		facebook_free(contact->member_entry);
		contact->member_entry = NULL;
	}
	return 0;
}

EXPORT_SYMBOL
int facebook_free_userinfo(facebook_user *userinfo)
{
	if(userinfo==NULL){
		facebook_dbg("userinfo is NULL, no need to free");
		return 0;
	}
	facebook_free_atomic(&userinfo->id);
	facebook_free_atomic(&userinfo->name);
	facebook_free_atomic(&userinfo->firstname);
	facebook_free_atomic(&userinfo->lastname);
	facebook_free_atomic(&userinfo->link);
	facebook_free_atomic(&userinfo->gender);
	facebook_free_atomic(&userinfo->third_party_id);
	facebook_free_atomic(&userinfo->email);
	facebook_free_atomic(&userinfo->locale);
	facebook_free_atomic(&userinfo->updated_time);
	facebook_free_atomic(&userinfo->about);
	facebook_free_atomic(&userinfo->bio);
	facebook_free_atomic(&userinfo->birthday);
	return 0;
}

int facebook_data_write_init(facebook_write_data *data,FacebookDataWrite_e data_type,void* fp)
{
	memset(data,0,sizeof(facebook_write_data));
	if(data_type==Facebook_data_write_file){
		data->file_handle = fp;
		data->data_type = Facebook_data_write_file;
	}
	else
		data->data_type = Facebook_data_write_buffer;
	return 0;
}

int facebook_data_write_free(facebook_write_data *data)
{
	if(data->data_head && data->data_type==Facebook_data_write_buffer){
		facebook_free(data->data_head);
		data->data_head=NULL;
		data->data_cur=NULL;
		data->data_len = 0;
		data->data_used = 0;
	}
	return 0;
}

char * facebook_get_emailaddr()
{
	return facebook_login_email;
}

int facebook_set_emailaddr(const char* email)
{
	int str_len = strlen(email);
	if(str_len>=LOGIN_MAX_LEN){
		facebook_dbg("Error Email Lenth is to long, len=%d,MAX_LEN=%d\n",str_len,LOGIN_MAX_LEN);
		return -1;
	}
	strcpy(facebook_login_email,email);
	return 0;
	
}

char * facebook_get_pwd()
{
	return facebook_login_pwd;
}

int facebook_set_pwd(const char* password)
{
	int str_len = strlen(password);
	if(str_len>=LOGIN_MAX_LEN){
		facebook_dbg("Error PWD Lenth is to long, len=%d,MAX_LEN=%d\n",str_len,LOGIN_MAX_LEN);
		return -1;
	}
	strcpy(facebook_login_pwd,password);
	return 0;
}

int debugFun(CURL* curl, curl_infotype type, char* str, size_t len, void* stream)
{
	char *reload = (char *)stream;
	char *locate = "Location: ";
	char *find = NULL;
	int find_len = 0;
	int i = 0;
	find = strstr(str, locate);
	if(find){
		if(reload){
			facebook_dbg("reload is not null");
			facebook_free(reload);
			reload = NULL;
		}
		find += 10;
		find_len = strlen(find);
		facebook_dbg("find_len is %d",find_len);
		//for(i=0;i<find_len;i++)
			//facebook_dbg("%d: 0x%x\t",i,find[i]);
		reload = (char *)facebook_malloc(find_len-1);
		if(reload==NULL){
			facebook_dbg("reload is null");
			return -1;
		}
		memcpy(reload, find, find_len-2);
		reload[find_len-2] = 0;
		url_reload_p = reload;
		facebook_dbg("reload is %s, len is %d",url_reload_p,strlen(url_reload_p));
		//for(i=0;i<strlen(url_reload_p);i++)
			//facebook_dbg("%d: 0x%x\t",i,url_reload_p[i]);
	}
	return 0;
}

int facebook_create_dir(char * dir_path)
{
	int rtn=0;
	rtn = access(dir_path,W_OK);
	if(rtn!=0){
		rtn = mkdir(dir_path,0777);
		if(rtn!=0){
			facebook_dbg("Make Dir Failed=%s\n",dir_path);
		}
	}
	return rtn;	
}

EXPORT_SYMBOL
int facebook_set_longin(const char* email, const char* pwd)
{
	if(facebook_set_emailaddr(email)!=0)
		return -1;
	if(facebook_set_pwd(pwd)!=0)
		return -1;
	return 0;
}

EXPORT_SYMBOL
int facebook_init_fdata(facebook_data *fdata)
{
	int rtn=0,tmpstr_len=0;
	char *tmpstr=NULL;
	memset(fdata,0,sizeof(facebook_data));

	tmpstr = facebook_get_emailaddr();
	tmpstr_len=strlen(tmpstr);
	memset(fdata->usermail,0,LOGIN_MAX_LEN);
	memcpy(fdata->usermail,tmpstr,tmpstr_len);
	facebook_dbg("user mail is %s",fdata->usermail);
	if(fdata->usermail==NULL){
		facebook_dbg("user mail is NULL!");
		rtn = -1;
		goto INIT_DATA_END;
	}

	tmpstr = facebook_get_pwd();
	tmpstr_len=strlen(tmpstr);
	memset(fdata->userpwd,0,LOGIN_MAX_LEN);
	memcpy(fdata->userpwd,tmpstr,tmpstr_len);
	facebook_dbg("user pwd is %s",fdata->userpwd);
	if(fdata->userpwd==NULL){
		facebook_dbg("user pwd is NULL!");
		rtn = -1;
		goto INIT_DATA_END;
	}
	
	fdata->access_token = NULL;
	url_reload_p = NULL;
	
	fdata->curl = curl_easy_init();
	if(fdata->curl != NULL){
		curl_easy_setopt(fdata->curl,CURLOPT_VERBOSE,1L);
		curl_easy_setopt(fdata->curl,CURLOPT_DEBUGDATA,url_reload_p);
		curl_easy_setopt(fdata->curl,CURLOPT_DEBUGFUNCTION, debugFun);
		curl_easy_setopt(fdata->curl,CURLOPT_SSL_VERIFYPEER,0);
		curl_easy_setopt(fdata->curl,CURLOPT_COOKIEJAR,"/mnt/vram/cookie.txt");
		curl_easy_setopt(fdata->curl,CURLOPT_TIMEOUT,CURL_TIME_OUT);
	}
	else{
		facebook_dbg("facebook curl is NULL!");
		rtn = -1;
		goto INIT_DATA_END;
	}

	if(facebook_create_thread(fdata)!=0){
		facebook_dbg("facebook create thread error!");
		rtn = -1;
		goto INIT_DATA_END;
	}
	facebook_req_init_queue();

	fdata->update_thread_id = -1;
INIT_DATA_END:
	facebook_dbg("Email=%s,PWD=%s\n",fdata->usermail,fdata->userpwd);

	if(rtn==-1){
		memset(fdata->usermail,0,LOGIN_MAX_LEN);
		memset(fdata->userpwd,0,LOGIN_MAX_LEN);
	}
	return rtn;
}

EXPORT_SYMBOL
int facebook_free_fdata(facebook_data * f_data)
{
	if(f_data==NULL){
		facebook_dbg("f_data is NULL, no need to free");
		return 0;
	}
	if(url_reload_p){
		facebook_free(url_reload_p);
		url_reload_p = NULL;
	}
	if(f_data->access_token){
		facebook_free(f_data->access_token);
		f_data->access_token = NULL;
	}
	if(f_data->curl){
		curl_easy_cleanup(f_data->curl);
	}
	facebook_thread_exit(f_data);
	facebook_exit_update(f_data);
	return 0;
}

EXPORT_SYMBOL
int facebook_authentication(facebook_data * f_data)
{
	int rtn = 0;
	rtn = facebook_auth(f_data);
	if(rtn==HttpStatus_FOUND){
		facebook_dbg("Authentication success!");
		curl_easy_setopt(f_data->curl,CURLOPT_HTTPGET,1L);
	}
	else{
		facebook_dbg("Authentication fail!");
	}
	return rtn;
}

EXPORT_SYMBOL
int facebook_query_auth_status(facebook_data * f_data, int query_cmd)
{
	int rtn = -1;
	rtn = facebook_req_query(f_data,FACEBOOK_CMD_AUTH,f_data);
	if(rtn>=0){
		if(f_data){
			if(query_cmd==FACEBOOK_QUERY_CMD_RESULT)
				return f_data->auth_status;
			else{
				facebook_dbg("Query Download Cmd Error cmd =%d",query_cmd);
				return -2;
			}
		}
		else 
			return -2;
	}
	else{
		return rtn;
	}
}

int facebook_check_file(char *filepath)
{
	FILE * fhandle=NULL;
	fhandle = fopen(filepath,"rb");
	if(fhandle){//the file is exit,do not need download
		facebook_dbg("%s exist",filepath);
		fclose(fhandle);
		return 1;
	}
	else{
		facebook_dbg("%s does not exist",filepath);
		return 0;
	}
}

EXPORT_SYMBOL
facebook_photo_down_info *facebook_init_download_info(int iscache,char* cache_dir,facebook_feed *feed,int which_entry,int isthumbnail)
{
	facebook_photo_down_info * down_info=NULL;
	char tmpbuf[128] = {0};
	facebook_album * album = NULL;
	facebook_member * member = NULL;
	down_info = (facebook_photo_down_info *)facebook_malloc(sizeof(facebook_photo_down_info));
	if(down_info==NULL){
		facebook_dbg("facebook_photo_down_info malloc error");
		return NULL;
	}
	memset(down_info,0,sizeof(facebook_photo_down_info));
	down_info->f_feed = feed;
	down_info->is_cache = iscache;
	down_info->which_entry = which_entry;
	down_info->isthumbnail = isthumbnail;
	down_info->photo_data = (facebook_write_data *)facebook_malloc(sizeof(facebook_write_data));
	if(down_info->photo_data == NULL){
		facebook_dbg("down_info->photo_data malloc failed");
		facebook_free_download_info(down_info);
		return NULL;
	}
	if(iscache){
		if(facebook_create_dir(cache_dir) != 0){
			facebook_dbg("create dir error");
			facebook_free_download_info(down_info);
			return NULL;
		}
		facebook_fill_atomic(cache_dir, &(down_info->cache_dir));
		facebook_dbg("down_info->cache_dir is %s",down_info->cache_dir.data);
		if(feed->type == ALBUM_FEED){
			if(which_entry<0 || which_entry>=((facebook_photoalbums *)feed->feed_p)->album_num){
				facebook_dbg("Entry Num is out of range MAX=%d,Your Value=%d",((facebook_photoalbums *)feed->feed_p)->album_num-1,which_entry);
				facebook_free_download_info(down_info);
				return NULL;
			}
			album = ((facebook_photoalbums *)feed->feed_p)->album_entry;
			album += which_entry;
			facebook_dbg("#######0x%x",album);
			if(album->count == 0){
				facebook_dbg("this album has no photo");
				facebook_free_download_info(down_info);
				return NULL;
			}
			facebook_fill_atomic(album->id.data, &(down_info->album_id));
		}
		else if(feed->type == PHOTO_FEED){
			album = (facebook_album *)feed->feed_p;
			facebook_dbg("album is 0x%x",album);
			if(which_entry<0 || which_entry>=album->count){
				facebook_dbg("Entry Num is out of range MAX=%d,Your Value=%d",album->count-1,which_entry);
				facebook_free_download_info(down_info);
				return NULL;
			}
			facebook_fill_atomic(album->id.data, &(down_info->album_id));
		}
		else if(feed->type == USER_PROFILE){
			facebook_fill_atomic("user_profile", &(down_info->album_id));
		}
		else if(feed->type == CONTACT_FEED){
			if(which_entry<0 || which_entry>=((facebook_contact *)feed->feed_p)->contact_num){
				facebook_dbg("Entry Num is out of range MAX=%d,Your Value=%d",((facebook_contact *)feed->feed_p)->contact_num-1,which_entry);
				facebook_free_download_info(down_info);
				return NULL;
			}
			member = ((facebook_contact *)feed->feed_p)->member_entry;
			member += which_entry;
			facebook_fill_atomic("user_profile", &(down_info->album_id));
		}
		facebook_dbg("down_info->album_id is %s",down_info->album_id.data);
		/*memset(tmpbuf,0,sizeof(tmpbuf));
		sprintf(tmpbuf,"%s/%s",down_info->cache_dir.data, down_info->album_id.data);
		facebook_dbg("tmpbuf is %s",tmpbuf);
		if(facebook_create_dir(tmpbuf) != 0){
			facebook_dbg("create dir error---%s",tmpbuf);
			facebook_free_download_info(down_info);
			return NULL;
		}*/
		if(feed->type == ALBUM_FEED){
			if(isthumbnail){
				memset(tmpbuf,0,sizeof(tmpbuf));
				sprintf(tmpbuf,"%s/thumbnail",down_info->cache_dir.data);
				facebook_dbg("tmpbuf is %s",tmpbuf);
				if(facebook_create_dir(tmpbuf) != 0){
					facebook_dbg("create dir error---%s",tmpbuf);
					facebook_free_download_info(down_info);
					return NULL;
				}
				memset(tmpbuf,0,sizeof(tmpbuf));
				sprintf(tmpbuf,"%s/thumbnail/%s.jpg",down_info->cache_dir.data, down_info->album_id.data);
			}
			else{
				memset(tmpbuf,0,sizeof(tmpbuf));
				sprintf(tmpbuf,"%s/%s.jpg",down_info->cache_dir.data, down_info->album_id.data);
			}
		}
		else if(feed->type == PHOTO_FEED){
			memset(tmpbuf,0,sizeof(tmpbuf));
			sprintf(tmpbuf,"%s/%s",down_info->cache_dir.data, down_info->album_id.data);
			facebook_dbg("tmpbuf is %s",tmpbuf);
			if(facebook_create_dir(tmpbuf) != 0){
				facebook_dbg("create dir error---%s",tmpbuf);
				facebook_free_download_info(down_info);
				return NULL;
			}
			if(isthumbnail){
				memset(tmpbuf,0,sizeof(tmpbuf));
				sprintf(tmpbuf,"%s/%s/thumbnail",down_info->cache_dir.data, down_info->album_id.data);
				facebook_dbg("tmpbuf is %s",tmpbuf);
				if(facebook_create_dir(tmpbuf) != 0){
					facebook_dbg("create dir error---%s",tmpbuf);
					facebook_free_download_info(down_info);
					return NULL;
				}
				memset(tmpbuf,0,sizeof(tmpbuf));
				sprintf(tmpbuf,"%s/%s/thumbnail/%s.jpg",down_info->cache_dir.data, down_info->album_id.data, (album->photo_entry+which_entry)->id.data);
			}
			else{
				memset(tmpbuf,0,sizeof(tmpbuf));
				sprintf(tmpbuf,"%s/%s/%s.jpg",down_info->cache_dir.data, down_info->album_id.data, (album->photo_entry+which_entry)->id.data);
			}
		}
		else if(feed->type == USER_PROFILE){
			sprintf(tmpbuf,"%s/%s/user_profile.jpg",down_info->cache_dir.data, down_info->album_id.data);
		}
		else if(feed->type == CONTACT_FEED){
			memset(tmpbuf,0,sizeof(tmpbuf));
			sprintf(tmpbuf,"%s/%s",down_info->cache_dir.data, down_info->album_id.data);
			facebook_dbg("tmpbuf is %s",tmpbuf);
			if(facebook_create_dir(tmpbuf) != 0){
				facebook_dbg("create dir error---%s",tmpbuf);
				facebook_free_download_info(down_info);
				return NULL;
			}
			memset(tmpbuf,0,sizeof(tmpbuf));
			sprintf(tmpbuf,"%s/%s/%s.jpg",down_info->cache_dir.data, down_info->album_id.data,member->id.data);
		}
		facebook_dbg("tmpbuf is %s",tmpbuf);
		if(feed->type != CONTACT_FEED && facebook_check_file(tmpbuf)==1){
			facebook_dbg("This photo has been downloaded before");
			facebook_free_download_info(down_info);
			return NULL;
		}
		FILE * fp = fopen(tmpbuf,"wb+");
		if(fp == NULL){
			facebook_dbg("file handle open failed");
			facebook_free_download_info(down_info);
			return NULL;
		}
		facebook_data_write_init(down_info->photo_data,Facebook_data_write_file,fp);
	}
	else{
		facebook_data_write_init(down_info->photo_data,Facebook_data_write_buffer,NULL);
	}
	
	return down_info;
};

EXPORT_SYMBOL
int facebook_free_download_info(facebook_photo_down_info* down_info)
{
	if(down_info){
		facebook_free_atomic(&down_info->cache_dir);
		facebook_free_atomic(&down_info->album_id);
		facebook_data_write_free(down_info->photo_data);
		facebook_free((void*)down_info);
		down_info = NULL;
	}
	return 0;
}

EXPORT_SYMBOL
facebook_feed * facebook_feed_type(facebook_type type, void * feed_p)
{
	facebook_feed * f_feed = NULL;
	if(feed_p == NULL){
		facebook_dbg("feed_p is NULL");
		return NULL;
	}
	f_feed = (facebook_feed *)facebook_malloc(sizeof(facebook_feed));
	if(f_feed == NULL){
		facebook_dbg("f_feed malloc failed");
		return NULL;
	}
	f_feed->type = type;
	f_feed->feed_p = feed_p;
	return f_feed;
}

EXPORT_SYMBOL
int facebook_free_type(facebook_feed * f_feed)
{
	if(f_feed){
		facebook_free((void*)f_feed);
		f_feed = NULL;
	}
	return 0;
}

/*int cover_index(facebook_album *album)
{
	facebook_dbg();
	char * cover_id = album->cover_photo.data;
	facebook_dbg("cover_id is %s",cover_id);
	int i=0;
	facebook_dbg("count is %d",album->count);
	for(i=0;i<album->count;i++){
		facebook_dbg("i is %d",i);
		facebook_dbg("%s",(album->photo_entry+i)->id.data);
		if(strcmp(cover_id,(album->photo_entry+i)->id.data)==0){
			facebook_dbg();
			return i;
		}
		else{
			facebook_dbg();
		}
	}
	return -1;
}*/

EXPORT_SYMBOL
int facebook_download_photo(facebook_data * f_data,	facebook_photo_down_info * f_down_info)
{
	int rtn = 0;
	CURL *tmpcurl = NULL;
	char tmpbuf[BUF_LENGTH]={0};
	char fullpath[BUF_LENGTH]={0};
	facebook_contact * contact = NULL;
	if(f_data==NULL){
		facebook_dbg("f_data is NULL");
		rtn = get_status(FACEBOOK_RTN_ERR_DOWNLOADPHOTO,0,0);
		goto facebook_download_photo_out;
	}
	if(f_down_info==NULL){
		facebook_dbg("f_down_info is NULL");
		rtn = get_status(FACEBOOK_RTN_ERR_DOWNLOADPHOTO,0,0);
		goto facebook_download_photo_out;
	}

	tmpcurl = curl_easy_init();
	if(tmpcurl != NULL){
		curl_easy_setopt(tmpcurl,CURLOPT_VERBOSE,1L);
		curl_easy_setopt(tmpcurl,CURLOPT_DEBUGDATA,url_reload_p);
		curl_easy_setopt(tmpcurl,CURLOPT_DEBUGFUNCTION, debugFun);
		curl_easy_setopt(tmpcurl,CURLOPT_SSL_VERIFYPEER,0);
		curl_easy_setopt(tmpcurl,CURLOPT_TIMEOUT,CURL_TIME_OUT);
	}
	else{
		facebook_dbg("tmpcurl is NULL!");
		rtn = get_status(FACEBOOK_RTN_ERR_DOWNLOADPHOTO,0,0);
		goto facebook_download_photo_out;
	}
	
	curl_easy_setopt(tmpcurl,CURLOPT_WRITEDATA,f_down_info->photo_data);
	curl_easy_setopt(tmpcurl,CURLOPT_WRITEFUNCTION,facebook_func_write_data);
	curl_easy_setopt(tmpcurl,CURLOPT_NOPROGRESS,0L);
	curl_easy_setopt(tmpcurl,CURLOPT_PROGRESSFUNCTION,facebook_func_prog);
	curl_easy_setopt(tmpcurl,CURLOPT_PROGRESSDATA,f_down_info->photo_data);
	
	if(f_down_info->f_feed->type == ALBUM_FEED){
		facebook_dbg("now is to download album cover");
		/*facebook_album *album = ((facebook_photoalbums *)f_down_info->f_feed->feed_p)->album_entry+f_down_info->which_entry;
		if(album->photo_entry==NULL){
			get_albumphoto_info(f_data, f_down_info->which_entry, (facebook_photoalbums *)f_down_info->f_feed->feed_p);
		}
		int index = cover_index(album);
		facebook_dbg("index is %d",index);
		if(index!=-1){
			if(f_down_info->isthumbnail==0){
				facebook_dbg();
				rtn = download_photo_big(tmpcurl, album, index);
			}
			else if(f_down_info->isthumbnail==1){
				facebook_dbg();
				rtn = download_photo_small(tmpcurl, album, index);
			}
		}*/
		rtn = get_album_cover(tmpcurl, f_data, ((facebook_photoalbums *)f_down_info->f_feed->feed_p)->album_entry+f_down_info->which_entry);
	}
	else if(f_down_info->f_feed->type == PHOTO_FEED){
		facebook_dbg("now is to download album photo");
		if(f_down_info->isthumbnail==0){
			rtn = download_photo_big(tmpcurl, (facebook_album *)f_down_info->f_feed->feed_p, f_down_info->which_entry);
		}
		else if(f_down_info->isthumbnail==1){
			rtn = download_photo_small(tmpcurl, (facebook_album *)f_down_info->f_feed->feed_p, f_down_info->which_entry);
		}
	}
	else if(f_down_info->f_feed->type == USER_PROFILE){
		facebook_dbg("now is to download user profile");
		rtn = get_user_picture(tmpcurl, f_data, (facebook_user *)f_down_info->f_feed->feed_p);
	}
	else if(f_down_info->f_feed->type == CONTACT_FEED){
		facebook_dbg("now is to download friend profile");
		contact = (facebook_contact *)f_down_info->f_feed->feed_p;
		rtn = get_friend_profile(tmpcurl, f_data, (contact->member_entry+f_down_info->which_entry)->id.data);
	}
facebook_download_photo_out:
	if(f_down_info->photo_data->file_handle){
		facebook_fflush(f_down_info->photo_data->file_handle);
		fclose(f_down_info->photo_data->file_handle);
		f_down_info->photo_data->file_handle = NULL;
	}
	if(tmpcurl){
		curl_easy_cleanup(tmpcurl);
		tmpcurl = NULL;
	}
	/*if(f_down_info->photo_data->data_head){
		FILE * fhandle = fopen("mnt/udisk/1.jpg","wb+");
		fwrite(f_down_info->photo_data->data_head, sizeof(unsigned char), f_down_info->photo_data->data_len, fhandle);
		fclose(fhandle);
		fhandle = NULL;
	}*/
	f_down_info->photo_data->status = rtn;
	if(f_down_info->photo_data->status != HttpStatus_OK){
		facebook_dbg("download photo error!");
		facebook_get_cache_path(f_down_info->f_feed,f_down_info->which_entry,f_down_info->isthumbnail,tmpbuf,BUF_LENGTH);
		sprintf(fullpath,"%s/%s",f_down_info->cache_dir.data,tmpbuf);
		facebook_dbg("the full path is %s",fullpath);
		facebook_fremove(fullpath);
	}
	facebook_dbg("download status is 0x%x",rtn);
	return rtn;
}

char * get_id(char * json_str)
{
	json_object *new_obj=NULL;
	char id[32] = {0};
	char *return_id = NULL;
	new_obj = json_tokener_parse(json_str);
	facebook_dbg("%s",json_str);
	json_object_object_foreach(new_obj, key, val){
		facebook_dbg("\t%s: %s", key, json_object_to_json_string(val));
		if(strcmp(key,"id")==0){
			memcpy(id,json_object_to_json_string(val),strlen(json_object_to_json_string(val)));
			return_id = delete_char(id);
			if(new_obj){
				facebook_dbg("free json object");
				json_object_put(new_obj);
			}
			facebook_dbg("id is %s",return_id);
			return return_id;
		}
	}
	return NULL;
}

EXPORT_SYMBOL
char * upload_feed(facebook_data * f_data, char * profile_id, char * message)
{
	char upload_feed[BUF_LENGTH] = {0};
	CURLcode curlcode=0;
	long m_http_code=0;
	int rtn = 0;
	char * feed_id = NULL;

	if(message==NULL){
		facebook_dbg("message is null----error!");
		return NULL;
	}
	if(profile_id==NULL){
		facebook_dbg("profile_id is null----error!");
		return NULL;
	}
	facebook_write_data * upload_data = (facebook_write_data *)facebook_malloc(sizeof(facebook_write_data));
	if(upload_data==NULL){
		facebook_dbg("upload data malloc failed");
		return NULL;
	}
	facebook_data_write_init(upload_data,Facebook_data_write_buffer,NULL);

	sprintf(upload_feed,"https://graph.facebook.com/%s/feed?%s&message=%s",profile_id,f_data->access_token,message);
	facebook_dbg("URL is %s, len is %d",upload_feed, strlen(upload_feed));
	
	curl_easy_setopt(f_data->curl,CURLOPT_URL,upload_feed);
	curl_easy_setopt(f_data->curl,CURLOPT_POST,1L);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEDATA,upload_data);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEFUNCTION,facebook_func_write_data);

	curlcode = curl_easy_perform(f_data->curl);
	curl_easy_getinfo(f_data->curl, CURLINFO_RESPONSE_CODE, &m_http_code);
	facebook_dbg("curlcode is %d",curlcode);
	facebook_dbg("m_http_code is %ld",m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		facebook_dbg("Connect Success");
		facebook_dbg("%s",upload_data->data_head);
		feed_id = get_id(upload_data->data_head);
	}
	else{
		facebook_dbg("Connect Fail");
	}
	if(upload_data){
		facebook_data_write_free(upload_data);
		facebook_free(upload_data);
	}
	return feed_id;
}

EXPORT_SYMBOL
char * upload_comments(facebook_data * f_data, char * id, char * message)
{
	char upload_comments[BUF_LENGTH] = {0};
	CURLcode curlcode=0;
	long m_http_code=0;
	int rtn = 0;
	char * comments_id = NULL;

	if(id==NULL){
		facebook_dbg("comment id is null----error!");
		return NULL;
	}
	if(message==NULL){
		facebook_dbg("message is null----error!");
		return NULL;
	}
	
	facebook_write_data * upload_data = (facebook_write_data *)facebook_malloc(sizeof(facebook_write_data));
	if(upload_data==NULL){
		facebook_dbg("upload data malloc failed");
		return NULL;
	}
	facebook_data_write_init(upload_data,Facebook_data_write_buffer,NULL);

	sprintf(upload_comments,"https://graph.facebook.com/%s/comments?%s&message=%s",id,f_data->access_token,message);
	facebook_dbg("URL is %s, len is %d",upload_comments, strlen(upload_comments));
	
	curl_easy_setopt(f_data->curl,CURLOPT_URL,upload_comments);
	curl_easy_setopt(f_data->curl,CURLOPT_POST,1L);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEDATA,upload_data);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEFUNCTION,facebook_func_write_data);

	curlcode = curl_easy_perform(f_data->curl);
	curl_easy_getinfo(f_data->curl, CURLINFO_RESPONSE_CODE, &m_http_code);
	facebook_dbg("curlcode is %d",curlcode);
	facebook_dbg("m_http_code is %ld",m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		facebook_dbg("Connect Success");
		facebook_dbg("%s",upload_data->data_head);
		comments_id = get_id(upload_data->data_head);
	}
	else{
		facebook_dbg("Connect Fail");
	}
	if(upload_data){
		facebook_data_write_free(upload_data);
		facebook_free(upload_data);
	}
	return comments_id;
}

EXPORT_SYMBOL
char * create_album(facebook_data * f_data, char * name, char * message)
{
	char create_album[BUF_LENGTH] = {0};

	CURLcode curlcode=0;
	long m_http_code=0;
	int rtn = 0;
	char * album_id = NULL;

	facebook_write_data * create_data = (facebook_write_data *)facebook_malloc(sizeof(facebook_write_data));
	if(create_data==NULL){
		facebook_dbg("create data malloc failed");
		return NULL;
	}
	facebook_data_write_init(create_data,Facebook_data_write_buffer,NULL);

	if(name==NULL){
		facebook_dbg("create new album need name,error!");
		goto create_album_out;
	}
	sprintf(create_album,"https://graph.facebook.com/me/albums?%s&name=%s",f_data->access_token,name);
	facebook_dbg("URL is %s, len is %d",create_album, strlen(create_album));
	if(message!=NULL){
		memcpy(create_album+strlen(create_album),"&message=",9);
		memcpy(create_album+strlen(create_album),message,strlen(message));
		facebook_dbg("URL is %s, len is %d",create_album, strlen(create_album));
	}
	
	curl_easy_setopt(f_data->curl,CURLOPT_URL,create_album);
	curl_easy_setopt(f_data->curl,CURLOPT_POST,1L);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEDATA,create_data);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEFUNCTION,facebook_func_write_data);

	curlcode = curl_easy_perform(f_data->curl);
	curl_easy_getinfo(f_data->curl, CURLINFO_RESPONSE_CODE, &m_http_code);
	facebook_dbg("curlcode is %d",curlcode);
	facebook_dbg("m_http_code is %ld",m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		facebook_dbg("Connect Success");
		facebook_dbg("%s",create_data->data_head);
		album_id= get_id(create_data->data_head);
	}
	else{
		facebook_dbg("Connect Fail");
	}
create_album_out:
	if(create_data){
		facebook_data_write_free(create_data);
		facebook_free(create_data);
	}
	return album_id;
}

void  facebook_sem_wait(sem_t *sem)
{
	int err;
FACEBOOK_PEND_REWAIT:
	err = sem_wait(sem);
	if(err == -1){
		int errsv = errno;
		if(errsv == EINTR){
			//Restart if interrupted by handler
			goto FACEBOOK_PEND_REWAIT;
		}
		else{
			facebook_dbg("work_sem_pend: errno:%d\n",errsv);
			return;
		}
	}

	return ;
}

void  facebook_sem_post(sem_t *sem)
{
	int err;
	err = sem_post(sem);
	return;
}

int  facebook_get_msg(facebook_data *gdata,facebook_ioctrl * req)
{
	int rtn=0;
	rtn = facebook_req_dequeue(gdata,req);
	return rtn;
}

void  *facebook_thread(void *arg)
{	
	int rtn = 0;
	facebook_data *f_data = (facebook_data *)arg;
	facebook_ioctrl req;
	if(f_data==NULL){
		facebook_dbg("Thread f_data=NULL\n");
		return NULL;
	}
	
	while(1){
		facebook_dbg("Wait Semi Start----------\n");
		facebook_sem_wait(&f_data->syn_lock.semi_start);
		pthread_testcancel();
		rtn = facebook_get_msg(f_data,&req);
		facebook_dbg("Get Msg idx=%d\n",rtn);
		if(rtn>=0){
			switch(req.iocmd){
				case FACEBOOK_CMD_DOWNLOADPHOTO:
					facebook_dbg("FACEBOOK_CMD_DOWNLOADPHOTO");
					if(req.para_size == sizeof(facebook_photo_down_info)){
						facebook_dbg("address is 0x%x",req.para);
						facebook_download_photo(f_data,(facebook_photo_down_info *)req.para);
					}
					break;
				case FACEBOOK_CMD_STOPDOWNLOAD:
					facebook_dbg("FACEBOOK_CMD_STOPDOWNLOAD");
					break;
				case FACEBOOK_CMD_UPLOADPHOTO:
					facebook_dbg("FACEBOOK_CMD_UPLOADPHOTO");
					break;
				case FACEBOOK_CMD_STOPUPLOAD:
					facebook_dbg("FACEBOOK_CMD_STOPUPLOAD");
					break;
				case FACEBOOK_CMD_AUTH:
					facebook_dbg("FACEBOOK_CMD_AUTH");
					if(req.para_size == sizeof(facebook_data)){
						facebook_dbg("address is 0x%x",req.para);
						facebook_authentication((facebook_data *)req.para);
					}
					break;
				case FACEBOOK_CMD_STOPAUTH:
					facebook_dbg("FACEBOOK_CMD_STOPAUTH");
					break;
				default:
					facebook_dbg("Not Supprot yet CMD=%d\n",req.iocmd);
					break;
			}
			facebook_req_done(f_data,rtn);
		}	
	}
	pthread_exit((void*)rtn);
	return NULL;
}

int facebook_create_thread(facebook_data * f_data)
{
	int rtn = 0;
	pthread_t  tid;

	if(sem_init(&f_data->syn_lock.semi_start,0,0)==-1){
		facebook_dbg("Sem init error");
		goto facebook_create_thread_out;
	}

	if(sem_init(&f_data->syn_lock.semi_end,0,0)==-1){
		sem_destroy(&f_data->syn_lock.semi_start);
		facebook_dbg("Sem init error");
		goto facebook_create_thread_out;
	}

	if(sem_init(&f_data->syn_lock.semi_req,0,0)==-1){
		sem_destroy(&f_data->syn_lock.semi_start);
		sem_destroy(&f_data->syn_lock.semi_end);
		facebook_dbg("Sem init error");
		goto facebook_create_thread_out;
	}

	rtn = pthread_create(&tid, NULL,facebook_thread,f_data);

	if(rtn){
		facebook_dbg("Create Facebook thread error!");
		sem_destroy(&f_data->syn_lock.semi_start);
		sem_destroy(&f_data->syn_lock.semi_end);
		sem_destroy(&f_data->syn_lock.semi_req);
		goto facebook_create_thread_out;
	}

	facebook_sem_post(&f_data->syn_lock.semi_req);
	
	f_data->thread_id = tid;
	f_data->is_thread_run = 1;
	
facebook_create_thread_out:
	return rtn;
}

int facebook_thread_exit(facebook_data *f_data)
{
	void * thread_ret=NULL;
	if(f_data && f_data->is_thread_run){
		pthread_cancel(f_data->thread_id);
		pthread_join(f_data->thread_id,&thread_ret);

		sem_destroy(&f_data->syn_lock.semi_start);
		sem_destroy(&f_data->syn_lock.semi_end);
		sem_destroy(&f_data->syn_lock.semi_req);
		f_data->is_thread_run = 0;
	}
	return 0;
}

unsigned long facebook_get_timestamp()
{
	struct timeval cur_time;
	if(gettimeofday(&cur_time,NULL)==0)	
		return cur_time.tv_sec*1000000L+cur_time.tv_usec;
	else{
		if(facebook_timestamp>=0xff)
			facebook_timestamp = 1;
		facebook_timestamp++;
		return facebook_timestamp;
	}
}

EXPORT_SYMBOL
int facebook_send_msg(facebook_data *f_data,facebook_ioctl_cmd cmd,void * para)
{
	int rtn = 0;
	int req_idx=0;
	facebook_ioctrl req;
	req.iocmd = cmd;
	req.para = para;
	switch(cmd){
		case FACEBOOK_CMD_STOPDOWNLOAD:
			{
				facebook_dbg("FACEBOOK_CMD_STOPDOWNLOAD");
				facebook_photo_down_info * down_info = (facebook_photo_down_info *)req.para;
				if(down_info)
					down_info->photo_data->cancel_write = 1;
				req.para_size = sizeof(facebook_photo_down_info);
				break;
			}
		case FACEBOOK_CMD_DOWNLOADPHOTO:
			facebook_dbg("FACEBOOK_CMD_DOWNLOADPHOTO");
			req.para_size = sizeof(facebook_photo_down_info);
			break;
		case FACEBOOK_CMD_STOPUPLOAD:
			{
				facebook_dbg("FACEBOOK_CMD_STOPUPLOAD");
				break;
			}
		case FACEBOOK_CMD_UPLOADPHOTO:
			facebook_dbg("FACEBOOK_CMD_UPLOADPHOTO");
			break;
		case FACEBOOK_CMD_AUTH:
			facebook_dbg("FACEBOOK_CMD_AUTH");
			req.para_size = sizeof(facebook_data);
			break;
		case FACEBOOK_CMD_STOPAUTH:
			facebook_dbg("FACEBOOK_CMD_STOPAUTH");
			break;
	}
	req.timestamp = facebook_get_timestamp();
	req_idx = facebook_req_enqueue(f_data,&req);
	facebook_dbg("Send Msg reqidx=%d\n",req_idx);
	if(req_idx==-1)
		rtn = -1;
	else{
		if(f_data){
			facebook_dbg("Post Semi Start~~~~~~~~~~~~~~~~~~~~~~\n");
			facebook_sem_post(&f_data->syn_lock.semi_start);
		}
		else
			rtn = -1;
	}
	return rtn;
}

EXPORT_SYMBOL
int facebook_query_download_status(facebook_data *f_data,facebook_photo_down_info * down_info,FacebookwebQueryCmd query_cmd)
{
	int rtn = -1;
	if(query_cmd==FACEBOOK_QUREY_CMD_PROGRESS){
		if(down_info)
			return down_info->photo_data->download_process;
		else
			return -2;
	}
	rtn = facebook_req_query(f_data,FACEBOOK_CMD_DOWNLOADPHOTO,down_info);
	if(rtn>=0){
		if(down_info){
			if(query_cmd==FACEBOOK_QUERY_CMD_RESULT)
				return down_info->photo_data->status;
			else if(query_cmd==FACEBOOK_QUREY_CMD_PROGRESS)
				return down_info->photo_data->download_process;
			else{
				facebook_dbg("Query Download Cmd Error cmd =%d",query_cmd);
				return 0;
			}
		}
		else 
			return 0;
	}
	else{
		if(query_cmd==FACEBOOK_QUREY_CMD_PROGRESS){
			//facebook_dbg("!!!!!!!!process is %d",down_info->photo_data->download_process);
			return down_info->photo_data->download_process;
		}
		return rtn;
	}
}

EXPORT_SYMBOL
int facebook_get_cache_path(facebook_feed *feed,int which_entry,int isthumbnail,char *pathbuf,int buf_len)
{
	facebook_album * album = NULL;
	
	if(feed->type == ALBUM_FEED){
		if(which_entry<0 || which_entry>=((facebook_photoalbums *)feed->feed_p)->album_num){
			facebook_dbg("Entry Num is out of range MAX=%d,Your Value=%d",((facebook_photoalbums *)feed->feed_p)->album_num-1,which_entry);
			return -1;
		}
		album = ((facebook_photoalbums *)feed->feed_p)->album_entry;
		album += which_entry;
		facebook_dbg("#######0x%x",album);
		if(album->count == 0){
			facebook_dbg("this album has no photo");
			return -1;
		}
		if(isthumbnail){
			sprintf(pathbuf,"thumbnail/%s.jpg",album->id.data);
		}
		else{
			sprintf(pathbuf,"%s.jpg",album->id.data);
		}
	}
	else if(feed->type == PHOTO_FEED){
		album = (facebook_album *)feed->feed_p;
		if(which_entry<0 || which_entry>=album->count){
			facebook_dbg("Entry Num is out of range MAX=%d,Your Value=%d",album->count-1,which_entry);
			return -1;
		}
		if(isthumbnail){
			sprintf(pathbuf,"%s/thumbnail/%s.jpg",album->id.data, (album->photo_entry+which_entry)->id.data);
		}
		else{
			sprintf(pathbuf,"%s/%s.jpg",album->id.data, (album->photo_entry+which_entry)->id.data);
		}
	}
	else if(feed->type == USER_PROFILE){
		sprintf(pathbuf,"profile/user_profile.jpg");
	}
	return 0;
}

int facebook_save_albumsfeed(facebook_data *f_data,facebook_photoalbums * feed_albums,char* filename)
{
	void *fhandle=NULL;
	fhandle = (void*)fopen(filename,"wb+");
	facebook_dbg("Save AlbumsFeed ===========handle=0x%x",fhandle);
	if(fhandle && feed_albums){
		facebook_write_albumsfeed(fhandle,0,feed_albums);
		facebook_fflush(fhandle);
		fclose(fhandle);
		fhandle = NULL;
	}
	else
		return -1;
	return 0;
}

EXPORT_SYMBOL
int facebook_save_albums_info(facebook_data *f_data,facebook_photoalbums * feed_albums,char* cache_dir,char* filename)
{
	char namebuf[BUF_LENGTH]="";
	int rtn = 0;
	if(facebook_get_ini_albumsname(f_data,cache_dir,filename,namebuf,BUF_LENGTH)==-1){
		facebook_dbg("get albumsname ini error!");
		rtn = -1;
		goto facebook_save_albums_info_out;
	}
	rtn = facebook_save_albumsfeed(f_data,feed_albums,namebuf);

facebook_save_albums_info_out:
	return rtn;
}

int facebook_save_albumfeed(facebook_data *f_data,facebook_album * feed_album,char* filename)
{
	void *fhandle=NULL;
	fhandle = (void*)fopen(filename,"wb+");
	facebook_dbg("Save AlbumFeed ===========handle=0x%x",fhandle);
	if(fhandle && feed_album){
		facebook_write_albumfeed(fhandle,0,feed_album);
		facebook_fflush(fhandle);
		fclose(fhandle);
		fhandle = NULL;
	}
	else
		return -1;
	return 0;
}

EXPORT_SYMBOL
int facebook_save_album_info(facebook_data *f_data,facebook_photoalbums *feed_albums,int which_album,char* cache_dir)
{
	int rtn = 0;
	char namebuf[BUF_LENGTH]="";
	facebook_album * feed_album=NULL;
	if(f_data==NULL || feed_albums==NULL ||(which_album<0 || which_album>=feed_albums->album_num)){
		facebook_dbg("f_data is NULL or feed_albums is NULL or album index is out of range");
		rtn = -1;
		goto facebook_save_album_info_out;
	}
	if(facebook_get_ini_albumname(feed_albums,which_album,cache_dir,namebuf,BUF_LENGTH)==-1){
		facebook_dbg("get albumname ini error!");
		rtn = -1;
		goto facebook_save_album_info_out;
	}
	feed_album = feed_albums->album_entry+which_album;
	rtn = facebook_save_albumfeed(f_data,feed_album,namebuf);
	
facebook_save_album_info_out:
	return rtn;
}

int facebook_save_contactfeed(facebook_data *f_data,facebook_contact * feed_contact,char* filename)
{
	void *fhandle=NULL;
	fhandle = (void*)fopen(filename,"wb+");
	facebook_dbg("Save ContactFeed ===========handle=0x%x",fhandle);
	if(fhandle && feed_contact){
		facebook_write_contactsfeed(fhandle,0,feed_contact);
		facebook_fflush(fhandle);
		fclose(fhandle);
		fhandle = NULL;
	}
	else
		return -1;
	return 0;
}

EXPORT_SYMBOL
int facebook_save_contact_info(facebook_data *f_data,facebook_contact * feed_contact,char* cache_dir)
{
	char namebuf[BUF_LENGTH]="";
	int rtn = 0;
	if(facebook_get_ini_contactname(f_data,cache_dir,namebuf,BUF_LENGTH)==-1){
		rtn = -1;
		goto facebook_save_contact_info_out;
	}
	rtn = facebook_save_contactfeed(f_data,feed_contact,namebuf);

facebook_save_contact_info_out:
	return rtn;
}

facebook_photoalbums * facebook_load_albumsfeed(facebook_data *gdata,char* filename)
{
	unsigned int entry_offset = 0;
	int rtn=0,i=0;
	void *fhandle=NULL;
	fhandle = (void*)fopen(filename,"rb");
	facebook_photoalbums * feed_albums=NULL;
	facebook_dbg("Read Albums ===========handle=0x%x",fhandle);
	feed_albums = (facebook_photoalbums*)facebook_malloc(sizeof(facebook_photoalbums));
	if(feed_albums==NULL){
		facebook_dbg("Read Album Malloc Feed Albums Failed!");
		rtn = -1;
		goto facebook_load_albumsfeed_out;
	}
	memset(feed_albums,0,sizeof(facebook_photoalbums));
	if(fhandle){
		facebook_read_albumsfeed(fhandle,0,feed_albums);
		facebook_printf_albumsfeed(feed_albums);
		facebook_dbg("ENTRY NUM======%d",feed_albums->album_num);
		if(feed_albums->album_num!=0){
			feed_albums->album_entry= (facebook_album*)facebook_malloc(feed_albums->album_num*sizeof(facebook_album));
			if(feed_albums->album_entry!=NULL){
				for(i=0;i<feed_albums->album_num;i++){
					entry_offset = ftell(fhandle);
					facebook_dbg("Entry Offset[%d]=0x%x",i,entry_offset);
					facebook_read_albumsentry(fhandle,entry_offset,feed_albums->album_entry+i);
					facebook_dbg("######Entry num=%d#########",i);
					facebook_printf_albumsentry(feed_albums->album_entry+i);
				}
			}
		}
	}
facebook_load_albumsfeed_out:
	if(rtn){
		if(feed_albums){
			facebook_free_photoalbums(feed_albums);
			feed_albums = NULL;
		}
	}
	if(fhandle)
		fclose(fhandle);
	return feed_albums;
}

EXPORT_SYMBOL
facebook_photoalbums * facebook_load_albums_info(facebook_data *f_data,char* cache_dir,char *filename)
{
	char namebuf[BUF_LENGTH]="";
	facebook_photoalbums * feed_albums=NULL;
	if(facebook_get_ini_albumsname(f_data,cache_dir,filename,namebuf,BUF_LENGTH)==-1){
		facebook_dbg("get albumsname ini error!");
		goto facebook_load_albums_info_out;
	}
	feed_albums = facebook_load_albumsfeed(f_data,namebuf);

facebook_load_albums_info_out:
	return feed_albums;
}

int facebook_load_albumfeed(facebook_data *gdata,char* filename,facebook_album * album)
{
	unsigned int entry_offset = 0;
	int rtn=0,i=0;
	void *fhandle=NULL;
	fhandle = (void*)fopen(filename,"rb");
	facebook_dbg("Read Album ===========handle=0x%x",fhandle);
	if(album==NULL){
		facebook_dbg("Read Album s NULL!");
		rtn = -1;
		goto facebook_load_albumfeed_out;
	}
	if(fhandle){
		//facebook_read_albumsentry(fhandle,0,album);
		facebook_printf_albumsentry(album);
		facebook_dbg("ENTRY NUM======%d",album->count);
		if(album->count!=0){
			album->photo_entry= (facebook_photo*)facebook_malloc(album->count*sizeof(facebook_photo));
			if(album->photo_entry!=NULL){
				for(i=0;i<album->count;i++){
					entry_offset = ftell(fhandle);
					facebook_dbg("Entry Offset[%d]=0x%x",i,entry_offset);
					facebook_read_photoentry(fhandle,entry_offset,album->photo_entry+i);
					facebook_printf_photoentry(album->photo_entry+i);
				}
			}
		}
	}
facebook_load_albumfeed_out:
	if(fhandle)
		fclose(fhandle);
	return rtn;
}

EXPORT_SYMBOL
int facebook_load_album_info(facebook_data *f_data,facebook_photoalbums *feed_albums,int which_album,char* cache_dir)
{
	char namebuf[BUF_LENGTH]="";
	int rtn = 0;
	if(f_data==NULL || feed_albums==NULL ||(which_album<0 || which_album>=feed_albums->album_num)){
		facebook_dbg("f_data is NULL or feed_albums is NULL or album index is out of range");
		rtn = -1;
		goto facebook_load_album_info_out;
	}
	if(facebook_get_ini_albumname(feed_albums,which_album,cache_dir,namebuf,BUF_LENGTH)==-1){
		facebook_dbg("get albumname ini error!");
		rtn = -1;
		goto facebook_load_album_info_out;
	}
	rtn = facebook_load_albumfeed(f_data,namebuf,feed_albums->album_entry + which_album);
facebook_load_album_info_out:
	return rtn;
}

facebook_contact * facebook_load_contactsfeed(facebook_data *f_data,char* filename)
{
	unsigned int entry_offset = 0;
	int rtn=0,i=0;
	void *fhandle=NULL;
	fhandle = (void*)fopen(filename,"rb");
	facebook_contact * feed_contact=NULL;
	facebook_dbg("Read Albums ===========handle=0x%x",fhandle);
	feed_contact = (facebook_contact*)facebook_malloc(sizeof(facebook_contact));
	if(feed_contact==NULL){
		facebook_dbg("Read Album Malloc Feed Albums Failed!");
		rtn = -1;
		goto facebook_load_feed_out;
	}
	memset(feed_contact,0,sizeof(facebook_contact));
	if(fhandle){
		facebook_read_contactsfeed(fhandle,0,feed_contact);
		facebook_printf_contactsfeed(feed_contact);
		facebook_dbg("ENTRY NUM======%d",feed_contact->contact_num);
		if(feed_contact->contact_num!=0){
			feed_contact->member_entry= (facebook_member*)facebook_malloc(feed_contact->contact_num*sizeof(facebook_member));
			if(feed_contact->member_entry!=NULL){
				for(i=0;i<feed_contact->contact_num;i++){
					entry_offset = ftell(fhandle);
					facebook_dbg("Entry Offset[%d]=0x%x",i,entry_offset);
					facebook_read_memberentry(fhandle,entry_offset,feed_contact->member_entry+i);
					facebook_dbg("######Entry num=%d#########",i);
					facebook_printf_memberentry(feed_contact->member_entry+i);
				}
			}
		}
	}

facebook_load_feed_out:
	if(rtn){
		if(feed_contact){
			facebook_free_contact(feed_contact);
			feed_contact = NULL;
		}
	}
	if(fhandle)
		fclose(fhandle);
	return feed_contact;
}

EXPORT_SYMBOL
facebook_contact* facebook_load_contact_info(facebook_data *f_data,char* cache_dir)
{
	char namebuf[BUF_LENGTH]="";
	facebook_contact * feed_contact=NULL;
	if(facebook_get_ini_contactname(f_data,cache_dir,namebuf,BUF_LENGTH)==-1){
		facebook_dbg("get contactname ini error!");
		goto facebook_load_contact_info_out;
	}
	feed_contact = facebook_load_contactsfeed(f_data,namebuf);
facebook_load_contact_info_out:
	return feed_contact;
}

int facebook_write_userinfo(void *file_handle,facebook_data *f_data)
{
	char tmpbuf[LOGIN_MAX_LEN*2] = {0};
	char writebuf[LOGIN_MAX_LEN*2] = {0};
	
	fseek((FILE*)file_handle, 0, SEEK_SET);
	while(fgets(tmpbuf,LOGIN_MAX_LEN*2,(FILE*)file_handle)){
		//facebook_dbg("%s",tmpbuf);
		if(strncmp(tmpbuf,f_data->usermail,strlen(f_data->usermail))==0){
			facebook_dbg("user %s has exist",f_data->usermail);
			return 0;
		}
	}
	sprintf(writebuf,"%s\t%s\n",f_data->usermail,f_data->userpwd);
	facebook_dbg("%s",writebuf);
	
	if(fwrite(writebuf, strlen(writebuf), 1,(FILE*)file_handle)!=1){
		facebook_dbg("Write User Infor Error!");
		return -1;
	}
	return 0;
}

EXPORT_SYMBOL
int facebook_save_userinfo(facebook_data *f_data)
{
	char namebuf[BUF_LENGTH]="/mnt/vram/facebook_user.ini";
	void *fhandle=NULL;
	fhandle = (void*)fopen(namebuf,"a+");
	facebook_dbg("Save User Info ===========handle=0x%x",fhandle);
	if(fhandle && f_data){
		facebook_write_userinfo(fhandle,f_data);
		facebook_fflush(fhandle);
		fclose(fhandle);
		fhandle = NULL;
	}
	else
		return -1;
	return 0;
}

int facebook_read_userinfo(void *file_handle)
{
	char tmpbuf[LOGIN_MAX_LEN*2] = {0};
	int user_num = 0;
	
	fseek((FILE*)file_handle, 0, SEEK_SET);
	while(fgets(tmpbuf,LOGIN_MAX_LEN*2,(FILE*)file_handle)){
		//facebook_dbg("%s",tmpbuf);
		user_num++;
	}
	facebook_dbg("%d",user_num);
	return user_num;
}

EXPORT_SYMBOL
int facebook_check_user()
{
	char namebuf[BUF_LENGTH]="/mnt/vram/facebook_user.ini";
	void *fhandle=NULL;
	int user_num = 0;
	fhandle = (void*)fopen(namebuf,"r");
	facebook_dbg("Check User Info ===========handle=0x%x",fhandle);
	if(fhandle){
		user_num = facebook_read_userinfo(fhandle);
		fclose(fhandle);
		fhandle = NULL;
	}
	else{
		facebook_dbg("this is no ini file");
		return 0;
	}
	return user_num;
}

int facebook_get_userinfo(void *file_handle, int user_index, char *buf, int cmd_type)
{
	char tmpbuf[LOGIN_MAX_LEN*2] = {0};
	int user_num = 0;
	char tmpbuf2[LOGIN_MAX_LEN] = {0};
	
	fseek((FILE*)file_handle, 0, SEEK_SET);
	while(fgets(tmpbuf,LOGIN_MAX_LEN*2,(FILE*)file_handle)){
		//facebook_dbg("%s",tmpbuf);
		user_num++;
		if(user_num==user_index)
			break;
	}
	facebook_dbg("%d---%s",user_num,tmpbuf);
	switch(cmd_type){
		case 0:
			sscanf(tmpbuf,"%s\t%s\n",buf,tmpbuf2);
			break;
		case 1:
			sscanf(tmpbuf,"%s\t%s\n",tmpbuf2,buf);
			break;
	}
	facebook_dbg("%s",buf);
	return 0;
}

EXPORT_SYMBOL
int facebook_load_useraccount(int user_index,char *user_account)
{
	char namebuf[BUF_LENGTH]="/mnt/vram/facebook_user.ini";
	void *fhandle=NULL;
	fhandle = (void*)fopen(namebuf,"r");
	facebook_dbg("Load User Info ===========handle=0x%x",fhandle);
	if(fhandle){
		facebook_get_userinfo(fhandle,user_index,user_account,0);
		facebook_fflush(fhandle);
		fclose(fhandle);
		fhandle = NULL;
	}
	else
		return -1;
	return 0;
}

EXPORT_SYMBOL
int facebook_load_userpwd(int user_index,char *user_account)
{
	char namebuf[BUF_LENGTH]="/mnt/vram/facebook_user.ini";
	void *fhandle=NULL;
	fhandle = (void*)fopen(namebuf,"r");
	facebook_dbg("Load User Info ===========handle=0x%x",fhandle);
	if(fhandle){
		facebook_get_userinfo(fhandle,user_index,user_account,1);
		facebook_fflush(fhandle);
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
	char call_buf[BUF_LENGTH] = {0};
	facebook_dbg("delete dir is %s",cache_dir);
	sprintf(call_buf,"rm -rf %s",cache_dir);
	rtn = system(call_buf);
	facebook_dbg("rtn is %d",rtn);
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

void *facebook_updatethread(void *arg)
{	
	int rtn = 0;
	int i=0;
	int j=0;
	struct timeval start_time, end_time;
	facebook_photoalbums * photoalbums = NULL;
	facebook_album * singlealbum = NULL;
	facebook_photo_down_info *f_down_info = NULL;
	facebook_feed *photo_feed = NULL;
	facebook_data *f_data = NULL;
	char * cache_dir = NULL;
	int iscache = 0;
	int isthumbnail = 0;
	facebook_update_t *update_arg = (facebook_update_t *)arg;
	static char last_update[] = "1970-01-01T00:00:00+0000";
	static char this_update[] = "1970-01-01T00:00:00+0000";
	char cache_dir_self[BUF_LENGTH] = {0};
	
	if(update_arg==NULL){
		facebook_dbg("Thread update_arg=NULL");
		rtn = -1;
		goto update_end;
	}
	f_data = update_arg->fdata;
	cache_dir = update_arg->cache_dir;
	iscache = update_arg->iscache;
	isthumbnail = update_arg->isthumbnail;
	if(f_data==NULL){
		facebook_dbg("Thread f_data=NULL");
		rtn = -1;
		goto update_end;
	}
	if(cache_dir==NULL){
		facebook_dbg("Thread cache_dir=NULL");
		rtn = -1;
		goto update_end;
	}
	if(facebook_create_dir(cache_dir) != 0){
		facebook_dbg("create dir error");
		rtn = -1;
		goto update_end;
	}
	sprintf(cache_dir_self,"%s/account/",cache_dir);
	if(facebook_create_dir(cache_dir_self) != 0){
		facebook_dbg("create self dir error");
		rtn = -1;
		goto update_end;
	}
	while(1){
	update_try_again:
		pthread_testcancel();
		
		facebook_dbg("Update Start----------");

		///<update user's self pictures
		photoalbums = facebook_get_albums_info(f_data, 0, 0);
		if(photoalbums == NULL){
			facebook_dbg("photoalbums is NULL");
			rtn = -1;
			goto update_try_again;
		}
		for(i=0;i<photoalbums->album_num;i++){
			facebook_dbg("last update time is %s",last_update);
			facebook_dbg("album update time is %s",(photoalbums->album_entry+i)->updated_time.data);
			if(strcmp(last_update,(photoalbums->album_entry+i)->updated_time.data)>=0){
				facebook_dbg("this album has been download!");
				continue;
			}
			if(get_albumphoto_info(f_data, i, photoalbums)!=0){
				facebook_dbg("Get Album %d Info Error!",i);
				rtn = -1;
				continue;
			}
			singlealbum = photoalbums->album_entry+i;
			photo_feed = facebook_feed_type(PHOTO_FEED,(void *)singlealbum);
			for(j=0; j<singlealbum->count; j++){
				facebook_dbg("last update time is %s",last_update);
				facebook_dbg("photo update time is %s",(singlealbum->photo_entry+j)->updated_time.data);
				if(strcmp(last_update,(singlealbum->photo_entry+j)->updated_time.data)>=0){
					facebook_dbg("this photo has been download!");
					continue;
				}

				if(get_free_space(TARGET_DISK)<FREE_SPACE){
					facebook_dbg("there is no enough space!");
					rtn = -1;
					goto free_resource;
				}
			
				f_down_info = facebook_init_download_info(iscache,cache_dir_self,photo_feed,j,isthumbnail);
				if(f_down_info==NULL){
					facebook_dbg("Init Download Info Error, Album is %d, Photo is %d",i,j);
					rtn = -1;
					continue;
				}
				rtn = facebook_download_photo(f_data, f_down_info);
				facebook_free_download_info(f_down_info);
				f_down_info = NULL;
			}
			if(strcmp(this_update,singlealbum->updated_time.data)<0){
				strcpy(this_update,singlealbum->updated_time.data);
				facebook_dbg("this update time is %s",this_update);
			}
			
			if(singlealbum){
				facebook_free_album(singlealbum);
				singlealbum = NULL;
			}
		}
	free_resource:
		if(singlealbum){
			facebook_free_album(singlealbum);
			singlealbum = NULL;
		}
		if(photo_feed){
			facebook_free_type(photo_feed);
			photo_feed = NULL;
		}
		if(photoalbums){
			facebook_free_photoalbums(photoalbums);
			photoalbums = NULL;
		}

		facebook_update_friends(update_arg,last_update, this_update);

		if(strcmp(last_update,this_update)<0){
			strcpy(last_update,this_update);
			facebook_dbg("last update time is %s",last_update);
		}
		
		gettimeofday(&start_time,NULL);
	delay_time:
		sleep(100);
		gettimeofday(&end_time, NULL);
		facebook_dbg("start time is %d, end time is %d",start_time.tv_sec,end_time.tv_sec);
		if(end_time.tv_sec-start_time.tv_sec < UPDATE_INTERVAL)
			goto delay_time;
	}
update_end:
	if(update_arg){
		facebook_free((void *)update_arg);
		update_arg = NULL;
	}
	pthread_exit((void*)rtn);
	return NULL;
}

EXPORT_SYMBOL
int facebook_create_update(facebook_data *f_data, int iscache, char* cache_dir, int isthumbnail)
{
	int rtn = -1;
	pthread_t  tid;
	facebook_update_t *update_arg = NULL;

	if(f_data==NULL){
		facebook_dbg("f_data is NULL");
		return -1;
	}
	if(cache_dir==NULL){
		facebook_dbg("cache_dir is NULL");
		return -1;
	}
	update_arg = (facebook_update_t *)facebook_malloc(sizeof(facebook_update_t));
	if(update_arg==NULL){
		facebook_dbg("update arg is NULL");
		return -1;
	}
	update_arg->fdata = f_data;
	update_arg->iscache = iscache;
	update_arg->cache_dir = cache_dir;
	update_arg->isthumbnail = isthumbnail;
	
	rtn = pthread_create(&tid, NULL,facebook_updatethread,update_arg);
	if(rtn){
		facebook_dbg("Create Facebook update thread error!");
	}
	else
		f_data->update_thread_id = tid;
	return rtn;
}

int facebook_exit_update(facebook_data *f_data)
{
	void * thread_ret=NULL;
	facebook_dbg("exit update thread");
	if(f_data){
		if(f_data->update_thread_id == -1){
			facebook_dbg("update thread is not running");
			return 0;
		}
		pthread_cancel(f_data->update_thread_id);
		pthread_join(f_data->update_thread_id,&thread_ret);
		f_data->update_thread_id = -1;
	}
	return 0;
}

EXPORT_SYMBOL
int facebook_select_friend(int friend_index, char *friend_id)
{
	char namebuf[BUF_LENGTH]="/mnt/vram/facebook_friend.ini";
	void *fhandle=NULL;
	char writebuf[BUF_LENGTH]="";
	fhandle = (void*)fopen(namebuf,"a+");
	facebook_dbg("Save selected friend ===========handle=0x%x",fhandle);
	if(fhandle){
		sprintf(writebuf, "%d\t%s\n", friend_index, friend_id);
		if(fwrite(writebuf, strlen(writebuf), 1,(FILE*)fhandle)!=1){
			facebook_dbg("Write User Infor Error!");
			fclose(fhandle);
			fhandle = NULL;
			return -1;
		}
		facebook_fflush(fhandle);
		fclose(fhandle);
		fhandle = NULL;
	}
	else
		return -1;
	return 0;
}

int facebook_get_friendid(void *file_handle, int friend_index, char *buf)
{
	char tmpbuf[LOGIN_MAX_LEN*2] = {0};
	int user_num = 0;
	char tmpbuf2[LOGIN_MAX_LEN] = {0};
	
	fseek((FILE*)file_handle, 0, SEEK_SET);
	while(fgets(tmpbuf,LOGIN_MAX_LEN*2,(FILE*)file_handle)){
		//facebook_dbg("%s",tmpbuf);
		if(user_num==friend_index)
			break;
		user_num++;
	}
	facebook_dbg("%d---%s",user_num,tmpbuf);
	sscanf(tmpbuf,"%s\t%s\n",tmpbuf2,buf);
	facebook_dbg("%s",buf);
	return 0;
}

int facebook_load_friendid(int friend_index,char *friend_id)
{
	char namebuf[BUF_LENGTH]="/mnt/vram/facebook_friend.ini";
	void *fhandle=NULL;
	fhandle = (void*)fopen(namebuf,"r");
	facebook_dbg("Load selected friend ===========handle=0x%x",fhandle);
	if(fhandle){
		facebook_get_friendid(fhandle,friend_index,friend_id);
		fclose(fhandle);
		fhandle = NULL;
	}
	else
		return -1;
	return 0;
}

int facebook_check_friend()
{
	char namebuf[BUF_LENGTH]="/mnt/vram/facebook_friend.ini";
	void *fhandle=NULL;
	int user_num = 0;
	char tmpbuf[LOGIN_MAX_LEN*2] = {0};
	fhandle = (void*)fopen(namebuf,"r");
	facebook_dbg("Check selected friend ===========handle=0x%x",fhandle);
	if(fhandle){
		fseek((FILE*)fhandle, 0, SEEK_SET);
		while(fgets(tmpbuf,LOGIN_MAX_LEN*2,(FILE*)fhandle)){
			//facebook_dbg("%s",tmpbuf);
			user_num++;
		}
		facebook_dbg("%d",user_num);
		fclose(fhandle);
		fhandle = NULL;
	}
	else{
		facebook_dbg("this is no ini file");
		return 0;
	}
	return user_num;
}

int facebook_update_friends(facebook_update_t *update_arg, char *last_update, char *this_update)
{
	int rtn = 0;
	facebook_data *f_data = NULL;
	char * cache_dir = NULL;
	int iscache = 0;
	int isthumbnail = 0;
	int friend_num = 0;
	int i=0,j=0,k=0;
	char friend_id[BUF_LENGTH]={0};
	
	facebook_photoalbums * photoalbums = NULL;
	facebook_album * singlealbum = NULL;
	facebook_photo_down_info *f_down_info = NULL;
	facebook_feed *photo_feed = NULL;

	char cache_dir_friends[BUF_LENGTH] = {0};

	if(update_arg==NULL){
		facebook_dbg("Thread update_arg=NULL");
		return -1;
	}
	f_data = update_arg->fdata;
	cache_dir = update_arg->cache_dir;
	iscache = update_arg->iscache;
	isthumbnail = update_arg->isthumbnail;
	if(f_data==NULL){
		facebook_dbg("Thread f_data=NULL");
		return -1;
	}
	if(cache_dir==NULL){
		facebook_dbg("Thread cache_dir=NULL");
		return -1;
	}

	sprintf(cache_dir_friends,"%s/friends/",cache_dir);
	if(facebook_create_dir(cache_dir_friends) != 0){
		facebook_dbg("create friends dir error");
		return -1;
	}

	friend_num = facebook_check_friend();
	if(friend_num>0){
		for(k=0;k<friend_num;i++){
			facebook_load_friendid(k,friend_id);

			memset(cache_dir_friends,0,sizeof(cache_dir_friends));
			sprintf(cache_dir_friends,"%s/friends/%s/",cache_dir,friend_id);
			if(facebook_create_dir(cache_dir_friends) != 0){
				facebook_dbg("create friends dir error");
				return -1;
			}

			photoalbums = get_albums_info(f_data, friend_id);
			if(photoalbums == NULL){
				facebook_dbg("photoalbums is NULL");
				rtn = -1;
				continue;
			}
			for(i=0;i<photoalbums->album_num;i++){
				facebook_dbg("last update time is %s",last_update);
				facebook_dbg("album update time is %s",(photoalbums->album_entry+i)->updated_time.data);
				if(strcmp(last_update,(photoalbums->album_entry+i)->updated_time.data)>=0){
					facebook_dbg("this album has been download!");
					continue;
				}
				if(get_albumphoto_info(f_data, i, photoalbums)!=0){
					facebook_dbg("Get Album %d Info Error!",i);
					rtn = -1;
					continue;
				}
				singlealbum = photoalbums->album_entry+i;
				photo_feed = facebook_feed_type(PHOTO_FEED,(void *)singlealbum);
				for(j=0; j<singlealbum->count; j++){
					facebook_dbg("last update time is %s",last_update);
					facebook_dbg("photo update time is %s",(singlealbum->photo_entry+j)->updated_time.data);
					if(strcmp(last_update,(singlealbum->photo_entry+j)->updated_time.data)>=0){
						facebook_dbg("this photo has been download!");
						continue;
					}

					if(get_free_space(TARGET_DISK)<FREE_SPACE){
						facebook_dbg("there is no enough space!");
						rtn = -1;
						goto free_resource;
					}
				
					f_down_info = facebook_init_download_info(iscache,cache_dir_friends,photo_feed,j,isthumbnail);
					if(f_down_info==NULL){
						facebook_dbg("Init Download Info Error, Album is %d, Photo is %d",i,j);
						rtn = -1;
						continue;
					}
					rtn = facebook_download_photo(f_data, f_down_info);
					facebook_free_download_info(f_down_info);
					f_down_info = NULL;
				}
				if(strcmp(this_update,singlealbum->updated_time.data)<0){
					strcpy(this_update,singlealbum->updated_time.data);
					facebook_dbg("this update time is %s",this_update);
				}
				
				if(singlealbum){
					facebook_free_album(singlealbum);
					singlealbum = NULL;
				}
			}
		free_resource:
			if(singlealbum){
				facebook_free_album(singlealbum);
				singlealbum = NULL;
			}
			if(photo_feed){
				facebook_free_type(photo_feed);
				photo_feed = NULL;
			}
			if(photoalbums){
				facebook_free_photoalbums(photoalbums);
				photoalbums = NULL;
			}
			if(rtn != 0){
				break;
			}
		}
	}
	return 0;
}