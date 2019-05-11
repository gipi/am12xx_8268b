#include "sys_resource.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "apps_vram.h"

#define SUBTILT_DEBUG

typedef struct laninfo_s
{
	short int group;				////属于哪一组
	int codepage; 			///code page 
	short int  idx;					///组内二级索引
	unsigned char exts[5];			///需要添加的后最名称
	
}laninfo_t;

typedef struct groupinfo_s
{
	short int groupidx;				///组索引
	short int idx_min;				///组内最小值(二级索引)
	short int idx_max;				///组内最大值(二级索引)
}groupinfo_t;

#define sp_info(fmt,arg...) //printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define sp_err(fmt,arg...) //printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

groupinfo_t groupinfo_array[]=
{
	{0 ,0,0},
	{1 ,0,0},
	{2 ,0,14},
	{3 ,0,0},
	{4 ,0,2},
	{5 ,0,8},
	{6 ,0,11},
	{7 ,0,65},
	{8 ,0,0},
	{9 ,0,2},
	{10,0,0},
	{11,0,16},
	{12,0,2},
	{13,0,0},
};


laninfo_t laninfo_array[]=
{
	{0,874,0,"-THA"},		//泰语                    					
	{1,932,0,"-JPN"},             //日语                    
	{2,936,0,"-HYE"},             //亚美尼亚语              
	{2,936,1,"-KAT"},             //格鲁吉亚语              
	{2,936,2,"-HIN"},             //印地语                  
	{2,936,3,"-PAN"},             //旁遮普语                
	{2,936,4,"-GUJ"},             //古吉拉特语              
	{2,936,5,"-TAM"},             //泰米尔语                
	{2,936,6,"-TEL"},             //泰卢固语                
	{2,936,7,"-KAN"},             //卡纳拉语                
	{2,936,8,"-MAR"},             //马拉地语                
	{2,936,9,"-SAN"},             //梵文                    
	{2,936,10,"-KNK"},            //孔卡尼语                
	{2,936,11,"-SYR"},            //叙利亚语                
	{2,936,12,"-DIV"},            //第维埃语                
	{2,936,13,"-CHS"},            //中文(中国)              
	{2,936,14,"-ZHI"},            //中文(新加坡)            
	{3,949,0,"-KOR"},             //朝鲜语                  
	{4,950,0,"-CHT"},             //中文(台湾)              
	{4,950,1,"-ZHH"},             //中文(香港特别行政区)    
	{4,950,2,"-ZHM"},             //中文(澳门特别行政区)    
	{5,1250,0,"-CSY"},            //捷克语                  
	{5,1250,1,"-HUN"},            //匈牙利语                
	{5,1250,2,"-PLK"},            //波兰语                  
	{5,1250,3,"-ROM"},            //罗马尼亚语              
	{5,1250,4,"-HRV"},            //克罗地亚语              
	{5,1250,5,"-SKY"},            //斯洛伐克语              
	{5,1250,6,"-SQI"},            //阿尔巴尼亚语            
	{5,1250,7,"-SLV"},            //斯洛文尼亚语            
	{5,1250,8,"-SRL"},            //塞尔维亚语(拉丁文)      
	{6,1251,0,"-BGR"},            //保加利亚语              
	{6,1251,1,"-RUS"},            //俄语                    
	{6,1251,2,"-UKR"},            //乌克兰语                
	{6,1251,3,"-BEL"},            //比利时语                
	{6,1251,4,"-MKI"},            //马其顿语(FYROM)         
	{6,1251,5,"-KKZ"},            //哈萨克语                
	{6,1251,6,"-KYR"},            //吉尔吉斯语(西里尔文)    
	{6,1251,7,"-TTT"},            //鞑靼语                  
	{6,1251,8,"-MON"},            //蒙古语(西里尔文)        
	{6,1251,9,"-AZE"},            //阿塞拜疆语(西里尔文)    
	{6,1251,10,"-UZB"},           //乌兹别克语(西里尔文)    
	{6,1251,11,"-SRB"},           //塞尔维亚语(西里尔文)    
	{7,1252,0 ,"-CAT"},           //加泰隆语                
	{7,1252,1 ,"-DAN"},           //丹麦语                  
	{7,1252,2 ,"-DEU"},           //德语(德国)              
	{7,1252,3 ,"-ENU"},           //英语(美国)              
	{7,1252,4 ,"-ESP"},           //西班牙语(传统)          
	{7,1252,5 ,"-FIN"},           //芬兰语                  
	{7,1252,6 ,"-FRA"},           //法语(法国)              
	{7,1252,7 ,"-ISL"},           //冰岛语                  
	{7,1252,8 ,"-ITA"},           //意大利语(意大利)        
	{7,1252,9 ,"-NLD"},           //荷兰语(荷兰)            
	{7,1252,10,"-NOR"},           //挪威语(伯克梅尔)        
	{7,1252,11,"-PTB"},           //葡萄牙语(巴西)          
	{7,1252,12,"-SVE"},           //瑞典语                  
	{7,1252,13,"-IND"},           //印度尼西亚语            
	{7,1252,14,"-EUQ"},           //巴士克语                
	{7,1252,15,"-AFK"},           //南非语                  
	{7,1252,16,"-FOS"},           //法罗语                  
	{7,1252,17,"-MSL"},           //马来语(马来西亚)        
	{7,1252,18,"-SWK"},           //斯瓦希里语              
	{7,1252,19,"-GLC"},           //加里西亚语              
	{7,1252,20,"-DES"},           //德语(瑞士)              
	{7,1252,21,"-ENG"},           //英语(英国)              
	{7,1252,22,"-ESM"},           //西班牙语(墨西哥)        
	{7,1252,23,"-FRB"},           //法语(比利时)            
	{7,1252,24,"-ITS"},           //意大利语(瑞士)          
	{7,1252,25,"-NLB"},           //荷兰语(比利时)          
	{7,1252,26,"-NON"},           //挪威语(尼诺斯克)        
	{7,1252,27,"-PTG"},           //葡萄牙语(葡萄牙)        
	{7,1252,28,"-SVF"},           //瑞典语(芬兰)            
	{7,1252,29,"-MSB"},           //马来语(文莱达鲁萨兰)    
	{7,1252,30,"-DEA"},           //德语(奥地利)            
	{7,1252,31,"-ENA"},           //英语(澳大利亚)          
	{7,1252,32,"-ESN"},           //西班牙语(国际)          
	{7,1252,33,"-FRC"},           //法语(加拿大)            
	{7,1252,34,"-DEL"},           //德语(卢森堡)            
	{7,1252,35,"-ENC"},           //英语(加拿大)            
	{7,1252,36,"-ESG"},           //西班牙语(危地马拉)      
	{7,1252,37,"-FRS"},           //法语(瑞士)              
	{7,1252,38,"-DEC"},           //德语(列支敦士登)        
	{7,1252,39,"-ENZ"},           //英语(新西兰)            
	{7,1252,40,"-ESC"},           //西班牙语(哥斯达黎加)    
	{7,1252,41,"-FRL"},           //法语(卢森堡)            
	{7,1252,42,"-ENI"},           //英语(爱尔兰)            
	{7,1252,43,"-ESA"},           //西班牙语(巴拿马)        
	{7,1252,44,"-FRM"},           //法语(摩纳哥)            
	{7,1252,45,"-ENS"},           //英语(南非)              
	{7,1252,46,"-ESD"},           //西班牙语(多米尼加共和国)
	{7,1252,47,"-ENJ"},           //英语(牙买加)            
	{7,1252,48,"-ESV"},           //西班牙语(委内瑞拉)      
	{7,1252,49,"-ENB"},           //英语(加勒比海)          
	{7,1252,50,"-ESO"},           //西班牙语(哥伦比亚)      
	{7,1252,51,"-ENL"},           //英语(伯利兹)            
	{7,1252,52,"-ESR"},           //西班牙语(秘鲁)          
	{7,1252,53,"-ENT"},           //英语(特立尼达)          
	{7,1252,54,"-ESS"},           //西班牙语(阿根廷)        
	{7,1252,55,"-ENW"},           //英语(津巴布韦)          
	{7,1252,56,"-ESF"},           //西班牙语(厄瓜多尔)      
	{7,1252,57,"-ENP"},           //英语(菲律宾)            
	{7,1252,58,"-ESL"},           //西班牙语(智利)          
	{7,1252,59,"-ESY"},           //西班牙语(乌拉圭)        
	{7,1252,60,"-ESZ"},           //西班牙语(巴拉圭)        
	{7,1252,61,"-ESB"},           //西班牙语(玻利维亚)      
	{7,1252,62,"-ESE"},           //西班牙语(萨尔瓦多)      
	{7,1252,63,"-ESH"},           //西班牙语(洪都拉斯)      
	{7,1252,64,"-ESI"},           //西班牙语(尼加拉瓜)      
	{7,1252,65,"-ESU"},	          //西班牙语(波多黎各(美))  
	{8,1253,0,"-ELL"},	          //希腊语                  
	{9,1254,0,"-TRK"},	          //土耳其语                
	{9,1254,1,"-AZE"},	          //阿塞拜疆语(拉丁文)      
	{9,1254,2,"-UZB"},            //乌兹别克语(拉丁文)      
	{10,1255,0,"-HEB"},	          //希伯来语                
	{11,1256,0 ,"-URD"},	        //乌都语                  
	{11,1256,1 ,"-FAR"},	        //法斯语                  
	{11,1256,2 ,"-ARI"},          //阿拉伯语(伊拉克)        
	{11,1256,3 ,"-ARE"},	        //阿拉伯语(埃及)          
	{11,1256,4 ,"-ARL"},	        //阿拉伯语(利比亚)        
	{11,1256,5 ,"-ARG"},	        //阿拉伯语(阿尔及利亚)    
	{11,1256,6 ,"-ARM"},          //阿拉伯语(摩洛哥)        
	{11,1256,7 ,"-ART"},	        //阿拉伯语(突尼斯)        
	{11,1256,8 ,"-ARO"},	        //阿拉伯语(阿曼)          
	{11,1256,9 ,"-ARY"},	        //阿拉伯语(也门)          
	{11,1256,10,"-ARS"},          //阿拉伯语(叙利亚)        
	{11,1256,11,"-ARJ"},	        //阿拉伯语(约旦)          
	{11,1256,12,"-ARB"},	        //阿拉伯语(黎巴嫩)        
	{11,1256,13,"-ARK"},	        //阿拉伯语(科威特)        
	{11,1256,14,"-ARU"},          //阿拉伯语(阿联酋)        
	{11,1256,15,"-ARH"},	        //阿拉伯语(巴林)          
	{11,1256,16,"-ARQ"},	        //阿拉伯语(卡塔尔)        
	{12,1257,0,"-ETI"},	          //爱沙尼亚语              
	{12,1257,1,"-LVI"},           //拉脱维亚语              
	{12,1257,2,"-LTH"},	          //立陶宛语                
	{13,1258,0,"-VIT"},	          //越南语                  
};

