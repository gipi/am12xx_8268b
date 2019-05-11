/**
 * Information and event reporting from the Picsel library to the Alien
 * application.
 *
 * This file contains definitions and declarations needed for the calls
 * from the Picsel library to inform the Alien application of events inside the
 * Picsel library that may require a response or an action from the Alien application.
 *
 * The alien application is free to ignore these events.
 *
 * @file
 * $Id: alien-notify.h,v 1.162.10.1 2014/05/27 15:16:44 denis Exp $
 */
/* Copyright (C) Picsel, 2004-2008. All Rights Reserved. */
/**
 * @addtogroup TgvContentInformation
 *
 * @{
 */

#ifndef ALIEN_NOTIFY_H
#define ALIEN_NOTIFY_H

#include "alien-types.h"
#include "alien-information.h"
#include "alien-error.h"
#include "alien-legacy.h"
#include "picsel-control.h"
#include "picsel-screen.h"
#include "picsel-types.h"
#include "picsel-print.h"

/**
 * Asynchronous notifications from the Picsel library to the Alien
 * application. These are sent to AlienEvent_information() and usually
 * include a structure giving more details; the type of that structure
 * depends on the event notified.
 *
 * Picsel uses information events extensively to respond to requests
 * made by the Alien application earlier, but for which the status
 * could not be confirmed immediately. Some responses can be made
 * within a few milliseconds, but others make take many seconds.
 * Some information events are provided unsolicited, regarding the
 * state of another part of the system, such as the Internet connection
 * or behaviour within the content being viewed.
 *
 * There are additional values that may be notified, which are not listed
 * here. These are specific to optional features, and are defined in
 * API header files specific to those features.
 *
 * See @ref TgvAsync_Queue for further explanation.
 *
 * Note that this enum is the extension of enum AlienInformation_Event
 * defined in 'alien-information.h'.
 */
enum
{
    /** Image has been sub-sampled;
        @c eventData is @ref AlienInformation_ImageSubSampledInfo.*/
    AlienInformation_ImageSubSampled = AlienInformation_CoreLast,

    /** A request to load a document has completed; @c eventData is
        @ref AlienInformation_DocumentLoadedInfo. */
    AlienInformation_DocumentLoaded,

    /** A document for the Videoplayer is required; @c eventData is NULL. */
    AlienInformation_DocumentRequired,

    /** Screen has been resized or rotated;
        @c eventData is AlienInformation_ScreenResizedInfo. */
    AlienInformation_ScreenResized,

    /** The current zoom value and zoom limits; @c eventData is
        @ref AlienInformation_ZoomInfo.*/
    AlienInformation_Zoom,

    /** Thumbnail has been generated; @c eventData is
        @ref AlienInformation_ThumbnailDoneInfo.
        Thumbnail generation is controlled by the
        @ref PicselConfig_enableThumbnails property. */
    AlienInformation_ThumbnailDone,

    /** The splash screen has finished; @c eventData is NULL.*/
    AlienInformation_SplashScreenDone,

    /** The Picsel library would like to be shutdown, but it can be
        left running; @c eventData is AlienInformation_ShutdownInfo. */
    AlienInformation_RequestShutdown,

    /** Response from PicselApp_getFileInfo(); @c eventData is
        @ref AlienInformation_FileInfo. */
    AlienInformation_FileInfoResult,

    /** Title of document has changed; @c eventData is
        @ref AlienInformation_TitleInfo.*/
    AlienInformation_Title,

    /** The current document has changed; @c eventData is
        @ref AlienInformation_DocumentOnScreenInfo. */
    AlienInformation_DocumentOnScreen,

    /** Document scale is at the minimum or maximum limit; @c eventData is
        @ref AlienInformation_ZoomLimitData.*/
    AlienInformation_ZoomLimitReached,

    /** An animation has started; @c eventData is
        @ref AlienInformation_DynamicDocumentLoadedInfo.*/
    AlienInformation_DynamicDocumentLoaded,

    /** Progress has been made in loading a file; @c eventData is
        @ref AlienInformation_FileProgress. */
    AlienInformation_FileProgressResult,

    /** Some of the document is now on screen; @c eventData is
        @ref AlienInformation_ContentAvailableInfo.*/
    AlienInformation_DocumentContentAvailable,

    /** Some of the document is now on screen, however it's
        not complete but partial; @c eventData is NULL. */
    AlienInformation_DocumentContentAvailableIncomplete,

    /** Screen capture is complete; @c eventData is
        @ref AlienInformation_ScreenCaptureData. */
    AlienInformation_ScreenCaptureComplete,

    /** Request for file save extension done;
        @c eventData is @ref AlienInformation_FileSaveExtensionInfo. */
    AlienInformation_FileSaveExtensionDone,

    /** File saving operation done; @c eventData is
        @ref AlienInformation_FileSaveDoneInfo.*/
    AlienInformation_FileSaveDone,

   /**  The download limit has been reached for this document, downloading
        will be aborted; @c eventData is
        @ref AlienInformation_DownloadLimitInfo.*/
    AlienInformation_DownloadLimitReached,

    /** Delivers pan and zoom information; @c eventData is
        @ref AlienInformation_PanInfo. */
    AlienInformation_Pan,

    /** Pan limits have been reached; @c eventData is
        @ref AlienInformation_PanLimitsReachedInfo.*/
    AlienInformation_PanLimitsReached,

    /** Occurs when the JavaScript engine has encountered an error;
        @c eventData is @ref AlienInformation_JavaScriptErrorInfo. */
    AlienInformation_JavaScriptError,

    /** Notification of HTTP status (status 200, OK, will not be sent),
        @c eventData is @ref AlienInformation_HttpStatusInfo.*/
    AlienInformation_HttpStatus,

    /** The page is too complex to display fully; @c eventData is
        @ref AlienInformation_PageTooComplexInfo. */
    AlienInformation_PageTooComplex,

    /** Anchor link has been followed; @c eventData is
        @ref AlienInformation_AnchorTransitionInfo.*/
    AlienInformation_AnchorTransition,

    /** Error in embedded content on page; @c eventData is
        @ref AlienInformation_EmbeddedContentErrorInfo. */
    AlienInformation_EmbeddedContentError,

    /** Redirection is occurring; @c eventData is
        @ref AlienInformation_RedirectInfo. */
    AlienInformation_Redirect,

    /** Redirection error occurred; @c eventData is
        @ref AlienInformation_RedirectErrorInfo. */
    AlienInformation_RedirectError,

    /** Authentication error occurred; @c eventData is
        @ref AlienInformation_AuthenticationErrorInfo. */
    AlienInformation_AuthenticationError,

    /** AntiVirus-related errors; @c eventData is
         @ref AlienInformation_AntiVirusErrorInfo. */
    AlienInformation_AntiVirusError,

    /** Whether a page is loading over a secure connection;
        @c eventData is @ref AlienInformation_SecurePageInfo*/
    AlienInformation_SecurePage,

    /** A document scroll event occurred during link stepping;
        @c eventData is @ref AlienInformation_DocumentScrollInfo.*/
    AlienInformation_DocumentScrollEvent,

    /** General network error; @c eventData is
        @ref AlienInformation_NetworkErrorInfo. */
    AlienInformation_NetworkError,

    /** Status of HTML file upload; @c eventData is
        @ref AlienInformation_FileUploadInfo. */
    AlienInformation_FileUploadStatus,

    /** Contains the authorisation info for a URL, as requested by
        PicselBrowser_getAuthorisationForUrl(); @c eventData is
        @ref AlienInformation_UrlAuthenticationInfo.*/
    AlienInformation_UrlAuthentication,

    /** The document has started loading; @c eventData is
        @ref AlienInformation_DocumentLoadingInfo.*/
    AlienInformation_DocumentLoading,

    /** Cookie information in response to PicselBrowser_getCookies();
        @c eventData is @ref AlienInformation_CookiesInfo */
    AlienInformation_GetCookies,

    /** Result from a call to PicselBrowser_setCookie(); @c eventData
        is @ref AlienInformation_SetCookieInfo. */
    AlienInformation_SetCookieResult,

    /** Set the pointer movement threshold; @c eventData is
        @ref AlienInformation_SetPointerThresholdInfo. */
    AlienInformation_SetPointerThreshold,

    /** A single document layout operation has started; @c eventData is NULL */
    AlienInformation_LayoutStarted,

    /** A single document layout operation has ended; @c eventData is NULL */
    AlienInformation_LayoutEnded,

    /** A sequence of document layout operations has completed; @c eventData is NULL */
    AlienInformation_LayoutComplete,

    /** The font size has changed; @c eventData is
        @ref AlienInformation_FontSizeChangeData. */
    AlienInformation_FontSizeChange,

    /** A form has been reset, so a a file reference is no longer needed;
        @c eventData is @ref AlienInformation_FileSelectResetInfo. */
    AlienInformation_FileSelectReset,

