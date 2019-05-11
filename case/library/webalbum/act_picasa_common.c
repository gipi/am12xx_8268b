#include "act_picasa_common.h"

extern int  picasa_fill_atomic(xmlChar * content,picasaweb_data_atomic_t *atomic_name);
extern int picasa_free_atomic(picasaweb_data_atomic_t *atomic_name);


picasaweb_schema_item_t schema_items[]=
{
	///< see http://code.google.com/intl/zh-CN/apis/gdata/docs/2.0/reference.html#alt
	{FEED_ALL,		"/atom:feed"},
	{FEED_TITLE,		"/atom:feed/atom:title"},
	{FEED_ID,		"/atom:feed/atom:id"},
	{FEED_HTML_LINK,	"/atom:feed/atom:link[@rel='alternate'][@type='text/html']/@href"},
	{FEED_DESCRIPTION,	"/atom:feed/atom:subtitle"},
	{FEED_LANGUAGE,		"/atom:feed/@xml:lang"},
	{FEED_COPYRIGHT,	"/atom:feed/atom:rights"},
	{FEED_AUTHOR_NAME,	"/atom:feed/atom:author/atom:name"},
	{FEED_AUTHOR_EMAIL,	"/atom:feed/atom:author/atom:email"},
	{FEED_LAST_UPDATE_DATE,	"/atom:feed/atom:updated"},
	{FEED_CATEGORY,		"/atom:feed/atom:category/@term"},
	{FEED_CATEGORY_SCHEME,	"/atom:feed/atom:category/@scheme"},
	{FEED_GENERATOR,	"/atom:feed/atom:generator"},
	{FEED_GENERATOR_URL,	"/atom:feed/atom:generator/@uri"},
	{FEED_ICON,		"/atom:feed/atom:icon"},
	{FEED_LOGO,		"/atom:feed/atom:logo"},

	{OPENSEARCH_RESULT,	"/atom:feed/openSearch:totalResults"},
	{OPENSEARCH_START_IDX,	"/atom:feed/openSearch:startIndex"},
	{OPENSEARCH_NUM_PER_PAGE,"/atom:feed/openSearch:itemsPerPage"},
	
	{ENTRY_ALL,		"/atom:feed/atom:entry"},
	{ENTRY_ID,		"/atom:feed/atom:entry/atom:id"},
	{ENTRY_TITLE,		"/atom:feed/atom:entry/atom:title"},
	{ENTRY_LINK,		"/atom:feed/atom:entry/atom:link"},
	{ENTRY_SUMMARY,		"/atom:feed/atom:entry/atom:summary"},
	{ENTRY_CONTENT,		"/atom:feed/atom:entry/atom:content"},///<If no content element, contain at least one <link rel="alternate"> element
	{ENTRY_AUTHOR_NAME,	"/atom:eed/atom:entry/atom:author/atom:name"},
	{ENTRY_AUTHOR_EMAIL,	"/atom:feed/atom:entry/atom:author/email"},
	{ENTRY_CATEGORY,	"/atom:feed/atom:entry/atom:category/@term"},
	{ENTRY_CATEGORY_SCHEME,	"/atom:feed/atom:entry/atom:category/@scheme"},
	{ENTRY_PUBLIC_DATE,	"/atom:feed/atom:entry/atom:published"},
	{ENTRY_UPDATE_DATE,	"/atom:feed/atom:entry/atom:updated"},

	{PHOTO_ID,		"/atom:feed/atom:entry/gphoto:id"},
};



static char kind_value[Kind_MAX][8]={
	"album",
	"photo",
	"comment",
	"tag",
	"user"
};

static char query_parameter[Query_MAX][16]={
	"access",
	"alt",
	"bbox",
	"fields",
	"imgmax",
	"kind",
	"l",
	"max-results",
	"prettyprint",
	"q",
	"start-index",
	"tag",
	"thumbsize",
};

char picasaweb_gdata_version[32]="GData-Version: 2.0";
char picasaweb_gdata_source[32]="Actions-WebAlbum-1.00";

inline void __print_xmlChar(char* name,xmlChar* xml_char)
{
	if(xml_char){
		printf("%s=%s\n",name,xml_char);
	}
	else
		printf("%s=NULL\n",name);
}

int _test_save_to_file(char* databuf,unsigned int datalen,char *filename)
{
#ifdef PICASA_WEBALBUM_DEBUG
	FILE *fp=NULL;
	unsigned int len_write=0;
	if(filename==NULL)
		return 0;
	fp = fopen(filename,"wb+");
	if(fp==NULL){
		printf("%s,%d:Open File Error=%s\n",__FILE__,__LINE__,filename);
		return 0;
	}
	len_write=fwrite(databuf,sizeof(char),datalen,fp);
	if(len_write!=datalen)
		printf("%s,%d:Write File Error!\n",__FILE__,__LINE__);
	else
		printf("Save File OK filename=%s\n",filename);
	printf("Len_Write=%d,data_len=%d\n",len_write,datalen);
	fclose(fp);
#endif
	return 1;
}


char * picasa_com_get_kind_value(PicasawebKindValue_e kind)
{
	if(kind>=Query_MAX)
		return NULL;
	else
		return kind_value[kind];
}

char * picasa_com_get_query_para(PicasawebQueryPara_e query)
{
	if(query>=Query_MAX)
		return NULL;
	else
		return query_parameter[query];
}

int __picasaweb_data_write_init(picasaweb_data_write_t *data,PicasawebDataWrite_e data_type,void* fp)
{
	memset(data,0,sizeof(picasaweb_data_write_t));
	if(data_type==Picasaweb_data_write_file){
		data->file_handle = fp;
		data->data_type = Picasaweb_data_write_file;
	}
	else
		data->data_type = Picasaweb_data_write_buffer;
	return 0;
}

int __picasaweb_data_write_free(picasaweb_data_write_t *data)
{
	if(data->data_head && data->data_type==Picasaweb_data_write_buffer){
		picasa_free(data->data_head);
		data->data_head=NULL;
		data->data_cur=NULL;
		data->data_len = 0;
		data->data_used = 0;
	}
	return 0;
}



/**
*@brief regist the namespace which will be used in parsing Atom
**/
int picasa_regist_namespace(xmlXPathContextPtr contex)
{
	if(xmlXPathRegisterNs(contex, BAD_CAST "atom", BAD_CAST "http://www.w3.org/2005/Atom")==-1)goto REGIST_ERROR;
	if(xmlXPathRegisterNs(contex, BAD_CAST "app", BAD_CAST "http://www.w3.org/2007/app")==-1)goto REGIST_ERROR;
	if(xmlXPathRegisterNs(contex, BAD_CAST "gphoto", BAD_CAST "http://schemas.google.com/photos/2007")==-1)goto REGIST_ERROR;
	if(xmlXPathRegisterNs(contex, BAD_CAST "media", BAD_CAST "http://search.yahoo.com/mrss/")==-1)goto REGIST_ERROR;			
	if(xmlXPathRegisterNs(contex, BAD_CAST "openSearch", BAD_CAST "http://a9.com/-/spec/opensearch/1.1/")==-1)goto REGIST_ERROR;
	if(xmlXPathRegisterNs(contex, BAD_CAST "gd", BAD_CAST "http://schemas.google.com/g/2005")==-1)goto REGIST_ERROR;
	if(xmlXPathRegisterNs(contex, BAD_CAST "georss", BAD_CAST "http://www.georss.org/georss")==-1)goto REGIST_ERROR;	
	if(xmlXPathRegisterNs(contex, BAD_CAST "gml", BAD_CAST "http://www.opengis.net/gml")==-1)goto REGIST_ERROR;	
	if(xmlXPathRegisterNs(contex, BAD_CAST "exif", BAD_CAST "http://schemas.google.com/photos/exif/2007")==-1)goto REGIST_ERROR;	
	return 0;

REGIST_ERROR:
	printf("%s,%d: Error Regist namespace Error!\n",__FILE__,__LINE__);
	return -1;
}


inline void __print_node(xmlNodePtr node)
{
	int idx=0;
	xmlChar * content=NULL;
	content = xmlNodeGetContent(node);
	if(node->name){
		idx = picasa_get_tagindex((char*)node->name);
		printf("[hash idx=%d]",idx);
		if(node->ns){
			printf("{ns=%s}",node->ns->href);
		}
		__print_xmlChar((char*)node->name,content);
	}
	if(content)
		xmlFree(content);
}

/**
@brief get the name space id which is defined in the picasaweb_entry_node_e
**/
inline int __picasa_get_namespace_id(xmlNodePtr node)
{
	int idx=-1;
	if(node->ns && node->ns->href){
		idx = picasa_get_tagindex((char*)node->ns->href);
	}
	return idx;
}

int __picasa_fill_node_attr(xmlNodePtr node,char* propname,picasaweb_data_atomic_t *atomic_name)
{
	xmlNodePtr childnode=NULL;
	xmlChar* tmpprop= xmlGetProp(node,(xmlChar*)propname);
	if(tmpprop){
		picasa_fill_atomic(tmpprop,atomic_name);
		//printf("[prop=%s]=%s\n",propname,tmpprop);	
		xmlFree(tmpprop);
		return 0;
	}
	else{
		//printf("fill node proname is not exit=%s\n",propname);
		return -1;
	}
}

int __picasa_fill_link_attr(xmlNodePtr node,picasaweb_entry_t *entry)
{
	int rtn = 0;
	int tag_idx=-1;
	xmlChar* tmpprop= xmlGetProp(node,(xmlChar*)"rel");
	if(tmpprop){
		if(strcmp((char*)tmpprop,"alternate")==0){
			
		}
		else if(strcmp((char*)tmpprop,"self")==0){
		}
		else if(strcmp((char*)tmpprop,"edit")==0){
			__picasa_fill_node_attr(node,"rel",&(entry->link_edit.rel));
			__picasa_fill_node_attr(node,"type",&(entry->link_edit.type));
			__picasa_fill_node_attr(node,"href",&(entry->link_edit.href));
		}
		else if(strcmp((char*)tmpprop,"http://schemas.google.com/g/2005#feed")==0){
			__picasa_fill_node_attr(node,"rel",&(entry->link_feed.rel));
			__picasa_fill_node_attr(node,"type",&(entry->link_feed.type));
			__picasa_fill_node_attr(node,"href",&(entry->link_feed.href));
		}
		xmlFree(tmpprop);
	}
	else
		rtn = -1;
	return rtn;
}

