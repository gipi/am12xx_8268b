#include "swf_ext.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <math.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/un.h>
#include "wifi_engine.h"

int mask_data[2][9]={{255,254,252,248,240,224,192,128,0},{8,7,6,5,4,3,2,1,0}};
int count_of_1=0;
char buf_arping[128] = {0};

int check_ip_arping(char *ip_address){
	char callbuf[50];
	FILE *fp=NULL;
	char * tmpbuf=NULL;
	int ret=-1;
	char *locate=NULL;
	char *locate_tmp;
	char *locate1;
	char reply_count[50];
	char buf[128] = {0};
	


	sprintf(callbuf,"arping -I eth0 -c 10 %s > /mnt/user1/lan/arping_result.txt",ip_address);
	printf("the call is %s\n",callbuf);	
	ret=system(callbuf);
	fprintf(stderr,"function:%s line:%d \n",__FUNCTION__,__LINE__);
	printf("ret == %d\n",ret);
	if(ret!=0)
    	printf("system call error:%s\n",strerror(errno));
 




	
	sprintf(callbuf,"arping -I eth0 -c 1 %s",ip_address);
	fp =popen(callbuf, "r");
	
    if(NULL == fp)
    {
    	fprintf(stderr,"function:%s line:%d \n",__FUNCTION__,__LINE__);

        printf("popen error:%s\n",strerror(errno));
        return -1;
    }
	
	//ret=fread(buf, 1, 128, fp);
	fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);
	
	//sleep(10);
	ret = pclose(fp);
/*	fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);

	fprintf(stderr,"function:%s line:%d strerror(errno):%s\n",__FUNCTION__,__LINE__,strerror(errno));
	printf("%s",buf);
	locate=strstr(buf,"Received ");
	printf("locate=====%s\n",locate);
	if(locate==NULL)
		return -1;
	
	locate_tmp=locate;
	locate_tmp+=strlen("Received ");
	
	printf("$$$$$$$$222\n");
	locate1=NULL;
	locate1=strstr(buf," reply");
	if(locate1==NULL)
		return -1;
	reply_count[0]=0;
	printf("strlen(locate_tmp)====%d\n",strlen(locate_tmp));
	
	printf("strlen(locate1)====%d\n",strlen(locate1));
	memcpy(reply_count,locate_tmp,strlen(locate_tmp)-strlen(locate1));
	printf("reply_count========%s\n",reply_count);
	
//	sleep(10);

	printf("----------check sleep over\n");
*/
	
	return 0;
}

int check_wifi_ip_arping(char *ip_address,int dongleType){
	char callbuf[50];
	FILE *fp=NULL;
	int ret=-1;
	char buf[128] = {0};
	int dongle_type = dongleType;

	printf("dongle_type=========check_wifi_ip_arping========%d\n",dongle_type);
	
	if(dongle_type == REALTEK_dongle){
		memset(callbuf,0,50);
		sprintf(callbuf,"arping -I wlan0 -c 15 %s ",ip_address);
		printf("the call is %s\n",callbuf); 
	}
	
	else if(dongle_type == RALINK_dongle){
		memset(callbuf,0,50);
		sprintf(callbuf,"arping -I ra0 -c 15 %s ",ip_address);
		printf("the call is %s\n",callbuf);		
	}
	fp = popen(callbuf,"r");
	while(!feof(fp))
	{	
		memset(buf,0,128);
		fgets(buf,128,fp);
	//	printf("buf ======================= %s \n",buf);
		if(strstr(buf,"Received 0 reply")!=NULL){
			ret =0 ;
			break;
		}
	}
		
	pclose(fp);
	
	return ret;
}

/*
int read_wifi_arping_reply(){
	
	FILE *fp = NULL;
	char buf[4096] ={0};
	//char callbuf[128];
	int ret=-1;
	char *locate1=NULL;
	if(access("/mnt/user1/lan/arping_wifi_result.txt",F_OK)==-1){
		printf("The file is not exsit!\n");
		return -1;
	}
	fp = fopen("/mnt/user1/lan/arping_wifi_result.txt","r");
		//fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);
	
	if(fp == NULL){
		printf("Cann't open the file!\n");
		return -1;		
	}
	ret=fread(buf, 1, 4096, fp);
	fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);
	ret=fclose(fp);
	fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);
	locate1=strstr(buf,"Received 0 reply");
	
	fprintf(stderr,"function:%s line:%d buf:%s\n",__FUNCTION__,__LINE__,buf);
	if(locate1==NULL){
		printf("locatel======Null");
		return -1;
		}
	else 
		{
		printf("locatel==========");
		return 0;
		}

}
*/
int read_arping_reply(){
	
	FILE *fp = NULL;
	char buf[4096] ={0};
	char callbuf[128];
	int ret=-1;
	char *locate1=NULL;
	if(access("/mnt/user1/lan/arping_result.txt",F_OK)==-1){
		printf("The file is not exsit!\n");
		return -1;
	}
	fp = fopen("/mnt/user1/lan/arping_result.txt","r");
		//fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);
	
		if(fp == NULL){
			printf("Cann't open the file!\n");
			return -1;		
		}
	ret=fread(buf, 1, 4096, fp);
	fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);
	ret=fclose(fp);
	fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);
	locate1=strstr(buf,"Received 0 reply");
	
	if(locate1==NULL)
		return -1;
	else 
		return 0;
		
}


