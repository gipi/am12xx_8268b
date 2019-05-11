#include "flickr_main.h"

char flickr_google_name[]="linshilin40@gmail.com";
char flickr_google_pwd[]="sllin0917";

//char flickr_fbook_name[]="actionsmicro@163.com";
char flickr_fbook_name[]="microactions@163.com";
char flickr_fbook_pwd[]="1234567890";

char flickr_yahoo_name[]="linshilin40@yahoo.com";
char flickr_yahoo_pwd[]="sllin0917";

char flickr_cache_dir[]="/mnt/udisk/flickr";

typedef struct flickr_test_cmd_s
{
	int test_cmd;	
	int is_thumbnail; ///< download thumbnail or large photo
	int which_contact;
	int which_album;
	int which_photo;
	char* photo_addr;
}flickr_test_cmd_t;

enum{
	CMD_SHOWINFO_ALBUMS,
	CMD_SHOWINFO_ALBUM,
	CMD_SHOWINFO_PHOTOS,
	CMD_DOWNLOADPHOTOS,
	CMD_DOWNLOADPHOTO,
	CMD_DOWNLOADALBUMS,
	CMD_DOWNLOADALBUM,
	CMD_DOWNLOADALBUMPHOTO,
	CMD_SHOWINFOCONTACT,
	CMD_INVALID,
};

flickr_test_cmd_t flickr_gcmd;

int check_para(flickr_gdata_t * gdata,int argc,char* argv[])
{
	int rtn = 0;
	if(argc<5){
		flickr_err("argc err, need email and pwd adn account type");
		return -1;
	}
	memset(&flickr_gcmd,0,sizeof(flickr_test_cmd_t));
	flickr_set_login_info(gdata,argv[1],argv[2],atoi(argv[3]));
	if(argc>=5){
		if(strcasecmp(argv[4],"showinfo-albums")==0)
			flickr_gcmd.test_cmd = CMD_SHOWINFO_ALBUMS;
		else if(strcasecmp(argv[4],"showinfo-album")==0)
			flickr_gcmd.test_cmd = CMD_SHOWINFO_ALBUM;
		else if(strcasecmp(argv[4],"showinfo_photos")==0)
			flickr_gcmd.test_cmd = CMD_SHOWINFO_PHOTOS;
		else if(strcasecmp(argv[4],"download_photos")==0)
			flickr_gcmd.test_cmd = CMD_DOWNLOADPHOTOS;
		else if(strcasecmp(argv[4],"download_photo")==0)
			flickr_gcmd.test_cmd = CMD_DOWNLOADPHOTO;
		else if(strcasecmp(argv[4],"download_albums")==0)
			flickr_gcmd.test_cmd = CMD_DOWNLOADALBUMS;
		else if(strcasecmp(argv[4],"download-album")==0)
			flickr_gcmd.test_cmd = CMD_DOWNLOADALBUM;
		else if(strcasecmp(argv[4],"download_album_photo")==0)
			flickr_gcmd.test_cmd = CMD_DOWNLOADALBUMPHOTO;
		else if(strcasecmp(argv[4],"showinfo_contact")==0)
			flickr_gcmd.test_cmd = CMD_SHOWINFOCONTACT;
		else
			flickr_gcmd.test_cmd = CMD_INVALID;
	}
	if(argc>=6){
		if(strcasecmp(argv[5],"me")==0)
			flickr_gcmd.which_contact = -1;
		else 
			flickr_gcmd.which_contact = atoi(argv[5]);
	}
		
	switch(flickr_gcmd.test_cmd){
		
		case CMD_SHOWINFO_ALBUMS:
			break;
		case CMD_SHOWINFO_ALBUM:
			if(argc>=7){
				flickr_gcmd.which_album = atoi(argv[6]);
				if(argc>7)
					flickr_gcmd.is_thumbnail = atoi(argv[7]);
			}
			else
				rtn = -1;
			break;
		case CMD_SHOWINFO_PHOTOS:
			if(argc>=7)
				flickr_gcmd.is_thumbnail = atoi(argv[6]);
			break;
		case CMD_DOWNLOADPHOTOS:
			break;
		case CMD_DOWNLOADPHOTO:
			if(argc>=7){
				flickr_gcmd.which_photo= atoi(argv[6]);
				if(argc>7)
					flickr_gcmd.is_thumbnail = atoi(argv[7]);
			}
			else
				rtn = -1;
			break;
		case CMD_DOWNLOADALBUMS:
			if(argc>=7)
				flickr_gcmd.is_thumbnail = atoi(argv[6]);
			break;
		case CMD_DOWNLOADALBUM:
			if(argc>=7){
				flickr_gcmd.which_album = atoi(argv[6]);
				if(argc>7)
					flickr_gcmd.is_thumbnail = atoi(argv[7]);
			}
			else
				rtn = -1;
			break;
		case CMD_DOWNLOADALBUMPHOTO:
			if(argc>=8){
				flickr_gcmd.which_album= atoi(argv[6]);
				flickr_gcmd.which_photo = atoi(argv[7]);
				if(argc>8)
					flickr_gcmd.is_thumbnail = atoi(argv[8]);
					
			}
			else
				rtn = -1;
			break;
		case CMD_SHOWINFOCONTACT:
			break;
		case CMD_INVALID:
			return -1;
	}
	if(rtn ==-1)
		flickr_err("Error Paras Error!");
	return 0;
}