int picasa_entry_enter_childnode(picasaweb_entry_t *entry,xmlNodePtr node)
{
	xmlNodePtr childnode=NULL;
	if(node->children!=NULL){
		childnode = node->children;
		while(childnode){
			picasa_fill_entry_element(entry,childnode);
			childnode = childnode->next;
		}
	}
	return 0;
}

int picasa_feed_enter_childnode(picasaweb_feed_t *feed,xmlNodePtr node)
{
	xmlNodePtr childnode=NULL;
	if(node->children!=NULL){
		childnode = node->children;
		while(childnode){
			picasa_fill_feed_element(feed,childnode);
			childnode = childnode->next;
		}
	}
	return 0;
}


/**
@brief check the cache_dir, if the dir is not exist, try to create
@param[in] dir_path : the path of the dir
@return 
	0 : succeed
	others : failed
**/
int __picasa_create_dir(char * dir_path)
{
	int rtn=0;
	rtn = access(dir_path,W_OK);
	if(rtn!=0){
		rtn = mkdir(dir_path,0777);
		if(rtn!=0){
			printf("%s,%d:Make Dir Failed=%s\n",__FILE__,__LINE__,dir_path);
		}
	}
	return rtn;	
}
#if 1
///*******File Operations ************////
void * picasa_fopen(char *path, char *mode){
	return (void*)fopen(path, mode);
}

int picasa_fclose(void *fp){
	return fclose((FILE*)fp);
}

long picasa_fread(void *fp, unsigned char *ptr, unsigned long nbytes){
	return fread(ptr, sizeof(unsigned char),nbytes,(FILE*)fp);
}

long picasa_fwrite(void *fp, unsigned char *ptr, unsigned long nbytes){
	return fwrite(ptr, sizeof(unsigned char), nbytes,(FILE*)fp);
}

long picasa_fseek_set(void *fp, long offset){
	return fseek((FILE*)fp, offset, SEEK_SET);
}

long picasa_fseek_cur(void *fp, long offset){
	return fseek((FILE*)fp, offset, SEEK_CUR);
}

long picasa_fseek_end(void *fp, long offset){
	return fseek((FILE*)fp, offset, SEEK_END);
}

long picasa_ftell(void *fp){
	return ftell((FILE*)fp);
}

int picasa_fflush(void *fp)
{
	int fd;
	fflush((FILE *)fp);
	fd = fileno((FILE *)fp);
	if(fsync(fd)==-1){
		printf("%s,%d: Fflush Error!\n",__FILE__,__LINE__);
		return -1;
	}
	return 0;
}


int picasa_fremove(char* file)
{
	int err=0;
	if(file){
		err = unlink(file);
		if(err != 0){
			picasa_err("Remove Error file=%s, reason=%s\n",file,strerror(errno));
			return -1;
		}
		return 0;
	}
	return -1;
}
#endif


#if 1
void __test_tell_filepos(void *file_handle)
{
	long pos=0;
	pos = picasa_ftell(file_handle);
	picasa_info("CurPos===0x%x\n",pos);
}

int picasa_get_ini_albumsname(picasaweb_gdata_t *gdata,char* cache_dir,char*filename,char* namebuf,int len)
{
	int dir_len = 0;
	if(cache_dir)
		dir_len = strlen(cache_dir);
	else
		return -1;
	if( __picasa_create_dir(cache_dir)!=0) // if the dir is not exist, try to create it first
		return -1;
	if(filename){
		if(strlen(filename)+4+dir_len>len){
			picasa_err("Sorry The name buf is too short,len=%d, needed %d,",len,strlen(filename)+4+dir_len);
			return -1;
		}
	}
	else{
		if(gdata->login_name_len+4+dir_len>len){
			picasa_err("Sorry The name buf is too short,len=%d, needed %d,",len,gdata->login_name_len+4);
			return -1;
		}
	}
	memset(namebuf,0, len);
	if(*(cache_dir+dir_len-1)!='/'){
		if(filename)
			sprintf(namebuf,"%s/%s.ini",cache_dir,filename);
		else
			sprintf(namebuf,"%s/%s.ini",cache_dir,gdata->login_name);
	}
	else{
		if(filename)
			sprintf(namebuf,"%s%s.ini",cache_dir,filename);
		else
			sprintf(namebuf,"%s%s.ini",cache_dir,gdata->login_name);
	}
	picasa_info("Name Buf = %s",namebuf);
	return 0;
	
}

int picasa_get_ini_contactname(picasaweb_gdata_t *gdata,char* cache_dir,char* namebuf,int len)
{
	int dir_len = 0;
	if(cache_dir)
		dir_len = strlen(cache_dir);
	else
		return -1;
	if( __picasa_create_dir(cache_dir)!=0) // if the dir is not exist, try to create it first
		return -1;
	
	if(8+dir_len>len){
		picasa_err("Sorry The name buf is too short,len=%d, needed %d,",len,8+4);
		return -1;
	}
	else{
		memset(namebuf,0, len);
		if(*(cache_dir+dir_len-1)!='/')
			sprintf(namebuf,"%s/contacts.ini",cache_dir);
		else
			sprintf(namebuf,"%scontacts.ini",cache_dir);
		picasa_info("Name Buf = %s",namebuf);
		return 0;
	}
}


int picasa_get_ini_albumname(picasaweb_feed_t *feed_albums,int which_album,char* cache_dir,char* namebuf,int len)
{
	picasaweb_entry_t * entry=NULL;
	int dir_len = 0;
	if(cache_dir)
		dir_len = strlen(cache_dir);
	else
		return -1;
	
	if( __picasa_create_dir(cache_dir)!=0) // if the dir is not exist, try to create it first
		return -1;
	entry = feed_albums->entry+which_album;
	if(entry->albumid.data_len+4+dir_len>len){
		picasa_err("Sorry The name buf is too short,len=%d, needed %d,",len,entry->albumid.data_len+4+dir_len);
		return -1;
	}
	else{
		memset(namebuf,0, len);
		if(*(cache_dir+dir_len-1)!='/')
			sprintf(namebuf,"%s/%s.ini",cache_dir,entry->id.data);
		else
			sprintf(namebuf,"%s%s.ini",cache_dir,entry->id.data);
		picasa_info("Name Buf = %s",namebuf);
		return 0;
	}

}
#if 1
int picasa_free_group_cont_element(picasaweb_media_content_attr_t *group_cont)
{
	picasa_free_atomic(&(group_cont->url));
	picasa_free_atomic(&(group_cont->type));
	picasa_free_atomic(&(group_cont->medium));
	picasa_free_atomic(&(group_cont->height));
	picasa_free_atomic(&(group_cont->width));
	picasa_free_atomic(&(group_cont->filesize));
	return 0;
}

int picasa_free_group_thumbnail_element(picasaweb_media_thumbnail_attr_t *group_thumb)
{
	picasa_free_atomic(&(group_thumb->url));
	picasa_free_atomic(&(group_thumb->height));
	picasa_free_atomic(&(group_thumb->width));
	return 0;
}

int picasa_free_group_element(picasaweb_media_group_t *group)
{
	picasa_free_group_cont_element(&group->content);
	picasa_free_atomic(&(group->credit));
	picasa_free_atomic(&(group->description));
	picasa_free_atomic(&(group->keywords));
	picasa_free_group_thumbnail_element(&group->thumbnail);
	picasa_free_atomic(&(group->title));
	return 0;
}

int picasa_free_author_element(picasaweb_author_t *author)
{
	picasa_free_atomic(&(author->name));
	picasa_free_atomic(&(author->email));
	picasa_free_atomic(&(author->uri));
	return 0;
}

int picasa_free_tags_element(picasaweb_exif_tags_t *tags)
{
	picasa_free_atomic(&(tags->distance));
	picasa_free_atomic(&(tags->exposure));
	picasa_free_atomic(&(tags->flash));
	picasa_free_atomic(&(tags->focallength));
	picasa_free_atomic(&(tags->fstop));
	picasa_free_atomic(&(tags->img_uniqueID));
	picasa_free_atomic(&(tags->iso));
	picasa_free_atomic(&(tags->make));
	picasa_free_atomic(&(tags->model));
	picasa_free_atomic(&(tags->time));	
	return 0;
}

int picasa_free_where_point_element(picasaweb_Point_t *point)
{
	picasa_free_atomic(&(point->pos));	
	return 0;
}

int picasa_free_where_element(picasaweb_where_t * where)
{
	picasa_free_where_point_element(&(where->point));
	return 0;
}

int picasa_free_link_element(picasaweb_link_attr_t *link)
{
	picasa_free_atomic(&(link->rel));	
	picasa_free_atomic(&(link->type));
	picasa_free_atomic(&(link->href));
	return 0;
}
#endif

#if 1
int picasa_reset_atomic(picasaweb_data_atomic_t *atomic,unsigned int *offset)
{
	if(atomic->data!=NULL){
		atomic->data = (char*)*offset;
		*offset +=atomic->data_len;
	}
	return 0;
}

int picasa_reset_author(picasaweb_author_t *author,unsigned int *offset)
{
	picasa_reset_atomic(&author->name,offset);
	picasa_reset_atomic(&author->email,offset);
	picasa_reset_atomic(&author->uri,offset);
	return 0;
}

int picasa_reset_exif_tags(picasaweb_exif_tags_t *tags,unsigned int* offset)
{
	picasa_reset_atomic(&tags->distance,offset);
	picasa_reset_atomic(&tags->exposure,offset);
	picasa_reset_atomic(&tags->flash,offset);
	picasa_reset_atomic(&tags->focallength,offset);
	picasa_reset_atomic(&tags->fstop,offset);
	picasa_reset_atomic(&tags->img_uniqueID,offset);
	picasa_reset_atomic(&tags->iso,offset);
	picasa_reset_atomic(&tags->make,offset);
	picasa_reset_atomic(&tags->model,offset);
	picasa_reset_atomic(&tags->time,offset);
	return 0;
}

int picasa_reset_media_content_attr(picasaweb_media_content_attr_t *content_attr,unsigned int *offset)
{
	picasa_reset_atomic(&content_attr->url,offset);
	picasa_reset_atomic(&content_attr->type,offset);
	picasa_reset_atomic(&content_attr->medium,offset);
	picasa_reset_atomic(&content_attr->height,offset);
	picasa_reset_atomic(&content_attr->width,offset);
	picasa_reset_atomic(&content_attr->filesize,offset);
	return 0;
}

