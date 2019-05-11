#include "webalbum_api.h"
#include <stdio.h>
#include <string.h>

picasaweb_gdata_t gdata;
photo_down_info_t * g_download_info=NULL;
picasa_semi_t picasa_semi;
static char picasaweb_login_email[64]="linshilin40@gmail.com";
static char picasaweb_login_pwd[64]="sllin0917";

//static char picasaweb_login_email[64]="lee.wayne@gmail.com"; 
//static char picasaweb_login_pwd[32]="xu31vul31bjo41";
static char cache_addr[64] = "/mnt/udisk/webalbum/";

enum{
	CMD_SHOWINFO_ALBUMS,
	CMD_SHOWINFO_ALBUM,
	CMD_DOWNLOADALBUM,
	CMD_DELETEALBUM,
	CMD_CREATEALBUM,
	CMD_UPDATEALBUM,
	
	CMD_DOWNLOADPHOTO,
	CMD_DELETEPHOTO,
	CMD_UPDATEPHOTO,
	CMD_UPLOADPHOTO,

	CMD_GET_CONTACT,
	CMD_INVALID,
};

typedef struct test_cmd_s
{
	int test_cmd;	
	int is_thumbnail; ///< download thumbnail or large photo
	int which_album;
	int which_entry;
	char* photo_addr;
}test_cmd_t;

test_cmd_t gcmd;

int check_para(int argc,char* argv[])
{
	int rtn = -1;
	if(argc<3){
		picasa_err("argc err, need email and pwd");
		return -1;
	}
	picasa_set_longin(argv[1],argv[2]);
	if(argc>=4){
		if(strcasecmp(argv[3],"showinfo-albums")==0)
			gcmd.test_cmd = CMD_SHOWINFO_ALBUMS;
		else if(strcasecmp(argv[3],"showinfo-album")==0)
			gcmd.test_cmd = CMD_SHOWINFO_ALBUM;
		else if(strcasecmp(argv[3],"download-photo")==0)
			gcmd.test_cmd = CMD_DOWNLOADPHOTO;
		else if(strcasecmp(argv[3],"download-album")==0)
			gcmd.test_cmd = CMD_DOWNLOADALBUM;
		else if(strcasecmp(argv[3],"update-photo")==0)
			gcmd.test_cmd = CMD_UPDATEPHOTO;
		else if(strcasecmp(argv[3],"update-album")==0)
			gcmd.test_cmd = CMD_UPDATEALBUM;
		else if(strcasecmp(argv[3],"delete-photo")==0)
			gcmd.test_cmd = CMD_DELETEPHOTO;
		else if(strcasecmp(argv[3],"delete-album")==0)
			gcmd.test_cmd = CMD_DELETEALBUM;
		else if(strcasecmp(argv[3],"create-album")==0)
			gcmd.test_cmd = CMD_CREATEALBUM;
		else if(strcasecmp(argv[3],"upload-photo")==0)
			gcmd.test_cmd = CMD_UPLOADPHOTO;	
		else if(strcasecmp(argv[3],"get_contact")==0)
			gcmd.test_cmd = CMD_GET_CONTACT;
		else
			gcmd.test_cmd = CMD_INVALID;
	}
	switch(gcmd.test_cmd){
		case CMD_SHOWINFO_ALBUMS:
			break;
		case CMD_SHOWINFO_ALBUM:
			if(argc>=5)
				gcmd.which_album = atoi(argv[4]);
			else
				return -1;
			break;
			break;
		case CMD_DOWNLOADALBUM:
			if(argc>=5)
				gcmd.is_thumbnail = atoi(argv[4]);
			else
				return -1;
			break;
		case CMD_DELETEALBUM:
		case CMD_UPDATEALBUM:
			if(argc>=5)
				gcmd.which_entry = atoi(argv[4]);
			else
				return -1;
			break;
		case CMD_CREATEALBUM:
			break;
		case CMD_DOWNLOADPHOTO:
			if(argc>=5)
				gcmd.which_album = atoi(argv[4]);
			else
				return -1;
			if(argc>=6)
				gcmd.is_thumbnail= atoi(argv[5]);
			else
				return -1;
			if(argc>=7)
				gcmd.which_entry = atoi(argv[6]);
			else
				gcmd.which_entry = -1;		
			break;
		case CMD_DELETEPHOTO:
		case CMD_UPDATEPHOTO:
			if(argc>=5)
				gcmd.which_album = atoi(argv[4]);
			else
				return -1;
			if(argc>=6)
				gcmd.which_entry = atoi(argv[5]);
			else
				gcmd.which_entry = -1;	
			break;
		case CMD_UPLOADPHOTO:
			if(argc>=5)
				gcmd.which_entry = atoi(argv[4]);
			else
				return -1;
			if(argc>=6)
				gcmd.photo_addr = argv[5];
			else
				return -1;
			break;
		case CMD_INVALID:
			return -1;
	}
	return 0;
}

