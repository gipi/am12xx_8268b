#include "flickr_common.h"

const char yahoo_frob_str[]="http://baidu.com?frob=";

const char yahoo_login_para[YAHOO_MAX*2][32]={
	"name=\".tries\" value=\"","\">",
	"name=\".src\" value=\"","\">",
	"name=\".md5\" value=\"","\">",
	"name=\".hash\" value=\"","\">",
	"name=\".js\" value=\"","\">",
	"name=\".last\" value=\"","\">",
	"name=\"promo\" value=\"","\">",
	"name=\".intl\" value=\"","\">",
	"name=\".bypass\" value=\"","\">",
	"name=\".partner\" value=\"","\">",
	"name=\".u\" value=\"","\">",
	"name=\".v\" value=\"","\">",
	"name=\".challenge\" value=\"","\">",
	"name=\".yplus\" value=\"","\">",
	"name=\".emailCode\" value=\"","\">",
	"name=\"pkg\" value=\"","\">",
	"name=\"stepid\" value=\"","\">",
	"name=\".ev\" value=\"","\">",
	"name=\"hasMsgr\" value=\"","\">",
	"name=\".chkP\" value=\"","\">",
	"name=\".done\" value=\"","\">",
	"name=\".pd\" value=\"","\">",
	"id=\"pad\" value=\"","\">",
	"id=\"aad\" value=\"","\">",
	"id=\"popup\" value=\"","\">",
};

const char auth_request_strs[AUTHREQ_MAX*2][32]={
	"name=\"magic_cookie\" value=\"","\" />",
	"name=\"perms\" value=\"","\">",
	"name=\"api_key\" value=\"","\">",
	"name=\"api_sig\" value=\"","\">",
	"name=\"frob\" value=\"","\">",
	"name=\"source\" value=\"","\">",
	"name=\"done_auth\" value=\"","\">",
};

int __fill_auth_request_paras(const char *value,int index,flickr_auth_request_t * auth_request)
{
	switch(index){
		case AUTHREQ_MAGICCOOKIE:
			flickr_fill_atomic(value,&auth_request->magic_cookie);
			break;
		case AUTHREQ_PREMS:
			flickr_fill_atomic(value,&auth_request->perms);
			break;
		case AUTHREQ_API_KEY:
			flickr_fill_atomic(value,&auth_request->api_key);
			break;
		case AUTHREQ_API_SIG:
			flickr_fill_atomic(value,&auth_request->api_sig);
			break;
		case AUTHREQ_FROB:
			flickr_fill_atomic(value,&auth_request->frob);
			break;
		case AUTHREQ_SOURCE:
			flickr_fill_atomic(value,&auth_request->source);
			break;
		case AUTHREQ_DONEAUTH:
			flickr_fill_atomic(value,&auth_request->done_auth);
			break;
		default:
			flickr_err("Index Error!");
	}
	return 0;
}

