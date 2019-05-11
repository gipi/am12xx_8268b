#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "picsel-debug.h"

#include "fileviewer.h"
#include "alien-timer.h"
#include "alien-screen.h"
#include "alien-error.h"
#include "alien-memory.h"
#include "alien-request.h"
#include "alien-event.h"
#include "alien-debug.h"
#include "alien-types.h"
#include "alien-context.h"
#include "picsel-focus.h"

#include "linux-alien.h"
#include "linux-alien-config-fv.h"


/* defined in fileviewer.c */
extern Alien_Context *globalAlienContext;
extern void *global_heap_buffer;
extern int load_percent;

/*****************************
 *	alien-timer
 ****************************/
static void Alien_idleEvent(void *arg, int timer_id)
{
    Alien_Context *ac = (Alien_Context*)arg;
    Picsel_Context *pc = ac->picselContext;

//  timer_id = TIMER_ID;

//  printf("call PicselApp_timerExpiry(), enter, timer_id: %d, tid: %x\n", timer_id, (unsigned int)(pthread_self()));
    PicselApp_timerExpiry(pc, timer_id);
//  printf("call PicselApp_timerExpiry(), leave, timer_id: %d, tid: %x\n", timer_id, (unsigned int)(pthread_self()));
}

void AlienTimer_request(Alien_Context *ac,
                        unsigned long *reference,
                        unsigned long ms)
{
    int timer_id;

//  printf("call %s(), ms:%d, tid: %x\n", __FUNCTION__, (unsigned int)ms, (unsigned int)(pthread_self()));

    timer_id = timer_add(ac->alienLinuxMain->ptimer, ms, Alien_idleEvent, (void*)ac);
//  timer_id = TIMER_ID;
    *reference = timer_id;
//  printf("\ttimer_id %d\n", timer_id);
}

int AlienTimer_cancel(Alien_Context *ac,
                      unsigned long *reference)
{
    int ret;
    int timer_id = (int)(*reference);
//  printf("call %s(), timer_id: %d, tid: %x\n", __FUNCTION__, timer_id, (unsigned int)(pthread_self()));

//  printf("\treturn 1\n");
//  return 1;

    ret = timer_remove(ac->alienLinuxMain->ptimer, timer_id);
    if(ret)
    {
//      printf("\treturn 0\n");
        return 0;
    }

//  printf("\treturn 1\n");
    return 1;
}


/******************************************
 *	alien-screen
 *****************************************/

void AlienScreen_pagesChanged(Alien_Context *ac,
                              int            currentPage,
                              int            numPages)
{
    ac->alienLinuxMain->page_num = currentPage;
    ac->alienLinuxMain->total_pages = numPages;
    PRINTF("call AlienScreen_pagesChanged(), %d / %d\n", currentPage, numPages);
}

void AlienScreen_getConfiguration(Alien_Context            *ac,
                                  AlienScreenConfiguration *config)
{
//  PRINTF("call %s()\n", __FUNCTION__);

    config->landscape = 0;

    config->xTopLeft = 0;
    config->yTopLeft = 0;

    config->picselScreenWidth    = ac->overrideScreenWidth;
    config->picselScreenHeight   = ac->overrideScreenHeight;

    config->physicalScreenWidth  = ac->overrideScreenWidth;
    config->physicalScreenHeight = ac->overrideScreenHeight;

    config->maxNonRotateWidth    = ac->overrideScreenWidth;
    config->maxNonRotateHeight   = ac->overrideScreenHeight;

    config->format    = PicselScreenFormat_b5g6r5;
    config->alignment = BufferAlignmentDefault;
}