int picasa_reset_thumbnail_attr(picasaweb_media_thumbnail_attr_t *thumbnail_attr,unsigned int *offset)
{
	picasa_reset_atomic(&thumbnail_attr->url,offset);
	picasa_reset_atomic(&thumbnail_attr->height,offset);
	picasa_reset_atomic(&thumbnail_attr->width,offset);
	return 0;
}

int picasa_reset_media_group(picasaweb_media_group_t * group,unsigned int *offset)
{
	picasa_reset_media_content_attr(&group->content,offset);
	picasa_reset_atomic(&group->credit,offset);
	picasa_reset_atomic(&group->description,offset);
	picasa_reset_atomic(&group->keywords,offset);
	picasa_reset_thumbnail_attr(&group->thumbnail,offset);
	picasa_reset_atomic(&group->title,offset);
	return 0;
}

int picasa_reset_point(picasaweb_Point_t *point,unsigned int * offset)
{
	picasa_reset_atomic(&point->pos,offset);
	return 0;
}

int picasa_reset_where(picasaweb_where_t *where,unsigned int *offset)
{
	picasa_reset_point(&where->point,offset);
	return 0;
}

int picasa_reset_link_attr(picasaweb_link_attr_t *link_attr,unsigned int *offset)
{
	picasa_reset_atomic(&link_attr->rel,offset);
	picasa_reset_atomic(&link_attr->type,offset);
	picasa_reset_atomic(&link_attr->href,offset);
	return 0;
}

int picasa_reset_feed(picasaweb_feed_t *feed,unsigned int offset)
{
	int bitmap_bytes = (feed->entry_num+7)/8;
	unsigned int tmp_offset=offset;
	unsigned int bitmap_offset=0;
	picasa_info("Reset TmpOffset Start =0x%x",tmp_offset);
	picasa_reset_atomic(&feed->attr_etag,&tmp_offset);
	picasa_reset_atomic(&feed->albumid,&tmp_offset);
	picasa_reset_atomic(&feed->id,&tmp_offset);
	picasa_reset_atomic(&feed->updated,&tmp_offset);
	picasa_reset_atomic(&feed->title,&tmp_offset);
	picasa_reset_atomic(&feed->subtitle,&tmp_offset);
	picasa_reset_atomic(&feed->icon,&tmp_offset);
	
	picasa_reset_author(&feed->author,&tmp_offset);
	
	picasa_reset_atomic(&feed->totalResults,&tmp_offset);
	picasa_reset_atomic(&feed->startIndex,&tmp_offset);
	picasa_reset_atomic(&feed->itemsPerPage,&tmp_offset);
	picasa_reset_atomic(&feed->maxPhotoPerAlbum,&tmp_offset);
	picasa_reset_atomic(&feed->nickname,&tmp_offset);
	picasa_reset_atomic(&feed->quotacurrent,&tmp_offset);
	picasa_reset_atomic(&feed->quotalimit,&tmp_offset);
	picasa_reset_atomic(&feed->thumbnail,&tmp_offset);
	picasa_reset_atomic(&feed->user,&tmp_offset);
	picasa_reset_atomic(&feed->access,&tmp_offset);
	picasa_reset_atomic(&feed->bytesUsed,&tmp_offset);
	picasa_reset_atomic(&feed->location,&tmp_offset);
	picasa_reset_atomic(&feed->numphotos,&tmp_offset);
	picasa_reset_atomic(&feed->numphotosremaining,&tmp_offset);
	
	picasa_reset_atomic(&feed->checksum,&tmp_offset);
	picasa_reset_atomic(&feed->commentCount,&tmp_offset);
	picasa_reset_atomic(&feed->height,&tmp_offset);
	picasa_reset_atomic(&feed->width,&tmp_offset);
	picasa_reset_atomic(&feed->rotation,&tmp_offset);
	picasa_reset_atomic(&feed->size,&tmp_offset);
	picasa_reset_atomic(&feed->timestamp,&tmp_offset);
	picasa_reset_atomic(&feed->videostatus,&tmp_offset);
	picasa_reset_atomic(&feed->photoid,&tmp_offset);
	picasa_reset_atomic(&feed->weight,&tmp_offset);

	bitmap_offset = tmp_offset + feed->weight.data_len;
	picasa_info("Bitmap Offset = 0x%x",bitmap_offset);
	
	feed->download_browsebitmap = (unsigned char*)bitmap_offset;
	feed->download_thumbnailbitmap = (unsigned char*)(bitmap_offset+bitmap_bytes);
	feed->entry = NULL;
	
	picasa_info("Reset TmpOffset End =0x%x",tmp_offset);
	return 0;
}

int picasa_reset_entry(picasaweb_entry_t *entry,unsigned int offset)
{
	unsigned int tmp_offset=offset;
	entry->child_feed =NULL;
	picasa_reset_atomic(&entry->attr_etag,&tmp_offset);
	picasa_reset_exif_tags(&entry->tags,&tmp_offset);
	picasa_reset_atomic(&entry->albumid,&tmp_offset);
	picasa_reset_atomic(&entry->id,&tmp_offset);
	picasa_reset_atomic(&entry->published,&tmp_offset);
	picasa_reset_atomic(&entry->updated,&tmp_offset);
	picasa_reset_atomic(&entry->edited,&tmp_offset);
	picasa_reset_atomic(&entry->rights,&tmp_offset);
	picasa_reset_atomic(&entry->summary,&tmp_offset);
	picasa_reset_atomic(&entry->title,&tmp_offset);
	
	picasa_reset_author(&entry->author,&tmp_offset);
	picasa_reset_media_group(&entry->group,&tmp_offset);
	picasa_reset_where(&entry->where,&tmp_offset);

	picasa_reset_link_attr(&entry->link_edit,&tmp_offset);
	picasa_reset_link_attr(&entry->link_feed,&tmp_offset);
	
	picasa_reset_atomic(&entry->maxPhotoPerAlbum,&tmp_offset);
	picasa_reset_atomic(&entry->nickname,&tmp_offset);
	picasa_reset_atomic(&entry->quotacurrent,&tmp_offset);
	picasa_reset_atomic(&entry->quotalimit,&tmp_offset);
	picasa_reset_atomic(&entry->thumbnail,&tmp_offset);
	picasa_reset_atomic(&entry->user,&tmp_offset);
	
	picasa_reset_atomic(&entry->access,&tmp_offset);
	picasa_reset_atomic(&entry->bytesUsed,&tmp_offset);
	picasa_reset_atomic(&entry->location,&tmp_offset);
	picasa_reset_atomic(&entry->numphotos,&tmp_offset);
	picasa_reset_atomic(&entry->numphotosremaining,&tmp_offset);
	
	picasa_reset_atomic(&entry->checksum,&tmp_offset);
	picasa_reset_atomic(&entry->commentCount,&tmp_offset);
	picasa_reset_atomic(&entry->height,&tmp_offset);
	picasa_reset_atomic(&entry->width,&tmp_offset);
	picasa_reset_atomic(&entry->rotation,&tmp_offset);
	picasa_reset_atomic(&entry->size,&tmp_offset);
	picasa_reset_atomic(&entry->timestamp,&tmp_offset);
	picasa_reset_atomic(&entry->videostatus,&tmp_offset);

	picasa_reset_atomic(&entry->photoid,&tmp_offset);
	picasa_reset_atomic(&entry->weight,&tmp_offset);
	
	picasa_reset_atomic(&entry->albumtitle,&tmp_offset);
	picasa_reset_atomic(&entry->albumdesc,&tmp_offset);
	picasa_reset_atomic(&entry->snippet,&tmp_offset);
	picasa_reset_atomic(&entry->snippettype,&tmp_offset);
	picasa_reset_atomic(&entry->truncated,&tmp_offset);
	return 0;
}

#endif

#if 1
int picasa_write_atomic(void *file_handle,picasaweb_data_atomic_t * atomic)
{
	int bytes_writes=0;
	if(atomic->data_len==0)
		return 0;
	bytes_writes= picasa_fwrite(file_handle,(unsigned char*)atomic->data,atomic->data_len);
	if(bytes_writes!=atomic->data_len){
		picasa_err("Write Atomic Err name=%s,wbytes=%d,needbytes=%d",atomic->data,bytes_writes,atomic->data_len);
	}
	return bytes_writes;
}

int picasa_write_author(void *file_handle,picasaweb_author_t * author)
{
	picasa_write_atomic(file_handle,&author->name);
	picasa_write_atomic(file_handle,&author->email);
	picasa_write_atomic(file_handle,&author->uri);
	return 0;
}

int picasa_write_exif_tags(void *file_handle,picasaweb_exif_tags_t *tags)
{
	picasa_write_atomic(file_handle,&tags->distance);
	picasa_write_atomic(file_handle,&tags->exposure);
	picasa_write_atomic(file_handle,&tags->flash);
	picasa_write_atomic(file_handle,&tags->focallength);
	picasa_write_atomic(file_handle,&tags->fstop);
	picasa_write_atomic(file_handle,&tags->img_uniqueID);
	picasa_write_atomic(file_handle,&tags->iso);
	picasa_write_atomic(file_handle,&tags->make);
	picasa_write_atomic(file_handle,&tags->model);
	picasa_write_atomic(file_handle,&tags->time);
	return 0;
}


int picasa_write_media_content_attr(void *file_handle,picasaweb_media_content_attr_t *content_attr)
{
	picasa_write_atomic(file_handle,&content_attr->url);
	picasa_write_atomic(file_handle,&content_attr->type);
	picasa_write_atomic(file_handle,&content_attr->medium);
	picasa_write_atomic(file_handle,&content_attr->height);
	picasa_write_atomic(file_handle,&content_attr->width);
	picasa_write_atomic(file_handle,&content_attr->filesize);
	return 0;
}


int picasa_write_thumbnail_attr(void *file_handle,picasaweb_media_thumbnail_attr_t *thumbnail_attr)
{
	picasa_write_atomic(file_handle,&thumbnail_attr->url);
	picasa_write_atomic(file_handle,&thumbnail_attr->height);
	picasa_write_atomic(file_handle,&thumbnail_attr->width);
	return 0;
}


int picasa_write_media_group(void * file_handle,picasaweb_media_group_t * group)
{
	picasa_write_media_content_attr(file_handle,&group->content);
	picasa_write_atomic(file_handle,&group->credit);
	picasa_write_atomic(file_handle,&group->description);
	picasa_write_atomic(file_handle,&group->keywords);
	picasa_write_thumbnail_attr(file_handle,&group->thumbnail);
	picasa_write_atomic(file_handle,&group->title);
	return 0;
}

