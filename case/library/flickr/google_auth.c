#include "flickr_common.h"

const char google_login_para_strs[LOGIN_PARA_MAX*2][16]={///
	"\"continue\"","\"continue\"",
	"\"followup\"","\"followup\"",
	"\"service\"","\"service\"",
	"\"dsh\"","\"dsh\"",
	"\"ltmpl\"","\"ltmpl\"",
	"\"shdf\"","\"shdf\"",
	"\"hl\"","\"hl\"",
	"\"scc\"","\"scc\"",
	"\"timeStmp\"","\"timeStmp\"",
	"\"secTok\"","\"secTok\"",
	"\"GALX\"","",
	"\'rmShown\'","",
	"\"signIn\"","\"signIn\"",
	"\"asts\"",""
};

const char open_id_paras[OPENID_MAX*2][48]={
	"action=\"","\">",
	"name=\"openid.ns\" value=\"","\" />",
	"name=\"openid.mode\" value=\"","\" />",
	"name=\"openid.op_endpoint\" value=\"","\" />",
	"name=\"openid.response_nonce\" value=\"","\" />",
	"name=\"openid.return_to\" value=\"","\" />",
	"name=\"openid.assoc_handle\" value=\"","\" />",
	"name=\"openid.signed\" value=\"","\" />",
	"name=\"openid.sig\" value=\"","\" />",
	"name=\"openid.identity\" value=\"","\" />",
	"name=\"openid.claimed_id\" value=\"","\" />",
	"name=\"openid.ns.ext1\" value=\"","\" />",
	"name=\"openid.ext1.mode\" value=\"","\" />",
	"name=\"openid.ext1.type.fn\" value=\"","\" />",
	"name=\"openid.ext1.value.fn\" value=\"","\" />",
	"name=\"openid.ext1.type.gid\" value=\"","\" />",
	"name=\"openid.ext1.value.gid\" value=\"","\" />",
	"name=\"openid.ext1.type.em\" value=\"","\" />",
	"name=\"openid.ext1.value.em\" value=\"","\" />",
	"name=\"openid.ext1.type.lg\" value=\"","\" />",
	"name=\"openid.ext1.value.lg\" value=\"","\" />",
	"name=\"openid.ext1.type.ln\" value=\"","\" />",
	"name=\"openid.ext1.value.ln\" value=\"","\" />",
	"name=\"openid.ns.ext2\" value=\"","\" />",
	"name=\"openid.ext2.auth_time\" value=\"","\" />",
	"name=\"openid.ext2.auth_policies\" value=\"","\" />",
	"name=\"openid.ns.ext3\" value=\"","\" />",
	"name=\"openid.ext3.mode\" value=\"","\" />",
};


char *flickcurl_extract_google_relation(char* str_start,char* str_end,char* str_src,char**nextpos)
{
	return flickr_extract_str(str_start,str_end,str_src,nextpos);
}

