#ifndef _INTERFACE_H_
#define _INTERFACE_H_

extern int fui_image_width;
extern int fui_image_height;
#define LCD_WIDTH_E 	fui_image_width
#define LCD_HEIGHT_E 	fui_image_height
typedef struct _doc_info_
{
	int type;
	int width;
	int height;
	int page_num;
}DOC_INFO;

typedef struct _page_info_
{
	void *img;
	int x;
	int y;
	int width;
	int height;
	int page_num;
}PAGE_INFO;

#ifdef __cplusplus
extern "C" {
#endif
//GUIFUNC
int InitProgram(void* gui);

int CloseProgram();

int OpenDocument(char *fname, int w, int h, int flag);

int CloseDocument();

int GetDocInfo(DOC_INFO *pInfo);

int GetPageInfo(PAGE_INFO *pInfo);

int JumpPage(int pageNum);

int FindTextBegin(char* text, int iscase, int isforward);

int FindNext();

int ZoomIn();

int ZoomOut();

int MoveUp();

int MoveDown();

int MoveLeft();

int MoveRight();

int MoveOffset(int x,int y);
int SetTextProperty(int fontsize);

int ScalePage(int scale);

int GetCurrentPageNum();

char** GetOutlinesContent();		//get PDF Outlines

int	 GetOutlinesSize();				//get size of PDF Outlines

int	 OutlinesExist();				//PDF Outline exist or not

int	 GotoOutlinesPage(int index);	//jump to line in Outlines

//GUIFUNC
int (*EbInitProgram)(void* gui);
int (*EbCloseProgram)();
int (*EbOpenDocument)(char *fname, int w, int h, int flag);
int (*EbCloseDocument)();
int (*EbGetDocInfo)(DOC_INFO *pInfo);
int (*EbGetPageInfo)(PAGE_INFO *pInfo);
int (*EbJumpPage)(int pageNum);
int (*EbFindTextBegin)(char* text, int iscase, int isforward);
int (*EbFindNext)();
int (*EbZoomIn)();
int (*EbZoomOut)();
int (*EbMoveUp)();
int (*EbMoveDown)();
int (*EbMoveLeft)();
int (*EbMoveRight)();
int (*EbMoveOffset)(int x,int y);
int (*EbSetTextProperty)(int fontsize);
int (*EbScalePage)(int scale);
int (*EbGetSumPageNum)(void);
int (*EbGetCurrentPageNum)(void);
char** (*EbGetOutlinesContent)();
int	 (*EbGetOutlinesSize)();
int	 (*EbOutlinesExist)();
int	 (*EbGotoOutlinesPage)(int index);
#ifdef __cplusplus
}
#endif

#endif//_INTERFACE_H_
