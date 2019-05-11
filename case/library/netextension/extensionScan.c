/*
 *	Extension API for scanning BSS.
 *
 */
#include "iwlib.h"	
#include <math.h>
#include <sys/time.h>
#include <net/ethernet.h>
#include "extensionScan.h"


//#define DEBUG_E
//#define DEBUG

#ifdef DEBUG_E
#define DBP(fmt,args...) printf(fmt,##args)
#else
#define DBP(fmt,args...)
#endif

/*
 * Scan state and meta-information, used to decode events...
 */
typedef struct iwscan_state
{
	/* State */
 	int ap_num;		/* Access Point number 1->N */
 	int val_index;	/* Value in table 0->(N-1) */
} iwscan_state;

/*
 * Frequency channel mapping table
 */
typedef struct freq_channel
{
	unsigned short freq;		//Mhz
	unsigned short ch;
}freq_ch;

freq_ch freq_ch_table[]={
	{2412,1},		//2.4G band
	{2417,2},
	{2422,3},
	{2427,4},
	{2432,5},
	{2437,6},
	{2442,7},
	{2447,8},
	{2452,9},
	{2457,10},
	{2462,11},
	{2467,12},
	{2472,13},
	{2484,14},
	{5180,36},	//5G band1
	{5190,38},
	{5200,40},
	{5210,42},
	{5220,44},
	{5230,46},
	{5240,48},
	{5260,52},
	{5280,56},
	{5300,60},
	{5320,64},
	{5500,100},	//5G band2
	{5520,104},
	{5540,108},
	{5560,112},
	{5580,116},
	{5600,120},
	{5620,124},
	{5640,128},
	{5660,132},
	{5680,136},
	{5700,140},
	{5745,149},	//5G band3
	{5765,153},
	{5785,157},
	{5805,161},
	{5825,165},
};

int freq_to_channel(double freq){
	int table_size = sizeof(freq_ch_table)/sizeof(freq_ch);
	int i;
	for(i=0;i<table_size;i++){
		if(freq == freq_ch_table[i].freq)
			return (int)freq_ch_table[i].ch;
	}
	printf("><error> cannot find the matched channel!\n");
	return -1;
}

/*------------------------------------------------------------------*/
void ether_addr_ntop(const struct ether_addr *eth, char *buf){
	  sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
	 	 eth->ether_addr_octet[0], eth->ether_addr_octet[1],
	 	 eth->ether_addr_octet[2], eth->ether_addr_octet[3],
	 	 eth->ether_addr_octet[4], eth->ether_addr_octet[5]);
}
/*
 * Parse one element from the scanning results
 */
static inline void parse_scanning_token(struct iw_event *		event,	/* Extracted token */
			 struct iwscan_state *	state,
			 sc_info *info_tmp,		/* Temporary ap info*/
			 sc_info *ap_info,		/* ap_info must return to you*/
			 char *hidden_essid)
{
	/* Now, let's decode the needed event */
	switch(event->cmd){
		case SIOCGIWAP:
			{
				/* In this event again , I consider the next ap info parse begin. fix me?*/
				if(info_tmp->sc_essid[0] != '\0')					//find a matched ap ,save it
					memcpy(ap_info,info_tmp,sizeof(sc_info));		//just save the preview one
				memset(info_tmp,0,sizeof(sc_info));
				
				char	buffer[128];	//Temporary buffer 					
				ether_addr_ntop((const struct ether_addr *)&event->u.ap_addr.sa_data, buffer);
				strncpy(info_tmp->sc_addr,buffer,SCANNING_MAC_LEN);		//maybe buffer is too long ^^
				DBP("Cell %02d - Address: %s \n", state->ap_num,info_tmp->sc_addr,sizeof(buffer));
				state->ap_num++;
			}
			break;
		case SIOCGIWFREQ:
			{
				double freq;			// Frequency/channel 
				int channel = -1;		// Converted to channel 
				int i;
				freq = (double) event->u.freq.m * pow(10,event->u.freq.e);
				DBP("\t freq:%g\n",freq);
				if(freq < KILO)
					channel = freq;
				else{
					if(freq > GIGA){
						freq /= MEGA;		//Mhz
						channel = freq_to_channel(freq);	//Convert to channel if possible 
					}
					else 
						printf("><error> the freq is %g but not in wifi normal frequency!!\n");
						
				}
				info_tmp->channel = channel;
				DBP("\t channel:%d\n", channel);
			}
			break;
		case SIOCGIWESSID:
			{
				char essid[IW_ESSID_MAX_SIZE+1];
				memset(essid, '\0', sizeof(essid));
				if((event->u.essid.pointer) && (event->u.essid.length))
					memcpy(essid, event->u.essid.pointer, event->u.essid.length);
				if(event->u.essid.flags){
				/* Does it have an ESSID index ? */
					if((event->u.essid.flags & IW_ENCODE_INDEX) > 1)
						DBP("\t ESSID:\"%s\" [%d]\n", essid,(event->u.essid.flags & IW_ENCODE_INDEX));
					else{
						if(!strcmp(hidden_essid,essid))
							strcpy(info_tmp->sc_essid,essid);
						DBP("\t ESSID:\"%s\"\n", essid);
					}
				}
				else
					DBP("\t ESSID:off/any/hidden\n");
				
			}
			break;
		default:
			break;
		}	
}


