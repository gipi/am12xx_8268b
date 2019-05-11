#include "facebook_common.h"
#include "facebook_json_analy.h"

size_t facebook_func_write_data(void *ptr, size_t size, size_t nmemb, void * userdata)
{
	facebook_write_data *write_data = (facebook_write_data*)userdata;
	unsigned int size_str=size*nmemb;
	unsigned int size_malloc=size_str+1;
	if(write_data->cancel_write){
		write_data->cancel_write = 0;
		facebook_dbg("%s,%d: Function Write Cancel!\n");
		return 0;
	}
	
	//facebook_dbg("size is %d,nmemb is %d",size,nmemb);
	if(write_data->data_type==Facebook_data_write_buffer){
		if(write_data->data_head==NULL){
			write_data->data_head=(char *)facebook_malloc(size_malloc);
			if(write_data->data_head==NULL){
				facebook_dbg("Malloc Failed, size is %d\n",size_malloc);
				return 0;
			}
			write_data->data_cur = write_data->data_head;
			write_data->data_len = size_malloc;
			write_data->data_used = 0;
		}
		else{
			unsigned int newsize=write_data->data_used+size_malloc;
			if(newsize >= write_data->data_len){
				///need to realloc the buffer to store the data received
				char * buf=NULL;
				buf = (char*)facebook_realloc(write_data->data_head,newsize);
				if(buf==NULL){
					facebook_dbg("Realloc Failed\n");
					facebook_data_write_free(write_data);
					return 0;
				}
				else{
					write_data->data_head = buf;
					write_data->data_cur = write_data->data_head + write_data->data_used;
					write_data->data_len = newsize;
				}
			}
		}
		memcpy(write_data->data_cur,ptr,size_str);
		write_data->data_used +=size_str;
		write_data->data_cur = write_data->data_head + write_data->data_used;
		*(write_data->data_head+write_data->data_used)=0;//should end with \0
		//facebook_dbg("now save string is \n%s",write_data->data_head);
		return size*nmemb;
	}
	else if(write_data->data_type==Facebook_data_write_file){
		void *fp = write_data->file_handle;
		if(fp==NULL)
		{
			facebook_dbg("Error---file handle is NULL");
			return 0;
		}
		return fwrite(ptr, sizeof(unsigned char), size*nmemb, (FILE*)fp);
	}
	else{
		facebook_dbg("Error---data type %d!",write_data->data_type);
		return 0;
	}
}

char * facebook_malloc(unsigned int size)
{
	char * buf = (char*)malloc(size);
	if(buf==NULL){
		facebook_dbg("Facebook Malloc failed----%d",size);
	}
	else{
		memset(buf,0,size);
	}
	return buf;
}

int facebook_free(void * buffer)
{
	if(buffer!=NULL){
		free(buffer);
	}
	return 0;
}

char * facebook_realloc(char *ptr,unsigned int newsize)
{
	char *buf=(char*)realloc(ptr,newsize);
	if(buf==NULL){
		facebook_dbg("Realloc Failed! new size is %d\n", newsize);
	}
	return buf;
}

int facebook_fflush(void *fp)
{
	int fd;
	fflush((FILE *)fp);
	fd = fileno((FILE *)fp);
	if(fsync(fd)==-1){
		facebook_dbg("flush Error!\n");
		return -1;
	}
	return 0;
}

int facebook_fremove(char* file)
{
	int err=0;
	if(file){
		err = unlink(file);
		if(err != 0){
			facebook_dbg("Remove Error file=%s, reason=%s\n",file,strerror(errno));
			return -1;
		}
		return 0;
	}
	return -1;
}

