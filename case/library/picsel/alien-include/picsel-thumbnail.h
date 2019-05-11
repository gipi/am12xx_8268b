/**
 * Page Thumbnail display within File Viewer
 *
 * $Id: picsel-thumbnail.h,v 1.29 2011/09/13 14:37:38 neilk Exp $
 * @file
 */
/* Copyright (C) Picsel, 2005. All Rights Reserved. */
/**
 * @defgroup TgvPageThumbnail Page Thumbnail
 * @ingroup TgvFileViewer
 *
 * Displays a small image of the current page in the corner of the screen as
 * well as the main document, to help navigate within large pages, when using
 * a Picsel document viewing product such as @ref TgvFileViewer.
 *
 * This is not related to the @ref ThumbnailGenerator or @ref
 * ThumbnailDatabase products (see @ref TgvProducts).
 *
 * @{
 */

#ifndef PICSEL_THUMBNAIL_H
#define PICSEL_THUMBNAIL_H

#include "alien-types.h"
#include "picsel-control.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Thumbnail mode. Used in @ref AlienInformation_ThumbnailResultInfo to
 * return the current thumbnail mode, and in PicselThumbnail_mode() to define
 * the required mode.
 */
typedef enum Picsel_ThumbnailMode
{
    /**
     * No thumbnail displayed.
     */
    Thumbnail_None = (1<<16),

    /**
     * Display a thumbnail on a transparent panel at the bottom of the
     * screen; superimposed on this thumbnail is a box indicating the current
     * position and zoom factor.
     */
    Thumbnail_DocumentMap,

    /**
     * Display a thumbnail on a black background at the top of the screen; a
     * magnifying glass icon shows the current document position, and the
     * bottom part of the screen shows the zoomed portion of the document.
     */
    Thumbnail_Magnify,

    /**
     * Display a grid of thumbnails for multiple pages;  The current document
     * view will be completely hidden.
     *
     * Whilst in this mode, pan and zoom requests are ignored.  Previous and
     * Next Page requests cause the view to be rebuilt with the previous/next
     * set of pages.
     *
     * Multiple pages currently supports two sizes:
     *   2 => 2 thumbnails in the grid.  When the screen is square or
     *        portrait orientation, the grid will be 2 thumbnails high and 1
     *        thumbnail wide.  When the screen is landscape orientation, the
     *        grid will be 2 thumbnails wide and 1 thumbnail high.
     *
     *   4 => 4 thumbnails in the grid.  Always 2 thumbnails high and 2
     *        thumbnails wide.
     *
     * @ref PicselConfig_enableMultiplePagesModeDocumentSizeLimit can be
     * enabled to ensure that if the currently loaded document does not
     * contain enough pages for the view, an error will be returned and the
     * thumbnail mode will not be changed.
     */
    Thumbnail_MultiplePages
}
Picsel_ThumbnailMode;

/**
 * Positioning of thumbnail onscreen: horizontal.
 * See PicselThumbnail_setPosition().
 */
typedef enum Thumbnail_XPos
{
    Thumbnail_XPos_Left = (1<<16), /**< Left of screen. */
    Thumbnail_XPos_Centre,         /**< Centred horizontally. */
    Thumbnail_XPos_Right,          /**< Right of screen. */
    Thumbnail_XPos_Default         /**< Use default value for this mode. */
}
Thumbnail_XPos;

/**
 * Positioning of thumbnail onscreen: vertical.
 * See PicselThumbnail_setPosition().
 */
typedef enum Thumbnail_YPos
{
    Thumbnail_YPos_Top = (1<<16),  /**< Top of screen. */
    Thumbnail_YPos_Centre,         /**< Centred vertically. */
    Thumbnail_YPos_Bottom,         /**< Bottom of screen. */
    Thumbnail_YPos_Default         /**< Use default value for this mode. */
}
Thumbnail_YPos;

/**
 * Information events from the Picsel Application.
 */
enum
{
    /**
     * Thumbnail result returned.
     */
    AlienInformation_ThumbnailResult = 0x12000,

    /**
     * Thumbnail of a specified page has been generated.
     */
    AlienInformation_ThumbnailGenerated,

    /**
     * Thumbnail dimensions of a specified page have been generated.
     * @c eventData is @ref AlienInformation_ThumbnailDimensionsInfo.
     */
    AlienInformation_ThumbnailDimensions
};

/**
 * Thumbnail-mode operation result.
 */
typedef enum PicselThumbnail_Result
{
    /**
     * Operation was successful.
     */
    PicselThumbnail_Success = (1<<16),

    /**
     * Operation failed.
     */
    PicselThumbnail_Failure,

    /**
     * Operation succeeded, but no thumbnail is available.
     */
    PicselThumbnail_NoThumbnail
}
PicselThumbnail_Result;