int __fill_openid_paras(const char *value,int index,flickcurl_yahoo_openid_t *openid)
{
	switch(index){
		case OPENID_URL:
			flickr_fill_atomic(value,&openid->openid_url);
			break;
		case OPENID_NS:
			flickr_fill_atomic(value,&openid->ns);
			break;
		case OPENID_MODE:
			flickr_fill_atomic(value,&openid->mode);
			break;
		case OPENID_OP_ENDPOINT:
			flickr_fill_atomic(value,&openid->op_endpoint);
			break;
		case OPENID_RESPONSE_NONCE:
			flickr_fill_atomic(value,&openid->response_nonce);
			break;
		case OPENID_RETURN_TO:
			flickr_fill_atomic(value,&openid->return_to);
			break;
		case OPENID_ASSOC_HANDLE:
			flickr_fill_atomic(value,&openid->assoc_handle);
			break;
		case OPENID_SIGNED:
			flickr_fill_atomic(value,&openid->openid_signed);
			break;
		case OPENID_SIG:
			flickr_fill_atomic(value,&openid->sig);
			break;
		case OPENID_IDENTITY:
			flickr_fill_atomic(value,&openid->identity);
			break;
		case OPENID_CLAIMED_ID:
			flickr_fill_atomic(value,&openid->claimed_id);
			break;
		case OPENID_NS_EXT1:
			flickr_fill_atomic(value,&openid->ns_ext1);
			break;
		case OPENID_EXT1_MODE:
			flickr_fill_atomic(value,&openid->ext1_mode);
			break;
		case OPENID_EXT1_TYPE_FN:
			flickr_fill_atomic(value,&openid->ext1_type_fn);
			break;
		case OPENID_EXT1_VALUE_FN:
			flickr_fill_atomic(value,&openid->ext1_value_fn);
			break;
		case OPENID_EXT1_TYPE_GID:
			flickr_fill_atomic(value,&openid->ext1_type_gid);
			break;
		case OPENID_EXT1_VALUE_GID:
			flickr_fill_atomic(value,&openid->ext1_value_gid);
			break;
		case OPENID_EXT1_TYPE_EM:
			flickr_fill_atomic(value,&openid->ext1_type_em);
			break;
		case OPENID_EXT1_VALUE_EM:
			flickr_fill_atomic(value,&openid->ext1_value_em);
			break;
		case OPENID_EXT1_TYPE_LG:
			flickr_fill_atomic(value,&openid->ext1_type_lg);
			break;
		case OPENID_EXT1_VALUE_LG:
			flickr_fill_atomic(value,&openid->ext1_value_lg);
			break;
		case OPENID_EXT1_TYPE_LN:
			flickr_fill_atomic(value,&openid->ext1_type_ln);
			break;
		case OPENID_EXT1_VALUE_LN:
			flickr_fill_atomic(value,&openid->ext1_value_ln);
			break;
		case OPENID_NS_EXT2:
			flickr_fill_atomic(value,&openid->ns_ext2);
			break;
		case OPENID_EXT2_AUTH_TIME:
			flickr_fill_atomic(value,&openid->ext2_auth_time);
			break;
		case OPENID_EXT2_AUTH_POLICIES:
			flickr_fill_atomic(value,&openid->ext2_auth_policies);
			break;
		case OPENID_NS_EXT3:
			flickr_fill_atomic(value,&openid->ns_ext3);
			break;
		case OPENID_EXT3_MODE:
			flickr_fill_atomic(value,&openid->ext3_mode);
			break;
		default:
			flickr_err("Crazy Index Error!");
	}
	return 0;
}



void __free_openid_paras(flickcurl_yahoo_openid_t *openid)
{
	if(openid){

		flickr_free_atomic(&openid->openid_url);
		flickr_free_atomic(&openid->ns);
		flickr_free_atomic(&openid->mode);
		flickr_free_atomic(&openid->op_endpoint);
		flickr_free_atomic(&openid->response_nonce);
		flickr_free_atomic(&openid->return_to);
		flickr_free_atomic(&openid->assoc_handle);
		flickr_free_atomic(&openid->openid_signed);
		flickr_free_atomic(&openid->sig);
		flickr_free_atomic(&openid->identity);
		flickr_free_atomic(&openid->claimed_id);
		flickr_free_atomic(&openid->ns_ext1);
		flickr_free_atomic(&openid->ext1_mode);
		flickr_free_atomic(&openid->ext1_type_fn);
		flickr_free_atomic(&openid->ext1_value_fn);
		flickr_free_atomic(&openid->ext1_type_gid);
		flickr_free_atomic(&openid->ext1_value_gid);
		flickr_free_atomic(&openid->ext1_type_em);
		flickr_free_atomic(&openid->ext1_value_em);
		flickr_free_atomic(&openid->ext1_type_lg);
		flickr_free_atomic(&openid->ext1_value_lg);
		flickr_free_atomic(&openid->ext1_type_ln);
		flickr_free_atomic(&openid->ext1_value_ln);
		flickr_free_atomic(&openid->ns_ext2);
		flickr_free_atomic(&openid->ext2_auth_time);
		flickr_free_atomic(&openid->ext2_auth_policies);
		flickr_free_atomic(&openid->ns_ext3);
		flickr_free_atomic(&openid->ext3_mode);
	}
}


