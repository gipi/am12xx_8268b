#include "flickr_common.h"

const char facebook_login_paras[FACEBOOK_MAX*2][48]={
	"login_attempt=","&amp",
	"popup=","&amp",	
	"fbconnect=","&amp",
	"display=","&amp",
	"next=","&amp",
	"req_perms=","&amp",
	"legacy_return=","\" id=",
	
	"name=\"lsd\" value=\"","\" autocomplete=",
	"name=\"api_key\" value=\"","\" /",
	"name=\"return_session\" value=\"","\" /",
	"name=\"cancel_url\" value=\"","\" /",
	"name=\"session_key_only\" value=\"","\" /",
	"name=\"session_version\" value=\"","\" /",
	"name=\"trynum\" value=\"","\" /",
	"name=\"default_persistent\" value=\"","\" /",
};

const char facebook_auth_paras[FACEBOOK_AUTH_MAX*2][48]={
	"/connect/uiserver.php?app_id=","&amp",
	"next=","&amp",
	"display=","&amp",
	"cancel_url=","&amp",
	"locale=","&amp",
	"perms=","&amp",
	"return_session=","&amp",
	"session_version=","&amp",
	"fbconnect=","&amp",
	"canvas=","&amp",
	"legacy_return=","&amp",
	"method=","\" /",
};

const char facebook_auth_allow_str[FACEBOOK_ALLOW_MAX*2][48]={
	"name=\"post_form_id\" value=\"","\" /",
	"name=\"fb_dtsg\" value=\"","\" autocomplete",
	"name=\"app_id\" value=\"","\" /",
	"name=\"display\" value=\"","\" /",
	"name=\"redirect_uri\" value=\"","\" /",
	"name=\"cancel_url\" value=\"","\" /",
	"name=\"locale\" value=\"","\" /",
	"name=\"perms\" value=\"","\" /",
	"name=\"return_session\" value=\"","\" /",
	"name=\"session_version\" value=\"","\" /",
	"name=\"fbconnect\" value=\"","\" /",
	"name=\"canvas\" value=\"","\" /",
	"name=\"legacy_return\" value=\"","\" /",
	"name=\"from_post\" value=\"","\" /",
	"name=\"__uiserv_method\" value=\"","\" /",
	"name=\"GdpEmailBucket_grantEmailType\" value=\"","\" /",
};


int  __fill_facebook_auth_allows_paras(const char *value,int index,facebook_auth_allows_t *facebook_allows)
{
	switch(index){
		case FACEBOOK_ALLOW_POSTFORMID:
			flickr_fill_atomic(value,&facebook_allows->post_form_id);
			break;
		case FACEBOOK_ALLOW_FBDTSG:
			flickr_fill_atomic(value,&facebook_allows->fb_dtsg);
			break;
		case FACEBOOK_ALLOW_APPID:
			flickr_fill_atomic(value,&facebook_allows->app_id);
			break;
		case FACEBOOK_ALLOW_DISPLAY:
			flickr_fill_atomic(value,&facebook_allows->display);
			break;
		case FACEBOOK_ALLOW_REDIRECTURI:
			flickr_fill_atomic(value,&facebook_allows->redirect_uri);
			break;
		case FACEBOOK_ALLOW_CANCELURL:
			flickr_fill_atomic(value,&facebook_allows->cancel_url);
			break;
		case FACEBOOK_ALLOW_LOCALE:
			flickr_fill_atomic(value,&facebook_allows->locale);
			break;
		case FACEBOOK_ALLOW_PERMS:
			flickr_fill_atomic(value,&facebook_allows->perms);
			break;
		case FACEBOOK_ALLOW_RETURNSESSION:
			flickr_fill_atomic(value,&facebook_allows->return_session);
			break;
		case FACEBOOK_ALLOW_SESSIONVERSION:
			flickr_fill_atomic(value,&facebook_allows->session_version);
			break;
		case FACEBOOK_ALLOW_FBCONNECT:
			flickr_fill_atomic(value,&facebook_allows->fbconnect);
			break;
		case FACEBOOK_ALLOW_CANVAS:
			flickr_fill_atomic(value,&facebook_allows->canvas);
			break;
		case FACEBOOK_ALLOW_LEGACYRETURN:
			flickr_fill_atomic(value,&facebook_allows->legacy_return);
			break;
		case FACEBOOK_ALLOW_FROMPOST:
			flickr_fill_atomic(value,&facebook_allows->from_post);
			break;
		case FACEBOOK_ALLOW_UNISERVMETHOD:
			flickr_fill_atomic(value,&facebook_allows->__uiserv_method);
			break;
		case  FACEBOOK_ALLOW_EMAILTYPE:
			flickr_fill_atomic(value,&facebook_allows->grantEmailType);
			break;
		default:
			flickr_info("Index Error!");
	}
	return 0;
}

void __free_facebook_auth_allows_paras(facebook_auth_allows_t *facebook_allows)
{
	if(facebook_allows){
		flickr_free_atomic(&facebook_allows->post_form_id);
		flickr_free_atomic(&facebook_allows->fb_dtsg);
		flickr_free_atomic(&facebook_allows->app_id);
		flickr_free_atomic(&facebook_allows->display);
		flickr_free_atomic(&facebook_allows->redirect_uri);
		flickr_free_atomic(&facebook_allows->cancel_url);
		flickr_free_atomic(&facebook_allows->locale);
		flickr_free_atomic(&facebook_allows->perms);
		flickr_free_atomic(&facebook_allows->return_session);
		flickr_free_atomic(&facebook_allows->session_version);
		flickr_free_atomic(&facebook_allows->fbconnect);
		flickr_free_atomic(&facebook_allows->canvas);
		flickr_free_atomic(&facebook_allows->legacy_return);
		flickr_free_atomic(&facebook_allows->from_post);
		flickr_free_atomic(&facebook_allows->__uiserv_method);
		flickr_free_atomic(&facebook_allows->grantEmailType);
	}
}