int picasa_write_point(void * file_handle,picasaweb_Point_t *point)
{
	picasa_write_atomic(file_handle,&point->pos);
	return 0;
}

int picasa_write_where(void * file_handle,picasaweb_where_t *where)
{
	picasa_write_point(file_handle,&where->point);
	return 0;
}

int picasa_write_link_attr(void * file_handle,picasaweb_link_attr_t *link_attr)
{
	picasa_write_atomic(file_handle,&link_attr->rel);
	picasa_write_atomic(file_handle,&link_attr->type);
	picasa_write_atomic(file_handle,&link_attr->href);
	return 0;
}

int picasa_write_bitmap(void * file_handle,unsigned char* bitmap,unsigned long nbytes)
{
	int bytes_writes=0;
	bytes_writes = picasa_fwrite(file_handle,bitmap,nbytes);
	if(bytes_writes!=nbytes){
		picasa_err("Write BitMap wbytes=%d,neebytes=%d",bytes_writes,nbytes);
	}
	return bytes_writes;
}

int picasa_write_entry(void *file_handle,unsigned int offset,picasaweb_entry_t *entry)
{
	picasaweb_entry_t tmp_entry;
	int entry_bytes=sizeof(picasaweb_entry_t);
	memcpy(&tmp_entry,entry,sizeof(picasaweb_entry_t));
	
	picasa_reset_entry(&tmp_entry,offset+entry_bytes);
	
	if(picasa_fseek_set(file_handle,offset)!=0)
		picasa_err("Seek Error! Offset=0x%x",offset);
	
	if(picasa_fwrite(file_handle,(unsigned char*)&tmp_entry,entry_bytes)!=entry_bytes){
		picasa_err("Write Entry Error!");
	}
	
	picasa_write_atomic(file_handle,&entry->attr_etag);
	picasa_write_exif_tags(file_handle,&entry->tags);
	
	picasa_write_atomic(file_handle,&entry->albumid);
	picasa_write_atomic(file_handle,&entry->id);
	picasa_write_atomic(file_handle,&entry->published);
	picasa_write_atomic(file_handle,&entry->updated);
	picasa_write_atomic(file_handle,&entry->edited);
	picasa_write_atomic(file_handle,&entry->rights);
	picasa_write_atomic(file_handle,&entry->summary);
	picasa_write_atomic(file_handle,&entry->title);

	picasa_write_author(file_handle,&entry->author);

	picasa_write_media_group(file_handle,&entry->group);
	
	picasa_write_where(file_handle,&entry->where);
	picasa_write_link_attr(file_handle,&entry->link_edit);
	picasa_write_link_attr(file_handle,&entry->link_feed);
	
	picasa_write_atomic(file_handle,&entry->maxPhotoPerAlbum);
	picasa_write_atomic(file_handle,&entry->nickname);
	picasa_write_atomic(file_handle,&entry->quotacurrent);
	picasa_write_atomic(file_handle,&entry->quotalimit);
	picasa_write_atomic(file_handle,&entry->thumbnail);
	picasa_write_atomic(file_handle,&entry->user);
	
	picasa_write_atomic(file_handle,&entry->access);
	picasa_write_atomic(file_handle,&entry->bytesUsed);
	picasa_write_atomic(file_handle,&entry->location);
	picasa_write_atomic(file_handle,&entry->numphotos);
	picasa_write_atomic(file_handle,&entry->numphotosremaining);
	picasa_write_atomic(file_handle,&entry->checksum);
	picasa_write_atomic(file_handle,&entry->commentCount);
	picasa_write_atomic(file_handle,&entry->height);
	picasa_write_atomic(file_handle,&entry->width);
	picasa_write_atomic(file_handle,&entry->rotation);
	picasa_write_atomic(file_handle,&entry->size);
	picasa_write_atomic(file_handle,&entry->timestamp);
	picasa_write_atomic(file_handle,&entry->videostatus);

	picasa_write_atomic(file_handle,&entry->photoid);
	picasa_write_atomic(file_handle,&entry->weight);
	picasa_write_atomic(file_handle,&entry->albumtitle);
	picasa_write_atomic(file_handle,&entry->albumdesc);
	picasa_write_atomic(file_handle,&entry->snippet);
	picasa_write_atomic(file_handle,&entry->snippettype);
	picasa_write_atomic(file_handle,&entry->truncated);

	return 0;	
}

int picasa_write_feed(void *file_handle,unsigned int offset,picasaweb_feed_t *feed)
{
	int i=0;
	unsigned int entry_offset=0;
	int bitmap_bytes = (feed->entry_num+7)/8;
	int feed_bytes=sizeof(picasaweb_feed_t);
	picasaweb_feed_t tmp_feed;

	picasa_info("Feed Size=%d\n",feed_bytes);
	memcpy(&tmp_feed,feed,feed_bytes);
	if(picasa_fseek_set(file_handle,offset)!=0)
		picasa_err("Seek Error! Offset=0x%x",offset);
	
	picasa_reset_feed(&tmp_feed,offset+feed_bytes);
	
	if(picasa_fwrite(file_handle,(unsigned char*)&tmp_feed,feed_bytes)!=feed_bytes){
		picasa_err("Write Feed Error!");
	}
	//__test_tell_filepos(file_handle);
	picasa_write_atomic(file_handle,&feed->attr_etag);
	picasa_write_atomic(file_handle,&feed->albumid);
	picasa_write_atomic(file_handle,&feed->id);
	picasa_write_atomic(file_handle,&feed->updated);
	picasa_write_atomic(file_handle,&feed->title);
	picasa_write_atomic(file_handle,&feed->subtitle);
	picasa_write_atomic(file_handle,&feed->icon);

	picasa_write_author(file_handle,&feed->author);

	picasa_write_atomic(file_handle,&feed->totalResults);
	picasa_write_atomic(file_handle,&feed->startIndex);
	picasa_write_atomic(file_handle,&feed->itemsPerPage);
	picasa_write_atomic(file_handle,&feed->maxPhotoPerAlbum);
	picasa_write_atomic(file_handle,&feed->nickname);
	picasa_write_atomic(file_handle,&feed->quotacurrent);
	picasa_write_atomic(file_handle,&feed->quotalimit);
	picasa_write_atomic(file_handle,&feed->thumbnail);
	picasa_write_atomic(file_handle,&feed->user);
	picasa_write_atomic(file_handle,&feed->access);
	picasa_write_atomic(file_handle,&feed->bytesUsed);
	picasa_write_atomic(file_handle,&feed->location);
	picasa_write_atomic(file_handle,&feed->numphotos);
	picasa_write_atomic(file_handle,&feed->numphotosremaining);

	picasa_write_atomic(file_handle,&feed->checksum);
	picasa_write_atomic(file_handle,&feed->commentCount);
	picasa_write_atomic(file_handle,&feed->height);
	picasa_write_atomic(file_handle,&feed->width);
	picasa_write_atomic(file_handle,&feed->rotation);
	picasa_write_atomic(file_handle,&feed->size);
	picasa_write_atomic(file_handle,&feed->timestamp);
	picasa_write_atomic(file_handle,&feed->videostatus);
	picasa_write_atomic(file_handle,&feed->photoid);
	picasa_write_atomic(file_handle,&feed->weight);

	picasa_write_bitmap(file_handle,feed->download_thumbnailbitmap,bitmap_bytes);
	picasa_write_bitmap(file_handle,feed->download_browsebitmap,bitmap_bytes);

	//__test_tell_filepos(file_handle);

	for(i=0;i<feed->entry_num;i++){
		entry_offset = picasa_ftell(file_handle);
		picasa_write_entry(file_handle,entry_offset,feed->entry+i);
	}
	return 0;
}

#endif

#if 1
int picasa_read_atomic(void *file_handle,picasaweb_data_atomic_t *atomic)
{
	char *tmp_data=NULL;
	int bytes_read=0;
	int rtn=0;
	if(atomic->data && atomic->data_len){
		if(picasa_fseek_set(file_handle,(long)atomic->data)!=0){
			picasa_err("Seek Error!,Offset = 0x%x",atomic->data);
			rtn = -1;
			goto READ_ATOMIC_END;
		}	
		tmp_data = (char*)picasa_malloc(atomic->data_len);
		if(tmp_data==NULL){
			picasa_err("Malloc Falied,Atomic DataLen=%d",atomic->data_len);
			rtn = -1;
			goto READ_ATOMIC_END;
		}
		if(picasa_fread(file_handle,(unsigned char*)tmp_data,atomic->data_len)!=atomic->data_len){
			picasa_err("Read Data Error!");
			rtn = -1;
			goto READ_ATOMIC_END;
		}	
		
	}
READ_ATOMIC_END:
	atomic->data = tmp_data;///< save the pointer of the data to the atomic
	return rtn;
}

int picasa_read_author(void *file_handle,picasaweb_author_t * author)
{
	picasa_read_atomic(file_handle,&author->name);
	picasa_read_atomic(file_handle,&author->email);
	picasa_read_atomic(file_handle,&author->uri);
	return 0;
}

int picasa_read_exif_tags(void *file_handle,picasaweb_exif_tags_t *tags)
{
	picasa_read_atomic(file_handle,&tags->distance);
	picasa_read_atomic(file_handle,&tags->exposure);
	picasa_read_atomic(file_handle,&tags->flash);
	picasa_read_atomic(file_handle,&tags->focallength);
	picasa_read_atomic(file_handle,&tags->fstop);
	picasa_read_atomic(file_handle,&tags->img_uniqueID);
	picasa_read_atomic(file_handle,&tags->iso);
	picasa_read_atomic(file_handle,&tags->make);
	picasa_read_atomic(file_handle,&tags->model);
	picasa_read_atomic(file_handle,&tags->time);
	return 0;
}

int picasa_read_media_content_attr(void *file_handle,picasaweb_media_content_attr_t *content_attr)
{
	picasa_read_atomic(file_handle,&content_attr->url);
	picasa_read_atomic(file_handle,&content_attr->type);
	picasa_read_atomic(file_handle,&content_attr->medium);
	picasa_read_atomic(file_handle,&content_attr->height);
	picasa_read_atomic(file_handle,&content_attr->width);
	picasa_read_atomic(file_handle,&content_attr->filesize);
	return 0;
}


