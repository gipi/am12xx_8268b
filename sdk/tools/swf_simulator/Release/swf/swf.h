#ifndef SWF_H
#define SWF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "swf_opt.h"
#include "swf_base.h"
#include "swf_io.h"
#include "swf_dec.h"
#include "swf_displaylist.h"
#include "swf_render.h"
#include "swf_time.h"
#include "swf_mem.h"
#include "swf_system.h"
#include "msprintf.h"
	
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#ifndef NULL
#define NULL (void*)0
#endif

/************************************************************************/
/* Special Macros                                                       */
/************************************************************************/
#define QSORT	swf_qsort
#define STRCMPI    strcasecmp
#define STRCPY(_d,_s) StrNCpy(_d,_s,AS_MAX_TEXT)
#define STRCAT(_d,_s) StrNCat(_d,_s,AS_MAX_TEXT)
#ifdef MIPS_VERSION
#define MEMCPY     memcpy
#define MEMSET     memset
#else
#define MEMCPY     memcpy
#define MEMSET     memset
#endif

/************************************************************************/
/* Method Declearation                                                  */
/************************************************************************/

//Defined in swf_shape.c
void swf_shape_define(SWF_DEC * s);
void swf_shape_remove(SHAPE * shape);
void swf_shape_scan(SWF_DEC * s, DISPLAYNODE * pDisplayNode, SHAPE * pShape);
void swf_shape_line(SWF_DEC * s, SWF_MATRIX * T, SWF_CXFORM * CxForm, SUBSHAPE * SubShape, INT8U * zbuffer, int depth);
int  swf_shape_empty(SWF_DEC * s);
int  swf_shape_rectangle(SWF_DEC * s, SWF_RECT * rect);
int  swf_is_shape_inside_window(SWF_RECT * window, SWF_RECT * r, SWF_MATRIX * T);

void Initial_RECT(SWF_RECT * pRect);
void Initial_Minimal_RECT(SWF_RECT * pRect);
void Initial_Identify_MATRIX(SWF_MATRIX * pMatrix);
void Initial_MATRIX(SWF_MATRIX * pMatrix);
void Initial_CXFORM(SWF_CXFORM * colortransform);
void MatrixMul(SWF_MATRIX * T0, SWF_MATRIX * T, SWF_MATRIX * T1);
void MatrixInv(SWF_MATRIX * T0, SWF_MATRIX * T1);
void CxFormMul(SWF_CXFORM * C0, SWF_CXFORM * C, SWF_CXFORM * C1);
int  fpMul(int faciend,int multiplier);
int  fpDiv(int dividend,int divisor);

int  strcasecmp(const char * cs,const char * ct);
char * StrNCpy(char *dest,const char *src,unsigned int count);
char * StrNCat(char *dest,const char *src,unsigned int count);
INT8S * StrCaseStr(const INT8S * s1,const INT8S * s2);
void swf_qsort(void *aa, unsigned int n, unsigned int es, int (*cmp)(const void *, const void *));

#define SWF_PLAY_RELOAD		0x01
#define SWF_PLAY_ACTION		0x02

//Defined in swf_displaylist.c
DISPLAYNODE * swf_create_displaynode(SWF_DEC * s, DISPLAYLIST * displayList, int Id, int depth, char * name);
DISPLAYLIST * swf_alloc_displaylist(DISPLAYNODE * parentNode, int frameCount);
void swf_remove_displayList(DISPLAYLIST * displayList);
void swf_remove_displayNode(DISPLAYNODE * displayNode);
void swf_remove_node(DISPLAYNODE * displayNode);
void swf_remove_list(DISPLAYLIST * displayList);
void swf_dec_clear_evironment(SWF_DEC * s);
void swf_set_changeflag(DISPLAYNODE * pDisNode, int flag);
void swf_get_next_focus(SWF_DEC * s, int key);
void swf_update_invalidate_node(DISPLAYNODE * displayNode);
void swf_update_node_rect(DISPLAYNODE * pDisNode, SWF_RECT * rect);
void swf_update_node_rect_exceptRoot(DISPLAYNODE * pDisNode, SWF_RECT * rect);
void swf_calculate_dispMatrix(DISPLAYNODE * pDisNode);
void swf_calculate_dispMatrix_exceptRoot(DISPLAYNODE * pDisNode);
void swf_register_listener(SWF_DEC * s, DISPLAYNODE * displayNode, int flag, int (*proc)(LISTENER * listener, int action, void* param));
void swf_unregister_listener(DISPLAYNODE * displayNode);
void swf_every_frame_event(SWF_DEC * s);
void swf_dec_clear_msg_queue();

