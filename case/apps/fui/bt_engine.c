///////////////////基础部分///////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "swf_types.h"
#include "swfdec.h"
#include "swf_ext.h"
///////////////////蓝牙部分///////////////////////

#ifdef MODULE_CONFIG_BLUETOOTH

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
 
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#ifdef _WIN32
#include <winsock2.h>
#define S_IRGRP 0
#define S_IROTH 0
#define S_IWGRP 0
#define S_IWOTH 0
#define S_IXGRP 0
#define S_IXOTH 0
#define sleep(n)	Sleep((n) * 1000)
#define mkdir(dir,mode)	_mkdir(dir)
#define lstat stat
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

/* just until there is a server layer in obexftp */
#include <obex.h>
#include <obexftp.h>
#include <object.h>
#include <common.h>
#define EOLCHARS "\n"

/* Application defined headers */
#define HDR_CREATOR  0xcf	/* so we don't require OpenOBEX 1.3 */

#define WORK_PATH_MAX		128
#define CUR_DIR				"./"

#define OBEX_TRANS_BLUETOOTH	4
#define isspace(c)	((c) == ' ')
static char *device = NULL;
static int channel = 10; /* OBEX_PUSH_HANDLE */

static char init_work_path[WORK_PATH_MAX];

volatile int finished = 0;
volatile int success = 0;
volatile int obexftpd_reset = 0;

uint32_t connection_id = 0;
int verbose = 0;

/*--------the bluetooth Global variables need in the function --------*/
#if 1
	#define btdbg_info(fmt, arg...) printf("BTINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
	
#else
	#define btdbg_info(fmt, arg...)
#endif
static int bt_wt = 0;  //the flages of write
bdaddr_t bdaddr;
//uint8_t channel = 0;
static pthread_t bt_id;
static int bt_status = -1;
obex_t *handle;

#define AM_CASE_SC_DIR         	"/am7x/case/scripts"
enum bt_status_t{
	BT_UNKOWN =-1,
	BT_CONN ,             /*connect*/
        BT_DISCON,          /*disconnect*/
	BT_REVINGS,         /*receiving*/ 
	BT_REVDONES,      /*receive done*/
	BT_SNDS,               /*sending*/
	BT_ADNORMAL       /*abnormal*/
} ;

/*--------the bluetooth Global variables---------------------------*/


//BEGIN of compositor the folder listing XML document
struct rawdata_stream {
	char		*data;
	unsigned int	block_size;
	unsigned int 	size;
	unsigned int 	max_size;
};

static struct rawdata_stream* INIT_RAWDATA_STREAM(unsigned int size)
{
	struct rawdata_stream *stream = malloc(sizeof(*stream));
	if (NULL == stream)
	{
		return NULL;
	}
	
	stream->data = malloc(size);
	if (NULL == stream->data)
	{
		free(stream);
		return NULL;
	}

       	memset(stream->data,0,size);
       	stream->size = 0;
       	stream->block_size = size;
       	stream->max_size = size;

	return stream;
}

static int ADD_RAWDATA_STREAM_DATA(struct rawdata_stream *stream, const char *data)
{
	char   		*databuf;
	unsigned int 	size;

	if (data == NULL) return 0;
	size = strlen(data);
	if (size == 0) return 0;
	if ((size + stream->size) >= stream->max_size)
	{
		while((size + stream->size) >= stream->max_size)
		{
			stream->max_size += stream->block_size;
		}
		databuf = realloc(stream->data, stream->max_size);
		if (NULL == databuf)
		{
			fprintf(stderr, "realloc() failed\n");
			return -1; 
		}
		stream->data = databuf;
	}
	strcat(stream->data, data);
	stream->size += size;
	return size;
}

static void FREE_RAWDATA_STREAM(struct rawdata_stream *stream)
{
	free(stream->data);
	free(stream);
}
	
#define FL_XML_VERSION(stream) \
	ADD_RAWDATA_STREAM_DATA(stream, "<?xml version=\"1.0\"?>" EOLCHARS);

#define FL_XML_TYPE(stream) \
	ADD_RAWDATA_STREAM_DATA(stream, "<!DOCTYPE folder-listing SYSTEM \"obex-folder-listing.dtd\">" EOLCHARS);