void __free_auth_request_paras(flickr_auth_request_t * auth_request)
{
	if(auth_request){
		flickr_free_atomic(&auth_request->magic_cookie);
		flickr_free_atomic(&auth_request->perms);
		flickr_free_atomic(&auth_request->api_key);
		flickr_free_atomic(&auth_request->api_sig);
		flickr_free_atomic(&auth_request->done_auth);
		flickr_free_atomic(&auth_request->source);
		flickr_free_atomic(&auth_request->frob);
	}
}

 int __fill_yahoo_login_paras(const char *value,int index,flickr_yahoo_login_para_t *yahoo_paras)
{
	switch(index){
		case YAHOO_TRIES:
			flickr_fill_atomic(value,&yahoo_paras->tries);
			break;
		case YAHOO_SRC:
			flickr_fill_atomic(value,&yahoo_paras->src);
			break;
		case YAHOO_MD5:
			flickr_fill_atomic(value,&yahoo_paras->md5);
			break;
		case YAHOO_HASH:
			flickr_fill_atomic(value,&yahoo_paras->hash);
			break;
		case YAHOO_JS:
			flickr_fill_atomic(value,&yahoo_paras->js);
			break;
		case YAHOO_LAST:
			flickr_fill_atomic(value,&yahoo_paras->last);
			break;
		case YAHOO_PROMO:
			flickr_fill_atomic(value,&yahoo_paras->promo);
			break;
		case YAHOO_INTL:
			flickr_fill_atomic(value,&yahoo_paras->intl);
			break;
		case YAHOO_BYPASS:
			flickr_fill_atomic(value,&yahoo_paras->bypass);
			break;
		case YAHOO_PARTNER:
			flickr_fill_atomic(value,&yahoo_paras->partner);
			break;
		case YAHOO_U:
			flickr_fill_atomic(value,&yahoo_paras->u);
			break;
		case YAHOO_V:
			flickr_fill_atomic(value,&yahoo_paras->v);
			break;
		case YAHOO_CHALLENGE:
			flickr_fill_atomic(value,&yahoo_paras->challenge);
			break;
		case YAHOO_YPLUS:
			flickr_fill_atomic(value,&yahoo_paras->yplus);
			break;
		case YAHOO_EMAILCODE:
			flickr_fill_atomic(value,&yahoo_paras->emailCode);
			break;
		case YAHOO_PKG:
			flickr_fill_atomic(value,&yahoo_paras->pkg);
			break;
		case YAHOO_STEPID:
			flickr_fill_atomic(value,&yahoo_paras->stepid);
			break;
		case YAHOO_EV:
			flickr_fill_atomic(value,&yahoo_paras->ev);
			break;
		case YAHOO_HASMSGR:
			flickr_fill_atomic(value,&yahoo_paras->hasMsgr);
			break;
		case YAHOO_CHKP:
			flickr_fill_atomic(value,&yahoo_paras->chkP);
			break;
		case YAHOO_DONE:
			flickr_fill_atomic(value,&yahoo_paras->done);
			break;
		case YAHOO_PD:
			flickr_fill_atomic(value,&yahoo_paras->pd);
			break;
		case YAHOO_PAD:
			flickr_fill_atomic(value,&yahoo_paras->pad);
			break;
		case YAHOO_AAD:
			flickr_fill_atomic(value,&yahoo_paras->aad);
			break;
		case YAHOO_POPUP:
			flickr_fill_atomic(value,&yahoo_paras->popup);
			break;
		default:
			flickr_err("Index Is Error!");
	}
	return 0;
}

void __free_yahoo_login_paras(flickr_yahoo_login_para_t *yahoo_paras)
{
	if(yahoo_paras){
		flickr_free_atomic(&yahoo_paras->tries);
		flickr_free_atomic(&yahoo_paras->src);
		flickr_free_atomic(&yahoo_paras->md5);
		flickr_free_atomic(&yahoo_paras->hash);
		flickr_free_atomic(&yahoo_paras->js);
		flickr_free_atomic(&yahoo_paras->last);
		flickr_free_atomic(&yahoo_paras->promo);
		flickr_free_atomic(&yahoo_paras->intl);
		flickr_free_atomic(&yahoo_paras->bypass);
		flickr_free_atomic(&yahoo_paras->partner);
		flickr_free_atomic(&yahoo_paras->u);
		flickr_free_atomic(&yahoo_paras->v);
		flickr_free_atomic(&yahoo_paras->challenge);
		flickr_free_atomic(&yahoo_paras->yplus);
		flickr_free_atomic(&yahoo_paras->emailCode);
		flickr_free_atomic(&yahoo_paras->pkg);
		flickr_free_atomic(&yahoo_paras->stepid);
		flickr_free_atomic(&yahoo_paras->ev);
		flickr_free_atomic(&yahoo_paras->hasMsgr);
		flickr_free_atomic(&yahoo_paras->chkP);
		flickr_free_atomic(&yahoo_paras->done);
		flickr_free_atomic(&yahoo_paras->pd);
		flickr_free_atomic(&yahoo_paras->pad);
		flickr_free_atomic(&yahoo_paras->aad);
		flickr_free_atomic(&yahoo_paras->popup);
		flickr_free_atomic(&yahoo_paras->login);
		flickr_free_atomic(&yahoo_paras->passwd);
		flickr_free_atomic(&yahoo_paras->save);
	}
}



void flickr_free_auth_request(flickr_auth_request_t * auth_request)
{
	if(auth_request){
		__free_auth_request_paras(auth_request);
		flickr_free((char*)auth_request);
	}
}

