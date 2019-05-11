/**
 * Configuration options for Picsel TGV products.
 *
 *
 * $Id: picsel-config.h,v 1.149.10.4 2014/11/21 03:26:30 gyunam Exp $
 * @file
 */
/* Copyright (C) Picsel, 2005-2008. All Rights Reserved. */
/**
 * @addtogroup TgvConfigureCore
 *
 * @{
 */

#ifndef PICSEL_CONFIG_H
#define PICSEL_CONFIG_H

#include "alien-types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Set an integer configuration value.
 *
 * Set an integer configuration value, such as the size of the data download
 * cache (@ref PicselConfig_cacheMaxSize). There are many configuration
 * options in TGV, described in @ref TgvConfigureCore.
 * Only the configuration options relevant for a product will have an effect
 * within the Picsel library, see @ref TgvProducts.
 *
 * This function will usually be called during initialisation, from within
 * AlienConfig_ready(), although some options can be set while running.
 *
 * @param[in] picselContext See AlienEvent_setPicselContext().
 * @param[in] key           Any configuration option with an integer value,
 *                          and which is relevant to this product.
 * @param[in] value         The numeric value to set.
 *
 * @return The queue status, normally 1. See @ref TgvAsync_Queue.
 *
 * @pre  This function is usually called from within AlienConfig_ready().
 *       Although some options can be set during operation , many will only
 *       take effect during initialisation.
 * @post The configuration may be ignored by the Picsel library if it is
 *       outside the valid range, or it may take effect much later,
 *       depending on the state of the Picsel library.
 * @see  PicselConfig_setString().
 */
int PicselConfig_setInt(Picsel_Context *picselContext,
                        const char *key,
                        int value);

/**
 * Set a string configuration value.
 *
 * Set a string configuration value, such as the location where vector fonts
 * can be found (@ref PicselConfig_fontsDirectory). There are many
 * configuration options in TGV, described in @ref TgvConfigureCore.
 * Only the configuration options relevant for a product will be available in
 * the Picsel library, see @ref TgvProducts.
 *
 * This functions will usually be called during initialisation, from within
 * AlienConfig_ready(), although some options can be set while running.
 *
 * @param[in] picselContext See AlienEvent_setPicselContext()
 * @param[in] key           Any configuration option with an string value,
 *                          and which is relevant to this product.
 * @param[in] str           The value to set. It must be encoded in UTF-8,
 *                          and null-terminated.
 *
 * @return The queue status, normally 1. See @ref TgvAsync_Queue.
 *
 * @pre  This function is usually called from within AlienConfig_ready().
 *       Although some options can be set during operation , many will only
 *       take effect during initialisation.
 * @post The configuration may be ignored by the Picsel library if it is
 *       outside the valid range, or it may take effect much later,
 *       depending on the state of the Picsel library.
 * @see  PicselConfig_setInt().
 */
int PicselConfig_setString(Picsel_Context *picselContext,
                           const char     *key,
                           const char     *str);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/**
 * A type definition for a fixed point 16.16 number.
 *
 * This type is used to define a fixed point number which has 16 bits of
 * integer precision and 16 bits of fractional precision.
 * This type can be used to define screen sizes measured in fractions of an
 * inch, or define the point size of a font. For example a 12-point font size
 * would be 12<<16.
 */
typedef unsigned long Picsel_Fixed1616;

/**
 * A type definition for a signed fixed point 16.16 number.
 *
 * This type is used to define a fixed point number which has 16 bits
 * of signed integer precision and 16 bits of fractional precision.
 */
typedef long Picsel_SignedFixed1616;


/**
 * Picsel colour definition.
 *
 * 24 bits of colour (8 red, 8 green, 8 blue) plus an 8 bit alpha channel
 * where the red component is stored in the most significant bits.
 * PicselConfig_rgbaFromComponents() can be used to populate it.
 *
 * This type is frequently used to pass to colour values to configuration
 * parameters via PicselConfig_setInt().
 *
 * For the colour components @c 0x00 is black and @c 0xff is full on for each
 * of the colour components.
 * For the alpha channel, @c 0 is fully transparent, @c 0xff is fully opaque.
 */
typedef unsigned long Picsel_ColourRgba;

/**
 * Builds a value of type @ref Picsel_ColourRgba from the component values.
 *
 * This can be used to generate colour values that can be passed to
 * configuration parameters via PicselConfig_setInt().
 *
 * @param r The red component, with a range of @c 0x00 to @c 0xff. Where
 *          @c 0x00 is black and @c 0xff is red full on.
 * @param g The green component, with a range of @c 0x00 to @c 0xff. Where
 *          @c 0x00 is black and @c 0xff is green full on.
 * @param b The blue component, with a range of @c 0x00 to @c 0xff. Where
 *          @c 0x00 is black and @c 0xff is blue full on.
 * @param a The alpha channel, with a range of @c 0x00 to @c 0xff. Where
 *          @c 0x00 is fully transparent and @c 0xff is fully opaque.
 *
 * @return The combined colour components and alpha channel as type
 *         @ref Picsel_ColourRgba.
 */
#define PicselConfig_rgbaFromComponents(r, g, b, a) \
(                                                   \
    ((((Picsel_ColourRgba)(r)) & 0xff) << 24 ) |    \
    ((((Picsel_ColourRgba)(g)) & 0xff) << 16 ) |    \
    ((((Picsel_ColourRgba)(b)) & 0xff) <<  8 ) |    \
    ((((Picsel_ColourRgba)(a)) & 0xff)       )      \
)

/** @} */ /* End Doxygen group TgvConfigureCore */

/**
 * @defgroup TgvConfigGeneral General Configuration Options
 * @ingroup TgvConfigureCore
 *
 * Configurable parameters that apply to many TGV products.
 *
 * These properties may be set in a variety of products. If used in
 * an inappropriate product, it will be ignored. These should generally
 * be set with PicselConfig_setInt() or PicselConfig_setString() before
 * the TGV product is launched.
 *
 * @{
 */

/**
 * Configuration parameter for setting the border colour used to decorate a
 * focused element.
 *
 * This configuration parameter would typically be set in
 * AlienConfig_ready(). If it is set at any other time, then it will only
 * take effect when the next document is loaded.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 * The colour is specified as type @ref Picsel_ColourRgba,
 * PicselConfig_rgbaFromComponents() can be used to set this value.
 *
 * A focused element is highlighted in the weak focus mode, @ref TgvFocus_Model
 * describes the focus models in further detail.
 *
 * Typical usage is:
 * @code
 * PicselConfig_setInt(picselContext,
 *                     PicselConfig_focusedBorderColour,
 *                     PicselConfig_rgbaFromComponents(0xff, 0x00, 0xff, 0xa0));
 * PicselConfig_setInt(picselContext,
 *                     PicselConfig_focusedFillColour,
 *                     PicselConfig_rgbaFromComponents(0xff, 0xff, 0x00, 0x40));
 * @endcode
 * The first call to PicselConfig_setInt() will set the border to purple
 * with some transparency applied (replace the alpha channel value @c 0xa0
 * with @c 0xff for non-transparent). The second call to
 * PicselConfig_setInt() will set the fill to light yellow. The fill alpha
 * should not be more than @c 0x80. Setting it any higher will obscure the
 * element. If the alpha is less than @c 0x20, then the colour might not be
 * seen.
 *
 * @product File Viewer Development Kit and Browser products.
 * @see     @ref PicselConfig_focusedFillColour,
 *          @ref PicselConfig_activeBorderColour,
 *          @ref PicselConfig_selectionBorderWidth.
 */
#define PicselConfig_focusedBorderColour "Picsel_FocusBorderColour"


/**
 * Configuration parameter for setting the fill colour used to decorate a
 * focused element.
 *
 * This configuration parameter would typically be set in
 * AlienConfig_ready(). If it is set at any other time, then it will only
 * take effect when the next document is loaded.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 * The colour is specified as type @ref Picsel_ColourRgba,
 * PicselConfig_rgbaFromComponents() can be used to set this value.
 *
 * A focused element is highlighted in the weak focus mode,
 * @ref TgvFocus_Model describes the focus models in further detail.
 *
 * See @ref PicselConfig_focusedBorderColour for an example.
 *
 * @product File Viewer Development Kit and Browser products.
 * @see     @ref PicselConfig_focusedBorderColour,
 *          @ref PicselConfig_activeFillColour.
 */
#define PicselConfig_focusedFillColour   "Picsel_FocusFillColour"


/**
 * Configuration parameter for setting the border colour used to decorate an
 * active element.
 *
 * An active element is a box around the highlighted link or interactive
 * control, and the box has a fill colour and a border line colour. The fill
 * is usually almost transparent so that users can still see the content.
 * An active element is highlighted in the strong focus mode,
 * @ref TgvFocus_Model describes the focus models in further detail.
 *
 * This configuration parameter would typically be set in
 * AlienConfig_ready(). If it is set at any other time, then it will only
 * take effect when the next document is loaded.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 * The colour is specified as type @ref Picsel_ColourRgba,
 * PicselConfig_rgbaFromComponents() can be used to set this value.
 *
 * Typical usage is:
 * @code
 * PicselConfig_setInt(picselContext,
 *                     PicselConfig_activeBorderColour,
 *                     PicselConfig_rgbaFromComponents(0xff, 0x00, 0xff, 0xa0));
 * PicselConfig_setInt(picselContext,
 *                     PicselConfig_activeFillColour,
 *                     PicselConfig_rgbaFromComponents(0xff, 0xff, 0x00, 0x40));
 * @endcode
 * The first call to PicselConfig_setInt() will set the border to purple
 * with some transparency applied (replace the alpha channel value @c 0xa0
 * with @c 0xff for non-transparent). The second call to
 * PicselConfig_setInt() will set the fill to light yellow. The fill alpha
 * should not be more than @c 0x80. Setting it any higher will obscure the
 * element. If the alpha is less than @c 0x20, then the colour might not be
 * seen.
 *
 * @product File Viewer Development Kit and Browser products.
 * @see     @ref PicselConfig_activeFillColour,
 *          @ref PicselConfig_focusedBorderColour.
 */
#define PicselConfig_activeBorderColour  "Picsel_ActiveBorderColour"


/**
 * Configuration parameter for setting the fill colour used to decorate an
 * active element.
 *
 * An active element is a box around the highlighted link or interactive
 * control, and the box has a fill colour and a border line colour. The fill
 * is usually almost transparent so that users can still see the content.
 * An active element is highlighted in the strong focus mode,
 * @ref TgvFocus_Model describes the focus models in further detail.
 *
 * This configuration parameter would typically be set in
 * AlienConfig_ready(). If it is set at any other time, then it will only
 * take effect when the next document is loaded.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 * The colour is specified as type @ref Picsel_ColourRgba,
 * PicselConfig_rgbaFromComponents() can be used to set this value.
 *
 * See @ref PicselConfig_activeBorderColour for an example.
 *
 * @product File Viewer Development Kit and the Browser Development Kit.
 * @see     @ref PicselConfig_activeBorderColour,
 *          @ref PicselConfig_focusedFillColour.
 */
#define PicselConfig_activeFillColour    "Picsel_ActiveFillColour"


/**
 * The colour used as the background highlight for an editable element that
 * has been selected for editing.
 *
 * This is the standard highlight colour used to indicate general element
 * selection for editing.
 *
 * This configuration parameter would typically be set in
 * AlienConfig_ready(). If it is set at any other time, then it will only
 * take effect when the next document is loaded.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 * The colour is specified as type @ref Picsel_ColourRgba,
 * PicselConfig_rgbaFromComponents() can be used to set this value.
 *
 * The colour is only updated when a document is loaded, changing the colour
 * will not affect the current document. AlienConfig_ready() would be an
 * appropriate place to set this parameter.
 *
 * @product File Viewer Development Kit and Browser products.
 * @see     @ref TgvEditing, @ref PicselConfig_selectionBorderColour,
 *          @ref PicselConfig_selectionBorderWidth.
 */
#define PicselConfig_selectionFillColour      "Picsel_SelectionFillColour"


