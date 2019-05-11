#include "act_picasa_hash.h"
#include "act_picasa_common.h"
#define LOWER_CASE(c) (((c)>='A'&&(c)<='Z')?((c)+'a'-'A'):(c))

struct picasa_hash_table_tagname __picasa_tagname_table[PICASA_HASH_TABLE_SIZE];

int picasa_hash_calc(char *str,int *sum)
{
    	unsigned int hash = 1315423911; // nearly a prime - 1315423911 = 3 * 438474637
	char c;

	*sum=0;
	while ((c = *str)){
		hash ^= ((hash << 5) + LOWER_CASE(c) + (hash >> 2));
		*sum +=c;
		str++;
	}
    	return (hash % PICASA_HASH_TABLE_SIZE);
}


void picasa_hash_addItem(struct picasa_hash_table_tagname * table, int item,char *str)
{
	unsigned char c=0;
	int base=0;
	int hash = picasa_hash_calc(str,&base);
	while(table[hash].full >= 0){
		hash = (hash + 1) % PICASA_HASH_TABLE_SIZE;
	}
	table[hash].full = hash;
	table[hash].base = base;
	table[hash].item = item;
	//printf("Item=%d,base=%d,full=%d\n",item,base,hash);
}

int picasa_hash_lookup(struct picasa_hash_table_tagname * table,char * str)
{
	int base;
	int hash = picasa_hash_calc(str,&base);
	while(table[hash].full >= 0){
		if(table[hash].full == hash && table[hash].base==base)
			return hash;
		hash = (hash + 1) % PICASA_HASH_TABLE_SIZE;
	}
	return -1;
}

void * picasa_hash_getItem(struct picasa_hash_table_tagname * table, int i)
{
	return (void*)table[i].item;
}

void picasa_hash_removeItem(struct picasa_hash_table_tagname * table, int i)
{
	table[i].full = -1;
}


void picasa_init_hash_table(struct picasa_hash_table_tagname * table,int tablesize)
{
	int i;
	for(i=0;i<tablesize;i++){
		table[i].full = -1;
		table[i].base = -1;
	}
}

