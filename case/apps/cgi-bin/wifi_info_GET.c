#include <stdio.h>
#include <stdlib.h>
#include "cgic.h"
#include "am_cgi.h"
#include "am_socket.h"
#ifndef __ACT_MODULE_CONFIG_H__ //For LinuxTool bug!! (i.e.; #if !define xxx)
#define __ACT_MODULE_CONFIG_H__
#include "../../../scripts/mconfig.h"
#endif
#include "../../include/ezcast_public.h"

int cgiMain(int argc, char *argv[])
{
	int ret;
	char cmd_string[512]={0};
	char callbuf[1000]={0};
	int probox_enable=-1;
    cgiHeaderCacheControl("no-cache, no-store, must-revalidate");
    cgiHeaderPragma("no-cache");
    cgiHeaderExpires(0);
	cgiHeaderContentType("text/html");

	cgiFormString("type", cmd_string, 512);
	if(strstr(cmd_string,"get_AM_TYPE")){
	#if (MODULE_CONFIG_EZCASTPRO_MODE==8075)
		probox_enable=1;
	#else
		probox_enable=0;
	#endif
		fprintf(cgiOut,"%d\n",probox_enable);		

	}
	
	else if(!strcmp(cmd_string,"EZMUSIC_ENABLE"))
		{
		
		#if MODULE_CONFIG_EZCAST_ENABLE
			#if MODULE_CONFIG_EZCASTPRO_MODE 
				#if (MODULE_CONFIG_EZCASTPRO_MODE==8075)
					#if MODULE_CONFIG_LAN_ONLY
						fprintf(cgiOut,"%s","9");//pro box lan only
					#else
						fprintf(cgiOut,"%s","6");//pro box
					#endif
				#else
					#if MODULE_CONFIG_EZWILAN_ENABLE
						fprintf(cgiOut,"%s","5");//pro lan
					#else
						fprintf(cgiOut,"%s","4");//pro dongle
					#endif
				#endif
			#elif MODULE_CONFIG_EZWILAN_ENABLE
			fprintf(cgiOut,"%s","3");//ezcat lan
			#elif MODULE_CONFIG_EZMUSIC_ENABLE
			fprintf(cgiOut,"%s","1");
			#elif MODULE_CONFIG_EZWIRE_ENABLE
				#if (MODULE_CONFIG_EZWIRE_TYPE==1 || MODULE_CONFIG_EZWIRE_TYPE>=5)
					fprintf(cgiOut,"%s","8"); //not audio:  1:MiraWire with 8252B;  5:SNOR flash with 8252N; 6: MiraWire with CDC/EDM; 7: MiraWire with CDC/EDM at snor
				#else
					fprintf(cgiOut,"%s","7");//EZWIRE type, 0: EZWire with 8251/8251W; 2:MiraWireDuo with 8252C; 3: MiraWirePlus with 8252B/8252W; 4: EZDocking with 8251W;
				#endif
			#else
			fprintf(cgiOut,"%s","2");
			#endif
		#endif
		
	}
	else if(!strcmp(cmd_string,"OTA_SERVER_ENABLE"))
		{
	#if (MODULE_CONFIG_EZCASTPRO_MODE==8075)
		#if OTA_SERVER_SET_ENABLE
			fprintf(cgiOut,"%s","1");
		#else
			fprintf(cgiOut,"%s","0");
		#endif
	#else
	
		#if OTA_SERVER_SET_ENABLE
			fprintf(cgiOut,"%s","1");
		#else
			fprintf(cgiOut,"%s","0");
		#endif
	#endif
	}
	else if(!strcmp(cmd_string,"CONFIG_BUSINESS_CUSTOMER"))
		{
		#if MODULE_CONFIG_BUSINESS_CUSTOMER
			fprintf(cgiOut,"%s","1");
		#else
			fprintf(cgiOut,"%s","0");
		#endif
	}
	else if(!strcmp(cmd_string,"ROUTER_ONLY_ENABLE"))
		{
		#if ROUTER_ONLY_ENABLE
			fprintf(cgiOut,"%s","1");
		#else
			fprintf(cgiOut,"%s","0");
		#endif
	}
	/*
	else if(!strcmp(cmd_string,"host_val"))
		{
		
		FILE *fp = NULL;
		fp=fopen("/mnt/vram/ezcast/ezcast.conf","r");
		if(fp==NULL)
			{
			perror("/mnt/vram/ezcast/ezcast.conf not exit !\n");
			fprintf(cgiOut,"%s","default");
			}		
		memset(callbuf,0,sizeof(callbuf));
		ret = fread(callbuf, 1, sizeof(callbuf), fp);	
		fclose(fp);
		if(ret > 0)
		{
			//strncpy(CGIsetting_priv.result_str,callbuf,strlen(callbuf));
			//sprintf(CGIsetting_priv.result_str,"%s",callbuf);
			fprintf(cgiOut,"%s",callbuf);
		}
		
	}
	*/
	else
	{
              char receiveline[1000]={0};
		ret=create_websetting_client(cmd_string);
		if(ret<0)
			return -1;
		websetting_client_write(cmd_string);
		ret=websetting_client_read(receiveline);
		fprintf(cgiOut,"%s",receiveline);
	}
	return ret;
}