int picasa_read_thumbnail_attr(void *file_handle,picasaweb_media_thumbnail_attr_t *thumbnail_attr)
{
	picasa_read_atomic(file_handle,&thumbnail_attr->url);
	picasa_read_atomic(file_handle,&thumbnail_attr->height);
	picasa_read_atomic(file_handle,&thumbnail_attr->width);
	return 0;
}


int picasa_read_media_group(void * file_handle,picasaweb_media_group_t * group)
{
	picasa_read_media_content_attr(file_handle,&group->content);
	picasa_read_atomic(file_handle,&group->credit);
	picasa_read_atomic(file_handle,&group->description);
	picasa_read_atomic(file_handle,&group->keywords);
	picasa_read_thumbnail_attr(file_handle,&group->thumbnail);
	picasa_read_atomic(file_handle,&group->title);
	return 0;
}

int picasa_read_point(void * file_handle,picasaweb_Point_t *point)
{
	picasa_read_atomic(file_handle,&point->pos);
	return 0;
}

int picasa_read_where(void * file_handle,picasaweb_where_t *where)
{
	picasa_read_point(file_handle,&where->point);
	return 0;
}

int picasa_read_link_attr(void * file_handle,picasaweb_link_attr_t *link_attr)
{
	picasa_read_atomic(file_handle,&link_attr->rel);
	picasa_read_atomic(file_handle,&link_attr->type);
	picasa_read_atomic(file_handle,&link_attr->href);
	return 0;
}


int picasa_read_entry(void* file_handle,unsigned int offset,picasaweb_entry_t * entry)
{
	picasaweb_entry_t tmp_entry;
	unsigned int entry_bytes=sizeof(picasaweb_entry_t);
	
	if(picasa_fseek_set(file_handle,offset)!=0)
			picasa_err("Seek Error!,Offset = 0x%x",offset);
	
	memset(&tmp_entry,0,sizeof(picasaweb_entry_t));
	if(picasa_fread(file_handle,(unsigned char*)&tmp_entry,entry_bytes)!=entry_bytes){
		picasa_err("Read Entry Error!");
	}

	picasa_info("Entry Bytes=0x%x",entry_bytes);
	picasa_read_atomic(file_handle,&tmp_entry.attr_etag);
	picasa_read_exif_tags(file_handle,&tmp_entry.tags);
	
	picasa_read_atomic(file_handle,&tmp_entry.albumid);
	picasa_read_atomic(file_handle,&tmp_entry.id);
	picasa_read_atomic(file_handle,&tmp_entry.published);
	picasa_read_atomic(file_handle,&tmp_entry.updated);
	picasa_read_atomic(file_handle,&tmp_entry.edited);
	picasa_read_atomic(file_handle,&tmp_entry.rights);
	picasa_read_atomic(file_handle,&tmp_entry.summary);
	picasa_read_atomic(file_handle,&tmp_entry.title);
	picasa_read_author(file_handle,&tmp_entry.author);
	
	picasa_read_media_group(file_handle,&tmp_entry.group);
	
	picasa_read_where(file_handle,&tmp_entry.where);
	
	picasa_read_link_attr(file_handle,&tmp_entry.link_edit);
	picasa_read_link_attr(file_handle,&tmp_entry.link_feed);
	picasa_read_atomic(file_handle,&tmp_entry.maxPhotoPerAlbum);
	picasa_read_atomic(file_handle,&tmp_entry.nickname);
	picasa_read_atomic(file_handle,&tmp_entry.quotacurrent);
	picasa_read_atomic(file_handle,&tmp_entry.quotalimit);
	picasa_read_atomic(file_handle,&tmp_entry.user);

	picasa_read_atomic(file_handle,&tmp_entry.access);
	picasa_read_atomic(file_handle,&tmp_entry.bytesUsed);
	picasa_read_atomic(file_handle,&tmp_entry.location);
	picasa_read_atomic(file_handle,&tmp_entry.numphotos);
	picasa_read_atomic(file_handle,&tmp_entry.numphotosremaining);

	picasa_read_atomic(file_handle,&tmp_entry.checksum);
	picasa_read_atomic(file_handle,&tmp_entry.commentCount);
	picasa_read_atomic(file_handle,&tmp_entry.height);
	picasa_read_atomic(file_handle,&tmp_entry.width);
	picasa_read_atomic(file_handle,&tmp_entry.rotation);
	picasa_read_atomic(file_handle,&tmp_entry.size);
	picasa_read_atomic(file_handle,&tmp_entry.timestamp);
	picasa_read_atomic(file_handle,&tmp_entry.videostatus);

	picasa_read_atomic(file_handle,&tmp_entry.photoid);

	picasa_read_atomic(file_handle,&tmp_entry.weight);

	picasa_read_atomic(file_handle,&tmp_entry.albumtitle);
	picasa_read_atomic(file_handle,&tmp_entry.albumdesc);
	picasa_read_atomic(file_handle,&tmp_entry.snippet);
	picasa_read_atomic(file_handle,&tmp_entry.snippettype);
	picasa_read_atomic(file_handle,&tmp_entry.truncated);

	memcpy(entry,&tmp_entry,entry_bytes);
	return 0;
}

int picasa_read_bitmap(void *file_handle,unsigned int offset,unsigned char** bitmap,unsigned int bitmap_bytes)
{
	unsigned char* tmp_data=NULL;
	*bitmap = NULL;
	if(bitmap_bytes==0 || file_handle==NULL)
		return 0;

	if(picasa_fseek_set(file_handle,offset)!=0){
		picasa_err("Seek Error!,Offset = 0x%x",offset);
		return -1;
	}
	
	tmp_data = (unsigned char*)picasa_malloc(bitmap_bytes);
	if(tmp_data==NULL){
		picasa_err("Malloc Falied,Bitmap Len=%d",bitmap_bytes);
		return -1;
	}
	if(picasa_fread(file_handle,tmp_data,bitmap_bytes)!=bitmap_bytes){
		picasa_err("Read Data Error!");
		return -1;
	}	
	*bitmap = tmp_data;///< save the pointer of the data to the atomic
	return 0;
}



int picasa_read_feed(void * file_handle,unsigned int offset,picasaweb_feed_t * feed)
{
	picasaweb_feed_t tmp_feed;
	unsigned int bitmap_offset=0;
	unsigned int bitmap_bytes=0;
	int feed_bytes=sizeof(picasaweb_feed_t);
	if(picasa_fseek_set(file_handle,offset)!=0)
			picasa_err("Seek Error!,Offset = 0x%x",offset);
	
	memset(&tmp_feed,0,feed_bytes);
	if(picasa_fread(file_handle,(unsigned char*)&tmp_feed,feed_bytes)!=feed_bytes){
		picasa_err("Read Feed Error!");
	}
	
	picasa_read_atomic(file_handle,&tmp_feed.attr_etag);
	picasa_read_atomic(file_handle,&tmp_feed.albumid);
	picasa_read_atomic(file_handle,&tmp_feed.id);
	picasa_read_atomic(file_handle,&tmp_feed.updated);
	picasa_read_atomic(file_handle,&tmp_feed.title);
	picasa_read_atomic(file_handle,&tmp_feed.subtitle);
	picasa_read_atomic(file_handle,&tmp_feed.icon);
	
	picasa_read_author(file_handle,&tmp_feed.author);
	
	picasa_read_atomic(file_handle,&tmp_feed.totalResults);
	picasa_read_atomic(file_handle,&tmp_feed.startIndex);
	picasa_read_atomic(file_handle,&tmp_feed.itemsPerPage);
	picasa_read_atomic(file_handle,&tmp_feed.maxPhotoPerAlbum);
	picasa_read_atomic(file_handle,&tmp_feed.nickname);
	picasa_read_atomic(file_handle,&tmp_feed.quotacurrent);
	picasa_read_atomic(file_handle,&tmp_feed.quotalimit);
	picasa_read_atomic(file_handle,&tmp_feed.thumbnail);
	picasa_read_atomic(file_handle,&tmp_feed.user);
	picasa_read_atomic(file_handle,&tmp_feed.access);
	picasa_read_atomic(file_handle,&tmp_feed.bytesUsed);
	picasa_read_atomic(file_handle,&tmp_feed.location);
	picasa_read_atomic(file_handle,&tmp_feed.numphotos);
	picasa_read_atomic(file_handle,&tmp_feed.numphotosremaining);
	
	picasa_read_atomic(file_handle,&tmp_feed.checksum);
	picasa_read_atomic(file_handle,&tmp_feed.commentCount);
	picasa_read_atomic(file_handle,&tmp_feed.height);
	picasa_read_atomic(file_handle,&tmp_feed.width);
	picasa_read_atomic(file_handle,&tmp_feed.rotation);
	picasa_read_atomic(file_handle,&tmp_feed.size);
	picasa_read_atomic(file_handle,&tmp_feed.timestamp);
	picasa_read_atomic(file_handle,&tmp_feed.videostatus);
	picasa_read_atomic(file_handle,&tmp_feed.photoid);
	picasa_read_atomic(file_handle,&tmp_feed.weight);

	bitmap_bytes =(tmp_feed.entry_num+7)/8;
	bitmap_offset = picasa_ftell(file_handle);
	if(picasa_read_bitmap(file_handle,bitmap_offset,&tmp_feed.download_thumbnailbitmap,bitmap_bytes*2)==0){
		tmp_feed.download_browsebitmap = tmp_feed.download_thumbnailbitmap+bitmap_bytes;
	}

	memcpy(feed,&tmp_feed,feed_bytes);
	return 0;
}

#endif

//////Just For Debug
#if 1
int __picasa_printf_atomic(char* name,picasaweb_data_atomic_t *atomic_name)
{
	if(name!=NULL && atomic_name!=NULL){
		if(atomic_name->data!=NULL){
			printf("[%s,len=%d]=%s\n",name,atomic_name->data_len,atomic_name->data);
		}
	}
	return 0;
}


int __picasa_printf_group_cont_element(picasaweb_media_content_attr_t *group_cont)
{
	__picasa_printf_atomic("content_url",&(group_cont->url));
	__picasa_printf_atomic("content_type",&(group_cont->type));
	__picasa_printf_atomic("content_medium",&(group_cont->medium));
	__picasa_printf_atomic("content_height",&(group_cont->height));
	__picasa_printf_atomic("content_width",&(group_cont->width));
	__picasa_printf_atomic("content_filesize",&(group_cont->filesize));
	return 0;
}

