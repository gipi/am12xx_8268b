#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ftp_socket.h"
#include "ftp.h"
#include <semaphore.h>

#define FTP_PROMPT        ("ftp> ")
#define COMMAND_SIZE    256
#define MAX_PROMPT_SIZE 128
#define MAX_RECV_SIZE    (1024 << 1)
#define false	0
#define true	1

extern char* rbuf;

static int ftp_set_nonblock(SOCKET_HANDLE socketHandle)
{
	int flag = fcntl(socketHandle, F_GETFL);
	flag |= O_NONBLOCK;
	if (fcntl(socketHandle, F_SETFL, flag) == -1)
	{
		return false;
	}
	return true;
}

static int ftp_send_buffer(SOCKET_HANDLE socketHandle, const char *buffer, size_t size)
{    
	return write(socketHandle, buffer, size);
}

static FTP_Ret ftp_send_command(SOCKET_HANDLE socketHandle, const char *command)
{
	char sendCommand[COMMAND_SIZE] = {0x00};    
	return_val_if_fail(socketHandle != INVALID_SOCKET, FTP_RET_INVALID_PARAMS);    
	return_val_if_fail(command != NULL, FTP_RET_INVALID_PARAMS);    
	
	sprintf(sendCommand, "%s\r\n", command);    
	Writen(socketHandle, sendCommand, strlen(sendCommand));    
	return FTP_RET_OK;
}

static long ftp_recv_command(SOCKET_HANDLE socketHandle, char *recvBuf, unsigned long recvMaxSize)
{    
	long recvSize = 0;    
	long totalSize = 0;    
	return_val_if_fail(socketHandle != INVALID_SOCKET, -1);    
	return_val_if_fail(recvBuf != NULL, -1);   
	
	do    
	{        
		recvSize = recv(socketHandle, recvBuf + totalSize, recvMaxSize - totalSize, 0);        
		totalSize += recvSize;    
	} while (recvSize > 0);    
	
	return totalSize;
}

static FTP_Ret ftp_init_obj(FTP_Obj *ftpObj)
{    
	return_val_if_fail(ftpObj != NULL, FTP_RET_INVALID_PARAMS);    
	ftpObj->commandChannel = INVALID_SOCKET;    
	ftpObj->dataChannel       = INVALID_SOCKET;    
	ftpObj->secTimeOut       = 60;    
	ftpObj->replyCode       = -1;    
	ftpObj->transMode      = FTP_MODE_PASV;    
	memset(ftpObj->replyString, 0x00, sizeof(ftpObj->replyString));    
	return FTP_RET_OK;
}

static FTP_Ret ftp_get_reply(FTP_Obj *ftpObj)
{    
	char recvBuf[MAX_RECV_SIZE] = {0x00};    
	return_val_if_fail(ftpObj != NULL, FTP_RET_INVALID_PARAMS);    
	return_val_if_fail(ftpObj->commandChannel != INVALID_SOCKET, FTP_RET_FAIL);    
	
	ftpObj->replyCode = -1;    
	memset(ftpObj->replyString, 0x00, sizeof(ftpObj->replyString));    
	while (ftp_socket_select(ftpObj->commandChannel, FD_READ, ftpObj->secTimeOut) > 0)    
	{
		ftp_set_nonblock(ftpObj->commandChannel);
		if (ftp_recv_command(ftpObj->commandChannel, recvBuf, MAX_RECV_SIZE) <= 0)        
		{
			fprintf(stderr, "Recv reply message failed.\n");
			return FTP_RET_FAIL;
		}
		else{
			printf("%s", recvBuf);            
			strncpy(ftpObj->replyString, recvBuf, strlen(recvBuf));            
			return FTP_RET_OK;        
		}    
	}    
	return FTP_RET_FAIL;
}

static FTP_Ret ftp_send_and_recv_reply(FTP_Obj *ftpObj, const char *command)
{    
	return_val_if_fail(ftpObj != NULL, FTP_RET_INVALID_PARAMS);    
	return_val_if_fail(command != NULL, FTP_RET_INVALID_PARAMS);    
	ftp_send_command(ftpObj->commandChannel, command);    
	memset(ftpObj->replyString, 0x00, sizeof(ftpObj->replyString));    
	return ftp_get_reply(ftpObj);
}