    /** Information about current network activity; @c eventData is
        @ref AlienInformation_NetworkActivityInfo. */
    AlienInformation_NetworkActivity,

    /** A request to disable/enable the backlight; @c eventData is
        @ref AlienInformation_BacklightControlData. */
    AlienInformation_BacklightControl,

    /** Request that the target device's full screen mode (if supported) -
        which should hide any status bars and other screen furniture
        to enable the Picsel application to access the entire screen area -
        should be enabled or disabled.
        Note that if this request requires a screen resize, then the Picsel
        library should be informed with a call to PicselScreen_resize()
        when the screen is resized.
        @c eventData is @ref AlienInformation_SetFullScreenInfo. */
    AlienInformation_SetFullScreen,

    /** A request to set the desired orientation of the device's screen
        (if supported by the platform). @c eventData is
        @ref AlienInformation_SetRequestedOrientationInfo. */
    AlienInformation_SetRequestedOrientation,

    /** Request to disable power saving mode;
        @c eventData is @ref AlienInformation_PowerSavingData. */
    AlienInformation_PowerSavingMode,

    /** Request to change the orientation of the hardware's screen
       @c eventData is @ref AlienInformation_HwOrientation. */
    AlienInformation_ChangeHwOrientation,

    /** Request to rotate buffer supplied via AlienScreen_update.
       @c eventData is @ref AlienInformation_ChangeUpdateRotationData. */
    AlienInformation_ChangeUpdateRotation,

    /** The viewed position of the document has changed;
       @c eventData is NULL */
    AlienInformation_DocPositionChanged,

    /** The page is too tall to be displayed fully;
        @c eventData is @ref AlienInformation_PageTooTallInfo.*/
    AlienInformation_PageTooTall,

    /** The current document has closed and all non-shared resources freed;
        @c eventData is @ref AlienInformation_DocumentClosedInfo.*/
    AlienInformation_DocumentClosed,

    /** Status of thumbnailer; @c eventData is
        @ref AlienInformation_ThumbnailerStatusInfo. */
    AlienInformation_ThumbnailerStatus,

    /** Result from a call to PicselFileviewer_getPageSummaryText();
     *  @c eventData is @ref AlienInformation_PageTextInfo. */
    AlienInformation_PageSummaryText,

    /** Result from a call to PicselFileviewer_getPageText();
     *  @c eventData is @ref AlienInformation_PageTextInfo. */
    AlienInformation_PageText,

    /** Indicates whether or not Continuous Scrolling Mode was enabled or
     *  disabled successfully.  Sent in response to a call to
     *  PicselFileviewer_enableContinuousScrollingMode();
     *  @c eventData is @ref AlienInformation_ContinuousScrollingModeInfo. */
    AlienInformation_ContinuousScrollingMode,

    /** Indicates whether or not Inertial Scrolling Mode was enabled or
     *  disabled successfully.  Sent in response to a call to
     *  PicselFileviewer_enableInertialScrollingMode();
     *  @c eventData is @ref AlienInformation_InertialScrollingModeInfo. */
    AlienInformation_InertialScrollingMode,

    /** Indicates whether or not the application currently expects text entry
     *  events such as IME text events @ref PicselApp_composingText and
     *  @ref PicselApp_commitText, or calls to @ref PicselApp_keyPress and
     *  @ref PicselApp_keyRelease
     *
     *  @see PicselConfig_inlineTextEntry
     *
     *  @c eventData is @ref AlienInformation_InlineTextEntryInfo. */
    AlienInformation_InlineTextEntry,

    /** Request by the application to store the specified string into the
     *  platform's clipboard
     *
     *  @c eventdata is @ref AlienInformation_ClipboardData. */
    AlienInformation_Clipboard,

    /** Indicates that a menu item is selected in the application.
     *
     * @c eventdata is @ref AlienInformation_MenuItemSelectedData. */
    AlienInformation_MenuItemSelected,

    /** The current progress of a PicselPrint_printDocument() request
     *
     *  @c eventdata is @ref AlienInformation_PrintDocumentProgressData. */
    AlienInformation_PrintDocumentProgress,

    /** Result from a call to PicselPrint_printDocument().
     *
     *  @c eventdata is @ref AlienInformation_PrintDocumentResultData */
    AlienInformation_PrintDocumentResult,

    /** Result from a call to PicselPrint_searchForPrinters().
     *
     *  @c eventdata is @ref AlienInformation_SearchForPrintersData */
    AlienInformation_SearchForPrintersResult,

   /** Indicates that the user has cancelled a document load operation.
    *
    *  @c eventData is @ref AlienInformation_DocumentLoadCancelledInfo. */
    AlienInformation_DocumentLoadCancelled,

    /** Indicates how many documents are currently open in the application.
    *
    *  @c eventData is @ref AlienInformation_DocumentsOpenCountInfo. */
    AlienInformation_DocumentsOpenCount
};

enum
{
    /** Indicates that the back icon has been selected by the
     *  user.
     *
     * @c eventdata is @ref AlienInformation_MenuItemSelectedData */
    AlienInformation_MenuItemSelected_Back      = 0x10001,

    /** Indicates that the home icon has been selected by the
     *  user.
     *
     * @c eventdata is @ref AlienInformation_MenuItemSelectedData */
    AlienInformation_MenuItemSelected_Home      = 0x10002,

    /** Indicates that the add new tab icon has been selected by the
     *  user.
     *
     * @c eventdata is @ref AlienInformation_MenuItemSelectedData */
    AlienInformation_MenuItemSelected_AddNewTab = 0x10003
};

/**
 * Contains thumbnail bitmap data, sent with
 * @ref AlienInformation_ThumbnailDone and
 * @ref AlienInformation_ThumbnailGenerated events.
 *
 * Passed as @c eventData for @ref AlienInformation_ThumbnailDone and
 * @ref AlienInformation_ThumbnailGenerated events, which may be generated
 * while every page of a document is being loaded. Not all document
 * types may generate a thumbnail.
 *
 * The structure will be freed when AlienEvent_information() returns, so
 * before returning the Alien application must copy any data it will require
 * later.
 *
 *
 * @product File Viewer.
 *
 */
typedef struct AlienInformation_ThumbnailDoneInfo
{
    Picsel_View     *picselView;    /* View handle */
    void            *bitmapData;    /* Picsel format bitmap data */
    int              width;         /* In pixels */
    int              height;        /* In pixels */
    int              page;          /* Page number for this bitmap, numbered from 0 */
    int              numBytes;      /* Number of bytes in the data element */
    int              widthBytes;    /* Number of bytes between any pixel,
                                     * and the same pixel on the scan-line
                                     * below. */
}
AlienInformation_ThumbnailDoneInfo;

/**
 * Gives the new screen dimensions, sent with an
 * @ref AlienInformation_ScreenResized event.
 *
 * Passed as @c eventData for an @ref AlienInformation_ScreenResized event,
 * sent in response to PicselScreen_rotate(), PicselScreen_resize,
 * PicselScreen_physicalRotation() or PicselScreen_resizeRotate().
 */
typedef struct AlienInformation_ScreenResizedInfo
{
    int            topLeftX;           /**< X coord of top left of screen */
    int            topLeftY;           /**< Y coord of top left of screen */
    int            width;              /**< New width of screen           */
    int            height;             /**< New height of screen          */
    PicselRotation rotation;           /**< New rotation of screen        */
    int            result;             /**< True if resize/rotate         *
                                            succeeded                     */
}
AlienInformation_ScreenResizedInfo;

/**
 * Gives the current zoom status of the view, sent with an
 * @ref AlienInformation_Zoom event.
 *
 * Passed as @c eventData for an @ref AlienInformation_Zoom event,
 * sent in response to PicselControl_getZoom() and when the zoom
 * of a document changes.
 *
 * All zoom values are percentages where 100% is represented by (1<<16).
 * See @ref TgvZoom_Magnification.
 *
 */
typedef struct AlienInformation_ZoomInfo
{
    Picsel_View        *picselView; /**< View Handle */
    unsigned long       zoom;       /**< Magnification value for zoom event  */
    unsigned long       minZoom;    /**< The minimum zoom level allowed for
                                         this document */
    unsigned long       maxZoom;    /**< The maximum zoom level allowed for
                                         this document */
    PicselControl_State state;      /**< State of the zoom operation. Each
                                         zoom will end with a
                                         PicselControl_End and may be
                                         proceeded by a single
                                         PicselControl_Start and any number
                                         of PicselControl_Continues.       */
}
AlienInformation_ZoomInfo;

/**
 * Gives the title, URL and MIME type of a document, sent with
 * an @ref AlienInformation_FileInfoResult event.
 *
 * Passed as @c eventData for an @ref AlienInformation_FileInfoResult event,
 * sent in response to PicselApp_getFileInfo().
 *
 * @note The strings will be freed by the Picsel library when
 *        AlienEvent_information() returns.
 */
