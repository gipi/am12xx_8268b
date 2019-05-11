#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

//void Settemppath(char *path);

void LoadDocument(void* p);

int OpenDocument(char* filename,int w,int h,int flag);

int LoadPrePage();

int LoadNextPage();

int GetCurrentPageNum();

int GetSumPageNum();

int JumpPage(int pageNum);

int FindTextBegin(char* text,int iscase,int isforward);

int MoveOffset(int xoffset,int yoffset);

int ScalePage(float scale);

int AutoPlay(int second);

int Rotation(int degree);

int CloseDocument();

//int InitProgram(char* filename);
int InitProgram(void* gui);

int CloseProgram();

//添加的功能
int SetTextProperty(int fontsize);

#ifdef __cplusplus
}
#endif

#endif