static FTP_Obj *ftp_connect_server(const char *ipAddress, const int port)
{    
	FTP_Obj *ftpObj = (FTP_Obj *)malloc(sizeof(FTP_Obj));    
	return_val_if_fail(ipAddress != NULL, NULL);   
	
	return_val_if_fail(ftpObj != NULL, NULL);    
	ftp_init_obj(ftpObj);   
	ftpObj->commandChannel = ftp_socket_create();    
	if (ftpObj->commandChannel == INVALID_SOCKET)    
	{       
		fprintf(stderr, "creating socket failed.\n");
		goto error;    
	}    
	else    
	{       
		int ret = ftp_socket_connect(ftpObj->commandChannel, ipAddress, port);        
		if (ret != 0)
		{           
			fprintf(stderr, "connect to %s failed.\n", ipAddress);
			goto error;    
		}        
		else {            
			fprintf(stdout, "connected to %s.\n", ipAddress);            
			if (ftp_get_reply(ftpObj) == FTP_RET_OK)            
			{              
				return ftpObj;
			}            
			else           
			{                
				goto error;          
			}       
		}
	}

error:    
	SAFE_FREE(ftpObj);    

	return NULL;
}

static void ftp_login_enter_name(char *userName, const int maxSize, const char *ipAddress)
{    
	char enterNamePrompt[MAX_PROMPT_SIZE] = {0x00};    
	sprintf(enterNamePrompt, "Name (%s): ", ipAddress);    
	printf("%s", enterNamePrompt);    
	fgets(userName, maxSize, stdin);    
	userName[strlen(userName) - 1] = '\0';
}

static void ftp_login_enter_password(char *userPassword, const int maxSize)
{    
	char enterPwPrompt[MAX_PROMPT_SIZE] = {0x00};    
	sprintf(enterPwPrompt, "Password: ");    
	printf("%s", enterPwPrompt);    
	fgets(userPassword, maxSize, stdin);    
	userPassword[strlen(userPassword) - 1] = '\0';
}

static FTP_Ret ftp_login(FTP_Obj *ftpObj, const char *ipAddress,char* usrname,char*psd)
{    
	char command[COMMAND_SIZE];    
	//char userName[20] = {0x00};    
	//char userPassword[20] = {0x00};   
	return_val_if_fail(ftpObj != NULL, FTP_RET_INVALID_PARAMS);    
	return_val_if_fail(ipAddress != NULL, FTP_RET_INVALID_PARAMS);        
	
	//ftp_login_enter_name(userName, 20, ipAddress);    
	memset(command, 0x00, sizeof(command));    
	sprintf(command, "USER %s", usrname);    
	if (ftp_send_and_recv_reply(ftpObj, command) == FTP_RET_OK)    
	{       
		if (strncmp(ftpObj->replyString, "331", 3) != 0)       
		{            
			return FTP_RET_FAIL;        
		}       
		//ftp_login_enter_password(psd, 20);        
		memset(command, 0x00, sizeof(command));        
		sprintf(command, "PASS %s", psd);       
		if (ftp_send_and_recv_reply(ftpObj, command) == FTP_RET_OK)        
		{            
			if (strncmp(ftpObj->replyString, "230", 3) != 0)           
			{               
				return FTP_RET_FAIL;           
			}        
		}   
	}    
	else    
	{        
		return FTP_RET_FAIL;    
	}    
	return FTP_RET_OK;
}

static FTP_Ret ftp_syst(FTP_Obj *ftpObj, const char *command)
{    
	return_val_if_fail(ftpObj != NULL, FTP_RET_INVALID_PARAMS);    
	return ftp_send_and_recv_reply(ftpObj, "SYST");
}

static FTP_Ret ftp_set_transfer_type(FTP_Obj *ftpObj, const char *command)
{    
	return_val_if_fail(ftpObj != NULL, FTP_RET_INVALID_PARAMS);    
	return ftp_send_and_recv_reply(ftpObj, command);
}

