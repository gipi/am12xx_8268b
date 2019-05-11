#ifndef _FACEBOOK_AUTH_H_

#define _FACEBOOK_AUTH_H_

#include "webalbum_api.h"
#include "facebook_common.h"

int get_success_page(facebook_data * f_data);

int check_autherror(char *msg);

int get_accesstoken(facebook_data * f_data);

int send_noallow(facebook_data * f_data, char *post_form_id, char *fb_dtsg, char *m_sess);

int send_allow(facebook_data * f_data, char *post_form_id, char *fb_dtsg, char *m_sess);

int reload_third(facebook_data * f_data);

int send_userinfo(facebook_data * f_data, char * post_from_id);

int reload_second(facebook_data * f_data);

int reload_first(facebook_data * f_data);

int facebook_auth(facebook_data * f_data);

char *  get_postformid(char *data);

char * get_fbdtsg(char *data);

char * get_msess(char *data);

char *  get_charset(char *data);

char *  get_login(char *data);

char * get_grant(char *data);

int facebook_logout();
#endif