/**
 * Thumbnail mode scaling rule for PicselThumbnail_setScalingRule(). Only
 * apply when thumbnail is too small. The default is
 * @ref Thumbnail_Scaling_StretchWidth.
 */
typedef enum Thumbnail_ScalingRule
{
    /**
     * Do not scale thumbnail.
     */
    Thumbnail_Scaling_None = (1<<16),

    /**
     * Scale thumbnail so it has the same proportions.
     */
    Thumbnail_Scaling_StretchProportional,

    /**
     * Just scale thumbnail width (default).
     */
    Thumbnail_Scaling_StretchWidth
}
Thumbnail_ScalingRule;


/**
 * Use this to specify a default size in PicselThumbnail_mode() or
 * PicselThumbnail_setScalingRule().
 */
enum
{
    Thumbnail_UseDefault = 0
};

/**
 * Thumbnail generation output formats
 */
typedef enum
{
    PicselThumbnail_OutputFormat_Rgb565,            /**< RGB565 format */
    PicselThumbnail_OutputFormat_Png,               /**< PNG format */
    PicselThumbnail_OutputFormat_Jpeg,              /**< Jpeg format */
    PicselThumbnail_OutputFormat_ForceSize = 65536  /**< Force the size of
                                                     *   the enum to 32 bit */
}
PicselThumbnail_OutputFormat;


/**
 * Thumbnail-mode operation result information.
 *
 * This structure will be passed to AlienEvent_information().  Not all
 * operations will report failure.  The Alien application should only assume
 * a change has taken place when the fields below update.
 */
typedef struct AlienInformation_ThumbnailResultInfo
{
    /**
     * Outcome of operation.
     */
    PicselThumbnail_Result result;

    /**
     * Current thumbnail mode.
     */
    Picsel_ThumbnailMode   mode;

    /**
     * Current thumbnail size. Either @ref Thumbnail_UseDefault or a value
     * set by PicselThumbnail_mode().
     */
    int                    size;

    /**
     * 1 if the thumbnail is visible; 0 otherwise.
     */
    int                    visible;

    /**
     * Current x position.
     */
    Thumbnail_XPos         xPos;

    /**
     * Current y position.
     */
    Thumbnail_YPos         yPos;

    /**
     * Current scaling rule.
     */
    Thumbnail_ScalingRule  scalingRule;
}
AlienInformation_ThumbnailResultInfo;


/**
 * Dimensions for a specified page
 */
typedef struct AlienInformation_ThumbnailDimensionsInfo
{
    PicselThumbnail_Result result; /**< Result of the operation */
    unsigned int           page;   /**< The page number.  Numbered from 0 */
    unsigned int           dpi;    /**< The dpi used for the calculation  */
    unsigned int           width;  /**< The width of the resulting thumbnail
                                    *   (in pixels) */
    unsigned int           height; /**< The height of the resulting thumbnail
                                    *   (in pixels) */
}
AlienInformation_ThumbnailDimensionsInfo;


/**
 * Specify how thumbnails are to be used when viewing the document.
 *
 * While viewing a page, a thumbnail can be displayed to:
 * - give an overview of the current page;
 * - show the position of the current visible screen on the page.
 *
 * Currently, there are three modes that can be used.  For document map mode,
 * the visible area of the document is the same, and the thumbnail is
 * superimposed at the bottom of the screen, on a transparent panel.  A
 * red transparent box on top of this thumbnail shows the current screen
 * position and size.  For magnify mode, the screen is split in two. A
 * magnifing glass icon can be used to navigate the thumbnail at the top,
 * and the bottom portion shows the zoomed document.  For multiple pages
 * mode, a grid of thumbnails is displayed.
 *
 * Thumbnail mode cannot be enabled until a document has loaded and the
 * @ref AlienInformation_DocumentLoaded event has been received.
 *
 * Magnify mode and multiple pages mode are only compatible with the normal
 * flow mode.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 * @param[in] mode          The thumbnail mode.
 * @param[in] size          Maximum height of thumbnail in pixels. This
 *                          height is used as the split screen height in
 *                          magnify mode; if 0 is passed, the split is
 *                          halfway up the screen.  In Multiple Pages mode,
 *                          size indicates the number of thumbnails to be
 *                          shown in the grid (currently only 2 and 4 are
 *                          supported).
 *
 * @return                  The queue status, normally 1. See @ref
 *                          TgvAsync_Queue. If the event is accepted, the Alien
 *                          application will receive an @ref
 *                          AlienInformation_ThumbnailResult event indicating
 *                          success or failure of the operation.
 */
int PicselThumbnail_mode(Picsel_Context       *picselContext,
                         Picsel_ThumbnailMode  mode,
                         int                   size);