flickr_auth_request_t * flickr_extract_auth_request(char* auth_reqeust_src)
{
	char * sub_reqeust_str=NULL;
	char * currentpos=NULL;
	char * nextpos=NULL;
	char * value=NULL;
	int i=0;
	flickr_auth_request_t *auth_request=NULL;
	auth_request =(flickr_auth_request_t*)flickr_malloc(sizeof(flickr_auth_request_t));
	if(auth_request==NULL)
		return auth_request;
	memset(auth_request,0,sizeof(flickr_auth_request_t));
	sub_reqeust_str = strstr(auth_reqeust_src,"method=\"post\"");
	//flickr_info("<SubReqStr>%s",sub_reqeust_str);
	if(sub_reqeust_str){
		currentpos = sub_reqeust_str;
		for(i=0;i<AUTHREQ_DONEAUTH;i++){
			value = flickr_extract_str(auth_request_strs[i*2],auth_request_strs[i*2+1],currentpos,&nextpos);
			if(value){
				flickr_info("<%s>%s",auth_request_strs[i*2],value);
				__fill_auth_request_paras(value,i,auth_request);
				flickr_free(value);
			}
			else
				break;
		}
		if(i!=AUTHREQ_DONEAUTH){
			flickr_free_auth_request(auth_request);
			auth_request =NULL;
		}
	}
	return auth_request;
}



void flickr_free_yahoo_paras(flickr_yahoo_login_para_t * yahoo_paras)
{
	if(yahoo_paras){
		__free_yahoo_login_paras(yahoo_paras);
		flickr_free((char*)yahoo_paras);
	}
}

 flickr_yahoo_login_para_t* flickr_extract_yahoo_paras(flickr_gdata_t *gdata,const char* src_data)
{
	flickr_yahoo_login_para_t * yahoo_paras=NULL;
	char * value=NULL;
	int i=0;
	char * currentpos=(char*)src_data;
	char * nextpos=NULL;
	yahoo_paras = (flickr_yahoo_login_para_t *)flickr_malloc(sizeof(flickr_yahoo_login_para_t));
	//flickr_info("!!!!!!!!!!!!!!!!!!!");
	if(yahoo_paras){
		memset(yahoo_paras,0,sizeof(flickr_yahoo_login_para_t));
		flickr_fill_atomic(gdata->user_email.data,&yahoo_paras->login);
		flickr_fill_atomic(gdata->user_pwd.data,&yahoo_paras->passwd);
		for(i=0;i<YAHOO_MAX;i++){
			value = flickr_extract_str(yahoo_login_para[i*2],yahoo_login_para[i*2+1],currentpos,&nextpos);
			if(value){
				//flickr_info("<%d>%s",i,value);
				__fill_yahoo_login_paras(value,i,yahoo_paras);
				flickr_free(value);
				currentpos = nextpos;
			}
			if(nextpos==NULL)
				break;
		}
		if(i!=YAHOO_MAX){
			flickr_info("Yahoo paras err");
			flickr_free_yahoo_paras(yahoo_paras);
			yahoo_paras = NULL;
		}
	}
	return yahoo_paras;
}