static FTP_Ret ftp_set_transfer_mode(FTP_Obj *ftpObj, const char *command)
{    
	return_val_if_fail(ftpObj != NULL, FTP_RET_INVALID_PARAMS);    
	return ftp_send_and_recv_reply(ftpObj, command);
}
static int ftp_pasv(FTP_Obj *ftpObj)
{    
	char pasv[32] = {0x00};    
	char hostAddr[16] = {0x00};    
	unsigned int addr_1, addr_2, addr_3, addr_4, port_1, port_2;   
	return_val_if_fail(ftpObj != NULL, false);    
	
	ftpObj->dataChannel = ftp_socket_create();   
	if (ftp_send_and_recv_reply(ftpObj, "PASV") != FTP_RET_OK)    
	{        
		return false;   
	}   
	strcpy(pasv, strchr(ftpObj->replyString, '(') );    
	sscanf(pasv, "(%d, %d, %d, %d, %d, %d)",             &addr_1, &addr_2, &addr_3, &addr_4, &port_1, &port_2);    
	sprintf(hostAddr, "%d.%d.%d.%d", addr_1, addr_2, addr_3, addr_4);    
	if (ftp_socket_connect(ftpObj->dataChannel, hostAddr, (port_1 << 8) | port_2) != 0)    
	{        
		return false;   
	}    
	return true;
}

#define UC(arg)    ( ( (int)arg) & 0xff)

static int ftp_port(FTP_Obj *ftpObj)
{    
	char *addr, *port;    
	char command[256] = {0x00};    
	struct sockaddr_in dataAddr;    
	size_t len = sizeof(dataAddr);    
	return_val_if_fail(ftpObj != NULL, false);    
	
	if (getsockname(ftpObj->commandChannel, (struct sockaddr *)&dataAddr, &len) != 0)    
	{       
		return false;    
	}    
	ftpObj->dataChannel = ftp_socket_create();    
	if (ftpObj->dataChannel == INVALID_SOCKET)    
	{        
		return false;    
	}    
	if (bind(ftpObj->dataChannel, (struct sockaddr *)&dataAddr, len) != 0)    
	{        
		return false;  
	}   
	if (getsockname(ftpObj->dataChannel, (struct sockaddr *)&dataAddr, &len) != 0)    
	{        
		return false;    
	}  
	if (ftp_socket_listen(ftpObj->dataChannel, 1) != 0)    
	{        
		return false;   
	}    
	addr = (char *)&dataAddr.sin_addr;    
	port = (char *)&dataAddr.sin_port;    
	sprintf(command, "PORT %d, %d, %d, %d, %d, %d", UC(addr[0]), UC(addr[1]), UC(addr[2]), UC(addr[3]),  UC(port[0]), UC(port[1]) );   
	if (ftp_send_and_recv_reply(ftpObj, command) != FTP_RET_OK)   
	{        
		return false;   
	}  
	return true;
}

static int ftp_init_data_channel(FTP_Obj *ftpObj)
{    
	return_val_if_fail(ftpObj != NULL, false);    
	if (ftpObj->dataChannel != INVALID_SOCKET)   
	{       
		ftp_socket_close(ftpObj->dataChannel);   
	}   
	if (ftpObj->transMode == FTP_MODE_PASV)    
	{        
		return ftp_pasv(ftpObj);    
	}    
	else   
	{       
		return ftp_port(ftpObj);  

	}    
	return false;

}

static int ftp_build_data_channel(FTP_Obj *ftpObj)
{    
	return_val_if_fail(ftpObj != NULL, false);    
	if (ftpObj->transMode == FTP_MODE_PASV)    
	{        
		return true;  
	}    
	if (ftp_socket_select(ftpObj->dataChannel, FD_ACCEPT, ftpObj->secTimeOut) <= 0)    
	{        
		return false;   
	}    
	SOCKET_HANDLE socketHandle = ftp_socket_accept(ftpObj->dataChannel);    
	if (socketHandle == INVALID_SOCKET)    
	{        
		return false;   
	}    
	else   
	{        
		ftp_socket_close(ftpObj->dataChannel);      
		ftpObj->dataChannel = socketHandle;        
		ftp_set_nonblock(ftpObj->dataChannel);    // set nonblock       
		return true;    
	}    
	return false;
}

#define MAX_RECV_DATA_SIZE 1024 * 100

static char* ftp_recv_data_channnel(FTP_Obj *ftpObj, const char *command)
{
	char *recvBuf = (char *)malloc(sizeof(char) * MAX_RECV_DATA_SIZE);    
	if (ftp_init_data_channel(ftpObj) != true)    
	{        
		ftp_socket_close(ftpObj->dataChannel);        
		return NULL;    
	}    
	if (ftp_send_and_recv_reply(ftpObj, command) != FTP_RET_OK)    
	{       
		return NULL; 
	}    
	if (ftp_build_data_channel(ftpObj) != true)  
	{      
		return NULL;   
	}    
	
	if (recvBuf == NULL)    
	{        
		return NULL;
	}    
	memset(recvBuf, 0x00, MAX_RECV_DATA_SIZE);    
	if (ftp_recv_command(ftpObj->dataChannel, recvBuf, MAX_RECV_DATA_SIZE) < 0)    
	{       
		return NULL;  
	}    
	return recvBuf;
}

