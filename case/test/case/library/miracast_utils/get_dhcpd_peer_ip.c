#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
* This tool parse the udhcpd.lease file and get the ip address 
* for a given MAC address and write to output file.
*
* usage: get_dhcpd_peer_ip MAC lease_file output_file.
*/
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;

typedef uint32_t leasetime_t;
struct dyn_lease {
	/* "nip": IP in network order */
	/* Unix time when lease expires. Kept in memory in host order.
	 * When written to file, converted to network order
	 * and adjusted (current time subtracted) */
	leasetime_t expires;
	uint32_t lease_nip;
	/* We use lease_mac[6], since e.g. ARP probing uses
	 * only 6 first bytes anyway. We check received dhcp packets
	 * that their hlen == 6 and thus chaddr has only 6 significant bytes
	 * (dhcp packet has chaddr[16], not [6])
	 */
	uint8_t lease_mac[6];
	char hostname[20];
	uint8_t pad[2];
	/* total size is a multiply of 4 */
};

static int to_lower_case(unsigned char *str)
{
	int len;
	int i;
	
	if(str == NULL){
		return -1;
	}

	len = strlen(str);
	for(i=0;i<len;i++){
		if(*(str+i) >= 'A' && *(str+i) <= 'F'){
			*(str+i) = *(str+i) - 'A' + 'a';
		}
	}

	return 0;
}

int main(int argc, char*argv[])
{
	FILE *output;
	FILE *lease_fp;
	int file_len=0;
	char nouse[12];
	int i;

	unsigned char ip[16];
	unsigned char ip_addr_str[16]={0};
	unsigned char mac[16];
	unsigned char mac_addr_str[16]={0};

	if(argc != 4){
		printf("usage:\n");
		printf("    get_dhcpd_peer_ip MAC lease_file output_file\n");
		exit(1);
	}

	to_lower_case(argv[1]);

	printf("---->MAC:%s,lease:%s,output_file:%s\n",argv[1],argv[2],argv[3]);

	lease_fp = fopen(argv[2],"rb");
	if(lease_fp == NULL){
		printf("---->lease file open error\n");
		exit(1);
	}

	fseek(lease_fp, 0, SEEK_END);
	file_len = ftell(lease_fp);


	/**
	* begine of each least is int64 which means 8 bytes.
	*/
	if(file_len != (sizeof(struct dyn_lease)+8)){
		printf("---->lease file len error:%d,but expected:%d\n",file_len,sizeof(struct dyn_lease)+8);
		fclose(lease_fp);
		exit(1);
	}
	fseek(lease_fp, 0, SEEK_SET);

	/**
	* no use for the first 12 bytes.
	*/
	fread(nouse, 1,12, lease_fp);

	/**
	* ip address
	*/
	for(i=0;i<4;i++){
		fread(&ip[i], 1,1, lease_fp);
	}
	sprintf(ip_addr_str,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
	/**
	* mac address
	*/
	for(i=0;i<6;i++){
		fread(&mac[i], 1,1, lease_fp);
	}
	sprintf(mac_addr_str,"%02x:%02x:%02x:%02x:%02x:%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	fclose(lease_fp);
	to_lower_case(mac_addr_str);
	
	printf("---->get from lease file,ip address:%s,mac address:%s\n",ip_addr_str,mac_addr_str);

	if(strcmp(mac_addr_str,argv[1])!=0){
		printf("---->mac address not match\n");
		exit(1);
	}

	output = fopen(argv[3],"wt");
	if(output){
		printf("---->mac address match\n");
		fputs(ip_addr_str, output);
		fputs("\n", output);
		fclose(output);
	}

	return 0;
	
	
}