int down_load_test(picasaweb_feed_t* feed,int isthumbnail,int isall,int which_entry)
{
	if(isall){
		int i=0;
		for(i=0;i<feed->entry_num;i++)
		{
			photo_down_info_t * down_info=NULL;
			int down_status=0;
			int err_status = 0;
			down_info = (photo_down_info_t *)picasa_init_download_info(1,cache_addr,feed,i,isthumbnail,&err_status);
			picasa_send_msg(&gdata,PICASA_CMD_DOWNLOADPHOTO,down_info);
			while(1){
				down_status = picasa_query_download_status(&gdata,down_info,QUERY_CMD_RESULT);
				if(down_status>=0 || down_status==-2){
					printf("Down Load OK, Status===%d\n",down_status);
					break;
				}
				else{
					if(down_info)
						picasa_info("DownLoading@@@@@@@@@precent=%d\n",down_info->prog_down);
					sleep(1);
				}
			}
			picasa_free_download_info(down_info);
		}
	}
	else{
		photo_down_info_t * down_info=NULL;
		int down_status=0;
		int err_status=0;
		down_info = (photo_down_info_t *)picasa_init_download_info(1,cache_addr,feed,which_entry,isthumbnail,&err_status);
		picasa_send_msg(&gdata,PICASA_CMD_DOWNLOADPHOTO,down_info);
		while(1){
			down_status = picasa_query_download_status(&gdata,down_info,QUERY_CMD_RESULT);
			if(down_status>=0 || down_status==-2){
				printf("Down Load OK, Status===%d\n",down_status);
				break;
			}
			else{
				if(down_info)
						picasa_info("DownLoading@@@@@@@@@precent=%d\n",down_info->prog_down);
				sleep(1);
			}
		}
		picasa_free_download_info(down_info);
	}
	return 0;
		
}

int update_album_test(picasaweb_feed_t* feed,int which_entry)
{
	album_info_t album_info;
	memset(&album_info,0,sizeof(album_info_t));
	picasa_fill_atomic((xmlChar*)"Actions_album_test",&(album_info.title));
	picasa_fill_atomic((xmlChar*)"Actions_album_test",&(album_info.summary));
	picasa_fill_atomic((xmlChar*)"Actions_album_test",&(album_info.location));
	picasa_fill_atomic((xmlChar*)"public",&(album_info.access));
	picasa_fill_atomic(NULL,&(album_info.timestamp));
	picasa_update_album_info(&album_info,&gdata,feed,which_entry);
	picasa_free_atomic(&album_info.title);
	picasa_free_atomic(&album_info.summary);
	picasa_free_atomic(&album_info.location);
	picasa_free_atomic(&album_info.access);
	picasa_free_atomic(&album_info.timestamp);
	return 0;
}

int create_album_test(picasaweb_feed_t* feed)
{
	album_info_t album_info;
	memset(&album_info,0,sizeof(album_info_t));
	picasa_fill_atomic((xmlChar*)"Actions_album_test",&(album_info.title));
	picasa_fill_atomic((xmlChar*)"Actions_album_test",&(album_info.summary));
	picasa_fill_atomic((xmlChar*)"Actions_album_test",&(album_info.location));
	picasa_fill_atomic((xmlChar*)"public",&(album_info.access));
	picasa_fill_atomic(NULL,&(album_info.timestamp));
	picasa_create_new_album(&album_info,&gdata);
	picasa_free_atomic(&album_info.title);
	picasa_free_atomic(&album_info.summary);
	picasa_free_atomic(&album_info.location);
	picasa_free_atomic(&album_info.access);
	picasa_free_atomic(&album_info.timestamp);
	return 0;
}

int update_photo_test(picasaweb_feed_t* feed,int which_entry)
{
	photo_info_t photo_info;
	memset(&photo_info,0,sizeof(photo_info_t));
	picasa_fill_atomic((xmlChar*)"Actions_photo_test",&(photo_info.summary));
	picasa_fill_atomic((xmlChar*)"Actions_photo_test",&(photo_info.description));
	picasa_update_photo_info(&photo_info,&gdata,feed,which_entry);
	picasa_free_atomic(&(photo_info.summary));
	picasa_free_atomic(&(photo_info.description));
	return 0;
}

int upload_photo_test(char* photopath,picasaweb_feed_t* feed,int which_entry)
{
	photo_upload_info_t * upload_info=NULL;
	int upload_status;
	upload_info=(photo_upload_info_t * )picasa_init_upload_info(photopath,feed,which_entry);
	if(upload_info){
		picasa_send_msg(&gdata,PICASA_CMD_UPLOADPHOTO,upload_info);
		while(1){
			upload_status = picasa_query_upload_status(&gdata,upload_info,QUERY_CMD_RESULT);
			if(upload_status>=0 || upload_status==-2){
				printf("UpLoad OK, Status===%d\n",upload_status);
				break;
			}
			else{
				if(upload_info)
						picasa_info("UpLoading@@@@@@@@@percent=%d/%d\n",upload_info->uploaded_bytes,upload_info->filesize_bytes);
				sleep(1);
			}
		}
	}
	picasa_free_upload_info(upload_info);
	return 0;
}