int __picasa_printf_group_thumbnail_element(picasaweb_media_thumbnail_attr_t *group_thumb)
{
	__picasa_printf_atomic("thumb_url",&(group_thumb->url));
	__picasa_printf_atomic("thumb_height",&(group_thumb->height));
	__picasa_printf_atomic("thumb_width",&(group_thumb->width));
	return 0;
}

int __picasa_printf_group_element(picasaweb_media_group_t *group)
{
	__picasa_printf_group_cont_element(&group->content);
	__picasa_printf_atomic("credit",&(group->credit));
	__picasa_printf_atomic("description",&(group->description));
	__picasa_printf_atomic("keywords",&(group->keywords));
	__picasa_printf_group_thumbnail_element(&group->thumbnail);
	__picasa_printf_atomic("title",&(group->title));
	return 0;
}

int __picasa_printf_author_element(picasaweb_author_t *author)
{
	__picasa_printf_atomic("author_name",&(author->name));
	__picasa_printf_atomic("author_email",&(author->email));
	__picasa_printf_atomic("author_uri",&(author->uri));
	return 0;
}

int __picasa_printf_tags_element(picasaweb_exif_tags_t *tags)
{
	__picasa_printf_atomic("distance",&(tags->distance));
	__picasa_printf_atomic("exposure",&(tags->exposure));
	__picasa_printf_atomic("flash",&(tags->flash));
	__picasa_printf_atomic("focallength",&(tags->focallength));
	__picasa_printf_atomic("fstop",&(tags->fstop));
	__picasa_printf_atomic("uniqueID",&(tags->img_uniqueID));
	__picasa_printf_atomic("iso",&(tags->iso));
	__picasa_printf_atomic("make",&(tags->make));
	__picasa_printf_atomic("model",&(tags->model));
	__picasa_printf_atomic("time",&(tags->time));	
	return 0;
}

int __picasa_printf_where_point_element(picasaweb_Point_t *point)
{
	__picasa_printf_atomic("pos",&(point->pos));	
	return 0;
}

int __picasa_printf_where_element(picasaweb_where_t * where)
{
	__picasa_printf_where_point_element(&(where->point));
	return 0;
}

int __picasa_printf_link_element(picasaweb_link_attr_t *link)
{
	__picasa_printf_atomic("link_rel",&(link->rel));
	__picasa_printf_atomic("link_type",&(link->type));
	__picasa_printf_atomic("link_href",&(link->href));	
	return 0;
}

int __picasa_printf_entry(picasaweb_entry_t *entry)
{
#ifdef PICASA_WEBALBUM_DEBUG
	__picasa_printf_tags_element(&entry->tags);
	
	__picasa_printf_atomic("etag",&entry->attr_etag);
	__picasa_printf_atomic("albumid",&entry->albumid);
	__picasa_printf_atomic("id",&entry->id);
	__picasa_printf_atomic("published",&entry->published);
	__picasa_printf_atomic("updated",&entry->updated);
	__picasa_printf_atomic("edited",&entry->edited);
	__picasa_printf_atomic("rights",&entry->rights);
	__picasa_printf_atomic("summary",&entry->summary);
	__picasa_printf_atomic("title",&entry->title);

	__picasa_printf_author_element(&entry->author);

	__picasa_printf_group_element(&entry->group);

	__picasa_printf_where_element(&entry->where);
	printf("LINK_Edit:\n");
	__picasa_printf_link_element(&entry->link_edit);
	printf("LINK_feed:\n");
	__picasa_printf_link_element(&entry->link_feed);
	__picasa_printf_atomic("maxPhotoPerAlbum",&entry->maxPhotoPerAlbum);
	__picasa_printf_atomic("nickname",&entry->nickname);
	__picasa_printf_atomic("quotacurrent",&entry->quotacurrent);
	__picasa_printf_atomic("quotalimit",&entry->quotalimit);
	__picasa_printf_atomic("thumbnail",&entry->thumbnail);
	__picasa_printf_atomic("user",&entry->user);

	__picasa_printf_atomic("access",&entry->access);
	__picasa_printf_atomic("bytesUsed",&entry->bytesUsed);
	__picasa_printf_atomic("location",&entry->location);
	__picasa_printf_atomic("numphotos",&entry->numphotos);
	__picasa_printf_atomic("numphotoremain",&entry->numphotosremaining);

	__picasa_printf_atomic("checksum",&entry->checksum);
	__picasa_printf_atomic("commentCount",&entry->commentCount);
	__picasa_printf_atomic("height",&entry->height);
	__picasa_printf_atomic("width",&entry->width);
	__picasa_printf_atomic("rotation",&entry->rotation);
	__picasa_printf_atomic("timestamp",&entry->timestamp);
	__picasa_printf_atomic("videostatus",&entry->videostatus);

	__picasa_printf_atomic("photoid",&entry->photoid);
	__picasa_printf_atomic("weight",&entry->weight);
	__picasa_printf_atomic("albumtitle",&entry->albumtitle);

	__picasa_printf_atomic("albumdesc",&entry->albumdesc);
	__picasa_printf_atomic("snippet",&entry->snippet);
	__picasa_printf_atomic("snippettype",&entry->snippettype);
	__picasa_printf_atomic("truncated",&entry->truncated);
#endif
	return 0;

}

int __picasa_printf_bitmap(unsigned char* bitmap,unsigned int bytes,int eachline)
{
#ifdef PICASA_WEBALBUM_DEBUG
	int i=0;
	picasa_info("Print Bitmap Start ++++++++++");
	for(i=0;i<bytes;i++){
		if(i!=0 && (i%eachline==0))
			printf("\n");
		else
			printf("0x%x ",*(bitmap+i));
	}
	printf("\n");
	picasa_info("Print Bitmap END ++++++++++");
#endif
	return 0;
}

int __picasa_printf_feed(picasaweb_feed_t *feed)
{
#ifdef PICASA_WEBALBUM_DEBUG
	int bitmap_bytes=(feed->entry_num+7)/8;
	__picasa_printf_atomic("feed_etag=",&feed->attr_etag);
	__picasa_printf_atomic("feed_id=",&feed->id);
	__picasa_printf_atomic("feed_updated",&feed->updated);
	__picasa_printf_atomic("feed_title",&feed->title);
	__picasa_printf_atomic("feed_subtitle",&feed->subtitle);
	__picasa_printf_atomic("feed_totalResults",&feed->totalResults);
	__picasa_printf_atomic("feed_startIndex",&feed->startIndex);
	__picasa_printf_atomic("feed_itemsPerPage",&feed->itemsPerPage);
	__picasa_printf_atomic("feed_user",&feed->user);
	__picasa_printf_atomic("feed_thumbnail",&feed->thumbnail);
	__picasa_printf_atomic("feed_quotalimit",&feed->quotalimit);
	__picasa_printf_atomic("feed_quotacurrent",&feed->quotacurrent);
	__picasa_printf_atomic("feed_maxPhotoPerAlbum",&feed->maxPhotoPerAlbum);

	__picasa_printf_bitmap(feed->download_thumbnailbitmap,bitmap_bytes,16);
	__picasa_printf_bitmap(feed->download_browsebitmap,bitmap_bytes,16);
	
	__picasa_printf_author_element(&feed->author);	
#endif
	return 0;

}
#endif
///////////////////////////////////////
/*
*@brief fill the entry according to the node
*param[in] entry	: the entry to be fill
*param[in] node		: the node which is the element of an entry	  
*/
int picasa_fill_entry_element(picasaweb_entry_t *entry,xmlNodePtr node)
{
	int idx=-1;
	int rtn = 0;
	int ns_idx=-1;
	xmlChar* content=NULL;
	content = xmlNodeGetContent(node);
	if(node->name){
		idx = picasa_get_tagindex((char*)node->name);
	}	
	if(idx!=-1){
		switch(idx){
			///both atom and gphoto
			case NODE_ID:
				ns_idx = __picasa_get_namespace_id(node);
				if(ns_idx==NS_GPHOTO){
					picasa_fill_atomic(content,&(entry->id));///<
				}
				break;
			case NODE_ALBUMID:
				picasa_fill_atomic(content,&(entry->albumid));///<
				break;
			///< for atom space
			case NODE_TITLE:
				ns_idx = __picasa_get_namespace_id(node);
				if(ns_idx==NS_ATOM)
					picasa_fill_atomic(content,&(entry->title));///< 
				else if(ns_idx==NS_MEDIA)
					picasa_fill_atomic(content,&(entry->group.title));///< 
				break;
			case NODE_LINK:	
				__picasa_fill_link_attr(node,entry);
				break;
			case NODE_SUMMARY:
				picasa_fill_atomic(content,&(entry->summary));///< 
				break;
			case NODE_CONTENT:
				ns_idx = __picasa_get_namespace_id(node);
				if(ns_idx==NS_MEDIA){
					__picasa_fill_node_attr(node,"url",&(entry->group.content.url));
					__picasa_fill_node_attr(node,"height",&(entry->group.content.height));
					__picasa_fill_node_attr(node,"width",&(entry->group.content.width));
					__picasa_fill_node_attr(node,"type",&(entry->group.content.type));
					__picasa_fill_node_attr(node,"medium",&(entry->group.content.medium));
				}
				break;
			case NODE_AUTHOR:
				picasa_entry_enter_childnode(entry,node);///<
				break;
			case NODE_AUTHOR_NAME:
				picasa_fill_atomic(content,&(entry->author.name));///<
				break;
			case NODE_AUTHOR_EMAIL:
				picasa_fill_atomic(content,&(entry->author.email));///<
				break;
			case NODE_AUTHOR_URI:
				picasa_fill_atomic(content,&(entry->author.uri));///<
				break;
			case NODE_CATEGORY:
				__picasa_fill_node_attr(node,"term",NULL);
				__picasa_fill_node_attr(node,"scheme",NULL);
				break;
			case NODE_PUBLIC_DATE:
				picasa_fill_atomic(content,&(entry->published));///< 
				break;
			case NODE_UPDATE_DATE:
				picasa_fill_atomic(content,&(entry->updated)); ///< 
				break;
			case NODE_EDIT_DATE:
				picasa_fill_atomic(content,&(entry->edited)); ///< 
				break;
			case NODE_RIGHTS:
				picasa_fill_atomic(content,&(entry->rights)); ///< 
				break;
			///< for gphoto space
			case NODE_PHOTO_LOCATION:
				picasa_fill_atomic(content,&(entry->location));///< 
				break;
			case NODE_PHOTO_ACCESS:
				picasa_fill_atomic(content,&(entry->access));///< 
				break;
			case NODE_PHOTO_TIMESTAMP:
				picasa_fill_atomic(content,&(entry->timestamp));///< 
				break;
			case NODE_PHOTO_NUMPHOTOS:
				picasa_fill_atomic(content,&(entry->numphotos));///< 
				break;
			case NODE_PHOTO_NUMPHOTOREMAIN:
				picasa_fill_atomic(content,&(entry->numphotosremaining));///< 
				break;
			case NODE_PHOTO_BYTESUSED:
				picasa_fill_atomic(content,&(entry->bytesUsed));///< 
				break;
			case NODE_PHOTO_USER:
				picasa_fill_atomic(content,&(entry->user));///< 
				break;
			case NODE_PHOTO_NICHNAME:
				picasa_fill_atomic(content,&(entry->nickname));///< 
				break;
	
			///< for media space
			case NODE_GROUP:
				picasa_entry_enter_childnode(entry,node);///<
				break;
			case NODE_GROUP_CREDIT:
				picasa_fill_atomic(content,&(entry->group.credit));///<
				break;
			case NODE_GROUP_DESC:
				picasa_fill_atomic(content,&(entry->group.description));///<
				break;
			case NODE_GROUP_KEYWORDS:
				picasa_fill_atomic(content,&(entry->group.keywords));///<
				break;
			case NODE_THUMBNAIL:
				ns_idx = __picasa_get_namespace_id(node);
				if(ns_idx==NS_MEDIA){
					xmlChar* tmpprop= xmlGetProp(node,(xmlChar*)"width");
					if(tmpprop){
						//printf("Trop===%s,url===%s",(char*)tmpprop,&(entry->group.thumbnail.url));
						if(entry->group.thumbnail.is_stored==0){
							if(strcmp((char*)tmpprop,"160")==0 || strcmp((char*)tmpprop,"72")==0){
								__picasa_fill_node_attr(node,"url",&(entry->group.thumbnail.url));
								__picasa_fill_node_attr(node,"height",&(entry->group.thumbnail.height));
								__picasa_fill_node_attr(node,"width",&(entry->group.thumbnail.width));
							}
							entry->group.thumbnail.is_stored = 1;
						}
						xmlFree(tmpprop);
					}
				}
				else if(ns_idx==NS_GPHOTO){///add for profile
					picasa_fill_atomic(content,&(entry->thumbnail));///<
				}
				break;
			///< for georss space
			case NODE_WHERE:
				picasa_entry_enter_childnode(entry,node);///<
				break;
			case NODE_POINT:
				picasa_entry_enter_childnode(entry,node);///<
				break;
			case NODE_POS:
				picasa_fill_atomic(content,&(entry->where.point.pos));///<
				break;

			case NODE_TAGS:
				picasa_entry_enter_childnode(entry,node);///<
				break;
			case NODE_FSTOP:
				picasa_fill_atomic(content,&(entry->tags.fstop));///<
				break;
			case NODE_DISTANCE:
				picasa_fill_atomic(content,&(entry->tags.distance));///<
				break;
			case NDDE_MAKE:
				picasa_fill_atomic(content,&(entry->tags.make));///<
				break;
			case NODE_MODEL:
				picasa_fill_atomic(content,&(entry->tags.model));///<
				break;
			case NODE_EXPOSURE:
				picasa_fill_atomic(content,&(entry->tags.exposure));///<
				break;
			case NODE_FLASH:
				picasa_fill_atomic(content,&(entry->tags.flash));///<
				break;
			case NODE_FOCALLENGTH:
				picasa_fill_atomic(content,&(entry->tags.focallength));///<
				break;
			case NODE_ISO:
				picasa_fill_atomic(content,&(entry->tags.iso));///<
				break;
			case NODE_TIME:
				picasa_fill_atomic(content,&(entry->tags.time));///<
				break;
			case NODE_IMAGEUNIQUEID:
				picasa_fill_atomic(content,&(entry->tags.img_uniqueID));///<
				break;
			default:
				printf("%s,%d:Hash Idx Not Defined!\n",__FILE__,__LINE__);
				rtn  = -1;
				break;

		}
	}
	else
		rtn = -1;
	if(content)
		xmlFree(content);
	return rtn;
}