int  __fill_facebook_login_paras(const char *value,int index,facebook_login_paras_t *facebook_paras)
{
	switch(index){
		case FACEBOOK_LOGINATTEMPT:
			flickr_fill_atomic(value,&facebook_paras->login_attempt);
			break;
		case FACEBOOK_POPUP:
			flickr_fill_atomic(value,&facebook_paras->popup);
			break;
		case FACEBOOK_FBCONNECT:
			flickr_fill_atomic(value,&facebook_paras->fbconnect);
			break;
		case FACEBOOK_DISPLAY:
			flickr_fill_atomic(value,&facebook_paras->display);
			break;
		case FACEBOOk_NEXT:
			flickr_fill_atomic(value,&facebook_paras->next);
			break;
		case FACEBOOK_REQPERMS:
			flickr_fill_atomic(value,&facebook_paras->req_perms);
			break;
		case FACEBOOK_LEGACYRETURN:
			flickr_fill_atomic(value,&facebook_paras->legacy_return);
			break;
		case FACEBOOK_LSD:
			flickr_fill_atomic(value,&facebook_paras->lsd);
			break;
		case FACEBOOK_APIKEY:
			flickr_fill_atomic(value,&facebook_paras->api_key);
			break;
		case FACEBOOK_RTURNSESSION:
			flickr_fill_atomic(value,&facebook_paras->return_session);
			break;
		case FACEBOOK_CANCEL_URL:
			flickr_fill_atomic(value,&facebook_paras->cancel_url);
			break;
		case FACEBOOK_SESSIONKEYONLY:
			flickr_fill_atomic(value,&facebook_paras->session_key_only);
			break;
		case FACEBOOK_SESSIONVERSION:
			flickr_fill_atomic(value,&facebook_paras->session_version);
			break;
		case FACEBOOK_TRYNUM:
			flickr_fill_atomic(value,&facebook_paras->try_num);
			break;
		case FACEBOOK_DEFAULT_PERSISTENT:
			flickr_fill_atomic(value,&facebook_paras->default_persistent);
			break;
		default:
			flickr_err("Index Error!");
			break;
	}
	return 0;
}


void __free_facebook_login_paras(facebook_login_paras_t *facebook_paras)
{
	if(facebook_paras){
		flickr_free_atomic(&facebook_paras->login_attempt);
		flickr_free_atomic(&facebook_paras->popup);
		flickr_free_atomic(&facebook_paras->fbconnect);
		flickr_free_atomic(&facebook_paras->display);
		flickr_free_atomic(&facebook_paras->next);
		flickr_free_atomic(&facebook_paras->req_perms);
		flickr_free_atomic(&facebook_paras->legacy_return);

		flickr_free_atomic(&facebook_paras->charset_test);
		flickr_free_atomic(&facebook_paras->lsd);
		flickr_free_atomic(&facebook_paras->api_key);
		flickr_free_atomic(&facebook_paras->return_session);
		flickr_free_atomic(&facebook_paras->cancel_url);
		flickr_free_atomic(&facebook_paras->session_key_only);
		flickr_free_atomic(&facebook_paras->session_version);
		flickr_free_atomic(&facebook_paras->try_num);
		flickr_free_atomic(&facebook_paras->email);
		flickr_free_atomic(&facebook_paras->pass);
		flickr_free_atomic(&facebook_paras->default_persistent);
		flickr_free_atomic(&facebook_paras->login);
	}
}


int  __fill_facebook_auth_paras(const char *value,int index,facebook_auth_paras_t *facebook_auth)
{
	switch(index){
		case FACEBOOK_AUTH_APPID:
			flickr_fill_atomic(value,&facebook_auth->app_id);
			break;
		case FACEBOOK_AUTH_NEXT:
			flickr_fill_atomic(value,&facebook_auth->next);
			break;
		case FACEBOOK_AUTH_DISPLAY:
			flickr_fill_atomic(value,&facebook_auth->display);
			break;
		case FACEBOOK_AUTH_CANCELURL:
			flickr_fill_atomic(value,&facebook_auth->cancel_url);
			break;
		case FACEBOOK_AUTH_LOCALE:
			flickr_fill_atomic(value,&facebook_auth->locale);
			break;
		case FACEBOOK_AUTH_PERMS:
			flickr_fill_atomic(value,&facebook_auth->perms);
			break;
		case FACEBOOK_AUTH_RETURNSESSION:
			flickr_fill_atomic(value,&facebook_auth->return_session);
			break;
		case FACEBOOK_AUTH_SESSIONVERSION:
			flickr_fill_atomic(value,&facebook_auth->session_version);
			break;
		case FACEBOOK_AUTH_FBCONNECT:
			flickr_fill_atomic(value,&facebook_auth->fbconnect);
			break;
		case FACEBOOK_AUTH_CANVAS:
			flickr_fill_atomic(value,&facebook_auth->canvas);
			break;
		case FACEBOOK_AUTH_LEGACYRETURN:
			flickr_fill_atomic(value,&facebook_auth->legacy_return);
			break;
		case FACEBOOK_AUTH_METHOD:
			flickr_fill_atomic(value,&facebook_auth->method);
			break;
		default:
			flickr_err("Index Error!");
			break;
	}
	return 0;
}

void __free_facebook_auth_paras(facebook_auth_paras_t * facebook_auth)
{
	if(facebook_auth){
		flickr_free_atomic(&facebook_auth->app_id);
		flickr_free_atomic(&facebook_auth->next);
		flickr_free_atomic(&facebook_auth->display);
		flickr_free_atomic(&facebook_auth->cancel_url);
		flickr_free_atomic(&facebook_auth->locale);
		flickr_free_atomic(&facebook_auth->perms);
		flickr_free_atomic(&facebook_auth->return_session);
		flickr_free_atomic(&facebook_auth->session_version);
		flickr_free_atomic(&facebook_auth->fbconnect);
		flickr_free_atomic(&facebook_auth->canvas);
		flickr_free_atomic(&facebook_auth->legacy_return);
		flickr_free_atomic(&facebook_auth->method);
	}
}


