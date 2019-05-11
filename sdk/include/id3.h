#ifndef ID3_H
#define ID3_H


typedef struct {
    unsigned int length;
    char* content;
}item_info_t;

typedef struct {
	item_info_t author;
	item_info_t composer;
	item_info_t album;
	item_info_t pic;
	item_info_t genre;
	item_info_t year;

	item_info_t title;
	item_info_t band;
	item_info_t conductor;
	item_info_t track;

	char image_type[5];
}MUSIC_INFO;



#define MASK_AUTHOR		0x1		//TPE1,Lead performer(s)/Soloist(s)
#define MASK_COMPOSER	0x2		//TCOM,Composer
#define MASK_ALBUM		0x4		//TALB,Album/Movie/Show title ;APIC,Attached picture
#define MASK_GENRE		0x8		//TCON,Content type,such as"Blues","ClassicRock","Country","Dance
#define MASK_YEAR		0x10	//TYER,Year

#define MASK_TITLE		0x20	//TIT2,Title/songname/content description
#define MASK_BAND		0x40	//TPE2,Band/orchestra/accompaniment
#define MASK_CONDUCTOR	0x80	//TPE3,Conductor/performer refinement
#define MASK_TRACK		0x100	//TRCK,Track number/Position in set


void  get_audio_info(void * handle,MUSIC_INFO*  MINFO ,unsigned int mode );
void id3_free(MUSIC_INFO*  MINFO);

#endif