int facebook_get_ini_albumsname(facebook_data *f_data,char* cache_dir,char*filename,char* namebuf,int len)
{
	int dir_len = 0;
	if(cache_dir)
		dir_len = strlen(cache_dir);
	else
		return -1;
	if(facebook_create_dir(cache_dir)!=0) // if the dir is not exist, try to create it first
		return -1;
	if(filename){
		if(strlen(filename)+4+dir_len>len){
			facebook_dbg("Sorry The name buf is too short,len=%d, needed %d,",len,strlen(filename)+4+dir_len);
			return -1;
		}
	}
	else{
		if(strlen(f_data->usermail)+4+dir_len>len){
			facebook_dbg("Sorry The name buf is too short,len=%d, needed %d,",len,strlen(f_data->usermail)+4+dir_len);
			return -1;
		}
	}
	memset(namebuf,0, len);
	if(*(cache_dir+dir_len-1)!='/'){
		if(filename)
			sprintf(namebuf,"%s/%s.ini",cache_dir,filename);
		else
			sprintf(namebuf,"%s/%s.ini",cache_dir,f_data->usermail);
	}
	else{
		if(filename)
			sprintf(namebuf,"%s%s.ini",cache_dir,filename);
		else
			sprintf(namebuf,"%s%s.ini",cache_dir,f_data->usermail);
	}
	facebook_dbg("Name Buf = %s",namebuf);
	return 0;
}

int facebook_get_ini_albumname(facebook_photoalbums *feed_albums,int which_album,char* cache_dir,char* namebuf,int len)
{
	facebook_album* album=NULL;
	int dir_len = 0;
	if(cache_dir)
		dir_len = strlen(cache_dir);
	else
		return -1;
	
	if(facebook_create_dir(cache_dir)!=0) // if the dir is not exist, try to create it first
		return -1;
	album = feed_albums->album_entry+which_album;
	if(album->id.data_len+4+dir_len>len){
		facebook_dbg("Sorry The name buf is too short,len=%d, needed %d,",len,album->id.data_len+4+dir_len);
		return -1;
	}
	else{
		memset(namebuf,0, len);
		if(*(cache_dir+dir_len-1)!='/')
			sprintf(namebuf,"%s/%s.ini",cache_dir,album->id.data);
		else
			sprintf(namebuf,"%s%s.ini",cache_dir,album->id.data);
		facebook_dbg("Name Buf = %s",namebuf);
		return 0;
	}
}

int facebook_get_ini_contactname(facebook_data *f_data,char* cache_dir,char* namebuf,int len)
{
	int dir_len = 0;
	if(cache_dir)
		dir_len = strlen(cache_dir);
	else
		return -1;
	if( facebook_create_dir(cache_dir)!=0) // if the dir is not exist, try to create it first
		return -1;
	
	if(8+4+dir_len>len){
		facebook_dbg("Sorry The name buf is too short,len=%d, needed %d,",len,8+4+dir_len);
		return -1;
	}
	else{
		memset(namebuf,0, len);
		if(*(cache_dir+dir_len-1)!='/')
			sprintf(namebuf,"%s/contacts.ini",cache_dir);
		else
			sprintf(namebuf,"%scontacts.ini",cache_dir);
		facebook_dbg("Name Buf = %s",namebuf);
		return 0;
	}
}

int facebook_reset_atomic(facebook_atomic *atomic,unsigned int *offset)
{
	if(atomic->data!=NULL){
		atomic->data = (char*)*offset;
		*offset +=atomic->data_len;
	}
	return 0;
}

int facebook_reset_from(facebook_from *from,unsigned int *offset)
{
	if(from){
		facebook_reset_atomic(&from->id,offset);
		facebook_reset_atomic(&from->name,offset);
	}
	return 0;
}

int facebook_reset_comments(facebook_comment *comments,unsigned int *offset)
{
	if(comments){
		facebook_reset_atomic(&comments->id,offset);
		facebook_reset_from(&comments->from,offset);
		facebook_reset_atomic(&comments->message,offset);
		facebook_reset_atomic(&comments->created_time,offset);
	}
	return 0;
}

int facebook_reset_images(facebook_picture *images,unsigned int *offset)
{
	int i=0;
	for(i=0;i<5;i++){
		if(images+i)
			facebook_reset_atomic(&(images+i)->source,offset);
	}
	return 0;
}