static FTP_Ret ftp_list(FTP_Obj *ftpObj, const char *command)
{    	
	char *recvBuf = ftp_recv_data_channnel(ftpObj, command);

	return_val_if_fail(ftpObj != NULL, FTP_RET_INVALID_PARAMS);    
	return_val_if_fail(command != NULL, FTP_RET_INVALID_PARAMS);    

	if (recvBuf != NULL)   
	{
		printf("%s", recvBuf);       

		memcpy(rbuf,recvBuf,100*1024);
		
		SAFE_FREE(recvBuf);        
		ftp_socket_close(ftpObj->dataChannel);       
		if (ftp_get_reply(ftpObj) == FTP_RET_OK)        
		{
			return FTP_RET_OK;
		}
	}
	return FTP_RET_FAIL;
}

static long ftp_get_file_size(const char *replyString)
{    
	char sizeBuf[10] = {0x00};   
	char *ptr = strchr(replyString, '(');    
	int index;    
	return_val_if_fail(replyString != NULL, -1);    
	
	for (index = 0, ptr += 1; index < 10; ++index, ++ptr)    
	{        
		if (*ptr != ' ')       
		{            
			sizeBuf[index] = *ptr;       
		}       
		else        
		{          
			break;       
		}   
	}    
	return atol(sizeBuf);
}


static int ftp_get_file_name(const char *replyString, char *fileName)
{    
	char *ptr = strstr(replyString, "for");    
	return_val_if_fail(replyString != NULL, -1);    
	
	if (ptr != NULL)    
	{       
		int index;        // (ptr += 4) : ignore the "for " substring        
		for (index = 0, ptr += 4; *ptr != '\0'; ++ptr, ++index)        
		{          
			if (*ptr != ' ')            
			{             
				fileName[index] = *ptr;      
			}
			else
			{             
				return true;         
			}      
		}   
	}   
	return false;
}
static FTP_Ret ftp_get_file(FTP_Obj *ftpObj, const char *command)
{    
	char *recvBuf = ftp_recv_data_channnel(ftpObj, command);   
	int i=0;
	return_val_if_fail(ftpObj != NULL, FTP_RET_INVALID_PARAMS);    
	return_val_if_fail(command != NULL, FTP_RET_INVALID_PARAMS);        

	if (recvBuf != NULL)    
	{        
		char fileName[256] = {0x00};       
		char fullname[256];
		FILE *fp;
		memset(fullname,0,256);
		sprintf(fullname,"%s","/mnt/udisk/");
		long fileSize = ftp_get_file_size(ftpObj->replyString);       
		if (ftp_get_file_name(ftpObj->replyString, fileName) == true)        
		{
			strcat(fullname,fileName);
			printf("filename==%s filesize = %d  %d\n",fullname,fileSize,__LINE__);
			fp = fopen(fullname,"ab+");
			if(fp==NULL)
			{
				printf("create local file error\n");
				fclose(fp);
				return FTP_RET_FAIL;
			}
				
			if (fileSize != 0)            
			{ 
				long writeSize = fwrite(recvBuf, sizeof(char),fileSize,fp);     
				fflush(fp);
				printf("writeSize==%d\n",writeSize);
				if (writeSize != fileSize)                
				{
					return FTP_RET_FAIL;       
				}            
			}           
			fclose(fp);        
		}        
		SAFE_FREE(recvBuf);        
		ftp_socket_close(ftpObj->dataChannel);       
		if (ftp_get_reply(ftpObj) == FTP_RET_OK)        
		{         
			return FTP_RET_OK;     
		}   
	}    
	return FTP_RET_FAIL;
}

static int ftp_put_file_name(const char *command, char *fileName)
{    
	char *ptr = strchr(command, ' ');    
	if (ptr != NULL)    
	{        
		int index = 0;        
		for (ptr += 1; *ptr != '\0'; ++index, ++ptr)        
		{           
			fileName[index] = *ptr;        
		}        
		return true;    
	}    
	return false;
}