int picasa_free_entry_element(picasaweb_entry_t *entry)
{
	if(entry==NULL)
		return -1;
	picasa_free_atomic(&entry->attr_etag);
	picasa_free_tags_element(&entry->tags);
	
	picasa_free_atomic(&entry->id);
	picasa_free_atomic(&entry->albumid);
	picasa_free_atomic(&entry->title);
	
	picasa_free_atomic(&entry->summary);
	picasa_free_author_element(&entry->author);

	picasa_free_group_element(&entry->group);

	picasa_free_where_element(&entry->where);

	picasa_free_link_element(&entry->link_edit);
	picasa_free_link_element(&entry->link_feed);

	picasa_free_atomic(&entry->published);
	picasa_free_atomic(&entry->updated);
	picasa_free_atomic(&entry->edited);
	picasa_free_atomic(&entry->rights);
	picasa_free_atomic(&entry->location);
	picasa_free_atomic(&entry->access);
	picasa_free_atomic(&entry->timestamp);
	picasa_free_atomic(&entry->numphotos);
	picasa_free_atomic(&entry->numphotosremaining);	
	picasa_free_atomic(&entry->bytesUsed);
	picasa_free_atomic(&entry->user);
	picasa_free_atomic(&entry->nickname);


	///< have't feed yet
	picasa_free_atomic(&entry->maxPhotoPerAlbum);
	picasa_free_atomic(&entry->quotacurrent);
	picasa_free_atomic(&entry->quotalimit);
	picasa_free_atomic(&entry->thumbnail);

	picasa_free_atomic(&entry->checksum);
	picasa_free_atomic(&entry->commentCount);
	picasa_free_atomic(&entry->height);
	picasa_free_atomic(&entry->width);
	picasa_free_atomic(&entry->rotation);

	picasa_free_atomic(&entry->videostatus);

	picasa_free_atomic(&entry->photoid);
	picasa_free_atomic(&entry->weight);
	picasa_free_atomic(&entry->albumtitle);

	picasa_free_atomic(&entry->albumdesc);
	picasa_free_atomic(&entry->snippet);
	picasa_free_atomic(&entry->snippettype);
	picasa_free_atomic(&entry->truncated);
	return 0;
	
}

int picasa_free_feed_element(picasaweb_feed_t *feed)
{
	int i=0;
	if(feed==NULL)
		return -1;
	picasa_free_atomic(&feed->attr_etag);
	picasa_free_atomic(&feed->id);
	picasa_free_atomic(&feed->albumid);
	picasa_free_atomic(&feed->updated);
	picasa_free_atomic(&feed->title);
	picasa_free_atomic(&feed->subtitle);
	picasa_free_atomic(&feed->totalResults);
	picasa_free_atomic(&feed->startIndex);
	picasa_free_atomic(&feed->itemsPerPage);
	picasa_free_atomic(&feed->user);
	picasa_free_atomic(&feed->nickname);
	picasa_free_atomic(&feed->thumbnail);
	picasa_free_atomic(&feed->quotalimit);
	picasa_free_atomic(&feed->quotacurrent);
	picasa_free_atomic(&feed->maxPhotoPerAlbum);

	picasa_free_author_element(&feed->author);
	
	return 0;
}

int picasa_fill_feed_element(picasaweb_feed_t *feed,xmlNodePtr node)
{
	int idx=-1;
	int rtn = 0;
	int ns_idx=-1;
	xmlChar* content=NULL;
	content = xmlNodeGetContent(node);
	if(node->name){
		idx = picasa_get_tagindex((char*)node->name);
	}	
	if(idx!=-1){
		switch(idx){
			case NODE_ID:
				ns_idx = __picasa_get_namespace_id(node);
				if(ns_idx==NS_ATOM){
					picasa_fill_atomic(content,&(feed->id));///<
				}
				break;
			case NODE_ALBUMID:
				picasa_fill_atomic(content,&(feed->albumid));///<
				break;
			case NODE_UPDATE_DATE:
				picasa_fill_atomic(content,&(feed->updated));
				break;
			case NODE_CATEGORY:
				__picasa_fill_node_attr(node,"term",NULL);
				__picasa_fill_node_attr(node,"scheme",NULL);
				break;
			case NODE_TITLE:
				ns_idx = __picasa_get_namespace_id(node);
				if(ns_idx==NS_ATOM)
					picasa_fill_atomic(content,&(feed->title));///< 
				break;
			case NODE_SUBTITLE:
				picasa_fill_atomic(content,&(feed->subtitle));
				break;	
			case NODE_TOTALRESULT:
				picasa_fill_atomic(content,&(feed->totalResults));
				break;
			case NODE_STARTINDEX:
				picasa_fill_atomic(content,&(feed->startIndex));
				break;
			case NODE_ITEMSPERPAGE:
				picasa_fill_atomic(content,&(feed->itemsPerPage));
				break;
			case NODE_PHOTO_USER:
				picasa_fill_atomic(content,&(feed->user));///< 
				break;
			case NODE_PHOTO_NICHNAME:
				picasa_fill_atomic(content,&(feed->nickname));///< 
				break;
			case NODE_THUMBNAIL:
				picasa_fill_atomic(content,&(feed->thumbnail));///< 
				break;	
			case NODE_QUOTALIMIT:
				picasa_fill_atomic(content,&(feed->quotalimit));///< 
				break;
			case NODE_QUOTACURRENT:
				picasa_fill_atomic(content,&(feed->quotacurrent));///< 
				break;
			case NODE_MAXPHOTOSPERALBUM:
				picasa_fill_atomic(content,&(feed->maxPhotoPerAlbum));///< 
				break;
			case NODE_AUTHOR:
				picasa_feed_enter_childnode(feed,node);///<
				break;
			case NODE_AUTHOR_NAME:
				picasa_fill_atomic(content,&(feed->author.name));///<
				break;
			case NODE_AUTHOR_EMAIL:
				picasa_fill_atomic(content,&(feed->author.email));///<
				break;
			case NODE_AUTHOR_URI:
				picasa_fill_atomic(content,&(feed->author.uri));///<
				rtn = -1;
				break;

		}
	}
	else
		rtn = -1;
	if(content!=NULL)
		xmlFree(content);
	return rtn;
}