typedef struct AlienInformation_FileInfo
{
    Picsel_View   *picselView; /**< NULL, reserved for future use. */
    unsigned char *title;      /**< Title of the file/document, NULL if
                                    the title cannot be determined           */
    unsigned char *url;        /**< Url of the file/document, NULL if
                                    the url cannot be determined             */
    unsigned char *mime;       /**< Mime type of the file/document, NULL if
                                    the mime type cannot be determined       */
}
AlienInformation_FileInfo;

/**
 * Gives the title of a document, sent with an
 * @ref AlienInformation_Title event.
 *
 * Passed as @c eventData for an @ref AlienInformation_Title event, sent when the
 * title of the document has changed.
 *
 * @note The string will be freed by the Picsel library when
 *        AlienEvent_information() returns.
 */
typedef struct AlienInformation_TitleInfo
{
    Picsel_View         *picselView; /**< NULL, reserved for future use. */
    const unsigned char *title;      /**< Document title (UTF8) */
}
AlienInformation_TitleInfo;

/**
 * Gives the URL of the document, sent with an
 * @ref AlienInformation_DocumentOnScreen event.
 *
 * Passed as @c eventData for an @ref AlienInformation_DocumentOnScreen
 * event, sent when the URL of document has changed.

 *
 * @note The string will be freed by the Picsel library when
 *        AlienEvent_information() returns.
 */
typedef struct AlienInformation_DocumentOnScreenInfo
{
    Picsel_View         *picselView; /**< NULL, reserved for future use. */
    unsigned char       *url;        /**< URL */
}
AlienInformation_DocumentOnScreenInfo;

/**
 *  Errors related to file save request. These include errors in
 *  getting the target file save extension, as well as errors in
 *  the file saving process itself.
 */
typedef enum PicselFileSaveError
{
    PicselFileSaveError_NoError = 65539,    /**< No error                         */
    PicselFileSaveError_TypeNotSupported,   /**< File type not supported          */
    PicselFileSaveError_NonImageObject,     /**< Clicked object is not an image   */
    PicselFileSaveError_NoImageFound,       /**< No image found in the document   */
    PicselFileSaveError_FocusNotFound,      /**< No focus found in the document   */
    PicselFileSaveError_UserCancelled,      /**< User has cancelled the operation */
    PicselFileSaveError_ImageNotAvailable,  /**< The focused image is not available */
    PicselFileSaveError_FileSaveFailed,     /**< Either the file save process has
                                                 failed, or an error has occurred */
    PicselFileSaveError_NoBackgroundImage   /**< No background image exists for
                                                 this page */
}
PicselFileSaveError;

/**
 * Gives extended information about a completed save operation, sent with
 * an @ref AlienInformation_FileSaveExtensionDone event.
 *
 * Passed as @c eventData for an @ref AlienInformation_FileSaveExtensionDone,
 * sent in response to PicselBrowser_getFileSaveExtension().
 */
typedef struct AlienInformation_FileSaveExtensionInfo
{
    Picsel_View        *picselView; /**< NULL, reserved for future use. */
    const char         *ext;        /**< Correct file save extension or NULL
                                         in case of error                    */
    PicselSaveType      saveType;   /**< File save type i.e. doc or image    */
    PicselFileSaveError error;      /**< Error code, values allowed given by
                                         PicselFileSaveError */
}
AlienInformation_FileSaveExtensionInfo;

/**
 * Gives information about a completed save operation, sent with
 * an @ref AlienInformation_FileSaveDone event.
 *
 * Passed as @c eventData for an @ref AlienInformation_FileSaveDone event,
 * sent in response to PicselBrowser_saveDocument()
 *
 */
typedef struct AlienInformation_FileSaveDoneInfo
{
    Picsel_View        *picselView; /**< NULL, reserved for future use. */
    PicselFileSaveError error;      /**< Error code, values allowed given by
                                         PicselFileSaveError  */
}
AlienInformation_FileSaveDoneInfo;

/**
 *   Which zoom limit, if any, a document has reached.
 */
typedef enum AlienInformation_ZoomLimit
{
    AlienZoomLimit_Minimum = (1<<16), /**< Document is at minimum allowed zoom */
    AlienZoomLimit_Maximum,           /**< Document is at maximum allowed zoom */
    AlienZoomLimit_None               /**< Document is not at a zoom limit */
}
AlienInformation_ZoomLimit;

/**
 * Gives which zoom limit, if any, a document has reached, sent with
 * an @ref AlienInformation_ZoomLimitReached event.
 *
 * Passed as @c eventData for an @ref AlienInformation_ZoomLimitReached event,
 * sent when a call to PicselControl_zoom() cause the document to
 * reach a point where it could be zoomed no further.
 * The most recent zoom request may not have been actioned.
 */
typedef struct AlienInformation_ZoomLimitData
{
    Picsel_View               *picselView;/**< NULL, reserved for future use. */
    AlienInformation_ZoomLimit zoomLimit; /**< The zoom limit that has been
                                               reached in the document */
}
AlienInformation_ZoomLimitData;

/**
 * Contains the bitmap data of a screenshot, sent with
 * an @ref AlienInformation_ScreenCaptureComplete event.
 *
 * Passed as @c eventData for an @ref AlienInformation_ScreenCaptureComplete event,
 * in response to PicselScreen_capture().
 */
typedef struct AlienInformation_ScreenCaptureData
{
    int    bytesWritten; /**< Number of bytes needed to capture the image */
    int    width;        /**< The width (in pixels) of the image captured */
    int    height;      /**< The height (in pixels) of the image captured */
    void  *dest;        /**< The destination for the data.  Assumes that
                             this has been pre-allocated to the correct
                             size by the caller of PicselScreen_capture(). */
}
AlienInformation_ScreenCaptureData;

/**
 * Contains information on document loading progress, sent with
 * an @ref AlienInformation_FileProgressResult event.
 *
 * Passed as @c eventData for an @ref AlienInformation_FileProgressResult event,
 * sent while a document is loading.
 */
typedef struct AlienInformation_FileProgress
{
    Picsel_View *picselView;   /**< NULL, reserved for future use. */
    int          status;       /**< 0 to 100(%) toward overall readiness
                                    of a document or -1 for an unknown
                                    status.                               */
    int          itemsLoaded;  /**< Number of images loaded               */
    int          itemsTotal;   /**< Total number of images expected       */
}
AlienInformation_FileProgress;

/**
 * Gives an indication that a download limit has been reached, sent with
 * an @ref AlienInformation_DownloadLimitReached event.
 *
 * Passed as @c eventData for an @ref AlienInformation_DownloadLimitReached event,
 * sent when a download has failed because the download size limit set by
 * @c PicselConfigBr_downloadLimit has been reached.
 *
 * @product Browser products only.
 */
typedef struct AlienInformation_DownloadLimitInfo
{
    Picsel_View     *picselView; /**< NULL, reserved for future use. */
}
AlienInformation_DownloadLimitInfo;

/**
 * Gives details of a pan or zoom operation, sent with
 * an @ref AlienInformation_Pan event.
 *
 * Passed as @c eventData for an @ref AlienInformation_Pan event, in response
 * to PicselControl_getPan(), and after every pan or, when in
 * @ref FlowMode_PowerZoom mode, zoom.
 *
 * All int values are in Picsel internal units (except where stated otherwise).
 * See @ref TgvCoordinate_Space_Zoom_Pan.
 */
typedef struct AlienInformation_PanInfo
{
    Picsel_View        *picselView;      /**< NULL, reserved for future use. */
    int                 x;               /**< Current x position of panned
                                          *   document                       */
    int                 y;               /**< Current y position of panned
                                          *   document                       */
    int                 width;           /**< Width of zoomed document       */
    int                 height;          /**< Height of zoomed document      */
    int                 screenWidth;     /**< Screen width                   */
    int                 screenHeight;    /**< Screen height                  */
    int                 scrX;            /**< Current x position of panned
                                          *   document in screen pixels      */
    int                 scrY;            /**< Current y position of panned
                                          *   document in screen pixels      */
    int                 pixWidth;        /**< Current width of zoomed
                                          *   document in screen pixels      */
    int                 pixHeight;       /**< Current height of zoomed
                                          *   document in screen pixels      */
    PicselControl_State state;           /**< State of the pan operation.
                                              Each pan will end with a
                                              PicselControl_End and may be
                                              proceeded by a single
                                              PicselControl_Start and any
                                              number of
                                              PicselControl_Continues.       */
}
AlienInformation_PanInfo;

/**
 * Pan limits.  If a limit is set, then no further panning is possible in
 * that direction.
 *
 * For example: 0x10005 = PicselPanLimit_Top | PicselPanlimit_Left
 * This means panning is not possible up and left, but is
 * possible down and right.
 */
