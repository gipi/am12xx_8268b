#include "facebook_auth.h"
#include "facebook_json_analy.h"

extern char * url_reload_p;

CURLcode curlcode=0;
long m_http_code=0;

int get_success_page(facebook_data * f_data)
{
	int rtn=0;

	FILE * fp = NULL;
	facebook_write_data * success_page = (facebook_write_data *)facebook_malloc(sizeof(facebook_write_data));
	if(success_page){
		facebook_dbg("success page malloc failed");
		rtn = FACEBOOK_RTN_ERR_LOGIN;
		goto get_success_page_out;
	}
	fp = fopen("/mnt/udisk/success_page.txt","wb+");
	if(fp==NULL){
		facebook_dbg("open success_page.txt error!");
		rtn = FACEBOOK_RTN_ERR_LOGIN;
		goto get_success_page_out;
	}
	facebook_data_write_init(success_page,Facebook_data_write_file,fp);

	facebook_dbg("URL is %s, len is %d",url_reload_p, strlen(url_reload_p));
	curl_easy_setopt(f_data->curl, CURLOPT_URL,url_reload_p);
	curl_easy_setopt(f_data->curl, CURLOPT_HTTPGET,1L);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEDATA,success_page);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEFUNCTION,facebook_func_write_data);

	curlcode = curl_easy_perform(f_data->curl);
	curl_easy_getinfo(f_data->curl, CURLINFO_RESPONSE_CODE, &m_http_code);
	facebook_dbg("curlcode is %d",curlcode);
	facebook_dbg("m_http_code is %ld",m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		facebook_dbg("Connect Success");
	}
	else if(curlcode==CURLE_OK && m_http_code==HttpStatus_FOUND){
		facebook_dbg("Need to reload again");
	}
	else{
		facebook_dbg("Connect fail");
		rtn = FACEBOOK_RTN_ERR_LOGIN;
	}
get_success_page_out:
	if(fp){
		fclose(fp);
		fp = NULL;
	}
	if(success_page){
		facebook_data_write_free(success_page);
		facebook_free(success_page);
		success_page = NULL;
	}
	return rtn;
}

int check_autherror(char *msg)
{
	char *locate = NULL;
	locate = strstr(msg,"Incorrect email/password combination.");
	if(locate!=NULL){
		return FACEBOOK_RTN_ERR_LOGINPASSWORD;
	}
	else{
		locate = strstr(msg,"Sorry, your account is temporarily unavailable. Please log in to www.facebook.com from a computer and follow the instructions given.");
		if(locate!=NULL){
			return FACEBOOK_RTN_ERR_ACCOUNTEXCEPTION;
		}
	}
	return FACEBOOK_RTN_ERR_LOGIN;
}

int get_accesstoken(facebook_data * f_data)
{
	char *locate = "access_token";
	facebook_dbg("%s",url_reload_p);
	char *find_token = NULL;
	find_token = strstr(url_reload_p, "access_token");
	if(find_token==NULL){
		facebook_dbg("There is something error for token found!");
		find_token = strstr(url_reload_p, "confirmemail.php");
		if(find_token){
			facebook_dbg("you must goto your email to confirm");
			return FACEBOOK_RTN_ERR_ACCOUNTCONFIRM;
		}
		return FACEBOOK_RTN_ERR_LOGIN;
	}
	facebook_dbg("find_token is %s",find_token);
	char *find_expires = strstr(url_reload_p, "&expires_in");
	if(find_expires==NULL){
		facebook_dbg("There is something error for expires found!");
		return FACEBOOK_RTN_ERR_LOGIN;
	}
	facebook_dbg("find_expires is %s",find_expires);
	int i=0;
	int len_token = strlen(find_token);
	int len_expires = strlen(find_expires);
	#if 0
	for(i=0;i<len_token;i++)
		facebook_dbg("%d:0x%x",i,find_token[i]);
	#endif
	if(f_data->access_token){
		facebook_free(f_data->access_token);
		f_data->access_token = NULL;
	}
	f_data->access_token = (char *)facebook_malloc(len_token-len_expires+1);
	if(f_data->access_token==NULL){
		facebook_dbg("f_data->access_token is null");
		return FACEBOOK_RTN_ERR_LOGIN;
	}
	memcpy(f_data->access_token, find_token, len_token-len_expires);
	f_data->access_token[len_token-len_expires] = 0;
	facebook_dbg("access token is %s, len is %d",f_data->access_token,strlen(f_data->access_token));
	return FACEBOOK_RTN_OK;
}