/**
 * DEPRECATED: The colour used as the background highlight for an editable
 * field element which has been selected for editing.
 *
 * Currently field elements, e.g. references and formulas, are not editable.
 * Elements that are not editable will not be selected, so currently this
 * highlight cannot be seen.
 *
 * This configuration parameter would typically be set in
 * AlienConfig_ready(). If it is set at any other time, then it will only
 * take effect when the next document is loaded.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 * The colour is specified as type @ref Picsel_ColourRgba,
 * PicselConfig_rgbaFromComponents() can be used to set this value.
 *
 * Associated configuration parameters are:
 *  - @ref PicselConfig_selectionFillColour
 *  - @ref PicselConfig_selectionBorderColour
 *  - @ref PicselConfig_selectionBorderWidth
 *
 * @product Not used.
 * @see     @ref TgvEditing.
 */
#define PicselConfig_selectionFieldFillColour "Picsel_SelectionFieldFillColour"


/**
 * The colour of the border around the highlight of an element that has been
 * selected for editing.
 *
 * Typically selected elements are only given borders in spreadsheet style
 * documents such as Excel.
 *
 * This configuration parameter would typically be set in
 * AlienConfig_ready(). If it is set at any other time, then it will only
 * take effect when the next document is loaded.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 * The colour is specified as type @ref Picsel_ColourRgba,
 * PicselConfig_rgbaFromComponents() can be used to set this value.
 *
 * @product TGV products with interactive navigation and link
 * selection. This includes web browsing and link navigation within
 * conventional documents.
 *
 * @see     @ref TgvEditing, @ref PicselConfig_selectionFillColour,
 *          @ref PicselConfig_selectionBorderWidth
 */
#define PicselConfig_selectionBorderColour    "Picsel_SelectionBorderColour"


/**
 * The width of the border around the highlight of an element that has been
 * selected for editing.
 *
 * The border width is measured in inches using type @ref Picsel_Fixed1616.
 *
 * Typically a border around selected items are only used for spreadsheet
 * style document such as Excel.
 *
 * This configuration parameter would typically be set in
 * AlienConfig_ready(). If it is set at any other time, then it will only
 * take effect when the next document is loaded.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * @product TGV products with interactive navigation and link
 * selection. This includes web browsing and link navigation within
 * conventional documents.
 * @see @ref TgvEditing, @ref PicselConfig_selectionFillColour,
 *      @ref PicselConfig_selectionBorderColour
 */
#define PicselConfig_selectionBorderWidth     "Picsel_SelectionBorderWidth"


/**
 * The colour of the insertion caret that has been placed for editing.
 *
 * This configuration parameter would typically be set in
 * AlienConfig_ready(). If it is set at any other time, then it will only
 * take effect when the next document is loaded.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 * The colour is specified as type @ref Picsel_ColourRgba,
 * PicselConfig_rgbaFromComponents() can be used to set this value.
 *
 * If no value is set, the insertion caret defaults to be the same colour
 * as the selection border.
 *
 * Associated configuration parameters are:
 *  - @ref PicselConfig_insertionCaretWidthDivisor
 *
 * @product FVDK with editing
 * @see     @ref TgvEditing.
 */
#define PicselConfig_insertionCaretColour    "Picsel_InsertionCaretColour"


/**
 * The fraction of a space used to determine the width of an insertion caret
 * that has been placed for editing. For example, a value of 2 means
 * the insertion caret is 50% of the width of a space character in the font
 * being used at that point.
 *
 * This configuration parameter would typically be set in
 * AlienConfig_ready(). If it is set at any other time, then it will only
 * take effect when the next document is loaded.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 * If no value is set, a default of 5 is used, which gives an insertion caret
 * width of 20% of a space character.
 *
 * Associated configuration parameters are:
 *  - @ref PicselConfig_insertionCaretColour
 *
 * @product Any Picsel product with editing support such as the File Viewer
 *          Development Kit.
 * @see     @ref TgvEditing.
 */
#define PicselConfig_insertionCaretWidthDivisor     "Picsel_InsertionCaretWidthDivisor"


/**
 * Colour used to pad the margins when the scaled thumbnail bitmap
 * has a different aspect ratio to the requested bitmap size.
 *
 * The colour is only updated when a document is loaded, changing the colour
 * will not affect the current document. AlienConfig_ready() would be an
 * appropriate place to set this parameter.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 * The colour is specified as type @ref Picsel_ColourRgba,
 * PicselConfig_rgbaFromComponents() can be used to set this value.
 *
 * The Alpha value is ignored.
 *
 * Default value is @c 0x000000ff, which is black.
 *
 * @product @ref TgvFileViewer "File Viewer",
 *          @ref ThumbnailGenerator "Thumbnail Generator"
 * @see     @ref PicselConfig_thumbnailsMax,
 *          @ref PicselConfig_thumbnailsMaxScaled,
 *          @ref PicselConfig_thumbnailWidth,
 *          @ref PicselConfig_thumbnailHeight.
 *          @ref PicselConfig_showBlankThumbnail,
 */
#define PicselConfig_thumbnailFillColour   "Picsel_ThumbnailFillColour"


/**
 * The colour used as the background highlight for the current search result.
 *
 * This configuration parameter would typically be set in
 * AlienConfig_ready(). If it is set at any other time, then it will only
 * take effect when the next document is loaded.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 * The colour is specified as type @ref Picsel_ColourRgba,
 * PicselConfig_rgbaFromComponents() can be used to set this value.
 *
 * Defaults to pale blue.
 *
 * @product Any Picsel product with search support such as the File Viewer
 *          Development Kit.
 * @see     @ref PicselConfig_searchBorderColour,
 *          @ref PicselConfig_searchBorderWidth,
 */
#define PicselConfig_searchFillColour       "Picsel_SearchFillColour"


/**
 * The colour used as the border for the current search result.
 *
 * This configuration parameter would typically be set in
 * AlienConfig_ready(). If it is set at any other time, then it will only
 * take effect when the next document is loaded.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 * The colour is specified as type @ref Picsel_ColourRgba,
 * PicselConfig_rgbaFromComponents() can be used to set this value.
 *
 * @product Any Picsel product with search support such as the File Viewer
 *          Development Kit.
 * @see     @ref PicselConfig_searchFillColour,
 *          @ref PicselConfig_searchBorderWidth,
 */
#define PicselConfig_searchBorderColour      "Picsel_SearchBorderColour"


/**
 * The width of the border around the highlight of the current search result.
 *
 * The border width is measured in inches using type @ref Picsel_Fixed1616.
 *
 * This configuration parameter would typically be set in
 * AlienConfig_ready(). If it is set at any other time, then it will only
 * take effect when the next document is loaded.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 * The colour is specified as type @ref Picsel_ColourRgba,
 * PicselConfig_rgbaFromComponents() can be used to set this value.
 *
 * Defaults to 0
 *
 * @product Any Picsel product with editing support such as the File Viewer
 *          Development Kit.
 * @see     @ref PicselConfig_searchFillColour,
 *          @ref PicselConfig_searchBorderColour,
 */
#define PicselConfig_searchBorderWidth       "Picsel_SearchBorderWidth"


/**
 * The colour used as the background highlight for non-focused search results
 * obtained using the PicselSearch_startList function.
 *
 * The focused result (selected by by a call to PicselSearch_snapToResultId())
 * will be highlighted using the PicselConfig_searchFillColour,
 * PicselConfig_searchBorderColour and PicselConfig_searchBorderWidth values.
 * All other results in the list will be highlighted using the
 * PicselConfig_searchListFillColour, PicselConfig_searchListBorderColour and
 * PicselConfig_searchListBorderWidth values.
 *
 * This configuration parameter would typically be set in
 * AlienConfig_ready(). If it is set at any other time, then it will only
 * take effect when the next document is loaded.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 * The colour is specified as type @ref Picsel_ColourRgba,
 * PicselConfig_rgbaFromComponents() can be used to set this value.
 *
 * Defaults to grey
 *
 * @product Any Picsel product with search support such as the File Viewer
 *          Development Kit.
 * @see     @ref PicselConfig_searchListBorderColour,
 *          @ref PicselConfig_searchListBorderWidth,
 */
#define PicselConfig_searchListFillColour    "Picsel_SearchListFillColour"


/**
 * The colour used as the border for non-focused search results
 * obtained using the PicselSearch_startList function.
 *
 * This configuration parameter would typically be set in
 * AlienConfig_ready(). If it is set at any other time, then it will only
 * take effect when the next document is loaded.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 * The colour is specified as type @ref Picsel_ColourRgba,
 * PicselConfig_rgbaFromComponents() can be used to set this value.
 *
 * @product Any Picsel product with search support such as the File Viewer
 *          Development Kit.
 * @see     @ref PicselConfig_searchListFillColour,
 *          @ref PicselConfig_searchListBorderWidth,
 */
#define PicselConfig_searchListBorderColour  "Picsel_SearchListBorderColour"


/**
 * The width of the border around the highlight for non-focused search
 * results obtained using the PicselSearch_startList function.
 *
 * The border width is measured in inches using type @ref Picsel_Fixed1616.
 *
 * This configuration parameter would typically be set in
 * AlienConfig_ready(). If it is set at any other time, then it will only
 * take effect when the next document is loaded.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 * The colour is specified as type @ref Picsel_ColourRgba,
 * PicselConfig_rgbaFromComponents() can be used to set this value.
 *
 * Defaults to 0
 *
 * @product Any Picsel product with editing support such as the File Viewer
 *          Development Kit.
 * @see     @ref PicselConfig_searchListFillColour,
 *          @ref PicselConfig_searchListBorderColour,
 */
#define PicselConfig_searchListBorderWidth   "Picsel_SearchListBorderWidth"


/**
 * Sets which document rotations are available.
 *
 * @ref PicselCmdRotate is used to step through the available document
 * rotations, this parameter will configure which rotations are available for
 * @ref PicselCmdRotate to step through. The unrotated document is always
 * available. The valid rotations are defined in the enum
 * @ref PicselConfig_Rotations. The required value for this configuration
 * parameter is obtained by applying a bitwise OR operation to each of the
 * required rotations listed in the enum @ref PicselConfig_Rotations.
 *
 * e.g. to set the available rotations as 90 degrees clockwise and 180
 * degrees, OR the desired settings:
 * @code
 * (PicselConfig_Rotation_Cw90 | PicselConfig_Rotation_180)
 * @endcode
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 * This configuration parameter would typically be set in AlienConfig_ready().
 *
 * If this parameter is not set, the default is unrotated and 90 clockwise
 * degrees. This is exported as PicselConfig_DefaultSupportedRotations.
 * For products that don't support different document rotations this
 * parameter will be set to unrotated.
 *
 * @product File Viewer Development Kit and Browser products.
 */
#define PicselConfig_rotatedScreens         "Picsel_rotatedScreens"


/**
 * A list of the document rotation positions.
 *
 * Used by @ref PicselConfig_rotatedScreens.
 */
typedef enum PicselConfig_Rotations
{
    /**
     * Default document orientation.
     */
    PicselConfig_Rotation_None = (1<<16),

    /**
     * The document's orientation would be rotated 90 degrees clockwise from
     * its default position.
     */
    PicselConfig_Rotation_Cw90 = PicselConfig_Rotation_None + 1,

    /**
     * The document's orientation would be rotated 180 degrees from its
     * default position.
     */
    PicselConfig_Rotation_180  = PicselConfig_Rotation_None + 2,

    /**
     * The document's orientation would be rotated 90 degrees anti-clockwise
     * from its default position.
     */
    PicselConfig_Rotation_Ac90 = PicselConfig_Rotation_None + 4
}
PicselConfig_Rotations;

enum
{
    PicselConfig_DefaultSupportedRotations = PicselConfig_Rotation_Cw90
};

/**
 * Specifies that the screen should be created maximised.
 *
 * Otherwise the screen is created using the margins as specified in
 * AlienScreen_getConfiguration().
 *
 * Not all Picsel products support screen resizing. If your library contains
 * more than one application, check the effect of this flag on each
 * application separately. If any applications do not display
 * correctly, modify AlienConfig_ready() to avoid setting this option for
 * that application.
 *
 * The default value is 0 (off); set to 1 (on) to start maximised.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * This configuration parameter can only be set in AlienConfig_ready().
 *
 * @product File Viewer and Browser Development Kit.
 */