int __fill_login_para_element(const char *value,int index,flickr_login_para_t *paras)
{
	char cont_str[]="";
	char *cont_fill=NULL;
	if(value==NULL)
		cont_fill = cont_str;
	else{
		if(strcmp(value,"\'\'")==0)
			cont_fill = cont_str;
		else
			cont_fill = (char*)value;
	}
	flickr_info("<index=%d>cont=%s",index,cont_fill);
	switch(index){
		case LOGIN_PARA_CONTINUE:
			flickr_fill_atomic(cont_fill,&paras->url_continue);
			break;
		case LOGIN_PARA_FOLLOWUP:
			flickr_fill_atomic(cont_fill,&paras->url_followup);
			break;
		case LOGIN_PARA_SERVICE:
			flickr_fill_atomic(cont_fill,&paras->service);
			break;
		case LOGIN_PARA_DSH:
			flickr_fill_atomic(cont_fill,&paras->dsh);
			break;
		case LOGIN_PARA_LTMPL:
			flickr_fill_atomic(cont_fill,&paras->ltmpl);
			break;
		case LOGIN_PARA_SHDF:
			flickr_fill_atomic(cont_fill,&paras->shdf);
			break;
		case LOGIN_PARA_HL:
			flickr_fill_atomic(cont_fill,&paras->hl);
			break;
		case LOGIN_PARA_SCC:
			flickr_fill_atomic(cont_fill,&paras->scc);
			break;
		case LOGIN_PARA_TIMESTAMP:
			flickr_fill_atomic(cont_fill,&paras->timeStmp);
			break;
		case LOGIN_PARA_SECTOK:
			flickr_fill_atomic(cont_fill,&paras->secTok);
			break;
		case LOGIN_PARA_GALX:
			flickr_fill_atomic(cont_fill,&paras->GALX);
			break;
		case LOGIN_PARA_RMSHOWN:
			flickr_fill_atomic(cont_fill,&paras->rmShown);
			break;
		case LOGIN_PARA_SIGNIN:
			flickr_fill_atomic(cont_fill,&paras->signIn);
			break;
		case LOGIN_PARA_ASTS:
			flickr_fill_atomic(cont_fill,&paras->asts);
			break;
		default:
			flickr_err("Err Login Para");
			break;
	}
	return 0;
}

void __free_login_para_element(flickr_login_para_t *paras)
{
	if(paras){
		flickr_free_atomic(&paras->url_continue);
		flickr_free_atomic(&paras->url_followup);
		flickr_free_atomic(&paras->service);
		flickr_free_atomic(&paras->dsh);
		flickr_free_atomic(&paras->ltmpl);
		flickr_free_atomic(&paras->shdf);
		flickr_free_atomic(&paras->hl);
		flickr_free_atomic(&paras->scc);
		flickr_free_atomic(&paras->timeStmp);
		flickr_free_atomic(&paras->secTok);
		flickr_free_atomic(&paras->GALX);
		flickr_free_atomic(&paras->Email);
		flickr_free_atomic(&paras->Pwd);
		flickr_free_atomic(&paras->rmShown);
		flickr_free_atomic(&paras->signIn);
		flickr_free_atomic(&paras->asts);
	}
}


void flickr_free_openid(flickcurl_yahoo_openid_t* openid)
{
	if(openid){
		__free_openid_paras(openid);
		flickr_free((char*)openid);
	}
}

flickcurl_yahoo_openid_t *flickr_extract_openid(char* src_data)
{
	flickcurl_yahoo_openid_t * openid;
	char * value=NULL;
	int i=0;
	openid = (flickcurl_yahoo_openid_t *)flickr_malloc(sizeof(flickcurl_yahoo_openid_t));
	char * currentpos=src_data;
	char * nextpos=NULL;
	flickr_info("!!!!!!!!!!!!!!!!!!!");
	if(openid){
		memset(openid,0,sizeof(flickcurl_yahoo_openid_t));
		for(i=0;i<OPENID_MAX;i++){
			value = flickr_extract_str(open_id_paras[i*2],open_id_paras[i*2+1],currentpos,&nextpos);
			if(value){
				if(value)
					flickr_info("<%d>%s",i,value);
				__fill_openid_paras(value,i,openid);
				flickr_free(value);
				currentpos = nextpos;
			}
			else
				break;
		}
		if(i!=OPENID_MAX){
			flickr_info("Get OpenID Error!");
			flickr_free_openid(openid);
			openid = NULL;
		}
	}
	return openid;
}

