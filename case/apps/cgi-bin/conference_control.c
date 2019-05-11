#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "htmlsetting_cgi.h"

#define SOCKET_ERROR        -1

#define PICO_QUERY_INFO				5
#define PICO_CONTROL				10

#define TCPCLIENT_HDR_SIZE			8 //Sequence(4)+TotalBytes(4)
#define TCPCOMMON_HDR_SIZE			24 //TCP frame common header
#define TCPPACKET_HDR_SIZE			(TCPCLIENT_HDR_SIZE+TCPCOMMON_HDR_SIZE) // TCP frame complete header

#define TCP_HDR_QUERYINFO_OFFSET	(TCPCLIENT_HDR_SIZE+8)
#define TCP_HDR_QUERYISIZE_OFFSET	(TCPCLIENT_HDR_SIZE+12)
#define TCP_HDR_QUERYRESULT_OFFSET	(TCPCLIENT_HDR_SIZE+16)

#define QUERY_INFO_OK				0x1

#define INFO_TCP_CONNECTIONS		2

#define TCP_HDR_CTRLRESULT_OFFSET	(TCPCLIENT_HDR_SIZE+20)

#define CONTROL_STOP				1
#define CONTROL_CHANGE				2
#define CONTROL_DISCONNECT			3
#define CONTROL_DISCONNECT_ALL		4
#define CONTROL_ASSIGN_HOST			5


#define FLAG_DPF_TO_DPF				0x2

#define DEFAULT_HOST_NAME	"127.0.0.1"
#define DEFAULT_HOST_PORT	0x0979

#define BUF_LEN 			1024*1024

//#define HOST_SOCKET_CMD

int debug_mode = 0;

typedef struct ezWifiPkt_s ezWifiPkt_t;
struct ezWifiPkt_s 
{
	unsigned long seqNo;
	unsigned long totalSize;
	unsigned long tag;
	unsigned char flag;
	unsigned char len;
	unsigned char reserve[2];
	unsigned char cdb[16];
};

typedef struct ezWifiQueryCdb_s ezWifiQueryCdb_t;
struct ezWifiQueryCdb_s 
{
	unsigned long info;
	unsigned long iSize;
	unsigned long result;
	unsigned long reserve;
};

typedef struct ezWifiReqCdb_s ezWifiReqCdb_t;
struct ezWifiReqCdb_s 
{
	unsigned long command;
	unsigned long splitSize;
	unsigned long displayId;
	union
	{
		unsigned long result;
		unsigned long iSize;
	};
};

int SendAndGetResponse(int hSocket, ezWifiPkt_t *pkt, unsigned char *buf, size_t buf_len, unsigned int data_size)
{
	int ret;
	
	ret = write(hSocket, pkt, sizeof(ezWifiPkt_t));
	if (ret != sizeof(ezWifiPkt_t))
	{
		if (debug_mode)
			fprintf(cgiOut, "write pkt error\n");

		return -1;
	}

	if (data_size != 0)
	{
		ret = write(hSocket, buf, data_size);
		if (ret != data_size)
		{
			if (debug_mode)
				fprintf(cgiOut, "write buf error\n");

			return -1;
		}
	}
	
	// read result
	fd_set readFds;
	struct timeval timeout;
	int nRead = 0;
	
	FD_ZERO(&readFds);
	FD_SET(hSocket, &readFds);
	timeout.tv_sec = 3;
	timeout.tv_usec = 0;

	ret = select(hSocket+1, &readFds, NULL, NULL, &timeout);
	if (ret == 0) //Timeout
	{
		if (debug_mode)
			fprintf(cgiOut, "select timeout\n");

		return -1;
	}
	else if (ret > 0)
	{
		if (FD_ISSET(hSocket, &readFds))
		{
			nRead = read(hSocket, buf, TCPPACKET_HDR_SIZE);
			if (nRead != TCPPACKET_HDR_SIZE)
			{
				if (debug_mode)
					fprintf(cgiOut, "read error\n");

				return -1;
			}
		}
	}
	else
	{
		if (debug_mode)
			fprintf(cgiOut, "select error\n");

		return -1;
	}

	return 0;
}

