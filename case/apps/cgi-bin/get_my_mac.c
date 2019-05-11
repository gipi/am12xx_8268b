#include <stdio.h>
#include "cgic.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define ARP_PATH	"/proc/net/arp"
#define BUFF_LEN	2048

int cgiMain(int argc, char *argv[])
{
    char *ip_address;
	char buff[BUFF_LEN];
	char ipaddress[128];
	int hw_type;
	int flags;
	char hw_address[128];
	char mask[128];
	char device[128];
    cgiHeaderCacheControl("no-cache, no-store, must-revalidate");
    cgiHeaderPragma("no-cache");
    cgiHeaderExpires(0);
    cgiHeaderContentType("text/html");

	ip_address = getenv("REMOTE_ADDR");
	if(ip_address == NULL){
		//fprintf(cgiOut, "get ip address error!!\n");
		return -1;
	}
	//fprintf(cgiOut, "My ip address is %s\n", ip_address);
	FILE *fp = fopen(ARP_PATH, "r");
	if(!fp){
		//fprintf(cgiOut, "open arp file error!!!\n");
		return -1;
	}
	//fprintf(cgiOut, "arp_fd = %d\n", arp_fd);
	memset(buff, 0, 2048);
	//fprintf(cgiOut, "buff cleaned!!!\n");
#if 0
	buff_len = read(arp_fd, buff, BUFF_LEN);
	if(buff_len < 0){
		//fprintf(cgiOut, "read arp file error!!!\n");
		close(arp_fd);
		return -1;
	}
	//fprintf(cgiOut, "%s\n", buff);
	close(arp_fd);
	//fprintf(cgiOut, "arp_fd closed!!!\n");
	ip_p = strstr(buff, ip_address);
	if(ip_p == NULL){
		//fprintf(cgiOut, "Could not find the ip %s\n", ip_address);
		return 1;
	}
	//fprintf(cgiOut, "\n%s\n", ip_p);
	sscanf(ip_p, "%s%x%x%s%s%s", ipaddress, &hw_type, &flags, hw_address, mask, device);
	fprintf(cgiOut, "%s", hw_address);
#endif

	do {
		memset(buff, 0, sizeof(buff));
		memset(ipaddress, 0, sizeof(ipaddress));
		memset(hw_address, 0, sizeof(hw_address));
		if(fgets(buff, sizeof(buff), fp) == NULL)
			break;

		sscanf(buff, "%s%x%x%s%s%s", ipaddress, &hw_type, &flags, hw_address, mask, device);
		if(strcmp(ipaddress, ip_address) == 0)
		{
			fprintf(cgiOut, "%s", hw_address);
			break;
		}
	}while(1);
	
    return 0;
}
