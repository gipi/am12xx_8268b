
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include "preferences.h"
#include "picsel-entrypoint.h"
#include "picsel-pixelblock.h"
#include "picsel-fileviewer.h"
#include "picsel-pointer.h"
#include "picsel-thumbnail.h"
#include "picsel-flowmode.h"
#include "picsel-version.h"
#include "picsel-config-fileviewer.h"

#include "preferences.h"
#include "alien-context.h"

#include "fileviewer.h"
#include "alien-debug.h"

static pthread_t global_thread_id = 0;
Alien_Context *globalAlienContext = NULL;
void *global_heap_buffer = NULL;
int load_percent = 0;

static int fv_error = 0;

#if 1 //-- Paul add for office reader set open mode.
int open_mode = 0;
#endif


static void alienPrefCallBack(PreferencesNode *prefNode)
{
    if (prefNode != NULL)
    {
        switch (prefNode->propertyType)
        {
            case PrefKeyType_StringProperty:
                /* */
                break;

            case PrefKeyType_IntProperty:
                if (strcmp("ALIEN_heap",prefNode->propertyKey)==0)
                {
                    /* The heap is specified in kilobytes */
                    globalAlienContext->overrideHeapSize =
                                prefNode->propertyInt * 1024;
                    break;
                }

                if (strstr(prefNode->propertyKey,"ALIEN_FONT_")!=NULL)
                {
                    Preferences_fontSet(&globalAlienContext->preferences,
                                         prefNode->propertyKey,
                                         prefNode->propertyInt,
                                        &globalAlienContext->prefFontInitStatus);
                    break;
                }

                if (strcmp("ALIEN_SCREEN_width",prefNode->propertyKey)==0)
                {
                    globalAlienContext->overrideScreenWidth =
                       prefNode->propertyInt;
                    break;
                }

                if (strcmp("ALIEN_SCREEN_height",prefNode->propertyKey)==0)
                {
                    globalAlienContext->overrideScreenHeight =
                       prefNode->propertyInt;
                    break;
                }
                break;

            case PrefKeyType_ColourProperty:
                /* */
                break;

            default:
                break;
        }
    }
}