char* flickr_decode_java_str(char* java_data,char *dec_str)
{
	char * tmp_str=NULL;
	char * next_str=NULL;
	char * str_get=NULL;
	flickr_data_write_t * str_decoded;
	int str_len = 0;
	int i=0,j=0;
	str_len = strlen(dec_str);
	next_str = tmp_str = java_data;
	str_decoded = (flickr_data_write_t*)flickr_malloc(sizeof(flickr_data_write_t));
	if(str_decoded){
		flickr_data_write_init(str_decoded,flickcurl_data_write_buffer,NULL);
		flickr_info("<des_str>%s",dec_str);
		while(next_str!=NULL){
			next_str = strstr(tmp_str,dec_str);
			if(next_str){
				str_decoded = flickr_strn_append(str_decoded,tmp_str,next_str-tmp_str);
				str_decoded = flickr_str_append(str_decoded,"%");
				tmp_str = next_str+str_len;
			}
		}
		str_decoded = flickr_str_append(str_decoded,tmp_str);
		flickr_info("<decode>%s",str_decoded->data_head);
		str_get = (char*)flickr_malloc(str_decoded->data_used);
		if(str_get){
			memset(str_get,0,str_decoded->data_used);
			for(i=0;i<str_decoded->data_used;i++){
				if(str_decoded->data_head[i]!='\\'){
					str_get[j]= *(str_decoded->data_head+i);
					j++;
				}
			}
		}
		flickr_data_write_free(str_decoded);
		flickr_free((char*)str_decoded);
	}
	return str_get;
}

flickr_data_write_t * flickr_get_facebook_authallows_referfield(facebook_auth_allows_t* facebook_allows)
{
	flickr_data_write_t * post_field=NULL;
	char *str_encode=NULL;
	post_field = (flickr_data_write_t *)flickr_malloc(sizeof(flickr_data_write_t));
	if(post_field){
		flickr_data_write_init(post_field,flickcurl_data_write_buffer,NULL);	
		post_field = flickr_str_append(post_field,"Refer: http://www.facebook.com/connect/uiserver.php?app_id=");
		post_field = flickr_str_append(post_field,facebook_allows->app_id.data);
		post_field = flickr_str_append(post_field,"&next=");
		if(facebook_allows->redirect_uri.data){
			str_encode = flickr_get_URLEncode_str(facebook_allows->redirect_uri.data,"=?:/");
			if(str_encode){
				 flickr_str_append(post_field,str_encode);
				 flickr_free(str_encode);
				 str_encode = NULL;
			}
		}
		post_field = flickr_str_append(post_field,"&display=");
		post_field = flickr_str_append(post_field,facebook_allows->display.data);
		post_field = flickr_str_append(post_field,"&cancel_url=");
		if(facebook_allows->cancel_url.data){
			str_encode = flickr_get_URLEncode_str(facebook_allows->cancel_url.data,"=?:/");
			if(str_encode){
				 flickr_str_append(post_field,str_encode);
				 flickr_free(str_encode);
				 str_encode = NULL;
			}
		}
		post_field = flickr_str_append(post_field,"&locale=");
		post_field = flickr_str_append(post_field,facebook_allows->locale.data);	
		post_field = flickr_str_append(post_field,"&perms=");
		if(facebook_allows->perms.data){
			str_encode = flickr_get_URLEncode_str(facebook_allows->perms.data,",");
			if(str_encode){
				 flickr_str_append(post_field,str_encode);
				 flickr_free(str_encode);
				 str_encode = NULL;
			}
		}
		post_field = flickr_str_append(post_field,"&return_session=");
		post_field = flickr_str_append(post_field,facebook_allows->return_session.data);	
		post_field = flickr_str_append(post_field,"&session_version=");
		post_field = flickr_str_append(post_field,facebook_allows->session_version.data);	
		post_field = flickr_str_append(post_field,"&fbconnect=");
		post_field = flickr_str_append(post_field,facebook_allows->fbconnect.data);	
		post_field = flickr_str_append(post_field,"&canvas=");
		post_field = flickr_str_append(post_field,facebook_allows->canvas.data);	
		post_field = flickr_str_append(post_field,"&legacy_return=");
		post_field = flickr_str_append(post_field,facebook_allows->legacy_return.data);	
		post_field = flickr_str_append(post_field,"&method=");
		post_field = flickr_str_append(post_field,facebook_allows->__uiserv_method.data);	
	}
	return post_field;
}
void flickr_free_facebook_authallows_referfield(flickr_data_write_t * facebook_authallow_referfield)
{
	if(facebook_authallow_referfield){
		flickr_data_write_free(facebook_authallow_referfield);
		flickr_free((char*)facebook_authallow_referfield);
	}
}