#define FL_XML_BODY_BEGIN(stream) \
	ADD_RAWDATA_STREAM_DATA(stream, "<folder-listing version=\"1.0\">" EOLCHARS);

#define FL_XML_BODY_END(stream) \
	ADD_RAWDATA_STREAM_DATA(stream, "</folder-listing>" EOLCHARS);

#define FL_XML_BODY_ITEM_BEGIN(stream) \
	ADD_RAWDATA_STREAM_DATA(stream, "<");

#define FL_XML_BODY_ITEM_END(stream) \
	ADD_RAWDATA_STREAM_DATA(stream, "/>" EOLCHARS);

inline static void FL_XML_BODY_FOLDERNAME(struct rawdata_stream *stream, char *name)
{
	ADD_RAWDATA_STREAM_DATA(stream, "folder name=\""); 
	ADD_RAWDATA_STREAM_DATA(stream, name);		
	ADD_RAWDATA_STREAM_DATA(stream, "\" ");
}

inline static void FL_XML_BODY_FILENAME(struct rawdata_stream *stream, char *name)
{
	ADD_RAWDATA_STREAM_DATA(stream, "file name=\""); 
	ADD_RAWDATA_STREAM_DATA(stream, name);		
	ADD_RAWDATA_STREAM_DATA(stream, "\" ");
}

inline static void FL_XML_BODY_SIZE(struct rawdata_stream *stream, unsigned int size)	
{
	const char format[] = "size=\"%d\" ";
	char str_size[sizeof(format)+14];

	snprintf(str_size,sizeof(str_size), format, size);
	ADD_RAWDATA_STREAM_DATA(stream, str_size);
}

inline static void FL_XML_BODY_PERM(struct rawdata_stream *stream, mode_t file, mode_t dir)	
{
	const char format[] = "user-perm=\"%s%s%s\" ";
	char str_perm[sizeof(format)];

	snprintf(str_perm,sizeof(str_perm),format,
		 (file & (S_IRUSR|S_IRGRP|S_IROTH)? "R": ""),
		 (file & (S_IWUSR|S_IWGRP|S_IWOTH)? "W": ""),
		 (dir & (S_IWUSR|S_IWGRP|S_IWOTH)? "D": ""));
	ADD_RAWDATA_STREAM_DATA(stream,str_perm);
}

inline static void FL_XML_BODY_TIME(struct rawdata_stream *stream,
                                    const char* type,
                                    time_t time)
{
	struct tm* tm = gmtime(&time);
	char str_tm[sizeof("=\"yyyymmddThhmmssZ\" ")];

	if (tm == NULL || stream == NULL ||
	    strftime(str_tm,sizeof(str_tm),"=\"%Y%m%dT%H%M%SZ\" ",tm) == 0)
		return;
	ADD_RAWDATA_STREAM_DATA(stream, type);
	ADD_RAWDATA_STREAM_DATA(stream, str_tm);
}

#define FL_XML_BODY_MTIME(stream,time) \
        FL_XML_BODY_TIME(stream,"modified",time)
#define FL_XML_BODY_CTIME(stream,time) \
        FL_XML_BODY_TIME(stream,"created",time)
#define FL_XML_BODY_ATIME(stream,time) \
        FL_XML_BODY_TIME(stream,"accessed",time)

//END of compositor the folder listing XML document

inline static int is_type_fl(const char *type)
{
	return (type && strcmp(type, XOBEX_LISTING) == 0);
}

static int connect_server(obex_t *handle, obex_object_t *object)
{
	obex_headerdata_t hv;
	uint8_t hi;
	uint32_t hlen;
	uint8_t *target = NULL;
	int target_len = 0;
	int ret;

	while (OBEX_ObjectGetNextHeader(handle, object, &hi, &hv, &hlen)) {
		switch(hi) {
		case OBEX_HDR_TARGET:
			if (0 < hlen)
			{
				if( (target = malloc(hlen))) {
					target_len = hlen;
					memcpy(target,hv.bs,target_len);
				}
			}
			break;
		default:	
			printf("%s() Skipped header %02x\n", __FUNCTION__, hi);
			break;
		}
	}

	OBEX_ObjectSetRsp(object, OBEX_RSP_SUCCESS, OBEX_RSP_SUCCESS);
	hv.bq4 = connection_id;
	if(OBEX_ObjectAddHeader(handle, object, OBEX_HDR_CONNECTION,
              		hv, sizeof(hv.bq4),
                            OBEX_FL_FIT_ONE_PACKET) < 0 )    {
                fprintf(stderr, "Error adding header CONNECTION\n");
                OBEX_ObjectDelete(handle, object);
                return -1;
        }
	if (target && target_len) {
		hv.bs = target;
		if(OBEX_ObjectAddHeader(handle,object,OBEX_HDR_WHO,
					hv,target_len,OBEX_FL_FIT_ONE_PACKET) < 0 ) {
			fprintf(stderr, "Error adding header WHO\n");
			OBEX_ObjectDelete(handle, object);
			return -1;
		}
		free(target);
	} 
	return 0;
}


