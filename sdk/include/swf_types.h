#ifndef _SWF_TYPES_H_
#define _SWF_TYPES_H_

//#include "am_types.h"

typedef struct _SwfContext			SWF_CONTEXT;
typedef struct _SwfDec				SWF_DEC;
typedef struct _AsContext			ASCONTEXT;
typedef struct _AsObject			AS_OBJECT;
typedef struct _AsArray				AS_ARRAY;
typedef struct _AsMcLoader			AS_MCLOADER;
typedef struct _AsFunction			AS_FUNCTION;
typedef struct _AsColor				AS_COLOR;
typedef struct _AsDate				AS_DATE;
typedef struct _AsSound				AS_SOUND;
typedef struct _AsMatrix			AS_MATRIX;
typedef struct _AsBitmapData		AS_BITMAPDATA;
typedef struct _AsSharedObject		AS_SHAREDOBJECT;
typedef struct _AsTextFormat		AS_TEXTFORMAT;
typedef struct _AsTextSnapshot		AS_TEXTSNAPSHOT;
typedef struct _AsPoint				AS_POINT;
typedef struct _AsRectangle			AS_RECTANGLE;
typedef struct _AsLoadVars			AS_LOADVARS;
typedef struct _AsColorTransform	AS_COLORTRANSFORM;
typedef struct _AsXMLNode			AS_XMLNODE;
typedef struct _AsXML				AS_XML;
typedef struct _AsError				AS_ERROR;
typedef struct _AsTransform			AS_TRANSFORM;
typedef struct _AsStyleSheet			AS_STYLESHEET;

#define DOUBLE_POINT

/**
* how many frame buffers should be used.
*/
#define FRAME_MAX_NUMBER    16

//#define FUI_TIME_DEBUG

#define INVALIDATE_RECT_ENABLE 1

#if INVALIDATE_RECT_ENABLE
//#define FRAMECOPY_RECT_ENABLE
#endif

#define CONFIG_SWFDEC_DROPFRAME_EN 1

/**
* seperate display from swf main thread.
*/
#ifndef FRAMECOPY_RECT_ENABLE
#ifndef FUI_TIME_DEBUG
#define INDEPENDENT_DISPLAY_EN
#endif
#endif


#ifdef FIX_POINT
typedef int AS_NUMBER;
#define ITOF(_n) ((_n) << 16)
#define FTOI(_n) ((_n) >> 16)
#else
#ifdef DOUBLE_POINT
typedef double AS_NUMBER;
#define ITOF(_n) ((double)(_n))
#else
typedef float AS_NUMBER;
#define ITOF(_n) ((float)(_n))
#endif
#define FTOI(_n) ((int)(_n))
#endif

/************************************************************************/
/* Decode Bitmap Flags */

#define SWF_BMP_FMT_COMPACT		0x02
#define SWF_BMP_FMT_ARGB        0x00
#define SWF_BMP_FMT_XRGB        0x01
#define SWF_BMP_FMT_RGB565		0x02
#define SWF_BMP_FMT_YUV422      0x03
#define SWF_BMP_MODE_IGNORE     0x00
#define SWF_BMP_MODE_REPEAT     0x01
#define SWF_BMP_MODE_EXTEND     0x02
#define SWF_BMP_MODE_BGCOLOR    0x03

/** Decode Bitmap type */
#define SWF_BMP_TYPE_JPEG       0x1
#define SWF_BMP_TYPE_BMP        0x2
#define SWF_BMP_TYPE_PNG        0x3
#define SWF_BMP_TYPE_GIF        0x4
#define SWF_BMP_TYPE_TIF        0x5

/************************************************************************/
/* Window Flags */
#define SWF_WINDOW_MASK		0x00FF
#define SWF_SIZE_MASK		0x000F
#define SWF_SIZE_HEIGHT		0x0001
#define SWF_SIZE_WIDTH		0x0002
#define SWF_SIZE_AUTO		0x0004
#define SWF_SIZE_FILL		0x0008
#define SWF_ALIGN_H_MASK	0x0030
#define SWF_ALIGN_H_LEFT	0x0010
#define SWF_ALIGN_H_RIGHT	0x0020
#define SWF_ALIGN_H_CENTER	0x0030
#define SWF_ALIGN_V_MASK	0x00C0
#define SWF_ALIGN_V_UP		0x0040
#define SWF_ALIGN_V_DOWN	0x0080
#define SWF_ALIGN_V_CENTER	0x00C0

/* Root SWF instance Flags */
#define SWF_STANDALONE		0x0100
#define SWF_NON_RELOAD		0x0200

/* Debug Flags */
#define SWF_DEBUG_MASK		0x0c00
#define SWF_DEBUG			0x0400
#define SWF_ECHO			0x0800

/* Active Status Flags */
#define SWF_STATE_MASK		0x3000
#define SWF_STATE_SLEEP		0x0000
#define SWF_STATE_ACTIVE	0x1000
#define SWF_STATE_PAUSE		0x3000