static void *file_viewer_proc(void *arg)
{
	FV_INIT_PARAM *param = (FV_INIT_PARAM*)arg;
	AlienLinuxMain *mainContext = NULL;
	Alien_Context *alienContext = NULL;
	int cmd = 0;
	int cmd_int=0;
	char *cmd_str = NULL;
	/* Create Alien Context */
	alienContext = calloc(sizeof(Alien_Context), 1);
	if(!alienContext)
	{
		printf("Out of memory during initialisation.\n");
		goto end;
	}
	globalAlienContext = alienContext;

	mainContext = calloc(sizeof(AlienLinuxMain), 1);
	if(mainContext == NULL)
	{
		printf("Out of memory during initialisation.\n");
		goto end;
	}
	alienContext->alienLinuxMain = mainContext;

	if(pthread_mutex_init(&mainContext->cmd_mutex, NULL) != 0)
	{
		printf("pthread_mutex_init() failed\n");
		goto end;
	}

	mainContext->ptimer = timer_init();
	if(!mainContext->ptimer)
	{
		printf("timer_init error\n");
		goto end;
	}

	mainContext->fv_screen_handle = param->fv_screen_handle;
	mainContext->fv_screen_update_rgb565 = param->fv_screen_update_rgb565;

	mainContext->notify_information = param->notify_information;

	strcpy(mainContext->locale_set, param->locale_set);
	mainContext->group = param->group;

	mainContext->page_fit = param->page_fit;

	alienContext->rotation = PicselRotation0;

	alienContext->inputType = InputType_Command;

	alienContext->overrideHeapSize = param->heap_size;///*1024*24;

	alienContext->overrideScreenWidth = param->frame_buf_width;
	alienContext->overrideScreenHeight = param->frame_buf_height;

	/* Create thread structure for main thread */
	alienContext->mainThread = AlienThread_create(alienContext, NULL, NULL, 0);
	alienContext->currentThread = alienContext->mainThread;
	
	globalAlienContext->initFn = Picsel_EntryPoint_FileViewer();

    globalAlienContext->preferences = NULL;

    globalAlienContext->prefInitStatus =
		Preferences_init(&(globalAlienContext->preferences),
                          "/am7x/case/data/fileviewer.cfg",
                          "\n",
                         &alienPrefCallBack,
                         &(globalAlienContext->prefFontInitStatus),
                          globalAlienContext);
	globalAlienContext->thumbMode = Thumbnail_None;

	PicselApp_start(alienContext,
					NULL,
					globalAlienContext->initFn,//Picsel_EntryPoint_FileViewer(),//Picsel_EntryPoint_default(),//
					Picsel_ThreadModel_alienThreads(),//Picsel_ThreadModel_softThreads(),//
					(alienContext->overrideHeapSize == 0 ? PicselApp_ExpandingHeap : 0));

	for(;;)
	{
		TIMER_CONTEXT *ptc = (TIMER_CONTEXT*)mainContext->ptimer;
		TIMER_EVENT *pte = NULL;
		pthread_mutex_lock(&ptc->mutex);
		if(ptc->pevents)
		{
			pte = ptc->pevents;
			if(pte->ms > 0)
			{
				unsigned long time_s = 0;
				unsigned long time_us = 0;
				OSGetTime(&time_s, &time_us);
				time_us = ((time_s-pte->time_s)*1000*1000+time_us)-pte->time_us;
				if(time_us/1000 < pte->ms)
					pte = NULL;
			}
			if(pte)
				ptc->pevents = pte->next;
		}
		pthread_mutex_unlock(&ptc->mutex);
		if(pte)
		{
			if(pte->proc)
				pte->proc(pte->arg, pte->timer_id);
			free(pte);
		}

		cmd = CMD_NONE;
		pthread_mutex_lock(&mainContext->cmd_mutex);
		if(mainContext->cmd != CMD_NONE)
		{
			cmd = mainContext->cmd;
			cmd_int = mainContext->cmd_int;
			if(cmd_str)
			{
				free(cmd_str);
				cmd_str = NULL;
			}
			if(mainContext->cmd_str)
			{
				cmd_str = mainContext->cmd_str;
				mainContext->cmd_str = NULL;
			}
			mainContext->cmd = CMD_NONE;
		}
		pthread_mutex_unlock(&mainContext->cmd_mutex);

		if(cmd == CMD_QUIT)
		{
			PRINTF("CMD_QUIT\n");
			break;
		}

		if( alienContext->picselInitComplete &&
			alienContext->picselContext &&
			mainContext->layoutComplete )
		{
			PicselFileCommand fileCommand;
			Alien_Context *ac = alienContext;
			Picsel_Context *pc = ac->picselContext;
			int ret;
			char *ext;
			if(cmd != CMD_NONE)
			{
				PicselKey key_code = PicselCmdNone;
			//	ch = tolower(ch);
				switch(cmd)
				{
				case CMD_MOVE_UP:
				//	key_code = PicselCmdPanUp;
					{
						int y = ac->overrideScreenHeight/4;//ac->alienLinuxMain->pan_screen_height/4;
						PicselControl_pan(pc, 0, y, PicselControl_Start);
						PicselControl_pan(pc, 0, y, PicselControl_End);
					}
					break;
				case CMD_MOVE_DOWN:
				//	key_code = PicselCmdPanDown;
					{
						int y = -(ac->overrideScreenHeight/4);//-(ac->alienLinuxMain->pan_screen_height/4);
						PicselControl_pan(pc, 0, y, PicselControl_Start);
						PicselControl_pan(pc, 0, y, PicselControl_End);
					}
					break;
				case CMD_MOVE_LEFT:
				//	key_code = PicselCmdPanLeft;
					{
						int x = ac->overrideScreenHeight/4;//ac->alienLinuxMain->pan_screen_width/4;
						PicselControl_pan(pc, x, 0, PicselControl_Start);
						PicselControl_pan(pc, x, 0, PicselControl_End);
					}
					break;
				case CMD_MOVE_RIGHT:
				//	key_code = PicselCmdPanRight;
					{
						int x = -(ac->overrideScreenWidth/4);//-(ac->alienLinuxMain->pan_screen_width/4);
						PicselControl_pan(pc, x, 0, PicselControl_Start);
						PicselControl_pan(pc, x, 0, PicselControl_End);
					}
					break;
				case CMD_ROTATE:
					switch(ac->rotation)
					{
					case PicselRotation0:
						ac->rotation = PicselRotation90;
						break;
					case PicselRotation90:
						ac->rotation = PicselRotation180;
						break;
					case PicselRotation180:
						ac->rotation = PicselRotation270;
						break;
					case PicselRotation270:
						ac->rotation = PicselRotation0;
						break;
					default:
						ac->rotation = PicselRotation0;
						break;
					}
					PicselScreen_rotate(pc, ac->rotation);
				//	key_code = PicselCmdRotate;
					break;
				case CMD_PAGE_UP:
					key_code = PicselCmdPanUpFullScreen;
					break;
				case CMD_PAGE_DOWN:
					key_code = PicselCmdPanDownFullScreen;
					break;
				case CMD_PAGE_PREV:
				//	PicselFileviewer_turnPage(pc, PicselPageTurn_Previous);
					key_code = PicselCmdPreviousPage;
					break;
				case CMD_PAGE_NEXT:
				//	PicselFileviewer_turnPage(pc, PicselPageTurn_Next);
					key_code = PicselCmdNextPage;
					break;
				case CMD_FIRST_PAGE:
				//	PicselFileviewer_turnPage(pc, PicselPageTurn_First);
					key_code = PicselCmdFirstPage;
					break;
				case CMD_LAST_PAGE:
				//	PicselFileviewer_turnPage(pc, PicselPageTurn_Last);
					key_code = PicselCmdLastPage;
					break;
				case CMD_ZOOM_IN:
				//	key_code = PicselCmdZoomIn;
					if(ac->alienLinuxMain->zoom != ac->alienLinuxMain->zoom_in)
					{
						#if 1
						unsigned int zoomCentreX = ac->overrideScreenWidth/2;
						unsigned int zoomCentreY = ac->overrideScreenHeight/2;
						#else
						unsigned int zoomCentreX = 0;
						unsigned int zoomCentreY = 0;						
						#endif
						unsigned long magnification = ac->alienLinuxMain->zoom_in;
					//	PicselControl_setZoom(pc, magnification, zoomCentreX, zoomCentreY);
						PicselControl_zoom(pc, zoomCentreX, zoomCentreY, PicselControl_Start, magnification);
					//	PicselControl_zoom(pc, zoomCentreX, zoomCentreY, PicselControl_Continue, magnification);
						PicselControl_zoom(pc, zoomCentreX, zoomCentreY, PicselControl_End, magnification);
					}
					break;
				case CMD_ZOOM_OUT:
				//	key_code = PicselCmdZoomOut;
					if(ac->alienLinuxMain->zoom != ac->alienLinuxMain->zoom_out)
					{
						#if 1
						unsigned int zoomCentreX = ac->overrideScreenWidth/2;
						unsigned int zoomCentreY = ac->overrideScreenHeight/2;
						#else
						unsigned int zoomCentreX = 0;
						unsigned int zoomCentreY = 0;						
						#endif						
						unsigned long magnification = ac->alienLinuxMain->zoom_out;
						PicselControl_zoom(pc, zoomCentreX, zoomCentreY, PicselControl_Start, magnification);
					//	PicselControl_zoom(pc, zoomCentreX, zoomCentreY, PicselControl_Continue, magnification);
						PicselControl_zoom(pc, zoomCentreX, zoomCentreY, PicselControl_End, magnification);
					}
					break;
				case CMD_FIT_SCREEN:
					key_code = PicselCmdFitPage;
					break;
				case CMD_FIT_WIDTH:
					key_code = PicselCmdFitWidth;
					break;
				case CMD_FIT_HEIGHT:
					key_code = PicselCmdFitHeight;
					break;
				case CMD_OPEN:
				//	if(!ac->alienLinuxMain->fileLoaded)
				//	{
				//		PicselApp_stopDocument(pc, NULL);
				//		OSSleep(100);
				//	}
		//			PicselApp_closeDocument(pc, NULL);
					ac->alienLinuxMain->fileLoaded = 0;
					ac->alienLinuxMain->page_num = 0;
					ac->alienLinuxMain->total_pages = 0;
					fileCommand.fileContents = cmd_str;
					fileCommand.fileLength = 0;
					fileCommand.fileExtension[0] = '\0';
					ext = strrchr(cmd_str, '.');
					if(ext)
					{
						strcpy(fileCommand.fileExtension, ext+1);
						if( !strcasecmp(ext, ".pdf") ||
							!strcasecmp(ext, ".ppt") ||
							!strcasecmp(ext, ".pptx") ||
							!strcasecmp(ext, ".pps") ||
							!strcasecmp(ext, ".ppsx") )
						{
							PicselConfig_setInt(pc, PicselConfigFV_singlePageLoad, 1);
						}
						else
						{
							PicselConfig_setInt(pc, PicselConfigFV_singlePageLoad, 0);
						}
						if( !strcasecmp(ext, ".xls") ||
							!strcasecmp(ext, ".xlsx") )
						{
							PicselConfig_setInt(pc, PicselConfigFV_panToPageEnable, 0);
						}
						else
						{
							PicselConfig_setInt(pc, PicselConfigFV_panToPageEnable, 1);
						}
					}
					PRINTF("open file:%s, ext:%s\n", cmd_str, ext);
					#if 0 //-- Paul add for excel error
                    if((strncmp(ext,".pdf",4)==0)||(strncmp(ext,".ppt",4)==0)) {
                    	PicselConfig_setInt(pc, PicselConfigFV_singlePageLoad, 1);
                    } else {
                    	PicselConfig_setInt(pc, PicselConfigFV_singlePageLoad, 0);
                    }					
					#endif
					ret = PicselApp_loadDocument(pc, &fileCommand, NULL, 0);
					PRINTF("PicselApp_loadDocument ret:%d\n", ret);
					break;
				case CMD_CLOSE:
				//	if(!ac->alienLinuxMain->fileLoaded)
				//	{
				//		PicselApp_stopDocument(pc, NULL);
				//		OSSleep(100);
				//	}
					PicselApp_closeDocument(pc, NULL);
					ac->alienLinuxMain->fileLoaded = 0;
					ac->alienLinuxMain->page_num = 0;
					ac->alienLinuxMain->total_pages = 0;
					break;
				case CMD_GOTO_PAGE:
					PicselFileviewer_gotoPage(pc, cmd_int);
					break;
				}
				if(key_code != PicselCmdNone)
				{
				//	PRINTF("key_code:0x%08x\n", key_code);
				//	PRINTF("call PicselApp_keyPress(), enter\n");
					PicselApp_keyPress(pc, key_code, 0);
				//	PRINTF("call PicselApp_keyPress(), leave\n");

				//	OSSleep(20);

				//	PRINTF("call PicselApp_keyRelease(), enter\n");
					PicselApp_keyRelease(pc, key_code);
				//	PRINTF("call PicselApp_keyRelease(), leave\n");
				}
			}
		}

		if(!pte && cmd == CMD_NONE)
			OSSleep(5);
	}


//	PRINTF("call PicselApp_shutDown(), enter\n");
	if(PicselApp_shutDown(alienContext->picselContext) == 1)
	{
		alienContext->picselInitComplete = 0;
	}
//	PRINTF("call PicselApp_shutDown(), leave\n");


end:

	if(alienContext->mainThread)
	{
		AlienThread_destroy(alienContext, alienContext->mainThread);
	}

	while(alienContext->threadList)
	{
		AlienThread_destroy(alienContext, alienContext->threadList);
	}

	if(mainContext)
	{
		if(mainContext->ptimer)
			timer_release(mainContext->ptimer);
		if(mainContext->cmd_str)
			free(mainContext->cmd_str);
		free(mainContext);
	}

	if(alienContext)
	{
		free(alienContext);
		alienContext = NULL;
		globalAlienContext = NULL;
	}

	if(arg)
		free(arg);
	if(cmd_str)
		free(cmd_str);

	return NULL;
}