#define PicselConfig_startFullscreen       "Picsel_startFullscreen"


/**
 * The default document rotation.
 *
 * This allows the alien application to set which of the document rotations
 * defined with @ref PicselConfig_rotatedScreens should be used as the
 * default.
 * If not set, defaults to  @ref PicselConfig_Rotation_None, also defaults to
 * this if the requested rotation is not allowed by
 * @ref PicselConfig_rotatedScreens.
 *
 * Only values defined in @ref PicselConfig_Rotations should be used.
 *
 * This configuration parameter should be set using PicselConfig_setInt()
 *
 * This configuration parameter can only be set in AlienConfig_ready().
 *
 * @pre     This must only be set after @ref PicselConfig_rotatedScreens has
 *          been configured.
 * @product SMS Viewer, File Viewer Development Kit and Browser products.
 */
#define PicselConfig_defaultScreen         "Picsel_defaultScreen"


/**
 * @deprecated
 * The maximum size of the download cache in bytes.
 *
 * The download cache is used for any data that is downloaded.  Any
 * downloaded files that are no longer being used will be moved into the
 * download cache.
 *
 * This configuration parameter defines the how large the download cache will
 * be allowed to get. If the download cache exceeds this (or
 * @ref PicselConfig_cacheMaxEntries) then cache entries will be purged
 * according to an algorithm dependent upon the Picsel product used.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 * This configuration parameter would typically be set in AlienConfig_ready().
 *
 * 0 indicates the cache is disabled. The default value is 0.
 *
 * @ref PicselConfig_cacheMaxEntries is another configuration parameter for
 * defining the download cache behaviour.
 *
 * @product Not used.
 * @see     @ref PicselConfig_cacheMaxEntries.
 */
#define PicselConfig_cacheMaxSize          "Picsel_CacheMaxSize"


/**
 * @deprecated
 * The maximum number of entries in the download cache.
 *
 * The download cache is used for any data that is downloaded.  Any
 * downloaded files that are no longer being used will be moved into the
 * download cache.
 *
 * This configuration parameter will determine the maximum number of files
 * that will be stored in the download cache. If the download cache exceeds
 * this (or @ref PicselConfig_cacheMaxSize) then cache entries will be
 * purged according to an algorithm dependent upon the Picsel product used.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 * This configuration parameter would typically be set in AlienConfig_ready().
 *
 * 0 indicates there is no limit on entries, and the size will be determined
 * by @ref PicselConfig_cacheMaxSize only.  The default value is 0.
 *
 * @ref PicselConfig_cacheMaxSize is another configuration parameter for
 * defining the download cache behaviour.
 *
 * @product Not used.
 * @see     @ref PicselConfig_cacheMaxSize.
 */
#define PicselConfig_cacheMaxEntries       "Picsel_CacheMaxEntries"

/**
 * The amount of memory (in bytes) Picsel should use for image caching.
 *
 * An image cache is an optimisation that can improve the performance of
 * graphically rich applications.  The typical size ranges from 0 to 2Mb.
 * The default varies between products but is normally 0.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * This configuration parameter should only be set in AlienConfig_ready().
 * If it is set at any other time, then the new value will not take effect.
 *
 * @note In some builds, cache management is automatic, and
 *       therefore this value may have no effect.
 *
 * @product Products that download data, SOL, Browser Development kit
 */
#define PicselConfig_imageCache            "Picsel_Image_Cache_Bytes"

/**
 * The maximum size of an uncompressed image (in bytes).
 *
 * Images that would consume more memory than this during decoding will not
 * be loaded.
 *
 * As an example, a 2000 * 2000 pixel image to be rendered using
 * @ref PicselScreenFormat_b5g6r5, will require 8MB of memory for decoding.
 * Set @ref PicselConfig_maxImageSize to @c 8000000 to prevent larger images
 * consuming significant blocks of memory.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 * If it is set at any other time, then it will only take effect when the
 * next document is loaded.
 *
 * @product Any document viewing product.
 */
#define PicselConfig_maxImageSize          "Picsel_Image_MaxSize"

/**
 * Allow application to automatically select the character encoding for
 * the documents.
 *
 * This is enabled by default.  If this is disabled, then any information
 * provided by the document about the character encoding will be ignored.
 * This will mean that some web sites will not be displayed as the author
 * intended them.
 *
 * The default value is 1 (on); set to 0 (off) to disable character set auto
 * detection.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 * If it is set at any other time, then it will only take effect when the
 * next document is loaded.
 *
 * @product Any document viewing product.
 * @see     @ref PicselConfig_defaultCharSet, @ref PicselConfig_CharSet.
 */
#define PicselConfig_enableAutoDetect        "EnableAutoDetect"

/**
 * Specify which character encoding Picsel should use in cases where the
 * document contains no indication.
 *
 * This can be set in 2 ways:
 *  - By calling PicselConfig_setInt(). See @ref PicselConfig_CharSet for a
 *    list of encoding types supported.
 *  - By calling PicselConfig_setString(). @ref PicselConfig_CharSet lists
 *    the string names for each of the supported encoding types.
 *
 * This configuration parameter should only be set in AlienConfig_ready().
 *
 * @product Any Picsel product with document viewing or browsing
 *          capabilities.
 * @see     @ref PicselConfig_enableAutoDetect.
 */
#define PicselConfig_defaultCharSet          "DefaultCharacterSet"

/**
 * Specify the language edition of the installed operating system.
 *
 * The language selected should conform to a language or font that has been
 * registered using the functions defined in @ref TgvLanguageInit.
 *
 * Examples of language codes:-
 *  - "en-gb"            - English (United Kingdom)
 *  - "en-us"            - English (United States)
 *  - "fr"               - French (Standard)
 *  - "ja"               - Japanese
 *
 * This configuration parameter should be set using PicselConfig_setString().
 *
 * This configuration parameter should only be set in AlienConfig_ready().
 *
 * @product Used by browser products.
 * @see     @ref TgvLanguageInit.
 */
#define PicselConfig_installedLanguage       "InstalledLanguage"

/**
 * Excel format for locale-dependent short dates.
 *
 * Cells in Excel can be formatted with a short date which is displayed
 * based on the region of the user's system. Set this to an Excel
 * format string to indicate how to display such cells in Picsel.
 *
 * Defaults to: @c "dd/mm/yyyy".
 *
 * This configuration parameter should be set using PicselConfig_setString().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 * If it is set at any other time, then it will only take effect when the
 * next document is loaded.
 *
 * @product FVDK, SOL, most document viewing products.
 */
#define PicselConfig_excelLocaleDateShort   "Picsel_excelLocaleDateShort"

/**
 * Excel format for locale-dependent long dates.
 *
 * Cells in Excel can be formatted with a long date which is displayed
 * based on the region of the user's system. Set this to an Excel
 * format string to indicate how to display such cells in Picsel.
 *
 * Defaults to: @c "dd mmmm yyyy".
 *
 * This configuration parameter should be set using PicselConfig_setString().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 * If it is set at any other time, then it will only take effect when the
 * next document is loaded.
 *
 * @product FVDK, SOL, most document viewing products.
 */
#define PicselConfig_excelLocaleDateLong    "Picsel_excelLocaleDateLong"

/**
 * Excel format for locale-dependent date and time.
 *
 * Cells in Excel can be formatted with a date/time which is displayed
 * based on the region of the user's system. Set this to an Excel
 * format string to indicate how to display such cells in Picsel.
 *
 * Defaults to: @c "dd/mm/yyyy hh:mm".
 *
 * This configuration parameter should be set using PicselConfig_setString().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 * If it is set at any other time, then it will only take effect when the
 * next document is loaded.
 *
 * @product FVDK, SOL, most document viewing products.
 */
#define PicselConfig_excelLocaleDateAndTime "Picsel_excelLocaleDateAndTime"

/**
 * Excel format for locale-dependent time.
 *
 * Cells in Excel can be formatted with a time which is displayed
 * according to the region of the user's system. Set this to an Excel
 * format string to indicate how to display such cells in Picsel.
 *
 * Defaults to: @c "hh:mm:ss".
 *
 * This configuration parameter should be set using PicselConfig_setString().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 * If it is set at any other time, then it will only take effect when the
 * next document is loaded.
 *
 * @product FVDK, SOL, most document viewing products.
 */
#define PicselConfig_excelLocaleTime        "Picsel_excelLocaleTime"

/**
 * Sets the delay before a held key autorepeats.
 *
 * This configuration parameter will set how long a key will need to be held
 * before the key autorepeat starts. Configuration parameter
 * @ref PicselConfig_keyAutorepeatRate is an associated configuration
 * parameter controlling key autorepeat behaviour.
 *
 * The value is in milliseconds, the default is 250.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 *
 * @pre     @p softRepeats will need to be enabled and @p softRelease
 *          disabled, which can be done by PicselApp_setKeyBehaviour().
 * @product FVDK
 */
#define PicselConfig_keyAutorepeatDelay        "Picsel_keyAutorepeatDelay"

/**
 * Sets the key autorepeat rate.
 *
 * This configuration parameter will set the repeat rate of the keys once the
 * key autorepeat starts. Configuration parameter
 * @ref PicselConfig_keyAutorepeatDelay is an associated configuration
 * parameter controlling key autorepeat behaviour.
 *
 * The value is in milliseconds, the default is 40
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 *
 * @pre     @p softRepeats will need to be enabled and @p softRelease
 *          disabled, which can be done by PicselApp_setKeyBehaviour().
 * @product FVDK
 */
#define PicselConfig_keyAutorepeatRate         "Picsel_keyAutorepeatRate"

/**
 * Sets the path at which to store persistent settings.
 *
 * The path given by this function should point to a writeable directory
 * where the Picsel Library may store persistent settings files such as
 * cookies, persistent cache, history, bookmarks.
 *
 * If this is not specified, these settings will not be stored.
 *
 * This configuration parameter should be set using PicselConfig_setString().
 *
 * This configuration parameter should only be set in AlienConfig_ready().
 *
 * @product SOL, Browser
 */
#define PicselConfig_settingsPath              "Picsel_settingsPath"

/**
 * Sets the path at which resource files should be looked for.
 *
 * The path given by this setting should point to a readable directory
 * where resources, such as MCF files, have been stored.
 *
 * If a valid resource path is not provided, then the Picsel app
 * may try to read files from the root directory of the file system.
 *
 * This configuration parameter should be set using PicselConfig_setString().
 *
 * This configuration parameter should only be set in AlienConfig_ready().
 *
 * @product SOL,UE2 Based Products.
 */
#define PicselConfig_resourcePath              "Picsel_resourcePath"

/**
 * Set the minimum width of an image to download.
 *
 * When flow mode @ref FlowMode_FitScreenWidth is enabled this configuration
 * parameter, along with
 * @ref PicselConfig_FlowMode_FitScreenWidth_minDownloadImageHeight, will
 * prevent images below the configured size form being downloaded.
 *
 * This allows for notably faster page download if HTTP pipelining is not
 * supported but can result in poor display of page contents. Set this values
 * if performance over display quality is desired.
 *
 * The values are in pixels, the defaults are 0.
 * Some previous Picsel releases have had this value set to 35.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 * If it is set at any other time, then it will only take effect when the
 * next document is loaded.
 *
 * @product Used by Picsel Browser products.
 */
#define PicselConfig_FlowMode_FitScreenWidth_minDownloadImageWidth    "reflow.reject.width"

/**
 * Set the minimum height of an image to download.
 *
 * When flow mode @ref FlowMode_FitScreenWidth is enabled this configuration
 * parameter, along with
 * @ref PicselConfig_FlowMode_FitScreenWidth_minDownloadImageHeight, will
 * prevent images below the configured size form being downloaded.
 *
 * This allows for notably faster page download if HTTP pipelining is not
 * supported but can result in poor display of page contents. Set this values
 * if performance over display quality is desired.
 *
 * The values are in pixels, the defaults are 0.
 * Some previous Picsel releases have had this value set to 35.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 * If it is set at any other time, then it will only take effect when the
 * next document is loaded.
 *
 * @product Used by Picsel Browser products.
 */