flickr_data_write_t * flickr_get_yahoo_para_postfield(flickr_yahoo_login_para_t* yahoo_paras)
{
	char *tmp_encoded_url=NULL;
	char *tmp_str=NULL;
	flickr_data_write_t * post_field=(flickr_data_write_t *)flickr_malloc(sizeof(flickr_data_write_t));
	if(post_field){
		flickr_data_write_init(post_field,flickcurl_data_write_buffer,NULL);

		post_field = flickr_str_append(post_field,".tries=");
		post_field = flickr_str_append(post_field,yahoo_paras->tries.data);
		post_field = flickr_str_append(post_field,"&.src=");
		post_field = flickr_str_append(post_field,yahoo_paras->src.data);
		post_field = flickr_str_append(post_field,"&.md5=");
		post_field = flickr_str_append(post_field,yahoo_paras->md5.data);
		post_field = flickr_str_append(post_field,"&.hash=");
		post_field = flickr_str_append(post_field,yahoo_paras->hash.data);
		post_field = flickr_str_append(post_field,"&.js=");
		post_field = flickr_str_append(post_field,yahoo_paras->js.data);
		post_field = flickr_str_append(post_field,"&.last=");
		post_field = flickr_str_append(post_field,yahoo_paras->last.data);
		post_field = flickr_str_append(post_field,"&promo=");
		post_field = flickr_str_append(post_field,yahoo_paras->promo.data);
		post_field = flickr_str_append(post_field,"&.intl=");
		post_field = flickr_str_append(post_field,yahoo_paras->intl.data);
		post_field = flickr_str_append(post_field,"&.bypass=");
		post_field = flickr_str_append(post_field,yahoo_paras->bypass.data);
		post_field = flickr_str_append(post_field,"&.partner=");
		post_field = flickr_str_append(post_field,yahoo_paras->partner.data);
		post_field = flickr_str_append(post_field,"&.u=");
		post_field = flickr_str_append(post_field,yahoo_paras->u.data);
		post_field = flickr_str_append(post_field,"&.v=");
		post_field = flickr_str_append(post_field,yahoo_paras->v.data);
		post_field = flickr_str_append(post_field,"&.challenge=");
		post_field = flickr_str_append(post_field,yahoo_paras->challenge.data);
		post_field = flickr_str_append(post_field,"&.yplus=");
		post_field = flickr_str_append(post_field,yahoo_paras->yplus.data);
		post_field = flickr_str_append(post_field,"&.emailCode=");
		post_field = flickr_str_append(post_field,yahoo_paras->emailCode.data);
		post_field = flickr_str_append(post_field,"&pkg=");
		post_field = flickr_str_append(post_field,yahoo_paras->pkg.data);
		post_field = flickr_str_append(post_field,"&stepid=");
		post_field = flickr_str_append(post_field,yahoo_paras->stepid.data);
		post_field = flickr_str_append(post_field,"&.ev=");
		post_field = flickr_str_append(post_field,yahoo_paras->ev.data);
		post_field = flickr_str_append(post_field,"&hasMsgr=");
		post_field = flickr_str_append(post_field,yahoo_paras->hasMsgr.data);
		post_field = flickr_str_append(post_field,"&.chkP=");
		post_field = flickr_str_append(post_field,yahoo_paras->chkP.data);
		post_field = flickr_str_append(post_field,"&.done=");
		#if 0
		if(yahoo_paras->done.data){
			char *next_tmp_str=NULL;
			//flickr_info("<0>%s",yahoo_paras->done.data);
			tmp_str = flickr_get_URL_encoded(yahoo_paras->done.data,'%');
			//flickr_info("<1>%s",tmp_str);
			if(tmp_str){
				next_tmp_str = flickr_get_URL_encoded(tmp_str,'=');
				//flickr_info("<2>%s",next_tmp_str);
				flickr_free(tmp_str);
				if(next_tmp_str){
					tmp_str = flickr_get_URL_encoded(next_tmp_str,'&');
					flickr_free(next_tmp_str);
					if(tmp_str){
						//flickr_info("<3>%s",tmp_str);
						post_field = flickr_str_append(post_field,tmp_str);
						flickr_free(tmp_str);
					}	
				}
			}
		}
		#endif
		if(yahoo_paras->done.data){
			tmp_str = flickr_get_URLEncode_str(yahoo_paras->done.data,"%=&");
			if(tmp_str){
				post_field = flickr_str_append(post_field,tmp_str);
				flickr_free(tmp_str);
			}
		}
		post_field = flickr_str_append(post_field,"&.pd=");
		post_field = flickr_str_append(post_field,yahoo_paras->pd.data);
		post_field = flickr_str_append(post_field,"&pad=");
		post_field = flickr_str_append(post_field,yahoo_paras->pad.data);
		post_field = flickr_str_append(post_field,"&aad=");
		post_field = flickr_str_append(post_field,yahoo_paras->aad.data);
		post_field = flickr_str_append(post_field,"&popup=");
		post_field = flickr_str_append(post_field,yahoo_paras->popup.data);
		post_field = flickr_str_append(post_field,"&login=");
		post_field = flickr_str_append(post_field,yahoo_paras->login.data);
		post_field = flickr_str_append(post_field,"&passwd=");
		post_field = flickr_str_append(post_field,yahoo_paras->passwd.data);
		post_field = flickr_str_append(post_field,"&.save=");
		post_field = flickr_str_append(post_field,yahoo_paras->save.data);
		post_field = flickr_str_append(post_field,"&passwd_raw=");
		post_field = flickr_str_append(post_field,yahoo_paras->passwd_raw.data);
	}

	return post_field;
}

void flickr_free_yahoo_para_postfield(flickr_data_write_t * yahoo_para_postfield)
{
	if(yahoo_para_postfield){
		flickr_data_write_free(yahoo_para_postfield);
		flickr_free((char*)yahoo_para_postfield);
	}
}