int down_load_photo(flickr_gdata_t *gdata,flickr_test_cmd_t *test_cmd)
{
	flickr_download_info_t * down_info=NULL;
	int down_status=0;
	int err_status=0;
	char tmpbuf[256]="";
	int rtn=0;
	down_info = flickr_init_download_info(1,flickr_cache_dir,gdata,test_cmd->which_contact,test_cmd->which_album,test_cmd->which_photo,test_cmd->is_thumbnail);
	if(down_info){
		flickr_send_msg(gdata,FLICKR_CMD_DOWNLOADPHOTO,down_info);
		while(1){
			down_status = flickr_query_download_status(gdata,down_info,FLICKR_QUERY_CMD_RESULT);
			if(down_status>=0 || down_status==-2){
				printf("Down Load OK, Status===%d\n",down_status);
				break;
			}
			else{
				if(down_info)
					flickr_info("DownLoading@@@status=%d@@@@@@precent=%d\n",down_status,down_info->prog_down);
				sleep(1);
			}
		}
		flickr_free_download_info(down_info);
	}
	rtn = flickr_get_cache_path(gdata,test_cmd->which_contact,test_cmd->which_album,test_cmd->which_photo,test_cmd->is_thumbnail,tmpbuf,256);
	if(rtn==0)
		flickr_info("Cache_Path==%s",tmpbuf);
	return down_status;
}

int down_load_photos(flickr_gdata_t *gdata,flickr_test_cmd_t *test_cmd)
{
	int pic_total_num=0;
	int i=0;
	flickr_info_query_t infor_query;
	memset(&infor_query,0,sizeof(flickr_info_query_t));
	infor_query.which_contact = test_cmd->which_contact;
	pic_total_num = (int)flickr_get_info(gdata,GET_PHOTOS_COUNT,&infor_query);
	flickr_info("pic_total_num=%d",pic_total_num);
	for(i=0;i<pic_total_num;i++){
		test_cmd->which_photo = i;
		down_load_photo(gdata,test_cmd);
	}
	return 0;
}

int down_load_album(flickr_gdata_t *gdata,flickr_test_cmd_t *test_cmd)
{
	int pic_total_num=0;
	int i=0;
	flickr_info_query_t infor_query;
	memset(&infor_query,0,sizeof(flickr_info_query_t));
	infor_query.which_contact = test_cmd->which_contact;
	infor_query.which_photoset = test_cmd->which_album;
	pic_total_num = (int)flickr_get_info(gdata,GET_PHOTOSET_PHOTOS_COUNT,&infor_query);
	flickr_info("pic_total_num=%d,which_album=%d",pic_total_num,test_cmd->which_album);
	flickr_get_photosets_photos_info(gdata,test_cmd->which_contact,test_cmd->which_album,FLICKR_PRI_NONE);
	for(i=0;i<pic_total_num;i++){
		test_cmd->which_photo = i;
		down_load_photo(gdata,test_cmd);
	}
	flickr_free_photosets_photos_info(gdata,flickr_gcmd.which_contact,flickr_gcmd.which_album);
	return 0;
}