int facebook_reset_albumsentry(facebook_album *album,unsigned int offset)
{
	unsigned int tmp_offset=offset;

	facebook_reset_atomic(&album->id,&tmp_offset);
	
	facebook_reset_from(&album->from,&tmp_offset);
	
	facebook_reset_atomic(&album->name,&tmp_offset);
	facebook_reset_atomic(&album->description,&tmp_offset);
	facebook_reset_atomic(&album->link,&tmp_offset);
	facebook_reset_atomic(&album->cover_photo,&tmp_offset);
	facebook_reset_atomic(&album->privacy,&tmp_offset);
	facebook_reset_atomic(&album->type,&tmp_offset);
	facebook_reset_atomic(&album->created_time,&tmp_offset);
	facebook_reset_atomic(&album->updated_time,&tmp_offset);

	facebook_reset_comments(&album->comments,&tmp_offset);
	album->photo_entry = NULL;
	
	return 0;
}

int facebook_reset_photoentry(facebook_photo *photo,unsigned int offset)
{
	unsigned int tmp_offset=offset;

	facebook_reset_atomic(&photo->id,&tmp_offset);
	
	facebook_reset_from(&photo->from,&tmp_offset);
	
	facebook_reset_atomic(&photo->name,&tmp_offset);
	facebook_reset_atomic(&photo->picture,&tmp_offset);
	facebook_reset_atomic(&photo->source,&tmp_offset);
	
	facebook_reset_images(photo->images,&tmp_offset);
	
	facebook_reset_atomic(&photo->link,&tmp_offset);
	facebook_reset_atomic(&photo->icon,&tmp_offset);
	facebook_reset_atomic(&photo->created_time,&tmp_offset);
	facebook_reset_atomic(&photo->updated_time,&tmp_offset);

	facebook_reset_comments(&photo->comments,&tmp_offset);
	
	return 0;
}

int facebook_reset_memberentry(facebook_member *member,unsigned int offset)
{
	unsigned int tmp_offset=offset;

	facebook_reset_atomic(&member->id,&tmp_offset);
	facebook_reset_atomic(&member->name,&tmp_offset);
	
	return 0;
}

int facebook_write_atomic(void *file_handle,facebook_atomic * atomic)
{
	int bytes_writes=0;
	if(atomic->data_len==0)
		return 0;
	bytes_writes = fwrite(atomic->data, 1, atomic->data_len,(FILE*)file_handle);
	if(bytes_writes!=atomic->data_len){
		facebook_dbg("Write Atomic Err name=%s,wbytes=%d,needbytes=%d",atomic->data,bytes_writes,atomic->data_len);
	}
	return bytes_writes;
}

int facebook_write_from(void *file_handle,facebook_from *from)
{
	if(from){
		facebook_write_atomic(file_handle,&from->id);
		facebook_write_atomic(file_handle,&from->name);
	}
	return 0;
}

int facebook_write_comments(void *file_handle,facebook_comment *comments)
{
	if(comments){
		facebook_write_atomic(file_handle,&comments->id);
		facebook_write_from(file_handle,&comments->from);
		facebook_write_atomic(file_handle,&comments->message);
		facebook_write_atomic(file_handle,&comments->created_time);
	}
	return 0;
}

int facebook_write_images(void *file_handle,facebook_picture *images)
{
	int i=0;
	for(i=0;i<5;i++){
		if(images+i){
			facebook_dbg("this image is 0x%x",images+i);
			facebook_write_atomic(file_handle,&((images+i)->source));
		}
	}
	return 0;
}

