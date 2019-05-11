//#include "ftp_socket.h"
#ifndef FTP_H
#define FTP_H

//DECLS_BEGIN

/**
*@addtogroup ftp_client_lib_s
*@{
*/


//#define  FTP_DEBUG
#ifdef  FTP_DEBUG
#define p_printf(format,args...)   printf(format, ##args)
#else
#define p_printf(format,args...)   do {} while (0)
#endif



void ftp_entry(void* arg);

typedef struct	_ftp_msg
{
	long type;
	char ftpmsg[128];	
}ftp_msg;
typedef struct _path_info
{
	unsigned char pathtype;

	char rwmode[64];
	char filesize[16];
	char createdate[64];
	char filename[128];
	
}path_info;
typedef struct _ftp_ls_info
{
	unsigned int itemnum;//the total number of items
	path_info * pinfo;//pointer to the path information
	
}ftp_ls_info;


/**
*@brief connect to the server
*
*@param[in] ipaddr : address of the server
*@param[in] usrname : username to login
*@param[in] psd : password
*@retval 0 : success
*@retval -1 : error
*/
int ftpclient_connect(char* ipaddr, char* usrname,char* psd);


/**
*@brief get the list of current work directory
*
*@param[in] NULL
*@returnl pointer to the ftp_ls_info structure, NULL if failed
*/
ftp_ls_info* ftpclient_get_list(void);


/**
*@brief enter the specified dir
*
*@param[in] dir : the directory to be entered
*@retval 0
*/
int ftpclient_enter_dir(char* dir);


/**
*@brief change the dir to the higher-up dir
*
*@param[in] NULL
*@retval 0
*/
int ftpclient_exit_dir(void);


/**
*@brief download the specified file
*
*@param[in] filename : filename to be downloaded
*@retval 0
*/
int ftpclient_get_file(char* filename);


/**
*@brief disconnect from the server
*
*@param[in] NULL
*@retval 0
*/
int ftpclient_close(void);

/**
 *@}
 */
//DECLS_END

#endif