enum
{
    PicselPanLimit_Base   = 0x10000,
    PicselPanLimit_Top    = 0x10001,
    PicselPanLimit_Bottom = 0x10002,
    PicselPanLimit_Left   = 0x10004,
    PicselPanLimit_Right  = 0x10008
};

/**
 * A type representing a combination of the PicselPanLimit* flags,
 * e.g @ref PicselPanLimit_Top| @ref PicselPanLimit_Left.
 */
typedef unsigned int AlienInformation_PanLimits;

/**
 * Gives details of the pan limits that have been reached, sent with
 * an @ref AlienInformation_PanLimitsReached event.
 *
 * Passed as @c eventData for an @ref AlienInformation_PanLimitsReached event,
 * sent when panning has reached the edge of the document.
 */
typedef struct AlienInformation_PanLimitsReachedInfo
{
    Picsel_View               *picselView;/**< NULL, reserved for future use. */
    AlienInformation_PanLimits panLimits; /**< Pan limits reached.  A
                                               combination of the current
                                               limits. 0 if panning is
                                               possible in any direction. */
    AlienInformation_PanLimits focusLimits; /**< Focus limits reached.  A
                                               combination of the current
                                               limits. 0 if focus can move
                                               in any direction. */
}
AlienInformation_PanLimitsReachedInfo;

/**
 * Gives details of the edge the screen that a pan moved over, sent with
 * an @ref AlienInformation_DocumentScrollEvent event.
 *
 * Passed as @c eventData for an @ref AlienInformation_DocumentScrollEvent event,
 * when the document is panned.
 */
typedef struct AlienInformation_DocumentScrollInfo
{
    Picsel_View               *picselView; /**< NULL, reserved for future use. */
    AlienInformation_PanLimits edge;       /**< Screen edge latest pan event
                                                has sent view over*/
}
AlienInformation_DocumentScrollInfo;

/**
 * Possible JavaScript errors.
 */
typedef enum PicselJavaScriptError
{
    PicselJavaScriptError_HostStackDepth = (1<<16) /**< Host out of stack while
                                                        parsing JavaScript */
}
PicselJavaScriptError;

/**
 * Gives details of the Javascript error, sent with an
 * @ref AlienInformation_JavaScriptError event.
 *
 * Passed as @c eventData for an @ref AlienInformation_JavaScriptError event,
 * sent when there is an error in Javascript running in the document.
 */
typedef struct AlienInformation_JavaScriptErrorInfo
{
    Picsel_View                *picselView; /**< NULL, reserved for future use. */
    PicselJavaScriptError       error;      /**< Type of error. */
}
AlienInformation_JavaScriptErrorInfo;

/**
 * Values associated with a @ref AlienInformation_RequestShutdown event.
 */
typedef enum PicselShutdown_Type
{
    PicselShutdown_ResumeHostApplication  = (1<<16), /**< Inform alien application
                                                          to resume running. */
    PicselShutdown_ShutdownHostApplication           /**< Inform alien application
                                                          to shutdown.       */
}
PicselShutdown_Type;

/**
 * Gives the requested shutdown type, sent with
 * an @ref AlienInformation_RequestShutdown event.
 *
 * Passed as @c eventData for an @ref AlienInformation_RequestShutdown event,
 * sent when the Picsel library wants to be shutdown.
 */
typedef struct AlienInformation_ShutdownInfo
{
    PicselShutdown_Type type;      /**< Requested shutdown action. */
}
AlienInformation_ShutdownInfo;

/**
 * Gives HTTP status information, sent with an
 * @ref AlienInformation_HttpStatus event
 *
 * Passed as @c eventData for an @ref AlienInformation_HttpStatus event,
 * sent when there is an error in loading a remote document.
 */
typedef struct AlienInformation_HttpStatusInfo
{
    Picsel_View      *picselView; /**< View handle */
    int               httpStatus; /**< HTTP status. */
    int               bodyExists; /**< For HTTP 1.1 servers only: 1 if an
                                       entity-body was sent with
                                       the status code */
    const char       *url;        /**< URL associated with HTTP
                                       status (UTF8) */
    PicselContentType contentType;/**< The content type of the URL */
}
AlienInformation_HttpStatusInfo;

/**
 * Gives an indication that content is available, sent with
 * an @ref AlienInformation_DocumentContentAvailable event.
 *
 * Passed as @c eventData for an
 * @ref AlienInformation_DocumentContentAvailable event, sent when some of
 * the document has been loaded.
 */
typedef struct AlienInformation_ContentAvailableInfo
{
    Picsel_View *picselView;     /**< View handle */
    int internetImagesDisplayed; /**< True if webpage displays images */
}
AlienInformation_ContentAvailableInfo;

/**
 * Status value for @ref AlienInformation_DocumentLoaded. Web pages only.
 */
typedef enum PicselLoadedStatus
{
    PicselLoadedStatus_NotLoaded = (1<<16), /**< No content could be loaded */
    PicselLoadedStatus_PartiallyLoaded,     /**< Not all content loaded */
    PicselLoadedStatus_FullyLoaded          /**< All content successfully loaded */
}
PicselLoadedStatus;

typedef enum PicselLoadedFlags
{
    PicselLoadedFlags_VirusUrl                      = (1<<0),
                /**< Document loading was  aborted because
                     PicselUserRequest_Type_AntiVirus_URL
                     was rejected by the alien */
    PicselLoadedFlags_SinglePageLoadingNotSupported = (1<<1),
                /**< The document does not support
                     single page loading. */
    PicselLoadedFlags_ForceSize                     = (1<<16)
                /**< Force this enum to be contained in a 32 bit int. */
}
PicselLoadedFlags;

/**
 * Gives the status of a document loading operation, sent with
 * an @ref AlienInformation_DocumentLoaded event.
 *
 * Passed as @c eventData for an @ref AlienInformation_DocumentLoaded event,
 * in response to PicselApp_loadDocument() or related function.
 *
 * This does not always mean the document loaded successfully; check the
 * @ref AlienInformation_DocumentLoadedInfo.status and
 * @ref AlienInformation_DocumentLoadedInfo.flags to determine whether
 * the document was successfully loaded.
 * Errors will have been sent if problems were encountered while loading,
 * e.g. @ref AlienInformation_HttpStatus if there was an error loading a
 * remote document.
 */
typedef struct AlienInformation_DocumentLoadedInfo
{
    Picsel_View       *picselView; /**< View handle */
    unsigned char     *url;        /**< Url of the file/document, NULL if
                                        the url cannot be determined             */
    PicselLoadedStatus status;     /**< Web pages only: Document loaded status */
    int internetImagesDisplayed;   /**< Web pages only: Number of images
                                        displayed. Does not include a
                                        background image. */
    int backgroundImageDisplayed;  /**< Web pages only: 1 if a background
                                        image is displayed, 0 otherwise */
    int locationHeader;            /**< Web pages only: 1 if a location
                                        header was sent for this page, 0
                                        if not */
    int bodyExists;                /**< Web pages only: 1 if a body exists
                                        for this page, 0 if not */
    int httpStatus;                /**< Last HTTP status for the main
                                        document */
    PicselConfig_CharSet charSet;  /**< Character encoding for the main
                                        document (see @ref alien-types.h for a
                                        list of encodings) */
    PicselLoadedFlags flags;       /**< Further information about the
                                        success/failure of document loading */
}
AlienInformation_DocumentLoadedInfo;

/**
 * Errors for @ref AlienInformation_EmbeddedContentError.
 */
typedef enum PicselEmbeddedContentError
{
    /** Script could not be compiled */
    PicselEmbeddedContentError_ScriptNotCompiled = (1<<16),

    /** Script could not be run */
    PicselEmbeddedContentError_ScriptNotRun,

    /** Content not found */
    PicselEmbeddedContentError_NotFound,

    /** Unable to decode content */
    PicselEmbeddedContentError_CannotDecode
}
PicselEmbeddedContentError;

/**
 *  Gives error information, sent with an
 *  @ref AlienInformation_EmbeddedContentError event.
 *
 *  Passed as @c eventData for an @ref AlienInformation_EmbeddedContentError event,
 *  sent when one of the @ref PicselEmbeddedContentError errors occurs while
 *  downloading or running embedded content.
 *
 */
typedef struct AlienInformation_EmbeddedContentErrorInfo
{
    Picsel_View               *picselView; /**< View handle */
    PicselEmbeddedContentError error;      /**< Error description */
    const char                *url;        /**< URL of embedded content (UTF8),
                                                may be NULL */
    const char                *mainUrl;    /**< URL of main page (UTF8),
                                                may be NULL */
}
AlienInformation_EmbeddedContentErrorInfo;

/**
 * Contains the URL to which the document has been panned, sent with
 * an @ref AlienInformation_AnchorTransition event.
 *
 * Passed as @c eventData for an @ref AlienInformation_AnchorTransition event,
 * sent when an anchor link has been followed to another point
 * on the current document; e.g., <a href="#top">Top of Page</a>
 *
 * @note @c url will be freed by the Picsel library when
 * AlienEvent_information() returns.
 */