int facebook_write_albumsentry(void *file_handle,unsigned int offset,facebook_album *album)
{
	facebook_album tmp_album;
	int entry_bytes=sizeof(facebook_album);
	memcpy(&tmp_album,album,sizeof(facebook_album));
	
	facebook_reset_albumsentry(&tmp_album,offset+entry_bytes);
	
	if(fseek((FILE*)file_handle, offset, SEEK_SET)!=0)
		facebook_dbg("Seek Error! Offset=0x%x",offset);
	
	if(fwrite(&tmp_album, sizeof(facebook_album), 1,(FILE*)file_handle)!=1){
		facebook_dbg("Write Entry Error!");
	}
	
	facebook_write_atomic(file_handle,&album->id);
	
	facebook_write_from(file_handle,&album->from);
	
	facebook_write_atomic(file_handle,&album->name);
	facebook_write_atomic(file_handle,&album->description);
	facebook_write_atomic(file_handle,&album->link);
	facebook_write_atomic(file_handle,&album->cover_photo);
	facebook_write_atomic(file_handle,&album->privacy);
	facebook_write_atomic(file_handle,&album->type);
	facebook_write_atomic(file_handle,&album->created_time);
	facebook_write_atomic(file_handle,&album->updated_time);

	facebook_write_comments(file_handle,&album->comments);
	return 0;	
}

int facebook_write_photoentry(void *file_handle,unsigned int offset,facebook_photo *photo)
{
	facebook_photo tmp_photo;
	int entry_bytes=sizeof(facebook_photo);
	memcpy(&tmp_photo,photo,sizeof(facebook_photo));
	
	facebook_reset_photoentry(&tmp_photo,offset+entry_bytes);
	
	if(fseek((FILE*)file_handle, offset, SEEK_SET)!=0)
		facebook_dbg("Seek Error! Offset=0x%x",offset);
	
	if(fwrite(&tmp_photo, sizeof(facebook_photo), 1,(FILE*)file_handle)!=1){
		facebook_dbg("Write Entry Error!");
	}
	
	facebook_write_atomic(file_handle,&photo->id);
	
	facebook_write_from(file_handle,&photo->from);
	
	facebook_write_atomic(file_handle,&photo->name);
	facebook_write_atomic(file_handle,&photo->picture);
	facebook_write_atomic(file_handle,&photo->source);
	
	facebook_write_images(file_handle,photo->images);
	
	facebook_write_atomic(file_handle,&photo->link);
	facebook_write_atomic(file_handle,&photo->icon);
	facebook_write_atomic(file_handle,&photo->created_time);
	facebook_write_atomic(file_handle,&photo->updated_time);

	facebook_write_comments(file_handle,&photo->comments);
	return 0;	
}

int facebook_write_memberentry(void *file_handle,unsigned int offset,facebook_member *member)
{
	facebook_member tmp_member;
	int entry_bytes=sizeof(facebook_member);
	memcpy(&tmp_member,member,sizeof(facebook_member));
	
	facebook_reset_memberentry(&tmp_member,offset+entry_bytes);
	
	if(fseek((FILE*)file_handle, offset, SEEK_SET)!=0)
		facebook_dbg("Seek Error! Offset=0x%x",offset);
	
	if(fwrite(&tmp_member, sizeof(facebook_member), 1,(FILE*)file_handle)!=1){
		facebook_dbg("Write Entry Error!");
	}
	facebook_write_atomic(file_handle,&member->id);
	facebook_write_atomic(file_handle,&member->name);
	return 0;	
}

int facebook_write_albumsfeed(void *file_handle,unsigned int offset,facebook_photoalbums *albums)
{
	int i=0;
	unsigned int entry_offset=0;
	int feed_bytes=sizeof(facebook_photoalbums);
	facebook_photoalbums tmp_albums;

	facebook_dbg("Feed Size=%d\n",feed_bytes);
	memcpy(&tmp_albums,albums,feed_bytes);
	if(fseek((FILE*)file_handle, offset, SEEK_SET)!=0)
		facebook_dbg("Seek Error! Offset=0x%x",offset);
	
	if(fwrite(&tmp_albums, sizeof(facebook_photoalbums), 1,(FILE*)file_handle)!=1){
		facebook_dbg("Write Albums Error!");
	}
	
	for(i=0;i<albums->album_num;i++){
		entry_offset = ftell((FILE*)file_handle);
		facebook_write_albumsentry(file_handle,entry_offset,albums->album_entry+i);
	}
	return 0;
}