void swf_create_walk_list(PBLIST src, PSHEAD dst);
void swf_remove_walk_list(PSHEAD head);
void swf_add_walk_list(PBLIST src, PSHEAD dst);

DISPLAYNODE * swf_alloc_displaynode(SWF_DEC * s);
int  swf_insert_displaynode(DISPLAYNODE * pDisNode, DISPLAYLIST * displayList, int copy_attr);
DISPLAYNODE * swf_duplicate_displaynode(DISPLAYNODE * pDisNode, char * name, int depth);
DISPLAYNODE * swf_attach_displaynode(SWF_DEC * s, DISPLAYLIST * displayList, char * id, char * name, int depth, AS_OBJECT * init);
void swf_set_mask_node(DISPLAYNODE * node, DISPLAYNODE * mask);
int  swf_is_node_enabled(DISPLAYNODE * node);
void swf_update_mask_displayNode(DISPLAYNODE * displayNode, int val);

void swf_remove_all_scanline(SWF_DEC * s, DISPLAYLIST * displayList);
void swf_update_dispMatrix(DISPLAYNODE * pDisNode);

int  swf_scan_frame(SWF_DEC * s);
void swf_show_frame(SWF_DEC * s);

int  swf_build_one_sprite(DISPLAYLIST * displayList, int firstTime, int forceAction);
int  swf_build_sprite(DISPLAYLIST * displayList);
int  swf_build_action(DISPLAYLIST * displayList);
int  swf_goto_frame(DISPLAYLIST * displayList, int n);
int  swf_goto_frame_with_action(DISPLAYLIST * displayList, int n);
int  swf_get_bounds(DISPLAYNODE * pDisNode, SWF_RECT * rect);
int swf_get_bounds_childs(DISPLAYNODE * pDisNode, SWF_RECT * rect, SWF_MATRIX * T);
int  swf_execute_action_parent_first(DISPLAYLIST * displayList);

void swf_place_object(SWF_DEC * s, TAG * tag, DISPLAYLIST * displayList);
void swf_place_object23(SWF_DEC * s, TAG * tag, DISPLAYLIST * displayList);
void swf_remove_object(SWF_DEC * s, TAG * tag, DISPLAYLIST * displayList);
void swf_remove_object2(SWF_DEC * s, TAG * tag, DISPLAYLIST * displayList);
void swf_protect(SWF_DEC * s, TAG * tag, DISPLAYLIST * displayList);

//Defined in swf_sprite.c
void swf_frame_label_define(SWF_DEC * s, int tagLength, FRAME * frame);
void swf_sprite_define(SWF_DEC * s);
void swf_sprite_remove(SPRITE * pSprite);

//Defined in swf_bitmap.c
void swf_get_original_size(SWF_BITMAP * pBitmap);
void swf_jpeg_decode(SWF_DEC * s, int id, SWF_BITMAP * pBitmap, int mode);
void swf_update_bitmap_cache(SWF_DEC * s, DISPLAYLIST * displayList);
void swf_JPEGTables_define(SWF_DEC * s);
void swf_bitsJPEG_define(SWF_DEC * s);
void swf_bitsJPEG2_define(SWF_DEC * s);
void swf_bitsJPEG3_define(SWF_DEC * s);
void swf_bitsLossless_define(SWF_DEC * s);
void swf_bitmap_remove(int id, SWF_BITMAP * bitmap);
int  swf_bitmap_create(SWF_DEC * s, int format);