char * flickr_get_yahoo_redir(char*yahoo_redir_src)
{	
	char * nextpos=NULL;
	return flickr_extract_str("href=\"","\">",yahoo_redir_src,&nextpos);
}


void flickr_free_authreq_postfield(flickr_data_write_t *post_filed)
{
	if(post_filed){
		flickr_data_write_free(post_filed);
		flickr_free((char*)post_filed);
	}
}

flickr_data_write_t * flickr_get_authreq_postfield(flickr_auth_request_t * auth_request)
{
	char *tmp_encoded_url=NULL;
	flickr_data_write_t * post_field=(flickr_data_write_t *)flickr_malloc(sizeof(flickr_data_write_t));
	if(post_field){
		flickr_data_write_init(post_field,flickcurl_data_write_buffer,NULL);
		post_field = flickr_str_append(post_field,"magic_cookie=");
		post_field = flickr_str_append(post_field,auth_request->magic_cookie.data);
		post_field = flickr_str_append(post_field,"&perms=");
		post_field = flickr_str_append(post_field,auth_request->perms.data);
		post_field = flickr_str_append(post_field,"&api_key=");
		post_field = flickr_str_append(post_field,auth_request->api_key.data);
		post_field = flickr_str_append(post_field,"&api_sig=");
		post_field = flickr_str_append(post_field,auth_request->api_sig.data);
		post_field = flickr_str_append(post_field,"&frob=");
		post_field = flickr_str_append(post_field,auth_request->frob.data);
		if(auth_request->source.data){
			post_field = flickr_str_append(post_field,"&source=");
			post_field = flickr_str_append(post_field,auth_request->source.data);
		}
		if(auth_request->done_auth.data){
			post_field = flickr_str_append(post_field,"&done_auth=");
			post_field = flickr_str_append(post_field,auth_request->done_auth.data);
		}
	}
	return post_field;
}


void flickr_free_auth_done(flickr_auth_request_t * auth_done)
{
	if(auth_done){
		__free_auth_request_paras(auth_done);
		flickr_free((char*)auth_done);
	}
}

flickr_auth_request_t * flickr_extract_auth_done(char* auth_done_src)
{
	char * currentpos=NULL;
	char * nextpos=NULL;
	char * value=NULL;
	int i=0;
	flickr_auth_request_t *auth_done=NULL;
	auth_done =(flickr_auth_request_t*)flickr_malloc(sizeof(flickr_auth_request_t));
	if(auth_done==NULL)
		return auth_done;
	memset(auth_done,0,sizeof(flickr_auth_request_t));
	currentpos = auth_done_src;
	for(i=0;i<AUTHREQ_MAX;i++){
		if(i==AUTHREQ_SOURCE) //should skip the auth source because it is not exist in the auth done web
			continue;
		value = flickr_extract_str(auth_request_strs[i*2],auth_request_strs[i*2+1],currentpos,&nextpos);
		if(value){
			__fill_auth_request_paras(value,i,auth_done);
			flickr_free(value);
		}
		else
			break;
	}
	if(i!=AUTHREQ_MAX){
		flickr_free_auth_done(auth_done);
		auth_done =NULL;
	}
	return auth_done;
}

size_t yahoo_func_write_header( void *ptr, size_t size, size_t nmemb, void *userdata)
{
	size_t rev_data=size*nmemb;
	char * header_find=NULL;
	flickr_data_write_t *frob_data = (flickr_data_write_t*)userdata;
	//flickr_info("<Head size=%d>%s",rev_data,(char*)ptr);
	header_find = strstr(ptr,yahoo_frob_str);
	if(header_find){
		frob_data = flickr_str_append(frob_data,header_find+strlen(yahoo_frob_str));
		//flickr_info("<Real Frob>%s",frob_data->data_head);
	}
	return rev_data;
}