/*------------------------------------------------------------------*/
/*
 * Perform a scanning on one device
 */
static int get_scanning_info(int skfd,char *ifname,char *essid,  sc_info *ap_info)
{
	struct iwreq		wrq;
	struct iw_scan_req	scanopt;		/* Options for 'set' */
	int scanflags = 0;				/* Flags for scan */
	unsigned char *buffer = NULL;		/* Results */
	int buflen = IW_SCAN_MAX_DATA; 	/* Min for compat WE<17 */
	struct iw_range range;
	int has_range;
	struct timeval	tv;				/* Select timeout */
	int timeout = 15000000;		/* 15s maybe too long ,but somtimes you must wait for so long.*/

	/* Get range stuff */
	has_range = (iw_get_range_info(skfd, ifname, &range) >= 0);

	/* Check if the interface could support scanning. */
	if((!has_range) || (range.we_version_compiled < 14)){
		fprintf(stderr, "%-8.16s	Interface doesn't support scanning.\n\n",ifname);
		return(-1);
	}

	/* Init timeout value -> 250ms between set and first get */
	tv.tv_sec = 0;
	tv.tv_usec = 250000;

	/* Clean up set args */
	memset(&scanopt, 0, sizeof(scanopt));

	/* Store the ESSID in the scan options */
	scanopt.essid_len = strlen(essid);
	memcpy(scanopt.essid, essid, scanopt.essid_len);
	/* Initialise BSSID as needed */
	if(scanopt.bssid.sa_family == 0){
		scanopt.bssid.sa_family = ARPHRD_ETHER;
		memset(scanopt.bssid.sa_data, 0xff, ETH_ALEN);
	}
	/* Scan only this ESSID */
	scanflags |= IW_SCAN_THIS_ESSID;

	/* Check if we have scan options */
	if(scanflags){
		wrq.u.data.pointer = (caddr_t) &scanopt;
		wrq.u.data.length = sizeof(scanopt);
		wrq.u.data.flags = scanflags;
	}
	else{
		wrq.u.data.pointer = NULL;
		wrq.u.data.flags = 0;
		wrq.u.data.length = 0;
	}

	/* Initiate Scanning */
	if(iw_set_ext(skfd, ifname, SIOCSIWSCAN, &wrq) < 0){
		if((errno != EPERM) || (scanflags != 0)){
			fprintf(stderr, "%-8.16s	Interface doesn't support scanning : %s\n\n",ifname, strerror(errno));
			return(-1);
		}

		/* If we don't have the permission to initiate the scan, we may
		   * still have permission to read left-over results.
		   * But, don't wait !!! */

		/* Not cool, it display for non wireless interfaces... */
		fprintf(stderr, "%-8.16s	(Could not trigger scanning, just reading left-over results)\n", ifname);

		tv.tv_usec = 0;
	}
	
	timeout -= tv.tv_usec;

	/* Forever */
	while(0){
		fd_set rfds;		/* File descriptors for select */
		int last_fd;		/* Last fd */
		int ret;

		/* We must re-generate rfds each time */
		FD_ZERO(&rfds);
		last_fd = -1;

		/* Wait until something happens */
		ret = select(last_fd + 1, &rfds, NULL, NULL, &tv);

		/* Check if there was an error */
		if(ret < 0){
			if(errno == EAGAIN || errno == EINTR)
				continue;
			fprintf(stderr, "Unhandled signal - exiting...\n");
			return -1;
		}

		/* Check if there was a timeout */
		if(ret == 0){
			unsigned char *	newbuf;

		realloc:
			/* (Re)allocate the buffer - realloc(NULL, len) == malloc(len) */
			newbuf = realloc(buffer, buflen);
			if(newbuf == NULL){
				if(buffer)
					free(buffer);
				fprintf(stderr, "%s: Allocation failed\n", __FUNCTION__);
				return -1;
			}
			buffer = newbuf;

		  	/* Try to read the results */
			wrq.u.data.pointer = buffer;
			wrq.u.data.flags = 0;
			wrq.u.data.length = buflen;
			if(iw_get_ext(skfd, ifname, SIOCGIWSCAN, &wrq) < 0){
				/* Check if buffer was too small (WE-17 only) */
				if((errno == E2BIG) && (range.we_version_compiled > 16)){

					/* Check if the driver gave us any hints. */
					if(wrq.u.data.length > buflen)
						buflen = wrq.u.data.length;
					else
						buflen *= 2;

					/* Try again */
					goto realloc;
				}

				/* Check if results not available yet */
				if(errno == EAGAIN)
				{
					/* Restart timer for only 100ms*/
					tv.tv_sec = 0;
					tv.tv_usec = 100000;
					timeout -= tv.tv_usec;
					if(timeout > 0)
						continue;	/* Try again later */
				}

				/* Bad error */
				free(buffer);
				fprintf(stderr, "%-8.16s	Failed to read scan data : %s\n\n",ifname, strerror(errno));
				return -2;
			}
		else
			/* We have the results, go to process them */
			break;
		}

	  /* In here, check if event and event type
	   * if scan event, read results. All errors bad & no reset timeout */
	}

	if(wrq.u.data.length){
		struct iw_event iwe;
		struct stream_descr	stream;
		struct iwscan_state	state = { .ap_num = 1, .val_index = 0 };
		int ret;
#ifdef DEBUG
		/* Debugging code. In theory useless, because it's debugged ;-) */
		int	i;
		printf("Scan result %d [%02X", wrq.u.data.length, buffer[0]);
		for(i = 1; i < wrq.u.data.length; i++)
			printf(":%02X", buffer[i]);
		printf("]\n");
#endif
		DBP("%-8.16s  Scan completed :\n", ifname);
		iw_init_event_stream(&stream, (char *) buffer, wrq.u.data.length);
		sc_info info_tmp;
		memset(&info_tmp,0,sizeof(sc_info));
		do{
			/* Extract an event and print it */
			ret = iw_extract_event_stream(&stream, &iwe,
						range.we_version_compiled);
			if(ret > 0)
				parse_scanning_token(&iwe, &state,&info_tmp,ap_info,essid);					
		}
		while(ret > 0);
		if(info_tmp.sc_essid[0] != '\0' ){
			memcpy(ap_info,&info_tmp,sizeof(sc_info));		//save the last one
			printf("<%s> ap_info:\n",__func__);
			printf("\t sc_addr is %s\n",ap_info->sc_addr);
			printf("\t sc_essid is %s\n",ap_info->sc_essid);
			printf("\t channel is %d\n",ap_info->channel);
		}
		if(ap_info->sc_essid[0] == '\0')
			printf("><> cannot find the ap info!!!\n");
	}
	else
		printf("%-8.16s  No scan results\n\n", ifname);

	free(buffer);

	return 0;
}

int extension_scanning(char *ifnane,char *essid, sc_info *ap_info){

	int skfd;			/* generic raw socket desc.	*/	

	if(ifnane == NULL){
		printf("need a interface!\n");
		return -1;
	}
	if(essid == NULL){
		printf("need a esssid.\n");
		return -1;
	}
	
	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0)
	{
		perror("socket");
		return -1;
	}

	get_scanning_info(skfd, ifnane, essid,ap_info);

 	/* Close the socket. */
	iw_sockets_close(skfd);

	return 0;
}


int main(int argc,char *argv[]){
	if(argc < 3){
		printf("><error> %s argv is less!\n");
		return -1;
	}
	sc_info ap_info;
	memset(&ap_info,0,sizeof(sc_info));
	printf("..............[%s %s %s]: test begin \n",argv[0],argv[1],argv[2]);
	extension_scanning(argv[1],argv[2],&ap_info);
	printf("..............test over!!!\n");
	if(ap_info.sc_essid[0]!='\0')
	{
		printf("ap_info:\n");
		printf("\t sc_addr is %s\n",ap_info.sc_addr);
		printf("\t sc_essid is %s\n",ap_info.sc_essid);
		printf("\t channel is %d\n",ap_info.channel);
	}
	return 1;
}
//just for test