void fv_get_version_info(char *v,
						 int size_v,
						 char *i,
						 int size_i,
						 char *c,
						 int size_c)
{
	if(globalAlienContext && globalAlienContext->picselContext)
	{
		int len;

		len = size_v;
		v[0] = 0;
		PicselVersion_getVersion(globalAlienContext->picselContext, (unsigned char*)v, (unsigned int*)(&len));

		len = size_i;
		i[0] = 0;
		PicselVersion_getIssue(globalAlienContext->picselContext, (unsigned char*)i, (unsigned int*)(&len));

		len = size_c;
		c[0] = 0;
		PicselVersion_getCustomer(globalAlienContext->picselContext, (unsigned char*)c, (unsigned int*)(&len));
	}
}

static void print_ver_info()
{
	char v[256];
	char i[256];
	char c[256];

	fv_get_version_info(v, sizeof(v), i, sizeof(i), c, sizeof(c));

	printf("Version: %s\n", v);
	printf("Issue: %s\n", i);
	printf("Customer: %s\n", c);
}

int fv_release()
{
	if(global_thread_id)
	{
		if(globalAlienContext && globalAlienContext->alienLinuxMain)
		{
			AlienLinuxMain *mainContext = globalAlienContext->alienLinuxMain;
			pthread_mutex_lock(&mainContext->cmd_mutex);
			mainContext->cmd = CMD_QUIT;
			pthread_mutex_unlock(&mainContext->cmd_mutex);
		}
		pthread_join(global_thread_id, NULL);
		global_thread_id = 0;
	}
	global_heap_buffer = NULL;
	return 0;
}