typedef struct AlienInformation_AnchorTransitionInfo
{
    unsigned char *url;      /**< Url of the current document - may be NULL in
                                  low memory conditions */
}
AlienInformation_AnchorTransitionInfo;

/**
 * Gives details of an HTTP redirection, sent with an
 * @ref AlienInformation_Redirect event.
 *
 * Passed as @c eventData for an @ref AlienInformation_Redirect event,
 * sent when a request for a remote document is redirected by the remote
 * server.  That is, the result is an HTTP status 301, 302, 303 or 307.
 *
 * URLs below may be NULL.
 *
 * @note All strings will be freed by the Picsel library when
 * AlienEvent_information() returns.
 */
typedef struct AlienInformation_RedirectInfo
{
    Picsel_View           *picselView;     /**< View handle */
    const char            *sourceUrl;      /**< URL of page that requested
                                                redirection (UTF8) */
    const char            *destinationUrl; /**< URL supplied by location
                                                header (UTF8) */
    PicselContentType      contentType;    /**< The content type of the source
                                                URL */
    int                    insecure;       /**< Non-zero if the redirect is
                                                insecure - redirecting from a
                                                HTTPS site to a HTTP site. */
    int                    httpStatus;     /**< HTTP status for redirection,
                                                or 0 for HTTP-EQUIV */
}
AlienInformation_RedirectInfo;

/**
 * Errors for @ref AlienInformation_RedirectError.
 */
typedef enum PicselRedirectError
{
    PicselRedirectError_MaxRequestsExceeded = (1<<16), /**< Exceeded maximum number
                                                            of redirections */
    PicselRedirectError_NoLocationHeader,              /**< Location header not
                                                            provided */
    PicselRedirectError_MissingHostOrScheme,           /**< Redirection to URL with
                                                            missing host or scheme */
    PicselRedirectError_UnsupportedScheme              /**< Redirection to an
                                                            unsupported scheme
                                                            (not HTTP or HTTPS) */
}
PicselRedirectError;

/**
 * Gives details an HTTP redirection error, sent with
 * an @ref AlienInformation_RedirectError event.
 *
 * Passed as @c eventData for an @ref AlienInformation_RedirectError event,
 * sent when an HTTP redirection fails.
 *
 * @note All strings will be freed by the Picsel library when
 * AlienEvent_information() returns.
 */
typedef struct AlienInformation_RedirectErrorInfo
{
    Picsel_View        *picselView;  /**< View handle */
    PicselRedirectError error;       /**< Error description */
    const char         *sourceUrl;   /**< URL of page that requested
                                          redirection (UTF8).
                                          May be NULL */
    PicselContentType   contentType; /**< The content type of the source
                                          URL */
    int                 httpStatus;  /**< HTTP status for redirection */
}
AlienInformation_RedirectErrorInfo;

/**
 * Errors for AlienInformation_AuthenticationError
 */
typedef enum PicselAuthenticationError
{
    PicselAuthenticationError_UnknownType = (1<<16), /**< Authentication
                                                          other than basic or
                                                          digest encountered
                                                          */
    PicselAuthenticationError_NoHeader,              /**< No authentication
                                                          header was found */
    PicselAuthenticationError_UnknownFormat          /**< The response header
                                                          was of an unknown
                                                          type */
}
PicselAuthenticationError;

/**
 * Gives details of an HTTP authentication error, sent with
 * an @ref AlienInformation_AuthenticationError event.
 *
 * Passed as @c eventData for an @ref AlienInformation_AuthenticationError
 * event, sent when HTTP authentication fails. @c sourceUrl may be NULL.
 *
 * @note All strings will be freed by the Picsel library when
 * AlienEvent_information() returns.
 */
typedef struct AlienInformation_AuthenticationErrorInfo
{
    Picsel_View              *picselView;  /**< View handle */
    PicselAuthenticationError error;       /**< Error description */
    const char               *sourceUrl;   /**< URL of page that requested
                                                authentication (UTF8) */
    PicselContentType         contentType; /**< The content type of the source
                                                URL */
}
AlienInformation_AuthenticationErrorInfo;


/**
 * Errors for @ref AlienInformation_AntiVirusError.
 */
typedef enum PicselAntiVirusError
{
    PicselAntiVirusError_Register = (1<<16), /**< Unable to register the callbacks
                                                  provided to PicselAntiVirus_register.
                                                  Picsel library will be unable to
                                                  support virus checking */
    PicselAntiVirusError_Initialise,         /**< Unable to initialise an alien
                                                  anti-virus context - Picsel will
                                                  not download the file */
    PicselAntiVirusError_URL,                /**< A URL was rejected by alien
                                                  application and not downloaded */
    PicselAntiVirusError_Content             /**< Data was rejected by alien
                                                  application and download was
                                                  stopped */
}
PicselAntiVirusError;

/**
 * Gives details of an error from the virus checker, sent with
 * an @ref AlienInformation_AntiVirusError event.
 *
 * Passed as @c eventData for an @ref AlienInformation_AntiVirusError event,
 * sent when the virus checker reports an error.
 * @c url will be NULL when error is @ref PicselAntiVirusError_Register.
 *
 * @note All strings will be freed by the Picsel library when
 * AlienEvent_information() returns.
 */
typedef struct AlienInformation_AntiVirusErrorInfo
{
    Picsel_View          *picselView; /**< NULL, reserved for future use. */
    PicselAntiVirusError  error;      /**< Error description */
    const char           *url;        /**< URL of the page with the error */
}
AlienInformation_AntiVirusErrorInfo;

/**
 * The security status of a remote document.
 */
typedef enum PicselSecureStatus
{
    PicselSecureStatus_Secure = (1<<16), /**< The page was loaded securely,
                                              and the certificate is valid */
    PicselSecureStatus_UserApproved,     /**< The page was loaded securely
                                              with an invalid certificate
                                              that was approved by the user */
    PicselSecureStatus_NotSecure         /**< The page was not loaded
                                              securely */
}
PicselSecureStatus;

/**
 * Gives details of the security status of a remote document, sent with
 * an @ref AlienInformation_SecurePage event.
 *
 * Passed as @c eventData for an @ref AlienInformation_SecurePage event,
 * when a secure page is loaded?
 *
 * This event will be sent to indicate whether a page is loading over a
 * secure connection, after a successful connection has been made
 * to the remote server.
 */
typedef struct AlienInformation_SecurePageInfo
{
    Picsel_View       *picselView;      /**< NULL, reserved for future use.*/
    PicselSecureStatus secure;          /**< Secure status of main page */
    int                insecureFrames;  /**< Set to 1 if there are insecure
                                             frames on the page */
}
AlienInformation_SecurePageInfo;

/**
 * Gives status of an HTTP file upload, sent with an
 * @ref AlienInformation_FileUploadStatus event.
 *
 * Passed as @c eventData for an @ref AlienInformation_FileUploadStatus
 * event, when the file has been successfully uploaded or the upload
 * has failed.
 *
 * @note All strings will be freed by the Picsel library when
 * AlienEvent_information() returns.
 */
typedef struct AlienInformation_FileUploadInfo
{
    Picsel_View     *picselView; /**< View handle */
    int              success;    /**< 1 if the upload success,
                                      0 if it failed */
    char            *filename;   /**< The file that has been uploaded, or
                                      NULL if the upload failed. */
}
AlienInformation_FileUploadInfo;

/**
 * Errors for AlienInformation_NetworkError
 */
typedef enum PicselNetworkError
{
    PicselNetworkError_DnsHostNotFound = (1<<16), /**< IP address can not be
                                                       obtained as host name
                                                       does not exist at DNS
                                                       server. */
    PicselNetworkError_TcpOpenFailed,             /**< TCP socket cannot be
                                                       opened */
    PicselNetworkError_TcpConnectFailed,          /**< TCP connect failed */
    PicselNetworkError_TcpReadFailed,             /**< Cannot read from TCP */
    PicselNetworkError_TcpWriteFailed,            /**< Cannot write to TCP */
    PicselNetworkError_SslConnectFailed,          /**< Cannot connect to
                                                       proxy with SSL */
    PicselNetworkError_SslHandshakeFailed,        /**< SSL handshake failed */
    PicselNetworkError_SslReadFailed,             /**< SSL read failed */
    PicselNetworkError_SslWriteFailed,            /**< SSL read failed */
    PicselNetworkError_TlsNotSupported,           /**< TLS not supported */
    PicselNetworkError_WriteTimerExpired,         /**< Write timer expired */
    PicselNetworkError_ReadTimerExpired,          /**< Read timer expired */
    PicselNetworkError_HttpReqGetTooBig,          /**< GET line size of the
                                                       HTTP request header is
                                                       bigger than max size */
    PicselNetworkError_HttpReqBodyTooBig,         /**< Body size of HTTP
                                                       request is bigger than
                                                       max size */
    PicselNetworkError_NetworkNotAvailable        /**< Networking is not
                                                       available */
}
PicselNetworkError;