#define PicselConfig_FlowMode_FitScreenWidth_minDownloadImageHeight   "reflow.reject.height"

/**
 * Specifies the flow mode that will be used to load the next document.
 *
 * This configuration parameter will set the initial flow mode that will be
 * used when the next document is loaded, or if configured in
 * AlienConfig_ready(), this specifies the flow mode that will be used at
 * startup. Please see @ref PicselFlowMode for a list of possible
 * parameters.
 *
 * The default flowmode at startup will depend on the product.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 * If it is set at any other time, then it will only take effect when the
 * next document is loaded.
 *
 * @product Any Picsel product with document viewing or browsing
 *          capabilities.
 * @see     @ref TgvFlowMode.
 */
#define PicselConfig_flowMode "Picsel_flowMode"

/**
 * Sets the zoom behaviour for changing the flow mode.
 *
 * If this is set to 1 then the zoom level will not change when moving to a
 * new flow mode. If this is set to 0, then the zoom level will be reset to
 * the default level when moving to a new flow mode, i.e. if the document has
 * been either zoomed in or out then when the flow mode is changed the zoom
 * level will return to the original level the document was opened at. The
 * flow mode can be changed with a call to PicselFlowMode_set().
 *
 * The value can be 0 or 1; the default is 1.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 * If it is set at any other time, then it will only take effect when the
 * next document is loaded.
 *
 * @product Any Picsel product with document viewing or browsing
 *          capabilities.
 * @see     @ref TgvFlowMode
 */
#define PicselConfig_flowModeKeepZoom "Picsel_flowModeKeepZoom"


/**
 * Sets the image to be displayed in place of a missing image.
 *
 * This image should be defined using a well formed URL e.g.
 * @c "file://localhost/image.png".  The image should be of any of the
 * available image formats as configured in AlienConfig_initialiseAgents().
 *
 * If no image is specified then a default image will be used.
 * It is advised to set @ref PicselConfig_placeholderWidthHeight if the image
 * is over 20 pixels high/wide.
 *
 * This configuration parameter should be set using PicselConfig_setString().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 * If it is set at any other time, then it will only take effect when the
 * next document is loaded.
 *
 * @product File Viewer and Browser Development Kits.
 * @see     @ref PicselConfig_placeholderWidthHeight
 */
#define PicselConfig_missingImagePlaceholder \
                                        "Picsel_missingImagePlaceholder"

/**
 * Sets the image to be displayed in place of a corrupt image.
 *
 * This place holder image will be used in place of a document's image if
 * there is an error parsing or displaying an image. For example this may be
 * due to a corrupt image file, or an out of memory event.
 *
 * This image should be defined using a well formed URL e.g.
 * @c "file://localhost/image.png".  The image should be of any of the
 * available image formats as configured in AlienConfig_initialiseAgents().
 *
 * If no image is specified then a default image will be used.
 * It is advised to set @ref PicselConfig_placeholderWidthHeight if the image
 * is over 20 pixels high/wide.
 *
 * This configuration parameter should be set using PicselConfig_setString().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 * If it is set at any other time, then it will only take effect when the
 * next document is loaded.
 *
 * @product File Viewer and Browser Development Kits.
 * @see     @ref PicselConfig_placeholderWidthHeight
 */
#define PicselConfig_corruptImagePlaceholder \
                                        "Picsel_corruptImagePlaceholder"

/**
 * Sets the image to be displayed in place of an unsupported image.
 *
 * This image should be defined using a well formed URL e.g.
 * @c "file://localhost/image.png".  The image should be of any of the
 * available image formats as configured in AlienConfig_initialiseAgents().
 *
 * If no image is specified then a default image will be used.
 * It is advised to set @ref PicselConfig_placeholderWidthHeight if the image
 * is over 20 pixels high/wide.
 *
 * This configuration parameter should be set using PicselConfig_setString().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 * If it is set at any other time, then it will only take effect when the
 * next document is loaded.
 *
 * @product File Viewer and Browser Development Kits.
 * @see     @ref PicselConfig_placeholderWidthHeight
 */
#define PicselConfig_unsupportedImagePlaceholder \
                                        "Picsel_unsupportedImagePlaceholder"

/**
 * Sets the image to be displayed in place of a not yet loaded image.
 *
 * This place holder image will be used in place of a document's image if the
 * image to displayed is not yet completely downloaded, and so cannot yet be
 * displayed within a document.
 *
 * This image should be defined using a well formed URL e.g.
 * @c "file://localhost/image.png".  The image should be of any of the
 * available image formats as configured in AlienConfig_initialiseAgents().
 *
 * If no image is specified then a default image will be used.
 * It is advised to set @ref PicselConfig_placeholderWidthHeight if the image
 * is over 20 pixels high/wide.
 *
 * This configuration parameter should be set using PicselConfig_setString().
 *
 * This configuration parameter would typically be set in
 * AlienConfig_ready(). If it is set at any other time, then it will only
 * take effect when the next document is loaded.
 *
 * @product File Viewer and Browser Development Kits.
 * @see     @ref PicselConfig_placeholderWidthHeight
 */
#define PicselConfig_unloadedImagePlaceholder \
                                        "Picsel_unloadedImagePlaceholder"

/**
 * If a placeholder image is set, then set this to the width or height of the
 * icon (whichever is largest).
 *
 * This ensures that the placeholder image will be displayed when no size
 * information is given for the image in the content.
 *
 * Default value is 20 pixels
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 * If it is set at any other time, then it will only take effect when the
 * next document is loaded.
 *
 * @product Used by Picsel Browser products.
 * @see     @ref PicselConfig_unloadedImagePlaceholder,
 *          @ref PicselConfig_unsupportedImagePlaceholder,
 *          @ref PicselConfig_corruptImagePlaceholder,
 *          @ref PicselConfig_missingImagePlaceholder.
 */
#define PicselConfig_placeholderWidthHeight \
                                        "Picsel_placeholderWidthHeight"


/**
 * Enables/Disables key repeats for Page Up/Down keys.
 *
 *  - 1 => Enable Page Up/Down key repeats
 *  - 0 => Disable Page Up/Down key repeats [default]
 *
 * If not set, defaults to Disabled
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 *
 * @product File Viewer and Browser Development Kits.
 * @see     @ref PicselConfig_keyAutorepeatRate,
 *          @ref PicselConfig_keyAutorepeatDelay.
 */
#define PicselConfig_keyPageUpDownRepeat   "Picsel_keyPageUpDownRepeat"

/**
 * Lock/unlock secret bookmarks.
 *
 * This property indicates whether bookmarks that have been flagged as being
 * "secret" will be visible.
 * The term bookmark is used in the traditional sense with respect to
 * browsers, which is to provide a method of storing a link to a URL.
 *  - 1 => Locked i.e. don't allow access to bookmarks flagged as "secret".
 *  - 0 => Unlocked i.e. allow access to bookmarks flagged as "secret".
 *         [default]
 *
 * If not set, defaults to Unlocked
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * This configuration parameter would typically be set in AlienConfig_ready(),
 * but it may be called at any other time during runtime after
 * AlienConfig_ready() has been called.
 *
 * @product Various Picsel products
 */
#define PicselConfig_secretBookmarkLock   "Picsel_secretBookmarkLock"

/**
 * Set the maximum number of bookmark entries.
 *
 * Sets the maximum number of bookmark entries that we will be stored. The
 * number of bookmarks can also be limited by
 * @ref PicselConfig_historyBmMaxFileLen.
 * The term bookmark is used in the traditional sense with respect to
 * browsers, which is to provide a method of storing a link to a URL.
 *
 * The supported range of values is 0..INT_MAX.
 *
 * This configuration parameter should be set using PicselConfig_setInt()
 * from within AlienConfig_ready().
 *
 * @product Used by Picsel Browser products.
 * @see     AlienHistory_move()
 */
#define PicselConfig_bookmarkMax  "Picsel_bookmarkMax"

/**
 * Set the maximum number of history entries.
 *
 * Sets the maximum number of history entries that will be stored. Each URL
 * visited will be added to the history list. The number of history items can
 * also be limited by @ref PicselConfig_historyBmMaxFileLen.
 *
 * The supported range of values is 0..INT_MAX.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * @product Used by Picsel Browser products.
 * @see     AlienHistory_move()
 */
#define PicselConfig_historyMax  "Picsel_historyMax"

/**
 * Set the maximum number of bookmark categories.
 *
 * Sets the maximum number of bookmark categories that can be created.
 *
 * The supported range of values is 0..INT_MAX.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * @product Various Picsel products
 */
#define PicselConfig_bookmarkMaxCategories   "Picsel_bookmarkMaxCategories"

/**
 * Set the maximum size for history and bookmark files in bytes.
 *
 * Like @ref PicselConfig_bookmarkMax and @ref PicselConfig_historyMax, this
 * will set a limit on how many bookmarks and history items will be stored,
 * by limiting the size of the file that will store them.
 *
 * The supported range of values is 0..UINT_MAX. If the file size is to be
 * unbounded then it should be assigned
 * @ref PicselConfig_HistoryBmMaxFileLenUnlimited.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * @product Used by Picsel Browser products.
 * @see      AlienHistory_move(), @ref PicselConfig_settingsPath.
 */
#define PicselConfig_historyBmMaxFileLen  "Picsel_historyBmMaxFileLen"

/**
 * Value to set for @ref PicselConfig_historyBmMaxFileLen if there is no
 * maximum limit for the history/bookmark file sizes.
 */
enum
{
    PicselConfig_HistoryBmMaxFileLenUnlimited = 0
};

/**
 * Set the default homepage.
 *
 * This will be used as the homepage if @ref PicselConfig_homepage is set to
 * NULL.  This is intended to store a fall back homepage for when the user
 * hasn't yet set a homepage, or if the user's preferences are cleared.
 *
 * This configuration parameter should be set using PicselConfig_setString().
 *
 * @product Various Picsel products
 */
#define PicselConfig_defaultHomepage   "Picsel_defaultHomepage"

/**
 * Set the current homepage.
 *
 * This is intended to store the user's preference for their current homepage.
 *
 * This configuration parameter should be set using PicselConfig_setString().
 *
 * @product Various Picsel products
 * @see     @ref PicselConfig_defaultHomepage
 */
#define PicselConfig_homepage           "Picsel_homepage"

/**
 * Value that indicates that the number of thumbnails has no maximum limit.
 *
 * @see @ref PicselConfig_thumbnailsMax,
 *      @ref PicselConfig_thumbnailsMaxScaled
 */
enum
{
    PicselConfig_ThumbnailsUnlimited = 0
};

/**
 * Set the maximum number of thumbnails created per document.
 *
 * Each page in a multi-page document will have a thumbnail created for it.
 * This configuration parameter will limit the number of thumbnails created
 * using an algorithm to generate the thumbnails around the current page.
 * This configuration option balances memory usage with performance, by
 * limiting the memory required to store large numbers of thumbnails with the
 * potential of having to re-create the same thumbnail multiple times.
 *
 * Use @ref PicselConfig_ThumbnailsUnlimited if thumbnails are to be created
 * for every page in the document.
 * The supported range of values is 0..UINT_MAX.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 *
 * @product File Viewer and Browser Development Kits.
 * @see     @ref PicselConfig_thumbnailFillColour,
 *          @ref PicselConfig_enableThumbnails,
 *          @ref PicselConfig_thumbnailsMaxScaled,
 *          @ref PicselConfig_showBlankThumbnail
 */
#define PicselConfig_thumbnailsMax  "Picsel_thumbnailsMax"

/**
 * Controls whether or not thumbnails are generated and passed to the Alien
 * application via the @ref AlienInformation_ThumbnailDone event.
 *
 *   0 => The Alien application does not require thumbnails.
 *   1 => Thumbnails are required and will be sent to the Alien application
 *        via the @ref AlienInformation_ThumbnailDone event.
 *
 * Defaults to 0.
 *
 * @product File Viewer and Browser Development Kits.
 * @see     @ref PicselConfig_thumbnailsMax,
 *          @ref PicselConfig_thumbnailFillColour,
 *          @ref PicselConfig_thumbnailsMaxScaled,
 *          @ref PicselConfig_showBlankThumbnail
 */