flickr_data_write_t * flickr_get_openid_postfield(flickcurl_yahoo_openid_t * yahoo_openid)
{
	char *tmp_encoded_url=NULL;
	flickr_data_write_t * post_field=(flickr_data_write_t *)flickr_malloc(sizeof(flickr_data_write_t));
	if(post_field){
		flickr_data_write_init(post_field,flickcurl_data_write_buffer,NULL);
		post_field = flickr_str_append(post_field,"openid.ns=");
		post_field = flickr_str_append(post_field,yahoo_openid->ns.data);
		post_field = flickr_str_append(post_field,"&openid.mode=");
		post_field = flickr_str_append(post_field,yahoo_openid->mode.data);	
		post_field = flickr_str_append(post_field,"&openid.op_endpoint=");
		post_field = flickr_str_append(post_field,yahoo_openid->op_endpoint.data);
		post_field = flickr_str_append(post_field,"&openid.response_nonce=");
		post_field = flickr_str_append(post_field,yahoo_openid->response_nonce.data);	
		post_field = flickr_str_append(post_field,"&openid.return_to=");
		post_field = flickr_str_append(post_field,yahoo_openid->return_to.data);	
		post_field = flickr_str_append(post_field,"&openid.assoc_handle=");
		post_field = flickr_str_append(post_field,yahoo_openid->assoc_handle.data);	
		post_field = flickr_str_append(post_field,"&openid.signed=");
		
		#if 0
		tmp_encoded_url = flickr_get_URL_encoded(yahoo_openid->openid_signed.data,',');
		if(tmp_encoded_url){
			post_field = flickr_str_append(post_field,tmp_encoded_url);
			flickr_free_URL_encoded(tmp_encoded_url);
		}
		else
			flickr_err("Sorry URL Encoded Error OpenID!");
		#else
			post_field = flickr_str_append(post_field,yahoo_openid->openid_signed.data);		
		#endif
		post_field = flickr_str_append(post_field,"&openid.sig=");
		post_field = flickr_str_append(post_field,yahoo_openid->sig.data);	
		post_field = flickr_str_append(post_field,"&openid.identity=");
		post_field = flickr_str_append(post_field,yahoo_openid->identity.data);	
		post_field = flickr_str_append(post_field,"&openid.claimed_id=");
		post_field = flickr_str_append(post_field,yahoo_openid->claimed_id.data);	
		post_field = flickr_str_append(post_field,"&openid.ns.ext1=");
		post_field = flickr_str_append(post_field,yahoo_openid->ns_ext1.data);	
		post_field = flickr_str_append(post_field,"&openid.ext1.mode=");
		post_field = flickr_str_append(post_field,yahoo_openid->ext1_mode.data);	
		post_field = flickr_str_append(post_field,"&openid.ext1.type.fn=");
		post_field = flickr_str_append(post_field,yahoo_openid->ext1_type_fn.data);	
		post_field = flickr_str_append(post_field,"&openid.ext1.value.fn=");
		post_field = flickr_str_append(post_field,yahoo_openid->ext1_value_fn.data);	
		post_field = flickr_str_append(post_field,"&openid.ext1.type.gid=");
		post_field = flickr_str_append(post_field,yahoo_openid->ext1_type_gid.data);	
		post_field = flickr_str_append(post_field,"&openid.ext1.value.gid=");
		post_field = flickr_str_append(post_field,yahoo_openid->ext1_value_gid.data);	
		post_field = flickr_str_append(post_field,"&openid.ext1.type.em=");
		post_field = flickr_str_append(post_field,yahoo_openid->ext1_type_em.data);	
		post_field = flickr_str_append(post_field,"&openid.ext1.value.em=");
		post_field = flickr_str_append(post_field,yahoo_openid->ext1_value_em.data);	
		post_field = flickr_str_append(post_field,"&openid.ext1.type.lg=");
		post_field = flickr_str_append(post_field,yahoo_openid->ext1_type_lg.data);	
		post_field = flickr_str_append(post_field,"&openid.ext1.value.lg=");
		post_field = flickr_str_append(post_field,yahoo_openid->ext1_value_lg.data);	
		post_field = flickr_str_append(post_field,"&openid.ext1.type.ln=");
		post_field = flickr_str_append(post_field,yahoo_openid->ext1_type_ln.data);	
		post_field = flickr_str_append(post_field,"&openid.ext1.value.ln=");
		post_field = flickr_str_append(post_field,yahoo_openid->ext1_value_ln.data);	
		post_field = flickr_str_append(post_field,"&openid.ns.ext2=");
		post_field = flickr_str_append(post_field,yahoo_openid->ns_ext2.data);	
		post_field = flickr_str_append(post_field,"&openid.ext2.auth_time=");
		post_field = flickr_str_append(post_field,yahoo_openid->ext2_auth_time.data);	
		post_field = flickr_str_append(post_field,"&openid.ext2.auth_policies=");
		post_field = flickr_str_append(post_field,yahoo_openid->ext2_auth_policies.data);
		post_field = flickr_str_append(post_field,"&openid.ns.ext3=");
		post_field = flickr_str_append(post_field,yahoo_openid->ns_ext3.data);
		post_field = flickr_str_append(post_field,"&openid.ext3.mode=");
		post_field = flickr_str_append(post_field,yahoo_openid->ext3_mode.data);
	}
	return post_field;
}

void flickr_free_openid_postfield(flickr_data_write_t *openid_postfield)
{
	if(openid_postfield){
		flickr_data_write_free(openid_postfield);
		flickr_free((char*)openid_postfield);
	}
}