/**
 * Gives details of a network error, sent with an
 * @ref AlienInformation_NetworkError event.
 *
 * Passed as @c eventData for an @ref AlienInformation_NetworkError event,
 * sent when there is a network error.
 * @c url below may be NULL.
 *
 * @note All strings will be freed by the Picsel library when
 * AlienEvent_information() returns.
 */
typedef struct AlienInformation_NetworkErrorInfo
{
    Picsel_View       *picselView;  /**< View handle */
    PicselNetworkError error;       /**< Error description */
    const char        *url;         /**< URL of content that was loading */
    PicselContentType  contentType; /**< The content type of the URL */
}
AlienInformation_NetworkErrorInfo;


/**
 * Type of activity, for @ref AlienInformation_NetworkActivity.
 *
 * These events are only enabled if PicselConfig_enableNotifyNetworkActivity
 * is enabled. If PicselConfig_reducedNotifyNetworkActivity
 * is also enabled, then these events are then only generated for the very
 * first HTTP fetch that occurs during a document fetch.
 */
typedef enum PicselNetworkActivity
{
    PicselNetworkActivity_HttpTransactionStarted,
                    /**< Sent at the start of a HTTP transaction, immediately
                     *   before we try to acquire a HTTP connection.
                     *   If we need to open a new connection,
                     *   the @ref AlienInformation_NetworkActivityInfo
                     *   'openingConnection' field is set 1. If we can reuse
                     *   an existing one, this is set to 0. */
    PicselNetworkActivity_SendingHttpRequest,
                    /**< Sent immediately before we send a HTTP request. */
    PicselNetworkActivity_ReceivingHttpResponse,
                    /**< Sent immediately before we start to read the HTTP
                     *   response. */
    PicselNetworkActivity_HttpTransactionComplete
                    /**< Sent when we've finished reading the HTTP response,
                     *   or earlier if the network connection has been
                     *   terminated. */
}
PicselNetworkActivity;

/**
 * Gives details of network activity, sent with an
 * @ref AlienInformation_NetworkActivity event.
 *
 * Passed as @c eventData for an @ref AlienInformation_NetworkActivity event,
 * sent whenever the Picsel library accesses the network.
 */
typedef struct AlienInformation_NetworkActivityInfo
{
    Picsel_View           *picselView;        /**< View handle */
    PicselNetworkActivity  activity;          /**< Activity description */
    const char            *url;               /**< URL of content that was
                                               *   loading */
    PicselContentType      contentType;       /**< The content type of the
                                               *   URL */
    int                    openingConnection; /**< Only used for
                                               *   HttpTransactionStarted
                                               *   event: Set to 1 if we're
                                               *   about to open a new
                                               *   connection; Set to 0 if
                                               *   we're able to reuse an
                                               *   existing one. */
}
AlienInformation_NetworkActivityInfo;


/**
 * Give details of the authentication information of a URL, sent with
 * an @ref AlienInformation_UrlAuthentication event.
 *
 * Passed as @c eventData for an @ref AlienInformation_UrlAuthentication
 * event, sent in response to PicselBrowser_getAuthorisationForUrl().
 *
 */
typedef struct AlienInformation_UrlAuthenticationInfo
{
    /** The URL for which information is being provided (i.e. the URL that
        the Alien application sent to PicselBrowser_getAuthorisationForUrl(). */
    const char *url;

    /** The cookies for the URL. All cookies are presented in a single string,
        not preceded by Cookies: or followed by a newline.  May be NULL. */
    const char *cookies;

    /** The username to use in authentication for the URL.  May be NULL. */
    const char *username;

    /** The password to use in authentication for the URL.  May be NULL. */
    const char *password;

    /** The realm to use in authentication for the URL.  May be NULL. */
    const char *realm;

    /** The challenge to use in authentication for the URL.  May be NULL. */
    const char *challenge;
}
AlienInformation_UrlAuthenticationInfo;


/**
 * Status for AlienInformation_CookiesInfo
 */
typedef enum PicselGetCookiesStatus
{
    PicselGetCookiesStatus_Success  = (1<<16), /**< Cookies obtained or
                                                    the enumeration is
                                                    complete. */
    PicselGetCookiesStatus_Failed              /**< No Cookies were
                                                    obtained. */
}
PicselGetCookiesStatus;

/**
 * Gives details of an individual cookie.
 *
 * Included in the @c cookies array of the @ref AlienInformation_CookiesInfo
 * struct.
 */
typedef struct AlienInformation_Cookie
{
    char          *domain;  /**< The domain used by the cookie. */
    char          *name;    /**< The name of the cookie. */
    char          *data;    /**< The value of the cookie. */
    char          *path;    /**< The path prefix for this cookie */
    unsigned long  expires; /**< The expiration date, which is the number of
                                 seconds since 00:00:00 on January 1, 1970,
                                 Coordinated Universal Time (UTC).
                                 If 0, the cookie will be removed when the
                                 application exits. */
    int            secure;  /**< 1 if cookie is only to be used on secure
                                 connections, otherwise 0. */
}
AlienInformation_Cookie;

/**
 * Gives details of an array of cookies, sent with
 * an @ref AlienInformation_GetCookies event.
 *
 * Passed as @c eventData for an @ref AlienInformation_GetCookies event,
 * in response to PicselBrowser_getCookies().
 */
typedef struct AlienInformation_CookiesInfo
{
    PicselGetCookiesStatus   status;  /**< The status of the event. */
    unsigned int             cookie;  /**< The cookie identifier supplied in
                                           the PicselBrowser_getCookies.
                                           call that generated this event */
    unsigned int             next;    /**< Cookie identifier, pass into next
                                           call to PicselBrowser_getCookies.
                                           A value of 0 indicates the
                                           enumeration is complete */
    unsigned int             count;   /**< Number of items in the cookies
                                           array. Can be 0 (enumeration
                                           complete) */
    AlienInformation_Cookie *cookies; /**< An array of cookies. Can be NULL */
}
AlienInformation_CookiesInfo;

/**
 * Status for AlienInformation_SetCookieInfo
 */
typedef enum PicselSetCookieStatus
{
    PicselSetCookieStatus_Success = (1<<16), /**< The cookie was set or was
                                                  rejected by the user */
    PicselSetCookieStatus_Failed             /**< The cookie was not set. */
}
PicselSetCookieStatus;

/**
 * Gives the result of a request to set a cookie, sent with
 * an @ref AlienInformation_SetCookieResult event.
 *
 * Passed as @c eventData for an @ref AlienInformation_SetCookieResult event,
 * sent in response to PicselBrowser_setCookie().
 *
 */
typedef struct AlienInformation_SetCookieInfo
{
    PicselSetCookieStatus  status;  /**< The status of the event. */
    const char            *urlName; /**< The url name supplied to
                                         PicselBrowser_setCookie() */
    const char            *cookie;  /**< The cookie string supplied to
                                         PicselBrowser_setCookie */
}
AlienInformation_SetCookieInfo;

/**
 * Gives an indication that an image has been sub-sampled, sent with
 * an @ref AlienInformation_ImageSubSampled event.
 *
 * Passed as @c eventData for an @ref AlienInformation_ImageSubSampled event,
 * when the display of a document has been sub-sampled.
 *
 * For information only; the Alien application need take no action.
 */
typedef struct AlienInformation_ImageSubSampledInfo
{
    Picsel_View *picselView;     /**< NULL, reserved for future use. */
}
AlienInformation_ImageSubSampledInfo;

/**
 * Gives an indication that the Picsel library is unable to fully display
 * a page, sent with an @ref AlienInformation_PageTooComplex event.
 *
 * Passed as @c eventData for an @ref AlienInformation_PageTooComplex event,
 * sent when some of the content of the current page of the document cannot
 * be displayed.
 *
 * The Alien application may inform the user, but no further
 * action is required.
 */
typedef struct AlienInformation_PageTooComplexInfo
{
    Picsel_View *picselView;     /**< NULL, reserved for future use. */
}
AlienInformation_PageTooComplexInfo;

/**
 * Gives an indication that a page is too tall, sent with an
 * @ref AlienInformation_PageTooTall event.
 *
 * Passed as @c eventData for an @ref AlienInformation_PageTooTall event,
 * sent when the height of the current page is too large for the Picsel
 * library to cope with.  Some content may be missing.
 *
 * The Alien application may inform the user, but no further action is
 * required.
 */
typedef struct AlienInformation_PageTooTallInfo
{
    Picsel_View *picselView;     /**< NULL, reserved for future use. */
}
AlienInformation_PageTooTallInfo;

