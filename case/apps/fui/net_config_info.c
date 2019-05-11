#include "net_config_info.h"

#define IFCONFIG_HWADDR_LEN (7)

int getIfconfigInfo(char *ifFace,ifconfigCmd_s ifconfigCmd,void *resultInfo,int resultInfoLength){
	
	struct sockaddr_in *addr;
	struct ifreq ifr;
	int sockfd;
	int ret = -1;
	unsigned char hwaddrTemp[IFCONFIG_HWADDR_LEN] = { '\0' };

	
	if(ifFace == NULL){
		printf("[%s - %d] ifFace ==== NULL ,should return !\n",__FILE__,__LINE__);
		goto  ___getIfconfigInfo_end__;
	}
	if(resultInfo == NULL){
		printf("[%s - %d] resultInfo ==== NULL ,should return !\n",__FILE__,__LINE__);
		goto  ___getIfconfigInfo_end__;
	}
	
	
	
	memset(resultInfo,0,resultInfoLength);
	memset(hwaddrTemp,0,IFCONFIG_HWADDR_LEN);
	
	sockfd = socket(AF_INET,SOCK_DGRAM,0);

	if(sockfd < 0){
		perror("socekt error");
		goto  ___getIfconfigInfo_end__;
	}
	strncpy(ifr.ifr_name,ifFace,IFNAMSIZ-1);
	
	NET_CONFIG_INFO_DBG("ifconfigCmd ====== %d\n",ifconfigCmd);
	switch(ifconfigCmd){
		
		case IFCONFIG_IP:
			if(ioctl(sockfd,SIOCGIFADDR,&ifr) == -1){
				perror("ioctl error");
				goto  ___getIfconfigInfo_end__;
			}
			
			addr = (struct sockaddr_in *)&(ifr.ifr_addr);
			memcpy((char*)resultInfo, inet_ntoa(addr->sin_addr),resultInfoLength - 1);
			ret = 0;
			
			NET_CONFIG_INFO_DBG("ip ==== %s \n",resultInfo);
			break;
			
		case IFCONFIG_MASK:
			if(ioctl(sockfd,SIOCGIFNETMASK,&ifr) == -1){
				perror("ioctl error");
				goto  ___getIfconfigInfo_end__;
			}
			
			addr = (struct sockaddr_in *)&ifr.ifr_addr;
			memcpy((char*)resultInfo, inet_ntoa(addr->sin_addr),resultInfoLength - 1);
			
			NET_CONFIG_INFO_DBG("mask ==== %s \n",resultInfo);
			ret = 0;
			
			break;
			
		case IFCONFIG_BRDADDR:
			if(ioctl(sockfd,SIOCGIFBRDADDR,&ifr) == -1){
				perror("ioctl error");
				goto  ___getIfconfigInfo_end__;
				
			}
			
			addr = (struct sockaddr_in *)&ifr.ifr_addr;
			memcpy((char*)resultInfo, inet_ntoa(addr->sin_addr),resultInfoLength - 1);
			ret = 0;
			NET_CONFIG_INFO_DBG("brocast add ==== %s \n",resultInfo);

			break;
			
		case IFCONFIG_HWADDR:
			
			
			if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == -1)
			{
				perror("ioctl error");
				goto  ___getIfconfigInfo_end__;
			}
			addr = (struct sockaddr_in *)&ifr.ifr_addr;
			memset(hwaddrTemp,0,IFCONFIG_HWADDR_LEN);
			memcpy(hwaddrTemp, ifr.ifr_hwaddr.sa_data, sizeof(hwaddrTemp));
			
			snprintf(resultInfo,resultInfoLength - 1,"%02x:%02x:%02x:%02x:%02x:%02x", 
				hwaddrTemp[0], hwaddrTemp[1], hwaddrTemp[2],  hwaddrTemp[3], hwaddrTemp[4], hwaddrTemp[5]);
			
			ret = 0;
			
			NET_CONFIG_INFO_DBG("mac ==== %s \n",resultInfo);
			break;
			
		default:
			
			NET_CONFIG_INFO_DBG("ifconfigCmd error ! \n");
			break;
	}
	

___getIfconfigInfo_end__:
	
	if(sockfd >= 0){
		close(sockfd);
	}
	return ret;
	
}
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
int ezcastpro_config_readwrite(char* str,char* filepath)
{
	int fd=-1,ret=-1;
	char buf[NET_MANUAL_CONFIG_BUFFER_LENGTH]={0};
	printf("%s,\n input str==%s, filepath==%s\n",__func__,str,filepath);	
	fd=open(filepath,O_RDWR);
	if(fd<0){
		printf("open %s fail!!",filepath);
		return -1;
	}
	printf("str[0]==0x%x\n",str[0]);
	if(str[0]==0){//read
		ret=read(fd,str,NET_MANUAL_CONFIG_BUFFER_LENGTH);
		close(fd);
		if(ret<NET_MANUAL_CONFIG_MINIMUM_LENGTH){//min conf data length
			printf("read %s fail,%d!!",filepath,ret);
			ret=-1;
		}else{
			printf("get str(%d)==%s;\n",ret,str);
			ret=0;
		}
	}else{//write
		ret=read(fd,buf,NET_MANUAL_CONFIG_BUFFER_LENGTH);	
		printf("old buf(%d)==%s;\n",ret,buf);
		lseek(fd,0,SEEK_SET);
//		ret=write(fd,str,NET_MANUAL_CONFIG_BUFFER_LENGTH);
		ret=write(fd,str,strlen(str));		
		if(ret>=NET_MANUAL_CONFIG_MINIMUM_LENGTH){
			fsync(fd);
			memset(buf,0,NET_MANUAL_CONFIG_BUFFER_LENGTH);
			lseek(fd,0,SEEK_SET);			
			ret=read(fd,buf,NET_MANUAL_CONFIG_BUFFER_LENGTH);	
			printf("new buf(%d)==%s;\n",ret,buf);
			ret=0;
		}else{
			printf("write %s fail,%d!!",filepath,ret);	
			ret=-1;
		}
		close(fd);
	}
	printf("%s,%d\n",__func__,ret);
	return ret;
}
#endif