/**
 * Set the position of the thumbnail onscreen. This works in both
 * @ref Thumbnail_DocumentMap and @ref Thumbnail_Magnify modes. See @ref
 * Thumbnail_XPos and @ref Thumbnail_YPos.
 *
 * This operation has no effect in @ref Thumbnail_MultiplePages mode.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 * @param[in] xPosition     Horizontal positioning.
 * @param[in] yPosition     Vertical positioning.
 *
 * @return                  The queue status, normally 1. See @ref
 *                          TgvAsync_Queue.
 */
int PicselThumbnail_setPosition(Picsel_Context *picselContext,
                                Thumbnail_XPos  xPosition,
                                Thumbnail_YPos  yPosition);

/**
 * Pan document, with support for panning by dragging the thumbnail position
 * box.
 *
 * Pans are part of a sequence, and the position in that sequence is shown by
 * by panState. A sequence begins with panState set to
 * @ref PicselControl_Start, and ends with panState set to
 * @ref PicselControl_End. Intermediate calls in the sequence have panState
 * set to @ref PicselControl_Continue. See @ref PicselControl_State diagram.
 *
 * The screen coordinates of the pointer are passed in (as xCoordinate and
 * yCoordinate). The function's action depends on where those pointer
 * coordinates are, according to the following three cases:
 * -# Inside the red thumbnail overlay (and therefore also within the
 * thumbnail). panX and panY are relative movements of the overlay. The
 * effect of panX and panY is scaled up in proportion to the size of the
 * thumbnail on screen, so even small values can make the document move by a
 * large amount.
 * -# Outside the red thumbnail overlay but still within the thumbnail. panX
 * and panY are ignored. The red overlay moves to the new pointer position.
 * -# Outside the red thumbnail overlay and also outside the thumbnail.
 * panX and panY move the whole document and the API then behaves exactly
 * like PicselControl_pan().
 *
 * The following is recommended:
 * - panX should be the difference between the current xCoordinate and the
 * previous value;
 * - panY should be the difference between the current yCoordinate and the
 * previous value.
 *
 * Assume that the origin (0,0) is at the top left of the physical screen and
 * co-ordinates grow downwards and to the right. See @ref TgvCoordinate_Space
 * and @ref TgvCoordinates_Relative.
 *
 * This operation has no effect in @ref Thumbnail_MultiplePages mode.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 * @param[in] xCoordinate   Horizontal position, in pixels.
 * @param[in] yCoordinate   Vertical position, in pixels.
 * @param[in] panX          Horizontal pan amount (negative moves document
 *                          left, positive moves document right).
 * @param[in] panY          Vertical pan amount (negative moves document up,
 *                          positive moves document down).
 * @param[in] panState      Whether starting, continuing, or completing a
 *                          pan operation. See @ref PicselControl_State.
 * @return                  The queue status, normally 1. See @ref
 *                          TgvAsync_Queue.
 *
 * @pre If a previous call of this function has already been made, panState
 * should be set to @ref PicselControl_Continue or @ref PicselControl_End.
 * @post If panState was not @ref PicselControl_End, then another call should
 * be made later with a state of @ref PicselControl_End. The screen display
 * quality will be poor until this is done.
 */
int PicselThumbnail_panPageOrThumbnail(Picsel_Context     *picselContext,
                                       int                 xCoordinate,
                                       int                 yCoordinate,
                                       int                 panX,
                                       int                 panY,
                                       PicselControl_State panState);

/**
 * In reflow mode, documents can be very tall but very thin, making
 * thumbnails difficult to see.
 *
 * Setting 'rule' to anything other than @ref Thumbnail_Scaling_None means
 * that a thumbnail will be tested to see if it is thinner than 'minWidth'.
 * If so, the thumbnail will be scaled so that it is 'minWidth' wide. This
 * will either be done maintaining the proportions (so the thumbnail will
 * get taller than the usual thumbnail area) or by making the thumbnail
 * wider but not taller (which means it won't exactly correspond to the
 * shape of the screen).
 *
 * Currently, this applies only to document map mode.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 * @param[in] rule          Rule to use when a thumbnail is less than the
 *                          minWidth wide.
 * @param[in] minWidth      Minimum width of thumbnail in pixels. Thumbnail
 *                          will be scaled if the width is less than this.
 *
 * @return                  The queue status, normally 1. See @ref
 *                          TgvAsync_Queue.
 *
 * @pre Currently, this applies only to @ref Thumbnail_DocumentMap mode.
 */
int PicselThumbnail_setScalingRule(Picsel_Context       *picselContext,
                                   Thumbnail_ScalingRule rule,
                                   int                   minWidth);