//Defined in swf_morph.c
void swf_morph_define(SWF_DEC * s);
void swf_morph_remove(MORPH * pMorph);
void swf_morph_scan(SWF_DEC * s, DISPLAYNODE * pDisplayNode, MORPH * pMorph);
void swf_morph_line(SWF_DEC * s, SWF_MATRIX * T, SWF_CXFORM * CxForm, int Ratio, MORPH * pMorph, unsigned char * zbuffer, int depth);

//Defined in swf_font.c
void swf_font_define(SWF_DEC * s);
void swf_font_remove(FONT * pFont);
void swf_fontinfo_define(SWF_DEC * s);
void swf_font2_define(SWF_DEC * s);

//Defined in swf_text.c
void swf_text_define(SWF_DEC * s);
void swf_edit_define(SWF_DEC * s);
void swf_text_remove(SWF_TEXT * pText);
void swf_edit_remove(SWF_EDIT * pEdit);
void swf_text_device_render(SWF_DEC * s, SWF_MATRIX * T, SWF_CXFORM * CxForm, SWF_TEXT * pText, unsigned char * zbuffer, int depth);
void swf_text_glyph_render (SWF_DEC * s, SWF_MATRIX * T, SWF_CXFORM * CxForm, SWF_TEXT * pText, unsigned char * zbuffer, int depth);
void swf_text_update(DISPLAYNODE * pTextNode, SWF_MATRIX * newDispMatrix);

void swf_edit_set_text(DISPLAYNODE * pDisNode, char * text);
void swf_edit_replace_text(DISPLAYNODE * pDisNode, char * text, int beginIndex, int endIndex);
char * swf_edit_get_text(DISPLAYNODE * pDisNode);
void swf_edit_set_textColor(DISPLAYNODE * pDisNode, INT32U color);
int  swf_edit_get_textColor(DISPLAYNODE * pDisNode);
void swf_edit_render(SWF_DEC * s, SWF_MATRIX * T, SWF_CXFORM * CxForm, EditPrivate *editPriv, SWF_EDIT * edit, INT8U * zbuffer, int depth);
const char * swf_edit_get_autoSize(DISPLAYNODE *pDisNode);
void swf_edit_set_autoSize(DISPLAYNODE *pDisNode, char * autoSize);
int swf_edit_get_wordWrap(DISPLAYNODE *pDisNode);
void swf_edit_set_wordWrap(DISPLAYNODE *pDisNode, int wordWrap);
int swf_edit_get_embedFonts(DISPLAYNODE *pDisNode);
void swf_edit_set_embedFonts(DISPLAYNODE *pDisNode, int embedFonts);
EditPrivate * swf_edit_set_init_format(DISPLAYNODE * pDisNode);
int swf_edit_get_textWidth(char * text, int size);
int swf_edit_get_length(DISPLAYNODE *pDisNode);
int swf_edit_perform_autoSize(SWF_EDIT *edit, SWF_TextFieldFormat *TextFieldFormat, char * text, int* _width, int* _height, int* _x, int* _y);
int swf_edit_get_border(DISPLAYNODE *pDisNode);
void swf_edit_set_border(DISPLAYNODE *pDisNode, int border);
int swf_edit_get_selectable(DISPLAYNODE *pDisNode);
void swf_edit_set_selectable(DISPLAYNODE *pDisNode, int selectable);
int swf_edit_get_textWidthHeight(DISPLAYNODE *pDisNode, int * tw, int * th);
int swf_edit_get_maxscroll(DISPLAYNODE *pDisNode);
void swf_edit_set_scroll(DISPLAYNODE *pDisNode, int scroll);
int swf_edit_get_scroll(DISPLAYNODE *pDisNode);