int mask_format_judging(int mask_1,int mask_2,int mask_3,int mask_4){
	printf("mask_1==%d,mask_2==%d,mask_3==%d,mask_4==%d\n",mask_1,mask_2,mask_3,mask_4);
	if(mask_1>=224){
		if(mask_2==0&&mask_3==0&&mask_4==0){
			int i=0;
			printf("??????\n");
			for(i=0;i<9;i++){
				printf("mask_1===%d\n",mask_1);
				
				printf("mask_data[0][i]===%d\n",mask_data[0][i]);
				if(mask_1==mask_data[0][i]){
					printf("i======%d\n",i);
					printf("mask_data[1][i]======%d\n",mask_data[1][i]);
						
					return mask_data[1][i];
				}
			}
		}
		if(mask_1==255&&mask_3==0&&mask_4==0){
			int j=0;
			for(j=0;j<0;j++){
				if(mask_2==mask_data[0][j])
					return (mask_data[1][j]+8);
			}
			
		}
		if(mask_1==255&&mask_2==255&&mask_4==0){
			int k=0;
			for(k=0;k<9;k++){
				if(mask_3==mask_data[0][k])
					return (mask_data[1][k]+16);
			}
		}
		if(mask_1==255&&mask_2==255&&mask_3==255){
			int l=0;
			for(l=0;l<9;l++){
				if(mask_4==mask_data[0][l])
					return (mask_data[1][l]+24);
			}
		}
		
	}
	
	return -1;
	
}

int mask_match_mask_judging(unsigned short int ip_1,unsigned short int ip_2,unsigned short int ip_3,unsigned short int ip_4,unsigned short int mask_1,unsigned short int mask_2,unsigned short int mask_3,unsigned short int mask_4){
	count_of_1=mask_format_judging(mask_1,mask_2,mask_3,mask_4);
	printf("the count_of_1======%d\n",count_of_1);
	if(count_of_1==-1){
		printf("the format of mask is error!\n");
		return -1;

	}
	else{
		int count_of_0=32-count_of_1;
		int ip_and_mask_combin1=ip_1&mask_1;
		int ip_and_mask_combin2=ip_2&mask_2; 
		int ip_and_mask_combin3=ip_3&mask_3;
		int ip_and_mask_combin4=ip_4&mask_4; 
		if(ip_and_mask_combin1==ip_1&&ip_and_mask_combin2==ip_2&&ip_and_mask_combin3==ip_3&&ip_and_mask_combin4==ip_4){
			printf("the ip is the network ip\n");
			return -2;
		}
		int conut_of_byte_1=count_of_1/8;
		if(conut_of_byte_1==0){
			int conut_of_bit_1=8-count_of_1;
			int ip_and_mask_combin1=ip_1&mask_1;
			int temp_data=pow(2,conut_of_bit_1)-1+ip_and_mask_combin1;
			if(temp_data==ip_1&&mask_4==255&&mask_3==255&&mask_2==255){
				return -3;
			}
		}
		if(conut_of_byte_1==1){
			int conut_of_bit_1=16-count_of_1;
			
			int ip_and_mask_combin2=ip_2&mask_2; 
			int temp_data=pow(2,conut_of_bit_1)-1+ip_and_mask_combin2;
			if(temp_data==ip_2&&mask_4==255&&mask_3==255){
				return -3;
			}
		}
		if(conut_of_byte_1==2){
			int conut_of_bit_1=24-count_of_1;
			
			int ip_and_mask_combin3=ip_3&mask_3;
			int temp_data=ip_and_mask_combin3+pow(2,conut_of_bit_1)-1;
			if(temp_data==ip_3&&mask_4==255){
				return -3;
			}
		}
		if(conut_of_byte_1==3){
			int conut_of_bit_1=32-count_of_1;
			
			int ip_and_mask_combin4=ip_4&mask_4; 
			printf("ip_and_mask_combin444444======%d\n",ip_and_mask_combin4);
			int temp_data=ip_and_mask_combin4+pow(2,conut_of_bit_1)-1;
			printf("temp_data444444444444444=====%d\n",temp_data);
			if(temp_data==ip_4){
				return -3;
			}
		}
	}
	return 0;
}

