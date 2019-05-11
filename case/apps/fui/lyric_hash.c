#ifdef MODULE_CONFIG_LYRICS


#include "lyric_analysis.h"


struct hash_table_tagname __tagname_table[HASH_TABLE_SIZE];

int hash_calc(char *str,int *sum)
{
    unsigned int hash = 1315423911; // nearly a prime - 1315423911 = 3 * 438474637
	char c;
	
	*sum=0;
	while ((c = *str)){
		hash ^= ((hash << 5) + LOWER_CASE(c) + (hash >> 2));
		*sum +=c;
		str++;
	}
    	return (hash % HASH_TABLE_SIZE);
}

void hash_addItem(struct hash_table_tagname * table, int item,char *str)
{
	unsigned char c=0;
	int base=0;
	int hash = hash_calc(str,&base);
	while(table[hash].full >= 0){
		hash = (hash + 1) % HASH_TABLE_SIZE;
	}
	table[hash].full = hash;
	table[hash].base = base;
	table[hash].item = item;
	//printf("Item=%d,base=%d,full=%d\n",item,base,hash);
}

int hash_lookup(struct hash_table_tagname * table,char * str)
{
	int base;
	int hash = hash_calc(str,&base);
	
	while(table[hash].full >= 0){
		if(table[hash].full == hash && table[hash].base==base)
			return hash;
		hash = (hash + 1) % HASH_TABLE_SIZE;
	}
	return -1;
}

void * hash_getItem(struct hash_table_tagname * table, int i)
{
	return (void*)table[i].item;
}

void hash_removeItem(struct hash_table_tagname * table, int i)
{
	table[i].full = -1;
}

void init_hash_table(struct hash_table_tagname * table,int tablesize)
{
	int i;
	for(i=0;i<tablesize;i++){
		table[i].full = -1;
		table[i].base = -1;
	}
}

void init_tag_table(void)
{
	struct hash_table_tagname * tmp = __tagname_table;
	
	init_hash_table(tmp,HASH_TABLE_SIZE);
	hash_addItem(tmp,LYRIC_TAG_AL,"al");
	hash_addItem(tmp,LYRIC_TAG_AR,"ar");
	hash_addItem(tmp,LYRIC_TAG_BY,"by");
	hash_addItem(tmp,LYRIC_TAG_OFFSET,"offset");
	hash_addItem(tmp,LYRIC_TAG_RE,"re");
	hash_addItem(tmp,LYRIC_TAG_TI,"ti");
	hash_addItem(tmp,LYRIC_TAG_VE,"ve");
}

int get_tag_index(char* str)
{
	int index=-1;
	int hash;
	struct hash_table_tagname * tmp = __tagname_table;
	
	hash=hash_lookup(tmp,str);
	if(hash!=-1){
		index = (int)hash_getItem(tmp,hash);
	}
	return index;
}


#endif

