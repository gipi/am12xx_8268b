#include <webalbum_api.h>

facebook_data fdata;

int analy_cmd(int argc, char* argv[])
{
	int cmd = -1;
	facebook_dbg("argc is %d",argc);
	if(argc<=3)
	{
		facebook_dbg("argc error!Need user email & pwd and cmd");
		return cmd;
	}
	if(strcasecmp(argv[3],"user_info")==0)
		cmd = USER_INFO;
	else if(strcasecmp(argv[3],"user_picture")==0)
		cmd = USER_PICTURE;
	else if(strcasecmp(argv[3],"albums_info")==0)
		cmd = ALBUMS_INFO;
	else if(strcasecmp(argv[3],"album_cover")==0)
		cmd = ALBUM_COVER;
	else if(strcasecmp(argv[3],"albumphoto_info")==0)
		cmd = ALBUMPHOTO_INFO;
	else if(strcasecmp(argv[3],"download_photo_big")==0)
		cmd = DOWNLOAD_PHOTO_BIG;
	else if(strcasecmp(argv[3],"download_photo_small")==0)
		cmd = DOWNLOAD_PHOTO_SMALL;
	else if(strcasecmp(argv[3],"friends_list")==0)
		cmd = FRIENDS_LIST;
	else if(strcasecmp(argv[3],"friendslist_members")==0)
		cmd = FRIENDSLIST_MEMBERS;
	else if(strcasecmp(argv[3],"friend_albums_info")==0)
		cmd = FRIEND_ALBUMS_INFO;
	else if(strcasecmp(argv[3],"friend_albumphoto_info")==0)
		cmd = FRIEND_ALBUMPHOTO_INFO;
	else if(strcasecmp(argv[3],"friend_download_photo_big")==0)
		cmd = FRIEND_DOWNLOAD_PHOTO_BIG;
	else if(strcasecmp(argv[3],"friend_download_photo_small")==0)
		cmd = FRIEND_DOWNLOAD_PHOTO_SMALL;
	else if(strcasecmp(argv[3],"friends")==0)
		cmd = FRIENDS;
	else if(strcasecmp(argv[3],"friend_album_cover")==0)
		cmd = FRIEND_ALBUM_COVER;
	else if(strcasecmp(argv[3],"upload_feed")==0)
		cmd = UPLOAD_FEED;
	else if(strcasecmp(argv[3],"upload_comments")==0)
		cmd = UPLOAD_COMMENTS;
	else if(strcasecmp(argv[3],"create_album")==0)
		cmd = CREATE_ALBUM;
	else if(strcasecmp(argv[3],"upload_photo")==0)
		cmd = UPLOAD_PHOTO;
	
	if(cmd==ALBUM_COVER || cmd==ALBUMPHOTO_INFO){
		if(argc!=5){
			facebook_dbg("get albumphoto info/cover error, need album index");
			cmd = -1;
		}
		else{
			facebook_dbg("album index is %d",atoi(argv[4]));
		}
	}
	if(cmd==DOWNLOAD_PHOTO_BIG || cmd==DOWNLOAD_PHOTO_SMALL){
		if(argc!=7){
			facebook_dbg("download photo error, need album index, start photo index and end photo index");
			cmd = -1;
		}
		else{
			facebook_dbg("album index is %d, start photo index is %d, end photo index is %d",atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
		}
	}
	if(cmd==FRIENDSLIST_MEMBERS){
		if(argc!=5){
			facebook_dbg("get friendslist members error, need friendslist index");
			cmd = -1;
		}
		else{
			facebook_dbg("friendlist index is %d",atoi(argv[4]));
		}
	}
	if(cmd==FRIEND_ALBUMS_INFO){
		if(argc!=5){
			facebook_dbg("get friend album info error, need friendlist/member index");
			cmd = -1;
		}
		else{
			facebook_dbg("friend index is %d",atoi(argv[4]));
		}
	}
	if(cmd==FRIEND_ALBUMPHOTO_INFO || cmd==FRIEND_ALBUM_COVER){
		if(argc!=6){
			facebook_dbg("get friend albumphoto info error, need friendlist/member/album index");
			cmd = -1;
		}
		else{
			facebook_dbg("friend index is %d, album is %d",atoi(argv[4]),atoi(argv[5]));
		}
	}
	if(cmd==FRIEND_DOWNLOAD_PHOTO_BIG || cmd==FRIEND_DOWNLOAD_PHOTO_SMALL){
		if(argc!=8){
			facebook_dbg("download friend's photo error, need friendlist/member/album/start photo and end photo index");
			cmd = -1;
		}
		else{
			facebook_dbg("friend index is %d, album is %d, start photo is %d, end photo is %d",atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), atoi(argv[7]));
		}
	}
	if(cmd==UPLOAD_FEED || cmd==UPLOAD_COMMENTS){
		if(argc!=5){
			facebook_dbg("upload feed error, need profile/comments id and message");
			cmd = -1;
		}
		else{
			facebook_dbg("message is %s",argv[4]);
		}
	}
	if(cmd==CREATE_ALBUM){
		if(argc!=6){
			facebook_dbg("create album error, need album name and message");
			cmd = -1;
		}
		else{
			facebook_dbg("name is %s, message is %s",argv[4], argv[5]);
		}
	}
	return cmd;
}