flickr_data_write_t * flickr_get_facebook_authallows_postfield(facebook_auth_allows_t* facebook_allows)
{
	flickr_data_write_t * post_field=NULL;
	char *str_encode=NULL;
	post_field = (flickr_data_write_t *)flickr_malloc(sizeof(flickr_data_write_t));
	if(post_field){
		flickr_data_write_init(post_field,flickcurl_data_write_buffer,NULL);	
		post_field = flickr_str_append(post_field,"post_form_id=");
		post_field = flickr_str_append(post_field,facebook_allows->post_form_id.data);
		post_field = flickr_str_append(post_field,"&fb_dtsg=");
		post_field = flickr_str_append(post_field,facebook_allows->fb_dtsg.data);
		post_field = flickr_str_append(post_field,"&app_id=");
		post_field = flickr_str_append(post_field,facebook_allows->app_id.data);
		post_field = flickr_str_append(post_field,"&display=");
		post_field = flickr_str_append(post_field,facebook_allows->display.data);
		post_field = flickr_str_append(post_field,"&redirect_uri=");
		if(facebook_allows->redirect_uri.data){
			str_encode = flickr_get_URLEncode_str(facebook_allows->redirect_uri.data,"=?:/");
			if(str_encode){
				 flickr_str_append(post_field,str_encode);
				 flickr_free(str_encode);
				 str_encode = NULL;
			}
		}
		post_field = flickr_str_append(post_field,"&cancel_url=");
		if(facebook_allows->cancel_url.data){
			str_encode = flickr_get_URLEncode_str(facebook_allows->cancel_url.data,"=?:/");
			if(str_encode){
				 flickr_str_append(post_field,str_encode);
				 flickr_free(str_encode);
				 str_encode = NULL;
			}
		}
		post_field = flickr_str_append(post_field,"&locale=");
		post_field = flickr_str_append(post_field,facebook_allows->locale.data);	
		post_field = flickr_str_append(post_field,"&perms=");
		if(facebook_allows->perms.data){
			str_encode = flickr_get_URLEncode_str(facebook_allows->perms.data,",");
			if(str_encode){
				 flickr_str_append(post_field,str_encode);
				 flickr_free(str_encode);
				 str_encode = NULL;
			}
		}
		post_field = flickr_str_append(post_field,"&return_session=");
		post_field = flickr_str_append(post_field,facebook_allows->return_session.data);	
		post_field = flickr_str_append(post_field,"&session_version=");
		post_field = flickr_str_append(post_field,facebook_allows->session_version.data);	
		post_field = flickr_str_append(post_field,"&fbconnect=");
		post_field = flickr_str_append(post_field,facebook_allows->fbconnect.data);	
		post_field = flickr_str_append(post_field,"&canvas=");
		post_field = flickr_str_append(post_field,facebook_allows->canvas.data);	
		post_field = flickr_str_append(post_field,"&legacy_return=");
		post_field = flickr_str_append(post_field,facebook_allows->legacy_return.data);	
		post_field = flickr_str_append(post_field,"&from_post=");
		post_field = flickr_str_append(post_field,facebook_allows->from_post.data);	
		post_field = flickr_str_append(post_field,"&__uiserv_method=");
		post_field = flickr_str_append(post_field,facebook_allows->__uiserv_method.data);	
		post_field = flickr_str_append(post_field,"&GdpEmailBucket_grantEmailType=");
		post_field = flickr_str_append(post_field,facebook_allows->grantEmailType.data);	
		post_field = flickr_str_append(post_field,"&grant_clicked=Allow");
	}
	return post_field;
}

void flickr_free_facebook_authallows_postfield(flickr_data_write_t * facebook_authallow_postfield)
{
	if(facebook_authallow_postfield){
		flickr_data_write_free(facebook_authallow_postfield);
		flickr_free((char*)facebook_authallow_postfield);
	}
}


void flickr_free_facebook_auth_allows_para(facebook_auth_allows_t* facebook_allows)
{
	if(facebook_allows){
		__free_facebook_auth_allows_paras(facebook_allows);
		flickr_free((char*)facebook_allows);
	}
}


facebook_auth_allows_t* flickr_extract_facebook_auth_allows_para(flickr_gdata_t *gdata,char* src_data)
{
	facebook_auth_allows_t	* fb_allow_paras=NULL;
	char * value=NULL;
	char * currentpos=NULL;
	char * nextpos=NULL;
	int i=0;
	fb_allow_paras = (facebook_auth_allows_t*)flickr_malloc(sizeof(facebook_auth_allows_t));
	currentpos = src_data;
	if(fb_allow_paras){
		memset(fb_allow_paras,0,sizeof(facebook_auth_allows_t));
		for(i=0;i<FACEBOOK_ALLOW_MAX;i++){
			value = flickr_extract_str(facebook_auth_allow_str[i*2],facebook_auth_allow_str[i*2+1],currentpos,&nextpos);
			if(value){
				//flickr_info("<%d>%s",i,value);
				__fill_facebook_auth_allows_paras(value,i,fb_allow_paras);
				flickr_free(value);
				currentpos = nextpos;
			}
			if(nextpos==NULL)
				break;
		}
		if(i!=FACEBOOK_ALLOW_MAX){
			flickr_free_facebook_auth_allows_para(fb_allow_paras);
			fb_allow_paras=NULL;
		}
	}
	return fb_allow_paras;
}


flickr_data_write_t * flickr_get_facebook_login_postfield(facebook_login_paras_t* facebook_paras)
{
	flickr_data_write_t * post_field=NULL;
	char * tmp_str=NULL;
	char * next_tmp_str = NULL;
	post_field = (flickr_data_write_t *)flickr_malloc(sizeof(flickr_data_write_t));
	if(post_field){
		flickr_data_write_init(post_field,flickcurl_data_write_buffer,NULL);	
		post_field = flickr_str_append(post_field,"charset_test=%E2%82%AC%2C%C2%B4%2C%E2%82%AC%2C%C2%B4%2C%E6%B0%B4%2C%D0%94%2C%D0%84");
		post_field = flickr_str_append(post_field,"&lsd=");
		post_field = flickr_str_append(post_field,facebook_paras->lsd.data);
		post_field = flickr_str_append(post_field,"&next=");
		post_field = flickr_str_append(post_field,facebook_paras->next.data);
		post_field = flickr_str_append(post_field,"&api_key=");
		post_field = flickr_str_append(post_field,facebook_paras->api_key.data);
		post_field = flickr_str_append(post_field,"&return_session=");
		post_field = flickr_str_append(post_field,facebook_paras->return_session.data);
		post_field = flickr_str_append(post_field,"&cancel_url=");
		if(facebook_paras->cancel_url.data){
			tmp_str = flickr_get_URL_encoded(facebook_paras->cancel_url.data,'=');
			if(tmp_str!=NULL){
				next_tmp_str = flickr_get_URL_encoded(tmp_str,'?');
				flickr_free(tmp_str);
				if(next_tmp_str){
					post_field = flickr_str_append(post_field,next_tmp_str);
					flickr_free(next_tmp_str);	
				}
			}
		}
		post_field = flickr_str_append(post_field,"&req_perms=");
		post_field = flickr_str_append(post_field,facebook_paras->req_perms.data);
		post_field = flickr_str_append(post_field,"&legacy_return=");
		post_field = flickr_str_append(post_field,facebook_paras->legacy_return.data);
		post_field = flickr_str_append(post_field,"&display=");
		post_field = flickr_str_append(post_field,facebook_paras->display.data);
		post_field = flickr_str_append(post_field,"&session_key_only=");
		post_field = flickr_str_append(post_field,facebook_paras->session_key_only.data);
		post_field = flickr_str_append(post_field,"&session_version=");
		post_field = flickr_str_append(post_field,facebook_paras->session_version.data);
		post_field = flickr_str_append(post_field,"&trynum=");
		post_field = flickr_str_append(post_field,facebook_paras->try_num.data);
		post_field = flickr_str_append(post_field,"&charset_test=%E2%82%AC%2C%C2%B4%2C%E2%82%AC%2C%C2%B4%2C%E6%B0%B4%2C%D0%94%2C%D0%84");
		post_field = flickr_str_append(post_field,"&lsd=");
		post_field = flickr_str_append(post_field,facebook_paras->lsd.data);
		post_field = flickr_str_append(post_field,"&email=");
		post_field = flickr_str_append(post_field,facebook_paras->email.data);
		post_field = flickr_str_append(post_field,"&pass=");
		post_field = flickr_str_append(post_field,facebook_paras->pass.data);
		post_field = flickr_str_append(post_field,"&default_persistent=");
		post_field = flickr_str_append(post_field,facebook_paras->default_persistent.data);
		post_field = flickr_str_append(post_field,"&login=connect");
	}

	return post_field;
}