static long ftp_put_file_size(const char *fileName)
{    
	struct stat buf;    
	if (stat(fileName, &buf) == -1)
	{
		return -1;
	}
	return buf.st_size;
}

static FTP_Ret ftp_put_file(FTP_Obj *ftpObj, const char *command)
{    
	char fileName[256] = {0x00};    
	return_val_if_fail(ftpObj != NULL, FTP_RET_INVALID_PARAMS);    
	return_val_if_fail(command != NULL, FTP_RET_INVALID_PARAMS);    
	if (ftpObj->commandChannel == INVALID_SOCKET)    
	{        
		return FTP_RET_FAIL;   
	}    
	if (ftp_init_data_channel(ftpObj) != true)    
	{        
		ftp_socket_close(ftpObj->dataChannel);        
		return FTP_RET_FAIL;    
	}    
	if (ftp_send_and_recv_reply(ftpObj, command) != FTP_RET_OK)    
	{     
		return FTP_RET_FAIL;   
	}    
	if (ftp_build_data_channel(ftpObj) != true)    
	{        
		return FTP_RET_FAIL;   
	}    
	

	if (ftp_put_file_name(command, fileName) == true)    
	{       
		long size = ftp_put_file_size(fileName);        
		printf("----->put: %s: %ld\n", fileName, size);        
		int fd = open(fileName, O_RDONLY);        
		if (fd != -1)        
		{            
			char *recvBuf = (char *)malloc(sizeof(char) * MAX_RECV_DATA_SIZE);           
			if (recvBuf != NULL)           
			{                
				ssize_t readSize = read(fd, recvBuf, size);       
				if (readSize <= 0)               
				{                 
					return FTP_RET_FAIL;            
				}    
				if (ftp_send_buffer(ftpObj->dataChannel, recvBuf, readSize) != -1)             
				{
					close(fd);                  
					ftp_socket_close(ftpObj->dataChannel);           
				}
				return ftp_get_reply(ftpObj);          
			}        
		}  
	}    

	return FTP_RET_FAIL;
}

static FTP_Ret ftp_delete_file(FTP_Obj *ftpObj, const char *command)
{    
	return_val_if_fail(ftpObj != NULL, FTP_RET_INVALID_PARAMS);    
	return_val_if_fail(command != NULL, FTP_RET_INVALID_PARAMS);        
	return ftp_send_and_recv_reply(ftpObj, command);
}

static FTP_Ret ftp_make_directory(FTP_Obj *ftpObj, const char *command)
{    
	return_val_if_fail(ftpObj != NULL, FTP_RET_INVALID_PARAMS);    
	return_val_if_fail(command != NULL, FTP_RET_INVALID_PARAMS);        
	return ftp_send_and_recv_reply(ftpObj, command);
}

static FTP_Ret ftp_remove_directory(FTP_Obj *ftpObj, const char *command)
{    
	return_val_if_fail(ftpObj != NULL, FTP_RET_INVALID_PARAMS);    
	return_val_if_fail(command != NULL, FTP_RET_INVALID_PARAMS);        
	return ftp_send_and_recv_reply(ftpObj, command);
}

static FTP_Ret ftp_change_directory(FTP_Obj *ftpObj, const char *command)
{    
	return_val_if_fail(ftpObj != NULL, FTP_RET_INVALID_PARAMS);    
	return_val_if_fail(command != NULL, FTP_RET_INVALID_PARAMS);        
	return ftp_send_and_recv_reply(ftpObj, command);
}

static FTP_Ret ftp_pwd(FTP_Obj *ftpObj, const char *command)
{    
	return_val_if_fail(ftpObj != NULL, FTP_RET_INVALID_PARAMS);    
	return_val_if_fail(command != NULL, FTP_RET_INVALID_PARAMS);       
	return ftp_send_and_recv_reply(ftpObj, "PWD");
}

static FTP_Ret ftp_close(FTP_Obj *ftpObj, const char *command)
{    
	return_val_if_fail(ftpObj != NULL, FTP_RET_INVALID_PARAMS);    
	return_val_if_fail(command != NULL, FTP_RET_INVALID_PARAMS);    
	if (ftpObj->commandChannel != INVALID_SOCKET)    
	{        
		FTP_Ret ret = ftp_send_and_recv_reply(ftpObj, command);       
		if (ret == FTP_RET_OK)       
		{           	
			ftp_socket_close(ftpObj->commandChannel);      
		}        
		else      
		{         
			return FTP_RET_FAIL;  
		}  
	}   
	if (ftpObj->dataChannel != INVALID_SOCKET)    
	{        
		ftp_socket_close(ftpObj->dataChannel);  
	}   
	SAFE_FREE(ftpObj);    
	return FTP_RET_OK;
}