int facebook_write_albumfeed(void *file_handle,unsigned int offset,facebook_album *album)
{
	int i=0;
	unsigned int entry_offset=0;
	
	for(i=0;i<album->count;i++){
		entry_offset = ftell((FILE*)file_handle);
		facebook_dbg("this photo is 0x%x",album->photo_entry+i);
		facebook_write_photoentry(file_handle,entry_offset,album->photo_entry+i);
	}
	return 0;
}

int facebook_write_contactsfeed(void *file_handle,unsigned int offset,facebook_contact *contacts)
{
	int i=0;
	unsigned int entry_offset=0;
	int feed_bytes=sizeof(facebook_contact);
	facebook_contact tmp_contacts;

	facebook_dbg("Feed Size=%d\n",feed_bytes);
	memcpy(&tmp_contacts,contacts,feed_bytes);
	if(fseek((FILE*)file_handle, offset, SEEK_SET)!=0)
		facebook_dbg("Seek Error! Offset=0x%x",offset);
	
	if(fwrite(&tmp_contacts, sizeof(facebook_contact), 1,(FILE*)file_handle)!=1){
		facebook_dbg("Write Contacts Error!");
	}
	
	for(i=0;i<contacts->contact_num;i++){
		entry_offset = ftell((FILE*)file_handle);
		facebook_write_memberentry(file_handle,entry_offset,contacts->member_entry+i);
	}
	return 0;
}

int facebook_read_atomic(void *file_handle,facebook_atomic *atomic)
{
	char *tmp_data=NULL;
	int bytes_read=0;
	int rtn=0;
	if(atomic->data && atomic->data_len){
		if(fseek((FILE*)file_handle, (long)atomic->data, SEEK_SET)!=0){
			facebook_dbg("Seek Error!,Offset = 0x%x",atomic->data);
			rtn = -1;
			goto facebook_read_atomic_out;
		}
		tmp_data = (char*)facebook_malloc(atomic->data_len);
		if(tmp_data==NULL){
			facebook_dbg("Malloc Falied,Atomic DataLen=%d",atomic->data_len);
			rtn = -1;
			goto facebook_read_atomic_out;
		}
		if(fread((unsigned char*)tmp_data, sizeof(unsigned char),atomic->data_len,(FILE*)file_handle)!=atomic->data_len){
			facebook_dbg("Read Data Error!");
			rtn = -1;
			goto facebook_read_atomic_out;
		}	
	}
facebook_read_atomic_out:
	atomic->data = tmp_data;///< save the pointer of the data to the atomic
	return rtn;
}

int facebook_read_from(void *file_handle,facebook_from * from)
{
	if(from){
		facebook_read_atomic(file_handle,&from->id);
		facebook_read_atomic(file_handle,&from->name);
	}
	return 0;
}

int facebook_read_comments(void *file_handle,facebook_comment * comments)
{
	if(comments){
		facebook_read_atomic(file_handle,&comments->id);
		facebook_read_from(file_handle,&comments->from);
		facebook_read_atomic(file_handle,&comments->message);
		facebook_read_atomic(file_handle,&comments->created_time);
	}
	return 0;
}

int facebook_read_images(void *file_handle,facebook_picture *images)
{
	int i=0;
	for(i=0;i<5;i++){
		if(images+i)
			facebook_read_atomic(file_handle,&(images+i)->source);
	}
	return 0;
}