#define PicselConfig_enableThumbnails "Picsel_enableThumbnails"

/**
 * Set the maximum number of scaled thumbnails created per document.
 *
 * If @ref PicselConfig_thumbnailWidth and @ref PicselConfig_thumbnailHeight
 * are both greater than 0, then a scaled thumbnail will be created for each
 * new page of the document and indicated to the Alien application via the
 * @ref AlienInformation_ThumbnailDone event.  This configuration parameter
 * will limit the number of pages that a thumbnail will be created for i.e.
 * if this is assigned a value of 1 then a scaled thumbnail will be created
 * for page 1 only, and if this is assigned a value of 5 then a scaled thumbnail
 * will be created for the first 5 pages only.
 *
 * Use @ref PicselConfig_ThumbnailsUnlimited if scaled thumbnails are to be
 * created for every page in the document.
 * The supported range of values is 0..INT_MAX.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 *
 * @product File Viewer and Browser Development Kits.
 * @see     @ref PicselConfig_thumbnailsMax,
 *          @ref PicselConfig_enableThumbnails,
 *          @ref PicselConfig_thumbnailFillColour,
 *          @ref PicselConfig_thumbnailWidth,
 *          @ref PicselConfig_thumbnailHeight.
 *          @ref PicselConfig_showBlankThumbnail,
 */
#define PicselConfig_thumbnailsMaxScaled  "Picsel_thumbnailsMaxScaled"

/**
 * Set whether to display a blank thumbnail when user invokes either
 * @ref Thumbnail_Magnify or @ref Thumbnail_DocumentMap mode while a document
 * is being loaded.
 *
 * Setting to 1 will enable the blank thumbnail. If a document is still being
 * loaded, so perhaps no thumbnail is available yet, then a blank thumbnail
 * will be displayed when switching to either @ref Thumbnail_Magnify or
 * @ref Thumbnail_DocumentMap modes. This is the default behaviour.
 *
 * Setting to 0 will disable the blank thumbnail. If a document is still
 * being loaded, so no thumbnail may yet be available; when an attempt to
 * switch to either @ref Thumbnail_Magnify or @ref Thumbnail_DocumentMap
 * mode a @ref AlienInformation_ThumbnailResult event will be sent to the Alien
 * application with the status @ref PicselThumbnail_NoThumbnail, and the mode
 * change will not occur.
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 * If it is set at any other time, then it will only take effect when the
 * next document is loaded.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * @product File Viewer and Browser Development Kits.
 * @see     @ref PicselConfig_thumbnailsMax,
 *          @ref PicselConfig_enableThumbnails,
 *          @ref PicselConfig_thumbnailsMaxScaled,
 *          @ref PicselConfig_thumbnailWidth,
 *          @ref PicselConfig_thumbnailHeight.
 *          @ref PicselConfig_thumbnailFillColour,
 */
#define PicselConfig_showBlankThumbnail       "Picsel_showBlankThumbnail"

/**
 * Impose a document size limit on the Thumbnail_MultiplePages thumbnail
 * mode.
 *
 * When enabled, if the document does not contain enough pages for the
 * Multiple Pages thumbnail view, the PicselThumbnail_mode operation will
 * fail.
 *
 *   0 => No document size limit
 *   1 => The following document size limits are applied:
 *          - When thumbnail 'size' = 2, the document must have at least 2
 *            pages for Thumbnail_MultiplePages mode to be selected.
 *          - When thumbnail 'size' = 4, the document must have at least 3
 *            pages for Thumbnail_MultiplePages mode to be selected.
 *
 * If not set, defaults to 0 (no document size limit)
 */
#define PicselConfig_enableMultiplePagesModeDocumentSizeLimit \
                                "Picsel_enableMultiplePagesDocumentSizeLimit"

/**
 * Set the maximum number of loops of an image animation.
 *
 * An image animation, such as an animated GIF, will replay its animation in
 * a loop a number of times. This configuration parameter will set the
 * maximum number of times the animation will loop and replay.
 *
 * The supported range of values is 0..UINT_MAX.
 * Defaults to 0 (no maximum; animation plays forever).
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 *
 * @product File Viewer and Browser Development Kits.
 */
#define PicselConfig_maxAnimLoops   "Picsel_maxAnimLoops"

/**
 * Possible settings for the @ref PicselConfig_cursorEnable configuration
 * parameter. See @ref TgvEmulatedCursor.
 */
typedef enum PicselConfig_Cursor
{
    /**
     * Enable cursor [default]
     */
    PicselConfig_Cursor_Enable         = (1<<16),

    /**
     * Enable cursor, but cursor won't appear until PicselCursor_show() is
     * called after loading a document.
     */
    PicselConfig_Cursor_EnableAndHide,

    /**
     * Disable cursor.
     */
    PicselConfig_Cursor_Disable
}
PicselConfig_Cursor;

/**
 * Enables/Disables the cursor used in Word editing and Widgets.
 *
 * @ref TgvEmulatedCursor provides further information on the Picsel cursor.
 *
 * The valid options are defined in the enum @ref PicselConfig_Cursor.
 * If not set, defaults to enabled
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 * If it is set at any other time, then it will only take effect when the
 * next document is loaded.
 *
 * @product Any Picsel product with editing support such as the File Viewer
 *          Development Kit.
 * @see     @ref TgvEmulatedCursor, @ref TgvEditing.
 */
#define PicselConfig_cursorEnable   "Picsel_cursorEnable"

/**
 * A URL to the pointer image that will be used to display an emulated
 * cursor.
 *
 * When in edit mode the Alien application can control what cursor type (see
 * @ref PicselCursor_Type) is displayed using PicselCursor_setCursor(). This
 * will allow the alien application to indicate a potentially slow operation.
 * This configuration option sets the icon to be used for the pointer cursor
 * type.
 *
 * Only supported for MSWord and MSExcel editing.
 *
 * This image should be defined using a well formed URL e.g.
 * @c "file://localhost/image.png".  The image should be of any of the
 * available image formats as configured in AlienConfig_initialiseAgents().
 *
 * If no image is specified then a default image will be used.
 *
 * This configuration parameter should be set using PicselConfig_setString().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 * If it is set at any other time, then it will only take effect when the
 * next document is loaded.
 *
 * @product Any Picsel product with editing support such as the File Viewer
 *          Development Kit.
 */
#define PicselConfig_pointerBitmap     "Picsel_pointerBitmap"

/**
 * A URL to the hourglass image that will be used to display an emulated
 * cursor.
 *
 * When in edit mode the Alien application can control what cursor type (see
 * @ref PicselCursor_Type) is displayed using PicselCursor_setCursor(). This
 * will allow the alien application to indicte a potentially slow operation.
 * This configuration option sets the icon to be used for the hourglass
 * cursor type.
 *
 * Only supported for MSWord and MSExcel editing.
 *
 * This image should be defined using a well formed URL e.g.
 * @c "file://localhost/image.png".  The image should be of any of the
 * available image formats as configured in AlienConfig_initialiseAgents().
 *
 * If no image is specified then a default image will be used.
 *
 * This configuration parameter should be set using PicselConfig_setString().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 * If it is set at any other time, then it will only take effect when the
 * next document is loaded.
 *
 * @product Any Picsel product with editing support such as the File Viewer
 *          Development Kit.
 */
#define PicselConfig_hourglassBitmap   "Picsel_hourglassBitmap"

/**
 * The transparency to apply to the cursor bitmap.
 *
 * When editing certain document types, a cursor may be available.  This
 * configuration parameter will set the transparancy level of this cursor.
 *
 * 0 is fully transparent, 0xff is fully opaque.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 * If it is set at any other time, then it will only take effect when the
 * next document is loaded.
 *
 * @product Any Picsel product with editing support such as the File Viewer
 *          Development Kit.
 */
#define PicselConfig_cursorTransparency  "Picsel_cursorTransparency"

/**
 * Configures whether Text documents will be displayed on one long "page", or
 * divided into many typical sheets.
 *
 * This configuration parameter instructs the text document agent to break up
 * the text file into pages.  This will allow the text file to be loaded page
 * by page when @ref PicselConfigFV_singlePageLoad is enabled.
 *
 * - 0 => multi page [default]
 * - 1 => single page
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 * If it is set at any other time, then it will only take effect when the
 * next document is loaded.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * @product File Viewer and Browser Development Kits.
 * @see     @ref TgvInitialiseAgents
 */
#define PicselConfig_textDaSinglePage  "Picsel_TextDaSinglePage"

/**
 * List of values used to indicate the focal point when zooming.
 *
 * This enum is used with configuration parameter
 * @ref PicselConfig_zoomAboutPoint, and defines the different objects that
 * will be used as the focal point of a zoom action.
 */
typedef enum PicselConfig_zoomPoint
{
    /**
     * Zoom using the centre of the screen as the focal point.
     */
    PicselConfig_ZoomPoint_Centre         = (1<<16),

    /**
     * Zoom using the pointing device as the focal point.
     * The position of the pointing device is assumed from the position
     * passed in the last call to PicselFocus_navigateScreen() or
     * PicselPointer_move(), PicselPointer_down() or PicselPointer_up(). If
     * none of these API's is called then this option behaves as
     * @ref PicselConfig_ZoomPoint_Centre.
     */
    PicselConfig_ZoomPoint_Pointer,

    /**
     * Zoom using the focused object as the focal point.
     */
    PicselConfig_ZoomPoint_Focus,

    /**
     * Zoom using the last point of focused item as the focal point.
     */
    PicselConfig_ZoomPoint_FocusOrPointer
}
PicselConfig_zoomPoint;

/**
 * Specifies what the focal point will be when zooming.
 *
 * This configuration parameter will determine what the focal point of a zoom
 * will be.  Enum @ref PicselConfig_zoomPoint lists all the valid options.
 *
 * The default is PicselConfigBr_zoomPoint_Centre.
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * @product File Viewer and Browser Development Kits.
 */
#define PicselConfig_zoomAboutPoint "Picsel_zoomAboutPoint"

/**
 * DEPRECATED: Specifies the maximum number session history entries.
 *
 * THIS CONFIGURATION PARAMETER IS NO LONGER IMPLEMENTED - DO NOT USE.
 *
 * @product Not used.
 */
#define PicselConfig_sessionHistoryMaxEntries "Picsel_sessionHistoryMaxEntries"

/**
 * Maximum number of simultaneous open documents allowed.
 *
 * Defaults to 1.
 *
 * NOT YET IMPLEMENTED - DO NOT USE.
 */
#define PicselConfig_maxDocuments   "Picsel_maxDocuments"

/**
 * Alternative select key for HTML multi select form widgets.
 *
 * Configures which @ref PicselKey press will trigger a select event in a
 * HTML multi-select form widget when in the strong focus mode. The widget
 * will remain in strong focus, allowing subsequent selections.
 *
 * Default is PicselCmdNone.
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * @product Used by Picsel Browser products.
 * @see     @ref TgvFocus_Model, @ref PicselConfig_multiSelectWidgetAltDefocus,
 *          @ref PicselConfig_multiSelectWidgetAltDefocus2.
 */
#define PicselConfig_multiSelectWidgetAltSelect   "Picsel_multiSelectWidgetAltSelect"

/**
 * First alternative defocus key for HTML multi select form widgets.
 *
 * Configures which @ref PicselKey press will trigger a defocus event in a
 * HTML multi-select form widget when in the strong focus mode. The defocus
 * event will move out of the strong focus mode and enter the weak focus
 * mode.
 *
 * You can define two different keys to defocus a multi-select widget.
 * Configure the first using @ref PicselConfig_multiSelectWidgetAltDefocus
 * and if required, the second using
 * @ref PicselConfig_multiSelectWidgetAltDefocus2.
 *
 * Default is PicselCmdNone.
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * @product Used by Picsel Browser products.
 * @see     @ref TgvFocus_Model, @ref PicselConfig_multiSelectWidgetAltDefocus,
 *          @ref PicselConfig_multiSelectWidgetAltDefocus2,
 *          @ref PicselInputType_Key.
 */