#define DIR_ADD		0x01
#define DIR_SUB		0x00

#define GROUP_ID_MAX 13
#define GROUP_ID_MIN  0

short int group_id=2;
short int subidx_id=13;///设置为中文

/*
*get the system language and match the lang array id for codepage 
*/

void get_system_lang(){
	int  lang = 0;
	struct sysconf_param sys_cfg_data;
	_get_env_data(&sys_cfg_data);
	lang = sys_cfg_data.local_language_id;
	switch(lang){		//the language order is match the system language which list in fui.xls
		case LAN_ENGLISH:
			group_id = 7;
			subidx_id = 21;
			break;
		case LAN_FRENCH:
			group_id = 7;
			subidx_id = 23;
			break;
		case LAN_GERMAN:
			group_id = 7;
			subidx_id = 2;
			break;
		case LAN_SPANISH:
			group_id = 7;
			subidx_id = 22;
			break;
		case LAN_PORTUGUESE:
			group_id = 7;
			subidx_id = 27;
			break;
		case LAN_CHS:
			group_id = 2;
			subidx_id = 13;
			break;
		case LAN_CHT:
			group_id = 4;
			subidx_id = 0;
			break;
		case LAN_ITALIAN:
			group_id = 7;
			subidx_id = 8;
			break;
		case LAN_NORWEGIAN:
			group_id = 7;
			subidx_id = 10;
			break;
		case LAN_SWEDISH:
			group_id = 7;
			subidx_id = 12;
			break;
		case LAN_DUTCH:
			group_id = 7;
			subidx_id = 9;
			break;
		case LAN_RUSSIAN:
			group_id = 6;
			subidx_id = 1;
			break;
		case LAN_POLISH:
			group_id = 5;
			subidx_id = 2;
			break;
		case LAN_FINNISH:
			group_id = 7;
			subidx_id = 5;
			break;
		case LAN_GREEK:
			group_id = 8;
			subidx_id = 0;
			break;
		case LAN_KOREAN:
			group_id = 3;
			subidx_id = 0;
			break;
		case LAN_HUNGARIAN:
			group_id = 5;
			subidx_id = 1;
			break;
		case LAN_CZECH:
			group_id = 5;
			subidx_id = 0;
			break;
		case LAN_ARABIC:
			group_id = 11;
			subidx_id = 16;
			break;
		case LAN_TURKISH:
			group_id = 9;
			subidx_id = 0;
			break;
		default : 
			break;		
	}
	//printf("%s,%d,lang is %d,\t group_id is %d,\t subidx_id is %d\n",__FUNCTION__,__LINE__,lang,group_id,subidx_id);
}