void  flickr_free_facebook_login_postfield(flickr_data_write_t * facebook_postfield)
{
	if(facebook_postfield){
		flickr_data_write_free(facebook_postfield);	
		flickr_free((char*)facebook_postfield);
	}
}
char* flickr_get_facebook_java_relocation(char* src_data)
{
	char * rel_addr=NULL;
	char *nextpos=NULL;
	char *rtn_value=NULL;
	rel_addr = flickr_extract_str("location.replace(\"","\");",src_data,&nextpos);
	if(rel_addr){
		rtn_value = flickr_decode_java_str(rel_addr,"\\u0025");
	}
	return rtn_value;
}

int flickr_connect_facebook_auth_rel(CURL * curl_handle, flickr_gdata_t * gdata, char * link_url)
{
	int rtn = FLICKR_RTN_OK;
	struct curl_slist *flickr_list_head=NULL;
	flickr_data_write_t receive_data;
	flickr_data_write_init(&receive_data,flickcurl_data_write_buffer,NULL);
	curl_easy_setopt(curl_handle,CURLOPT_HTTPGET,1L);
	curl_easy_setopt(curl_handle,CURLOPT_FOLLOWLOCATION, 1);
	//curl_easy_setopt(curl_handle,CURLOPT_COOKIE,NULL);
	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER,NULL);
	//curl_easy_setopt(curl_handle,CURLOPT_COOKIELIST,"FLUSH");
	curl_easy_setopt(curl_handle,CURLOPT_URL,link_url);
	flickr_info("<relocation>%s",link_url);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEDATA,&receive_data);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,flickr_func_write_data);
	//flickr_list_head = curl_slist_append(flickr_list_head,"Accept-Language: zh_CN");
	//flickr_list_head = curl_slist_append(flickr_list_head,"Accept-Encoding: gzip, deflate");
	//flickr_list_head = curl_slist_append(flickr_list_head,"Connection: Keep-Alive");
	flickr_list_head = curl_slist_append(flickr_list_head,"User-Agent: Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.1; Trident/4.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; Tablet PC 2.0");
	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER,flickr_list_head);

	
	curl_easy_perform(curl_handle);
	_test_save_to_file(receive_data.data_head,receive_data.data_used,"facebook_auth_allow_replace.html");
	flickr_data_write_free(&receive_data);

	if(flickr_list_head)
		curl_slist_free_all(flickr_list_head);
	return rtn;
}

int flickr_connect_facebook_allow(CURL* curl_handle,flickr_gdata_t *gdata,facebook_auth_allows_t* facebook_allows)
{
	int rtn = FLICKR_RTN_OK;
	struct curl_slist *flickr_list_head=NULL;
	flickr_data_write_t * post_field=NULL;
	flickr_data_write_t * refer_field=NULL;
	char * fb_java_relocation=NULL;
	flickr_data_write_t receive_data;
	post_field = flickr_get_facebook_authallows_postfield(facebook_allows);

	if(post_field==NULL){
		rtn = FLICKR_RTN_FACEBOOK_AUTHALLOW_PF_ERR;
		goto FACEBOOK_ALLOW_END;
	}

	refer_field = flickr_get_facebook_authallows_referfield(facebook_allows);
	if(refer_field==NULL){
		flickr_free_facebook_authallows_postfield(post_field);
		rtn = FLICKR_RTN_FACEBOOK_AUTHALLOW_PF_ERR;
		goto FACEBOOK_ALLOW_END;
	}
	flickr_data_write_init(&receive_data,flickcurl_data_write_buffer,NULL);
	curl_easy_setopt(curl_handle,CURLOPT_POST,1L);
	curl_easy_setopt(curl_handle,CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl_handle,CURLOPT_URL,"http://www.facebook.com/connect/uiserver.php");
	curl_easy_setopt(curl_handle, CURLOPT_COOKIELIST,"FLUSH");
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS,post_field->data_head);
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE,post_field->data_used);
	
	flickr_info("<Allow ReferFiled>%s",refer_field->data_head);
	flickr_info("<Allow PostFiled>%s",post_field->data_head);
	flickr_list_head = curl_slist_append(flickr_list_head,refer_field->data_head);
	flickr_list_head = curl_slist_append(flickr_list_head,"Connection: Keep-Alive");
	flickr_list_head = curl_slist_append(flickr_list_head,"Pragma: no-cache");
	flickr_list_head = curl_slist_append(flickr_list_head,"User-Agent: Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.1; Trident/4.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; Tablet PC 2.0");
	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER,flickr_list_head);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEDATA,&receive_data);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,flickr_func_write_data);
	curl_easy_perform(curl_handle);
	
	_test_save_to_file(receive_data.data_head,receive_data.data_used,"facebook_auth_allow.html");

	fb_java_relocation = flickr_get_facebook_java_relocation(receive_data.data_head);

	if(post_field)
		flickr_free_facebook_authallows_postfield(post_field);
	if(refer_field)
		flickr_free_facebook_authallows_referfield(refer_field);
	
	flickr_data_write_free(&receive_data);
	if(fb_java_relocation){
		rtn = flickr_connect_facebook_auth_rel(curl_handle,gdata,fb_java_relocation);
		flickr_free(fb_java_relocation);
	}


	if(flickr_list_head)
		curl_slist_free_all(flickr_list_head);