/* Focus Navigation Flags */
#define SWF_FOCUS_AUTO		0x4000
#define SWF_KEY_NAVIGATE	0x8000

/* Format Flag */
#define SWF_FORMAT_MASK		0xF0000
#define SWF_FORMAT_YUV422	0x00000
#define SWF_FORMAT_RGB565	0x10000
#define SWF_FORMAT_RGB888	0x20000

/* Transparent Flag Mask */
#define SWF_TRANS_MASK		(SWF_DEBUG_MASK | SWF_FORMAT_MASK)

/* Default Flags */
#define SWF_ORIGINAL_FLAG    (SWF_ALIGN_H_CENTER | SWF_ALIGN_V_CENTER | SWF_STATE_ACTIVE | SWF_KEY_NAVIGATE)
#define SWF_LBOX_FLAG    (SWF_SIZE_AUTO | SWF_ALIGN_H_CENTER | SWF_ALIGN_V_CENTER | SWF_STATE_ACTIVE | SWF_KEY_NAVIGATE)
#define SWF_DEFAULT_FLAG    (SWF_SIZE_AUTO | SWF_ALIGN_H_CENTER | SWF_ALIGN_V_CENTER | SWF_SIZE_FILL | SWF_STATE_ACTIVE | SWF_KEY_NAVIGATE)
#define SWF_FULLSCREEN_FLAG (SWF_SIZE_HEIGHT | SWF_SIZE_WIDTH | SWF_SIZE_FILL | SWF_STATE_ACTIVE | SWF_KEY_NAVIGATE | SWF_FOCUS_AUTO)

/************************************************************************/
/* Messages */

#define SWF_MSG_NULL		0x0000
#define SWF_MSG_PLAY		0x0001
#define SWF_MSG_SLEEP		0x0002
#define SWF_MSG_ACTIVE		0x0003
#define SWF_MSG_PAUSE		0x0004
#define SWF_MSG_TIMER		0x0005
#define SWF_MSG_SOUND_END	0x0006
#define SWF_MSG_RELOAD		0x0007
#define SWF_MSG_EDITCHANGED		0x0008
#define SWF_MSG_LOAD_END		0x0009

/**
* Key messages.
*/

/// key mask
#define SWF_MSG_KEY			0x0100
#define SWF_MSG_CTRLKEY		0x1000

/// special key
#define SWF_MSG_EXIT			0x0105

/// general key
#define SWF_MSG_KEY_LBUTTON			(SWF_MSG_KEY | 0x1)
#define SWF_MSG_KEY_RBUTTON			(SWF_MSG_KEY | 0x2)
#define SWF_MSG_KEY_CANCEL			(SWF_MSG_KEY | 0x3)
#define SWF_MSG_KEY_MBUTTON			(SWF_MSG_KEY | 0x4)
                                                        
#define SWF_MSG_KEY_BACK				(SWF_MSG_KEY | 0x8)
#define SWF_MSG_KEY_TAB				(SWF_MSG_KEY | 0x9)
                                                        
#define SWF_MSG_KEY_CLEAR				(SWF_MSG_KEY | 0x0C)
#define SWF_MSG_KEY_RETURN			(SWF_MSG_KEY | 0x0D)
#define SWF_MSG_ENTER					SWF_MSG_KEY_RETURN                                                       
#define SWF_MSG_KEY_SHIFT				(SWF_MSG_KEY | 0x10)
#define SWF_MSG_KEY_CONTROL			(SWF_MSG_KEY | 0x11)
#define SWF_MSG_KEY_MENU				(SWF_MSG_KEY | 0x12)
#define SWF_MSG_KEY_PAUSE				(SWF_MSG_KEY | 0x13)
#define SWF_MSG_KEY_CAPITAL			(SWF_MSG_KEY | 0x14)
#define SWF_MSG_KEY_WIFI_CHANGE     (SWF_MSG_KEY | 0xCA)


#define SWF_MSG_KEY_RIGHT_PASSWORD_CONNECT  (SWF_MSG_KEY | 0xF8)

                                                        
#define SWF_MSG_KEY_JUNJA				(SWF_MSG_KEY | 0x17)
#define SWF_MSG_KEY_FINAL				(SWF_MSG_KEY | 0x18)
#define SWF_MSG_KEY_HANJA				(SWF_MSG_KEY | 0x19)
                                                        
#define SWF_MSG_KEY_ESCAPE			(SWF_MSG_KEY | 0x1B)
#define SWF_MSG_KEY_CONVERT			(SWF_MSG_KEY | 0x1C)
#define SWF_MSG_KEY_NONCONVERT		(SWF_MSG_KEY | 0x1D)
#define SWF_MSG_KEY_ACCEPT			(SWF_MSG_KEY | 0x1E)
#define SWF_MSG_KEY_MODECHANGE		(SWF_MSG_KEY | 0x1F)
                                                        