//Defined in swf_dictionary.c
int  swf_get_dictionary_id(int Id);
DICTIONARY * swf_alloc_dictionary(SWF_DEC * s, unsigned int size, int Id);
void swf_remove_dictionary_list(SWF_DEC * s);
void swf_remove_dictionary(SWF_DEC * s, int Id);
void swf_remove_dictionary_res(SWF_DEC * s);
int  swf_alloc_dictinary_id(SWF_DEC * s);
void swf_free_dictinary_id(SWF_DEC * s, int id);

//Defined in swf_render.c
void swf_scanline_zbuffer(SWF_DEC * s, SCANLINE * scanline);
void swf_node_render(DISPLAYNODE * node, unsigned char * zbuffer, int depth);
void swf_pair_render(SWF_DEC * s, SCANLINE * scanline);
void swf_line_render(SWF_DEC * s, BLIST * pathList, SWF_MATRIX * T, unsigned int color, unsigned int width, unsigned char * zbuffer, int depth);
void swf_background_render(SWF_DEC * s);
void swf_focus_render(SWF_DEC * s, DISPLAYNODE * pDisNode, unsigned int color);
void swf_box_render(SWF_DEC * s, SWF_RECT * box, unsigned int color);
void swf_frame2screen_render(SWF_DEC * s, SWF_RECT * rect, int frame);
int  swf_update_buffer_layer(SWF_RECT * window, SHEAD * ScanLineList, SWF_MATRIX * T, unsigned char* layer, int stride, int curVal, int op);
void swf_set_fillinfo(SWF_DEC * s, FILLSTYLE * FillStyle, SWF_CXFORM * CxForm, SWF_MATRIX * T, FILLINFO * FillInfo);
void swf_register_ingore_win(SWF_CONTEXT * ctx, int x, int y, int w, int h);
void swf_unregister_ingore_win(SWF_CONTEXT * ctx);

//Defined in swf_render_low.c
void swf_render(RENDER_INFO * info);
void swf_render_fill(FILLINFO * fillinfo);
void swf_render_mask(SWF_DEC * s, MASKINFO * maskinfo);
void swf_render_text(SWF_DEC * s, TEXTINFO * textinfo);
void swf_render_edit(SWF_DEC * s, EDITINFO * editinfo);
void swf_render_line(SWF_DEC * s, LINEINFO * lineinfo);
void swf_render_box(SWF_DEC * s, BOXINFO * boxinfo);
unsigned int swf_color_low(SWF_DEC * s, unsigned int color);
void swf_point_low(SWF_DEC * s, unsigned int color, int x, int y, unsigned char * z_buffer, int depth);
void swf_line_low(SWF_DEC * s, unsigned int color, int x0, int y0, int x1, int y1, SWF_RECT * boundary, unsigned char * z_buffer, int depth);
void swf_text_low(SWF_DEC * s, INT32U color, EditPrivate * editPriv, int x,  int y, int x1, INT32U height, SWF_RECT * boundary, INT8U * z_buffer, int depth, int wordWrap, int	align, int leading, int	multiline);
void swf_box_low(SWF_DEC * s, BOXINFO * box);

//Defined in swf_geometry.c
void swf_init_simple_link(SIMPLE_LINK_HEAD * LinkHead);
void swf_destory_simple_link(SIMPLE_LINK_HEAD * LinkHead);
void swf_reset_simple_link(SIMPLE_LINK_HEAD * LinkHead);
SIMPLE_LINK * swf_alloc_simple_link(SIMPLE_LINK_HEAD * LinkHead);
void swf_add_point_to_link(SIMPLE_LINK_HEAD * LinkHead, int x, int y, int flag);