FACEBOOK_ALLOW_END:
	return rtn;
	
}


flickr_data_write_t * flickr_get_facebook_auth_url(facebook_auth_paras_t* facebook_auth)
{
	flickr_data_write_t * post_field=NULL;
	char * tmp_str=NULL,*next_tmp_str=NULL;
	post_field = (flickr_data_write_t *)flickr_malloc(sizeof(flickr_data_write_t));
	if(post_field){
		flickr_data_write_init(post_field,flickcurl_data_write_buffer,NULL);	
		post_field = flickr_str_append(post_field,"http://www.facebook.com/connect/uiserver.php?app_id=");
		post_field = flickr_str_append(post_field,facebook_auth->app_id.data);
		post_field = flickr_str_append(post_field,"&next=");
		post_field = flickr_str_append(post_field,facebook_auth->next.data);
		post_field = flickr_str_append(post_field,"&display=");
		post_field = flickr_str_append(post_field,facebook_auth->display.data);
		post_field = flickr_str_append(post_field,"&cancel_url=");
		post_field = flickr_str_append(post_field,facebook_auth->cancel_url.data);

		post_field = flickr_str_append(post_field,"&locale=");
		post_field = flickr_str_append(post_field,facebook_auth->locale.data);
		post_field = flickr_str_append(post_field,"&perms=");
		post_field = flickr_str_append(post_field,facebook_auth->perms.data);
		post_field = flickr_str_append(post_field,"&return_session=");
		post_field = flickr_str_append(post_field,facebook_auth->return_session.data);
		post_field = flickr_str_append(post_field,"&session_version=");
		post_field = flickr_str_append(post_field,facebook_auth->session_version.data);
		post_field = flickr_str_append(post_field,"&fbconnect=");
		post_field = flickr_str_append(post_field,facebook_auth->fbconnect.data);
		post_field = flickr_str_append(post_field,"&canvas=");
		post_field = flickr_str_append(post_field,facebook_auth->canvas.data);
		post_field = flickr_str_append(post_field,"&legacy_return=");
		post_field = flickr_str_append(post_field,facebook_auth->legacy_return.data);
		post_field = flickr_str_append(post_field,"&method=");
		post_field = flickr_str_append(post_field,facebook_auth->method.data);
	}
	return post_field;
}

void  flickr_free_facebook_auth_url(flickr_data_write_t* facebook_auth_postfield)
{
	if(facebook_auth_postfield){
		flickr_data_write_free(facebook_auth_postfield);
		flickr_free((char*)facebook_auth_postfield);
	}
}

void flickr_free_facebook_auth_paras(facebook_auth_paras_t* facebook_auth)
{
	if(facebook_auth){
		__free_facebook_auth_paras(facebook_auth);
		flickr_free((char*)facebook_auth);
	}
}

facebook_auth_paras_t	*flickr_extract_facebook_auth_paras(flickr_gdata_t *gdata,char* src_data)
{
	facebook_auth_paras_t	* facebook_auth=NULL;
	char * value=NULL;
	char * currentpos=NULL;
	char * nextpos=NULL;
	int i=0;
	facebook_auth = (facebook_auth_paras_t*)flickr_malloc(sizeof(facebook_auth_paras_t));
	currentpos = src_data;
	if(facebook_auth){
		memset(facebook_auth,0,sizeof(facebook_auth_paras_t));
		for(i=0;i<FACEBOOK_AUTH_MAX;i++){
			value = flickr_extract_str(facebook_auth_paras[i*2],facebook_auth_paras[i*2+1],currentpos,&nextpos);
			if(value){
				//flickr_info("<%d>%s",i,value);
				__fill_facebook_auth_paras(value,i,facebook_auth);
				flickr_free(value);
				currentpos = nextpos;
			}
			if(nextpos==NULL)
				break;
		}
		if(i!=FACEBOOK_AUTH_MAX){
			flickr_free_facebook_auth_paras(facebook_auth);
			facebook_auth=NULL;
		}
	}
	return facebook_auth;
}



flickr_data_write_t * flickr_get_facebook_login_postUrl(facebook_login_paras_t* facebook_paras)
{
	flickr_data_write_t * post_field=NULL;
	post_field = (flickr_data_write_t *)flickr_malloc(sizeof(flickr_data_write_t));
	if(post_field){
		flickr_data_write_init(post_field,flickcurl_data_write_buffer,NULL);	
		post_field = flickr_str_append(post_field,"https://www.facebook.com/login.php?login_attempt=");
		post_field = flickr_str_append(post_field,facebook_paras->login_attempt.data);
		post_field = flickr_str_append(post_field,"&popup=");
		post_field = flickr_str_append(post_field,facebook_paras->popup.data);
		post_field = flickr_str_append(post_field,"&display=");
		post_field = flickr_str_append(post_field,facebook_paras->display.data);
		post_field = flickr_str_append(post_field,"&next=");
		post_field = flickr_str_append(post_field,facebook_paras->next.data);
		post_field = flickr_str_append(post_field,"&req_perms=");
		post_field = flickr_str_append(post_field,facebook_paras->req_perms.data);
		post_field = flickr_str_append(post_field,"&legacy_return=");
		post_field = flickr_str_append(post_field,facebook_paras->legacy_return.data);	
	}
	return post_field;
}

void  flickr_free_facebook_login_postUrl(flickr_data_write_t * facebook_posturl)
{
	if(facebook_posturl){
		flickr_data_write_free(facebook_posturl);	
		flickr_free((char*)facebook_posturl);
	}
}


void flickr_free_facebook_login_paras(facebook_login_paras_t	* facebook_paras)
{
	if(facebook_paras){
		__free_facebook_login_paras(facebook_paras);
		flickr_free((char*)facebook_paras);
	}
}