int fv_init(FV_INIT_PARAM *param)
{
	int ret;
	int sleep_cnt = 0;
	FV_INIT_PARAM *fv_param = NULL;

	if(!param || globalAlienContext || global_thread_id){
		//printf("%s %d:param is %p\t globalAlienContext is %p\t global_thread_id is %d\n",__FUNCTION__,__LINE__,param,globalAlienContext,global_thread_id);
		return -1;
	}
	fv_param = (FV_INIT_PARAM*)malloc(sizeof(FV_INIT_PARAM));
	if(!fv_param)
	{
		printf("fv_init(), malloc error\n");
		return -1;
	}
	memcpy(fv_param, param, sizeof(FV_INIT_PARAM));

	global_heap_buffer = param->heap_buffer;
	ret = pthread_create(&global_thread_id, NULL, &file_viewer_proc, (void*)fv_param);
	if(ret)
	{
		printf("pthread_create: errno:%d, %s\n", ret, strerror(ret));
		free(fv_param);
		return -1;
	}
	while(!globalAlienContext ||
		  !globalAlienContext->picselInitComplete ||
		  !globalAlienContext->alienLinuxMain ||
		  !globalAlienContext->alienLinuxMain->layoutComplete)
	{
	//	printf("%s %d:globalAlienContext is %p\t globalAlienContext->picselInitComplete is %d\t globalAlienContext->alienLinuxMain is %p \t globalAlienContext->alienLinuxMain->layoutComplete is %d\n",__FUNCTION__,__LINE__,globalAlienContext,globalAlienContext->picselInitComplete,globalAlienContext->alienLinuxMain,globalAlienContext->alienLinuxMain->layoutComplete);
		OSSleep(100);
		sleep_cnt++;
		if(sleep_cnt > 120)
		{
	//		printf("%s %d:globalAlienContext is %p\t globalAlienContext->picselInitComplete is %d\t globalAlienContext->alienLinuxMain is %p \t globalAlienContext->alienLinuxMain->layoutComplete is %d\n",__FUNCTION__,__LINE__,globalAlienContext,globalAlienContext->picselInitComplete,globalAlienContext->alienLinuxMain,globalAlienContext->alienLinuxMain->layoutComplete);
			fv_release();
			return -1;
		}
	}
	print_ver_info();

	return 0;
}