int GatewayMatchJudging(unsigned short int ip_1,unsigned short int ip_2,unsigned short int ip_3,unsigned short int ip_4,unsigned short int gateway_1,unsigned short int gateway_2,unsigned short int gateway_3,unsigned short int gateway_4){
	printf(" ip is :%d.%d.%d.%d\n",ip_1,ip_2,ip_3,ip_4);
	printf("gw is :%d.%d.%d.%d\n",gateway_1,gateway_2,gateway_3,gateway_4);
	if(ip_1 == gateway_1 && ip_2 == gateway_2 && ip_3 == gateway_3 && gateway_4 > 0 && gateway_4 <=255)
		return 0;
	else{
		printf("gate way match error!!!\n");
		return -1;
		}
}

#define DEBUG(format,...) printf("File: "__FILE__", Line: %05d: "format"\n", __LINE__, ##__VA_ARGS__) 

typedef struct _ip_addr_ {
    unsigned char addr[4];
} IP_ADDR;
/* Mos: rewrite the check, only check does Gate way IP into same sub-net with IP */
int webset_ip_gw_subnet_check(char *ipx, char *gatewayx, char *maskx)
{
    IP_ADDR ip, gw_ip, mask;
    unsigned char end;
    int i,j;
    int ret = 0;
    int mask_bit = 0;

    /* Mos: Check IP and  gateway IP format */
    if (sscanf(ipx,"%d.%d.%d.%d%c",&ip.addr[0],&ip.addr[1],&ip.addr[2],&ip.addr[3],&end)!=4){
        DEBUG("IP format error!\n");
        return -1;
    }
    if (sscanf(gatewayx,"%d.%d.%d.%d%c",&gw_ip.addr[0],&gw_ip.addr[1],&gw_ip.addr[2],&gw_ip.addr[3],&end)!=4){
        DEBUG("Gateway IP format error!\n");
        return -1;
    }
    for (i=0; i<4; i++){
        if (i<=2){
            if ((ip.addr[i] < 0) || (ip.addr[i] > 255)){
                DEBUG("IP format error!\n");
                return -1;
            }
            if ((gw_ip.addr[i] < 0) || (gw_ip.addr[i] > 255)){
                DEBUG("Gateway IP format error!\n");
                return -1;
            }
        }
        else{
            if ((ip.addr[i] <= 0) || (ip.addr[i] >= 255)){
                DEBUG("IP format error!\n");
                return -1;
            }
            if ((gw_ip.addr[i] <= 0) || (gw_ip.addr[i] >= 255)){
                DEBUG("Gateway IP format error!\n");
                return -1;
            }
        }
    }

    /* Mos: Check subnet mask format and correct */
    if (sscanf(maskx,"%d.%d.%d.%d%c",&mask.addr[0],&mask.addr[1],&mask.addr[2],&mask.addr[3],&end)!=4){
        DEBUG("Subnet-net mask format error!\n");
        return -1;
    }
    else{
        for (i=0; i<4; i++){
            if ((mask.addr[i] < 0) || (mask.addr[i] > 255)){
                DEBUG("Subnet-net mask format error!\n");
                return -1;
            }
        }
        for (i=3; i>=0; i--){
            if ((mask.addr[i] == 0) && (mask_bit == 0)){
                continue;
            }
            else{
                for (j=0; j<8; j++){
                    if ( (mask_bit == 0) && (mask.addr[i] & (1<<j)) ){
                        mask_bit = 1;
                    }
                    if ( (mask_bit == 1) && ((mask.addr[i] & (1<<j)) == 0)){
                        DEBUG("Subnet-net mask format error!\n");
                        return -1;
                    }
                }
            }
        }
    }

    /* Mos: Check does IP and Gateway IP into same subnet */
    for (i=0; i<4; i++){
        if ((ip.addr[i] & mask.addr[i]) != (gw_ip.addr[i] & mask.addr[i])){
            DEBUG("IP and Gateway IP does not in same sub-net!\n");
            return -1;
        }
    }

    return 0;
}

int webDnsCheck(const char *str)
{
	int i,a[4];
	char end;
	if(sscanf(str,"%d.%d.%d.%d%c",&a[0],&a[1],&a[2],&a[3],&end)!=4)
	{
		printf("dns format error\n");
		return -1;
	}
		
	for (i=0;i<4;i++)
	{
		printf("%d\n",a[i]);
		if(a[i]<0||a[i]>255)
			return -1;	
	}
	return 0;
}