#define PicselConfig_multiSelectWidgetAltDefocus  "Picsel_multiSelectWidgetAltDefocus"

/**
 * Second alternative defocus key for HTML multi select form widgets.
 *
 * Configures another @ref PicselKey press to trigger a defocus event in a
 * HTML multi-select form widget when in the strong focus mode. The defocus
 * event will move out of the strong focus mode and enter the weak focus
 * mode.
 *
 * You can define two different keys to defocus a multi-select widget.
 * Configure the first using @ref PicselConfig_multiSelectWidgetAltDefocus
 * and if required, the second using
 * @ref PicselConfig_multiSelectWidgetAltDefocus2.
 *
 * Default is PicselCmdNone.
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * @product Used by Picsel Browser products.
 * @see     @ref TgvFocus_Model, @ref PicselConfig_multiSelectWidgetAltDefocus,
 *          @ref PicselConfig_multiSelectWidgetAltDefocus2.
 */
#define PicselConfig_multiSelectWidgetAltDefocus2 "Picsel_multiSelectWidgetAltDefocus2"

/**
 * Enables/Disables Smart Scrolling (in supported builds).
 *
 * Smart Scrolling is an advanced link selection/panning model.  When enabled,
 * the Picsel library will dynamically switch between panning and stepping
 * through links in a document.  If there is a focusable item in the direction
 * the user wishes to pan, the focus will be set to that item.  Otherwise,
 * the document will be panned.
 *
 *  - 1 => Enable Smart Scrolling [default]
 *  - 0 => Disable Smart Scrolling
 *
 * If not set, defaults to Enabled (in supported builds)
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * @product Used by Picsel Browser Products, FVDK.
 * @see     @ref TgvFocus_Model.
 */
#define PicselConfig_enableSmartScrolling   "Picsel_enableSmartScrolling"

/**
 * Specifies whether empty files will be displayed.
 *
 * If this is enabled, empty files (a files of 0 size) will
 * be displayed, which will result in a single blank white page on screen. By
 * default, this is disabled, and the error
 * @ref PicselDocumentError_AgentMatchFailedNoData will be returned for empty
 * files.
 *
 *  - 0 => Don't display empty files [default]
 *  - 1 => Translate empty files
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * @product File Viewer and Browser Development Kits.
 */
#define PicselConfig_allowEmptyFiles  "Picsel_AllowEmptyFiles"


/**
 * A full path, specifying the location of vector fonts suitable for the
 * Picsel font engine.
 *
 * Picsel will search this directory and load all fonts identified.
 *
 * This does not need to be specified as the host can instruct Picsel to
 * include fonts in their library by using the Picsel_Font_Register_* APIs.
 * Providing fonts to the Picsel library can have some benefits however; it
 * allows Picsel-powered applications to maintain a consistent 'look and
 * feel' with legacy applications and it potentially enables ROM savings by
 * removing the need to embed fonts in the Picsel library.
 * The Picsel library already includes fonts for all supported languages,
 * which must be registered  at compile-time using functions like
 * Picsel_Font_Register_arar(). It can also accept fonts stored on the device
 * in supported formats such as uncompressed TrueType @c *.ttf files. Please
 * see the Picsel Format Support specification for the details of supported
 * fonts.
 *
 * A path should be in the following form (as described in
 * @ref AlienFile_PathNames). Note that more than one path can be specified
 * using commas to separate paths.  e.g.
 * @code
 * C:/Windows/Fonts/,C:/Windows/
 * @endcode
 *
 * It is acceptable to pass a directory that may sometimes contain no font
 * files (although, obviously, the Picsel library will then need to contain
 * fonts).
 *
 * PicselConfig_setString() should be used to set this configuration value.
 * This value should only be configured in AlienConfig_ready() for it to take
 * effect.
 *
 * In addition to the standard functions required for file viewing, the
 * following functions must be implemented to enable fonts to be loaded
 *  - AlienFile_findFirst()
 *  - AlienFile_findNext()
 *  - AlienFile_findAgain()
 *  - AlienFile_findDone()
 *  - AlienFile_findAllDone()
 *
 * @product All Picsel products.
 */
#define PicselConfig_fontsDirectory "Picsel_fontsDirectory"

/**
 * Preferred font size to be used by Picsel library.
 *
 * The font size is defined using the @ref Picsel_Fixed1616 type.
 *
 * The default font size is 10 (10<<16).
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * @product Various Picsel products
 */
#define PicselConfig_fontSize "PicselConfig_fontSize"

/**
 * Property specifying whether @ref AlienInformation_NetworkActivity events
 * are enabled.
 *
 * Network activity events are used to indicate that network activity is
 * taking place and indicate the type of activity. The type of activity is
 * defined in @ref PicselNetworkActivity, which will be provided with each
 * @ref AlienInformation_NetworkActivity event in the
 * @ref AlienInformation_NetworkActivityInfo structure. Disabling network
 * activity events will reduce the overall event activity, potentially
 * improving performance.
 *
 *  - 0 => disable the events [default]
 *  - 1 => enable the events
 *
 * If not set the Default is 0.
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * @product Any product that supports networking.
 * @see     @ref PicselConfig_reducedNotifyNetworkActivity.
 */
#define PicselConfig_enableNotifyNetworkActivity "Picsel_EnableNotifyNetworkActivity"

/**
 * Property specifying whether @ref AlienInformation_NetworkActivity events
 * are only generated for the first file load during a document download.
 *
 *  - 0 => Don't restrict the @ref AlienInformation_NetworkActivity events in
 *         any way. [default]
 *  - 1 => Restrict the @ref AlienInformation_NetworkActivity events so that
 *         they only get generated for the very first HTTP fetch that occurs
 *         during a document fetch.
 *
 * If not set the default is 0.
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * @pre     @ref PicselConfig_enableNotifyNetworkActivity needs to be enabled
 *          for this option to take effect.
 *
 * @product Any product that supports networking.
 * @see     @ref PicselConfig_enableNotifyNetworkActivity,
 *          @ref PicselNetworkActivity,
 *          @ref AlienInformation_NetworkActivity,
 *          @ref AlienInformation_NetworkActivityInfo.
 */
#define PicselConfig_reducedNotifyNetworkActivity "Picsel_ReducedNotifyNetworkActivity"


/**
 * The final stage output scale factor to be applied by the Alien in the
 * screen update handling.
 *
 * This configuration parameter would typically be set in AlienConfig_ready().
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * @product Various Picsel products
 */
#define PicselConfig_screenScaleFactor "Picsel_screenScaleFactor"

/**
 * Determines whether or not certain types of documents that are not read in
 * a sequential order (e.g. pre-2007 Office documents) should be read from
 * disk in large fixed sized blocks or chunks that occupy only one logical
 * unit of the file.  With such file formats, there is a device dependent
 * performance trade-off between reducing the number of reads and reducing
 * the number of wasted bytes read in.
 *
 * This configuration parameter would typically be set in AlienConfig_ready()
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * Possible values are:
 *    0 - The file is read in one logical unit at a time, so there are more
 *        reads, but no wasted file-transfer.  This is the optimum choice on
 *        most devices.
 *    1 - The file is read in a fixed size block at a time.  This results in
 *        fewer reads, but if the block covers a non-sequential part of the
 *        file, there may be some wasted reading.
 *
 * Default value is 0.
 *
 * @product Any product.
 */
#define PicselConfig_readInLargeBlocks "Picsel_readInLargeBlocks"



/**
 * Sets the text (via PicselConfig_setString()) to be used to indicate that
 * an empty placeholder paragraph in a Powerpoint document can be edited.
 * This is only relevant to builds with Editing enabled.
 *
 * The default value is "Select this paragraph to edit"
 *
 * @product File Viewer Development Kit and Browser products (with Editing).
 */
#define PicselConfig_PowerpointEmptyPlaceholderText \
                                    "Picsel_PowerpointEmptyPlaceholderText"


/**
 * Indicates whether the host can rotate the buffer containing the
 * screen image passed by AlienScreen_update before plotting it.
 * If the host can do this, the Picsel library may indicate the buffer
 * needs rotating by sending an AlienInformation_ChangeUpdateRotation
 * event with the rotation required to be applied by the host.
 *
 * It can be more efficient for the host to carry out the rotation rather
 * than the Picsel library. If however the host cannot carry out the rotation
 * the Picsel library will do so instead.
 *
 * Note even if the host indicates it has the capability to rotate the
 * buffer, the Picsel library may still do the rotation internally.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * Possible values are:
 *    0 - The host cannot carry out the rotation of the buffer
 *    1 - The host can carry out the rotation of the buffer
 *
 * Default value is 0
 *
 * @product CUI, ZUI
 */
#define PicselConfig_UpdateRotationCapable \
                                    "Picsel_UpdateRotationCapable"


/**
 * Indicates whether files with EXIF thumbnails available should
 * have the thumbnails cached in the database.
 *
 * Caching the EXIF thumbnails can increase performance on platforms where
 * file access is slow. However it increases the size of the database.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * Possible values are:
 *    0 - EXIF thumbnails should not be cached in the database.
 *    1 - EXIF thumbnails should be cached in the database.
 *
 * Default value is 1 for DCS, 0 for other Picsel products
 *
 * @product CUI, ZUI
 */
#define PicselConfig_cacheExifThumbs \
                                    "Picsel_cacheExifThumbs"

/**
 * Set the user's desktop.
 *
 * The value is a UTF-8 string specifying the path of the user's desktop.
 * This is a readable and writeable location on the device that can be used
 * to store files.
 *
 * Default is NULL.
 * @product CUI and DCS
 */
#define PicselConfig_userDesktopFolder "Picsel_userDesktopFolder"

/**
 * Set the user's documents folder.
 *
 * The value is a UTF-8 string specifying the path of the user's documents
 * folder. This is a readable and writeable location on the device where
 * the user's documents are stored or a location that can be used to store
 * documents if a current location does not exist.
 *
 * Default is NULL.
 * @product SOL, OA, CUI. Most products with file management
 */
#define PicselConfig_userDocumentsFolder "Picsel_userDocumentsFolder"

/**
 * Set the user's movies folder.
 *
 * The value is a UTF-8 string specifying the path of the user's movies
 * folder. This is a readable and writeable location on the device where
 * the user's video files are stored or a location that can be used to
 * store videos if a current location does not exist.
 *
 * Default is NULL.
 * @product CUI and DCS
 */
#define PicselConfig_userMoviesFolder "Picsel_userMoviesFolder"

/**
 * Set the user's music folder.
 *
 * The value is a UTF-8 string specifying the path of the user's music
 * folder. This is a readable and writeable location on the device where
 * the user's audio files are stored or a location that can be used to
 * store audio files if a current location does not exist.
 *
 * Default is NULL.
 * @product CUI and DCS
 */
#define PicselConfig_userMusicFolder "Picsel_userMusicFolder"

/**
 * Set the user's pictures folder.
 *
 * The value is a UTF-8 string specifying the path of the user's pictures
 * folder. This is a readable and writeable location on the device where
 * the user's pictures are stored or a location that can be used to store
 * pictures if a current location does not exist.
 *
 * Default is NULL.
 * @product CUI and DCS
 */
#define PicselConfig_userPicturesFolder "Picsel_userPicturesFolder"

/**
 * Set the user's widgets folder.
 *
 * The value is a UTF-8 string specifying the path of the user's widgets
 * folder. This is a readable and writeable location on the device where
 * downloaded widgets can be stored. The widgets that are stored in this
 * location can be executed; this poses a security threat so ensure that
 * they have come from a trusted location.
 *
 * Default is NULL.
 * @product CUI and DCS
 */
#define PicselConfig_userWidgetsFolder "Picsel_userWidgetsFolder"

/**
 * Set the key configurations for applications which are using
 * the @ref PicselInputType_Key input method, these are defined in terms
 * of Picsel key codes (not command codes), see picsel-app.h for details
 */