#define SWF_MSG_KEY_SPACE				(SWF_MSG_KEY | 0x20)
#define SWF_MSG_KEY_EXCLAMATION			(SWF_MSG_KEY | 0x21) //!
#define SWF_MSG_KEY_DOUBLE_QUOTATION	(SWF_MSG_KEY | 0x22) //"
#define SWF_MSG_KEY_POUND 				(SWF_MSG_KEY | 0x23) //#
#define SWF_MSG_KEY_DOLLAR				(SWF_MSG_KEY | 0x24) //$
#define SWF_MSG_KEY_PERCENT				(SWF_MSG_KEY | 0x25) //%
#define SWF_MSG_KEY_AND					(SWF_MSG_KEY | 0x26) //&
#define SWF_MSG_KEY_SINGLE_QUOTATION		(SWF_MSG_KEY | 0x27) //'
#define SWF_MSG_KEY_PARENTHESE_OPEN		(SWF_MSG_KEY | 0x28) //(
#define SWF_MSG_KEY_PARENTHESE_CLOSE	(SWF_MSG_KEY | 0x29) //)
#define SWF_MSG_KEY_MULTIPLICATION		(SWF_MSG_KEY | 0x2a) //*
#define SWF_MSG_KEY_PLUS					(SWF_MSG_KEY | 0x2b) //+
#define SWF_MSG_KEY_COMMA				(SWF_MSG_KEY | 0x2c) //,
#define SWF_MSG_KEY_HYPHEN				(SWF_MSG_KEY | 0x2d) //-
#define SWF_MSG_KEY_FULL_STOP				(SWF_MSG_KEY | 0x2e) //.
#define SWF_MSG_KEY_SLASH					(SWF_MSG_KEY | 0x2f) ///

#define SWF_MSG_KEY_0					(SWF_MSG_KEY | 0x30)
#define SWF_MSG_KEY_1					(SWF_MSG_KEY | 0x31)
#define SWF_MSG_KEY_2					(SWF_MSG_KEY | 0x32)
#define SWF_MSG_KEY_3					(SWF_MSG_KEY | 0x33)
#define SWF_MSG_KEY_4					(SWF_MSG_KEY | 0x34)
#define SWF_MSG_KEY_5					(SWF_MSG_KEY | 0x35)
#define SWF_MSG_KEY_6					(SWF_MSG_KEY | 0x36)
#define SWF_MSG_KEY_7					(SWF_MSG_KEY | 0x37)
#define SWF_MSG_KEY_8					(SWF_MSG_KEY | 0x38)
#define SWF_MSG_KEY_9					(SWF_MSG_KEY | 0x39)

#define SWF_MSG_KEY_COLON					(SWF_MSG_KEY | 0x3a) //:
#define SWF_MSG_KEY_SEMICOLON			(SWF_MSG_KEY | 0x3b) //;
#define SWF_MSG_KEY_ANGLE_BRACKET_OPEN	(SWF_MSG_KEY | 0x3c) //<
#define SWF_MSG_KEY_EQUAL					(SWF_MSG_KEY | 0x3d) //=
#define SWF_MSG_KEY_ANGLE_BRACKET_CLOSE	(SWF_MSG_KEY | 0x3e) //>
#define SWF_MSG_KEY_QUESTION				(SWF_MSG_KEY | 0x3f) //?
#define SWF_MSG_KEY_ALPHA					(SWF_MSG_KEY | 0x40) //@

#define SWF_MSG_KEY_A					(SWF_MSG_KEY | 0x41)
#define SWF_MSG_KEY_B					(SWF_MSG_KEY | 0x42)
#define SWF_MSG_KEY_C					(SWF_MSG_KEY | 0x43)
#define SWF_MSG_KEY_D					(SWF_MSG_KEY | 0x44)
#define SWF_MSG_KEY_E					(SWF_MSG_KEY | 0x45)
#define SWF_MSG_KEY_F					(SWF_MSG_KEY | 0x46)
#define SWF_MSG_KEY_G					(SWF_MSG_KEY | 0x47)
#define SWF_MSG_KEY_H					(SWF_MSG_KEY | 0x48)
#define SWF_MSG_KEY_I					(SWF_MSG_KEY | 0x49)
#define SWF_MSG_KEY_J					(SWF_MSG_KEY | 0x4A)
#define SWF_MSG_KEY_K					(SWF_MSG_KEY | 0x4B)
#define SWF_MSG_KEY_L					(SWF_MSG_KEY | 0x4C)
#define SWF_MSG_KEY_M					(SWF_MSG_KEY | 0x4D)
#define SWF_MSG_KEY_N					(SWF_MSG_KEY | 0x4E)
#define SWF_MSG_KEY_O					(SWF_MSG_KEY | 0x4F)
#define SWF_MSG_KEY_P					(SWF_MSG_KEY | 0x50)
#define SWF_MSG_KEY_Q					(SWF_MSG_KEY | 0x51)
#define SWF_MSG_KEY_R					(SWF_MSG_KEY | 0x52)
#define SWF_MSG_KEY_S					(SWF_MSG_KEY | 0x53)
#define SWF_MSG_KEY_T					(SWF_MSG_KEY | 0x54)
#define SWF_MSG_KEY_U					(SWF_MSG_KEY | 0x55)
#define SWF_MSG_KEY_V					(SWF_MSG_KEY | 0x56)
#define SWF_MSG_KEY_W					(SWF_MSG_KEY | 0x57)
#define SWF_MSG_KEY_X					(SWF_MSG_KEY | 0x58)
#define SWF_MSG_KEY_Y					(SWF_MSG_KEY | 0x59)
#define SWF_MSG_KEY_Z					(SWF_MSG_KEY | 0x5A)