static struct curl_slist * flickr_get_refer(flickr_login_para_t *para)
{
	struct curl_slist *refer_field_list=NULL;
	char * tmpstr=NULL;
	flickr_data_write_t * refer_field=(flickr_data_write_t *)flickr_malloc(sizeof(flickr_login_para_t));
	if(refer_field==NULL)
		return refer_field_list;
	
	flickr_data_write_init(refer_field,flickcurl_data_write_buffer,NULL);
	refer_field = flickr_str_append(refer_field,"Referer: https://www.google.com/accounts/ServiceLogin?service=lso&passive=1209600&continue=https://accounts.google.com/o/openid2/auth?st%3D");
	refer_field = flickr_str_append(refer_field,para->url_continue.data);
	refer_field = flickr_str_append(refer_field,"%26from_login%3D1%26hl%3Dzh-TW");
	refer_field = flickr_str_append(refer_field,"&followup=https://accounts.google.com/o/openid2/auth?st%3D");
	refer_field = flickr_str_append(refer_field,para->url_followup.data);
	refer_field = flickr_str_append(refer_field,"%26from_login%3D1%26hl%3Dzh-TW");
	refer_field = flickr_str_append(refer_field,"&ltmpl=");
	refer_field = flickr_str_append(refer_field,para->ltmpl.data);
	refer_field = flickr_str_append(refer_field,"&shdf=");
	refer_field = flickr_str_append(refer_field,para->shdf.data);
	refer_field = flickr_str_append(refer_field,"&hl=");
	refer_field = flickr_str_append(refer_field,para->hl.data);
	refer_field = flickr_str_append(refer_field,"&scc=");
	refer_field = flickr_str_append(refer_field,para->scc.data);
	
	flickr_info("<Refer=>%s",refer_field->data_head);
	refer_field_list = curl_slist_append(refer_field_list,refer_field->data_head);
	
	flickr_data_write_free(refer_field);
	flickr_free((char*)refer_field);
	return refer_field_list;
}

static void __convert_to_URL_encoded(flickr_data_write_t * post_field,flickr_data_atomic_t *atomic)
{
	int i=0;
	char* tmpbuf=NULL;
	int tmpstr_len=0;
	if(atomic && atomic->data_len!=0){
		tmpstr_len = atomic->data_len*2;
		tmpbuf = (char*)flickr_malloc(tmpstr_len);
		if(tmpbuf){
			memset(tmpbuf,0,tmpstr_len);
			for(i=0;i<atomic->data_len-1;i++){
				tmpbuf[i*2]='%';
				tmpbuf[i*2+1]=atomic->data[i];
				printf("0x%2x ",tmpbuf[i*2+1]);
			}	
			flickr_info("Url Encode=%s",tmpbuf);
			post_field=flickr_str_append(post_field,tmpbuf);
			flickr_free(tmpbuf);
		}
	}
}

static flickr_data_write_t * flickr_get_postfield(flickr_login_para_t *para)
{
	flickr_data_write_t * post_field=(flickr_data_write_t *)flickr_malloc(sizeof(flickr_data_write_t));
	if(post_field){
		flickr_data_write_init(post_field,flickcurl_data_write_buffer,NULL);
		post_field = flickr_str_append(post_field,"ltmpl=");
		post_field = flickr_str_append(post_field,para->ltmpl.data);
		post_field = flickr_str_append(post_field,"&continue=https%3A%2F%2Faccounts.google.com%2Fo%2Fopenid2%2Fauth%3Fst%3D");
		post_field = flickr_str_append(post_field,para->url_continue.data);
		post_field = flickr_str_append(post_field,"%26from_login%3D1%26hl%3Dzh-TW");
		post_field = flickr_str_append(post_field,"&followup=https%3A%2F%2Faccounts.google.com%2Fo%2Fopenid2%2Fauth%3Fst%3D");
		post_field = flickr_str_append(post_field,para->url_followup.data);
		post_field = flickr_str_append(post_field,"%26from_login%3D1%26hl%3Dzh-TW");
		post_field = flickr_str_append(post_field,"&service=");
		post_field = flickr_str_append(post_field,para->service.data);
		post_field = flickr_str_append(post_field,"&dsh=");
		post_field = flickr_str_append(post_field,para->dsh.data);
		post_field = flickr_str_append(post_field,"&ltmpl=");
		post_field = flickr_str_append(post_field,para->ltmpl.data);
		post_field = flickr_str_append(post_field,"&shdf=");
		post_field = flickr_str_append(post_field,para->shdf.data);
		post_field = flickr_str_append(post_field,"&hl=");
		post_field = flickr_str_append(post_field,para->hl.data);
		post_field = flickr_str_append(post_field,"&ltmpl=");
		post_field = flickr_str_append(post_field,para->ltmpl.data);
		post_field = flickr_str_append(post_field,"&scc=");
		post_field = flickr_str_append(post_field,para->scc.data);
		post_field = flickr_str_append(post_field,"&timeStmp=");
		if(para->timeStmp.data_len!=0)
			post_field = flickr_str_append(post_field,para->timeStmp.data);
		post_field = flickr_str_append(post_field,"&secTok=");
		if(para->secTok.data_len!=0)
			post_field = flickr_str_append(post_field,para->secTok.data);
		post_field = flickr_str_append(post_field,"&GALX=");
		post_field = flickr_str_append(post_field,para->GALX.data);
		post_field = flickr_str_append(post_field,"&Email=");
		post_field = flickr_str_append(post_field,para->Email.data);
		post_field = flickr_str_append(post_field,"&Passwd=");
		post_field = flickr_str_append(post_field,para->Pwd.data);
		post_field = flickr_str_append(post_field,"&rmShown=");
		post_field = flickr_str_append(post_field,para->rmShown.data);
		post_field = flickr_str_append(post_field,"&signIn=");
		__convert_to_URL_encoded(post_field,&para->signIn);
		post_field = flickr_str_append(post_field,"&asts=");
	}
	return post_field;
}