/**
 * Gives an indication that an animation has started, sent with an
 * @ref AlienInformation_DynamicDocumentLoaded event.
 *
 * Passed as @c eventData for an @ref AlienInformation_DynamicDocumentLoaded
 * event, sent when a document containing dynamic content, such as an animation,
 * has loaded.
 */
typedef struct AlienInformation_DynamicDocumentLoadedInfo
{
    Picsel_View *picselView;     /**< NULL, reserved for future use. */
}
AlienInformation_DynamicDocumentLoadedInfo;

/**
 * Gives an indication that a document is loading, sent with an
 * @ref AlienInformation_DocumentLoading event.
 *
 * Passed as @c eventData for an @ref AlienInformation_DocumentLoading event,
 * sent when a document starts loading
 *
 * The Alien application may show a 'busy' indicator.
 * @ref AlienInformation_DocumentLoaded should cancel any 'busy' indicator.
 */
typedef struct AlienInformation_DocumentLoadingInfo
{
    Picsel_View   *picselView; /**< NULL, reserved for future use. */
    unsigned char *url;        /**< Url of the file/document, NULL if
                                    the url cannot be determined   */
}
AlienInformation_DocumentLoadingInfo;

/**
 * Provides information about a document which has been closed.  Sent with an
 * @ref AlienInformation_DocumentClosed event.
 *
 * Passed as @c eventData for an @ref AlienInformation_DocumentClosed event,
 * which is sent when a document has been closed and all non-shared resources
 * have been freed.
 */
typedef struct AlienInformation_DocumentClosedInfo
{
    Picsel_View   *picselView; /**< NULL, reserved for future use. */
    unsigned char *url;        /**< Url of the file/document, NULL if
                                    the url cannot be determined   */
}
AlienInformation_DocumentClosedInfo;

/**
 * Provides information about a document loading operation which has been
 * cancelled by the user.  Sent with an @ref AlienInformation_DocumentLoadCancelled
 * event.
 */
typedef struct AlienInformation_DocumentLoadCancelledInfo
{
    Picsel_View   *picselView; /**< NULL, reserved for future use. */
    unsigned char *url;        /**< Url of the file/document, NULL if
                                    the url cannot be determined   */
}
AlienInformation_DocumentLoadCancelledInfo;

/**
 * Gives details of the pointer threshold, sent with an
 * @ref AlienInformation_SetPointerThreshold event.
 *
 * Passed as @c eventData for an @ref AlienInformation_SetPointerThreshold
 * event, sent whenever the pointer threshold changes.
 * Specifies the requested sensitivity of the Alien application to pointer
 * moves.  So, if the Alien application detects a pointer movement less
 * than this threshold it can delay calling PicselPointer_move() until
 * further pointer moves are detected to bring the total pointer movement
 * above this threshold.
 *
 * However, if the time between moves below the threshold is significant,
 * then calling PicselPointer_move() anyway may improve the responsiveness
 * of the library. How long a time delay is "significant" will depend on the
 * properties of the device.
 *
 * @product Various Picsel products
 *
 */
typedef struct AlienInformation_SetPointerThresholdInfo
{
    int threshold; /**< Pointer moves of fewer than @c threshold pixels may be
                        discarded. */
}
AlienInformation_SetPointerThresholdInfo;

/**
 * Gives details of the new font size, sent with an
 * @ref AlienInformation_FontSizeChange event.
 *
 * Passed as @c eventData for an @ref AlienInformation_FontSizeChange event,
 * can be sent at any time while the Picsel library is running.
 *
 * For information only, the Alien application need take no action.
 *
 * @product Various Picsel products
 *
 */
typedef struct AlienInformation_FontSizeChangeData
{
    /** New text size set by Picsel library in points. Format is 16.16
     *  fixed point.  For example, a 10-point font size would be 10<<16
     */
    int newTextSize;
}
AlienInformation_FontSizeChangeData;

/**
 * Gives details of a file reference that is no longer required due to a
 * form being reset, sent with an @ref AlienInformation_FileSelectReset
 * event.
 *
 * Passed as @c eventData for an @ref AlienInformation_FileSelectReset event,
 * sent when an HTML form containing a path previously set by the
 * Alien application in response to a
 * @ref PicselUserRequest_Type_ChooseFileName request.
 *
 * Will only ever be sent if requested by the Alien application using
 * @c PicselConfig_FileSelectNotifyReset.
 *
 * The Alien application may find this useful if, perhaps for efficiency
 * reasons, it maintains some resources relating to the file the user
 * selected, expecting it to be accessed again soon.  When the form is reset,
 * the Alien application should free such resources.
 *
 * @product Browser products only.
 *
 */
typedef struct AlienInformation_FileSelectResetInfo
{
    Picsel_View     *picselView; /**< NULL, reserved for future use. */
    char            *filename;  /**< The file whose reference is no
                                    longer required. */
}
AlienInformation_FileSelectResetInfo;

/**
 * Backlight modes.
 * AlienBacklightMode_Enable causes the backlight to be turned on (or the
 * timer to be reset depending on device behaviour). Default indicates
 * the device should return to the device's default settings for the backlight.
 * Used in @ref AlienInformation_BacklightControlData.
 */
typedef enum AlienInformation_BacklightMode
{
    AlienBacklightMode_Default = (1<<16),
    AlienBacklightMode_Enable
}
AlienInformation_BacklightMode;

/**
 * Gives details of requested backlight mode, sent with
 * an @ref AlienInformation_BacklightControl event.
 *
 * Passed as @c eventData for an @ref AlienInformation_BacklightControl event,
 * sent at any time while the Picsel library is running.
 *
 * @product Various Picsel products
 */
typedef struct AlienInformation_BacklightControlData
{
    AlienInformation_BacklightMode  backlightMode;
}
AlienInformation_BacklightControlData;

/**
 * Gives details of requested setting of FullScreen mode, sent with
 * an @ref AlienInformation_SetFullScreen event.
 *
 * Passed as @c eventData for an @ref AlienInformation_SetFullScreen event,
 * sent at any time while the Picsel library is running.
 */
typedef struct AlienInformation_SetFullScreenInfo
{
    int                     enable;         /**< 1 to request FullScreen mode
                                                 be enabled;
                                                 0 to request FullScreen mode
                                                 not be enabled. */
}
AlienInformation_SetFullScreenInfo;

/**
 * Specification of screen orientations. Used by the
 * @ref AlienInformation_SetRequestedOrientationInfo event.
 */
typedef enum AlienScreenOrientation
{
    AlienScreenOrientation_Default = (1<<16),
                                /**< The device's default way of orientating
                                     the screen, which may be based on sensor
                                     input */
    AlienScreenOrientation_Landscape,
                                /**< The device's default Landscape
                                     orientation */
    AlienScreenOrientation_Portrait,
                                /**< The device's default Portrait
                                     orientation */
    AlienScreenOrientation_Locked
                                /**< Lock screen to the current orientation */
}
AlienScreenOrientation;

/**
 * Gives details of requested screen orientation, sent with
 * an @ref AlienInformation_SetRequestedOrientation event.
 *
 * Passed as @c eventData for an
 * @ref AlienInformation_SetRequestedOrientation event,
 * sent at any time while the Picsel library is running.
 *
 * Called with 'orientation' set to AlienScreenOrientation_Landscape or
 * AlienScreenOrientation_Portrait to request that the device be fixed in
 * landscape or portrait mode respectively.
 * Called with 'orientation' set to AlienScreenOrientation_Default to
 * cancel the request to fix the display in landscape or portrait mode.
 *
 * If supported and if a rotation is required, the alien would be expected to
 * either:
 * - Schedule resizing the alien screen and then making a call back to
 *   PicselScreen_resize, if the alien is handling the rotation; or
 * - Schedule making a call back to PicselScreen_rotate, if the Picsel
 *   library is handling the rotation.
 */
typedef struct AlienInformation_SetRequestedOrientationInfo
{
    AlienScreenOrientation  orientation;
}
AlienInformation_SetRequestedOrientationInfo;

/**
 * Power saving modes.
 * Disable causes the power saving mode to be turned off
 * (or the timer to be reset depending on device behaviour). Default
 * indicates the device should return to the device's default settings for power saving.
 * Used in @ref AlienInformation_PowerSavingData.
 */
typedef enum AlienInformation_PowerMode
{
    AlienPowerSavingMode_Default = (1<<16),
    AlienPowerSavingMode_Disable
}
AlienInformation_PowerMode;

/**
 * Gives details of requested power saving mode, sent with
 * an @ref AlienInformation_PowerSavingMode event.
 *
 * Passed as @c eventData for an @ref AlienInformation_PowerSavingMode event,
 * sent at any time while the Picsel library is running.
 *
 */
typedef struct AlienInformation_PowerSavingData
{
    AlienInformation_PowerMode     powerSavingMode;
}
AlienInformation_PowerSavingData;