#define SWF_MSG_KEY_BRACKET_OPEN			(SWF_MSG_KEY | 0x5b) //[
#define SWF_MSG_KEY_BACKSLASH			(SWF_MSG_KEY | 0x5c) //\\/
#define SWF_MSG_KEY_BRACKET_CLOSE		(SWF_MSG_KEY | 0x5d) //]
#define SWF_MSG_KEY_OR					(SWF_MSG_KEY | 0x5e) //^
#define SWF_MSG_KEY_DASH					(SWF_MSG_KEY | 0x5f) //_
#define SWF_MSG_KEY_DUNHAO 				(SWF_MSG_KEY | 0x60) //`

#define SWF_MSG_KEY_a					(SWF_MSG_KEY | 0x61)
// b c d ...
#define SWF_MSG_KEY_z					(SWF_MSG_KEY | 0x7A)

#define SWF_MSG_KEY_BRACE_OPEN			(SWF_MSG_KEY | 0x7b) //{
#define SWF_MSG_KEY_VERTICAL_BAR			(SWF_MSG_KEY | 0x7c) //|
#define SWF_MSG_KEY_BRACE_CLOSE			(SWF_MSG_KEY | 0x7d) //}
#define SWF_MSG_KEY_TILDE  				(SWF_MSG_KEY | 0x7e) //~
#define SWF_MSG_KEY_DEL  					(SWF_MSG_KEY | 0x7f) //delete

#define SWF_MSG_KEY_WIFIMODE_CHANGE		(SWF_MSG_KEY | 0xCA)
#define SWF_MSG_KEY_WIFI_FAILED		(SWF_MSG_KEY | 0xC2)

#define SWF_MSG_KEY_PAGE_UP			(SWF_MSG_CTRLKEY | 0x21)
#define SWF_MSG_KEY_PAGE_DOWN		(SWF_MSG_CTRLKEY | 0x22)
#define SWF_MSG_KEY_END				(SWF_MSG_CTRLKEY | 0x23)
#define SWF_MSG_KEY_HOME				(SWF_MSG_CTRLKEY | 0x24)
#define SWF_MSG_KEY_LEFT				(SWF_MSG_CTRLKEY | 0x25)
#define SWF_MSG_KEY_UP				(SWF_MSG_CTRLKEY | 0x26)
#define SWF_MSG_KEY_RIGHT				(SWF_MSG_CTRLKEY | 0x27)
#define SWF_MSG_KEY_DOWN				(SWF_MSG_CTRLKEY | 0x28)
#define SWF_MSG_KEY_SELECT			(SWF_MSG_CTRLKEY | 0x29)
#define SWF_MSG_KEY_PRINT				(SWF_MSG_CTRLKEY | 0x2A)
#define SWF_MSG_KEY_EXECUTE			(SWF_MSG_CTRLKEY | 0x2B)
#define SWF_MSG_KEY_SNAPSHOT			(SWF_MSG_KEY | 0x2C)
#define SWF_MSG_KEY_INSERT			(SWF_MSG_CTRLKEY | 0x2D)
#define SWF_MSG_KEY_DELETE			(SWF_MSG_CTRLKEY | 0x2E)
#define SWF_MSG_KEY_HELP				(SWF_MSG_CTRLKEY | 0x2F)

#define SWF_MSG_KEY_LWIN 		         (SWF_MSG_CTRLKEY | 0x5B)
#define SWF_MSG_KEY_RWIN 	           (SWF_MSG_CTRLKEY | 0x5C)
#define SWF_MSG_KEY_APPS 	           (SWF_MSG_CTRLKEY | 0x5D)
                                                        
