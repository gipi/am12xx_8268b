#ifndef FPLAYER_H
#define FPLAYER_H


/**
*@addtogroup swfdec_lib_s
*@{
*/

#include "swf_types.h"
//#include "act_plugin.h"
#include "swf_2d.h"

enum{
	SWF_GETINFO_WIDTH=0,
	SWF_GETINFO_HEIGHT=1
};

#ifdef __cplusplus
extern "C" {
#endif


struct display_callback
{
	void (*put_display_task)(int);           /// tell display thread to do display
	int (*get_current_display_addr)(int);    /// get the address that currently show
};


/**
* @brief register the display thread's callback function to 2D thread.
*
* @param dcallback: a pointer to callback struct.
* @return always 0
* @see the definition of the "display_callback" structure
*/
extern int SWF_RegisterDisplayCallback(struct display_callback *dcallback);




/**
* @brief initialize the context of swf
*
* @param[in] param	: a pointer to the structure of SWF_CONTEXTINFO
* @return always 1
* @see the definition of SWF_CONTEXTINFO structure
*/
extern int SWF_Context_Init(void * param);


/**
* @brief release the context of swf
*
* @param[in] NULL
* @return always 1
*/
extern int SWF_Context_Release(void);


/**
* @brief add an instance into the playing context.
*
* @param[in] filename :  the swf file path to be added
* @param[in] window : the information of the window can be seen
* @param[in] flag : the flag which can determine the attributes the window or the action of the swf decoder
* @param[in] info :a pointer to the structure of SWF_INSTINFO, the function will fill the structure
* @return a pointer to the structure  of SWF_DEC
* @see the definition of SWF_INSTINFO , SWF_RECT and SWF_DEC structure
*/
extern SWF_DEC *SWF_AddInst(char * filename, SWF_RECT * window, int flag, SWF_INSTINFO * info);


/**
*@brief remove specified instance file from context
*
*@param[in] s : a pointer to the structure  of SWF_DEC which had been added into context
*@return NULL
*@see the definition of  SWF_DEC structure,
*/
extern void SWF_RemoveInst(SWF_DEC * s);



/**
*@brief get the active instance in context
*
*@param[in] NULL
*@return  a pointer to the structure  of SWF_DEC which is active 
*@see the definition of  SWF_DEC structure,
*/
extern SWF_DEC *SWF_GetActiveInst(void);


/**
*@brief get the playing context
*
*@param[in] NULL
*@return  a pointer to the structure  of SWF_CONTEXT
*@see the definition of  SWF_CONTEXT structure,
*/
extern SWF_CONTEXT *SWF_GetContext(void);


/**
*@brief send a message to swf player.
*
*@param[in] s which decoder the message to be sent.
*@param[in] msg message type.
*@param[in] param the parameter that the message takes.
*@return  0 for sucess , -1 for failed.
*/
extern int SWF_Message(SWF_DEC * s, int msg, void * param);

/**
*@brief dispath message in the swf player's message queue.
*
*@return  the message that have been processed.
*/
extern int SWF_Dispatch(void);


/**
*@brief prepare for the next frame.
*
*@param[in] s playing instance to be prepared.
*@return  0 for sucess , -1 for failed.
*/
extern int SWF_Prepare(SWF_DEC * s);


/**
*@brief update the current frame.
*
*@param[in] s playing instance to be update.
*@return  0 for sucess , -1 for failed.
*/
extern void SWF_Update(SWF_DEC * s);


/**
*@brief load the active playing instance.
*
*@param[in] s playing instance to be load.
*@return  0 for sucess , -1 for failed.
*/
extern int SWF_Load(SWF_DEC * s);


/**
*@brief set the state of the pointed instance.
*
*@param[in] s the pointed instance.
*@param[in] state the state to be set to.
*/
extern void SWF_SetState(SWF_DEC * s, int state);


/**
*@brief get the state of the pointed instance.
*
*@param[in] s the pointed instance.
*@return  state of the instance.
*/
extern int SWF_GetState(SWF_DEC * s);


/**
*@brief set the playing window of the instance.
*
*@param[in] s the pointed instance.
*@param[in] window window size.
*@param[in] flag additional flag.
*/
extern void SWF_SetWindow(SWF_DEC * s, SWF_RECT * window, int flag);



/**
*@brief malloc memory from the player's share heap.
*
*@param[in] size size to be malloc.
*@return  address to a physical continuous memory or NULL.
*/
extern void *SWF_Malloc(unsigned int size);

/**
*@brief 
*
*@param[in] p address of the memory.
*/
extern void *SWF_MemVir2Phy(void * p);

/**
*@brief malloc memory from the player's basic heap.
*
*@param[in] size size to be malloc.
*@return  address to a physical continuous memory or NULL.
*/
extern void *SWF_StaticMalloc(unsigned int size);


/**
*@brief free the memory that malloc by SWF_Malloc() or SWF_StaticMalloc().
*
*@param[in] p address of the memory.
*/
extern void SWF_Free(void * p);


/**
*@brief register an external function call.
*
*@param[in] name name of the function.
*@param[in] classNo the class that the function belongs to.
*@param[in] func pointer of the function.
*/
extern void SWF_RegisterExtCall(char * name, int classNo, int (*func)(void * h));


/**
*@brief get the parameter number that the ActionScript function.
*
*@param[in] h a transparent handle, user will ignore it.
*@return  the number of parameters.
*/
extern int SWF_GetParamNum(void * h);


/**
*@brief get the current parameter type in the parameter stack.
*
*@param[in] h a transparent handle, user will ignore it.
*@return  the type of current parameter.
*/
extern int SWF_GetParamType(void * h);


/**
*@brief if the current parameter in the stack is a number,
*    use this function to get its value.
*
*@param[in] h a transparent handle, user will ignore it.
*@return  the value of current parameter of type number.
*/
extern AS_NUMBER SWF_GetNumber(void * h);


/**
*@brief if the current parameter in the stack is string,
*    use this function to get its value.
*
*@param[in] h a transparent handle, user will ignore it.
*@return  the value of current parameter of type string.
*/
extern char *SWF_GetString(void * h);


/**
*@brief if the current parameter in the stack is object,
*    use this function to get its value.
*
*@param[in] h a transparent handle, user will ignore it.
*@return  a pointer to the object.
*/
extern void *SWF_GetObject(void * h);


/**
*@brief alloc an object,only for internal use.
*
*@param[in] size the size of the object struct.
*@return  a pointer to the object.
*/
extern void *SWF_AllocObject(unsigned int size);


/**
*@brief set an string variable to an object,only for internal use.
*
*@param[in] h a transparent handle, user will ignore it.
*@param[in] object a pointer to the object.
*@param[in] name name of the varialbe that the object will take.
*@param[in] str the string pointer.
*/
extern void SWF_AddStringToObject(void * h, void * object, char * name, char * str);



/**
*@brief set an number variable to an object,only for internal use.
*
*@param[in] h a transparent handle, user will ignore it.
*@param[in] object a pointer to the object.
*@param[in] name name of the varialbe that the object will take.
*@param[in] number the value of the variable.
*/
extern void SWF_AddNumberToObject(void * h, void * object, char * name, AS_NUMBER number);


/**
*@brief set a member to an object,only for internal use.
*
*@param[in] h a transparent handle, user will ignore it.
*
*@return  0 for success and -1 for failed.
*/
extern int SWF_SetMember(void * h);



/**
*@brief get a member from an object,only for internal use.
*
*@param[in] h a transparent handle, user will ignore it.
*
*@return  0 for success and -1 for failed.
*/
extern int SWF_GetMember(void * h);


/**
*@brief delete a member variable from an object,only for internal use.
*
*@param[in] h a transparent handle, user will ignore it.
*@param[in] object pointer to the object.
*@param[in] name member variable name.
*
*@return  0 for success and -1 for failed.
*/
extern int SWF_DeleteMember(void * h, void * object, char * name);


/**
*@brief give the return value of an external implemented function
*    if this function has an return value type of number.
*
*@param[in] h a transparent handle, user will ignore it.
*@param[in] n return value of this function.
*/
extern void SWF_PutNumber(void * h, AS_NUMBER n);



/**
*@brief give the return value of an external implemented function
*    if this function has an return value type of string.
*
*@param[in] h a transparent handle, user will ignore it.
*@param[in] str return value of this function with the string.
*/
extern void SWF_PutString(void * h, char * str);


/**
*@brief give the return value of an external implemented function
*    if this function has an return value type of object.
*
*@param[in] h a transparent handle, user will ignore it.
*@param[in] str return value of this function with the object.
*/
extern void SWF_PutObject(void * h, void * obj);


/**
*@brief give the return value of an external implemented function
*    if this function has an return value type of void.
*
*@param[in] h a transparent handle, user will ignore it.
*/
extern void SWF_PutNull(void * h);



/**
*@brief get the file path of the root swf file that currently
*    in playing.
*
*@param[in] h a transparent handle, user will ignore it.
*@return  pointer to the file path.
*/
extern char *SWF_GetFilePath(void * h);



/**
*@brief attach a bitmap that has decode into a memory to 
*    an swf object.
*
*@param[in] target pointer to the swf object.
*@param[in] pixel memory buffer of the bitmap.
*@param[in] width width of the object.
*@param[in] height height of the object.
*@param[in] dec_w width of the bitmap.
*@param[in] dec_h height of the bitmap.
*@param[in] stride stride of the buffer memory.
*@param[in] format format of decoded bitmap.
*@param[in] GetBitmapStatus a function of that detects the decoding status of the bitmap.
*
*/
extern void SWF_AttachBitmap(void * target, unsigned int* pixel, int width, int height, int dec_w, int dec_h, int stride, int format, int (*GetBitmapStatus)(int));



/**
*@brief dettach a bitmap from an swf object.
*
*@param[in] target pointer to the swf object.
*@param[in] clone a flag that indicates whether the bitmap buffer should 
*    should be cloned. if clone=1, the buffer will be cloned ; if clone=0,
*    the buffer data will not be cloned.
*@return  pointer to the bitmap buffer if clone is 0, 0 if clone is 1.
*/
extern int SWF_DetachBitmap(void * target, int clone);

/**
*@brief set the flag whether the buffer should be freed by swf engine when dettach a bitmap from an swf object.
*
*@param[in] target pointer to the swf object.
*@param[in] isfree a flag that indicates whether the bitmap buffer should 
*    should be freed by swf engine. if isfree=1, the buffer will be freed by swf engine when removing the swf object, a movieclip container of the bitmap
*    if isfree=0, the buffer data will not be freed by swf engine.
*/
extern void SWF_SetDetachBitmapFlag(void * target, int isfree);

/**
*@brief set some device dependent functions to swf decoder.
*
*@param[in] s instance of the that to be set.
*@param[in] dev_func device functions.

*@see please see the struct SWF_DEV_FUNC.
*
*/
extern void SWF_SetDeviceFunction(SWF_DEC * s, SWF_DEV_FUNC * dev_func);



/**
*@brief get the device dependent functions from swf decoder.
*
*@param[in] s instance of the swf file.
*@param[out] dev_func device functions.
*
*@see please see the struct SWF_DEV_FUNC.
*
*/
extern void SWF_GetDeviceFunction(SWF_DEC * s, SWF_DEV_FUNC * dev_func);


extern int SWF_CreateVividTemplate(char * swf_file, char * thumb, char * vt_file);
extern void SWF_LoadMovie(void * target, char * url, int width, int height, SWF_DEV_FUNC * func);
extern void SWF_RegisterIngoreWin(int x, int y, int w, int h);
extern void SWF_UnregisterIngoreWin(void);
extern void SWF_2DFill(FILLINFO * info);


/**
*@brief get the context information from swf decoder.
*
*@return a pointer to a context information.
*@see please see the struct SWF_CONTEXTINFO.
*/
extern SWF_CONTEXTINFO *SWF_GetContextInfo(void);


/**
*@brief get the current memory used of the swf share heap.
*
*@return the memory used of the share heap.
*/
extern int SWF_MemCurrent();


/**
*@brief get the total memory of the swf share heap.
*
*@return the total memory of the share heap.
*/
extern int SWF_MemTotal();


/**
*@brief get the current memory used of the swf basic heap.
*
*@return the memory used of the basic heap.
*/
extern int SWF_MemBasicCurrent();


/**
*@brief get the total memory of the swf basic heap.
*
*@return the total memory of the basic heap.
*/
extern int SWF_MemBasicTotal();


/**
*@brief dump the memory of the share heap.
*
*@param[in] dump if dump=1 the state information will be printed; if dump=0 the state 
*    information will not be printed.
*
*@return the memory that has been used.
*/
extern int SWF_MemCheck(int dump);



/**
*@brief get the swf decoder instance information.
*
*@param[in] inst instance of the swf file.
*@param[in] type type of the information SWF_GETINFO_WIDTH or SWF_GETINFO_HEIGHT.
*@param[out] data a pointer to the data that will be stored.
*
*@return  0 for success and -1 for failed.
*/
extern int SWF_GetInstInfo(void * inst, int type, void *data);


/**
*@brief get the frame rate of the current root swf file.
*
*@return  the frame rate.
*/
extern unsigned int SWF_GetCurrentFrameRate();


/**
*@brief get the last loaded swf instance's name .
*
*@param[out] name the name to be stored.
*
*@return  0 for success and -1 for failed.
*/
extern int	SWF_GetLatestInstName(char *name);


extern void keyboard_show(int flag);
extern void Enable_Chinese(int flag);
extern void SWF_SetTextColor(void* obj, unsigned int nIndexBegin, unsigned int nIndexEnd, unsigned int color );
extern void SWF_getTextCount( char* str, unsigned int strCount, unsigned int * engCount, unsigned int * chnCount );
extern int Gb2312ToUni(unsigned short * pOut, int OutLen, char *pIn, int InLen);
extern void Gb2312ToUtf8(char * pOut,char *pText, int pLen);
extern void Utf8ToGb2312(char *pOut, char *pText, int pLen);
extern void table_init();
extern void table_exit();
extern int Mouse_init(int x, int y);
extern void Mouse_Move(int init, int x, int y, int lcd_width, int lcd_height);
extern void Mouse_Position_Change(int init, int x, int y, int lcd_width, int lcd_height);
extern int IsMouseVisible();
extern int SetMouseShow();
extern int SetMouseHide();
extern int GetMouseX();
extern int GetMouseY();


/**
*@brief set the background color of the stage.
*
*@param[in] color the background color, format is ARGB with alpha omitted.
*
*/
extern void SWF_GUISetStageBkColor(unsigned int color);


/**
*@brief Wait the 2D thread to finish its work.
*/
extern int SWF_Sync2D();


/**
*@brief Get the current highest framerate of all the SWFs that in playing.
*/

extern int SWF_GetCurretHighestFramerate(void);



/**
*@brief get ime message.
*@return -1 for failed and others for success.
*/
extern int GetImeMessage();


/**
*@brief set filename of imeswf.
*@param[in] swfname filename of imeswf.
*@return  1 for success and 0 for failed.
*/
extern int SetImeSwfName(char *swfname);


/**
*@brief Set ime state.
*
*@param[in] state state of ime.
*@param[in] param parameters.
*
*@return  1 for success and 0 for failed.
*/
extern int SetImeState(int state, SWF_RECT *param);


/**
*@brief set range of key value for ime.
*
*@param[in] keymin minimal key value.
*@param[in] keymax maximal key value.
*
*@return  1 for success and 0 for failed.
*/
extern int SetImeKeyRange(int keymin, int keymax);

/**
*@brief get current render frame.
*@return 0 for failed and others for success.
*/
extern int SWF_GetCurrentFrame();

#ifdef __cplusplus
}
#endif


/**
 *@}
 */

#endif