int flickr_connect_to_auth_done(CURL *curl_handle,flickr_gdata_t *gdata,flickr_auth_request_t * auth_done)
{
	int rtn=FLICKR_RTN_OK;
	flickr_data_write_t * authdone_postfield=NULL;
	flickr_data_write_t receive_data;
	flickr_data_write_t frob_data;
	flickr_data_write_init(&receive_data,flickcurl_data_write_buffer,NULL);
	flickr_data_write_init(&frob_data,flickcurl_data_write_buffer,NULL);
	authdone_postfield = flickr_get_authreq_postfield(auth_done);
	if(authdone_postfield==NULL){
		rtn = FLICKR_RTN_YAHOO_AUTHDONE_PF_ERR;
		goto YAHOO_AUTHDONE_END;
	}
		
	curl_easy_setopt(curl_handle,CURLOPT_POST,1L);
	curl_easy_setopt(curl_handle,CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl_handle,CURLOPT_URL,"http://www.flickr.com/services/auth/");
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS,authdone_postfield->data_head);
	flickr_info("<Post_field Size=%d>data=%s",authdone_postfield->data_used,authdone_postfield->data_head);
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE,authdone_postfield->data_used);

	curl_easy_setopt(curl_handle,CURLOPT_WRITEDATA,&receive_data);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,flickr_func_write_data);

	curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER,&frob_data);
	curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION,yahoo_func_write_header);
	
	curl_easy_perform(curl_handle);
	_test_save_to_file(receive_data.data_head,receive_data.data_used,"yahoo_authdone.html");

	flickr_free_authreq_postfield(authdone_postfield);
	if(receive_data.data_used!=0){
		flickr_fill_atomic(frob_data.data_head,&gdata->auth_frob);
		rtn = FLICKR_RTN_OK;
	}
	else
		rtn = FLICKR_RTN_YAHOO_AUTHDONE_ERR;
	flickr_data_write_free(&receive_data);
	flickr_data_write_free(&frob_data);
YAHOO_AUTHDONE_END:
	return rtn;
}


int flickr_connect_to_auth_request(CURL *curl_handle,flickr_gdata_t *gdata,flickr_auth_request_t * auth_request)
{
	int rtn = FLICKR_RTN_OK;
	flickr_data_write_t * authreq_postfield=NULL;
	flickr_auth_request_t * auth_done=NULL;	
	flickr_data_write_t receive_data;
	flickr_data_write_init(&receive_data,flickcurl_data_write_buffer,NULL);

	authreq_postfield = flickr_get_authreq_postfield(auth_request);
	if(authreq_postfield==NULL){
		rtn =  FLICKR_RTN_YAHOO_AUTHREQ_PF_ERR;
		goto YAHOO_AUTHREQ_END;
	}
	
	curl_easy_setopt(curl_handle,CURLOPT_POST,1L);
	curl_easy_setopt(curl_handle,CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl_handle,CURLOPT_URL,"http://www.flickr.com/services/auth/");
	curl_easy_setopt(curl_handle,CURLOPT_POSTFIELDS,authreq_postfield->data_head);
	flickr_info("<Post_field Size=%d>data=%s",authreq_postfield->data_used,authreq_postfield->data_head);
	curl_easy_setopt(curl_handle,CURLOPT_POSTFIELDSIZE,authreq_postfield->data_used);

	receive_data.cancel_write = gdata->is_auth_cancel; ///< check whether the auth should be cancelled
	curl_easy_setopt(curl_handle,CURLOPT_WRITEDATA,&receive_data);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,flickr_func_write_data);
	curl_easy_perform(curl_handle);
	_test_save_to_file(receive_data.data_head,receive_data.data_used,"yahoo_authrequest.html");
	
	flickr_free_authreq_postfield(authreq_postfield);
	if(receive_data.data_used!=0)
		auth_done = flickr_extract_auth_done(receive_data.data_head);

	flickr_data_write_free(&receive_data);
	if(auth_done){
		rtn = flickr_connect_to_auth_done(curl_handle,gdata,auth_done);
		flickr_free_auth_done(auth_done);
	}
	else
		rtn = FLICKR_RTN_YAHOO_AUTHDONE_ERR;

YAHOO_AUTHREQ_END:
	return rtn;
	
}