/*get the codepage*/
int subtitle_get_codepage(short int groupid,short int subidx,unsigned char * exts)
{
	int codepage=0;
	int i=0;
	int array_len = sizeof(laninfo_array)/sizeof(laninfo_t);
	laninfo_t *laninfo=NULL;
	//printf("%s,%d,group_id is %d,\t subidx_id is %d\n",__FUNCTION__,__LINE__,group_id,subidx_id);
	for(i=0;i<array_len;i++){
		laninfo = laninfo_array+i;
		if(laninfo->group == groupid && laninfo->idx==subidx){
			memcpy(exts,laninfo->exts,5);
			codepage =  laninfo->codepage;
		}
	}
	
	return codepage;
}

short int subtitle_set_groupidx(int dir)
{	
	int code_page=0;
	unsigned char exts[5];

	#ifndef SUBTILT_DEBUG
	return -1;
	#endif
	
	if(dir==DIR_ADD){
		group_id ++;
		if(group_id>GROUP_ID_MAX)
			group_id=GROUP_ID_MIN;
	}
	else{
		group_id --;
		if(group_id<GROUP_ID_MIN)
			group_id=GROUP_ID_MAX;
	}
	code_page=subtitle_get_codepage(group_id,0,exts);

	sp_info("Code Page==%d",code_page);
	return group_id;
}