int picasa_set_downloadbit(picasaweb_feed_t *feed,int which_entry,int isthumbnail)
{
	int rtn=0;
	int whichbit=0,whichbyte=0;
	whichbyte = which_entry/8;
	whichbit = which_entry%8;
	if(feed->entry_num <=0 || which_entry>=feed->entry_num || which_entry<0){
		rtn = -1;
		goto SET_DOWNLOADBIT_END;
	}	
	if(isthumbnail){
		feed->download_thumbnailbitmap[whichbyte] = feed->download_thumbnailbitmap[whichbyte] | (1<<whichbit);
	}
	else
		feed->download_browsebitmap[whichbyte] = feed->download_browsebitmap[whichbyte] | (1<<whichbit);

SET_DOWNLOADBIT_END:
	return rtn;
}

int picasa_clear_downloadbit(picasaweb_feed_t *feed,int which_entry,int isthumbnail)
{
	int rtn=0;
	int whichbit=0,whichbyte=0;
	whichbyte = which_entry/8;
	whichbit = which_entry%8;
	if(feed->entry_num <=0 || which_entry>=feed->entry_num || which_entry<0){
		rtn = -1;
		goto CLEAR_DOWNLOADBIT_END;
	}	
	if(isthumbnail){
		feed->download_thumbnailbitmap[whichbyte] = feed->download_thumbnailbitmap[whichbyte] & ~(1<<whichbit);
	}
	else
		feed->download_browsebitmap[whichbyte] = feed->download_browsebitmap[whichbyte] & ~(1<<whichbit);

CLEAR_DOWNLOADBIT_END:
	return rtn;	
}

int picasa_query_downloadbit(picasaweb_feed_t *feed,int which_entry,int isthumbnail)
{
	int rtn=0;
	int whichbit=0,whichbyte=0;
	whichbyte = which_entry/8;
	whichbit = which_entry%8;
	if(feed->entry_num <=0 || which_entry>=feed->entry_num || which_entry<0){
		rtn = -1;
		goto QUREY_DOWNLOADBIT_END;
	}

	if(isthumbnail){
		rtn = (feed->download_thumbnailbitmap[whichbyte]>>whichbit) & 0x1;
	}
	else
		rtn = (feed->download_browsebitmap[whichbyte]>>whichbit) & 0x1;
	
QUREY_DOWNLOADBIT_END:
	return rtn;

}


unsigned long picasa_get_file_size(const char *filename)
{
	struct stat buf;
	if(stat(filename, &buf)<0){
		return 0;
	}
	return (unsigned long)buf.st_size;
}

/**
*@brief get the photo type and the photo name
*@param[in] filename	: the fullpath of the file or photo url
*@param[in] namebuf	: where the photo name will be stored, you should free the space by call picasa_free_photo_type()
*@return
	- -1 	: failed
	- others 	: photo type,  see picasaweb_photo_type_e
@see picasa_free_photo_type()
**/
int picasa_get_photo_type(const char* filename,char **namebuf)
{
	int filename_len=0;
	int rtn=0;
	char get_ext=0;
	int i=0;
	char fileext[5]="";
	filename_len = strlen(filename);
	if(filename_len!=0){
		for(i=filename_len-1;i>=0;i--){
			if(get_ext==0 && filename[i]=='.'){
				memcpy(fileext,filename+i+1,filename_len-1-i);
				get_ext = 1;
			}
			if(filename[i]=='/'){
				*namebuf = (char*)picasa_malloc(filename_len-i);
				if(*namebuf){
					memcpy(*namebuf,filename+i+1,filename_len-1-i);
					*(*namebuf+filename_len-i-1) = 0;
				}
				else{
					rtn = -1;
					goto GET_PHOTO_TYPE_END;
				}		
				break;
			}
		}
		if(get_ext==1 && i>=0){
			printf("fileext=%s,name=%s\n",fileext,*namebuf);
		}
	}

GET_PHOTO_TYPE_END:
	if(get_ext){
		if(strcasecmp(fileext,"jpg")==0 || strcasecmp(fileext,"jpeg")==0){
			rtn = PICASA_PHOTO_TYPE_JPEG;
		}
		else if(strcasecmp(fileext,"bmp")==0){
			rtn = PICASA_PHOTO_TYPE_BMP;
		}
		else if(strcasecmp(fileext,"gif")==0){
			rtn = PICASA_PHOTO_TYPE_GIF;
		}
		else if(strcasecmp(fileext,"png")==0){
			rtn = PICASA_PHOTO_TYPE_PNG;
		}
		else{
			picasa_info("%s,%d:Error File type=%s, not support yet!",__FILE__,__LINE__,fileext);
			rtn = -1;
		}
	}
	if(rtn==-1){
		if(*namebuf)
			picasa_free(*namebuf);
		*namebuf = NULL;
	}	
	return rtn;
}

/**
@brief call this fucntion to release the buffer which had malloced in the picasa_get_photo_type();
@param[in] filename : the pointer where the filename stored
@return always 0
**/
int picasa_free_photo_type(char* filename)
{
	if(filename){
		picasa_free(filename);
		filename = NULL;
	}
	return 0;
}



/**
@brief get the full path of the photo in the structure picasa_path_t
@param[in] path 	: the picasa_path_t which include the path
@param[in] fullpathbuf	: where to store the path
@return 
	- 0 failed
	- others : the length of the buffer had been used
***/
int picasa_get_file_path(picasa_path_t *path,char* fullpathbuf)
{
	int buf_used_len=0;
	if(path){
		if(path->cache_dir.data[path->cache_dir.data_len-2]=='/'){
			if(path->album_id.data)
				sprintf(fullpathbuf,"%s%s",path->cache_dir.data,path->album_id.data);
			else
					sprintf(fullpathbuf,"%s%s",path->cache_dir.data,path->photo_name.data);//it is feed_albums and not thumbnial
		}
		else{
			if(path->album_id.data)
				sprintf(fullpathbuf,"%s/%s",path->cache_dir.data,path->album_id.data);
			else
				sprintf(fullpathbuf,"%s/s%s",path->cache_dir.data,path->photo_name.data);//it is feed_albums and not thumbnial
		}

		if(path->album_id.data){
			if(path->album_id.data[path->album_id.data_len-2]=='/')
				sprintf(fullpathbuf,"%s%s",fullpathbuf,path->photo_name.data);
			else
				sprintf(fullpathbuf,"%s/%s",fullpathbuf,path->photo_name.data);
		}
		picasa_info("Path=%s\n",fullpathbuf);
		buf_used_len = strlen(fullpathbuf);
	}
	if(buf_used_len==0){
		picasa_err("Get File Path Error Something may be wrong!");
	}
	return buf_used_len;
}

/**
@brief check whether the file is exist, remember to call picasa_close_file() to release the resource that this function occupied
@param[in] path : the pointer to the picasa_path_t where the path of the file stored
@param[out] status : where the status returned, 0: the if is not exist, and had been created, 
				PICASA_FILE_EXIST: the file is exist,
				PICASA_FILE_CREATE_FAIL:the file is not exist, created failed,
				PICASA_FILE_BUFFER_ERR: the buffer for the fullpath is short
@return
 	- NULL: the file is exist or create failed
	- others: the handle of the file which had been created  
@see picasa_close_file()
**/
void * picasa_open_file(picasa_path_t *path,int *err_status)
{
	char tmpbuf[TMP_BUF_LEN]="";
	void * fhandle=NULL;
	if(picasa_get_file_path(path,tmpbuf)==0){
		*err_status = PICASA_FILE_BUFFER_ERR;
		return NULL;
	}
	fhandle = picasa_fopen(tmpbuf,"rb");
	if(fhandle){//the file is exit,do not need download
		*err_status = PICASA_FILE_EXIST;
		picasa_fclose(fhandle);
		return NULL;
	}
	else{
		fhandle = picasa_fopen(tmpbuf,"wb+");
		if(fhandle==NULL){
			*err_status = PICASA_FILE_CREATE_FAIL;
			return NULL;
		}
	}
	*err_status = 0;
	return fhandle;
	
}

/**

**/
int picasa_close_file(void * fp)
{
	if(fp){
		picasa_fclose(fp);
	}
	return 0;
}

/**
@biref checking the path ,if it is not exit, make it
**/
int picasa_check_path(picasa_path_t *path,int isthumbnail)
{	
	int rtn=PICASA_PATH_OK;
	char tmpbuf[TMP_BUF_LEN]="";
	char tmpalbumid[64]="";	
	char *tmpstr=NULL;
	if(path==NULL){
		rtn =  PICASA_PATH_CACHEDIR_ERROR;
		goto PICASA_PATH_CHECK_END;
	}
	rtn = __picasa_create_dir(path->cache_dir.data);
	if(rtn!=0){
		rtn =  PICASA_PATH_CACHEDIR_ERROR;
		goto PICASA_PATH_CHECK_END;
	}
	if(isthumbnail){//should check the album_id first
		tmpstr = strstr(path->album_id.data,"/");
		if(tmpstr){
			memcpy(tmpalbumid,path->album_id.data,tmpstr-path->album_id.data+1);
		}
		else{
			if(strcmp(path->album_id.data,"thumbnail")!=0){//it is not feed_albums
				rtn =  PICASA_PATH_ALBUMID_ERROR;
				goto PICASA_PATH_CHECK_END;
			}
		}
		if(path->cache_dir.data[path->cache_dir.data_len-2]=='/'){
			sprintf(tmpbuf,"%s%s",path->cache_dir.data,tmpalbumid);
		}
		else
			sprintf(tmpbuf,"%s/%s",path->cache_dir.data,tmpalbumid);
		picasa_info("PATH THUMBNAIL=%s\n",tmpbuf);
		rtn = __picasa_create_dir(tmpbuf);
		if(rtn!=0){
			rtn =  PICASA_PATH_ALBUMID_ERROR;
			goto PICASA_PATH_CHECK_END;
		}
	
	}
	if(path->album_id.data){
		if(path->cache_dir.data[path->cache_dir.data_len-2]=='/'){
			sprintf(tmpbuf,"%s%s",path->cache_dir.data,path->album_id.data);
		}
		else
			sprintf(tmpbuf,"%s/%s",path->cache_dir.data,path->album_id.data);
		picasa_info("PATH=%s\n",tmpbuf);
		rtn = __picasa_create_dir(tmpbuf);
	}
	if(rtn!=0){
		rtn =  PICASA_PATH_ALBUMID_ERROR;
		goto PICASA_PATH_CHECK_END;
	}	
PICASA_PATH_CHECK_END:
	picasa_info("%s,%d:Rtn=%d\n",__FILE__,__LINE__,rtn);
	return rtn;
}
#endif