static 
int fv_send_cmd(int cmd, int cmd_int, char *cmd_str)
{
	if( globalAlienContext &&
		globalAlienContext->picselInitComplete &&
		globalAlienContext->alienLinuxMain &&
		globalAlienContext->alienLinuxMain->layoutComplete )
	{
		AlienLinuxMain *mainContext = globalAlienContext->alienLinuxMain;
		pthread_mutex_lock(&mainContext->cmd_mutex);
		mainContext->cmd = cmd;
		mainContext->cmd_int = cmd_int;
		mainContext->cmd = cmd;
		if(mainContext->cmd_str)
		{
			free(mainContext->cmd_str);
			mainContext->cmd_str = NULL;
		}
		if(cmd_str)
			mainContext->cmd_str = strdup(cmd_str);
		pthread_mutex_unlock(&mainContext->cmd_mutex);
	}
	else
		return -1;
	return 0;
}

int fv_open_file(char *fname)
{
	printf("%s %d\n",__func__,__LINE__);
	return fv_send_cmd(CMD_OPEN, 0, fname);
}

int fv_close_file()
{
	if(globalAlienContext && globalAlienContext->alienLinuxMain)
	{
		int sleep_cnt = 0;
		globalAlienContext->alienLinuxMain->documentClosed = 0;
		if(fv_send_cmd(CMD_CLOSE, 0, NULL))
			return -1;
		while(!globalAlienContext->alienLinuxMain->documentClosed)
		{
			OSSleep(20);
			if(++sleep_cnt > 250)
				return -1;
		//	printf("sleep %d\n", sleep_cnt);
		}
	}
	else
	{
		return -1;
	}
	return 0;//fv_send_cmd(CMD_CLOSE, 0, NULL);
}