void AlienScreen_update(Alien_Context      *ac,
                        void               *buffer,
            /*@unused@*/PicselScreenFormat  format,
                        unsigned int        width,
                        unsigned int        height,
                        unsigned int        widthBytes,
                        unsigned int        updateX,
                        unsigned int        updateY,
                        unsigned int        updateWidth,
                        unsigned int        updateHeight,
            /*@unused@*/unsigned int        xTopLeft,
            /*@unused@*/unsigned int        yTopLeft)
{
    PRINTF("call %s()\n", __FUNCTION__);
    PRINTF("\tbuffer: 0x%08x\n", (unsigned int)buffer);
    PRINTF("\tformat: ");
    switch(format)
    {
    case PicselScreenFormat_g8:
        PRINTF("g8\n");
        break;
    case PicselScreenFormat_r5g5b5x1:
        PRINTF("r5g5b5x1\n");
        break;
    case PicselScreenFormat_b5g6r5:
        PRINTF("b5g6r5\n");
        break;
    case PicselScreenFormat_r8g8b8x8:
        PRINTF("r8g8b8x8\n");
        break;
    case PicselScreenFormat_b4g4r4x4:
        PRINTF("b4g4r4x4\n");
        break;
    case PicselScreenFormat_ycbcr444:
        PRINTF("ycbcr444\n");
        break;
    case PicselScreenFormat_ycbcr420:
        PRINTF("ycbcr420\n");
        break;
    case PicselScreenFormat_ycbcr422:
        PRINTF("ycbcr422\n");
        break;
    case PicselScreenFormat_uyvy:
        PRINTF("uyvy\n");
        break;
    case PicselScreenFormat_ForceSize:
        PRINTF("ForceSize\n");
        break;
    default:
        PRINTF("%d\n", format);
    }
    PRINTF("\twidth: %u\n", width);
    PRINTF("\theight: %u\n", height);
    PRINTF("\twidthBytes: %u\n", widthBytes);
    PRINTF("\tupdateX: %u\n", updateX);
    PRINTF("\tupdateY: %u\n", updateY);
    PRINTF("\tupdateWidth: %u\n", updateWidth);
    PRINTF("\tupdateHeight: %u\n", updateHeight);
    PRINTF("\txTopLeft: %u\n", xTopLeft);
    PRINTF("\tyTopLeft: %u\n", yTopLeft);
//  PRINTF("\toverrideScreenWidth: %u\n", ac->overrideScreenWidth);
//  PRINTF("\toverrideScreenHeight: %u\n", ac->overrideScreenHeight);
//  PRINTF("call AlienScreen_update(), buf:0x%08x, w:%d, h:%d\n",
//          (unsigned int)buffer, ac->overrideScreenWidth, ac->overrideScreenHeight);

    if(ac->alienLinuxMain->fv_screen_update_rgb565)
    {
        ac->alienLinuxMain->fv_screen_update_rgb565(ac->alienLinuxMain->fv_screen_handle,
                                                    buffer,
                                                    width,
                                                    height,
                                                    widthBytes,
                                                    updateX,
                                                    updateY,
                                                    updateWidth,
                                                    updateHeight);
    }
}



/*************************************
 *	alien-error
 ************************************/

void AlienError_fatal(Alien_Context *ac)
{
    PRINTF("Fatal error occurred\n");
    abort();
}
/*
static void showError( Alien_Context *ac, char *mess)
{
    g_idle_add_full(G_PRIORITY_DEFAULT,
                    GtkAlien_showErrorMessagebox,
                    mess,
                    NULL);
}
*/
void AlienError_error(Alien_Context      *ac,
                      PicselError         error,
          /*@unused@*/PicselErrorData    *errorData)
{
    PRINTF("call %s(), tid: %d, ", __FUNCTION__, gettid());

    switch (error)
    {
    case PicselDocumentError_AgentMatchFailed:
        PRINTF("AgentMatchFailed\n");
        break;
    case PicselDocumentError_AgentMatchFailedNoData:
        PRINTF("AgentMatchFailedNoData\n");
        break;
    case PicselDocumentError_DocumentTranslationFailed:
        PRINTF("DocumentTranslationFailed\n");
        break;
    case PicselDocumentError_InternalError:
        PRINTF("InternalError\n");
        break;
    case PicselDocumentError_DocumentPasswordProtected:
        PRINTF("DocumentPasswordProtected\n");
#if FILE_ENCRYPTION_DETECT
	fv_set_error_code(error);
#endif
        break;
    case PicselDocumentError_UnsupportedCharset:
        PRINTF("UnsupportedCharset\n");
        break;
    case PicselError_OutOfMemory:
        PRINTF("OutOfMemory\n");
//	showError(ac, _("RES_ALERT_OUTOFMEMORY"));
        break;
    case PicselError_SettingsPathUndefined:
        PRINTF("SettingsPathUndefined\n");
        break;
    case PicselError_UnsupportedJSPopup:
        PRINTF("UnsupportedJSPopup\n");
        break;
    case PicselError_UploadFileTooLarge:
        PRINTF("UploadFileTooLarge\n");
        break;
    case PicselError_LastErrorUnused:
        PRINTF("LastErrorUnused\n");
        break;
    default:
        PRINTF("Unknown %d\n", error);
        break;
    }
}

/*****************************************
 *	alien-config
 ****************************************/
void AlienConfig_ready(Alien_Context *ac)
{
    /* default to using the fileviewer configuration */
    Fv_AlienConfig_ready(ac);


    PicselApp_setKeyBehaviour(globalAlienContext->picselContext,
                              1, /* we don't provide key repeat events */
                              0 /* we do deliver release events */
                              );

    /* set 'sleep' time to zero */
    Picsel_setTimeSlice(globalAlienContext->picselContext, 0, 200);

    /* read and update any preference values */
    Preferences_read(ac->picselContext,
                     ac->preferences);
}