int main(int argc, char* argv[])
{
	int rtn = 0;
	facebook_cmd fb_cmd;
	char *curlver;
	facebook_user *userinfo = NULL;
	facebook_photoalbums *photoalbums = NULL;
	int i=0;
	facebook_photo_down_info *f_down_info= NULL;
	facebook_feed *f_feed = NULL;
	facebook_friendlists *friendlists = NULL;
	facebook_contact * contact=NULL;

	facebook_data *facebook_data_s = &fdata;
	if(facebook_data_s==NULL){
		facebook_dbg("facebook_data_s malloc failed");
		rtn = -1;
		goto main_out;
	}
	
	fb_cmd = analy_cmd(argc,argv);
	facebook_dbg("cmd is %d",fb_cmd);
	if(fb_cmd==-1){
		facebook_dbg("argv is error!");
		rtn = -1;
		goto main_out;
	}
	rtn = facebook_set_longin(argv[1],argv[2]);
	if(rtn!=0){
		facebook_dbg("set login error!");
		goto main_out;
	}
	
	curlver = curl_version();
	facebook_dbg("curl version is %s",curlver);
	rtn = facebook_init_fdata(facebook_data_s);
	if(rtn!=0){
		facebook_dbg("init fdata error!");
		goto main_out;
	}
	
	if(facebook_data_s->curl != NULL){
		if(facebook_authentication(facebook_data_s)==0){
			facebook_dbg("Authentication success!");
			switch(fb_cmd)
			{
				case USER_INFO:
					userinfo = get_user_info(facebook_data_s);
					break;
				case USER_PICTURE:
					userinfo = get_user_info(facebook_data_s);
					f_feed = facebook_feed_type(USER_PROFILE,(void *)userinfo);
					
					f_down_info = facebook_init_download_info(1,"/mnt/udisk/facebook/",f_feed,0,1);
					facebook_download_photo(facebook_data_s, f_down_info);
					facebook_free_download_info(f_down_info);
					break;
				case ALBUMS_INFO:
					get_user_info(facebook_data_s);
					photoalbums = facebook_get_albums_info(facebook_data_s, 0, 0);
					break;
				case ALBUM_COVER:
					get_user_info(facebook_data_s);
					photoalbums = facebook_get_albums_info(facebook_data_s, 0, 0);
					f_feed = facebook_feed_type(ALBUM_FEED,(void *)photoalbums);
					
					f_down_info = facebook_init_download_info(1,"/mnt/udisk/facebook/",f_feed,atoi(argv[4]),1);
					facebook_download_photo(facebook_data_s, f_down_info);
					facebook_free_download_info(f_down_info);
					break;
				case ALBUM_COMMENT:
					break;
				case ALBUMPHOTO_INFO:
					get_user_info(facebook_data_s);
					photoalbums = facebook_get_albums_info(facebook_data_s, 0, 0);
					get_albumphoto_info(facebook_data_s, atoi(argv[4]), photoalbums);
					break;
				case DOWNLOAD_PHOTO_BIG:
					get_user_info(facebook_data_s);
					photoalbums = facebook_get_albums_info(facebook_data_s, 0, 0);
					if(get_albumphoto_info(facebook_data_s, atoi(argv[4]), photoalbums) == 0){
						f_feed = facebook_feed_type(PHOTO_FEED,(void *)(photoalbums->album_entry+atoi(argv[4])));
					}
					for(i=atoi(argv[5]);i<=atoi(argv[6]);i++){
						f_down_info = facebook_init_download_info(1,"/mnt/udisk/facebook/",f_feed,i,0);
						facebook_download_photo(facebook_data_s, f_down_info);
						facebook_free_download_info(f_down_info);
					}
					break;
				case DOWNLOAD_PHOTO_SMALL:
					get_user_info(facebook_data_s);
					photoalbums = facebook_get_albums_info(facebook_data_s, 0, 0);
					if(get_albumphoto_info(facebook_data_s, atoi(argv[4]), photoalbums) == 0){
						f_feed = facebook_feed_type(PHOTO_FEED,(void *)(photoalbums->album_entry+atoi(argv[4])));
					}
					for(i=atoi(argv[5]);i<=atoi(argv[6]);i++){
						f_down_info = facebook_init_download_info(1,"/mnt/udisk/facebook/",f_feed,i,1);
						facebook_download_photo(facebook_data_s, f_down_info);
						facebook_free_download_info(f_down_info);
					}
					break;
				case FRIENDS_LIST:
					friendlists = get_friends_list(facebook_data_s);
					break;
				case FRIENDSLIST_MEMBERS:
					friendlists = get_friends_list(facebook_data_s);
					get_listmembers_info(facebook_data_s, friendlists->friendlist_entry+atoi(argv[4]));
					break;
				case FRIEND_ALBUMS_INFO:
					contact = get_contact(facebook_data_s);
					photoalbums = facebook_get_albums_info(facebook_data_s, contact, atoi(argv[4]));
					break;
				case FRIEND_ALBUMPHOTO_INFO:
					contact = get_contact(facebook_data_s);
					photoalbums = facebook_get_albums_info(facebook_data_s, contact, atoi(argv[4]));
					get_albumphoto_info(facebook_data_s, atoi(argv[5]), photoalbums);
					break;
				case FRIEND_DOWNLOAD_PHOTO_BIG:
					contact = get_contact(facebook_data_s);
					photoalbums = facebook_get_albums_info(facebook_data_s, contact, atoi(argv[4]));
					if(get_albumphoto_info(facebook_data_s, atoi(argv[5]), photoalbums) == 0){
						f_feed = facebook_feed_type(PHOTO_FEED,(void *)(photoalbums->album_entry+atoi(argv[5])));
					}
					for(i=atoi(argv[6]);i<=atoi(argv[7]);i++){
						f_down_info = facebook_init_download_info(1,"/mnt/udisk/facebook/",f_feed,i,0);
						facebook_download_photo(facebook_data_s, f_down_info);
						facebook_free_download_info(f_down_info);
					}
					break;
				case FRIEND_DOWNLOAD_PHOTO_SMALL:
					contact = get_contact(facebook_data_s);
					photoalbums = facebook_get_albums_info(facebook_data_s, contact, atoi(argv[4]));
					if(get_albumphoto_info(facebook_data_s, atoi(argv[5]), photoalbums) == 0){
						f_feed = facebook_feed_type(PHOTO_FEED,(void *)(photoalbums->album_entry+atoi(argv[5])));
					}
					for(i=atoi(argv[6]);i<=atoi(argv[7]);i++){
						f_down_info = facebook_init_download_info(1,"/mnt/udisk/facebook/",f_feed,i,1);
						facebook_download_photo(facebook_data_s, f_down_info);
						facebook_free_download_info(f_down_info);
					}
					break;
				case FRIENDS:
					contact = get_contact(facebook_data_s);
					break;
				case FRIEND_ALBUM_COVER:
					contact = get_contact(facebook_data_s);
					photoalbums = facebook_get_albums_info(facebook_data_s, contact, atoi(argv[4]));
					f_feed = facebook_feed_type(ALBUM_FEED,(void *)photoalbums);
					
					f_down_info = facebook_init_download_info(1,"/mnt/udisk/facebook/",f_feed,atoi(argv[5]),1);
					facebook_download_photo(facebook_data_s, f_down_info);
					facebook_free_download_info(f_down_info);
					break;
				case UPLOAD_FEED:
					userinfo = get_user_info(facebook_data_s);
					facebook_dbg("%s",upload_feed(facebook_data_s, userinfo->id.data, argv[4]));
					break;
				case UPLOAD_COMMENTS:
					photoalbums = facebook_get_albums_info(facebook_data_s, 0, 0);
					for(i=0;i<photoalbums->album_num;i++){
						facebook_dbg("%s",upload_comments(facebook_data_s, (photoalbums->album_entry+i)->id.data, argv[4]));
					}
					break;
				case CREATE_ALBUM:
					facebook_dbg("%s",create_album(facebook_data_s,argv[4], argv[5]));
					break;
				case UPLOAD_PHOTO:
					break;
			}
		}
		else{
			facebook_dbg("Authentication fail!");
			rtn = -1;
		}
		
	}
	else{
		printf("facebook_data_s->curl is NULL");
		rtn = -1;
		goto main_out;
	}
main_out:
	if(rtn == 0){
		facebook_dbg("user logout");
		//facebook_logout();
	}
	facebook_free_fdata(facebook_data_s);
	facebook_free_photoalbums(photoalbums);
	facebook_free_friendlists(friendlists);
	facebook_free_userinfo(userinfo);
	return rtn;
}