void swf_release_scanline(SHEAD * scanlineList);
void swf_insert_scanline(SWF_DEC * s, SHEAD * scanlineList, BLIST * polyPath, SWF_MATRIX * T, FILLSTYLE * fillstyle);
void swf_compute_masked_scanline(SWF_DEC * s, DISPLAYNODE * node);
void swf_compute_transform_matrix(SWF_DEC * s, SWF_MATRIX * T);
void swf_compute_color_transform(SWF_DEC * s, SWF_CXFORM * C);
void swf_apply_color_transform(SWF_DEC * s, SWF_CXFORM * C);
void swf_bezier(int x1,int y1,int x2,int y2,int x3,int y3,int div,int * bpNum,int * bpBuff);
int  swf_is_rect_inside_window(SWF_RECT * window, SWF_RECT * rect, SWF_MATRIX * T);
int  swf_is_valid_mask(DISPLAYNODE * node, DISPLAYNODE * mask);
void swf_trans_line2poly(BLIST * line, int width, BLIST * poly);


//Defined in swf_action.c
void swf_action_register_class(SWF_CONTEXT * ctx, int classNo, AS_CLASS * classObj);
ASCODE * swf_action_define(SWF_DEC * s, int asLength, FRAME * Frame, int Flag);
void swf_action_init_action(SWF_DEC * s);
void swf_action_remove(ASCODE * AsCode);
ASCONTEXT * swf_alloc_as_context(SWF_DEC * s, AS_OBJECT * object, ASCODE * code, ASCONTEXT * parent);
void swf_free_as_context(ASCONTEXT * as);
void swf_remove_vars(AS_OBJECT * obj);
int  swf_action_execute(SWF_DEC * s, AS_OBJECT * object, ASCODE * AsCode, ASCONTEXT * parent);
int swf_action_handle_event(SWF_DEC * s, DISPLAYNODE * pDisNode, int msg, void * param);
void swf_action_init_class_table(SWF_CONTEXT * ctx);
AS_VAR * swf_action_create_var(AS_OBJECT * object, SHEAD * varList, char * name);
AS_VALUE * swf_action_get_variable(char * varName, AS_OBJECT * object);
int swf_action_set_variable(AS_VALUE * asValue, char * varName, AS_OBJECT * object);
int swf_action_new_variable(AS_VALUE * asValue, char * varName, AS_OBJECT * object);
void swf_action_copy_value(AS_VALUE * src, AS_VALUE * dst);
void swf_action_dump_stack(ASCONTEXT * as, int n);
void swf_action_dump_value(AS_VALUE * asValue);
char * swf_action_dump_object(AS_OBJECT * object);
void swf_action_get_node_path(DISPLAYNODE * node, char * path);
DISPLAYNODE * swf_get_node_by_path(ASCONTEXT * as, char * path);
int swf_action_GetVariable(ASCONTEXT * as);
DISPLAYNODE * swf_action_find_node_by_name(char * name, DISPLAYNODE * displayNode);
void swf_as_array_free_elem(AS_ARRAY * array);
void swf_action_copy_local(AS_OBJECT * as, AS_OBJECT * target);

//Defined in swf_action_types.c
int swf_as_value_to_string (AS_VALUE * value, AS_STRING * str);
AS_NUMBER swf_as_value_to_number (AS_VALUE *value);
AS_NUMBER swf_as_float_to_number(unsigned char * fl);
AS_NUMBER swf_as_double_to_number(unsigned char * fl);
AS_NUMBER swf_as_double_to_number_be(INT8U * fl);
AS_NUMBER swf_as_int_to_number(unsigned char * fl);
AS_NUMBER swf_nan(void);
int swf_is_nan(AS_NUMBER x);
int swf_as_hash(char *str);
int swf_ftoa(INT8U * fl, char *s);
int swf_is_Infinity(AS_NUMBER x);
int swf_number_to_string(AS_NUMBER number, char * str, int radix);

//Defined in swf_enviroment.c
void swf_file_attributes(SWF_DEC * s);
void swf_export_assets(SWF_DEC * s);
void swf_import_assets(SWF_DEC * s);