/******************************************
 *	alien-memory
 ****************************************/

//#define MEMORY_USAGE_TRACE
#ifdef MEMORY_USAGE_TRACE
static pthread_mutex_t mut_mutex = PTHREAD_MUTEX_INITIALIZER;
#define MUT_BLK_MAX     1024
static unsigned int mut_mem_blks[MUT_BLK_MAX][2];
static unsigned int mut_init = 0;
static unsigned int mut_total_size = 0;
static unsigned int mut_total_blks = 0;
static unsigned int mut_max_size = 0;
static unsigned int mut_max_blks = 0;
static unsigned int mut_max_blk_size = 0;
#endif


void *AlienMemory_mallocStack(int size)
{
    void *p = malloc(size);
//  PRINTF("call %s(), addr:0x%08x, size:%d\n", __FUNCTION__, (unsigned int)p, size);
    return p;
}

void AlienMemory_freeStack(void *mem)
{
//  PRINTF("call %s(), addr:0x%08x\n", __FUNCTION__, (unsigned int)mem);
    free(mem);
}

void *AlienMemory_malloc(size_t *size)
{
    void *ptr;

    if(globalAlienContext->overrideHeapSize)
        *size = globalAlienContext->overrideHeapSize;

    if(global_heap_buffer && globalAlienContext->overrideHeapSize)
        ptr = global_heap_buffer;
    else
        ptr = malloc(*size);

    if (ptr == NULL) /* we're returning nothing */
        *size = 0;

#ifdef MEMORY_USAGE_TRACE
    pthread_mutex_lock(&mut_mutex);
    if(!mut_init)
    {
        memset(mut_mem_blks, 0, sizeof(mut_mem_blks));
        mut_init = 1;
    }
    if(mut_init)
    {
        int i;
        for(i=0;i<MUT_BLK_MAX;i++)
        {
            if(!mut_mem_blks[i][0])
            {
                unsigned int addr = (unsigned int)ptr;
                unsigned int len = (unsigned int)(*size);
                mut_mem_blks[i][0] = addr;
                mut_mem_blks[i][1] = len;
                mut_total_size += len;
                mut_total_blks++;
                if(len > mut_max_blk_size)
                    mut_max_blk_size = len;
                if(mut_total_blks > mut_max_blks)
                    mut_max_blks = mut_total_blks;
                if(mut_total_size > mut_max_size)
                    mut_max_size = mut_total_size;
                break;
            }
        }
    }
    pthread_mutex_unlock(&mut_mutex);

    printf("call AlienMemory_malloc(), addr:0x%08x, size:%d, %u, %u, %u, %u, %u\n",
            (unsigned int)ptr, *size,
            mut_total_size, mut_total_blks,
            mut_max_blk_size, mut_max_blks, mut_max_size);
#endif

    return ptr;
}


void AlienMemory_free(void *mem)
{
#ifdef MEMORY_USAGE_TRACE
    unsigned int addr = (unsigned int)mem;
    unsigned int len = 0;

    pthread_mutex_lock(&mut_mutex);
    if(!mut_init)
    {
        memset(mut_mem_blks, 0, sizeof(mut_mem_blks));
        mut_init = 1;
    }
    if(mut_init)
    {
        int i;
        for(i=0;i<MUT_BLK_MAX;i++)
        {
            if(mut_mem_blks[i][0] == addr)
            {
                len = mut_mem_blks[i][1];
                mut_total_size -= len;
                mut_total_blks--;
                mut_mem_blks[i][0] = 0;
                mut_mem_blks[i][1] = 0;
                break;
            }
        }
    }
    pthread_mutex_unlock(&mut_mutex);

    printf("call AlienMemory_free(), addr:0x%08x, size:%u, %u, %u\n",
            addr, len, mut_total_size, mut_total_blks);
#endif
    if(!global_heap_buffer || !globalAlienContext->overrideHeapSize)
        free(mem);
}


/*******************************************
 *	alien-request
 ******************************************/