/**
@brief call this function to initializing the hash table for extracting the entry infomation 
@param[in] NULL
@return NULL
**/
void picasa_init_tag_table(void)
{
	struct picasa_hash_table_tagname * tmp = __picasa_tagname_table;
	
	picasa_init_hash_table(tmp,PICASA_HASH_TABLE_SIZE);
	
	///< namespace
	picasa_hash_addItem(tmp,NS_ATOM,"http://www.w3.org/2005/Atom");
	picasa_hash_addItem(tmp,NS_GPHOTO,"http://schemas.google.com/photos/2007");
	picasa_hash_addItem(tmp,NS_MEDIA,"http://search.yahoo.com/mrss/");
	picasa_hash_addItem(tmp,NS_GEORSS,"http://www.georss.org/georss");

	///both atom and gphoto
	picasa_hash_addItem(tmp,NODE_ID,"id");
	picasa_hash_addItem(tmp,NODE_ALBUMID,"albumid");
	///both atom and media
	picasa_hash_addItem(tmp,NODE_CONTENT,"content");
	picasa_hash_addItem(tmp,NODE_TITLE,"title");

	///< for atom space
	picasa_hash_addItem(tmp,NODE_LINK,"link");
	picasa_hash_addItem(tmp,NODE_SUMMARY,"summary");

	picasa_hash_addItem(tmp,NODE_AUTHOR,"author");
	picasa_hash_addItem(tmp,NODE_AUTHOR_NAME,"name");
	picasa_hash_addItem(tmp,NODE_AUTHOR_EMAIL,"email");
	picasa_hash_addItem(tmp,NODE_AUTHOR_URI,"uri");
	picasa_hash_addItem(tmp,NODE_CATEGORY,"category");
	
	picasa_hash_addItem(tmp,NODE_PUBLIC_DATE,"published");
	picasa_hash_addItem(tmp,NODE_UPDATE_DATE,"updated");
	picasa_hash_addItem(tmp,NODE_EDIT_DATE,"edited");
	picasa_hash_addItem(tmp,NODE_RIGHTS,"rights");

	picasa_hash_addItem(tmp,CATEGORY_ATTR_TERM,"term");
	picasa_hash_addItem(tmp,CATEGORY_ATTR_SCHEME,"shceme");
	picasa_hash_addItem(tmp,LINK_ATTR_REL,"rel");
	picasa_hash_addItem(tmp,LINK_ATTR_TYPE,"type");
	picasa_hash_addItem(tmp,LINK_ATTR_HREF,"href");
	

	///< for gphoto space
	picasa_hash_addItem(tmp,NODE_PHOTO_LOCATION,"location");
	picasa_hash_addItem(tmp,NODE_PHOTO_ACCESS,"access");
	picasa_hash_addItem(tmp,NODE_PHOTO_TIMESTAMP,"timestamp");
	picasa_hash_addItem(tmp,NODE_PHOTO_NUMPHOTOS,"numphotos");
	picasa_hash_addItem(tmp,NODE_PHOTO_NUMPHOTOREMAIN,"numphotosremaining");
	picasa_hash_addItem(tmp,NODE_PHOTO_BYTESUSED,"bytesUsed");
	picasa_hash_addItem(tmp,NODE_PHOTO_USER,"user");
	picasa_hash_addItem(tmp,NODE_PHOTO_NICHNAME,"nickname");
	
	///< for media space
	picasa_hash_addItem(tmp,NODE_GROUP,"group");
	picasa_hash_addItem(tmp,NODE_GROUP_CREDIT,"credit");
	picasa_hash_addItem(tmp,NODE_GROUP_DESC,"description");
	picasa_hash_addItem(tmp,NODE_GROUP_KEYWORDS,"keywords");
	picasa_hash_addItem(tmp,NODE_THUMBNAIL,"thumbnail");
	
	///< for georss space
	picasa_hash_addItem(tmp,NODE_WHERE,"where");
	picasa_hash_addItem(tmp,NODE_POINT,"Point");
	picasa_hash_addItem(tmp,NODE_POS,"pos");

	picasa_hash_addItem(tmp,NODE_SUBTITLE,"subtitle");
	picasa_hash_addItem(tmp,NODE_TOTALRESULT,"totalResults");
	picasa_hash_addItem(tmp,NODE_STARTINDEX,"startIndex");
	picasa_hash_addItem(tmp,NODE_ITEMSPERPAGE,"itemsPerPage");
	picasa_hash_addItem(tmp,NODE_QUOTALIMIT,"quotalimit");
	picasa_hash_addItem(tmp,NODE_QUOTACURRENT,"quotacurrent");
	picasa_hash_addItem(tmp,NODE_MAXPHOTOSPERALBUM,"maxPhotosPerAlbum");


	picasa_hash_addItem(tmp,NODE_TAGS,"tags");
	picasa_hash_addItem(tmp,NODE_FSTOP,"fstop");
	picasa_hash_addItem(tmp,NDDE_MAKE,"make");
	picasa_hash_addItem(tmp,NODE_MODEL,"model");
	picasa_hash_addItem(tmp,NODE_EXPOSURE,"exposure");
	picasa_hash_addItem(tmp,NODE_FLASH,"flash");
	picasa_hash_addItem(tmp,NODE_FOCALLENGTH,"focallength");
	picasa_hash_addItem(tmp,NODE_ISO,"iso");
	picasa_hash_addItem(tmp,NODE_TIME,"time");
	picasa_hash_addItem(tmp,NODE_IMAGEUNIQUEID,"imageUniqueID");
	picasa_hash_addItem(tmp,NODE_DISTANCE,"distance");
}


int picasa_get_tagindex(char* str)
{
	int index=-1;
	int hash;
	struct picasa_hash_table_tagname * tmp = __picasa_tagname_table;
	
	hash=picasa_hash_lookup(tmp,str);
	if(hash!=-1){
		index = (int)picasa_hash_getItem(tmp,hash);
	}
	return index;
}