int facebook_read_albumsentry(void* file_handle,unsigned int offset,facebook_album * album)
{
	facebook_album tmp_album;
	unsigned int entry_bytes=sizeof(facebook_album);
	
	if(fseek((FILE*)file_handle, offset, SEEK_SET)!=0)
		facebook_dbg("Seek Error!Offset = 0x%x",offset);
	
	memset(&tmp_album,0,sizeof(facebook_album));
	if(fread(&tmp_album, sizeof(facebook_album),1,(FILE*)file_handle)!=1){
		facebook_dbg("Read Entry Error!");
		return -1;
	}

	facebook_dbg("Entry Bytes=0x%x",entry_bytes);
	facebook_read_atomic(file_handle,&tmp_album.id);
	
	facebook_read_from(file_handle,&tmp_album.from);
	
	facebook_read_atomic(file_handle,&tmp_album.name);
	facebook_read_atomic(file_handle,&tmp_album.description);
	facebook_read_atomic(file_handle,&tmp_album.link);
	facebook_read_atomic(file_handle,&tmp_album.cover_photo);
	facebook_read_atomic(file_handle,&tmp_album.privacy);
	facebook_read_atomic(file_handle,&tmp_album.type);
	facebook_read_atomic(file_handle,&tmp_album.created_time);
	facebook_read_atomic(file_handle,&tmp_album.updated_time);
	
	facebook_read_comments(file_handle,&tmp_album.comments);

	memcpy(album,&tmp_album,entry_bytes);
	return 0;
}

int facebook_read_photoentry(void* file_handle,unsigned int offset,facebook_photo * photo)
{
	facebook_photo tmp_photo;
	unsigned int entry_bytes=sizeof(facebook_photo);
	
	if(fseek((FILE*)file_handle, offset, SEEK_SET)!=0)
		facebook_dbg("Seek Error!Offset = 0x%x",offset);
	
	memset(&tmp_photo,0,sizeof(facebook_photo));
	if(fread(&tmp_photo, sizeof(facebook_photo),1,(FILE*)file_handle)!=1){
		facebook_dbg("Read Entry Error!");
		return -1;
	}

	facebook_dbg("Entry Bytes=0x%x",entry_bytes);

	facebook_read_atomic(file_handle,&tmp_photo.id);
	
	facebook_read_from(file_handle,&tmp_photo.from);
	
	facebook_read_atomic(file_handle,&tmp_photo.name);
	facebook_read_atomic(file_handle,&tmp_photo.picture);
	facebook_read_atomic(file_handle,&tmp_photo.source);
	
	facebook_read_images(file_handle,tmp_photo.images);
	
	facebook_read_atomic(file_handle,&tmp_photo.link);
	facebook_read_atomic(file_handle,&tmp_photo.icon);
	facebook_read_atomic(file_handle,&tmp_photo.created_time);
	facebook_read_atomic(file_handle,&tmp_photo.updated_time);
	
	facebook_read_comments(file_handle,&tmp_photo.comments);

	memcpy(photo,&tmp_photo,entry_bytes);
	return 0;
}

int facebook_read_memberentry(void* file_handle,unsigned int offset,facebook_member * member)
{
	facebook_member tmp_member;
	unsigned int entry_bytes=sizeof(facebook_member);
	
	if(fseek((FILE*)file_handle, offset, SEEK_SET)!=0)
		facebook_dbg("Seek Error!Offset = 0x%x",offset);
	
	memset(&tmp_member,0,sizeof(facebook_member));
	if(fread(&tmp_member, sizeof(facebook_member),1,(FILE*)file_handle)!=1){
		facebook_dbg("Read Entry Error!");
		return -1;
	}

	facebook_dbg("Entry Bytes=0x%x",entry_bytes);
	facebook_read_atomic(file_handle,&tmp_member.id);
	facebook_read_atomic(file_handle,&tmp_member.name);
	memcpy(member,&tmp_member,entry_bytes);
	return 0;
}

int facebook_read_albumsfeed(void * file_handle,unsigned int offset,facebook_photoalbums * albums)
{
	facebook_photoalbums tmp_albums;
	int feed_bytes=sizeof(facebook_photoalbums);
	if(fseek((FILE*)file_handle, offset, SEEK_SET)!=0)
		facebook_dbg("Seek Error!Offset = 0x%x",offset);
	
	memset(&tmp_albums,0,feed_bytes);
	if(fread(&tmp_albums, sizeof(facebook_photoalbums),1,(FILE*)file_handle)!=1){
		facebook_dbg("Read Feed Error!");
	}
	memcpy(albums,&tmp_albums,feed_bytes);
	return 0;
}