#define PicselConfig_zoomInKey             "PicselConfig_zoomInKey"
#define PicselConfig_zoomOutKey            "PicselConfig_zoomOutKey"
#define PicselConfig_rotateKey             "PicselConfig_rotateKey"
#define PicselConfig_previousPageKey       "PicselConfig_previousPageKey"
#define PicselConfig_nextPageKey           "PicselConfig_nextPageKey"
#define PicselConfig_fitPageKey            "PicselConfig_fitPageKey"
#define PicselConfig_fitWidthKey           "PicselConfig_fitWidthKey"
#define PicselConfig_fitHeightKey          "PicselConfig_fitHeightKey"
#define PicselConfig_firstPageKey          "PicselConfig_firstPageKey"
#define PicselConfig_lastPageKey           "PicselConfig_lastPageKey"
#define PicselConfig_panUpKey              "PicselConfig_panUpKey"
#define PicselConfig_panDownKey            "PicselConfig_panDownKey"
#define PicselConfig_panLeftKey            "PicselConfig_panLeftKey"
#define PicselConfig_panRightKey           "PicselConfig_panRightKey"
#define PicselConfig_panUpFullScreenKey    "PicselConfig_panUpFullScreenKey"
#define PicselConfig_panDownFullScreenKey  "PicselConfig_panDownFullScreenKey"
#define PicselConfig_panLeftFullScreenKey  "PicselConfig_panLeftFullScreenKey"
#define PicselConfig_panRightFullScreenKey "PicselConfig_panRightFullScreenKey"
#define PicselConfig_selectKey             "PicselConfig_selectKey"
#define PicselConfig_defocusKey            "PicselConfig_defocusKey"
#define PicselConfig_defocus2Key           "PicselConfig_defocus2Key"
#define PicselConfig_historyBackKey        "PicselConfig_historyBackKey"
#define PicselConfig_historyForwardKey     "PicselConfig_historyForwardKey"

/** The maximum number of characters in one of the above key strings including the
 *  trailing NUL character. */
#define PicselConfig_maxKeyStringLength    (35)

/**
 * Enable stereoscopic 3D rendering.
 *
 * Determines whether or not the document content or UI elements should be
 * stereoscopically rendered. The property has effect only if the stereoscopic
 * rendering is enabled in the Picsel library. This feature is not included
 * by default. Speak to your Picsel support representative for information
 * whether your build supports this feature.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * Valid values are 0 and 1.
 *
 * 0 - normal (2d) rendering.
 * 1 - stereoscopic rendering.
 *
 * The value is set to 0 by default.
 *
 * Most of stereoscopic rendering methods require the application
 * to be run on stereoscopic-enabled hardware in order to see the effect.
 * It's not required for both anaglyph methods
 * @ref PicselConfig_StereoMode_Anaglyph and
 * @ref PicselConfig_StereoMode_ColourCode although the screen may need
 * to be colour calibrated.
 * @product FVDK, SOL where 3D stereo rending is enabled.
 */
#define PicselConfig_enableStereo             "Picsel_enableStereo"

/**
 * Set the stereoscopic 3D rendering mode.
 *
 * The property specifies stereoscopic rendering method and should be
 * assigned a value from @ref PicselConfig_stereoModeValues.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * The property has effect only if @ref PicselConfig_enableStereo is
 * set to 1 and stereoscopic rendering is enabled in Picsel library.
 * This feature is not available in the Picsel library by default.
 * Speak to your Picsel support representative for information whether your
 * build supports this feature.
 *
 * @note The left screen copy always goes first in all rendering methods.
 *
 * @note Most of stereoscopic rendering methods require the application
 * to be run on stereoscopic-enabled hardware in order to see the effect.
 * It's not required for anaglyph methods
 * @p PicselConfig_StereoMode_Anaglyph and
 * @p PicselConfig_StereoMode_ColourCode although the screen may need
 * to be colour calibrated.
 */
#define PicselConfig_stereoMode               "Picsel_stereoMode"

/**
 * Set the level of perceived stereoscopic depth.
 *
 * Determines the level of stereoscopic effect. It is applied to all
 * stereoscopic modes set by @ref PicselConfig_stereoMode. The value is
 * a separation between elements in left and right screen copies. It is
 * a percentage value normally in the range -100 .. 100 but it may
 * exceed the range to exaggerate stereoscopic effect although it is
 * not recommended. Actual separation for each displayed item is calculated
 * in Picsel's stereoscopic rendering engine so cannot be defined by user.
 * Zero value means no stereo effect, negative value is used for a reversed
 * stereo. Recommended value is 100.
 *
 * The property has effect only if @ref PicselConfig_enableStereo is
 * set to 1 and stereoscopic rendering is enabled in Picsel library.
 * This feature is not available in the Picsel library by default.
 * Speak to your Picsel support representative for information whether your
 * build supports this feature.
 *
 */
#define PicselConfig_stereoDepth              "Picsel_stereoDepth"

/**
 * Possible values for PicselConfig_stereoMode
 */
typedef enum PicselConfig_stereoModeValues
{
    PicselConfig_StereoMode_None,       /**< Standard 2d mode.
                                             No stereoscopic effect. */
    PicselConfig_StereoMode_Scanlines,  /**< Alternate scanlines method
                                             where both screen copies
                                             are horizontally interlaced.
                                             Odd lines belong to right screen
                                             copy, even lines belong to left
                                             screen copy assuming
                                             the line count starts from zero.
                                             The method is used for screens
                                             with horizontally arranged
                                             micro-polarizers. */
    PicselConfig_StereoMode_Columns,    /**< Alternate columns method where
                                             both screen copies are vertically
                                             interlaced. Odd columns belong
                                             right screen copy, even columns
                                             belong to left screen copy
                                             assuming the line count starts
                                             from zero. The method is used
                                             for parallax barrier auto
                                             stereoscopic screens. */
    PicselConfig_StereoMode_Anaglyph,   /**< Anaglyph method. Colour
                                             glasses with red filter over
                                             left eye and cyan filter over
                                             right eye are required in order
                                             to see the stereoscopic effect.
                                             No special hardware is needed.  */
    PicselConfig_StereoMode_Shutter,    /**< Shutter glasses method where
                                             both screen copies are
                                             alternately switched. */
    PicselConfig_StereoMode_SideBySide, /**< Side By Side Mode, both screen
                                             copies are squeezed horizontally.*/
    PicselConfig_StereoMode_SideBySideDs, /**< This mode is similar to a dual
                                             screen mode. The difference is that
                                             both screen copies are squeezed
                                             horizontally. */
    PicselConfig_StereoMode_ColourCode, /**< Colour Code method. Colour
                                             glasses with brown filter over
                                             left eye and orange-blue filter
                                             over right eye are required in
                                             order to see the stereoscopic
                                             effect. No special hardware
                                             is needed. */
    PicselConfig_StereoMode_DualScreen, /**< Dual-Screen method. This is
                                             intended for devices that have
                                             their own stereo display
                                             mechanisms which require two
                                             identically-configured screen
                                             buffers as input. Selecting this
                                             method will result in
                                             @ref AlienScreen_dualUpdate
                                             being called to update both
                                             screens in place of the standard
                                             @ref AlienScreen_update. */

    PicselConfig_StereoMode_ForceSize = 0x10000 /**< Force the enum to be
                                                     contained in a 32 bit
                                                     integer */
}
PicselConfig_stereoModeValues;


/**
 * Enable Hardware accelerated rendering via multiple GPU Images.
 *
 * Determines whether or not rendering should be done by the GPU via
 * @ref TgvGPU. When this value is set true, screen updates
 * will not be sent by the normal screen update mechanism.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * Valid values are 0 and 1.
 *
 * 0 - normal rendering via @ref AlienScreen_update.
 * 1 - GPU-based rendering via @ref TgvGPU.
 *
 * The value is set to 0 by default.
 *
 */
#define PicselConfig_gpuRendering "Picsel_gpuRendering"

/**
 * Indicate a device's capacity for rendering to offscreen buffers (in the
 * form of GPU images) via the GPU.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * Valid values are 0 and 1.
 *
 * 0 - offscreen rendering is not supported.
 * 1 - offscreen rendering is supported.
 *
 * The value is set to 1 by default.
 *
 */
#define PicselConfig_gpuOffscreenRendering "Picsel_gpuOffscreenRendering"

/**
 * Specify the initial amount of memory available for GPU data.
 *
 * The value, in bytes, for the initial maximum amount of data that can be
 * uploaded to GPU memory via @ref TgvGPU, in the form of vertex buffers and
 * textures.
 *
 * If less memory than this amount is available on the device, the limit
 * will be adjusted down by the system. If more memory is available, the
 * limit may expand from this initial setting if
 * @ref PicselConfig_gpuMemoryLimit is set.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * The value is set to 20 megabytes by default.
 *
 */
#define PicselConfig_gpuMemorySize "Picsel_gpuMemorySize"

/**
 * Specify the maximum size for expansion of GPU memory usage.
 *
 * This configuration parameter should be set using PicselConfig_setInt().
 *
 * The value is set to 0 by default. Any value set less than the one for
 * @ref PicselConfig_gpuMemorySize will mean that the GPU memory size will
 * not expand beyond its original setting.
  */
#define PicselConfig_gpuMemoryLimit "Picsel_gpuMemoryLimit"

/**
 * Specifies how the application should handle unsaved edits on shutdown.
 *
 * Valid values are:
 *
 * 0 - Perform the default shutdown behaviour (usually the Picsel
 * application will prompt the user to save or discard edits).
 *
 * 1 - Always save over the existing file on shutdown without requiring user
 * interaction.  Once the save has completed, the application will be
 * shutdown.  Note that if the save fails, the application will still be
 * shutdown and any unsaved edits will be lost.  Failure to save will be
 * non-destructive (i.e. the source file will not be altered).
 *
 * 2 - Always discard any unsaved edits on shutdown without requiring user
 * interaction.
 *
 * Defaults to 0
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_saveActionOnShutdown "Picsel_saveActionOnShutdown"

/**
 * Specify the character(s) expected at the end of a line of text on the
 * alien platform.  This will be used to add the correct line-endings to
 * any dialogue strings in @ref AlienUserRequest_request calls.
 *
 * The string should be NULL-terminated UTF-8
 *
 * For example, on Windows and Symbian "\r\n" is the expected line-ending,
 * whereas on Linux and OS-X "\n" is expected.
 *
 * This configuration parameter should be set using PicselConfig_setString().
 *
 * This configuration parameter should only be set in AlienConfig_ready().
 *
 * Defaults to "\n"
 *
 * @product UE2 Based Products.  This may be expanded to other applications.

 */
#define PicselConfig_lineEnding      "Picsel_lineEnding"

/**
 * Specifies whether the alien platform supports text entry without a modal
 * dialogue which would obscure the document view.
 *
 * Valid values are:
 *
 * - Set to 0 - Text entry is only possible via a dialogue which will obscure the document.
 *     Text entry requests will be made from the Picsel app using
 *     @ref PicselUserRequest_Type_String user requests.
 *
 * - Set to 1 - Text entry is possible without obscuring the document
 *     e.g. A hard/soft keyboard is available to trigger @ref PicselApp_keyPress
 *          and @ref PicselApp_keyRelease calls, and/or
 *          the operating system supports passing IME composing text to the Picsel
 *          app via @ref PicselApp_composingText and @ref PicselApp_commitText
 *          Text entry requests may be made from the Picsel app using
 *          @ref AlienInformation_InlineTextEntry notifications.
 *          It is still possible that @ref PicselUserRequest_Type_String requests
 *          will be made in some circumstances.
 *
 * Defaults to 0
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_inlineTextEntry "Picsel_inlineTextEntry"

/**
 * Specifies whether or not cursor keys rotate with the device.
 *
 * Valid values are:
 *
 * - Set to 0 so Cursor keys do not rotate - The Picsel library will map the key codes
 *     to the current rotation.
 *
 * - Set to 1 so Cursor keys do rotate - No rotation mapping is required.
 *
 * Defaults to 0
 */
#define PicselConfig_cursorKeysRotate "Picsel_cursorKeysRotate"

/**
 * Specifies whether the alien platform supports e-mail. This
 * configuration parameter would typically be set in AlienConfig_ready().
 *
 * In order to support e-mail, the alien application must accept
 * PicselUserRequest_Type_MessageEdit requests from the Picsel application.
 *
 * Valid values are:
 * - Set to 0 to disable email sending. This is the default.
 * - Set to 1 so e-mail is supported
 */