#define SWF_MSG_KEY_NUMPAD0		       (SWF_MSG_CTRLKEY | 0x60)
#define SWF_MSG_KEY_NUMPAD1	         (SWF_MSG_CTRLKEY | 0x61)
#define SWF_MSG_KEY_NUMPAD2	         (SWF_MSG_CTRLKEY | 0x62)
#define SWF_MSG_KEY_NUMPAD3	         (SWF_MSG_CTRLKEY | 0x63)
#define SWF_MSG_KEY_NUMPAD4	         (SWF_MSG_CTRLKEY | 0x64)
#define SWF_MSG_KEY_NUMPAD5	         (SWF_MSG_CTRLKEY | 0x65)
#define SWF_MSG_KEY_NUMPAD6	         (SWF_MSG_CTRLKEY | 0x66)
#define SWF_MSG_KEY_NUMPAD7	         (SWF_MSG_CTRLKEY | 0x67)
#define SWF_MSG_KEY_NUMPAD8	         (SWF_MSG_CTRLKEY | 0x68)
#define SWF_MSG_KEY_NUMPAD9	         (SWF_MSG_CTRLKEY | 0x69)
#define SWF_MSG_KEY_MULTIPLY          (SWF_MSG_CTRLKEY | 0x6A)
#define SWF_MSG_KEY_ADD		           (SWF_MSG_CTRLKEY | 0x6B)
#define SWF_MSG_KEY_SEPARATOR         (SWF_MSG_CTRLKEY | 0x6C)
#define SWF_MSG_KEY_SUBTRACT          (SWF_MSG_CTRLKEY | 0x6D)
#define SWF_MSG_KEY_DECIMAL	         (SWF_MSG_CTRLKEY | 0x6E)
#define SWF_MSG_KEY_DIVIDE	           (SWF_MSG_CTRLKEY | 0x6F)
                                                        
#define SWF_MSG_KEY_F1		             (SWF_MSG_CTRLKEY | 0x70)
#define SWF_MSG_KEY_F2		             (SWF_MSG_CTRLKEY | 0x71)
#define SWF_MSG_KEY_F3		             (SWF_MSG_CTRLKEY | 0x72)
#define SWF_MSG_KEY_F4				(SWF_MSG_CTRLKEY | 0x73)
#define SWF_MSG_KEY_F5		             (SWF_MSG_CTRLKEY | 0x74)
#define SWF_MSG_KEY_F6		             (SWF_MSG_CTRLKEY | 0x75)
#define SWF_MSG_KEY_F7		             (SWF_MSG_CTRLKEY | 0x76)
#define SWF_MSG_KEY_F8		             (SWF_MSG_CTRLKEY | 0x77)
#define SWF_MSG_KEY_F9		             (SWF_MSG_CTRLKEY | 0x78)
#define SWF_MSG_KEY_F10		           (SWF_MSG_CTRLKEY | 0x79)
#define SWF_MSG_KEY_F11		           (SWF_MSG_CTRLKEY | 0x7A)
#define SWF_MSG_KEY_F12		           (SWF_MSG_CTRLKEY | 0x7B)
#define SWF_MSG_KEY_F13		           (SWF_MSG_CTRLKEY | 0x7C)
#define SWF_MSG_KEY_F14		           (SWF_MSG_CTRLKEY | 0x7D)
#define SWF_MSG_KEY_F15		           (SWF_MSG_CTRLKEY | 0x7E)
#define SWF_MSG_KEY_F16		           (SWF_MSG_CTRLKEY | 0x7F)
#define SWF_MSG_KEY_F17		           (SWF_MSG_CTRLKEY | 0x80)
#define SWF_MSG_KEY_F18		           (SWF_MSG_CTRLKEY | 0x81)
#define SWF_MSG_KEY_F19		           (SWF_MSG_CTRLKEY | 0x82)
#define SWF_MSG_KEY_F20		           (SWF_MSG_CTRLKEY | 0x83)
#define SWF_MSG_KEY_F21		           (SWF_MSG_CTRLKEY | 0x84)
#define SWF_MSG_KEY_F22		           (SWF_MSG_CTRLKEY | 0x85)
#define SWF_MSG_KEY_F23	             (SWF_MSG_CTRLKEY | 0x86)
#define SWF_MSG_KEY_F24		           (SWF_MSG_CTRLKEY | 0x87)
                                                        
#define SWF_MSG_KEY_NUMLOCK	         (SWF_MSG_CTRLKEY | 0x90)
#define SWF_MSG_KEY_SCROLL	           (SWF_MSG_CTRLKEY | 0x91)
                                                        
#define SWF_MSG_KEY_LSHIFT	           (SWF_MSG_CTRLKEY | 0xA0)
#define SWF_MSG_KEY_RSHIFT	           (SWF_MSG_CTRLKEY | 0xA1)
#define SWF_MSG_KEY_LCONTROL          (SWF_MSG_CTRLKEY | 0xA2)
#define SWF_MSG_KEY_RCONTROL          (SWF_MSG_CTRLKEY | 0xA3)
#define SWF_MSG_KEY_LMENU	           (SWF_MSG_CTRLKEY | 0xA4)
#define SWF_MSG_KEY_RMENU	           (SWF_MSG_CTRLKEY | 0xA5)