static int ftp_command_filter(const char *command, char *filter)
{   
	int index = 0;   
	for ( ; *command != '\0'; ++index, ++command)   
	{       
		if (*command != ' ')       
		{          
			filter[index] = *command;
		}     
		else     
		{            
			return true;   
		} 
	}    
	return false;

}

static int ftp_replay_command(char *sendCommand, const char *commandName)
{    
	char temp[COMMAND_SIZE] = {0x00};    
	char *ptr = strchr(sendCommand, ' ');   
	if (ptr == NULL)   
	{        
		strcpy(sendCommand, commandName);       
		return true;  
	}   
	strcpy(temp, commandName);    
	strcat(temp, ptr);    
	if (strlen(temp) <= COMMAND_SIZE)    
	{        
		strcpy(sendCommand, temp);
		return true;   
	}   
	return false;
}

typedef struct _ftp_do_command
{   
	char command[COMMAND_SIZE];    
	FtpCommandFunc ftp_do_command;
}ftp_command_tag;

const FtpCommandFunc ftp_command_func[] =
{
	ftp_list,
	ftp_list,
	ftp_pwd,
	ftp_change_directory,
	ftp_set_transfer_type,
	ftp_set_transfer_type,
	ftp_set_transfer_mode,
	ftp_get_file,
	ftp_put_file,    
	ftp_delete_file,
	ftp_syst,
	ftp_make_directory,
	ftp_remove_directory,
	ftp_close,
	ftp_close,
};

const char *command_name[] = 
{"ls", "dir", "pwd", "cd", "ascii", "binary", "passive", "get", "put", "delete", "system",    "mkdir", "rmdir", "quit", "bye",};
const char *replace_command[] = 
{"LIST", "LIST", "PWD", "CWD", "TYPE A", "TYPE I", "passive", "RETR", "STOR", "DELE", "SYST",    "MKD", "RMD", "QUIT", "QUIT",};

static int ftp_return_command_index(const char *command)
{   
	int index;
	int number = sizeof(command_name) / sizeof(char *);    
	for (index = 0; index < number; ++index)    
	{       
		if (strcmp(command, command_name[index]) == 0)      
		{         
			return index;       
		}   
	}    
	return -1;
}
extern char* ip_addr;
extern char* usr;
extern char* usr_psd;
extern sem_t v_ftp;
extern sem_t v_ls;
extern char msg_content[128];
void ftp_entry(void* arg)
{

	char* ipAddress=ip_addr;;
	char* usrname=usr;
	char* psd=usr_psd;
	int port = 21;
	int i=0;

	FTP_Obj *ftpObj = ftp_connect_server(ipAddress, port);
	
	if (ftpObj != NULL)
	{
		ftp_login(ftpObj, ipAddress,usrname,psd);
		ftp_syst(ftpObj, NULL);
		ftp_set_transfer_mode(ftpObj, "TYPE I");
		printf("loging usrname=%s,psd=%s\n",usrname,psd);
		while (1)
		{
			char enterCommand[256] = {0x00};
			char filter[256] = {0x00};
			int index;
			_ftp_sem_wait(&v_ftp);
			memset(enterCommand, 0x00, sizeof(enterCommand));
			
			memcpy(enterCommand,msg_content,sizeof(msg_content));
			
			//enterCommand[strlen(enterCommand)] = '\0';
			printf("receive cmd==%s\n",enterCommand);		
			ftp_command_filter(enterCommand, filter);
			index = ftp_return_command_index(filter);
			if (index >= 0)
			{
				ftp_replay_command(enterCommand, replace_command[index]);
				printf("-------->%s\n", enterCommand);
				ftp_command_func[index](ftpObj, enterCommand);
				_ftp_sem_post(&v_ls);
			}
			if (strcmp(enterCommand, "QUIT") == 0)
			{
				break;
			}
			else if (strcmp(enterCommand, "") == 0)
			{
				continue;
			}
			
		
		}    
	}
	//return FTP_RET_OK;
}

