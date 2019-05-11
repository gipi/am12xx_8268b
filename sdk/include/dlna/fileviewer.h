#ifndef _FILEVIEWER_H_
#define _FILEVIEWER_H_

/*
 *    - @c ar-ar   Arabic
 *    - @c az-cyrl-az  Azeri, Cyrillic - Azerbaijan
 *    - @c az-latn-az  Azeri, Latin - Azerbaijan
 *    - @c bn-in   Begali - India
 *    - @c bg-bg   Bulgarian - Bulgaria
 *    - @c cs-cz   Czech - Czech Republic
 *    - @c da-dk   Danish - Denmark
 *    - @c de-de   German - Germany
 *    - @c el-gr   Greek - Greece
 *    - @c en-gb   English - United Kingdom
 *    - @c en-us   English - United States
 *    - @c es-co   Spanish - Colombia
 *    - @c es-do   Spanish - Dominican Republic
 *    - @c es-es   Spanish - Spain
 *    - @c et-ee   Estonian - Estonia
 *    - @c fi-fi   Finnish - Finland
 *    - @c fr-ca   French - Canada
 *    - @c fr-fr   French - France
 *    - @c gu-in   Gujarati - India
 *    - @c he-il   Hebrew - Israel
 *    - @c hi-in   Hindi - India
 *    - @c hr-hr   Croatian - Croatia
 *    - @c hu-hu   Hungarian - Hungary
 *    - @c it-it   Italian - Italy
 *    - @c ja-jp   Japanese - Japan
 *    - @c kk-kz   Kazakh - Kazakhstan
 *    - @c kn-in   Kannada - India
 *    - @c ko-kr   Korean - Korea
 *    - @c lt-lt   Lithuanian - Lithuania
 *    - @c lv-lv   Latvian - Latvia
 *    - @c ml-in   Malayalam - India
 *    - @c mr-in   Marathi - India
 *    - @c nl-nl   Dutch - The Netherlands
 *    - @c no-no   Norwegian
 *    - @c or-in   Oriya - India
 *    - @c pa-in   Punjabi - India
 *    - @c pl-pl   Polish - Poland
 *    - @c pt-br   Portuguese - Brazil
 *    - @c pt-pt   Portuguese - Portugal
 *    - @c ro-ro   Romanian - Romania
 *    - @c ru-ru   Russian - Russia
 *    - @c sk-sk   Slovak - Slovakia
 *    - @c sl-si   Slovenian - Slovenia
 *    - @c sv-se   Swedish - Sweden
 *    - @c te-in   Telugu - India
 *    - @c ta-in   Tamil - India
 *    - @c th-th   Thai - Thailand
 *    - @c tr-tr   Turkish - Turkey
 *    - @c uk-ua   Ukrainian - Ukraine
 *    - @c vi-vn   Vietnamese - Vietnam
 *    - @c zh-cn   Chinese - China
 *    - @c zh-tw   Chinese - Taiwan
 */

#define GROUP_EUROPE_AND_MIDDLE_EAST	0
#define GROUP_EASTERN_LANGUAGES			1
#define GROUP_INDIC_NORTH				2
#define GROUP_INDIC_SOUTH				3

#define FV_PAGE_FIT_NONE				0
#define FV_PAGE_FIT_SCREEN				1
#define FV_PAGE_FIT_WIDTH				2

#if defined(__cplusplus)
extern "C" {
#endif /* (__cplusplus) */

typedef void (*FV_SCREEN_UPDATE_RGB565_FUNC)(void *handle,
											void *buffer,
											unsigned int width,
											unsigned int height,
											unsigned int widthBytes,
											unsigned int updateX,
											unsigned int updateY,
											unsigned int updateWidth,
											unsigned int updateHeight);
typedef struct _fv_init_param_
{
	void *fv_screen_handle;
	FV_SCREEN_UPDATE_RGB565_FUNC fv_screen_update_rgb565;

	unsigned int frame_buf_width;
	unsigned int frame_buf_height;

	void *heap_buffer;
	unsigned int heap_size;

	char locale_set[8];
	int group;

	int page_fit;

	void (*notify_information)(const char*);

}FV_INIT_PARAM;


int fv_init(FV_INIT_PARAM *param);
int fv_release();
int fv_open_file(char *fname);
int fv_close_file();
int fv_next_page();
int fv_prev_page();
int fv_first_page();
int fv_last_page();
int fv_goto_page(int page_num);
int fv_page_up();
int fv_page_down();
int fv_zoom_in();
int fv_zoom_out();
int fv_fit_width();
int fv_fit_height();
int fv_fit_screen();
int fv_move_up();
int fv_move_down();
int fv_move_left();
int fv_move_right();
int fv_rotate();

int fv_get_page_num(int *page_num, int *total_pages);
int fv_get_zoom(int *zoom, int *zoom_min, int *zoom_max);

void fv_get_version_info(char *v,
						 int size_v,
						 char *i,
						 int size_i,
						 char *c,
						 int size_c);

#if defined(__cplusplus)
}
#endif /* (__cplusplus) */

#endif//_FILEVIEWER_H_