/// system key message
#define SWF_MSG_KEY_MEDIA_CHANGE              (SWF_MSG_KEY | 0xD0)       ///> e.g. card in/out
#define SWF_MSG_KEY_USB_IN                   (SWF_MSG_KEY | 0xD1)       ///> usb plug in
#define SWF_MSG_KEY_USB_OUT                  (SWF_MSG_KEY | 0xD2)       ///> usb plug out
#define SWF_MSG_KEY_RING                     (SWF_MSG_KEY | 0xD3)       ///> alarm ring
#define SWF_MSG_KEY_USB_HID_IN 		 (SWF_MSG_KEY | 0xD4)       ///> usb plug out
#define SWF_MSG_KEY_USB_HID_OUT		 (SWF_MSG_KEY | 0xD5)       ///> alarm ring

#define SWF_MSG_KEY_EZLAUNCHER_IN 		(SWF_MSG_KEY | 0xD8)      
#define SWF_MSG_KEY_EZLAUNCHER_OUT		(SWF_MSG_KEY | 0xD9)  

#define SWF_MSG_KEY_SWITCH_AOA 		 (SWF_MSG_KEY | 0xDA)       ///> ezwire aoa mode
#define SWF_MSG_KEY_SWITCH_ADB 		 (SWF_MSG_KEY | 0xDB)       ///>ezwire adb mode
#define SWF_MSG_KEY_ADB_STOP	 		(SWF_MSG_KEY | 0xDC)       ///> ezwire adb stop
                                                               
#define SWF_MSG_KEY_EXT_PHOTO                    (SWF_MSG_KEY | 0xE0)       ///> remote key: photo
#define SWF_MSG_KEY_EXT_MUSIC                    (SWF_MSG_KEY | 0xE1)       ///> remote key: music
#define SWF_MSG_KEY_EXT_VIDEO	                 (SWF_MSG_KEY | 0xE2)       ///> remote key: video
#define SWF_MSG_KEY_EXT_CALENDAR	             (SWF_MSG_KEY | 0xE3)       ///> remote key: calendar
#define SWF_MSG_KEY_EXT_SETUP	                 (SWF_MSG_KEY | 0xE4)       ///> remote key: setup
#define SWF_MSG_KEY_EXT_FILE	                 (SWF_MSG_KEY | 0xE5)       ///> remote key: file
#define SWF_MSG_KEY_EXT_TEXT	                 (SWF_MSG_KEY | 0xE6)       ///> remote key: text
#define SWF_MSG_KEY_EXT_VVD	                     (SWF_MSG_KEY | 0xE7)       ///> remote key: vvd
#define SWF_MSG_KEY_EXT_IRMENU	                 (SWF_MSG_KEY | 0xE8)       ///> remote key: menu
#define SWF_MSG_KEY_EXT_PLAYER	                 (SWF_MSG_KEY | 0xE9)       ///> remote key: player
#define SWF_MSG_KEY_EXT_VIEW	                 (SWF_MSG_KEY | 0xEA)       ///> remote key: view
#define SWF_MSG_KEY_EXT_PREV	                 (SWF_MSG_KEY | 0xEB)       ///> remote key: prev
#define SWF_MSG_KEY_EXT_NEXT	                 (SWF_MSG_KEY | 0xEC)       ///> remote key: next
#define SWF_MSG_KEY_EXT_COPY	                 (SWF_MSG_KEY | 0xED)       ///> remote key: copy
#define SWF_MSG_KEY_EXT_PLAY	                 (SWF_MSG_KEY | 0xEE)       ///> remote key: play
#define SWF_MSG_KEY_EXT_COMBO	                 (SWF_MSG_KEY | 0xEF)       ///> remote key: combo
#define SWF_MSG_KEY_EXT_ZOOM	                 (SWF_MSG_KEY | 0xF0)       ///> remote key: zoom
#define SWF_MSG_KEY_EXT_ROTATE	                 (SWF_MSG_KEY | 0xF1)       ///> remote key: rotate
#define SWF_MSG_KEY_EXT_MUTE	                 (SWF_MSG_KEY | 0xF2)       ///> remote key: mute
#define SWF_MSG_KEY_EXT_VOL_PLUS	             (SWF_MSG_KEY | 0xF3)       ///> remote key: vol+
#define SWF_MSG_KEY_EXT_VOL_MINUS	             (SWF_MSG_KEY | 0xF4)       ///> remote key: vol-
#define SWF_MSG_KEY_EXT_POWER                    (SWF_MSG_KEY | 0xF5)       ///> remote:power

#define SWF_MSG_KEY_CARDUPGRADE                    (SWF_MSG_KEY | 0xF6)       ///> remote:power
#define SWF_MSG_KEY_NOT_SUPPORT			(SWF_MSG_KEY | 0xF7)       ///> file no support msg
#define SWF_MSG_KEY_IME_STATE                    (SWF_MSG_KEY | 0xFF)       ///> ime state changed,only used by ime



