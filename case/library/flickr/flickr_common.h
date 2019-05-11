#ifndef __FLICKR_COMMON_H_
#define __FLICKR_COMMON_H_
#include "flickr_main.h"


#ifdef __cplusplus
extern "C" {
#endif




typedef enum
{
	LOGIN_PARA_CONTINUE,
	LOGIN_PARA_FOLLOWUP,
	LOGIN_PARA_SERVICE,
	LOGIN_PARA_DSH,
	LOGIN_PARA_LTMPL,
	LOGIN_PARA_SHDF,
	LOGIN_PARA_HL,
	LOGIN_PARA_SCC,
	LOGIN_PARA_TIMESTAMP,
	LOGIN_PARA_SECTOK,
	LOGIN_PARA_GALX,
	LOGIN_PARA_RMSHOWN,
	LOGIN_PARA_SIGNIN,
	LOGIN_PARA_ASTS,
	LOGIN_PARA_MAX,
}login_para_idx_e;

typedef enum
{
	OPENID_URL,
	OPENID_NS,
	OPENID_MODE,
	OPENID_OP_ENDPOINT,
	OPENID_RESPONSE_NONCE,
	OPENID_RETURN_TO,
	OPENID_ASSOC_HANDLE,
	OPENID_SIGNED,
	OPENID_SIG,
	OPENID_IDENTITY,
	OPENID_CLAIMED_ID,
	OPENID_NS_EXT1,
	OPENID_EXT1_MODE,
	OPENID_EXT1_TYPE_FN,
	OPENID_EXT1_VALUE_FN,
	OPENID_EXT1_TYPE_GID,
	OPENID_EXT1_VALUE_GID,
	OPENID_EXT1_TYPE_EM,
	OPENID_EXT1_VALUE_EM,
	OPENID_EXT1_TYPE_LG,
	OPENID_EXT1_VALUE_LG,
	OPENID_EXT1_TYPE_LN,
	OPENID_EXT1_VALUE_LN,
	OPENID_NS_EXT2,
	OPENID_EXT2_AUTH_TIME,
	OPENID_EXT2_AUTH_POLICIES,
	OPENID_NS_EXT3,
	OPENID_EXT3_MODE,
	OPENID_MAX,
}openid_paras_e;

typedef enum
{
	YAHOO_TRIES,
	YAHOO_SRC,
	YAHOO_MD5,
	YAHOO_HASH,
	YAHOO_JS,
	YAHOO_LAST,
	YAHOO_PROMO,
	YAHOO_INTL,
	YAHOO_BYPASS,
	YAHOO_PARTNER,
	YAHOO_U,
	YAHOO_V,
	YAHOO_CHALLENGE,
	YAHOO_YPLUS,
	YAHOO_EMAILCODE,
	YAHOO_PKG,
	YAHOO_STEPID,
	YAHOO_EV,
	YAHOO_HASMSGR,
	YAHOO_CHKP,
	YAHOO_DONE,
	YAHOO_PD,
	YAHOO_PAD,
	YAHOO_AAD,
	YAHOO_POPUP,
	YAHOO_MAX,
}yahoo_login_para_e;

typedef enum
{
	AUTHREQ_MAGICCOOKIE,
	AUTHREQ_PREMS,
	AUTHREQ_API_KEY,
	AUTHREQ_API_SIG,
	AUTHREQ_FROB,
	AUTHREQ_SOURCE,
	AUTHREQ_DONEAUTH,
	AUTHREQ_MAX,
}auth_reqeust_e;


typedef enum
{
	FACEBOOK_LOGINATTEMPT,
	FACEBOOK_POPUP,
	FACEBOOK_FBCONNECT,
	FACEBOOK_DISPLAY,
	FACEBOOk_NEXT,
	FACEBOOK_REQPERMS,
	FACEBOOK_LEGACYRETURN,
	
	FACEBOOK_LSD,
	FACEBOOK_APIKEY,
	FACEBOOK_RTURNSESSION,
	FACEBOOK_CANCEL_URL,
	FACEBOOK_SESSIONKEYONLY,
	FACEBOOK_SESSIONVERSION,
	FACEBOOK_TRYNUM,
	FACEBOOK_DEFAULT_PERSISTENT,
	FACEBOOK_MAX,
}facebook_login_paras_e;

typedef enum
{
	FACEBOOK_AUTH_APPID,
	FACEBOOK_AUTH_NEXT,
	FACEBOOK_AUTH_DISPLAY,
	FACEBOOK_AUTH_CANCELURL,
	FACEBOOK_AUTH_LOCALE,
	FACEBOOK_AUTH_PERMS,
	FACEBOOK_AUTH_RETURNSESSION,
	FACEBOOK_AUTH_SESSIONVERSION,
	FACEBOOK_AUTH_FBCONNECT,
	FACEBOOK_AUTH_CANVAS,
	FACEBOOK_AUTH_LEGACYRETURN,
	FACEBOOK_AUTH_METHOD,
	FACEBOOK_AUTH_MAX,
}facebook_auth_paras_e;

typedef enum
{
	FACEBOOK_ALLOW_POSTFORMID,
	FACEBOOK_ALLOW_FBDTSG,
	FACEBOOK_ALLOW_APPID,
	FACEBOOK_ALLOW_DISPLAY,
	FACEBOOK_ALLOW_REDIRECTURI,
	FACEBOOK_ALLOW_CANCELURL,
	FACEBOOK_ALLOW_LOCALE,
	FACEBOOK_ALLOW_PERMS,
	FACEBOOK_ALLOW_RETURNSESSION,
	FACEBOOK_ALLOW_SESSIONVERSION,
	FACEBOOK_ALLOW_FBCONNECT,
	FACEBOOK_ALLOW_CANVAS,
	FACEBOOK_ALLOW_LEGACYRETURN,
	FACEBOOK_ALLOW_FROMPOST,
	FACEBOOK_ALLOW_UNISERVMETHOD,
	FACEBOOK_ALLOW_EMAILTYPE,
	FACEBOOK_ALLOW_MAX,
}facebook_auth_allow_e;


typedef struct flickr_login_para_s
{
	flickr_data_atomic_t url_continue;
	flickr_data_atomic_t url_followup;
	flickr_data_atomic_t service;
	flickr_data_atomic_t dsh;
	flickr_data_atomic_t ltmpl;
	flickr_data_atomic_t shdf;
	flickr_data_atomic_t hl;
	flickr_data_atomic_t scc;
	flickr_data_atomic_t timeStmp;
	flickr_data_atomic_t secTok;
	flickr_data_atomic_t GALX;
	flickr_data_atomic_t Email;
	flickr_data_atomic_t Pwd;
	flickr_data_atomic_t rmShown;
	flickr_data_atomic_t signIn;
	flickr_data_atomic_t asts;
}flickr_login_para_t;

typedef struct flickcurl_google_relocation_s
{
	flickr_data_atomic_t url_service;
	flickr_data_atomic_t sidt;
	flickr_data_atomic_t url_continue;
}flickcurl_Google_relocation_t;

typedef struct flickcurl_yahoo_openid_s
{
	flickr_data_atomic_t openid_url;
	flickr_data_atomic_t ns;
	flickr_data_atomic_t mode;
	flickr_data_atomic_t op_endpoint;
	flickr_data_atomic_t response_nonce;
	flickr_data_atomic_t return_to;
	flickr_data_atomic_t assoc_handle;
	flickr_data_atomic_t openid_signed;
	flickr_data_atomic_t sig;
	flickr_data_atomic_t identity;
	flickr_data_atomic_t claimed_id;
	flickr_data_atomic_t ns_ext1;
	flickr_data_atomic_t ext1_mode;
	flickr_data_atomic_t ext1_type_fn;
	flickr_data_atomic_t ext1_value_fn;
	flickr_data_atomic_t ext1_type_gid;
	flickr_data_atomic_t ext1_value_gid;
	flickr_data_atomic_t ext1_type_em;
	flickr_data_atomic_t ext1_value_em;
	flickr_data_atomic_t ext1_type_lg;
	flickr_data_atomic_t ext1_value_lg;
	flickr_data_atomic_t ext1_type_ln;
	flickr_data_atomic_t ext1_value_ln;
	flickr_data_atomic_t ns_ext2;
	flickr_data_atomic_t ext2_auth_time;
	flickr_data_atomic_t ext2_auth_policies;
	flickr_data_atomic_t ns_ext3;
	flickr_data_atomic_t ext3_mode;
		
}flickcurl_yahoo_openid_t;


typedef struct flickr_auth_request_s
{
	flickr_data_atomic_t magic_cookie;
	flickr_data_atomic_t api_key;
	flickr_data_atomic_t api_sig;
	flickr_data_atomic_t perms;
	flickr_data_atomic_t frob;
	flickr_data_atomic_t done_auth;
	flickr_data_atomic_t source;
}flickr_auth_request_t;

typedef struct facebook_login_para_s
{
	///< for post url
	flickr_data_atomic_t login_attempt;
	flickr_data_atomic_t popup;
	flickr_data_atomic_t fbconnect;
	flickr_data_atomic_t display;
	flickr_data_atomic_t next;
	flickr_data_atomic_t req_perms;
	flickr_data_atomic_t legacy_return;


	///< for post field
	flickr_data_atomic_t charset_test;
	flickr_data_atomic_t lsd;
	//flickr_data_atomic_t next;
	flickr_data_atomic_t api_key;
	flickr_data_atomic_t return_session;
	flickr_data_atomic_t cancel_url;
	//flickr_data_atomic_t req_perms;
	//flickr_data_atomic_t legacy_return;
	//flickr_data_atomic_t display;
	flickr_data_atomic_t session_key_only;
	flickr_data_atomic_t session_version;
	flickr_data_atomic_t try_num;
	//flickr_data_atomic_t charset_test;
	//flickr_data_atomic_t lsd;
	flickr_data_atomic_t email;
	flickr_data_atomic_t pass;
	flickr_data_atomic_t default_persistent;
	flickr_data_atomic_t login;
}facebook_login_paras_t;

typedef struct facebook_auth_paras_s
{
	flickr_data_atomic_t app_id;
	flickr_data_atomic_t next;
	flickr_data_atomic_t display;
	flickr_data_atomic_t cancel_url;
	flickr_data_atomic_t locale;
	flickr_data_atomic_t perms;
	flickr_data_atomic_t return_session;
	flickr_data_atomic_t session_version;
	flickr_data_atomic_t fbconnect;
	flickr_data_atomic_t canvas;
	flickr_data_atomic_t legacy_return;
	flickr_data_atomic_t method;
}facebook_auth_paras_t;


typedef struct facebook_auth_allows_s
{
	flickr_data_atomic_t post_form_id;
	flickr_data_atomic_t fb_dtsg;
	flickr_data_atomic_t app_id;
	flickr_data_atomic_t display;
	flickr_data_atomic_t redirect_uri;
	flickr_data_atomic_t cancel_url;
	flickr_data_atomic_t locale;
	flickr_data_atomic_t perms;
	flickr_data_atomic_t return_session;
	flickr_data_atomic_t session_version;
	flickr_data_atomic_t fbconnect;
	flickr_data_atomic_t canvas;
	flickr_data_atomic_t legacy_return;
	flickr_data_atomic_t from_post;
	flickr_data_atomic_t __uiserv_method;
	flickr_data_atomic_t grantEmailType;
}facebook_auth_allows_t;

typedef struct flickr_yahoo_login_para_s
{
	flickr_data_atomic_t tries;
	flickr_data_atomic_t src;
	flickr_data_atomic_t md5;
	flickr_data_atomic_t hash;
	flickr_data_atomic_t js;
	flickr_data_atomic_t last;
	flickr_data_atomic_t promo;
	flickr_data_atomic_t intl;
	flickr_data_atomic_t bypass;
	flickr_data_atomic_t partner;
	flickr_data_atomic_t u;
	flickr_data_atomic_t v;
	flickr_data_atomic_t challenge;
	flickr_data_atomic_t yplus;
	flickr_data_atomic_t emailCode;
	flickr_data_atomic_t pkg;
	flickr_data_atomic_t stepid;
	flickr_data_atomic_t ev;
	flickr_data_atomic_t hasMsgr;
	flickr_data_atomic_t chkP;
	flickr_data_atomic_t done;
	flickr_data_atomic_t pd;
	flickr_data_atomic_t pad;
	flickr_data_atomic_t aad;
	flickr_data_atomic_t popup;
	flickr_data_atomic_t login;
	flickr_data_atomic_t passwd;
	flickr_data_atomic_t save;
	flickr_data_atomic_t passwd_raw;
}flickr_yahoo_login_para_t;


static char yahoo_service_auth[]="http://www.flickr.com/services/auth/?";
static char flickr_cookie_addr[]="/mnt/vram/cookie.txt";
static char google_service_auth[]="https://www.google.com/accounts/ServiceLoginAuth";


flickr_data_write_t * flickr_str_append(flickr_data_write_t * xml_list,char* str_add);
flickr_data_write_t * flickr_strn_append(flickr_data_write_t * xml_list,char* str_add,int size);
char * flickr_extract_str(const char* str_start,const char* str_end,char* str_src,char**nextpos);
char * flickr_get_URL_encoded(char* src,unsigned char c_convert);
char * flickr_get_URLEncode_str(char * src,char* conv);
size_t flickr_func_write_data( void *ptr, size_t size, size_t nmemb, void *userdata);
size_t flickr_func_write_header( void *ptr, size_t size, size_t nmemb, void *userdata);
int flickr_data_write_init(flickr_data_write_t *data,FlickcurlDataWrite_e data_type,void* fp);
int flickr_data_write_free(flickr_data_write_t *data);

char * flickr_get_yahoo_redir(char*yahoo_redir_src);
 flickr_yahoo_login_para_t* flickr_extract_yahoo_paras(flickr_gdata_t *gdata,const char* src_data);


#ifdef __cplusplus
}
#endif

#endif