void flickr_free_postfield(flickr_data_write_t *post_filed)
{
	if(post_filed){
		flickr_data_write_free(post_filed);
		flickr_free((char*)post_filed);
	}
}

flickcurl_Google_relocation_t * flickr_get_google_reloation_addr(char *javascript)
{
	char * nextpos=NULL;
	char * currentpos=NULL;
	char *value=NULL;
	currentpos = javascript;
	flickr_info("##############");
	flickcurl_Google_relocation_t * google_relocation=(flickcurl_Google_relocation_t *)flickr_malloc(sizeof(flickcurl_Google_relocation_t));
	if(google_relocation==NULL)
		return google_relocation;
	
	value=flickcurl_extract_google_relation("http://","&amp",currentpos,&nextpos);
	flickr_fill_atomic(value,&google_relocation->url_service);
	if(value)
		flickr_free(value);
	currentpos =nextpos;
	
	value=flickcurl_extract_google_relation("sidt=","&amp",currentpos,&nextpos);
	flickr_fill_atomic(value,&google_relocation->sidt);
	if(value)
		flickr_free(value);
	currentpos =nextpos;
	
	value=flickcurl_extract_google_relation("continue=","&#39",currentpos,&nextpos);
	flickr_fill_atomic(value,&google_relocation->url_continue);
	if(value)
		flickr_free(value);
	
	return google_relocation;
}

void flickr_free_google_reloation_addr(flickcurl_Google_relocation_t *google_relocation)
{
	if(google_relocation){
		flickr_free_atomic(&google_relocation->url_service);
		flickr_free_atomic(&google_relocation->sidt);
		flickr_free_atomic(&google_relocation->url_continue);
		flickr_free((char*)google_relocation);
	}
}

int flick_google_login(CURL*curl_handle,flickcurl_Google_relocation_t *google_relocation)
{
	flickr_data_write_t * google_rel_url=(flickr_data_write_t *)flickr_malloc(sizeof(flickr_login_para_t));
	flickr_data_write_t google_login_data;
	flickr_data_write_init(&google_login_data,flickcurl_data_write_buffer,NULL);
	if(google_rel_url){
		flickr_data_write_init(google_rel_url,flickcurl_data_write_buffer,NULL);
		google_rel_url=flickr_str_append(google_rel_url,google_relocation->url_service.data);
		google_rel_url=flickr_str_append(google_rel_url,"&");
		google_rel_url=flickr_str_append(google_rel_url,google_relocation->sidt.data);
		google_rel_url=flickr_str_append(google_rel_url,"&");
		google_rel_url=flickr_str_append(google_rel_url,google_relocation->url_continue.data);
	}
	flickr_info("GOOGLE LOGINURL=%s",google_rel_url->data_head);
	curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(curl_handle, CURLOPT_URL,google_rel_url->data_head);
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);	
	curl_easy_setopt(curl_handle,CURLOPT_WRITEDATA,&google_login_data);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,flickr_func_write_data);
	
	curl_easy_perform(curl_handle);
	_test_save_to_file(google_login_data.data_head,google_login_data.data_used,"google_login.html");
	
	if(google_rel_url){
		flickr_data_write_free(google_rel_url);
		flickr_free((char*)google_rel_url);
	}
	{
		flickcurl_yahoo_openid_t * yahoo_openid=NULL;
		yahoo_openid = flickr_extract_openid(google_login_data.data_head);
		if(yahoo_openid)
			flickr_free_openid(yahoo_openid);
	}
	flickr_data_write_free(&google_login_data);
	return 0;
}