void AlienUserRequest_request(Alien_Context *alienContext, PicselUserRequest_Request *request)
{
    AlienUserRequest_Request *alienRequest;
    int ret = 0; /* error by default */

    PRINTF("call %s()\n", __FUNCTION__);

    /* Create a temporary data structure to store the request */
    alienRequest = malloc(sizeof(*alienRequest));
    if (alienRequest == NULL)
        goto error_occurred;

    alienRequest->ac = alienContext;
    alienRequest->picselRequest = request;

    switch(request->type)
    {
    case PicselUserRequest_Type_Transition:
        if (request->requestData->transition.type != PicselUserRequest_Transition_Type_Popup)
        {
            /* Accept a normal transition straight away */
            request->requestData->transition.response = PicselUserRequest_Transition_Accept;
        //  g_idle_add_full(G_PRIORITY_DEFAULT, userRequestIdleCallback, alienRequest, NULL);
            timer_add(alienContext->alienLinuxMain->ptimer, 0, userRequestIdleCallback, alienRequest);
            ret = 1;
        }
        break;
    case PicselUserRequest_Type_Document_Password:
        {
        //  g_idle_add_full(G_PRIORITY_DEFAULT, requestPasswordCallback, alienRequest, NULL);
#if FILE_ENCRYPTION_DETECT
		fv_set_error_code(PicselDocumentError_DocumentPasswordProtected);
		ret = 0;
#else
		timer_add(alienContext->alienLinuxMain->ptimer, 0, requestPasswordCallback, alienRequest);
		ret = 1;
#endif
        }
        break;
    default:
        PRINTF(("AlienUserRequest_request type '%s' not handled\n", PicselDebug_getUserRequestType(request->type)));
	break;
    }

error_occurred:
    if (ret == 0)
    {
        if (alienRequest != NULL)
            free(alienRequest);
        request->result = PicselUserRequest_Result_Error;
    }
    else
    {
        request->result = PicselUserRequest_Result_Accepted;
    }
}

/***************************************
 *	alien-debug
 **************************************/

void AlienDebug_output(const char *string)
{
    char                  timebuf[15];
    static struct timeval startTime = {-1,-1};
    struct timeval        tv;
    int                   timeStamp;
    static int       nextLogNoTimestamp = 0;

    if (*string == 0)
        return;

    /* calculate time difference and print timestamp */

    gettimeofday(&tv, NULL);

    if (startTime.tv_sec == -1)
    {
        startTime.tv_sec = tv.tv_sec;
        startTime.tv_usec = tv.tv_usec;
    }

    timeStamp = difftimeval(tv, startTime);
    snprintf(timebuf, sizeof(timebuf), "%03d.%03d",
             timeStamp/1000,
             timeStamp%1000);

    if (0 == nextLogNoTimestamp)
        printf("%s:", timebuf);

    /* if the debug string is not terminated with a carriage return then
       set a flag so that the next call will not output a timestamp */
    if (string[strlen(string)-1] != '\n')
        nextLogNoTimestamp = 1;
    else
        nextLogNoTimestamp = 0;

    printf("%s", string);

    fflush(stdout);
}
             
/**********************************************
 *	alien-event
 *********************************************/

