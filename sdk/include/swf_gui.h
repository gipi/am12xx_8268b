#ifndef _SWF_GUI_H
#define _SWF_GUI_H

/**
* @file swf_gui.h
* 
* @brief this header file describes the structs and APIs 
*    for gui drawing operations.
*
* @author Simon Lee.
*/

#include "swf_types.h"


/**
*@addtogroup swfdec_lib_s
*@{
*/

/**
* @brief the virtual device that drawing operations
*   take on.
*/
typedef struct _gui_virtual_device{
	unsigned int dev_vaddr;    ///< the device vitual address
	unsigned int dev_paddr;    ///< the device physical address
	int pixel_format;         ///< the pixel formate of the device
	int width,height;          ///< the width and height in pixel

	unsigned int text_color;    ///< text color,ARGB
	unsigned int text_bkcolor_en;   ///< text background color enable.
	unsigned int text_bkcolor;  ///< text background color.

	unsigned int font_size;
	
}VDC,*HVDC;


/**
* @brief draw line on the device.
*
* @param dev the painting device.
* @param line line descriptor.
* @return  0 for success and -1 for failed.
*/
extern int SWF_GUIDrawLine(HVDC dev,UIPATH* line);



/**
* @brief draw a polygon on the device.
*
* @param dev the painting device.
* @param Polygon polygon descriptor.
* @return  0 for success and -1 for failed.
*/
extern int SWF_GUIFillPolygon(HVDC dev,UIPATH* Polygon);



/**
* @brief draw a polygon on the device.
*
* @param dev the painting device.
* @param text text descriptor.
* @return  0 for success and -1 for failed.
*/
extern int SWF_GUIDrawText(HVDC dev,UITEXT *text);


/**
* @brief draw a polygon on the device.
*
* @param dev the painting device.
* @param text text descriptor.
* @return  0 for success and -1 for failed.
*/
extern int SWF_GUIGetText(HVDC dev,UITEXT *text);


/**
* @brief draw a picture on the device.
*
* @param dev the painting device.
* @param pic pic descriptor.
* @return  0 for success and -1 for failed.
*/
extern int SWF_GUILoadPic(HVDC dev,UIPIC* pic);



/**
 *@}
 */

#endif