#ifdef HOST_SOCKET_CMD
int QueryTCPConnections(int hSocket)
{
	unsigned char buf[BUF_LEN];

	ezWifiPkt_t pkt;
	memset(&pkt, 0, sizeof(ezWifiPkt_t));
	
	pkt.seqNo = 0;
	pkt.totalSize = TCPCOMMON_HDR_SIZE;
	pkt.tag = PICO_QUERY_INFO;
	pkt.flag = FLAG_DPF_TO_DPF;
	pkt.len = sizeof(ezWifiQueryCdb_t);

	ezWifiQueryCdb_t cdb;
	memset(&cdb, 0, sizeof(ezWifiQueryCdb_t));
	cdb.info = INFO_TCP_CONNECTIONS;
	memcpy(&pkt.cdb, &cdb, sizeof(ezWifiQueryCdb_t));

	unsigned int data_size = 0;	
	if (SendAndGetResponse(hSocket, &pkt, buf, sizeof(buf), 0) == 0)
	{
		unsigned long result = ((unsigned long *)(buf+TCP_HDR_QUERYRESULT_OFFSET))[0];
		if (result != QUERY_INFO_OK)
		{
			if (debug_mode)
				fprintf(cgiOut, "result != QUERY_INFO_OK, result=%ld\n", result);

			return -1;
		}
		
		data_size = ((unsigned long*)(buf+TCP_HDR_QUERYISIZE_OFFSET))[0];
		if (data_size == 0)
		{
			if (debug_mode)
				fprintf(cgiOut, "data_size == 0\n");

			return 0;
		}
		
		int nRead = read(hSocket, buf, data_size);
		if (nRead != data_size)
		{
			if (debug_mode)
				fprintf(cgiOut, "nRead != data_size, nRead=%d, data_size=%d\n", nRead, data_size);

			return -1;
		}
	
		if (data_size + 1 <= sizeof(buf))
		{
			buf[data_size] = 0; // add null terminator
		}

		fprintf(cgiOut, "%s", buf);
	}

	return 0;
}
#else
int QueryTCPConnections()
{
	char cmd_string[512]={0};
	char *ret_string;
	int ret_len;

	snprintf(cmd_string, sizeof(cmd_string), "query_tcp_connections");
	int ret = htmlsetting_func("htmlsetting_confCtrlAndReturn", cmd_string, strlen(cmd_string)+1, (void **)&ret_string, &ret_len);
	fprintf(cgiOut, "%s", ret_string);
	del_return((void **)&ret_string);
	return ret;
}
#endif

//查找字符串最后一次出现的位置
char *strrstr (const char*string, const char*str)
{
    char *index = NULL;
    const char *strcmd = NULL;
    char *ret = NULL;
    strcmd=string;
    do{
        index = strstr(strcmd, str);
        if (index != NULL){
            ret = index;
            strcmd=index+10;
        }
    }while(index != NULL);
    return ret;
}

#ifdef HOST_SOCKET_CMD
int find_host_ip(int hSocket)
{
	unsigned char buf[BUF_LEN];
	char setcmd_tmp[20]={0};
	char *find_host_ip=NULL;
	char *locate_start=NULL;
	char *locate_end=NULL;
	char *locate=NULL;

	ezWifiPkt_t pkt;
	memset(&pkt, 0, sizeof(ezWifiPkt_t));
	
	pkt.seqNo = 0;
	pkt.totalSize = TCPCOMMON_HDR_SIZE;
	pkt.tag = PICO_QUERY_INFO;
	pkt.flag = FLAG_DPF_TO_DPF;
	pkt.len = sizeof(ezWifiQueryCdb_t);

	ezWifiQueryCdb_t cdb;
	memset(&cdb, 0, sizeof(ezWifiQueryCdb_t));
	cdb.info = INFO_TCP_CONNECTIONS;
	memcpy(&pkt.cdb, &cdb, sizeof(ezWifiQueryCdb_t));

	unsigned int data_size = 0;	
	if (SendAndGetResponse(hSocket, &pkt, buf, sizeof(buf), 0) == 0)
	{
		unsigned long result = ((unsigned long *)(buf+TCP_HDR_QUERYRESULT_OFFSET))[0];
		if (result != QUERY_INFO_OK)
		{
			if (debug_mode)
				fprintf(cgiOut, "result != QUERY_INFO_OK, result=%ld\n", result);

			return -1;
		}
		
		data_size = ((unsigned long*)(buf+TCP_HDR_QUERYISIZE_OFFSET))[0];
		if (data_size == 0)
		{
			if (debug_mode)
				fprintf(cgiOut, "data_size == 0\n");

			return 0;
		}
		
		int nRead = read(hSocket, buf, data_size);
		if (nRead != data_size)
		{
			if (debug_mode)
				fprintf(cgiOut, "nRead != data_size, nRead=%d, data_size=%d\n", nRead, data_size);

			return -1;
		}
	
		if (data_size + 1 <= sizeof(buf))
		{
			buf[data_size] = 0; // add null terminator
		}
		
		//locate=buf;
		locate_start=strstr(buf,"host");
		if(locate_start==NULL){
			fprintf(cgiOut,"null");
			return -1;
		}
		else{
			//memset(setcmd_tmp,0,sizeof(setcmd_tmp));
			//strncpy(setcmd_tmp,buf,strlen(buf)-strlen(locate_start));
			//locate=setcmd_tmp;
			locate=(char *)malloc(BUF_LEN*sizeof(char));
			if(locate != NULL){
				strncpy(locate,buf,strlen(buf)-strlen(locate_start));
			}
			locate_start=strrstr(locate,"<IPAddress>");
			
			locate_start+=strlen("<IPAddress>");
			locate_end=strstr(locate_start,"</IPAddress>");
			
			find_host_ip=(char *)malloc(16*sizeof(char));
			if(find_host_ip != NULL){
				strncpy(find_host_ip,locate_start,strlen(locate_start)-strlen(locate_end));
				strncpy(setcmd_tmp, find_host_ip,strlen(find_host_ip));
			}

			fprintf(cgiOut,"%s\n",setcmd_tmp);

			free(find_host_ip);
			free(locate);
			return 0;
		}
	}

	return 0;
}
#else
int find_host_ip()
{
	char cmd_string[512]={0};
	char *ret_string;
	int ret_len;

	snprintf(cmd_string, sizeof(cmd_string), "find_host");
	int ret = htmlsetting_func("htmlsetting_confCtrlAndReturn", cmd_string, strlen(cmd_string)+1, (void **)&ret_string, &ret_len);
	fprintf(cgiOut, "%s", ret_string);
	del_return((void **)&ret_string);
	return ret;
}
#endif