void AlienEvent_information(Alien_Context          *ac,
                            AlienInformation_Event  event,
                            void                   *eventData)
{
    const char *str;


    str = PicselDebug_getInformationEvent(event, eventData);

    PRINTF("call %s(), %s\n", __FUNCTION__, str);
    PRINTF("%s => %d ==> %s\n",__FILE__,__LINE__,str);
//  PRINTF("AlienInformation_Event '%s' received, %d\n", str, event);

    if(ac->alienLinuxMain->notify_information)
    {
        ac->alienLinuxMain->notify_information(str);
    }
    switch (event)
    {
    case AlienInformation_DocumentLoaded:
        {
            AlienInformation_DocumentLoadedInfo *info;
            info = (AlienInformation_DocumentLoadedInfo*)eventData;
            PRINTF("\tstatus:");
            if(info->status == PicselLoadedStatus_NotLoaded)
                PRINTF(" NotLoaded");
            else if(info->status == PicselLoadedStatus_PartiallyLoaded)
                PRINTF(" PartiallyLoaded");
            else if(info->status == PicselLoadedStatus_FullyLoaded)
                PRINTF(" FullyLoaded");
            else
                PRINTF(" %d", info->status);
            PRINTF("\n");
            PRINTF("\tcharSet:");
            switch(info->charSet)
            {
            case PicselConfig_CharSet_Iso88591:
                PRINTF("ISO-8859-1 Western\n");
                break;
            case PicselConfig_CharSet_Big5:
                PRINTF("Big5 Traditional Chinese\n");
                break;
            case PicselConfig_CharSet_Gbk:
                PRINTF("GBK Simplified Chinese\n");
                break;
            case PicselConfig_CharSet_ShiftJis:
                PRINTF("Shift_JIS Japanese\n");
                break;
            case PicselConfig_CharSet_Iso2022Jp:
                PRINTF("ISO-2022-JP Japanese\n");
                break;
            case PicselConfig_CharSet_EucJp:
                PRINTF("EUC-JP Japanese\n");
                break;
            case PicselConfig_CharSet_Windows949:
                PRINTF("WINDOWS-949 Korean\n");
                break;
            case PicselConfig_CharSet_Utf8:
                PRINTF("UTF-8 Universal alphabet\n");
                break;
            case PicselConfig_CharSet_Win1251:
                PRINTF("WINDOWS-1251 Cyrillic\n");
                break;
            case PicselConfig_CharSet_Koi8:
                PRINTF("KOI8 Cyrillic\n");
                break;
            case PicselConfig_CharSet_Iso88597:
                PRINTF("ISO-8859-7 Greek\n");
                break;
            case PicselConfig_CharSet_Unsupported:
                PRINTF("Unsupported\n");
                break;
            default:
                PRINTF("Unsupported, %d\n", info->charSet);
            }
            PRINTF("\tflags:");
            if(info->flags&PicselLoadedFlags_VirusUrl)
                PRINTF(" VirusUrl");
            if(info->flags&PicselLoadedFlags_SinglePageLoadingNotSupported)
                PRINTF(" SinglePageLoadingNotSupported");
            if(info->flags&PicselLoadedFlags_ForceSize)
                PRINTF("ForceSize");
            if(!info->flags)
                PRINTF("0");
            PRINTF("\n");

            ac->alienLinuxMain->fileLoaded = 1;

        //  AlienDebug_output("Document loaded.\n");
        //  PRINTF(("Timings for document load:\n"));
        //  LinuxAlien_displayResourceUsage();
        //  if ( info->status == PicselLoadedStatus_FullyLoaded )
        //  {
        //      g_idle_add_full(G_PRIORITY_DEFAULT,
        //                      GtkAlien_documentLoaded,
        //                      "", NULL);
        //  }
        }
        break;

    case AlienInformation_DocumentOnScreen:
        {
            AlienInformation_DocumentOnScreenInfo *info = (AlienInformation_DocumentOnScreenInfo*)eventData;
            PRINTF("\t%s\n", info->url);
        }
        break;

    case AlienInformation_DocumentClosed:
        ac->alienLinuxMain->documentClosed = 1;
        break;

    case AlienInformation_SecurePage:
        {
            AlienInformation_SecurePageInfo *info = (AlienInformation_SecurePageInfo*)eventData;
            PRINTF("\tsecure:");
            if(info->secure == PicselSecureStatus_Secure)
                PRINTF(" Secure");
            else if(info->secure == PicselSecureStatus_UserApproved)
                PRINTF(" UserApproved");
            else if(info->secure == PicselSecureStatus_NotSecure)
                PRINTF(" NotSecure");
            else
                PRINTF(" %d", info->secure);
            PRINTF("\n");
            PRINTF("\tinsecureFrames: %d\n", info->insecureFrames);
        }
        break;

    case AlienInformation_ImageSubSampled:
        {
        //  PRINTF(("Image subsampled\n"));
        //  showError(ac, _("RES_ALERT_IMAGESUBSAMPLED"));
        }
        break;

    case AlienInformation_FileInfoResult:
        {
            AlienInformation_FileInfo *fileinfo = (AlienInformation_FileInfo*)eventData;
            PRINTF("\ttitle: %s\n", fileinfo->title);
            PRINTF("\turl: %s\n", fileinfo->url);
            PRINTF("\tmime: %s\n", fileinfo->mime);
        //  if(fileinfo->mime == NULL)
        //  {
        //      ac->alienLinuxMain->fileLoaded = 1;
        //  }
//          if (NULL != fileinfo->mime)
//          {
//          }
//          else
//          {
//              /* We won't get a true docloaded(failed) so synthesize it here */
//              g_idle_add_full(G_PRIORITY_DEFAULT,
//                              GtkAlien_documentLoaded,
//                              NULL, NULL);
//          }
// 
//          PRINTF(("Got file info...\n"));
//          PRINTF(("Title : %s\n", fileinfo->title));
//          PRINTF(("URL : %s\n", fileinfo->url));
//          PRINTF(("MIME : %s\n", fileinfo->mime));
        }
        break;
    case AlienInformation_Zoom:
        {
            AlienInformation_ZoomInfo *ifo = (AlienInformation_ZoomInfo*)eventData;
            unsigned long zoom, zoom_in, zoom_out;
            int i, zoom_table_size;
            static unsigned long zoom_table[] =
            {
                (1UL<<16)/100,          // 1%
                (625UL<<16)/10000,      // 6.25%
                (125UL<<16)/1000,       // 12.5%
                (25UL<<16)/100,         // 25%
                (333UL<<16)/1000,       // 33.3%
                (50UL<<16)/100,         // 50%
                (666UL<<16)/1000,       // 66.6%
                (75UL<<16)/100,         // 75%
                (100UL<<16)/100,        // 100%
                (125UL<<16)/100,        // 125%
                (150UL<<16)/100,        // 150%
                (200UL<<16)/100,        // 200%,
                (300UL<<16)/100,        // 300%,
                (400UL<<16)/100,        // 400%,
                (500UL<<16)/100,        // 500%,
                (600UL<<16)/100,        // 600%,
                (700UL<<16)/100,        // 700%,
                (800UL<<16)/100         // 800%,
            };
            PRINTF("\tzoom: %d%%\n", (int)(ifo->zoom*100)/65536);
            PRINTF("\tminZoom: %d%%\n", (int)(ifo->minZoom*100)/65536);
            PRINTF("\tmaxZoom: %d%%\n", (int)(ifo->maxZoom*100)/65536);
            switch(ifo->state)
            {
            case PicselControl_Start:
                PRINTF("\tstate: Start\n");
                break;
            case PicselControl_Continue:
                PRINTF("\tstate: Continue\n");
                break;
            case PicselControl_Release:
                PRINTF("\tstate: Release\n");
                break;
            case PicselControl_End:
                PRINTF("\tstate: End\n");
                break;
            default:
                PRINTF("\tstate: %d\n", ifo->state);
                break;
            }

            ac->alienLinuxMain->zoom = ifo->zoom;
            ac->alienLinuxMain->zoom_min = ifo->minZoom;
            ac->alienLinuxMain->zoom_max = ifo->maxZoom;
            zoom = ifo->zoom;
            zoom_in = ifo->maxZoom;
            zoom_out = ifo->minZoom;
            zoom_table_size = sizeof(zoom_table)/sizeof(zoom_table[0]);
            for(i=0;i<zoom_table_size;i++)
            {
                if(zoom > zoom_table[i])
                {
                    if(zoom-zoom_table[i] <= 300)
                        zoom = zoom_table[i];
                }
                else if(zoom < zoom_table[i])
                {
                    if(zoom_table[i]-zoom <= 300)
                        zoom = zoom_table[i];
                }
                if(zoom <= zoom_table[i])
                {
                    if(i == 0)
                        zoom_out = ifo->minZoom;
                    else
                        zoom_out = zoom_table[i-1];
                    if(zoom < zoom_table[i])
                        zoom_in = zoom_table[i];
                    else if(i+1 < zoom_table_size)
                        zoom_in = zoom_table[i+1];
                    else
                        zoom_in = ifo->maxZoom;
                    break;
                }
            }
            if(zoom_out < ifo->minZoom)
                zoom_out = ifo->minZoom;
            else if(zoom_out > ifo->maxZoom)
                zoom_out = ifo->maxZoom;
            if(zoom_in < ifo->minZoom)
                zoom_in = ifo->minZoom;
            else if(zoom_in > ifo->maxZoom)
                zoom_in = ifo->maxZoom;
            if(zoom_out > zoom_in)
                zoom_out = zoom_in;
            ac->alienLinuxMain->zoom_in = zoom_in;
            ac->alienLinuxMain->zoom_out = zoom_out;
            PRINTF("\tzoom_in:%d%%\n", (int)(zoom_in*100)/65536);
            PRINTF("\tzoom_out:%d%%\n", (int)(zoom_out*100)/65536);

        //  AlienInformation_ZoomInfo *zoomData = (AlienInformation_ZoomInfo *)eventData;
        //  if (NULL != ac->tsHandler)
        //      Touchscreen_updateZoomValue(ac->tsHandler, zoomData->zoom);
        }
        break;

    case AlienInformation_ZoomLimitReached:
        {
            AlienInformation_ZoomLimitData *data = (AlienInformation_ZoomLimitData*)eventData;
            switch(data->zoomLimit)
            {
            case AlienZoomLimit_Minimum:
                PRINTF("\tzoomLimit: Minimum\n");
                break;
            case AlienZoomLimit_Maximum:
                PRINTF("\tzoomLimit: Maximum\n");
                break;
            case AlienZoomLimit_None:
                PRINTF("\tzoomLimit: None\n");
                break;
            default:
                PRINTF("\tzoomLimit: %d\n", data->zoomLimit);
                break;
            }
        //  AlienInformation_ZoomLimitData *data = eventData;
        //  g_idle_add_full(G_PRIORITY_DEFAULT,
        //                  GtkAlien_updateZoom,
        //                  GINT_TO_POINTER(data->zoomLimit),
        //                  NULL);
        }
        break;

    case AlienInformation_DocumentContentAvailable:
        {
            AlienInformation_ContentAvailableInfo *ifo = (AlienInformation_ContentAvailableInfo*)eventData;
            PRINTF("\tinternetImagesDisplayed: %d\n", ifo->internetImagesDisplayed);
        }
        break;

    case AlienInformation_ThumbnailDone:
        {
            AlienInformation_ThumbnailDoneInfo *ifo = (AlienInformation_ThumbnailDoneInfo*)eventData;
            PRINTF("\tbitmapData: 0x%08x\n", (unsigned int)ifo->bitmapData);
            PRINTF("\twidth: %d\n", ifo->width);
            PRINTF("\theight: %d\n", ifo->height);
            PRINTF("\tpage: %d\n", ifo->page);
            PRINTF("\tnumBytes: %d\n", ifo->numBytes);
            PRINTF("\twidthBytes: %d\n", ifo->widthBytes);
        }
        break;

    case AlienInformation_ScreenResized:
        {
            AlienInformation_ScreenResizedInfo *resizeData = (AlienInformation_ScreenResizedInfo *)eventData;
            PRINTF("\ttopLeftX: %d\n", resizeData->topLeftX);
            PRINTF("\ttopLeftY: %d\n", resizeData->topLeftY);
            PRINTF("\twidth: %d\n", resizeData->width);
            PRINTF("\theight: %d\n", resizeData->height);
            PRINTF("\trotation: ");
            switch(resizeData->rotation)
            {
            case PicselRotationNoChange:
                PRINTF("NoChange\n");
                break;
            case PicselRotation0:
                PRINTF("0\n");
                break;
            case PicselRotation90:
                PRINTF("90\n");
                break;
            case PicselRotation180:
                PRINTF("180\n");
                break;
            case PicselRotation270:
                PRINTF("270\n");
                break;
            case PicselRotation_ForceSize:
                PRINTF("ForceSize\n");
                break;
            default:
                PRINTF("%d\n", resizeData->rotation);
                break;
            }
            PRINTF("\tresult: %d\n", resizeData->result);

        //  AlienInformation_ScreenResizedInfo *resizeData = (AlienInformation_ScreenResizedInfo *)eventData;
        //  if (NULL != ac->tsHandler)
        //      Touchscreen_updateRotation( ac->tsHandler,
        //                                  resizeData->rotation);
        }
        break;

    case AlienInformation_Pan:
        {
            AlienInformation_PanInfo *ifo = (AlienInformation_PanInfo*)eventData;
            PRINTF("\tx: %d\n", ifo->x);
            PRINTF("\ty: %d\n", ifo->y);
            PRINTF("\twidth: %d\n", ifo->width);
            PRINTF("\theight: %d\n", ifo->height);
            PRINTF("\tscreenWidth: %d\n", ifo->screenWidth);
            PRINTF("\tscreenHeight: %d\n", ifo->screenHeight);
            switch(ifo->state)
            {
            case PicselControl_Start:
                PRINTF("\tstate: Start\n");
                break;
            case PicselControl_Continue:
                PRINTF("\tstate: Continue\n");
                break;
            case PicselControl_Release:
                PRINTF("\tstate: Release\n");
                break;
            case PicselControl_End:
                PRINTF("\tstate: End\n");
                break;
            default:
                PRINTF("\tstate: %d\n", ifo->state);
                break;
            }
        }
        break;
    case AlienInformation_InitComplete:
    //  AlienDebug_output("Init complete.\n");
    //  AlienDebug_output("Timings for initialisation:\n");
    //  LinuxAlien_displayResourceUsage();
    //  LinuxAlien_resetResourceUsage();
        ac->picselInitComplete = 1;
        break;

    case AlienInformation_LayoutComplete:
        ac->alienLinuxMain->layoutComplete = 1;
        break;

    case AlienInformation_splashScreenDone:
        /* Inform the user if the
         * preferences.cfg file cannot be found
         */
    //  if (ac->prefInitStatus == PrefError_ConfigFileOpenFailed)
    //  {
    //      g_idle_add_full(G_PRIORITY_DEFAULT,
    //              GtkAlien_prefFileNotFound,
    //              NULL,
    //              NULL);
    //  }
        /* Inform the user if a font enabled in
         * the config file cannot be found.
         */
    //  if (ac->prefFontInitStatus == PrefFontError_FontNotAvailable)
    //  {
    //      showError(ac, _("RES_PREF_OEM_PREF_FONT_COULD_NOT_BE_REGISTERED"));
    //  }
        break;

    case AlienInformation_FlowMode:
        {
            AlienInformation_FlowModeInfo *resultInfo;
            resultInfo = (AlienInformation_FlowModeInfo *)eventData;

            switch(resultInfo->flowMode)
            {
            case FlowMode_Normal:
                PRINTF("\tflowMode: Normal\n");
                break;
            case FlowMode_FitScreenWidth:
                PRINTF("\tflowMode: FitScreenWidth\n");
                break;
            case FlowMode_PowerZoom:
                PRINTF("\tflowMode: PowerZoom\n");
                break;
            default:
                PRINTF("\tflowMode: %d\n", resultInfo->flowMode);
                break;
            }
            switch(resultInfo->result)
            {
            case FlowResult_Failure:
                PRINTF("\tresult: Failure\n");
                break;
            case FlowResult_Success:
                PRINTF("\tresult: Success\n");
                break;
            case FlowResult_UnsupportedFileType:
                PRINTF("\tresult: UnsupportedFileType\n");
                break;
            default:
                PRINTF("\tresult: %d\n", resultInfo->result);
                break;
            }
        //  PRINTF(("Flowmode result: %d\n", resultInfo->flowMode));
            ac->flowMode = resultInfo->flowMode;
            ac->changingFlowMode = 0;
        }
        break;

    case AlienInformation_FocusInformation:
        {
            PicselFocus_Information *focusInfo = (PicselFocus_Information*)eventData;
            PRINTF("\tflags:");
            if(focusInfo->flags&PicselFocus_FocusedItem)
                PRINTF(" FocusedItem");
            if(focusInfo->flags&PicselFocus_StrongFocus)
                PRINTF(" StrongFocus");
            if(focusInfo->flags&PicselFocus_SelectWidget)
                PRINTF(" SelectWidget");
            if(focusInfo->flags&PicselFocus_MultiSelectWidget)
                PRINTF(" MultiSelectWidget");
            if(!(focusInfo->flags&(PicselFocus_FocusedItem|PicselFocus_StrongFocus|PicselFocus_SelectWidget|PicselFocus_MultiSelectWidget)))
                PRINTF(" %x", focusInfo->flags);
            PRINTF("\n");
            if(focusInfo->targetUrl)
                PRINTF("\ttargetUrl: %s\n", focusInfo->targetUrl);
            else
                PRINTF("\ttargetUrl: null\n");
            PRINTF("\titemType: ");
            if(focusInfo->itemType == PicselFocus_Item_None)
                PRINTF("None");
            else if(focusInfo->itemType == PicselFocus_Item_Link)
                PRINTF("Link");
            else if(focusInfo->itemType == PicselFocus_Item_Widget)
                PRINTF("Widget");
            else
                PRINTF("%d", focusInfo->itemType);
            PRINTF("\n");
        //  PRINTF(("Focus information: flags = %x\n",focusInfo->flags));
        //  if (focusInfo->targetUrl != NULL)
        //  {
        //      PRINTF(("targetUrl = %s\n", focusInfo->targetUrl));
        //  }
        }
        break;

    case AlienInformation_FocusResult:
        {
            PicselFocus_Result *focusResult = (PicselFocus_Result*)eventData;
            PRINTF("\tfailure: ");
            if(focusResult->failure == PicselFocus_ErrorNone)
                PRINTF("ErrorNone");
            else if(focusResult->failure == PicselFocus_ErrorUnknown)
                PRINTF("ErrorUnknown");
            else if(focusResult->failure == PicselFocus_OffDocumentEdge)
                PRINTF("OffDocumentEdge");
            else
                PRINTF("%d", focusResult->failure);
            PRINTF("\n");
        //  PRINTF(("Focus result: error = %d\n",focusResult->failure));
            if(focusResult->failure == PicselFocus_ErrorNone)
            {
                /* retrieve information about the currently focussed link */
                PicselFocus_getInformation(ac->picselContext);
            }
        }
        break;

    case AlienInformation_RequestShutdown:
    //  PRINTF(("Picsel requested shutdown\n"));
    //  GtkAlien_quit();
        break;

    case AlienInformation_SetPointerThreshold:
        {
            AlienInformation_SetPointerThresholdInfo *data = eventData;
            ac->pointerSizeThreshold = data->threshold;
        }
        break;

    case AlienInformation_FileProgressResult:
        {
            AlienInformation_FileProgress *ifo = (AlienInformation_FileProgress*)eventData;
            //nick add for loading
            load_percent = ifo->status;
            PRINTF("\tstatus: %d%%\n", ifo->status);
            PRINTF("\titemsLoaded: %d\n", ifo->itemsLoaded);
            PRINTF("\titemsTotal: %d\n", ifo->itemsTotal);
        //  AlienInformation_FileProgress *data = eventData;
        //  progressBarInfo.percentage = data->status;
        //  g_timeout_add(0,
        //                GtkAlien_updateProgressBar,
        //               &progressBarInfo);
        }
        break;

    default:
    //  PRINTF(("AlienInformation_Event '%s' received\n", str));
        break;
    }
}
 