int send_noallow(facebook_data * f_data, char *post_form_id, char *fb_dtsg, char *m_sess)
{
	char send_noallow[] = "http://m.facebook.com/connect/uiserver.php";
	char post_buf1[] = "&app_id=187161051325141&display=wap&redirect_uri=http%3A%2F%2Fwww.facebook.com%2Fconnect%2Flogin_success.html%2F&response_type=token&fbconnect=1&perms=publish_stream%2Cuser_photos%2Cuser_videos%2Cuser_photo_video_tags%2Cread_friendlists%2Cfriends_photos%2Cfriends_videos&from_login=1&refsrc=http%3A%2F%2Fm.facebook.com%2Flogin.php&refid=9";

	char post_buf2[] = "&_rdr=&from_post=1&__uiserv_method=permissions.request&cancel_clicked=%E4%B8%8D%E5%85%81%E8%AE%B8";
	char post_buf[1024] = {0};
	int rtn=FACEBOOK_RTN_OK;

	sprintf(post_buf,"post_form_id=%s&fb_dtsg=%s%s&m_sess=%s%s",post_form_id, fb_dtsg, post_buf1, m_sess, post_buf2);
	facebook_dbg("post buf is %s",post_buf);

	facebook_dbg("URL is %s, len is %d",send_noallow, strlen(send_noallow));
	curl_easy_setopt(f_data->curl, CURLOPT_URL,send_noallow);
	curl_easy_setopt(f_data->curl, CURLOPT_POST,1L);
	curl_easy_setopt(f_data->curl, CURLOPT_POSTFIELDS,post_buf);
	curl_easy_setopt(f_data->curl, CURLOPT_POSTFIELDSIZE,strlen(post_buf));

	curlcode = curl_easy_perform(f_data->curl);
	curl_easy_getinfo(f_data->curl, CURLINFO_RESPONSE_CODE, &m_http_code);
	facebook_dbg("curlcode is %d",curlcode);
	facebook_dbg("m_http_code is %ld",m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		facebook_dbg("Connect Success");
	}
	else if(curlcode==CURLE_OK && m_http_code==HttpStatus_FOUND){
		facebook_dbg("Need to get access token");
	}
	else{
		facebook_dbg("Connect fail");
		rtn = FACEBOOK_RTN_ERR_LOGIN;
	}
	return rtn;
}

int send_allow(facebook_data * f_data, char *post_form_id, char *fb_dtsg, char *m_sess)
{
	char send_allow[] = "http://m.facebook.com/connect/uiserver.php";
	char post_buf1[] = "&app_id=187161051325141&display=wap&redirect_uri=http%3A%2F%2Fwww.facebook.com%2Fconnect%2Flogin_success.html%2F&response_type=token&fbconnect=1&perms=publish_stream%2Cuser_photos%2Cuser_videos%2Cuser_photo_video_tags%2Cread_friendlists%2Cfriends_photos%2Cfriends_videos&from_login=1&refsrc=http%3A%2F%2Fm.facebook.com%2Flogin.php&refid=9";

	char post_buf2[] = "&_rdr=&from_post=1&__uiserv_method=permissions.request&grant_clicked=%E5%85%81%E8%AE%B8";
	char post_buf[1024] = {0};
	int rtn=FACEBOOK_RTN_OK;

	sprintf(post_buf,"post_form_id=%s&fb_dtsg=%s%s&m_sess=%s%s",post_form_id, fb_dtsg, post_buf1, m_sess, post_buf2);
	facebook_dbg("post buf is %s",post_buf);

	facebook_dbg("URL is %s len is %d",send_allow, strlen(send_allow));
	curl_easy_setopt(f_data->curl, CURLOPT_URL,send_allow);
	curl_easy_setopt(f_data->curl, CURLOPT_POST,1L);
	curl_easy_setopt(f_data->curl, CURLOPT_POSTFIELDS,post_buf);
	curl_easy_setopt(f_data->curl, CURLOPT_POSTFIELDSIZE,strlen(post_buf));

	curlcode = curl_easy_perform (f_data->curl);
	curl_easy_getinfo(f_data->curl, CURLINFO_RESPONSE_CODE, &m_http_code);
	facebook_dbg("curlcode is %d",curlcode);
	facebook_dbg("m_http_code is %ld",m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		facebook_dbg("Connect Success");
	}
	else if(curlcode==CURLE_OK && m_http_code==HttpStatus_FOUND){
		facebook_dbg("Need to get access token");
		rtn = get_accesstoken(f_data);
	}
	else{
		facebook_dbg("Connect fail");
		rtn = FACEBOOK_RTN_ERR_LOGIN;
	}
	return rtn;
}

