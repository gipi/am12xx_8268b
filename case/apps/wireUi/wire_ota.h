#ifndef __WIRE_OTA_H
#define __WIRE_OTA_H

#define VERSION_CFG_PATH		"/etc/version.conf"
#define VERSION_PATH			"/etc/version.conf"
#define GETOTAURL_OTACONFFILE	"/tmp/ota_config.json"
#define GETOTAURL_OTACONF		GETOTAURL_OTACONFFILE

int otaEntry(char *url);


#endif