int facebook_read_contactsfeed(void * file_handle,unsigned int offset,facebook_contact* contacts)
{
	facebook_contact tmp_contacts;
	int feed_bytes=sizeof(facebook_contact);
	if(fseek((FILE*)file_handle, offset, SEEK_SET)!=0)
		facebook_dbg("Seek Error!Offset = 0x%x",offset);
	
	memset(&tmp_contacts,0,feed_bytes);
	if(fread(&tmp_contacts, sizeof(facebook_contact),1,(FILE*)file_handle)!=1){
		facebook_dbg("Read Feed Error!");
	}
	memcpy(contacts,&tmp_contacts,feed_bytes);
	return 0;
}

int facebook_printf_atomic(char* name,facebook_atomic *atomic_name)
{
	if(name!=NULL && atomic_name!=NULL){
		if(atomic_name->data!=NULL){
			facebook_dbg("%s=%s,len=%d",name,atomic_name->data,atomic_name->data_len);
		}
		else{
			facebook_dbg("the data is NULL");
		}
	}
	else{
		facebook_dbg("the atomic is NULL");
	}
	return 0;
}

int facebook_printf_from(facebook_from *from)
{
	if(from){
		facebook_printf_atomic("from id",&from->id);
		facebook_printf_atomic("from name",&from->name);
	}
	return 0;
}

int facebook_printf_comments(facebook_comment *comments)
{
	if(comments){
		facebook_printf_atomic("comments id",&comments->id);
		facebook_printf_from(&comments->from);
		facebook_printf_atomic("comments message",&comments->message);
		facebook_printf_atomic("comments created time",&comments->created_time);
	}
	return 0;
}

int facebook_printf_images(facebook_picture *images)
{
	int i=0;
	for(i=0;i<5;i++){
		if(images+i)
			facebook_printf_atomic("images source",&(images+i)->source);
	}
	return 0;
}

int facebook_printf_albumsentry(facebook_album *album)
{
	if(album){
		facebook_printf_atomic("album id",&album->id);
	
		facebook_printf_from(&album->from);
		
		facebook_printf_atomic("album name",&album->name);
		facebook_printf_atomic("album description",&album->description);
		facebook_printf_atomic("album link",&album->link);
		facebook_printf_atomic("album cover photo",&album->cover_photo);
		facebook_printf_atomic("album privacy",&album->privacy);
		facebook_printf_atomic("album type",&album->type);
		facebook_printf_atomic("album created time",&album->created_time);
		facebook_printf_atomic("album updated time",&album->updated_time);
		
		facebook_printf_comments(&album->comments);
	}
	return 0;
}

int facebook_printf_photoentry(facebook_photo *photo)
{
	if(photo){
		facebook_printf_atomic("photo id",&photo->id);
	
		facebook_printf_from(&photo->from);
		
		facebook_printf_atomic("photo name",&photo->name);
		facebook_printf_atomic("photo picture",&photo->picture);
		facebook_printf_atomic("photo source",&photo->source);
		
		facebook_printf_images(photo->images);
		
		facebook_printf_atomic("photo link",&photo->link);
		facebook_printf_atomic("photo icon",&photo->icon);
		facebook_printf_atomic("photo created time",&photo->created_time);
		facebook_printf_atomic("photo updated time",&photo->updated_time);
		
		facebook_printf_comments(&photo->comments);
	}
	return 0;
}

int facebook_printf_memberentry(facebook_member *member)
{
	if(member){
		facebook_printf_atomic("member id",&member->id);
		facebook_printf_atomic("member name",&member->name);
	}
	return 0;
}

int facebook_printf_albumsfeed(facebook_photoalbums *feed_albums)
{
	if(feed_albums){
		facebook_dbg("albums num=%d",feed_albums->album_num);
	}
	return 0;
}

int facebook_printf_contactsfeed(facebook_contact *feed_contacts)
{
	if(feed_contacts){
		facebook_dbg("contacts num=%d",feed_contacts->contact_num);
	}
	return 0;
}