int reload_third(facebook_data * f_data)
{
	int rtn=FACEBOOK_RTN_OK;
	char *post_form_id=NULL, *fb_dtsg=NULL, *m_sess=NULL;

	facebook_write_data * reload_third = (facebook_write_data *)facebook_malloc(sizeof(facebook_write_data));
	if(reload_third==NULL){
		facebook_dbg("reload third malloc failed");
		rtn = FACEBOOK_RTN_ERR_MEMORY;
		goto reload_third_out;
	}
	facebook_data_write_init(reload_third,Facebook_data_write_buffer,NULL);

	facebook_dbg("URL is %s, len is %d",url_reload_p, strlen(url_reload_p));
	curl_easy_setopt(f_data->curl, CURLOPT_URL,url_reload_p);
	curl_easy_setopt(f_data->curl, CURLOPT_HTTPGET,1L);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEDATA,reload_third);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEFUNCTION,facebook_func_write_data);

	curlcode = curl_easy_perform(f_data->curl);
	curl_easy_getinfo(f_data->curl, CURLINFO_RESPONSE_CODE, &m_http_code);
	facebook_dbg("curlcode is %d",curlcode);
	facebook_dbg("m_http_code is %ld",m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		facebook_dbg("Connect Success");
		post_form_id = get_postformid(reload_third->data_head);
		fb_dtsg = get_fbdtsg(reload_third->data_head);
		m_sess = get_msess(reload_third->data_head);
		if(post_form_id==NULL || fb_dtsg==NULL || m_sess==NULL){
			facebook_dbg("post form id or fb_dtsg or m_sess is NULL!");
			rtn = check_autherror(reload_third->data_head);
			goto reload_third_out;
		}
		rtn = send_allow(f_data, post_form_id, fb_dtsg, m_sess);
	}
	else if(curlcode==CURLE_OK && m_http_code==HttpStatus_FOUND){
		facebook_dbg("Need to get access token");
		rtn = get_accesstoken(f_data);
	}
	else{
		facebook_dbg("Connect fail");
		rtn = FACEBOOK_RTN_ERR_LOGIN;
	}
reload_third_out:
	if(post_form_id){
		facebook_free(post_form_id);
		post_form_id = NULL;
	}
	if(fb_dtsg){
		facebook_free(fb_dtsg);
		fb_dtsg = NULL;
	}
	if(m_sess){
		facebook_free(m_sess);
		m_sess = NULL;
	}
	if(reload_third){
		facebook_data_write_free(reload_third);
		facebook_free(reload_third);
		reload_third = NULL;
	}
	return rtn;
}

int send_userinfo(facebook_data * f_data, char * post_from_id)
{
	char send[] = "https://www.facebook.com/login.php?m=m&next=http%3A%2F%2Fm.facebook.com%2Fconnect%2Fuiserver.php%3Fmethod%3Dpermissions.request%26app_id%3D187161051325141%26display%3Dwap%26redirect_uri%3Dhttp%253A%252F%252Fwww.facebook.com%252Fconnect%252Flogin_success.html%252F%26response_type%3Dtoken%26fbconnect%3D1%26perms%3Dpublish_stream%252Cuser_photos%252Cuser_videos%252Cuser_photo_video_tags%252Cread_friendlists%252Cfriends_photos%252Cfriends_videos%26from_login%3D1&refsrc=http%3A%2F%2Fm.facebook.com%2Flogin.php&refid=9";

	char post_buf1[] = "&charset_test=%E2%82%AC%2C%C2%B4%2C%E2%82%AC%2C%C2%B4%2C%E6%B0%B4%2C%D0%94%2C%D0%84&";
	char post_buf2[] = "&login=%E7%99%BB%E5%BD%95";
	char post_buf[512] = {0};
	sprintf(post_buf,"lsd=&post_form_id=%s%semail=%s&pass=%s%s",post_from_id, post_buf1,f_data->usermail,f_data->userpwd,post_buf2);
	facebook_dbg("post buf is %s, len is %d",post_buf,strlen(post_buf));

	int rtn=FACEBOOK_RTN_OK;

	facebook_dbg("URL is %s, len is %d",send, strlen(send));
	curl_easy_setopt(f_data->curl,CURLOPT_URL,send);
	curl_easy_setopt(f_data->curl, CURLOPT_POST,1L);
	curl_easy_setopt(f_data->curl, CURLOPT_POSTFIELDS,post_buf);
	curl_easy_setopt(f_data->curl, CURLOPT_POSTFIELDSIZE,strlen(post_buf));

	curlcode = curl_easy_perform(f_data->curl);
	curl_easy_getinfo(f_data->curl, CURLINFO_RESPONSE_CODE, &m_http_code);
	facebook_dbg("curlcode is %d",curlcode);
	facebook_dbg("m_http_code is %ld",m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		facebook_dbg("Connect Success");
	}
	else if(curlcode==CURLE_OK && m_http_code==HttpStatus_FOUND){
		facebook_dbg("need to reload third");
		rtn = reload_third(f_data);
	}
	else{
		facebook_dbg("Connect fail");
		rtn = FACEBOOK_RTN_ERR_LOGIN;
	}
	return rtn;
}