int down_load_albums(flickr_gdata_t *gdata,flickr_test_cmd_t *test_cmd)
{
	int photoset_num=0;
	flickr_info_query_t infor_query;
	int i=0;
	memset(&infor_query,0,sizeof(flickr_info_query_t));
	infor_query.which_contact = test_cmd->which_contact;
	photoset_num = (int)flickr_get_info(gdata,GET_PHOTOSET_COUNT,&infor_query);
	flickr_info("Photoset_num=%d",photoset_num);
	for(i=0;i<photoset_num;i++){
		test_cmd->which_album = i;
		down_load_album(gdata,test_cmd);
	}
	return 0;
}

int show_contacts(flickr_gdata_t*gdata,flickr_test_cmd_t *test_cmd)
{
	int i=0;
	int rtn=0;
	int contact_num=0;
	char *value=NULL;
	flickr_info_query_t info_query;
	rtn = flickr_get_contacts(gdata);
	flickr_info("@@@@@@@@Get contact OK@@@@@@rtn=%d@@@@@@@",rtn);
	value = flickr_get_info(gdata,GET_CONTACT_COUNT,&info_query);
	printf("value===0x%x",value);
	contact_num = (int)value;
	flickr_info("Contact Num==%d",contact_num);
	for(i=0;i<contact_num;i++){
		flickr_info("===========Contact[%d] Information===========",i);
		flickr_get_user_info(gdata,i);
		flickr_free_user_info(gdata,i);
	}
	return 0;
}

int main(int argc,char* argv[])
{	
	int rtn=FLICKR_RTN_OK;
	flickr_gdata_t * gdata=NULL;
	gdata = flickr_init_gdata();

	//flickr_set_login_info(gdata,flickr_google_name,flickr_google_pwd,ACCOUNT_TYPE_GOOGLE);
	//flickr_set_login_info(gdata,flickr_fbook_name,flickr_fbook_pwd,ACCOUNT_TYPE_FACEBOOK);
	if(check_para(gdata,argc,argv)==-1)
		return 0;
	
	flickr_auth(gdata);

	switch(flickr_gcmd.test_cmd){
		case CMD_SHOWINFO_ALBUMS:
		case CMD_SHOWINFO_ALBUM:
			rtn = flickr_get_photosets_info(gdata,flickr_gcmd.which_contact);
			if(flickr_gcmd.test_cmd==CMD_SHOWINFO_ALBUM && rtn==0)
				rtn = flickr_get_photosets_photos_info(gdata,flickr_gcmd.which_contact,flickr_gcmd.which_album,FLICKR_PRI_NONE);			
			break;
		case CMD_SHOWINFO_PHOTOS:
			flickr_get_photos_info(gdata,flickr_gcmd.which_contact,FLICKR_PRI_NONE);
			break;
		case CMD_DOWNLOADPHOTOS:
			flickr_get_photos_info(gdata,flickr_gcmd.which_contact,FLICKR_PRI_NONE);
			down_load_photos(gdata,&flickr_gcmd);
			break;
		case CMD_DOWNLOADPHOTO:
			flickr_get_photos_info(gdata,flickr_gcmd.which_contact,FLICKR_PRI_NONE);
			down_load_photo(gdata,&flickr_gcmd);
			break;
		case CMD_DOWNLOADALBUMS:
		case CMD_DOWNLOADALBUM:
		case CMD_DOWNLOADALBUMPHOTO:
			flickr_get_photosets_info(gdata,flickr_gcmd.which_contact);
			if(flickr_gcmd.test_cmd==CMD_DOWNLOADALBUMPHOTO){
				flickr_get_photosets_photos_info(gdata,flickr_gcmd.which_contact,flickr_gcmd.which_album,FLICKR_PRI_NONE);
				down_load_photo(gdata,&flickr_gcmd);
				flickr_free_photosets_photos_info(gdata,flickr_gcmd.which_contact,flickr_gcmd.which_album);
			}
			else if(flickr_gcmd.test_cmd==CMD_DOWNLOADALBUM)
				down_load_album(gdata,&flickr_gcmd);
			else if(flickr_gcmd.test_cmd==CMD_DOWNLOADALBUMS)
				down_load_albums(gdata,&flickr_gcmd);
			flickr_free_photosets_info(gdata,flickr_gcmd.which_contact);
			break;
		case CMD_SHOWINFOCONTACT:
			show_contacts(gdata,&flickr_gcmd);
			break;
	}
	
	flickr_free_gdata(gdata);
	return 1;
}