//Defined in swf_action_xxx.c
int  swf_sprite_listener(LISTENER * listener, int action, void* param);
AS_OBJECT * swf_as_global_getGlobalObj();
void swf_as_global_timer(unsigned int n);
void swf_as_global_timer_free(AS_OBJECT * obj);
char * swf_as_global_findName(AS_OBJECT * container, AS_OBJECT * target);
AS_OBJECT * swf_as_global_findObject(char * path);
int  swf_as_default_skip_args(ASCONTEXT * as);
AS_VALUE * swf_as_array_getElem(AS_ARRAY * array, int i);
void swf_as_array_setElem(AS_ARRAY * array, AS_VALUE * value, int i);
int  swf_as_key_get();
BLIST * swf_as_keyListener_get();
BLIST * swf_as_mouseListener_get();
int  swf_as_key_set(int key);
void swf_as_key_reset();
void swf_as_key_remove(AS_OBJECT * obj);
void swf_as_mouse_remove(AS_OBJECT * obj);
int swf_as_mouse_set(int mouse);
int swf_as_external_call(ASCONTEXT * as, char * name);
void swf_as_object_init(SWF_DEC * s, AS_OBJECT * object, int Class, AS_OBJECT * proto);
AS_OBJECT * swf_as_object_alloc(SWF_DEC * s, int Class, int size, AS_OBJECT * proto);
void swf_as_object_free(AS_OBJECT * obj);
void swf_as_object_remove_ref(AS_OBJECT * obj);
SWF_DEC * swf_as_object_find_ref(AS_OBJECT * obj, SWF_DEC *inst);
void swf_as_object_remove_ref_from_inst(AS_OBJECT * obj, SWF_DEC * s);
void swf_as_object_remove_unused(SWF_DEC * s);
void swf_as_object_delete_event_definer(AS_OBJECT * target);
int swf_as_object_get(ASCONTEXT * as);
int swf_as_object_set(ASCONTEXT * as);
int swf_as_object_call(ASCONTEXT * as);
AS_FUNCTION * swf_as_function_alloc(SWF_DEC * s);
int swf_load_resouce(DISPLAYNODE * target, char * url, SWF_DEC *asSwf);
void swf_as_movieClipLoader_action(SWF_DEC * s, AS_MCLOADER * mcloader, DISPLAYNODE * pDisNode, char * eventStr);
char * swf_as_string_getBuffer();
void swf_remove_swf(DISPLAYNODE * target);
void swf_as_sound_complete();
void swf_as_sound_delete();
int swf_as_find_object_from_asheap2(SWF_DEC * s,AS_OBJECT *object);

//Defined in swf_stat.c
void counter_reset(int i);
void counter_reset_all();
void counter_add(int i, int v);
int counter_get(int i);

//Defined in swf_gradient.c
void swf_lineargrad2bitmap(SWF_DEC * s, FILL * Fill);
void swf_radialgrad2bitmap(SWF_DEC * s, FILL * Fill);

//Defined in swf_button.c
void swf_button_define(SWF_DEC * s);
void swf_button2_define(SWF_DEC * s);
void swf_button_build(SWF_DEC * s, DISPLAYNODE * displayNode);
void swf_button_remove(BUTTON * button);
void swf_button_sound_define(SWF_DEC * s);
int swf_button_listener(LISTENER * listener, int action, void* param);

//Defined in swf_dump.c
void swf_dump_displaylist(DISPLAYLIST * displayList, int n);
int  swf_dump_bitmap_memory(SWF_DEC * s);
int  swf_dump_scanline_memory(DISPLAYNODE * displaynode);
int  swf_dump_shape_memory(SWF_CONTEXT * ctx);
int  swf_dump_tag_memory(SWF_CONTEXT * ctx);

//Defined in swf_hash.c
int  swf_hash_calc(SWF_DEC * s, char * str);
void swf_hash_addItem(SWF_HASH * table, int base, char * str, void * item);
int  swf_hash_lookup(SWF_HASH * table, int base, char * str);
void*swf_hash_getItem(SWF_HASH * table, int i);
void swf_hash_removeItem(SWF_HASH * table, int i);