int reload_second(facebook_data * f_data)
{
	int rtn=FACEBOOK_RTN_OK;
	char * post_form_id=NULL;

	facebook_write_data * reload_second = (facebook_write_data *)facebook_malloc(sizeof(facebook_write_data));
	if(reload_second==NULL){
		facebook_dbg("reload second malloc failed");
		rtn = FACEBOOK_RTN_ERR_MEMORY;
		goto reload_second_out;
	}
	facebook_data_write_init(reload_second,Facebook_data_write_buffer,NULL);
	
	facebook_dbg("URL is %s, len is %d",url_reload_p, strlen(url_reload_p));
	curl_easy_setopt(f_data->curl, CURLOPT_URL,url_reload_p);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEDATA,reload_second);
	curl_easy_setopt(f_data->curl,CURLOPT_WRITEFUNCTION,facebook_func_write_data);

	curlcode = curl_easy_perform (f_data->curl);
	curl_easy_getinfo(f_data->curl, CURLINFO_RESPONSE_CODE, &m_http_code);
	facebook_dbg("curlcode is %d",curlcode);
	facebook_dbg("m_http_code is %ld",m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		facebook_dbg("Connect Success");
	}
	else{
		facebook_dbg("Connect fail");
		rtn = FACEBOOK_RTN_ERR_LOGIN;
	}
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		post_form_id = get_postformid(reload_second->data_head);
		if(post_form_id==NULL){
			facebook_dbg("post form id is NULL!");
			rtn = FACEBOOK_RTN_ERR_LOGIN;
			goto reload_second_out;
		}
		rtn = send_userinfo(f_data, post_form_id);
	}
reload_second_out:
	if(post_form_id){
		facebook_free(post_form_id);
		post_form_id = NULL;
	}
	if(reload_second){
		facebook_data_write_free(reload_second);
		facebook_free(reload_second);
		reload_second = NULL;
	}
	return rtn;
}

int reload_first(facebook_data * f_data)
{
	char *reload_first = NULL;
	int rtn=FACEBOOK_RTN_OK;

	facebook_dbg("URL is %s, len is %d",url_reload_p, strlen(url_reload_p));
	curl_easy_setopt(f_data->curl,CURLOPT_URL,url_reload_p);

	curlcode = curl_easy_perform (f_data->curl);
	curl_easy_getinfo(f_data->curl, CURLINFO_RESPONSE_CODE, &m_http_code);
	facebook_dbg("curlcode is %d",curlcode);
	facebook_dbg("m_http_code is %ld",m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		facebook_dbg("Connect Success");
	}
	else if(curlcode==CURLE_OK && m_http_code==HttpStatus_FOUND){
		facebook_dbg("Need to reload second");
		rtn = reload_second(f_data);
	}
	else{
		facebook_dbg("Connect fail");
		rtn = FACEBOOK_RTN_ERR_LOGIN;
	}
	return rtn;
}

int facebook_auth(facebook_data * f_data)
{
	char auth_url[] = "https://www.facebook.com/dialog/oauth?client_id=187161051325141&redirect_uri=http%3A%2F%2Fwww.facebook.com%2Fconnect%2Flogin_success.html%2F&scope=publish_stream,user_photos,user_videos,user_photo_video_tags,read_friendlists,friends_photos,friends_videos&response_type=token&display=wap";
	int rtn = FACEBOOK_RTN_OK;
	int i;
	int status = 0;

	facebook_dbg("URL is %s, len is %d",auth_url, strlen(auth_url));
	
	curl_easy_setopt(f_data->curl,CURLOPT_URL,auth_url);

	curlcode = curl_easy_perform(f_data->curl);
	curl_easy_getinfo(f_data->curl, CURLINFO_RESPONSE_CODE, &m_http_code);
	facebook_dbg("curlcode is %d",curlcode);
	facebook_dbg("m_http_code is %ld",m_http_code);
	if(curlcode==CURLE_OK && m_http_code==HttpStatus_OK){
		facebook_dbg("Connect Success");
	}
	else if(curlcode==CURLE_OK && m_http_code==HttpStatus_FOUND){
		facebook_dbg("Need to reload first");
		rtn = reload_first(f_data);
	}
	else{
		facebook_dbg("Connect Fail");
		rtn = FACEBOOK_RTN_ERR_LOGIN;
	}
	status = get_status(rtn, curlcode, m_http_code);
	facebook_dbg("authentication status is 0x%x",status);
	f_data->auth_status = status;
	return status;
}

