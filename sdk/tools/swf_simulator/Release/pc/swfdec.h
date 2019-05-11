#ifndef FPLAYER_H
#define FPLAYER_H

#include "swf_types.h"
#include "act_plugin.h"
#include "swf_2d.h"

enum{
	SWF_GETINFO_WIDTH=0,
	SWF_GETINFO_HEIGHT=1
};

#ifdef __cplusplus
extern "C" {
#endif

int SWF_Context_Init(void * param);
int SWF_Context_Release(void);

SWF_DEC * SWF_AddInst(char * filename, SWF_RECT * window, int flag, SWF_INSTINFO * info);
void      SWF_RemoveInst(SWF_DEC * s);
SWF_DEC * SWF_GetActiveInst(void);
SWF_CONTEXT * SWF_GetContext(void);

int       SWF_Message(SWF_DEC * s, int msg, void * param);
int       SWF_Dispatch(void);
int       SWF_Prepare(SWF_DEC * s);
void      SWF_Update(SWF_DEC * s);
int       SWF_Load(SWF_DEC * s);
void      SWF_SetState(SWF_DEC * s, int state);
int       SWF_GetState(SWF_DEC * s);
void      SWF_SetWindow(SWF_DEC * s, SWF_RECT * window, int flag);
void *    SWF_Malloc(unsigned int size);
void *    SWF_StaticMalloc(unsigned int size);
void      SWF_Free(void * p);

void      SWF_RegisterExtCall(char * name, int classNo, int (*func)(void * h));
int       SWF_GetParamNum(void * h);
int       SWF_GetParamType(void * h);
AS_NUMBER SWF_GetNumber(void * h);
char *    SWF_GetString(void * h);
void *    SWF_GetObject(void * h);
void *    SWF_AllocObject(unsigned int size);
void      SWF_AddStringToObject(void * h, void * object, char * name, char * str);
void	  SWF_AddNumberToObject(void * h, void * object, char * name, AS_NUMBER number);
int       SWF_SetMember(void * h);
int       SWF_GetMember(void * h);
int       SWF_DeleteMember(void * h, void * object, char * name);
void      SWF_PutNumber(void * h, AS_NUMBER n);
void      SWF_PutString(void * h, char * str);
void      SWF_PutObject(void * h, void * obj);
void      SWF_PutNull(void * h);
char *    SWF_GetFilePath(void * h);

void      SWF_AttachBitmap(void * target, INT32U* pixel, INT32S width, INT32S height, INT32S dec_w, INT32S dec_h, INT32S stride, INT32S format, INT32S (*GetBitmapStatus)(INT32S));
void      SWF_DetachBitmap(void * target, INT32S clone);

void      SWF_SetDeviceFunction(SWF_DEC * s, SWF_DEV_FUNC * dev_func);
void      SWF_GetDeviceFunction(SWF_DEC * s, SWF_DEV_FUNC * dev_func);

int       SWF_CreateVividTemplate(char * swf_file, char * thumb, char * vt_file);
void      SWF_LoadMovie(void * target, char * url, INT32S width, INT32S height, SWF_DEV_FUNC * func);

void      SWF_RegisterIngoreWin(int x, int y, int w, int h);   
void      SWF_UnregisterIngoreWin(void);
void      SWF_2DFill(FILLINFO * info);

SWF_CONTEXTINFO * SWF_GetContextInfo(void);

int       SWF_MemCurrent();
int       SWF_MemTotal();
int       SWF_MemBasicCurrent();
int       SWF_MemBasicTotal();
int       SWF_MemCheck(int dump);

INT32U	SWF_GetCurrentFrameRate();
int	SWF_GetLatestInstName(char *name);
void keyboard_show(int flag);
void Enable_Chinese(int flag);
void SWF_SetTextColor(void* obj, unsigned int nIndexBegin, unsigned int nIndexEnd, unsigned int color );
void SWF_getTextCount( char* str, unsigned int strCount, unsigned int * engCount, unsigned int * chnCount );

int Gb2312ToUni(unsigned short * pOut, int OutLen, char *pIn, int InLen);
void Gb2312ToUtf8(char * pOut,char *pText, int pLen);
void Utf8ToGb2312(char *pOut, char *pText, int pLen);

void table_init();
void table_exit();

#ifdef __cplusplus
}
#endif

#endif