facebook_login_paras_t	*flickr_extract_facebook_login_paras(flickr_gdata_t *gdata,char* src_data)
{
	facebook_login_paras_t	* facebook_paras=NULL;
	char * value=NULL;
	char * currentpos=NULL;
	char * nextpos=NULL;
	int i=0;
	facebook_paras = (facebook_login_paras_t*)flickr_malloc(sizeof(facebook_login_paras_t));
	currentpos = src_data;
	if(facebook_paras){
		memset(facebook_paras,0,sizeof(facebook_login_paras_t));
		flickr_fill_atomic(gdata->user_email.data,&facebook_paras->email);
		flickr_fill_atomic(gdata->user_pwd.data,&facebook_paras->pass);
		for(i=0;i<FACEBOOK_MAX;i++){
			value = flickr_extract_str(facebook_login_paras[i*2],facebook_login_paras[i*2+1],currentpos,&nextpos);
			if(value){
				//flickr_info("<%d>%s",i,value);
				__fill_facebook_login_paras(value,i,facebook_paras);
				flickr_free(value);
				currentpos = nextpos;
			}
			if(nextpos==NULL)
				break;
		}
		if(i!=FACEBOOK_MAX){
			flickr_free_facebook_login_paras(facebook_paras);
			facebook_paras=NULL;
		}
	}
	return facebook_paras;
}


int flickr_connect_facebook_auth(CURL* curl_handle,flickr_gdata_t *gdata,facebook_auth_paras_t* facebook_auth)
{
	int rtn = FLICKR_RTN_OK;
	flickr_data_write_t * get_url=NULL;
	facebook_auth_allows_t* fb_auth_allows=NULL;
	flickr_data_write_t receive_data;
	get_url = flickr_get_facebook_auth_url(facebook_auth);

	if(get_url==NULL){
		rtn = FLICKR_RTN_FACEBOOK_AUTH_PF_ERR;
		goto FACEBOOK_AUTH_END;
	}
	
	flickr_data_write_init(&receive_data,flickcurl_data_write_buffer,NULL);
	curl_easy_setopt(curl_handle,CURLOPT_HTTPGET,1L);
	curl_easy_setopt(curl_handle,CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl_handle,CURLOPT_URL,get_url->data_head);

	flickr_info("<Facebook AUTH URL>%s",get_url->data_head);
	receive_data.cancel_write = gdata->is_auth_cancel; ///< check whether the auth should be cancelled
	curl_easy_setopt(curl_handle,CURLOPT_WRITEDATA,&receive_data);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,flickr_func_write_data);
	curl_easy_perform(curl_handle);

	_test_save_to_file(receive_data.data_head,receive_data.data_used,"facebook_auth.html");
	
	if(get_url)
		flickr_free_facebook_auth_url(get_url);
	
	fb_auth_allows = flickr_extract_facebook_auth_allows_para(gdata,receive_data.data_head);
	flickr_data_write_free(&receive_data);

	if(fb_auth_allows){
		rtn = flickr_connect_facebook_allow(curl_handle,gdata,fb_auth_allows);		
		flickr_free_facebook_auth_allows_para(fb_auth_allows);
	}

	flickr_info("#########################################");

FACEBOOK_AUTH_END:
	return rtn;
}


int flickr_connect_facebook_login(CURL* curl_handle,flickr_gdata_t *gdata,facebook_login_paras_t* facebook_paras)
{
	int rtn = FLICKR_RTN_OK;
	facebook_auth_paras_t	* facebook_auth=NULL;
	struct curl_slist *flickr_list_head=NULL;
	flickr_data_write_t * post_field=NULL;
	flickr_data_write_t * post_url=NULL;
	flickr_data_write_t receive_data;
	post_field = flickr_get_facebook_login_postfield(facebook_paras);
	if(post_field==NULL){
		rtn = FLICKR_RTN_FACEBOOK_PF_ERR;
		goto FACEBOOK_LOGIN_END;
	}
	post_url = flickr_get_facebook_login_postUrl(facebook_paras);
	if(post_url==NULL){
		flickr_free_facebook_login_postfield(post_field);
		rtn = FLICKR_RTN_FACEBOOK_PF_ERR;
		goto FACEBOOK_LOGIN_END;
	}
	flickr_data_write_init(&receive_data,flickcurl_data_write_buffer,NULL);
	curl_easy_setopt(curl_handle,CURLOPT_POST,1L);
	curl_easy_setopt(curl_handle,CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl_handle,CURLOPT_URL,post_url->data_head);
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS,post_field->data_head);
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE,post_field->data_used);

	flickr_info("<Facebook LOGIN URL>%s",post_url->data_head);
	flickr_info("<Facebook PostField>%s",post_field->data_head);
	
	flickr_list_head = curl_slist_append(flickr_list_head,"User-Agent: Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.1; Trident/4.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; Tablet PC 2.0");
	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER,flickr_list_head);


	receive_data.cancel_write = gdata->is_auth_cancel; ///< check whether the auth should be cancelled
	curl_easy_setopt(curl_handle,CURLOPT_WRITEDATA,&receive_data);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,flickr_func_write_data);
	curl_easy_perform(curl_handle);

	_test_save_to_file(receive_data.data_head,receive_data.data_used,"facebook_login_pwd.html");

	if(post_url)
		flickr_free_facebook_login_postUrl(post_url);
	if(post_field)
		flickr_free_facebook_login_postfield(post_field);

	if(receive_data.data_used!=0)
		facebook_auth = flickr_extract_facebook_auth_paras(gdata,receive_data.data_head);
	flickr_data_write_free(&receive_data);

	if(facebook_auth){
		rtn = flickr_connect_facebook_auth(curl_handle,gdata,facebook_auth);
		flickr_free_facebook_auth_paras(facebook_auth);
	}
	else
		rtn =FLICKR_RTN_FACEBOOK_AUTH_ERR;

	if(flickr_list_head)
		curl_slist_free_all(flickr_list_head);
FACEBOOK_LOGIN_END:
	return rtn;
	
}