char *  get_postformid(char *data)
{
	facebook_dbg("%s",data);
	char * find = strstr(data, "post_form_id\" value=\"");
	if(find){
		find += 21;
		//facebook_dbg("%s",find);
		char * post_form_id = (char *)facebook_malloc(POST_FORM_ID_LEN+1);
		if(post_form_id==NULL){
			facebook_dbg("post form id malloc failed");
			return NULL;
		}
		memcpy(post_form_id, find, POST_FORM_ID_LEN);
		post_form_id[POST_FORM_ID_LEN] = 0;
		facebook_dbg("post form id is %s",post_form_id);
		return post_form_id;
	}
	else
		return NULL;
}

char * get_fbdtsg(char *data)
{
	//facebook_dbg("%s",data);
	char * find = strstr(data, "fb_dtsg\" value=\"");
	if(find){
		find += 16;
		//facebook_dbg("%s",find);
		char * fb_dtsg = (char *)facebook_malloc(FB_DTSG_LEN+1);
		if(fb_dtsg==NULL){
			facebook_dbg("fb dtsg malloc failed");
			return NULL;
		}
		memcpy(fb_dtsg, find, FB_DTSG_LEN);
		fb_dtsg[FB_DTSG_LEN] = 0;
		facebook_dbg("fb_dtsg is %s",fb_dtsg);
		return fb_dtsg;
	}
	else
		return NULL;
}

char * get_msess(char *data)
{
	//facebook_dbg("%s",data);
	char * find = strstr(data, "m_sess\" value=\"");
	if(find){
		find += 15;
		//facebook_dbg("%s",find);
		char * m_sess = (char *)facebook_malloc(M_SESS_LEN+1);
		if(m_sess==NULL){
			facebook_dbg("m sess malloc failed");
			return NULL;
		}
		memcpy(m_sess, find, M_SESS_LEN);
		m_sess[M_SESS_LEN] = 0;
		facebook_dbg("m_sess is %s",m_sess);
		return m_sess;
	}
	else
		return NULL;
}

char *  get_charset(char *data)
{
	//facebook_dbg("%s",data);
	char * find = strstr(data, "charset_test");
	if(find){
		find += 21;
		//facebook_dbg("%s",find);
		char * charset_test = (char *)facebook_malloc(24);
		if(charset_test==NULL){
			facebook_dbg("charset test malloc failed");
			return NULL;
		}
		memcpy(charset_test, find, 23);
		charset_test[23] = 0;
		facebook_dbg("charset_test is %s",charset_test);
		int i=0;
		for(i=0;i<=23;i++)
			facebook_dbg("0x%x",charset_test[i]);
		return charset_test;
	}
	else
		return NULL;
}

char *  get_login(char *data)
{
	//facebook_dbg("%s",data);
	char * find = strstr(data, "input type=\"submit\" value=\"");
	if(find){
		find += 27;
		//facebook_dbg("%s",find);
		char * log_in = (char *)facebook_malloc(7);
		if(log_in==NULL){
			facebook_dbg("log in malloc failed");
			return NULL;
		}
		memcpy(log_in, find, 6);
		log_in[6] = 0;
		facebook_dbg("log_in is %s",log_in);
		int i=0;
		for(i=0;i<=6;i++)
			facebook_dbg("0x%x",log_in[i]);
		return log_in;
	}
	else
		return NULL;
}

char * get_grant(char *data)
{
	//facebook_dbg("%s",data);
	char * find = strstr(data, "id=\"grant_clicked\" for=\"");
	if(find){
		find += 49;
		//facebook_dbg("%s",find);
		char * grant = (char *)facebook_malloc(7);
		if(grant==NULL){
			facebook_dbg("grant malloc failed");
			return NULL;
		}
		memcpy(grant, find, 6);
		grant[6] = 0;
		facebook_dbg("log_in is %s",grant);
		int i=0;
		for(i=0;i<=6;i++)
			facebook_dbg("0x%x",grant[i]);
		return grant;
	}
	else
		return NULL;
}

int facebook_logout()
{
	char logout[] = "http://www.facebook.com/logout.php";
	return 0;
}
