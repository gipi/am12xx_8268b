
#ifndef FTP_SOCKET_H

#define FTP_SOCKET_H

//DECLS_BEGIN
#define INVALID_SOCKET  (~0)
#define FD_READ_BIT      0
#define FD_READ          (1 << FD_READ_BIT)
#define FD_WRITE_BIT     1
#define FD_WRITE         (1 << FD_WRITE_BIT)
#define FD_OOB_BIT       2
#define FD_OOB           (1 << FD_OOB_BIT)
#define FD_ACCEPT_BIT    3
#define FD_ACCEPT        (1 << FD_ACCEPT_BIT)
#define FD_CONNECT_BIT   4
#define FD_CONNECT 	(1 << FD_CONNECT_BIT)
#define FD_CLOSE_BIT     5
#define FD_CLOSE         (1 << FD_CLOSE_BIT)


#define FTP_REPLY_SIZE 512

typedef enum _Ret
{   
	FTP_RET_OK,    
	FTP_RET_OOM,    
	FTP_RET_STOP,    
	FTP_RET_INVALID_PARAMS,    
	FTP_RET_FAIL
} FTP_Ret;

#define DECLS_BEGIN
#define DECLS_END

#define and &&
#define or  ||
#define return_if_fail(p)    if (!(p))     {printf("%s:%d Warning: "#p" failed.\n",__func__, __LINE__); return;}
#define return_val_if_fail(p, ret)    if (!(p))    {printf("%s:%d Warning: "#p" failed.\n", __func__, __LINE__); return (ret); }
		
#define SAFE_FREE(p)    if (p != NULL) {free(p); p = NULL;}

typedef int SOCKET_HANDLE;


typedef enum _ftp_mode{
	FTP_MODE_PASV,
	FTP_MODE_PORT
}FTP_trans_mode;

typedef struct	_FTP_obj
{    
	SOCKET_HANDLE commandChannel;    
	SOCKET_HANDLE dataChannel;    
	unsigned int secTimeOut;    
	int replyCode;    
	char replyString[FTP_REPLY_SIZE];    
	FTP_trans_mode transMode;
}FTP_Obj;

typedef FTP_Ret (*FtpCommandFunc)(FTP_Obj *ftpObj, const char *command);

int ftp_socket_connect(int socketHandle, const char *ipAddress, const int port);

SOCKET_HANDLE ftp_socket_create(void);

int ftp_socket_select(int socketHandle, int event, int secTime);

FTP_Ret ftp_socket_close(int socketHandle);

FTP_Ret ftp_socket_listen(int socketHandle, int maxListen);

int ftp_socket_accept(int socketHandle);

FTP_Ret ftp_socket_bind_and_listen(int socketHandle, const char *ipAddress, const int port);

//DECLS_END

#endif