int flickr_connect_facebook_redir(CURL* curl_handle,flickr_gdata_t *gdata,char* link_url)
{
	flickr_data_write_t receive_data;
	struct curl_slist *flickr_list_head=NULL;
	int rtn = FLICKR_RTN_OK;
	facebook_login_paras_t	* facebook_paras=NULL;
	
	flickr_info("<size=%d>facebook_link=%s",strlen(link_url),link_url);
	flickr_data_write_init(&receive_data,flickcurl_data_write_buffer,NULL);	
	curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(curl_handle, CURLOPT_URL,link_url);

	receive_data.cancel_write = gdata->is_auth_cancel; ///< check whether the auth should be cancelled
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA,&receive_data);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION,flickr_func_write_data);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER,NULL);
	curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION,NULL);
	curl_easy_setopt(curl_handle, CURLOPT_COOKIE,NULL);
	curl_easy_setopt(curl_handle, CURLOPT_COOKIELIST,"FLUSH");
	flickr_list_head = curl_slist_append(flickr_list_head,"Connection: Keep-Alive");
	flickr_list_head = curl_slist_append(flickr_list_head,"User-Agent: Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.1; Trident/4.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; Tablet PC 2.0");
	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER,flickr_list_head);
	
	curl_easy_perform(curl_handle);
	_test_save_to_file(receive_data.data_head,receive_data.data_used,"facebook_redir.html");

	if(receive_data.data_used!=0)
		facebook_paras = flickr_extract_facebook_login_paras(gdata,receive_data.data_head);

	flickr_data_write_free(&receive_data);

	if(facebook_paras){
		rtn = flickr_connect_facebook_login(curl_handle,gdata,facebook_paras);
		flickr_free_facebook_login_paras(facebook_paras);
	}
	else
		rtn = FLICKR_RTN_FACEBOOK_REDIR_ERR;

	if(flickr_list_head)
		curl_slist_free_all(flickr_list_head);

CONNECT_FACEBOOK_REDIR_END:
	return rtn;
}


char * flickr_get_redir_facebook(char*src_header)
{
	int size_head=0,size=0,sizecopy=0;
	const char new_host[]="https://www.facebook.com/";
	char * last_link_url=NULL;
	char * version_find=NULL;
	size = strlen(src_header);
	size_head = strlen("https://www.facebook.com/");
	last_link_url = (char*)flickr_malloc(size);
	
	if(last_link_url!=NULL){
		flickr_info("<size=%d>",size);
		memset(last_link_url,0,size);
		version_find = strstr(src_header,"&v=1.0");
		if(version_find){
			sprintf(last_link_url,"%s",new_host);
			sizecopy = version_find-src_header -size_head;
			memcpy(last_link_url+strlen(new_host),src_header+size_head,sizecopy);
			strcat(last_link_url,"&v=1.0&locale=en_US"); 
		}
		else{
			flickr_free(last_link_url);
			last_link_url = NULL;
		}
	}
	return last_link_url;
}

int  flickr_connect_facebook_relocation(CURL* curl_handled,flickr_gdata_t *gdata,char* link_url)
{

	flickr_data_write_t receive_data;
	struct curl_slist *flickr_list_head=NULL;
	CURL* curl_handle = curl_easy_init();//fc->curl_handle;
	if(curl_handle==NULL){
		flickr_err("Init Curl Error!");
	}
	curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl_handle, CURLOPT_COOKIEJAR,flickr_cookie_addr);
	//curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST,"GET");;
	curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
	flickr_info("<Rel>%s",link_url);
	curl_easy_setopt(curl_handle, CURLOPT_URL,link_url);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA,&receive_data);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION,flickr_func_write_data);

	flickr_list_head = curl_slist_append(flickr_list_head,"Cookie: L=2; W=1305511315; act=1305510617414%2F5; x-referer=http%3A%2F%2Fwww.facebook.com%2Fprofile.php%3Fid%3D100002247546062%23%2Fprofile.php%3Fid%3D100002247546062");
	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER,flickr_list_head);
	curl_easy_perform(curl_handle);
	_test_save_to_file(receive_data.data_head,receive_data.data_used,"facebook_relocation.html");

	flickr_data_write_free(&receive_data);
	return 0;
}



int flickr_connect_facebook(CURL* curl_handle,flickr_gdata_t *gdata,char* link_url)
{
	int rtn = FLICKR_RTN_OK;
	flickr_data_write_t receive_data;
	flickr_data_write_t relocation_data;
	char * redir_facebook_login=NULL;
	flickr_login_para_t para;
	struct curl_slist *flickr_list_facebook_head=NULL;
	
	flickr_data_write_init(&receive_data,flickcurl_data_write_buffer,NULL);	
	flickr_data_write_init(&relocation_data,flickcurl_data_write_buffer,NULL);
	
	curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(curl_handle, CURLOPT_URL,link_url);

	receive_data.cancel_write = gdata->is_auth_cancel; ///< check whether the auth should be cancelled
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA,&receive_data);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION,flickr_func_write_data);

	curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER,&relocation_data);
	curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION,flickr_func_write_header);

	curl_easy_perform(curl_handle);

	_test_save_to_file(receive_data.data_head,receive_data.data_used,"facebook_login.html");

	flickr_data_write_free(&receive_data);

	if(relocation_data.data_used!=0){
		flickr_info("<Relocation>%s",relocation_data.data_head);
		redir_facebook_login = flickr_get_redir_facebook(relocation_data.data_head);
		flickr_data_write_free(&relocation_data);
		if(redir_facebook_login){
			rtn = flickr_connect_facebook_redir(curl_handle,gdata,redir_facebook_login);
			//rtn = flickr_connect_facebook_relocation(curl_handle,gdata,redir_facebook_login);
			rtn = 0;
		}
		else
			rtn = FLICKR_RTN_FACEBOOK_REDIR_ERR;
	}
	else
		rtn = FLICKR_RTN_FACEBOOK_LOGIN_ERR;

CONNECT_FACEBOOK_END:
	return rtn;
}