#ifdef HOST_SOCKET_CMD
int HandleControl(int hSocket)
{
	unsigned char buf[1024];

	ezWifiPkt_t pkt;
	memset(&pkt, 0, sizeof(ezWifiPkt_t));
	
	pkt.seqNo = 0;
	pkt.totalSize = TCPCOMMON_HDR_SIZE;
	pkt.tag = PICO_CONTROL;
	pkt.flag = FLAG_DPF_TO_DPF;
	pkt.len = sizeof(ezWifiReqCdb_t);
	
	ezWifiReqCdb_t cdb;
	memset(&cdb, 0, sizeof(ezWifiReqCdb_t));

	char control[1024];
	memset(control, 0, sizeof(control));
	cgiFormString("control", control, sizeof(control));
	if (strcmp(control, "control_stop") == 0)
	{
		cdb.command = CONTROL_STOP;
	}
	else if (strcmp(control, "control_change") == 0)
	{
		cdb.command = CONTROL_CHANGE;
		
		char control_data[1024];
		memset(control_data, 0, sizeof(control_data));
		cgiFormString("split_count", control_data, sizeof(control_data));
		int split_count = atoi(control_data);
		if (split_count < 1 || split_count > 4)
		{
			if (debug_mode)
				fprintf(cgiOut, "invalid split_count: %d \n", split_count);

			return -1;
		}
		else
		{
			cdb.splitSize = split_count;
		}

		memset(control_data, 0, sizeof(control_data));
		cgiFormString("position", control_data, sizeof(control_data));
		int position = atoi(control_data);
		if (position < 1 || position > 4)
		{
			if (debug_mode)
				fprintf(cgiOut, "invalid position: %d \n", position);

			return -1;
		}
		else
		{
			cdb.displayId = position;
		}
	}
	else if (strcmp(control, "control_disconnect") == 0)
	{
		cdb.command = CONTROL_DISCONNECT;	
	}
	else if(strcmp(control, "control_disconnect_all") == 0)
	{
		cdb.command = CONTROL_DISCONNECT_ALL;	
	}
	else if(strcmp(control, "control_assign_host") == 0)
	{
		cdb.command = CONTROL_ASSIGN_HOST;	
	}
	else
	{
		if (debug_mode)
			fprintf(cgiOut, "unknown control: %s\n", control);

		return -1;
	}

	memset(buf, 0, sizeof(buf));
	cgiFormString("ip_address", buf, sizeof(buf));
	if (strlen(buf) == 0)
	{
		if (debug_mode)
			fprintf(cgiOut, "no ip address\n");

		return -1;
	}
	
	cdb.iSize = strlen(buf);
	memcpy(&pkt.cdb, &cdb, sizeof(ezWifiReqCdb_t));

	unsigned int data_size = cdb.iSize;
	pkt.totalSize += data_size;

	if (SendAndGetResponse(hSocket, &pkt, buf, sizeof(buf), data_size) == 0)
	{
		unsigned long result = ((unsigned long *)(buf+TCP_HDR_CTRLRESULT_OFFSET))[0];

		fprintf(cgiOut, "%ld", result);
	}
	
	return 0;
}
#else
int HandleControl()
{
	char webval[1024];
	char cmd[512];

	memset(webval, 0, sizeof(webval));
	cgiFormString("control", webval, sizeof(webval));
	if(strcmp(webval, "control_disconnect_all") == 0)
	{
		snprintf(cmd, sizeof(cmd), "control_disconnect_all");
	}
	else
	{
		char ipAddr[128];
		memset(ipAddr, 0, sizeof(ipAddr));
		cgiFormString("ip_address", ipAddr, sizeof(ipAddr));
		if (strlen(ipAddr) == 0)
		{
			if (debug_mode)
				fprintf(cgiOut, "no ip address\n");
		
			return -1;
		}
		
		if (strcmp(webval, "control_stop") == 0)
		{
			snprintf(cmd, sizeof(cmd), "control_stop,ip_address=%s", ipAddr);
		}
		else if (strcmp(webval, "control_change") == 0)
		{
			char control_data[64];
			memset(control_data, 0, sizeof(control_data));
			cgiFormString("split_count", control_data, sizeof(control_data));
			int split_count = atoi(control_data);
			if (split_count < 1 || split_count > 4)
			{
				if (debug_mode)
					fprintf(cgiOut, "invalid split_count: %d \n", split_count);
			
				return -1;
			}
			
			memset(control_data, 0, sizeof(control_data));
			cgiFormString("position", control_data, sizeof(control_data));
			int position = atoi(control_data);
			if (position < 1 || position > 4)
			{
				if (debug_mode)
					fprintf(cgiOut, "invalid position: %d \n", position);
			
				return -1;
			}
			snprintf(cmd, sizeof(cmd), "control_change,ip_address=%s,split_count=%d,position=%d", ipAddr, split_count, position);
		}
		else if (strcmp(webval, "control_disconnect") == 0)
		{
			snprintf(cmd, sizeof(cmd), "control_disconnect,ip_address=%s", ipAddr);
		}
		else if(strcmp(webval, "control_assign_host") == 0)
		{
			snprintf(cmd, sizeof(cmd), "control_assign_host,ip_address=%s", ipAddr);
		}
		else
		{
			if (debug_mode)
				fprintf(cgiOut, "unknown control: %s\n", webval);
		
			return -1;
		}
	}

	int ret = htmlsetting_func("htmlsetting_confCtrl", cmd, strlen(cmd)+1, NULL, NULL);
	fprintf(cgiOut, "%d", ret);
	return ret;
}
#endif