static void set_server_path(obex_t *handle, obex_object_t *object)
{
	char *name = NULL;
	char fullname[WORK_PATH_MAX];

	// "Backup Level" and "Don't Create" flag in first byte
	uint8_t setpath_nohdr_dummy = 0;
	uint8_t *setpath_nohdr_data;
	obex_headerdata_t hv;
	uint8_t hi;
	uint32_t hlen;

	OBEX_ObjectGetNonHdrData(object, &setpath_nohdr_data);
	if (NULL == setpath_nohdr_data) {
		setpath_nohdr_data = &setpath_nohdr_dummy;
		printf("nohdr data not found\n");
	}
	printf("nohdr data: %x\n", *setpath_nohdr_data);

	while(OBEX_ObjectGetNextHeader(handle, object, &hi, &hv, &hlen))	{
		switch(hi)	{
		case OBEX_HDR_NAME:
			printf("%s() Found name\n", __FUNCTION__);
			if (0 < hlen)
			{
				if( (name = malloc(hlen / 2)))	{
					OBEX_UnicodeToChar((uint8_t*)name, hv.bs, hlen);
					printf("name:%s\n", name);
				}
			}
			else
			{
				if (verbose) printf("set path to root\n");
				chdir(init_work_path);
			}
			break;
			
		default:
			printf("%s() Skipped header %02x\n", __FUNCTION__, hi);
		}
	}	
	
	if (name)
	{
		if (strstr(name, "/../"))
		{
			OBEX_ObjectSetRsp(object, OBEX_RSP_CONTINUE, OBEX_RSP_FORBIDDEN);
		} else {
		strcpy(fullname, CUR_DIR);
		strncat(fullname, name, sizeof(fullname)-1);
		if ((*setpath_nohdr_data & 2) == 0) {
			if (verbose) printf("mkdir %s\n", name);
			if (mkdir(fullname, 0755) < 0) {
				perror("requested mkdir failed");
			}
		}
		if (verbose) printf("Set path to %s\n",fullname);
		if (chdir(fullname) < 0)
		{
			perror("requested chdir failed\n");
			OBEX_ObjectSetRsp(object, OBEX_RSP_CONTINUE, OBEX_RSP_FORBIDDEN);
		}
		}
		free(name);
		name = NULL;
	}
}

/*
  * Get the filesize in a "portable" way
  */
static int get_filesize(const char *filename)
{
	struct stat stats;
	stat(filename, &stats);
	if (S_ISDIR(stats.st_mode)) {
		fprintf(stderr,"GET of directories not implemented !!!!\n");
		return -1;
	}
	return (int) stats.st_size;
}

/*
  * Read a file and alloc a buffer for it
  */
static uint8_t* easy_readfile(const char *filename, int *file_size)
{
	int actual;
	int fd;
	uint8_t *buf;

	*file_size = get_filesize(filename);
	printf("name=%s, size=%d\n", filename, *file_size);

	fd = open(filename, O_RDONLY, 0);

	if (fd == -1)
	{
		return NULL;
	}
	
	buf = malloc(*file_size);
	if(buf == NULL)
	{
		return NULL;
	}

	actual = read(fd, buf, *file_size);
	close(fd); 

	*file_size = actual;
	return buf;
}