int dowload_user_profile(picasaweb_feed_t* feed)
{
	int i=0;
	if(feed==NULL)
		return -1;
	for(i=0;i<feed->entry_num;i++){
		photo_down_info_t * down_info=NULL;
		int down_status=0;
		int err_status = 0;
		picasa_info("@@@@down load Profile Entry===%d",i);
		down_info = (photo_down_info_t *)picasa_init_download_info(1,cache_addr,feed,i,0,&err_status);
		if(down_info){
			picasa_download_photo(&gdata, down_info);
			picasa_free_download_info(down_info);
			down_info=NULL;
		}
	}
}

int main(int argc,char* argv[])
{
	int status=0;
	char * curlver;

	picasa_info("argc===%d\n",argc);

	if(check_para(argc,argv)==-1){
		printf("##########Sorry parameter ERR############\n");
		return 0;
	}
	picasa_init_resource();

	curlver = curl_version();
	printf("curlversion=%s,pid==0x%x\n",curlver,getpid());
	//picasa_set_longin(picasaweb_login_email,picasaweb_login_pwd);
	
	picasa_init_gdata(&gdata);
	if(gdata.curl) {
		//picasa_debug(gdata.curl);
		status=picasa_authentication(&gdata);
		
		if(status==HttpStatus_OK){
			xmlDocPtr doc_ptr=NULL;
			
			picasaweb_feed_t * feed_albums=NULL;
			picasaweb_feed_t * feed_albums_load=NULL;
			picasaweb_feed_t * feed_one_album=NULL;
			picasaweb_feed_t * feed_one_album_load=NULL;
			picasaweb_feed_t * feed_contact = NULL;
			feed_albums = (picasaweb_feed_t *)picasa_get_albums_info(&gdata,NULL,0);
			
			switch(gcmd.test_cmd){
				case CMD_SHOWINFO_ALBUMS:
					picasa_save_albums_info(&gdata,feed_albums,cache_addr,NULL);
					feed_albums_load = picasa_load_albums_info(&gdata,cache_addr,NULL);
					if(feed_albums_load)
						picasa_free_albums_info(feed_albums_load);
					break;
				case CMD_SHOWINFO_ALBUM:
					feed_one_album = (picasaweb_feed_t *)picasa_get_album_info(&gdata,feed_albums,gcmd.which_album);
					picasa_save_album_info(&gdata,feed_albums,gcmd.which_album,cache_addr);
					feed_one_album_load = picasa_load_album_info(&gdata,feed_albums,gcmd.which_album,cache_addr);
					if(feed_one_album_load)
						picasa_free_album_info(feed_albums,gcmd.which_album);
					break;
				case CMD_DOWNLOADALBUM:
					down_load_test(feed_albums,gcmd.is_thumbnail,1,0);
					break;
				case CMD_DELETEALBUM:
					picasa_delete_album(&gdata,feed_albums,gcmd.which_entry);
					break;
				case CMD_UPDATEALBUM:
					update_album_test(feed_albums,gcmd.which_entry);
					break;
				case CMD_CREATEALBUM:
					create_album_test(feed_albums);
					break;

				case CMD_DOWNLOADPHOTO:
					feed_one_album = (picasaweb_feed_t *)picasa_get_album_info(&gdata,feed_albums,gcmd.which_album);
					if(gcmd.which_entry==-1)
						down_load_test(feed_one_album,gcmd.is_thumbnail,1,0);
					else 
						down_load_test(feed_one_album,gcmd.is_thumbnail,1,gcmd.which_entry);
					break;
				case CMD_UPDATEPHOTO:
					feed_one_album = (picasaweb_feed_t *)picasa_get_album_info(&gdata,feed_albums,gcmd.which_album);
					update_photo_test(feed_one_album,gcmd.which_entry);
					break;
				case CMD_DELETEPHOTO:
					feed_one_album = (picasaweb_feed_t *)picasa_get_album_info(&gdata,feed_albums,gcmd.which_album);
					picasa_delete_photo(&gdata,feed_one_album,gcmd.which_entry);
					break;
				case CMD_UPLOADPHOTO:
					upload_photo_test(gcmd.photo_addr,feed_albums,gcmd.which_entry);
					break;
				case CMD_GET_CONTACT:
					feed_contact = picasa_get_contact(&gdata);
					//picasa_save_contact_info(&gdata, feed_contact,cache_addr);
					//picasa_load_contact_info(& gdata, cache_addr);
					dowload_user_profile(feed_contact);
					if(feed_contact)
						picasa_free_contact(feed_contact);
					break;
			}
			if(feed_one_album)
				picasa_free_album_info(feed_albums,gcmd.which_album);
			
			picasa_free_albums_info(feed_albums);
		}

		/* always cleanup */
		
	}
	picasa_free_gdata(&gdata);
	picasa_info("################Work Done################\n");
	return 0;
}