int cgiMain() {
	char debug[1024];
	memset(debug, 0, sizeof(debug));
	cgiFormString("debug", debug, sizeof(debug));
	if (strcmp(debug, "1") == 0)
	{
		debug_mode = 1;
	}

#ifdef HOST_SOCKET_CMD
	int hSocket;                 /* handle to socket */
    struct sockaddr_in Address;  /* Internet socket address stuct */

    /* make a socket */
    hSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(hSocket == SOCKET_ERROR)
    {
		if (debug_mode)
			fprintf(cgiOut, "socket error\n");

        return -1;
    }

    /* fill address struct */
    Address.sin_addr.s_addr=inet_addr(DEFAULT_HOST_NAME);
    Address.sin_port=htons(DEFAULT_HOST_PORT);
    Address.sin_family=AF_INET;
	
    /* connect to host */
	system("ifconfig lo up");
    if(connect(hSocket,(struct sockaddr*)&Address,sizeof(Address)) == SOCKET_ERROR)
    {
		if (debug_mode)
			fprintf(cgiOut, "connect error\n");

		return -1;
    }	
#endif

	cgiHeaderContentType("text/plain");
	
	char type[1024];
	memset(type, 0, sizeof(type));
	cgiFormString("type", type, sizeof(type));
	if (strcmp(type, "query_tcp_connections") == 0)
	{
#ifdef HOST_SOCKET_CMD
		QueryTCPConnections(hSocket);
#else
		QueryTCPConnections();
#endif
	}
	else if (strcmp(type, "find_host") == 0)
	{	
#ifdef HOST_SOCKET_CMD
		find_host_ip(hSocket);
#else
		find_host_ip();
#endif
	}
	else if (strcmp(type, "control") == 0)
	{
#ifdef HOST_SOCKET_CMD
		HandleControl(hSocket);
#else
		HandleControl();
#endif
	}
	else
	{
		if (debug_mode)
			fprintf(cgiOut, "unknown type: %s\n", type);

	}

#ifdef HOST_SOCKET_CMD
    if(close(hSocket) == SOCKET_ERROR)
    {
		if (debug_mode)
			fprintf(cgiOut, "close error\n");

        return -1;
    }
#endif
	
	return 0;
}