/**
* mouse message.
*/
#define SWF_MSG_MOUSE			0x0200
#define SWF_MSG_LBUTTONDOWN		(SWF_MSG_MOUSE|0x1)
#define SWF_MSG_MOVE			(SWF_MSG_MOUSE|0x2)
#define SWF_MSG_LBUTTONUP		(SWF_MSG_MOUSE|0x3)
#define SWF_MSG_RBUTTONDOWN		(SWF_MSG_MOUSE|0x4)
#define SWF_MSG_RBUTTONUP		(SWF_MSG_MOUSE|0x5)
#define SWF_MSG_MBUTTONDOWN		(SWF_MSG_MOUSE|0x6)
#define SWF_MSG_MBUTTONUP		(SWF_MSG_MOUSE|0x7)
#define SWF_MSG_WHEEL		(SWF_MSG_MOUSE|0x8)
#define SWF_MSG_LDRAG		(SWF_MSG_MOUSE|0x9)
#define SWF_MSG_RDRAG		(SWF_MSG_MOUSE|0xA)
#define SWF_MSG_MDRAG		(SWF_MSG_MOUSE|0xB)

/**
* input method message
*/
#define SWF_MSG_IME			0x0400
#define SWF_MSG_KEYSPECIAL	0x0800


/************************************************************************/
/* Font Flags */

#define SWF_FONT_UTF8		0x0000
#define SWF_FONT_UTF16		0x0001
#define SWF_FONT_SINGLE		0x0002

/************************************************************************/
/* Basic types */

typedef struct _Rect
{
	int Xmin;
	int Ymin;
	int Xmax;
	int Ymax;
}SWF_RECT;

typedef struct _SwfMatrix
{
	int ScaleX;
	int ScaleY;	
	int RotateSkew0;
	int RotateSkew1;	
	int TranslateX;
	int TranslateY;
}SWF_MATRIX;

typedef struct _SwfCxForm
{
	int RedMultTerm,GreenMultTerm,BlueMultTerm,AlphaMultTerm;
	int RedAddTerm,GreenAddTerm,BlueAddTerm,AlphaAddTerm;
}SWF_CXFORM;

/************************************************************************/
/* Initializing structure */

typedef enum {
	SWFDEC_AS_TYPE_UNDEFINED = 0,
	SWFDEC_AS_TYPE_NULL = 0,
	SWFDEC_AS_TYPE_BOOLEAN,
	SWFDEC_AS_TYPE_INT,
	SWFDEC_AS_TYPE_NUMBER,
	SWFDEC_AS_TYPE_STRING,
	SWFDEC_AS_TYPE_OBJECT,
} SwfdecAsValueType;

typedef enum {
	AS_OBJECT_SPRITE = 0,
	AS_OBJECT_MATH,
	AS_OBJECT_STRING,
	AS_OBJECT_ARRAY,
	AS_OBJECT_KEY,
	AS_OBJECT_EXTERNAL,
	AS_OBJECT_OBJECT,
	AS_OBJECT_GLOBAL,
	AS_OBJECT_DEFAULT,
	AS_OBJECT_MCLOADER,
	AS_OBJECT_FUNCTION,
	AS_OBJECT_COLOR,
	AS_OBJECT_DATE,
	AS_OBJECT_SELECTION,
	AS_OBJECT_SOUND,
	AS_OBJECT_MATRIX,
	AS_OBJECT_BITMAPDATA,
	AS_OBJECT_STAGE,
	AS_OBJECT_MOUSE,
	AS_OBJECT_SHAREDOBJECT,
	AS_OBJECT_TEXTFORMAT,
	AS_OBJECT_TEXTSNAPSHOT,
	AS_OBJECT_POINT,
	AS_OBJECT_RECTANGLE,
	AS_OBJECT_ASBROADCASTER,
	AS_OBJECT_BOOLEAN,
	AS_OBJECT_NUMBER,
	AS_OBJECT_LOADVARS,
	AS_OBJECT_COLORTRANSFORM,
	AS_OBJECT_XMLNODE,
	AS_OBJECT_XML,
	AS_OBJECT_ERROR,
	AS_OBJECT_SYSTEM,
	AS_OBJECT_IME,
	AS_OBJECT_CAPABILITIES,
	AS_OBJECT_TRANSFORM,
	AS_OBJECT_TEXTRENDERER,
	AS_OBJECT_STYLESHEET,
	AS_OBJECT_TYPE_NUM,
	AS_CLASS_SYSTEM,
	AS_CLASS_USER
} AsObjectType;

#define HEAP_TYPE_BASIC		0
#define HEAP_TYPE_SHARE		1

typedef struct _SwfHeapInfo
{
	int heap_addr;
	int heap_size;
	int heap_type;
	/** the base virtual and physical address for this heap */
	int heap_base_virtual_addr;
	int heap_base_phy_addr;
} SWF_HEAPINFO;