/**
 * Instructs the host what rotation to apply to the buffer
 * passed by an AlienScreen_update. Sent with an
 *  @ref AlienInformation_ChangeUpdateRotation event.
 *
 * Passed as @c eventData for an @ref
 * AlienInformation_ChangeUpdateRotation event.
 *
 * Will only ever be sent if the Alien application indicates
 * support via @c PicselConfig_UpdateRotationCapable.
 */
typedef struct AlienInformation_ChangeUpdateRotationData
{
    PicselRotation newRotation;
}
AlienInformation_ChangeUpdateRotationData;

/**
 * Gives details of the requested new screen orientation, sent with
 * an @ref AlienInformation_ChangeHwOrientation event.
 *
 * Passed as @c eventData for an @ref AlienInformation_ChangeHwOrientation
 * event, sent to indicate the new screen orientation to set and
 * provide an identifier to pass to PicselScreen_notifyHwOrientationChange()
 * on completion.
 *
 * This call is used when the device has a method to plot its screen
 * in landscape with hardware acceleration. It's expected that the Alien
 * application will need to call PicselScreen_resize() during this operation
 * and it should wait for the resize result before calling
 * PicselScreen_notifyHwOrientationChange() to indicate the overall success
 * or failure of the operation.
 *
 */
typedef struct AlienInformation_HwOrientation
{
    PicselRotation      newOrientation;
    int                 uId; /**< Operation identifier to pass to
                                  PicselScreen_notifyHwOrientationChange() */
}
AlienInformation_HwOrientation;

/**
 * Thumbnailer status data.
 */
typedef enum AlienInformation_ThumbnailerStatusData
{
    PicselThumbnailer_Started = (1<<16),
    PicselThumbnailer_Stopped
}
AlienInformation_ThumbnailerStatusData;

/**
 * Gives details of thumbnailer's status, sent with
 * an @ref AlienInformation_ThumbnailerStatus event.
 *
 * Passed as @c eventData for an @ref AlienInformation_ThumbnailerStatus event,
 * sent at any time while the thumbnailer is running.
 *
 */
typedef struct AlienInformation_ThumbnailerStatusInfo
{
    AlienInformation_ThumbnailerStatusData thumbnailerStatus;
}
AlienInformation_ThumbnailerStatusInfo;

/**
 * Passed as @c eventData for an @ref AlienInformation_PageSummaryText event,
 * sent in response to a PicselFileviewer_getPageSummaryText() call.
 *
 * Also passed as @c eventData for an @ref AlienInformation_PageText event,
 * sent in response to a PicselFileviewer_getPageText() call.
 *
 * @note The string will be freed by the Picsel library when
 *       AlienEvent_information() returns.
 */
typedef struct AlienInformation_PageTextInfo
{
    int          success;  /**< 1 if the operation succeeded,
                            *   0 if an error occurred */

    int          page;     /**< The page which the text was retreived from. */
    Picsel_Utf8 *text;     /**< A NULL terminated string containing the text.
                            *   Will be NULL if no text was found, or on error.
                            */
}
AlienInformation_PageTextInfo;

/**
 * Passed as @c eventData for an @ref AlienInformation_ContinuousScrollingMode
 * event, sent in response to a PicselFileviewer_enableContinuousScrollingMode()
 * call.
 */
typedef struct AlienInformation_ContinuousScrollingModeInfo
{
    int success;  /**< 1 if the operation succeeded,
                   *   0 if an error occurred */
}
AlienInformation_ContinuousScrollingModeInfo;

/**
 * Passed as @c eventData for an @ref AlienInformation_InertialScrollingMode
 * event, sent in response to a PicselFileviewer_enableInertialScrollingMode()
 * call.
 */
typedef struct AlienInformation_InertialScrollingModeInfo
{
    int success;  /**< 1 if the operation succeeded,
                   *   0 if an error occurred */
}
AlienInformation_InertialScrollingModeInfo;

/**
 * Passed as @c eventData for an @ref AlienInformation_InlineTextEntry
 * event.
 */
typedef struct AlienInformation_InlineTextEntryInfo
{
    int sendText;  /**< 1 if it would be appropriate to send text entry events
                    *   0 if text entry events are likely to be ignored */
    int secret;    /**< 1 if text being entered is secret and no predictive
                    *     text candidates should be displayed
                    *   0 if text entry isn't secret */
    int reset;     /**< 1 if the current composing text ought be reset
                    *   0 if not */
}
AlienInformation_InlineTextEntryInfo;

/**
 * Passed as @c eventData for an @ref AlienInformation_Clipboard event.
 */
typedef struct AlienInformation_ClipboardData
{
    /** UTF-8 formatted text to be copied to the clipboard, should
     *  be copied by the Alien as it isn't guaranteed to exist beyond
     *  the scope of this event */
    const char *text;
}
AlienInformation_ClipboardData;

/**
 * Passed as @c eventData for an @ref AlienInformation_MenuItemSelected event.
 */
typedef struct AlienInformation_MenuItemSelectedData
{
    /** The id of selected menu item. Please Ask your Picsel representative
     *  for more information. */
    int menuItemId;
}
AlienInformation_MenuItemSelectedData;

/**
 * The state of a PicselPrint_printDocument() operation
 */
typedef enum AlienInformation_PrintDocumentState
{
    /**< Page data is currently being generated */
    AlienInformation_PrintDocumentState_GeneratingPageData = 65536,

    /**< Page data is being sent to the printer */
    AlienInformation_PrintDocumentState_SendingDataToPrinter
}
AlienInformation_PrintDocumentState;

/**
 * Passed as @c eventData for an @ref AlienInformation_PrintDocumentProgress event
 */
typedef struct AlienInformation_PrintDocumentProgressData
{
    /**< The state of the current print operation */
    AlienInformation_PrintDocumentState state;

    /**< The page currently being processed.  This is the printed page number
     *   rather than the document page number.  Page numbers start at 1. */
    unsigned int                        currentPage;

    /**< The total number of pages to be printed */
    unsigned int                        totalPages;
}
AlienInformation_PrintDocumentProgressData;

/**
 * Error codes associated with a @ref PicselPrint_printDocument request
 */
typedef enum AlienInformation_PrintDocumentResult_Error
{
    /**< No error occurred */
    AlienInformation_PrintDocumentResult_Error_NoError,

    /**< The operation was cancelled with a call to @ref PicselPrint_cancel */
    AlienInformation_PrintDocumentResult_Error_Cancelled,

    /**< The is no document loaded */
    AlienInformation_PrintDocumentResult_Error_DocumentNotLoaded,

    /**< An unknown error occurred */
    AlienInformation_PrintDocumentResult_Error_Unknown,

    /**< Invalid parameters were supplied in the @ref PicselPrint_printDocument
     *   request */
    AlienInformation_PrintDocumentResult_Error_BadParams,

    /**< The Picsel library ran out of memory while generating the print
     *   data */
    AlienInformation_PrintDocumentResult_Error_OutOfMemory,

    /**< The specified printer was not found */
    AlienInformation_PrintDocumentResult_Error_PrinterNotFound,

    /**< Force size to be 32-bit */
    AlienInformation_PrintDocumentResult_Error_ForceSize = (1<<16)
}
AlienInformation_PrintDocumentResult_Error;

/**
 * Passed as @c eventData for an @ref AlienInformation_PrintDocumentResult event
 */
typedef struct AlienInformation_PrintDocumentResultData
{
    /**< The result of the operation.  1 == success, 0 == failure */
    unsigned char                              result;

    /**< Indicates the reason for failure */
    AlienInformation_PrintDocumentResult_Error error;
}
AlienInformation_PrintDocumentResultData;

/**
 * Information about a found printer.
 */
typedef struct AlienInformation_PrinterInfo
{
    const char            *ip;              /**< The printer's IP address */
    const char            *name;            /**< The printer's name */
    const char            *description;     /**< The printer's description */
    PicselPrint_Emulation  emulation;       /**< The printer emulation mode */
    unsigned char          colourSupported; /**< Indicates whether or not
                                             *   colour printing is supported/
                                             *   0 == mono, 1 == colour */
}
AlienInformation_PrinterInfo;

/**
 * Passed as @c eventData for an @ref AlienInformation_SearchForPrintersResult
 * event
 */
typedef struct AlienInformation_SearchForPrintersData
{
    AlienInformation_PrinterInfo *printers;    /**< Array of printers found.
                                                *   NULL if no printers
                                                *   were found. */
    int                           numPrinters; /**< Number of entries in
                                                *   the printers array */
}
AlienInformation_SearchForPrintersData;

/**
 * Passed as @c eventData for an @ref AlienInformation_DocumentsOpenCount event
 */
typedef struct AlienInformation_DocumentsOpenCountInfo
{
    int numberOfOpenDocuments;  /**< The number of documents currently open in
                                 *   the application */
}
AlienInformation_DocumentsOpenCountInfo;

/**
 * @}
 */

#endif /* !ALIEN_NOTIFY_H */
