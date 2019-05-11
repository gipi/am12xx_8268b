#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "key_map.h"

#define USE_FULL_KBD		//use full for readboy

#define KEY_INFO_OFFSET	48	//cfg file data offset

struct key_item
{
	int pccode;
	int keycode;
};
typedef struct _key_map
{
	int totoalkey;
	struct key_item*	kitem;
}key_map;

int key_type;
key_map* cur_map=NULL;


#ifdef USE_FULL_KBD
struct full_kbd_item
{
	int pccode1;
	int pccode2;
	int keycode;
};
typedef struct _full_kbd_map
{
	int totoalkey;
	int CapsLock_Flag;
	int  Shift_Flag;     
	struct full_kbd_item*	kitem;
}full_kbd_map;

full_kbd_map* board_map=NULL;
#endif



int analy_normalkbd_cfgfile(FILE*fp)
{
	int rcnt;
	int keycnt=0;
	unsigned char buf[12];
	key_map* kmap=NULL;
	do{
		rcnt = fread(buf,sizeof(char),2*sizeof(int),fp);
		if(rcnt==8)
			keycnt++;
	}while(rcnt==8);
	kmap = (key_map*)malloc(sizeof(key_map));
	kmap->totoalkey = keycnt;
	kmap->kitem = (struct key_item*)malloc(keycnt*sizeof(struct key_item));
	fseek(fp,KEY_INFO_OFFSET+4,SEEK_SET);
	rcnt = fread(kmap->kitem,sizeof(char),sizeof(struct key_item)*keycnt,fp);
	if(rcnt!=keycnt*sizeof(struct key_item))
	{
		printf("normal kbd cfg file item num error,check it again\n");
		free(kmap->kitem);
		free(kmap);
		fclose(fp);
		return -1;
	}	
	if(cur_map)
	{
		free(cur_map->kitem);
		free(cur_map);
		cur_map = NULL;
	}
	cur_map = kmap;
#if 0
	int i;
	if(cur_map)
	{
		for(i=0;i<cur_map->totoalkey;i++)
		{
			printf("pccode[%d]==%x,hwcode[%d]==%x\n",i,cur_map->kitem[i].pccode,i,cur_map->kitem[i].keycode);
		}
	}
#endif	
	return 0;
}
#ifdef USE_FULL_KBD
int analy_fullkbd_cfgfile(FILE*fp)
{
	int rcnt;
	int keycnt=0;
	unsigned char buf[12];
	full_kbd_map* keyboardmap=NULL;
	do{
		rcnt = fread(buf,sizeof(char),3*sizeof(int),fp);
		if(rcnt==12)
			keycnt++;
	}while(rcnt==12);
	keyboardmap = (full_kbd_map*)malloc(sizeof(full_kbd_map));
	keyboardmap->totoalkey = keycnt;
	keyboardmap->kitem = (struct full_kbd_item*)malloc(keycnt*sizeof(struct full_kbd_item));
	fseek(fp,KEY_INFO_OFFSET+4,SEEK_SET);
	rcnt = fread(keyboardmap->kitem,sizeof(char),sizeof(struct full_kbd_item)*keycnt,fp);
	if(rcnt!=keycnt*sizeof(struct full_kbd_item))
	{
		printf("full kbd cfg file item num error,check it again\n");
		free(keyboardmap->kitem);
		free(keyboardmap);
		fclose(fp);
		return -1;
	}	
	if(board_map)
	{
		free(board_map->kitem);
		free(board_map);
		board_map = NULL;
	}
	keyboardmap->CapsLock_Flag=0;
	keyboardmap->Shift_Flag=0;
	board_map = keyboardmap;	
	return 0;
}
#endif

int read_swfkey_cfgfile(char* file)
{
	FILE* fp;
	int i,type;
	fp = fopen(file,"rb");
	if(fp==NULL)
	{
		printf("read %s error,please check whether it is exist\n",file);
		return -1;
	}
	fseek(fp,KEY_INFO_OFFSET,SEEK_SET);
	fread(&type,sizeof(char),sizeof(int),fp);
	if(0==type)//read normal kbd cfgfile
	{
		key_type=0;
		if(analy_normalkbd_cfgfile(fp)!=0)
			return -1;
	}
#ifdef USE_FULL_KBD
	else//read full kbd cfgfile
	{	
		key_type=1;
		if(analy_fullkbd_cfgfile(fp)!=0)
			return -1;		
	}
#endif
	fclose(fp);
	return 0;

}

int swf_key_read(struct am_sys_msg *msg)
{
	unsigned int i,hwkey=0,send_data=0,data=0;
	static unsigned char status=0;
	if(msg->type==SYSMSG_KEY)
	{
		hwkey = msg->dataload;
		if(key_type==0)
		{
			if(cur_map==NULL)
				return -1;
			for(i=0;i<cur_map->totoalkey;i++)
			{
				if(cur_map->kitem[i].keycode==hwkey)
					return 0x0100|cur_map->kitem[i].pccode;	
			}			
		}
#ifdef USE_FULL_KBD
		else if(key_type==1)
		{
			if(board_map==NULL)
				return -1;
			data=hwkey&0xFF;
			status=(hwkey>>24)&0xf;
			
			switch(status)
			{
				case 1://release
				{
					if(data==VK_KEY_SHIFT) //release shift 键
						board_map->Shift_Flag=0;	
					break;
				}
				case 2://press
				{					
					if(data==VK_KEY_CAPSLOCK)		//大小写切换
						board_map->CapsLock_Flag=(board_map->CapsLock_Flag&0x01)^0x01;
					if(data==VK_KEY_SHIFT)			//按下shift 键
						board_map->Shift_Flag=1;	
					
					if(board_map->Shift_Flag==0)		//不按shift 键			
					{
						for(i=0;i<board_map->totoalkey;i++)
						{
							if(board_map->kitem[i].keycode==hwkey)
								send_data= board_map->kitem[i].pccode1;
						}
						if(send_data==0)
							return 0;
						send_data=0x0100|send_data;
					}
					else								//按住shift 键
					{
						for(i=0;i<board_map->totoalkey;i++)
						{
							if(board_map->kitem[i].keycode==hwkey)
								send_data=board_map->kitem[i].pccode2;
						}
						if(send_data==0)
							return 0;
						send_data=0x1100|send_data;						
					}
					break;
				}
				
				case 3://hold key
				{	
					if(board_map->Shift_Flag==0)		//不按shift 键			
					{
						for(i=0;i<board_map->totoalkey;i++)
						{
							if(board_map->kitem[i].keycode==hwkey)
								send_data= board_map->kitem[i].pccode1;
						}
						if(send_data==0)
							return 0;
						send_data=0x0100|send_data;
					}
					else								//按住shift 键
					{
						for(i=0;i<board_map->totoalkey;i++)
						{
							if(board_map->kitem[i].keycode==hwkey)
								send_data=board_map->kitem[i].pccode2;
						}
						if(send_data==0)
							return 0;
						send_data=0x1100|send_data;						
					}
					break;
				}
				default:
					break;
			}
			return  send_data;
		}
#endif
	}
	return 0;
}