void flickr_free_login_para(flickr_login_para_t *paras)
{
	__free_login_para_element(paras);
}

char *flickcurl_get_login_paras(const char*name,const char*id,char* strsrc,char** startpot,const char* start_mark,const char*end_mark)
{
	char *value=NULL;
	int value_len=0;
	int offset=0;
	char *name_ptr=NULL,*value_ptr_start=NULL,*value_ptr_end=NULL;
	char tmpbuf[FLICKR_TMP_BUF_LEN];
	memset(tmpbuf,0,FLICKR_TMP_BUF_LEN);
	if(strlen(id)!=0)
		sprintf(tmpbuf,"name=%s id=%s",name,id); 
	else
		sprintf(tmpbuf,"name=%s",name); 
	name_ptr = strstr(strsrc,tmpbuf);
	*startpot = NULL;
	if(name_ptr){
		value_ptr_start = strstr(name_ptr,start_mark);
		offset = strlen(start_mark);
		if(value_ptr_start){
			value_ptr_end = strstr(value_ptr_start+offset,end_mark);
			if(value_ptr_end){
				value_len = value_ptr_end-value_ptr_start-offset;
				*startpot = value_ptr_end;
				if(value_len!=0){
					value =(char*)flickr_malloc(value_len+1);
					if(value){
						memset(value,0,value_len+1);
						memcpy(value,value_ptr_start+offset,value_len);
						//flickr_info("name=%s,value=%s",name,value);
					}
				}
			}
		}
	}
	//flickr_info("<%s><startpot=0x%x>",tmpbuf,*startpot);
	return value;
}


int flickr_extract_login_para(char *dataptr,flickr_login_para_t *paras,flickr_gdata_t *gdata)
{
	char * nextpara=NULL;
	char *currentpara=NULL;
	int rtn=FLICKR_RTN_OK;
	int i=0;
	char * value=NULL;
	currentpara = dataptr;

	if(gdata==NULL || gdata->user_email.data==NULL || gdata->user_pwd.data==NULL)
		return FLICKR_RTN_GOOGLE_LOGIN_ERR;

	memset(paras,0,sizeof(flickr_login_para_t));
	flickr_fill_atomic(gdata->user_email.data,&paras->Email);
	flickr_fill_atomic(gdata->user_pwd.data,&paras->Pwd);
	for(i=0;i<LOGIN_PARA_MAX;i++){
		if(i==LOGIN_PARA_TIMESTAMP || i==LOGIN_PARA_SECTOK)
			value = flickcurl_get_login_paras(google_login_para_strs[i*2],google_login_para_strs[i*2+1],currentpara,&nextpara,"value=","/");
		else if(i==LOGIN_PARA_CONTINUE || i == LOGIN_PARA_FOLLOWUP)
			value = flickcurl_get_login_paras(google_login_para_strs[i*2],google_login_para_strs[i*2+1],currentpara,&nextpara,"st=","&");
		else
			value = flickcurl_get_login_paras(google_login_para_strs[i*2],google_login_para_strs[i*2+1],currentpara,&nextpara,"value=\"","\"");
		if(value){	///< if the string is found, store it to the paras
			__fill_login_para_element(value,i,paras);
			flickr_free(value);
			currentpara = nextpara;
		}
		if(nextpara==NULL) ///<it is means that the string can't be found
			break;
	}
	
	if(i!=LOGIN_PARA_MAX){
		flickr_err("Get GOOGLE Login Para Error!");
		flickr_free_login_para(paras);
		rtn = FLICKR_RTN_GOOGLE_LOGIN_ERR;
	}		
	return rtn;
}