#define PicselConfig_emailSupported "PicselConfig_emailSupported"

/**
 * Specifies the level of print support offered by the alien application.
 * This configuration parameter would typically be set in AlienConfig_ready().
 *
 * In order to support printing, the alien application must accept
 * PicselUserRequest_Type_PrintDocument requests from the Picsel application.
 *
 * Valid values are:
 *
 * - Set to 0 - printing is not supported. This is the default.
 * - Set to 1 - printing is supported via a native user interface
 * - Set to 2 - printing is supported and all settings can be controlled
 *     programmatically
 */
#define PicselConfig_printingSupported "PicselConfig_printingSupported"

/**
 * Specifies whether the alien application offers camera support or not.
 * This configuration parameter would typically be set in AlienConfig_ready().
 *
 * In order to support image insert from camera, the alien application must
 * accept PicselUserRequest_Type_GetImage requests, from the Picsel application,
 * with an image type of PicselUserRequest_ImageType_Camera.
 *
 * Valid values are:
 *
 * 0 - camera is not supported.
 *
 * 1 - camera is supported. This is the default.
 *
 */
#define PicselConfig_cameraSupported "PicselConfig_cameraSupported"

/**
 * Specifies whether the alien application offers image picker support or not.
 * This configuration parameter would typically be set in AlienConfig_ready().
 *
 * In order to support image insert, the alien application must accept
 * PicselUserRequest_Type_GetImage requests, from the Picsel application,
 * with an image type of PicselUserRequest_ImageType_Filesystem.
 *
 * Valid values are:
 *
 * 0 - image picker is not supported.
 *
 * 1 - image picker is supported. This is the default.
 *
 */
#define PicselConfig_imagePickerSupported "PicselConfig_imagePickerSupported"

/**
 * Specifies whether the alien platform has a Menu button. This
 * configuration parameter would typically be set in AlienConfig_ready().
 *
 * Valid values are:
 *
 * 0 - the platform doesn't have a Menu button. This is the default.
 *
 * 1 - the platform has a Menu button
 */
#define PicselConfig_hasMenuButton "PicselConfig_hasMenuButton"

/**
 * Specify a platform-specific app market URL for the application.
 *
 * This should be usuable by the AlienSystem_execute() implementation for the
 * platform as the "parameters" string parameter when the "task" parameter
 * is set to AlienSystem_Task_UrlLauncher
 *
 * The string should be NULL-terminated UTF-8
 *
 * For example, for the Google Android marketplace this would be
 * "market://details?id=<packagename>"
 *
 *
 * This configuration parameter should be set using PicselConfig_setString().
 *
 * This configuration parameter should only be set in AlienConfig_ready().
 *
 * Defaults to an empty string, which indicates that there is no app market
 * URL for this application on this platform.
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_appMarketUrl "PicselConfig_appMarketUrl"

/**
 * Specify a platform-specific app market rating URL for the application.
 *
 * This should be usuable by the AlienSystem_execute() implementation for the
 * platform as the "parameters" string parameter when the "task" parameter
 * is set to AlienSystem_Task_UrlLauncher
 *
 * The string should be NULL-terminated UTF-8
 *
 * For example, for the Google Android marketplace this would be
 * "market://details?id=<packagename>"
 *
 *
 * This configuration parameter should be set using PicselConfig_setString().
 *
 * This configuration parameter should only be set in AlienConfig_ready().
 *
 * Defaults to an empty string, which indicates that there is no app market
 * rating URL for this application on this platform.
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_appMarketRateUrl "PicselConfig_appMarketRateUrl"

/**
 * Specify whether or not "Save As" should be available in the application.
 *
 * When enabled, the application will allow the user to save edits to a new
 * file.
 *
 * When disabled, the application will only allow the user to save edits to
 * the source file.
 *
 * Valid values are:
 * - Set to 0 to Disable "Save As"
 * - Set to 1 to Enable "Save As".  This is the default.
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_saveAs "PicselConfig_saveAs"

/**
 * Specify whether or not "Save" should be available in the application.
 *
 * When enabled, the application will allow the user to save edits.
 *
 * Valid values are:
 * 0 - Disable "Save"
 * 1 - Enable "Save".  This is the default.
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_allowSave "PicselConfig_allowSave"

/**
 * Specify whether or not "Save and Close" should be available in the
 * application.
 *
 * When enabled, the application will present a button which allows the user
 * to save and close the current document.
 *
 * Valid values are:
 * 0 - Disable "Save and Close".  This is the default.
 * 1 - Enable "Save and Close".
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_allowSaveAndClose "PicselConfig_allowSaveAndClose"

/**
 * Specify whether or not the upload option should be available in the
 * application.
 *
 * When enabled, the application will present upload button which allow the
 * user to upload a document.  When selected, the alien application will
 * receive an @ref PicselUserRequest_Type_UploadDocument request.
 *
 * Valid values are:
 * 0 - Disable upload.  This is the default.
 * 1 - Enable upload.
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_allowUpload "PicselConfig_allowUpload"

/**
 * Specify whether or not the uploadAs option should be available in the
 * application.
 *
 * When enabled, the application will present uploadAs button which allow the
 * user to upload a document.  When selected, the alien application will
 * receive an @ref PicselUserRequest_Type_UploadDocument request.
 *
 * Valid values are:
 * 0 - Disable uploadAs.  This is the default.
 * 1 - Enable uploadAs.
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_allowUploadAs "PicselConfig_allowUploadAs"

/**
 * Specify whether or not "PDF Export" should be available in the application.
 *
 * When enabled, the application will allow the user to export documents to
 * PDF where supported.
 *
 * Valid values are:
 * 0 - Disable "PDF Export"
 * 1 - Enable "PDF Export".  This is the default.
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_allowPdfExport "PicselConfig_allowPdfExport"

/**
 * Specify whether or not document editing should be available in the
 * application.
 *
 * When enabled, the application will allow the user to make changes to
 * supported editable documents.
 *
 * When disabled, the application will not allow any document editing.
 *
 * Valid values are:
 * - Set to 0 to Disable editing
 * - Set to 1 to Enable editing.  This is the default.
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_docEditable "PicselConfig_docEditable"

/**
 * Specify whether or not spreadsheet column resizing should be available in
 * the application. This option is only available when the document editing
 * mode is disable (PicselConfig_docEditable is set to 0). So, if document
 * editing is ON then this property will be ignored.
 *
 * When enabled, the application will allow the user to resize spreadsheet
 * column.
 *
 * When disabled, the application will not allow any column resizing.
 *
 * Valid values are:
 * 0 - Disable resizing spreadsheet column.  This is the default.
 * 1 - Enable resizing spreadsheet column.
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_columnResizable "PicselConfig_columnResizable"

/**
 * Specify the path to an image to be used as a watermark over document content.
 *
 * When set, the application will overlay the document with the specified
 * image.
 *
 * By default, no watermark is set.
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_documentWatermark "PicselConfig_documentWatermark"

/**
 * Specify the optional path to an image to be used as a watermark over
 * document content for landscape mode.
 *
 * When set, the application will overlay the document with the specified
 * image, when in landscape mode.  When not set, behaviour will default to
 * use of PicselConfig_documentWatermark in all cases.
 *
 * By default, no landscape version watermark is set.
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_documentWatermarkLandscape "PicselConfig_documentWatermarkLandscape"

/**
 * Specify the Author field for annotations
 *
 * Valid values are a character string specifying the author, or NULL if the
 * author is not known.
 *
 * Currently used for PDF annotations; allows the alien to specify
 * a default author to be used when an annotation is created.
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_annotationAuthor "PicselConfig_annotationAuthor"

/**
 * Specify the current annotation colour
 *
 * The colour is specified in rgbx order: r = MSB, x = LSB (x is ignored)
 *
 * Currently used for PDF annotations; specifies the colour to be set
 * when an annotation is created.
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_annotationColour "PicselConfig_annotationColour"

/**
 * Specify whether or not the app was started with a temp file that may not be
 * valid on subsequent runs. If true, we wont attempt to get or set any
 * metadata for the file.
 *
 * Valid values are:
 * 0 - Started normally or with a file that is not temp
 * 1 - Started with a temp file.
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_startedWithFile "PicselConfig_startedWithFile"

/**
 * A list of initial application document views.
 *
 * Used by @ref PicselConfig_startView.
 */
typedef enum PicselConfig_Views
{
    /**
     * Full document view. This is the default.
     */
    PicselConfig_View_Default = 1,

    /**
     * The document is opened in the Print UI view.
     */
    PicselConfig_View_Print = 2,

    /**
     * The document is opened in the slideshow view.
     *
     * This is only valid for PPT/PPTX documents.
     */
    PicselConfig_View_Slideshow = 3,

    /**
     * The document is opened in the Document Info View.
     */
    PicselConfig_View_DocumentInfo = 4,

    /**
     * The application will open directly into the Visual Explorer.
     */
    PicselConfig_View_VisualExplorer = 5
}
PicselConfig_Views;

/**
 * Specify the initial document 'View' to display on startup.
 *
 * If not specified the application default view will be displayed.
 *
 * See @ref PicselConfig_Views for valid values.
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_startView "PicselConfig_startView"

/**
 * Specify the desired behaviour on document closure.
 *
 * Valid values are:
 * 0 - Default
 * 1 - Always return to dashboard
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_documentCloseBehaviour "PicselConfig_documentCloseBehaviour"

/**
 * Specify the full path which should be used when saving the current
 * document.
 *
 * This property can be set at any time, and is read whenever a document save
 * operation is initiated by the user.
 *
 * If not set, or if set to an empty string, the application will attempt to
 * overwrite the source document.
 *
 * This configuration parameter should be set using PicselConfig_setString().
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_documentSavePath "PicselConfig_documentSavePath"

/**
 * Specify the display name to use for the next document to be loaded.
 * This can be used when eg the filename of the document is a non user
 * friendly or programatically generated name.
 *
 * This property should be set before a document is loaded.
 *
 * If not set, or if set to an empty string, the application will
 * derive a suitable display name to use from the document filename.
 *
 * This configuration parameter should be set using PicselConfig_setString().
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_documentDisplayName "PicselConfig_documentDisplayName"

/**
 * Indicate that an external hardware keyboard is present.
 *
 * Valid values are:
 * 0 - No external hardware keyboard is present.
 * 1 - An external hardware keyboard is present.
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_externalKeyboardPresent "PicselConfig_externalKeyboardPresent"

/**
 * Specify whether or not clipboard features (e.g. cut/copy/paste) should be
 * available in the application.
 *
 * Valid values are:
 *  0 - Disable clipboard features.
 *  1 - Enable clipboard features.  This is the default.
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_allowClipboard "PicselConfig_allowClipboard"

/**
 * Document sharing methods.
 *
 * Values may be bitwise OR'd (|) together to enable multiple document
 * sharing methods.
 *
 * Used by @ref PicselConfig_allowedDocumentShareMethods
 */
typedef enum PicselConfig_DocumentShareMethod
{
    /**< Disable document sharing */
    PicselConfig_DocumentShareMethod_None      = 0,

    /**< Allow the user to share documents by email using the
     *   @ref PicselUserRequest_Type_MessageEdit request */
    PicselConfig_DocumentShareMethod_Email     = (1 << 0),

    /**< Allow the user to share documents via a generic sharing mechanism
     *   using the @ref PicselUserRequest_Type_ShareDocument request */
    PicselConfig_DocumentShareMethod_Share     = (1 << 1),

    /**< Force the enum to be contained in a 32 bit integer */
    PicselConfig_DocumentShareMethod_ForceSize = (1 << 16)
}
PicselConfig_DocumentShareMethod;

/**
 * Specify which document sharing methods should be available in the
 * application.
 *
 * See @ref PicselConfig_DocumentShareMethod for valid values.
 *
 * The default value is PicselConfig_DocumentShareMethod_Email.
 *
 * @product UE2 Based Products.  This may be expanded to other applications.
 */
#define PicselConfig_allowedDocumentShareMethods "PicselConfig_allowedDocumentShareMethods"

#endif /* !PICSEL_CONFIG_H */

/** @} */