short int subtitle_set_subidx(int dir)
{
	int code_page=0;
	unsigned char exts[5];

	#ifndef SUBTILT_DEBUG
	return -1;
	#endif
	
	if(dir==DIR_ADD){
		subidx_id ++;
		if(subidx_id>groupinfo_array[group_id].idx_max)
			subidx_id=groupinfo_array[group_id].idx_min;
		if(subidx_id<groupinfo_array[group_id].idx_min)
			subidx_id=groupinfo_array[group_id].idx_max;
	}
	else{
		subidx_id --;
		if(subidx_id<groupinfo_array[group_id].idx_min)
			subidx_id=groupinfo_array[group_id].idx_max;
		if(subidx_id>groupinfo_array[group_id].idx_max)
			subidx_id=groupinfo_array[group_id].idx_min;
	}
	
	code_page=subtitle_get_codepage(group_id,subidx_id,exts);
	sp_info("SUB IDX==%d, codepage==%d,exts=%s",subidx_id,code_page,exts);
	return subidx_id;
}





int subtitle_extend_filename(char *src_name,char**dest_name)
{
#ifndef SUBTILT_DEBUG
	return -1;
#endif

	int code_page=0;
	int name_len;
	int i=0;
	unsigned char exts[5];
	if(!src_name || !dest_name)
		return -1;
	code_page=subtitle_get_codepage(group_id,subidx_id,exts);

	if(code_page==0)
		return -1;
	
	name_len= strlen(src_name);
	if(name_len){
		*dest_name = malloc(name_len+5);
		if(*dest_name == NULL)
			return -1;
		memset(*dest_name,0,name_len+5);

		for(i=name_len-1;i>=0;i--){///先复制后缀名
			*(*dest_name+i+4) = src_name[i];
			if(src_name[i]=='.')
				break;
		}
		
		memcpy(*dest_name,src_name,i);///把文件名复制过去
		memcpy(*dest_name+i,exts,4);///填充后面的名称
	}
	else
		return -1;

	sp_info("EXTEND SUBTITLE FILE=%s",*dest_name);
	return 0;
}

void subtitle_free_filename(char**dest_name)
{
#ifndef SUBTILT_DEBUG
	return ;
#endif

	if(!dest_name && *dest_name!=NULL)
		free(*dest_name);
}

int subtitle_get_charset()
{
	int src_charset=LAN_GB2312;
	int code_page=0;
	unsigned char exts[5];
	code_page=subtitle_get_codepage(group_id,subidx_id,exts);
	switch(code_page){
		case 847:
			src_charset = LAN_CP874;
			break;
		case 932:
			src_charset = LAN_CP932;
			break;
		case 936:
			src_charset = LAN_GB2312;
			break;
		case 949:
			src_charset = LAN_CP949;
			break;
		case 950:
			src_charset = LAN_CP950;
			break;
		case 1250:
			src_charset = LAN_CP1250;
			break;
		case 1251:
			src_charset = LAN_CP1251;
			break;
		case 1252:
			src_charset = LAN_CP1252;
			break;
		case 1253:
			src_charset = LAN_CP1253;
			break;
		case 1254:
			src_charset = LAN_CP1254;
			break;
		case 1255:
			src_charset = LAN_CP1255;
			break;
		case 1256:
			src_charset = LAN_CP1256;
			break;
		case 1257:
			src_charset = LAN_CP1257;
			break;
		case 1258:
			src_charset = LAN_CP1258;
			break;
		default:
			sp_err("code page error!");
	}

	return src_charset;
}


