#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int get_rand_psk(char *rank_tail_p,int sum_tmp){
	unsigned long rank_interger = 0;
	rank_interger = (sum_tmp % (99999999-10000000+1))+ 10000000;
	//printf("rank_interger ========= %d\n",rank_interger);
	sprintf(rank_tail_p,"%d",rank_interger);	
	//printf("rank_tail_p ========= %s\n",rank_tail_p);
	return 0;
}

int main(int argc, char **argv){
	int i = 0;

	if(argc <= 1){
		printf("Please enter a SSID!!!\n");
		exit(1);
	}
	for(i=1; i<argc; i++){
		char psk[20];
		char *p = strrchr(argv[i], '-');
		if(p == NULL)
			p = argv[i];
		if(strlen(p) < 8){
			printf("%s : error\t", argv[i]);
			continue;
		}
		int len = strlen(p);
		int sum = (p[len-1]<<24) | (p[len-2]<<20) 
			| (p[len-3]<<16) | (p[len-4]<<12) 
			| (p[len-5]<<8) | (p[len-6]<<4) 
			| p[len-7] | p[len-8];
		get_rand_psk(psk, sum);
		printf("%s : %s\t", argv[i], psk);
	}
	printf("\n");
}