static void get_server(obex_t *handle, obex_object_t *object)
{
	uint8_t *buf = NULL;

	obex_headerdata_t hv;
	uint8_t hi;
	uint32_t hlen;
	int file_size;

	char *name = NULL;
	char *type = NULL;
	printf("%s()\n", __FUNCTION__);

	while(OBEX_ObjectGetNextHeader(handle, object, &hi, &hv, &hlen))	{
		switch(hi)	{
		case OBEX_HDR_NAME:
			printf("%s() Found name\n", __FUNCTION__);
			if( (name = malloc(hlen / 2)))	{
				OBEX_UnicodeToChar((uint8_t*)name, hv.bs, hlen);
				printf("name:%s\n", name);
			}
			break;
			
		case OBEX_HDR_TYPE:
			if( (type = malloc(hlen + 1)))	{
				strcpy(type, (char *)hv.bs);
			}
			printf("%s() type:%s\n", __FUNCTION__, type);

		case 0xbe: // user-defined inverse push
			printf("%s() Found inverse push req\n", __FUNCTION__);
       			printf("data:%02x\n", hv.bq1);
			break;
			

		case OBEX_HDR_APPARAM:
			printf("%s() Found apparam\n", __FUNCTION__);
       			printf("name:%d (%02x %02x ...)\n", hlen, *hv.bs, *(hv.bs+1));
			break;
			
		default:
			printf("%s() Skipped header %02x\n", __FUNCTION__, hi);
		}
	}

	if (is_type_fl(type))
	{
		struct dirent		*dirp;
		DIR			*dp;
		struct stat		statdir;
		struct stat		statbuf;
		char			*filename;
		struct rawdata_stream	*xmldata;

		xmldata = INIT_RAWDATA_STREAM(512);
		if (NULL == xmldata)
			goto out;

		FL_XML_VERSION(xmldata);
		FL_XML_TYPE(xmldata);
		FL_XML_BODY_BEGIN(xmldata);

		stat(CUR_DIR, &statdir);
		dp = opendir(CUR_DIR);
		while(NULL != dp && NULL != (dirp = readdir(dp))) 
		{
			if (0 == strcmp(dirp->d_name, ".") || 0 == strcmp(dirp->d_name, ".."))
				continue;

			FL_XML_BODY_ITEM_BEGIN(xmldata);

			//Adding 1 bytes due to containing '\0' 
			filename = malloc(strlen(CUR_DIR) + strlen(dirp->d_name) + 1);
			strcpy(filename, CUR_DIR);
			strcat(filename, dirp->d_name);

			lstat(filename, &statbuf);
			if (0 == S_ISDIR(statbuf.st_mode)) 
				FL_XML_BODY_FILENAME(xmldata, dirp->d_name);				
			else	
				FL_XML_BODY_FOLDERNAME(xmldata, dirp->d_name);
			
			FL_XML_BODY_SIZE(xmldata, statbuf.st_size);
			FL_XML_BODY_PERM(xmldata, statbuf.st_mode, statdir.st_mode);
			FL_XML_BODY_MTIME(xmldata, statbuf.st_mtime);
			FL_XML_BODY_CTIME(xmldata, statbuf.st_ctime);
			FL_XML_BODY_ATIME(xmldata, statbuf.st_atime);

			FL_XML_BODY_ITEM_END(xmldata);
			
			
			free(filename);
			filename = NULL;
		}
		FL_XML_BODY_END(xmldata);

		closedir(dp);
		printf("xml doc:%s\n", xmldata->data);
		
		//composite the obex obejct
		OBEX_ObjectSetRsp(object, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
		hv.bs = (uint8_t *)xmldata->data;
		OBEX_ObjectAddHeader(handle, object, OBEX_HDR_BODY, hv, xmldata->size, 0);
		hv.bq4 = xmldata->size;
		OBEX_ObjectAddHeader(handle, object, OBEX_HDR_LENGTH, hv, sizeof(uint32_t), 0);
		FREE_RAWDATA_STREAM(xmldata);

	}
	else if (name)
	{
		printf("%s() Got a request for %s\n", __FUNCTION__, name);
		
		buf = easy_readfile(name, &file_size);
		if(buf == NULL) {
			printf("Can't find file %s\n", name);
			OBEX_ObjectSetRsp(object, OBEX_RSP_NOT_FOUND, OBEX_RSP_NOT_FOUND);
			return;
		}

		OBEX_ObjectSetRsp(object, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
		hv.bs = buf;
		OBEX_ObjectAddHeader(handle, object, OBEX_HDR_BODY, hv, file_size, 0);
		hv.bq4 = file_size;
		OBEX_ObjectAddHeader(handle, object, OBEX_HDR_LENGTH, hv, sizeof(uint32_t), 0);
	}
	else
	{
		printf("%s() Got a GET without a name-header!\n", __FUNCTION__);
		OBEX_ObjectSetRsp(object, OBEX_RSP_NOT_FOUND, OBEX_RSP_NOT_FOUND);
		return;
	}

	if (NULL != buf)
	{
		free(buf);
	}

out:
	if (NULL != name)
	{
		free(name);
	}

	if (NULL != type)
	{
		free(type);
	}	
	return;
}

/*
 * Function safe_save_file ()
 *
 *    First remove path and add "/tmp/". Then save.
 *
 */
static int safe_save_file(char *name, const uint8_t *buf, int len)
{
	char *s = NULL;
	char filename[255] = {0,};
	int fd;
	int actual;
	static int save_actual = 0;
	//printf("Filename = %s\n", name);
	//unsigned short * p = name;
	//printf("0x%x 0x%x 0x%x 0x%x\n",p[0],p[1],p[2],p[3]);
	s = strrchr(name, '/');
	if (s == NULL)
		s = name;
	else
		s++;

	strncat(filename, s, 250);
	fd = open(filename, O_RDWR |O_CREAT |O_APPEND, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

	if ( fd < 0) {
		perror( "filename");
		return -1;
	}
	actual = write(fd, buf, len);
	fsync(fd);
	usleep(50);
	save_actual += actual;
	close(fd);

	//printf( "Wrote %s (%d bytes)\n", filename, save_actual);
	return actual;
}

/*
  * Function put_done()
  *    Parse what we got from a PUT
  */
static int put_done(obex_t *handle, obex_object_t *object, int final)
{
	obex_headerdata_t hv;
	uint8_t hi;
	uint32_t hlen;
	const uint8_t *body = NULL;
	int body_len = 0;
	static char *name = NULL;
	char fullname[WORK_PATH_MAX];
	struct stat statbuf;
	//char *namebuf = NULL;
	int ret;
	while(OBEX_ObjectGetNextHeader(handle, object, &hi, &hv, &hlen))	{
		switch(hi)	{
		case OBEX_HDR_BODY:
			body = hv.bs;
			body_len = hlen;
			//fprintf(stderr, "hv.bs=%p, hlen=%d\n", hv.bs, hlen);
			bt_wt = 1;
			break;
		case OBEX_HDR_NAME:
			if (NULL != name)
			{
				free(name);
			}
			if( (name = malloc(hlen / 2)))	{
				//unsigned short * p = name;
				//printf("0x%x 0x%x 0x%x 0x%x\n",p[0],p[1],p[2],p[3]);
				OBEX_UnicodeToChar((uint8_t *)name, hv.bs, hlen);
				//fprintf(stderr, "put file name: %s\n", name);
			}
			break;

		case OBEX_HDR_LENGTH:
			//printf("HEADER_LENGTH = %d\n", hv.bq4);
			break;

		case HDR_CREATOR:
			//printf("CREATORID = %#x\n", hv.bq4);
			break;
		
		default:
			//printf("%s () Skipped header %02x\n", __FUNCTION__ , hi);
			break;
		}
	}
	if(!body)	{
		//printf("Got a PUT without a body\n");
		OBEX_ObjectSetRsp(object, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
	}
	if(!name)	{
		name = strdup("OBEX_PUT_Unknown_object");
		//printf("Got a PUT without a name. Setting name to %s\n", name);

	}

	if (body)
	{
		safe_save_file(name, body, body_len);	
		//usleep(50);
	}
	if(final && !body) {
		strcpy(fullname, CUR_DIR);
		strcat(fullname, name);
		if (!stat(fullname, &statbuf)) {
			perror("stat failed");
		}
		if (S_ISDIR(statbuf.st_mode)) {
			printf("Removing dir %s\n", name);
			rmdir(fullname);
		} else {
			printf("Deleting file %s\n", name);
			unlink(fullname);
		}
	}
	if (final)
	{
		free(name);
		name = NULL;
		printf( "===object receive done===\n");
		ret = 3;
	}
	else{  //bq:this putting........
		//fprintf(stderr,"<<<<<<<<<putting...........\n");
		ret = 2;
	}
	return ret;
}


/*
 * Function server_indication()
 *
 * Called when a request is about to come or has come.
 *
 */
static void server_request(obex_t *handle, obex_object_t *object, int UNUSED(event), int cmd)
{

	switch(cmd)	{
	case OBEX_CMD_SETPATH:
		printf("Received SETPATH command\n");
		OBEX_ObjectSetRsp(object, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
		set_server_path(handle, object);
		break;
	case OBEX_CMD_GET:
		/* A Get always fits one package */
		get_server(handle, object);
		break;
	case OBEX_CMD_PUT:
		printf("Received PUT command\n");
		OBEX_ObjectSetRsp(object, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
		bt_status = put_done(handle, object, 1);
		break;
	case OBEX_CMD_CONNECT:
//		OBEX_ObjectSetRsp(object, OBEX_RSP_SUCCESS, OBEX_RSP_SUCCESS);
		bt_status = connect_server(handle, object);
		connection_id++;
		break;
	case OBEX_CMD_DISCONNECT:
		OBEX_ObjectSetRsp(object, OBEX_RSP_SUCCESS, OBEX_RSP_SUCCESS);
		break;
	default:
		printf("%s () Denied %02x request\n", __FUNCTION__, cmd);
		OBEX_ObjectSetRsp(object, OBEX_RSP_NOT_IMPLEMENTED, OBEX_RSP_NOT_IMPLEMENTED);
		break;
	}
	return;
}


static void obex_event(obex_t *handle, obex_object_t *obj, int mode, int event, int obex_cmd, int obex_rsp)
{
	char progress[] = "\\|/-";
	static unsigned int i = 0;
	switch (event) {
	case OBEX_EV_STREAMAVAIL:
       	printf("Time to read some data from stream\n");
        break;

	case OBEX_EV_LINKERR:{
        	finished = 1;
        	obexftpd_reset = 1;
       		 success = FALSE;
		//bt_status = 5;
		fprintf(stderr, "failed: %d\n", obex_cmd);
		break;
	}
    	case OBEX_EV_REQ:
        	printf("Incoming request %02x\n", obex_cmd);
		/* Comes when a server-request has been received. */
		server_request(handle, obj, event, obex_cmd);
		break;
		
	case OBEX_EV_REQHINT:
        /* An incoming request is about to come. Accept it! */
		switch(obex_cmd) {
		case OBEX_CMD_PUT:
		case OBEX_CMD_CONNECT:
		case OBEX_CMD_DISCONNECT:
			OBEX_ObjectSetRsp(obj, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
			break;
		default:
			OBEX_ObjectSetRsp(obj, OBEX_RSP_NOT_IMPLEMENTED, OBEX_RSP_NOT_IMPLEMENTED);
			break;
		}
		break;

	case OBEX_EV_REQCHECK:
		/* e.g. mode=01, obex_cmd=03, obex_rsp=00 */
            	printf("%s() OBEX_EV_REQCHECK: mode=%02x, obex_cmd=%02x, obex_rsp=%02x\n", __func__, 
				mode, obex_cmd, obex_rsp);
		OBEX_ObjectSetRsp(obj, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
		break;
	case OBEX_EV_REQDONE:
	        //finished = TRUE;
        	if(obex_rsp == OBEX_RSP_SUCCESS)
	        	success = TRUE;
        	else {
	            success = FALSE;
        	    printf("%s() OBEX_EV_REQDONE: obex_rsp=%02x\n", __func__, obex_rsp);
	        }
		break;

	case OBEX_EV_PROGRESS:
		//fprintf(stderr, "%c%c", 0x08, progress[i++]);
		fflush(stdout);
		if (i >= strlen(progress))
			i = 0;
			
		switch(obex_cmd) {
		case OBEX_CMD_PUT:
			//fprintf(stderr, "obex_ev_progress: obex_cmd_put\n");
			bt_status = put_done(handle, obj, 0);
			break;
		default:
			break;
		}
		break;
	case OBEX_EV_ABORT:{
		/* Request was aborted */
		//bt_status = 5;
            	printf("%s() OBEX_EV_ABORT: mode=%02x, obex_cmd=%02x, obex_rsp=%02x\n", __func__, 
				mode, obex_cmd, obex_rsp);
		obexftpd_reset = 1;
		break;
	}
	case OBEX_EV_STREAMEMPTY:
		//(void) cli_fillstream(cli, object);
		break;

	case OBEX_EV_UNEXPECTED:
		/* Unexpected data, not fatal */
		break;
    default:
         printf("%s() Unhandled event %d\n", __func__, event);
         break;

	}
}

/*
  *get the hcid dbusdeamon process pid
  */
static int get_hcid()
{
	char callbuf[256]={0};
	char buf[128] = {0};
	int rtn;
	FILE *fp=NULL;
	char *str_info = NULL;
	char *str_find =NULL;
	int hcid_pid = 0 ;
	sprintf(callbuf,"ps");
	btdbg_info("the call is %s",callbuf);
	fp = popen(callbuf, "r");
       if(NULL == fp)
      {
		fprintf(stderr,"popen error:%s\n",strerror(errno));
        	return -1;
      }
	
	int i = 0;
	while(fgets(buf, sizeof(buf), fp) != NULL)
    	{
    		str_info = buf;
		int ret;
		while(*str_info != '\n'){
			if(!isspace(*str_info)){
	    			ret = strncmp(str_info,"hcid",4);
				if(ret == 0){
					if(!isspace(*str_info)){
						str_find = buf;
						hcid_pid = atoi(str_find);
						fprintf(stderr,"hcid_pid = %d\n",hcid_pid);
						break;
					}
				}
			}
			str_info++;
		}
		if(hcid_pid)
			break;
		i++;
	}
	 if(pclose(fp) < 0)
	 	fprintf(stderr,"close failed\n");
	return hcid_pid;
}

/*
  *function:kill the hcid process and dbus-damemon ,
  *because we need one hcid and one dbus-daemon process
  */
static void bt_process()
{
	system("killall hcid");
	system("killall dbus-daemon");
}

/*
  *function:process  cannot bind sys_socket problem
  */
static void bt_dbus_syssocket()
{
#if 0
	system("rm -rf  /usr/local/var/run/dbus");
	system("mkfs.vfat  /dbus.img -v");
	printf("the mode 1\n");
	usleep(1000);
	system("umount  /usr/local/var/run");
	usleep(1000);
	system("mount  /dbus.img  /usr/local/var/run");
	usleep(1000);
	system("mkdir -m 777  /usr/local/var/run/dbus");
	usleep(1000);
	//system("mkdir -m 777 dbus");
	usleep(1000);
#endif
}

static int start_btopushserver()
{

	char callbuf[128]={0};
	int ret = 0;
	printf("start_btopushserver\n");
	sprintf(callbuf,"%s%s/%s","sh ",AM_CASE_SC_DIR,"start_opushserver.sh");
	btdbg_info("the call is %s",callbuf);
	ret = system(callbuf);
	usleep(1000);
	return 0;
}

/*reset the dbus-daemon env*/
static int bt_dbus_env()
{
	char *dbus_env_name = "DBUS_SYSTEM_BUS_ADDRESS";
	char *dbus_env="unix:path=/tmp/system_bus_socket";
	if(setenv(dbus_env_name,dbus_env,1) < 0){
		fprintf(stderr,"setenv failed\n");
		return -1;
	}
	printf("env = %s\n",getenv(dbus_env_name));
	return 0;
}

/*
  *becauce the bluetooth device have two states(iscan,pscan)
  *      hciconfig hci0 noscan(close piscan)
  */
 static int bt_pisan_state(int num)
{
	int ret;
	if(num==0){
		ret = system("hciconfig hci0 noscan");
		if(ret!=0)
			goto _err;
		usleep(500);
		ret = system("hciconfig hci0 pscan");
		if(ret!=0)
			goto _err;
	}

	if(num==1){
		ret = system("hciconfig hci0 piscan");
		if(ret!=0)
			goto _err;	
	}
_err:
	if(ret!=0){
		ret = -1;
	}
	return ret;
		
}
static void start_server(int transport)
{
	int use_sdp = 0;
	obex_t *handle = NULL;
	int hcid_pid;
	struct sockaddr_in saddr;
	bt_dbus_env();
	printf("debug 0\n");
sdp_server:
	start_btopushserver();
     if (0 > obexftp_sdp_register_ftp(channel))
       	{
       		fprintf(stderr, "register to SDP Server failed.\n");
		bt_process();
		goto sdp_server;
       	}
       	else
       	{
       		use_sdp = 1;
       	}
	
reset:
	handle = OBEX_Init(transport, obex_event, 0);
	if (NULL == handle) {
       		perror("failed to init obex.");
       		exit(-1);
	}	

	if (0 > BtOBEX_ServerRegister(handle, NULL, channel)) {
       			perror("failed to register bluetooth server");
	       		exit(-1);
	}

	printf("Waiting for connection...\n");

	(void) OBEX_ServerAccept(handle, obex_event, NULL);

	while (!finished) {
		OBEX_HandleInput(handle, 1);
	}

	OBEX_Cleanup(handle);
	sleep(1); /* throttle */

	if (obexftpd_reset)
	{
		fprintf(stderr, "obexftpd reset\n");
		obexftpd_reset = 0;
		finished = 0;
		success = 0;
		goto reset;
	}
	
	if (use_sdp)
	{
		fprintf(stderr, "sdp unregister\n");
		if (0 > obexftp_sdp_unregister_ftp())
		{
       			fprintf(stderr, "unregister from SDP Server failed.\n");
		}
	}

}


/*the status of bluetooth status*/
static int bt_message(int bt_status)
{
	int ret;
	switch(bt_status){
	case 0:
		ret = BT_CONN;
		break;
	case 1:
		ret = BT_DISCON;
		break;
	case 2:
		ret = BT_REVINGS;
		break;
	case 3:
		ret = BT_REVDONES;
		break;
	case 4:
		ret = BT_SNDS;
		break;
	case 5:
		ret = BT_ADNORMAL;
		break;
	default:
		ret = BT_UNKOWN;
		break;
	}
	return ret;
}


/*stop bt device opush server*/
static int stop_btopushserver(void * handle)
{
	char callbuf[128]={0};
	int ret = 0;
	SWFEXT_FUNC_BEGIN(handle);
	bt_process();
	
	if(bt_id){
		pthread_cancel(bt_id);
		pthread_join(bt_id,NULL);
	}

	ret = system("hciconfig hci0 down");
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}

/*the as layer get the statue*/
static int get_btopush_status(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(bt_status);
	SWFEXT_FUNC_END();
	
}



/*the mainloop of receive object*/
static void bt_receive_daemon()
{
	start_server(OBEX_TRANS_BLUETOOTH);
}


static int bt_receive(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	system("sh /bluetooth_init.sh");
	chdir("/mnt/udisk/");
	printf("----start receive event----- \n");
    	usleep(100);
	system("hciconfig hci0 up");

	
	int err = pthread_create(&bt_id,NULL,bt_receive_daemon,NULL);
	if(err != 0)
	{
		printf("can't create thread: %s\n",strerror(err));
		return -1;
	}
	
	Swfext_PutNumber(err);
	SWFEXT_FUNC_END();
}

int emptyfunc(void*handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();
}

#else

static int stop_btopushserver(void * handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	printf("[%s %d]Please Open Bluetooth Macro-Definition Firstly!!\n",__FILE__,__LINE__);
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();
}

static int get_btopush_status(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("[%s %d]Please Open Bluetooth Macro-Definition Firstly!!\n",__FILE__,__LINE__);
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();
	
}

static int bt_receive(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("[%s %d]Please Open Bluetooth Macro-Definition Firstly!!\n",__FILE__,__LINE__);
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();
}

int emptyfunc(void*handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	printf("[%s %d]Please Open Bluetooth Macro-Definition Firstly!!\n",__FILE__,__LINE__);
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();
}


#endif

int swfext_bt_register(void)
{
	//SWFEXT_REGISTER("bt_StartServer",start_btopushserver);
	SWFEXT_REGISTER("bt_StartServer",emptyfunc);
	SWFEXT_REGISTER("bt_StopServer", stop_btopushserver);
	SWFEXT_REGISTER("bt_Rec", bt_receive);
	SWFEXT_REGISTER("bt_Status", get_btopush_status);
	return 0;
}