int fv_next_page()
{
	return fv_send_cmd(CMD_PAGE_NEXT, 0, NULL);
}

int fv_prev_page()
{
	return fv_send_cmd(CMD_PAGE_PREV, 0, NULL);
}

int fv_first_page()
{
	return fv_send_cmd(CMD_FIRST_PAGE, 0, NULL);
}

int fv_last_page()
{
	return fv_send_cmd(CMD_LAST_PAGE, 0, NULL);
}

int fv_goto_page(int page_num)
{
	return fv_send_cmd(CMD_GOTO_PAGE, page_num, NULL);
}

int fv_page_up()
{
	return fv_send_cmd(CMD_PAGE_UP, 0, NULL);
}

int fv_page_down()
{
	return fv_send_cmd(CMD_PAGE_DOWN, 0, NULL);
}

int fv_zoom_in()
{
	return fv_send_cmd(CMD_ZOOM_IN, 0, NULL);
}

int fv_zoom_out()
{
	return fv_send_cmd(CMD_ZOOM_OUT, 0, NULL);
}

int fv_fit_width()
{
	return fv_send_cmd(CMD_FIT_WIDTH, 0, NULL);
}

int fv_fit_height()
{
	return fv_send_cmd(CMD_FIT_HEIGHT, 0, NULL);
}

int fv_fit_screen()
{
	return fv_send_cmd(CMD_FIT_SCREEN, 0, NULL);
}

int fv_move_up()
{
	return fv_send_cmd(CMD_MOVE_UP, 0, NULL);
}

int fv_move_down()
{
	return fv_send_cmd(CMD_MOVE_DOWN, 0, NULL);
}

int fv_move_left()
{
	return fv_send_cmd(CMD_MOVE_LEFT, 0, NULL);
}

int fv_move_right()
{
	return fv_send_cmd(CMD_MOVE_RIGHT, 0, NULL);
}

int fv_rotate()
{
	return fv_send_cmd(CMD_ROTATE, 0, NULL);
}

#if 1 //-- Paul add for office reader open mode.
void fv_set_open_mode(int openmode)
{
	open_mode = openmode;
}
#endif

int fv_get_page_num(int *page_num, int *total_pages)
{
	if(globalAlienContext && globalAlienContext->alienLinuxMain)
	{
		if(page_num)
			*page_num = globalAlienContext->alienLinuxMain->page_num;
		if(total_pages)
			*total_pages = globalAlienContext->alienLinuxMain->total_pages;
	}
	else
	{
		return -1;
	}

	return 0;
}

int fv_get_zoom(int *zoom, int *zoom_min, int *zoom_max)
{
	if(globalAlienContext && globalAlienContext->alienLinuxMain)
	{
		if(zoom)
			*zoom = (globalAlienContext->alienLinuxMain->zoom*100)/65536;
		if(zoom_min)
			*zoom_min = (globalAlienContext->alienLinuxMain->zoom_min*100)/65536;
		if(zoom_max)
			*zoom_max = (globalAlienContext->alienLinuxMain->zoom_max*100)/65536;
	}
	else
	{
		return -1;
	}

	return 0;
}

// nick add for loading
int fv_get_load_percent()
{
	printf("return load_percent = %d\n",load_percent);
	return load_percent;
}

int fv_get_error_code(){
	return fv_error;
}

void fv_set_error_code(int error){
	fv_error = error;
}

void fv_reset_error_code(){
	fv_error = 0;
}