//Defined in swf_aa.c
int swf_aa_render(SWF_DEC * s, BLIST * polyPath, SWF_MATRIX * T, SWF_CXFORM * CxForm, FILLSTYLE * style, INT8U * zbuffer, int depth, SWF_RECT *AA_bounds);
void swf_aa_wideline(SWF_DEC * s, BLIST * LinePath, SWF_MATRIX * T, SWF_CXFORM * CxForm, INT32U width, INT32U Color, INT8U * zbuffer, int depth);

//Defined in swf_cache.c
int swf_cache_save_bmp(SWF_DEC * s, int Id);
int swf_cache_drop_bmp(SWF_CONTEXT * ctx, int heap_type);
void   swf_cache_take_bmp(SWF_DEC * s, int Id);
void   swf_cache_update_bmp(SWF_CONTEXT * ctx);

//Defined in swf_vvd.c
int swf_vvd_createTemplate(char * input, char * thumb, char * output);

//Defined in swf_sound.c
void swf_sound_define(SWF_DEC * s);
void swf_sound_start(SWF_DEC * s, TAG * tag, DISPLAYLIST * displayList);
void swf_sound_stream_head(SWF_DEC * s, TAG * tag, DISPLAYLIST * displayList);
void swf_sound_remove(SWF_SOUND * pSound);
void ParseSoundInfo(SWF_DEC * s, int Id, TAG * tag, SOUND_INFO * info);
void swf_sound_play(SWF_DEC * s, int Id, SOUND_INFO * sound_info, int nFrame);
void swf_sound_stream_block(SWF_DEC * s, TAG * tag, DISPLAYLIST * displayList);

int swf_get_full_path(char * base, char * url, char *full_path);
int swf_to_string(AS_ARRAY *array, char * buf, char *delimiter);
int swf_as_init_array(ASCONTEXT * as);

FLOAT32 date2long(AS_DATE * obj);
void long2date(AS_DATE * obj, AS_NUMBER tm);

void swf_init_text_format(SWF_TextFieldFormat *fieldFormat, SWF_EDIT * edit, EditPrivate * priv);
void swf_set_text_format(SWF_EDIT *pEdit, AS_TEXTFORMAT *asfmt, EditPrivate * priv, int beginIndex, int endIndex);

int swf_get_text_snapshot(AS_TEXTSNAPSHOT * snapshot);
int swf_flush_shared_object(AS_SHAREDOBJECT *shareObj);

void swf_edit_get_fieldWidthHeight(DISPLAYNODE * displayNode, SWF_RECT *rect);

int swf_process_keyboard_msg(SWF_DEC *s, char *text, int x, int y, int flag);
int swf_get_input_text(SWF_DEC *s, int event_type, int x, int y);

int swf_edit_get_leading(DISPLAYNODE * displayNode);
int swf_edit_get_font_size(DISPLAYNODE * displayNode);
int swf_edit_get_align(DISPLAYNODE * displayNode);
void swf_edit_set_align(DISPLAYNODE *pDisNode, int align);

int get_txt_index_by_cursor(char *text, int x_press, int y_press, int word_total, int w_edge, int font_size, int leading, int scroll);
int edit_text_calc(char * text);

void swf_enable_chinese(SWF_DEC *s, int flag);
int swf_swap_frame_buffer(SWF_DEC * s, int forced);

void swf_update_mouse_shape(SWF_DEC * s, int x, int y);
void Mouse_Move(int init, int x, int y, int lcd_width, int lcd_height);

void swf_edit_set_maxscroll(DISPLAYNODE *pDisNode, int maxscroll);	//special for input text
void swf_edit_set_hscroll(DISPLAYNODE *pDisNode, int hscroll);
int swf_edit_get_hscroll(DISPLAYNODE *pDisNode);

int swf_load_variables_num(AS_OBJECT * target, char * url);

#ifdef __cplusplus
}
#endif
#endif

