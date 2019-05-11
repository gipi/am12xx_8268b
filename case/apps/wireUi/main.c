#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#include "wire_config.h"
#include "wire_hotplug.h"
#include "wire_key.h"
#include "wire_log.h"
#include "wire_ota.h"

#define LOCKDOWN_PATH	"/mnt/vram/ezcast/lockdown"

static int isLockdownPlistStr(const char *str)
{
	char *plistP = NULL;
	char *dictP = NULL;

	const char *endStr1 = NULL;
	const char *endStr2 = NULL;
	if(!str)
		return 0;

	if(strcasestr(str, (const char *)"<?xml") == NULL)
		return 0;

	plistP = strcasestr(str, (const char *)"<plist");
	if(!plistP)
		return 0;

	dictP = strcasestr(str, (char *)"<dict>");
	if(!dictP)
		return 0;

	if(plistP < dictP)
	{
		endStr1 = "</dict>";
		endStr2 = "</plist>";
	}
	else
	{
		endStr1 = "</plist>";
		endStr2 = "</dict>";
	}

	char *p = strcasestr(str, endStr1);
	if(!p)
		return 0;

	if(strcasestr(p, endStr2) == NULL)
		return 0;

	return 1;
}

static int isPlistFile(const char *path)
{
	int res = 0;
	struct stat statInfo;
	char *buf = NULL;
	int fd = -1;
	int ret = -1;
	
	if(!path)
	{
		EZCASTWARN("file path is NULL!!\n");
		goto __END__;
	}
	fd = open(path, O_RDONLY);
	if(fd < 0)
	{
		EZCASTWARN("file open fail![%s]<%s>\n", path, strerror(errno));
		goto __END__;
	}
	ret = fstat(fd, &statInfo);
	if(ret)
	{
		EZCASTWARN("fstat fail![%s]<%s>\n", path, strerror(errno));
		goto __END__;
	}
	int fSize = statInfo.st_size;
	if(fSize <= 0)
	{
		EZCASTWARN("file size error!![(%d)%s]\n", fSize, path);
		goto __END__;
	}

	buf = (char *)malloc(fSize + 1);
	if(!buf)
	{
		EZCASTWARN("malloc fail![%s]<%s>\n", path, strerror(errno));
		res = -1;
		goto __END__;
	}

	char *p = buf;
	int needRead = fSize;
	do{
		ret = read(fd, p, needRead);
		if(ret < 0)
		{
			EZCASTWARN("read fail![%s]<%s>\n", path, strerror(errno));
			goto __END__;
		}
		needRead -= ret;
		p += ret;
	}while(needRead > 0);
	buf[fSize] = '\0';
	
	res = isLockdownPlistStr(buf);

__END__:
	if(fd >= 0)
	{
		close(fd);
		fd = -1;
	}
	
	if(buf)
		free(buf);
	buf = NULL;
	
	return res;
}

static int isPlistFileName(const char *name)
{
	if(name)
	{
		char *p = strrchr(name, '.');
		if(p)
		{
			p++;
			if((*p != '\0') && (strcasecmp(p, "plist") == 0))
				return 1;
		}
	}

	return 0;
}

static void checkLockdownFile()
{
	DIR *dp = NULL;
	
	dp = opendir(LOCKDOWN_PATH);
	if(dp == NULL)
	{
		EZCASTWARN("Open directory fail!!\n");
		goto __END__;
	}

	struct dirent *entry = NULL;
	while((entry = readdir(dp)) != NULL)
	{
		if(isPlistFileName(entry->d_name))
		{
			char path[128];
			snprintf(path, sizeof(path), "%s/%s", LOCKDOWN_PATH, entry->d_name);
			if(!access(path, F_OK) && !isPlistFile(path))
			{
				EZCASTWARN("It's not lockdown file!!![%s]\n", path);
				unlink(path);
			}
		}
	}

__END__:
	if(dp)
	{
		closedir(dp);
		dp = NULL;
	}
}

int main(int argc, char *argv[])
{
	int rpipe;
	int key_pipe = 0;
	int parent_msg_id;
	int ret,maxfd = 0;
	fd_set rfds,wfds,efds;
	struct timeval tv;
	if(argc > 1){
		if(argc != 4){
			WLOGE("error parameters\n");
			return -1;
		}
		parent_msg_id = (int)strtol(argv[1],NULL,10);
		rpipe = (int)strtol(argv[2],NULL,10);
		key_pipe = (int)strtol(argv[3],NULL,10);
	}
	else{
		WLOGE("fui arg err\n");
		return -1;
	}
	
	if(key_pipe > rpipe)
		maxfd = key_pipe;
	else
		maxfd = rpipe;
	checkLockdownFile();
	wire_set_context();
	ezwireUiInit();
	ezwireInit();
	#if (EZWIRE_TYPE==MIRAPLUG ||MODULE_CONFIG_ADB_MIRROR_ONLY==1)//iOS & Android with8M snor
	sys_volume_init();
       DocmdSocketUnix();
	#endif
	productInit();
	#if (EZWIRE_TYPE==MIRAPLUG ||MODULE_CONFIG_ADB_MIRROR_ONLY==1)
	read_andorid_table_file();
	#endif
	#if (EZWIRE_TYPE==MIRAPLUG)
	ezcast_bitmask_init();
	#endif
	#if (EZWIRE_TYPE==MIRALINE)
	ezFuiRemoteCallbackSet(otaEntry);
	#endif
	wire_hotplug_handle_thread();
	ret = initLibUsbHandle();
	if(ret < 0){
		WLOGE("initLibUsbHandle err\n");
		return -1;
	}

	while(1){

		FD_ZERO(&rfds);
		FD_SET(rpipe, &rfds);
		FD_SET(key_pipe, &rfds);
		struct timeval timeout;
		
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		ret = select(maxfd+1, &rfds, NULL, NULL, &timeout);
		if(ret == 0){
			;//timeout
		}else if(ret > 0){
			if(is_wire_usb_event(rpipe, &rfds)){
				wire_usb_handler(rpipe);
			}
			if(is_wire_key_event(key_pipe,&rfds)){
				wire_key_handler(key_pipe);
			}
		}
#if !defined(MODULE_CONFIG_AD_UI_ENABLE) || MODULE_CONFIG_AD_UI_ENABLE==0
		audioSineCheck();
#endif
	}

	ezwireUiRelease();

	deinitLibUsbHandle();
	wire_relesae_context();
	return 0;
}