int flickr_connect_to_yahoo_redir(CURL *curl_handle,flickr_gdata_t*gdata,char* yahoo_redir)
{
	flickr_data_write_t receive_data;
	flickr_data_write_t frob_data;
	int rtn = FLICKR_RTN_OK;
	flickr_auth_request_t * auth_request=NULL;
	flickr_auth_request_t * auth_done=NULL;
	
	flickr_info("<Yahoo redir>%s",yahoo_redir);
	flickr_data_write_init(&receive_data,flickcurl_data_write_buffer,NULL);
	flickr_data_write_init(&frob_data,flickcurl_data_write_buffer,NULL);
	curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(curl_handle, CURLOPT_URL,yahoo_redir);

	receive_data.cancel_write = gdata->is_auth_cancel; ///< check whether the auth should be cancelled
	curl_easy_setopt(curl_handle,CURLOPT_WRITEDATA,&receive_data);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,flickr_func_write_data);

	curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER,&frob_data);
	curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION,yahoo_func_write_header);
	curl_easy_perform(curl_handle);
	_test_save_to_file(receive_data.data_head,receive_data.data_used,"/mnt/udisk/yahoo_redir.html");
 	/// the auth frob may appear in the header in the redir.html

	if(frob_data.data_head!=NULL){
		flickr_fill_atomic(frob_data.data_head,&gdata->auth_frob);
		flickr_data_write_free(&frob_data);
		flickr_data_write_free(&receive_data);
		rtn = FLICKR_RTN_OK;
		goto YAHOO_REDIR_END;
	}
	else{
		flickr_data_write_free(&frob_data);
		if(receive_data.data_used!=0){
			auth_request = flickr_extract_auth_request(receive_data.data_head);
		}
		else{ 
			rtn = FLICKR_RTN_YAHOO_REDIR_ERR;
			goto YAHOO_REDIR_END;
		}
			
		if(auth_request){	
			flickr_data_write_free(&receive_data);
			rtn = flickr_connect_to_auth_request(curl_handle,gdata,auth_request);
			flickr_free_auth_request(auth_request);
		}
		else{
			auth_done = flickr_extract_auth_done(receive_data.data_head);
			flickr_data_write_free(&receive_data);
			if(auth_done){
				rtn = flickr_connect_to_auth_done(curl_handle,gdata,auth_done);
				flickr_free_auth_done(auth_done);
			}
			else
				rtn = FLICKR_RTN_YAHOO_AUTHDONE_ERR;
		}
	}
YAHOO_REDIR_END:
 	return rtn;
	
}

int flickr_connect_yahoo(CURL* curl_handle,flickr_gdata_t *gdata, flickr_yahoo_login_para_t* yahoo_paras)
{
	int rtn = FLICKR_RTN_OK;
	flickr_data_write_t received_data;
	char * yahoo_redir=NULL;
	flickr_data_write_t * yahoo_para_postfield=NULL;
	yahoo_para_postfield = flickr_get_yahoo_para_postfield(yahoo_paras);
	flickr_data_write_init(&received_data,flickcurl_data_write_buffer,NULL);
	if(yahoo_para_postfield==NULL){
		rtn = FLICKR_RTN_YAHOO_LOGIN_ERR;
		goto CONNECT_YAHOO_END;
	}
	flickr_info("<Post_field Size=%d>%s",yahoo_para_postfield->data_used,yahoo_para_postfield->data_head);
	

	curl_easy_setopt(curl_handle,CURLOPT_POST,1L);
	curl_easy_setopt(curl_handle,CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl_handle,CURLOPT_URL,"https://login.yahoo.com/config/login?");
	curl_easy_setopt(curl_handle,CURLOPT_POSTFIELDS,yahoo_para_postfield->data_head);
	curl_easy_setopt(curl_handle,CURLOPT_POSTFIELDSIZE,yahoo_para_postfield->data_used);

	received_data.cancel_write = gdata->is_auth_cancel; ///< check whether the auth should be canceled
	curl_easy_setopt(curl_handle,CURLOPT_WRITEDATA,&received_data);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,flickr_func_write_data);
	curl_easy_perform(curl_handle);
	
	_test_save_to_file(received_data.data_head,received_data.data_used,"/mnt/udisk/yahoo_auth.html");

	if(received_data.data_used!=0)
		yahoo_redir = flickr_get_yahoo_redir(received_data.data_head);
	
	flickr_data_write_free(&received_data);
	if(yahoo_redir){
		rtn = flickr_connect_to_yahoo_redir(curl_handle,gdata,yahoo_redir);
		flickr_free(yahoo_redir);
	}
	else
		rtn = FLICKR_RTN_YAHOO_REDIR_ERR;
	
	if(yahoo_para_postfield)
		flickr_free_yahoo_para_postfield(yahoo_para_postfield);	
CONNECT_YAHOO_END:
	return rtn;
}