typedef struct _SwfFontInfo
{
	char * font_path;
	int    font_size;
} SWF_FONTINFO;

typedef struct _SwfImageInfo
{
	void * swf_inst;
	int id;
	unsigned char * jpeg_data;
	void* handle;
	int  jpeg_size;
	unsigned char * pixel;
	int  width;
	int  height;
	int  stride;
	int  flag;
	int  (*GetStatus)(int id);
	int dataType;
} SWF_IMAGE_INFO;

#define SOUND_SYNC_STOP			2
#define SOUND_SYNC_NOMULTIPLE	1
#define SWF_GET_SOUND_SYNC(x)   (((x) >> 16) & 0x3)

#define SOUND_FMT_UNCOMP_BE		0
#define SOUND_FMT_ADPCM			1
#define SOUND_FMT_MP3			2
#define SOUND_FMT_UNCOMP_LE		3
#define SOUND_FMT_NELLYMOSER	6
#define SWF_GET_SOUND_FMT(x)	(((x) >> 4) & 0xF)

#define SOUND_RATE_5_5KHZ		0
#define SOUND_RATE_11KHZ		1
#define SOUND_RATE_22KHZ		2
#define SOUND_RATE_44KHZ		3
#define SWF_GET_SOUND_RATE(x)	(((x) >> 2) & 0x3)

#define SOUND_BPS_SND8BIT		0
#define SOUND_BPS_SND16BIT		1
#define SWF_GET_SOUND_BPS(x)	(((x) >> 1) & 0x1)

#define SOUND_TYPE_MONO			0
#define SOUND_TYPE_STEREO		1
#define SWF_GET_SOUND_TYPE(x)	((x) & 0x1)

typedef struct _SwfAudioInfo
{
	void * swf_inst;
	int id;
	int frame_count;
	unsigned char* audio_data;
	int audio_size;
	int loop;
	int flag;
	int DeviceId;
	int format;
	int sample_rate;
	int channels;
	int offset;
	int owner;
} SWF_AUDIO_INFO;

typedef struct _SwfDeviceFunction
{
	int (*decode_bitmap)(SWF_IMAGE_INFO * info);
	int (*play_audio)(SWF_AUDIO_INFO * info);
	int (*release)(SWF_DEC * s);
	int (*KeySound_Trigger)();
} SWF_DEV_FUNC;

typedef struct _SwfContextInfo
{
	SWF_HEAPINFO * heaps;
	SWF_DEV_FUNC defaultFunc;
	SWF_FONTINFO * fonts;
	
	int    frameBuffer[FRAME_MAX_NUMBER];
	int    frameWidth;
	int    frameHeight;
	int    frameNumber;

	char	reload[256];
} SWF_CONTEXTINFO;

typedef struct _SwfInstInfo
{
	char  signature[3];
	unsigned char  version;
	unsigned int file_length;
	unsigned short width;
	unsigned short height;
	unsigned short frame_rate;
	unsigned short frame_count;
} SWF_INSTINFO;

typedef enum {
	ELEMENT_NODE=1,
		ATTRIBUTE_NODE=2,
		TEXT_NODE=3,
		CDATA_SECTION_NODE=4,
		ENTITY_REFERENCE_NODE=5,
		ENTITY_NODE=6,
		PROCESSING_INSTRUCTION_NODE=7,
		COMMENT_NODE=8,
		DOCUMENT_NODE=9,
		DOCUMENT_TYPE_NODE=10,
		DOCUMENT_FRAGMENT_NODE=11,
		NOTATION_NODE=12,
} XMLNodeType;

typedef struct _SwfTimerEventSerialize
{
    SWF_DEC * s;
    int msg;
    void * param;
}SWF_TimerEventSerialize;

//////////////////////////////////////////////////////////////////////////
typedef struct
{
	int x;//1pixel/16
	int y;//x.4;
	int flag;
} UIPOINT;

typedef enum
{
	TYPE_MOVETO = 0,
	TYPE_LINETO  = 1,
	TYPE_CURVETO = 2,
} UILINEFLAG;

typedef struct
{
	UIPOINT *pointArray;
	int npoints;
	int width;//x.4;
	unsigned int color;//ARGB;
} UIPATH;

typedef struct
{
	int x; //TEXT START POINT
	int y;
	int font; //FONT INDEX
	int size; //FONT SIZE
	int w;
	int h;
	int direction;
	unsigned int color;
	char *string;
} UITEXT;

typedef struct
{
	char* filename;
	void *fp;
	int x;
	int y;
	int width;
	int height;
	UIPATH clippath;

} UIPIC;


typedef enum
{
	HIT_INPUTTEXT=0,
	HIT_OUTSIDE,
	ESC_BY_USER
}IME_MESSAGE;

//////////////////////////////////////////////////////////////////////////

#endif

