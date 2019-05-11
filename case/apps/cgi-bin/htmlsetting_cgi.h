#ifndef	_HTMLSETTING_CGI_H_
#define	_HTMLSETTING_CGI_H_

#define HTMLSETTING_CGI_ENABLE
#define UNIX_SOCKET_PATCH	"/tmp/htmlsetting_patch"
#define	CMDLEN_MARK			"/c"
#define VALLEN_MARK			"/v"
#define SENDLEN_MARK		"/r"
#define END_MARK			"/e"

//#define CGI_DEBUG_ENABLE
#ifdef CGI_DEBUG_ENABLE
#define cgidbg_info(fmt,arg...) printf("--cgi--MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#else
#define cgidbg_info(fmt, arg...)
#endif

struct msglen_s{
	char flag[2];
	int len;
};

// If you do not need "return_val" or "return_len", then let it equal NULL, and if "return_val" not equal NULL, you should do "del_return(return_val);";
// If have not "val", then let "val = NULL" and "len = 0".
int htmlsetting_func(char *cmd, void *val, int len, void **return_val, int *return_len);
int del_return(void **return_val);

#endif