/**
 * Hide the current thumbnail and everything associated with it. This
 * un-splits the screen and removes the magnifying glass pointer in @ref
 * Thumbnail_Magnify mode, the highlighted overlay in @ref
 * Thumbnail_DocumentMap mode, and the grid of thumbnails in @ref
 * Thumbnail_MultiplePages mode.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 *
 * @return                  The queue status, normally 1. See @ref
 *                          TgvAsync_Queue.
 */
int PicselThumbnail_hide(Picsel_Context *picselContext);

/**
 * Unhide the current thumbnail and everything associated with it, including
 * the screen split and magnifying glass pointer in @ref Thumbnail_Magnify
 * mode, the highlighted overlay in  @ref Thumbnail_DocumentMap mode, and
 * the grid of thumbnails in @ref Thumbnail_MultiplePages mode.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 *
 * @return                  The queue status, normally 1. See @ref
 *                          TgvAsync_Queue.
 */
int PicselThumbnail_show(Picsel_Context *picselContext);

/**
 * Generate a thumbnail of a certain page in desired dimension.
 *
 * When thumbnail generation has been completed,
 * @ref AlienInformation_ThumbnailGenerated event is delivered to the client
 * with a parameter, @ref AlienInformation_ThumbnailDoneInfo, which holds
 * thumbnail image data and information. This parameter is NULL if the
 * thumbnail was not successfully generated.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 * @param[in] page          Page number of thumbnail to be generated,
 *                          starting from zero.
 * @param[in] width         Width of thumbnail in pixels.
 * @param[in] height        Height of thumbnail in pixels.
 * @param[in] ratio         The maximum aspect ratio of a thumbnail to be
 *                          generated as an integer percentage. If the page's
 *                          aspect ratio exceeds this, the thumbnail will
 *                          represent only the top left of the page (the
 *                          longer side may be cropped). For example, for A4
 *                          and Letter size page, it should be 150; for
 *                          lengthy pages it should be 100.
 *
 * @return                  The queue status, normally 1. See @ref
 *                          TgvAsync_Queue.
 */
int PicselThumbnail_generate(Picsel_Context *picselContext,
                             int             page,
                             int             width,
                             int             height,
                             int             ratio);

/**
 * Generate a thumbnail of a certain page with the desired DPI.
 *
 * The size of the generated thumbnail can be constrained by setting maxWidth
 * and maxHeight.  This will crop the thumbnail to maxHeight/maxWidth pixels
 * from the top/left corner.
 *
 * When thumbnail generation has been completed,
 * @ref AlienInformation_ThumbnailGenerated event is delivered to the client
 * with a parameter, @ref AlienInformation_ThumbnailDoneInfo, which holds
 * thumbnail image data and information. This parameter is NULL if the
 * thumbnail was not successfully generated.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 * @param[in] page          Page number of thumbnail to be generated,
 *                          starting from zero.
 * @param[in] maxWidth      Maximum width of thumbnail in pixels.  Set to 0
 *                          if no maximum is to be applied.
 * @param[in] maxHeight     Maximum height of thumbnail in pixels.  Set to 0
 *                          if no maximum is to be applied.
 * @param[in] dpi           The DPI to render the page at.
 * @param[in] format        The output format to use.
 * @param[in] quality       The quality of output required - Only used when
 *                          PicselThumbnail_OutputFormat_Jpeg is used.
 *                          Suitable values are in the range 0 to 100.
 *
 * @return                  The queue status, normally 1. See @ref
 *                          TgvAsync_Queue.
 */
int PicselThumbnail_generateWithDpi(Picsel_Context               *picselContext,
                                    int                           page,
                                    int                           maxWidth,
                                    int                           maxHeight,
                                    int                           dpi,
                                    PicselThumbnail_OutputFormat  format,
                                    int                           quality);

/**
 * Cancel any outstanding thumbnail generation requests
 *
 * Any queued or in-progress thumbnail generation operations will be
 * cancelled.  No @ref AlienInformation_ThumbnailGenerated event will be
 * sent for these operations.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 *
 * @return                  The queue status, normally 1. See @ref
 *                          TgvAsync_Queue.
 */
int PicselThumbnailer_stopThumbnailGeneration(Picsel_Context *picselContext);

/**
 * Calculate the dimensions of the specified page when rendered at the
 * specified dpi.
 *
 * The dimensions will be calculated asynchronously and reported back to the
 * alien in an  @ref AlienInformation_ThumbnailDimensions information event.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 * @param[in] page          Page number to get the dimensions of, starting
 *                          from zero.
 * @param[in] dpi           The DPI to use for the calculation
 *
 * @return                  The queue status, normally 1. See @ref
 *                          TgvAsync_Queue.
 */
int PicselThumbnail_getThumbnailDimensions(Picsel_Context *picselContext,
                                           int             page,
                                           int             dpi);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PICSEL_THUMBNAIL_H */