int flickr_connect_to_openid(CURL* curl_handle,flickr_gdata_t *gdata,flickcurl_yahoo_openid_t * yahoo_openid)
{
	flickr_data_write_t * openid_postfield=NULL;
	flickr_data_write_t receive_data;
	char * yahoo_redir=NULL;
	int rtn = FLICKR_RTN_OK;
	flickr_data_write_init(&receive_data,flickcurl_data_write_buffer,NULL);

	openid_postfield = flickr_get_openid_postfield(yahoo_openid);
	if(openid_postfield==NULL){
		rtn = FLICKR_RTN_YAHOO_OPENID_PF_ERR;
		goto CONNECT_OPENID_END;
	}
	curl_easy_setopt(curl_handle,CURLOPT_POST,1L);
	curl_easy_setopt(curl_handle,CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl_handle,CURLOPT_URL,yahoo_openid->openid_url.data);
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS,openid_postfield->data_head);
	flickr_info("<Post_field Size=%d>data=%s",openid_postfield->data_used,openid_postfield->data_head);
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE,openid_postfield->data_used);


	receive_data.cancel_write = gdata->is_auth_cancel; ///< check whether the auth should be cancelled
	curl_easy_setopt(curl_handle,CURLOPT_WRITEDATA,&receive_data);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,flickr_func_write_data);	
	curl_easy_perform(curl_handle);
	
	_test_save_to_file(receive_data.data_head,receive_data.data_used,"yahoo_openid.html");
	
	flickr_free_openid_postfield(openid_postfield);
	if(receive_data.data_used!=0)
		yahoo_redir = flickr_get_yahoo_redir(receive_data.data_head);
	
	flickr_data_write_free(&receive_data);
	if(yahoo_redir){
		rtn = flickr_connect_to_yahoo_redir(curl_handle,gdata,yahoo_redir);
		flickr_free(yahoo_redir);
	}
	else
		rtn = FLICKR_RTN_YAHOO_REDIR_ERR;
	
CONNECT_OPENID_END:
	return rtn;
	
}

int flickr_google_auth(CURL* curl_handle,flickr_gdata_t *gdata,flickr_login_para_t *para)
{
	flickr_data_write_t * post_field=NULL;
	flickr_data_write_t receive_data;
	int rtn=FLICKR_RTN_OK;
	flickcurl_yahoo_openid_t * yahoo_openid=NULL;
	
	flickr_data_write_init(&receive_data,flickcurl_data_write_buffer,NULL);

	post_field = flickr_get_postfield(para);
	if(post_field==NULL){
		rtn = FLICKR_RTN_GOOGLE_PF_ERR;
		goto GOOGLE_AUTH_END;
	}
	
	curl_easy_setopt(curl_handle,CURLOPT_POST,1L);
	curl_easy_setopt(curl_handle,CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl_handle,CURLOPT_URL,google_service_auth);
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS,post_field->data_head);
	flickr_info("<Post_field Size=%d>data=%s",post_field->data_used,post_field->data_head);
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE,post_field->data_used);

	receive_data.cancel_write = gdata->is_auth_cancel; ///< check whether the auth should be cancelled
	curl_easy_setopt(curl_handle,CURLOPT_WRITEDATA,&receive_data);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,flickr_func_write_data);
	curl_easy_perform(curl_handle);
	
	_test_save_to_file(receive_data.data_head,receive_data.data_used,"google_auth.html");

	
	if(receive_data.data_used!=0)
		yahoo_openid = flickr_extract_openid(receive_data.data_head);
	
	flickr_data_write_free(&receive_data);
	flickr_free_postfield(post_field);	
	if(yahoo_openid){
		rtn = flickr_connect_to_openid(curl_handle,gdata,yahoo_openid);
		flickr_free_openid(yahoo_openid);
	}
	else
		rtn = FLICKR_RTN_YAHOO_OPENID_ERR;

GOOGLE_AUTH_END:
	return rtn;
}

/**
@brief get the web of google to enter name and password
**/
int flickr_connect_google(CURL* curl_handle,flickr_gdata_t *gdata,char* link_url)
{
	flickr_data_write_t receive_data;
	flickr_login_para_t para;
	int rtn=FLICKR_RTN_OK;

	flickr_data_write_init(&receive_data,flickcurl_data_write_buffer,NULL);	
	curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(curl_handle, CURLOPT_URL,link_url);

	receive_data.cancel_write = gdata->is_auth_cancel; ///< check whether the auth should be cancelled
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA,&receive_data);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION,flickr_func_write_data);
	curl_easy_perform(curl_handle);
	_test_save_to_file(receive_data.data_head,receive_data.data_used,"google_login.html");

	if(receive_data.data_head!=0){
		rtn = flickr_extract_login_para(receive_data.data_head,&para,gdata);
	}
	flickr_data_write_free(&receive_data);
	if(rtn==FLICKR_RTN_OK){
		rtn = flickr_google_auth(curl_handle,gdata,&para);
		flickr_free_login_para(&para);
	}
	else
		rtn = FLICKR_RTN_GOOGLE_AUTH_ERR;
		
	
CONNECT_GOOGLE_END:
	return rtn;
}
