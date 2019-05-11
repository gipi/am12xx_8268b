/**
 * Printing of Documents
 *
 * $Id: picsel-print.h,v 1.21 2012/12/18 13:54:05 neilk Exp $
 * @file
 */
/* Copyright (C) Picsel, 2006. All Rights Reserved. */
/**
 * @defgroup TgvPrinting Printing
 * @ingroup TgvFileViewer
 *
 * Bitmap image output for printing documents on paper.
 *
 * The printing method currently supported by the Picsel Library is to
 * print individual pages as 16 bits per pixel
 * (@ref PicselScreenFormat_b5g6r5 "b5g6r5") raw bitmaps.
 *
 * Each page is broken up into a number of horizontal 'bands' to reduce
 * memory requirements. These bands are then passed to the Alien
 * Application for processing. We anticipate that this will be
 * printing, but other uses are possible, such as format conversion.
 *
 * The number of bands is specified by the Alien Application. Printing
 * many times to a very small band is typically much slower than
 * printing a small number of times to a larger band.
 *
 * The Alien Application is wholly responsible for converting the raw bitmap
 * data to any other format and for integration to any Alien printing
 * services, e.g. communicating with print drivers etc. The Picsel
 * Library simply supplies the bitmap data.
 *
 * The Picsel Library is running on the host operating system,
 * and so cannot guarantee that any function communicating with the
 * operating system will return in a timely fashion, or indeed that it
 * will return at all. All callbacks, therefore, should be regarded as
 * asynchronous and it is not advised to block other operations while
 * awaiting the return of a callback from the host operating system.
 *
 * For instance, PicselPrint_getPageSizes() contacts the host
 * operating system to request information, which is then returned
 * using the callback PicselPrintPageSizeCallback. It is not possible
 * to guarantee when that information will get returned.
 *
 * The following diagram describes the flow of function calls/callbacks
 * during a typical printing session.
 *
 * @dotfile print-flow.msc
 *
 * @{
 */

#ifndef PICSEL_PRINT_H
#define PICSEL_PRINT_H

# include "alien-types.h"
# include "alien-event.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Opaque structure owned by the Picsel library and passed by the Alien
 * application to some Picsel functions.
 */
typedef struct PicselPrint_PageData PicselPrint_PageData;


/**
 * Structure that holds the size of a page in inches.
 *
 * The values are held in fixed 16:16 format so 1 inch == 1<<16 (1 inch ==
 * 2.54 cm).
 *
 * See @ref Picsel_Fixed1616 for more on the 16:16 type
 *
 */
typedef struct PicselPrint_PageSize
{
    int  widthInches;   /**< page width in inches */
    int  heightInches;  /**< page height in inches*/
}
Picsel_PagePrintSize;

/**
 * Printer emulation
 */
typedef enum PicselPrint_Emulation
{
    PicselPrint_Emulation_Unknown   = 0,
    PicselPrint_Emulation_PCL3GUI   = (1<<0),
    PicselPrint_Emulation_PCL6      = (1<<1),
    PicselPrint_Emulation_PCL5      = (1<<2),
    PicselPrint_Emulation_GDI       = (1<<3),
    PicselPrint_Emulation_ESCPR     = (1<<4),
    PicselPrint_Emulation_SPL       = (1<<5),
    PicselPrint_Emulation_BJRASTER  = (1<<6),
    PicselPrint_Emulation_ESCP2     = (1<<7),
    PicselPrint_Emulation_HPGDI     = (1<<8),
    PicselPrint_Emulation_ForceSize = (1<<16) /**< Force size to be 32-bit */
}
PicselPrint_Emulation;

/**
 * Print output quality
 */
typedef enum PicselPrint_Quality
{
    PicselPrint_Quality_100dpi    = 100,    /**< 100 DPI */
    PicselPrint_Quality_150dpi    = 150,    /**< 150 DPI */
    PicselPrint_Quality_200dpi    = 200,    /**< 200 DPI */
    PicselPrint_Quality_300dpi    = 300,    /**< 300 DPI */
    PicselPrint_Quality_400dpi    = 400,    /**< 400 DPI */
    PicselPrint_Quality_ForceSize = (1<<16) /**< Force size to be 32-bit */
}
PicselPrint_Quality;

/**
 * Paper size for print output
 */
typedef enum PicselPrint_PaperSize
{
    PicselPrint_PaperSize_A3      = 65536,
    PicselPrint_PaperSize_A4,
    PicselPrint_PaperSize_A5,
    PicselPrint_PaperSize_B5,
    PicselPrint_PaperSize_Letter,
    PicselPrint_PaperSize_Legal,
    PicselPrint_PaperSize_Executive
}
PicselPrint_PaperSize;

/**
 * Scale mode for print output
 */
typedef enum PicselPrint_Scale
{
    /**
     * Scale the document content to the paper size, maintaining the aspect
     * ratio.  Note, some document content may be positioned outside the
     * printable area, in which case it will be cropped.
     */
    PicselPrint_Scale_PaperSize         = 65536,

    /**
     * Scale the document content to the printable area, maintaining the
     * aspect ratio.
     */
    PicselPrint_Scale_PrintableArea,

    /**
     * Print the document content at its original size.  Note, large pages
     * will be cropped to the paper size, and small pages will be centered
     * on the paper.
     */
    PicselPrint_Scale_OriginalSize
}
PicselPrint_Scale;

/**
 * Flags controlling the print output.  These flags may be bitwise OR'd (|)
 * together.
 */
enum
{
    /**
     * Paper layout options
     */
    PicselPrint_OutputFlags_Portrait   = 0,      /**< The document should
                                                  *   be printed in portrait
                                                  *   orientation */
    PicselPrint_OutputFlags_Landscape  = (1<<0), /**< The document should
                                                  *   be printed in landscape
                                                  *   orientation */

    /**
     * Colour options
     */
    PicselPrint_OutputFlags_Colour     = 0,      /**< The document should be
                                                  *   printed in full colour,
                                                  *   if supported by the
                                                  *   printer */
    PicselPrint_OutputFlags_Monochrome = (1<<1)  /**< The document should be
                                                  *   printed in monochrome */
};

/**
 * Type to store a combination of the above flags.
 */
typedef unsigned int PicselPrint_OutputFlags;


/**
 * Callback that provides a note of the page sizes for the currently loaded
 * document.
 *
 * This is invoked some time after PicselPrint_getPageSizes() is called,
 * and is guaranteed to be called (unless the Picsel library is
 * shutdown while the request is being processed).
 *
 * @param[in] alienContext  Set by the Alien Application as part of
 *                          PicselApp_start()
 * @param[in] pageSize      Pointer to the first of an array of pages.
 *                          The Picsel library owns the memory for this
 *                          and may free it after this function
 *                          completes.  If the Alien Application
 *                          requires access to it for longer, then it
 *                          must make a copy.
 *                          This will be NULL in error cases.
 * @param[in] pageCount     The number of pages for the current document
 *                          This will be 0 in error cases.
 */
typedef void (*PicselPrint_PageSizeCallback)(
                                         Alien_Context        *alienContext,
                                         Picsel_PagePrintSize *pageSize,
                                         int                   pageCount);


/**
 * Callback that occurs once initialisation for a print request has
 * completed.
 *
 * This is invoked some time after a call to
 * PicselPrint_printPageInitialise() returns, and is guaranteed to be
 * called (unless the Picsel library is shutdown while the request is
 * being processed).
 *
 * @param[in] alienContext  See PicselApp_start()
 * @param[in] pageData      Structure that is owned by the Picsel
 *                          library. This must be passed to any calls
 *                          to PicselPrint_printBand(). If this is NULL
 *                          then initialisation failed and printing
 *                          cannot proceed.
 */
typedef void (*PicselPrint_InitialiseCallback)(
                                           Alien_Context        *alienContext,
                                           PicselPrint_PageData *pageData);


/**
 * Callback that occurs once finalisation for a print request has
 * completed.
 *
 * This is invoked some time after a call to
 * PicselPrint_printPageFinalise() returns, and is guaranteed to be
 * called unless the Picsel library is shutdown while the request is
 * being processed.
 *
 * @param[in] alienContext  See PicselApp_start()
 */
typedef void (*PicselPrint_FinaliseCallback)(Alien_Context *alienContext);


/**
 * Alien Application provided callback that is called by the Picsel library
 * when a band of a page has completed printing.
 *
 * This is guaranteed to be invoked once for every successful call to
 * PicselPrint_printBand(), i.e, for each band completed on the page (unless
 * the Picsel library was shutdown while PicselPrint_printBand() was being
 * processed).
 *
 * If printing of the band failed, widthPixels, widthBytes and heightPixels
 * will all be set to 0.
 *
 * Note that document content is currently always printed in 16 bits per
 * pixel @ref PicselScreenFormat_b5g6r5 "b5g6r5" format. Use one of the
 * utility functions (such as
 * Picsel_PixelBlock_convert_b5g6r5_r8g8b8()) to convert the output to
 * a different colour format or depth.
 *
 * @param[in] alienContext  See PicselApp_start().
 * @param[in] bandNumber    The number of the band requested.
 * @param[in] widthPixels   The width of the band in pixels.  All bands for
 *                          any given page will have the same width.
 * @param[in] widthBytes    The number of bytes taken up by a single row of
 *                          pixels within a band, including any padding bytes.
 * @param[in] heightPixels  The height in pixels of the band.
 * @param[in] moreToGo      1 if there is more content to print; 0 otherwise
 */
typedef void (*PicselPrint_BandCallback)(Alien_Context *alienContext,
                                         int            bandNumber,
                                         int            widthPixels,
                                         int            widthBytes,
                                         int            heightPixels,
                                         int            moreToGo);


/**
 * Query the sizes for all pages of the currently loaded document.
 *
 * This function is asynchronous; the results will be provided via the
 * callback at some later point.
 *
 * Results will be unreliable if the current document is still loading or a
 * new document is loaded after this is called but before the callback
 * runs.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 * @param[in] picselView    Reserved for future use.  Set this to NULL.
 * @param[in] callback      Pointer to a function that will be called when
 *                          the array of page sizes is available
 *
 * @return The queue status, normally 1. See @ref TgvAsync_Queue.
 */
int PicselPrint_getPageSizes(Picsel_Context               *picselContext,
                             Picsel_View                  *picselView,
                             PicselPrint_PageSizeCallback  callback);


/**
 * Prepare a page for printing.
 *
 * This can safely be called from a callback from Picsel library code
 * (e.g. the PicselPrint_PageSizeCallback() function).
 *
 * This function cannot be cancelled.  Use PicselPrint_printPageFinalise()
 * once 'callback' has run if you want to halt printing.
 *
 * The Alien Application is responsible for setting the size of the band
 * that a document will be printed to.  A small band requires little memory
 * but may mean that a document takes a long time to print; a large band
 * uses more memory but is much quicker to print a full page.  Picsel
 * recommend that a band at least 104 pixels high is used, with the height
 * being a multiple of 8 pixels (to maximise efficiency and quality in JPEG
 * compression)
 *
 * For a Portrait A4 page printed at 200 pixels per inch and a 2
 * bytes-per-pixel output (the default for all Picsel products), the buffer
 * size should then be 8.25 * 200 * 104 * 2 = 343200 (ie. around 336K)
 *
 * Optionally, the page can be saved to a JPEG file at the same time.  To
 * enable this supply the full file name to write to in the @c jpegPath
 * parameter; otherwise pass NULL.  The quality of the JPEG can be set from
 * 0..100 where 100 is the highest level.
 *
 * Should the document to be printed exceed bandWidthPixels *
 * pageLengthPixels then a rectangular portion of the document starting
 * from the top-left-hand corner of the document will be printed.
 *
 * Example
 * @htmlonly
 * <pre>
  |------------|--------------------|
  |            |                    |  ^
  |            |                    |  |
  | printed    |                    |  |
  |            |                    |  |
  |            |    discarded       |
  |            |                    | pageLengthPixels
  |------------|                    |
  |                                 |  |
  |   discarded                     |  |
  |                                 |  |
  |                                 |  |
  |                                 |  |
  |                   discarded     |  |
  |                                 |  |
  |                                 |  \/
  |                                 |
  |----------------------------------

 <------- bandWidthPixels -------->

 * </pre>
 * @endhtmlonly
 *
 *
 * @param[in] picselContext     Set by AlienEvent_setPicselContext().
 * @param[in] picselView        Reserved for future use.  Set this to NULL.
 * @param[in] jpegPath          Pointer to path of file to print to, or NULL
 *                              if no output file is to be created.
 * @param[in] jpegQuality       Jpeg quality (0..100).
 * @param[in] pageNumber        Which page to print.  Based from 0.
 * @param[in] resolution        The resolution to print at.  In dots (pixels)
 *                              per inch.
 * @param[in] buffer            Pointer to the block of memory that the
 *                              Picsel library will print to.  The Alien
 *                              Application is responsible for allocating
 *                              and managing this memory.
 * @param[in] bufferSizeBytes   The size of 'buffer' in bytes.
 * @param[in] bandWidthPixels   The width (in pixels) of the bands to be
 *                              printed. This will increase/decrease in
 *                              relation to @c resolution.
 * @param[in] pageLengthPixels  The length (in pixels) of the target page
 * @param[in] fitToPage         1 if the document page should be scaled to fit
 *                              bandWidthPixels and/or pageLengthPixels; 0
 *                              otherwise.
 * @param[in] centreOnPage      1 if the document should be centred within a
 *                              page of size (bandWidthPixels,
 *                              pageLengthPixels); 0 otherwise.
 *                              This option is ignored if 'fitToPage' is set
 *                              to 0.
 * @param[in] callback          Pointer to a function that will be called
 *                              when initialisation completes.
 *
 * @return      The queue status, normally 1. See @ref TgvAsync_Queue.
 *
 * @post PicselPrint_InitialiseCallBack is always called after this
 *       function, and will be NULL upon error (such as invalid
 *       @c pageNumber).
 */
int PicselPrint_printPageInitialise(
                       Picsel_Context                 *picselContext,
                       Picsel_View                    *picselView,
                       const char                     *jpegPath,
                       int                             jpegQuality,
                       int                             pageNumber,
                       int                             resolution,
                       void                           *buffer,
                       int                             bufferSizeBytes,
                       int                             bandWidthPixels,
                       int                             pageLengthPixels,
                       int                             fitToPage,
                       int                             centreOnPage,
                       PicselPrint_InitialiseCallback  callback);


/**
 * Halt the printing process for a page, freeing any Picsel library
 * resources associated with it.
 *
 * This function is asynchronous so the buffer provided
 * in PicselPrint_printPageInitialise() cannot be freed until the
 * PicselPrint_FinaliseCallback() function is run.
 *
 * This can safely be called from a callback from Picsel library code
 * (e.g. the PicselPrint_PageSizeCallback() function).
 *
 * @param[in] picselContext     Set by AlienEvent_setPicselContext().
 * @param[in] pageData          Set earlier by PicselPrint_InitialiseCallback()
 * @param[in] callback          Pointer to a function that will be called when
 *                              finalisation completes
 *
 * @return      The queue status, normally 1. See @ref TgvAsync_Queue.
 *
 * @post PicselPrint_FinaliseCallback   always called after this function,
 *                                      to free the buffer provided in
 *                                      PicselPrint_printPageInitialise()
 */
int PicselPrint_printPageFinalise(Picsel_Context               *picselContext,
                                  PicselPrint_PageData         *pageData,
                                  PicselPrint_FinaliseCallback  callback);


/**
 * Print a horizontal band of the document to the buffer supplied to
 * PicselPrint_printPageInitialise().
 *
 * @pre This must not be called while any other print request,
 *      including other calls to this function, are outstanding.  It
 *      may however be called from a callback from Picsel code (e.g.
 *      PicselPrint_InitialiseCallback() or PicselPrint_BandCallback()).
 *
 * @param[in] picselContext     Set by AlienEvent_setPicselContext().
 * @param[in] pageData          Set earlier by PicselPrint_InitialiseCallback().
 * @param[in] bandNumber        The band to print.  Based from 0, with band
 *                              0 being the top of the page.
 * @param[in] callback          Pointer to a function that will be called
 *                              when the band is available.
 *
 * @return The queue status, normally 1. See @ref TgvAsync_Queue.
 *
 * @post PicselPrint_BandCallback() will be called once later.
 */
int PicselPrint_printBand(Picsel_Context           *picselContext,
                          PicselPrint_PageData     *pageData,
                          int                       bandNumber,
                          PicselPrint_BandCallback  callback);


/**
 * Cancel an outstanding call to PicselPrint_printBand().  The associated
 * PicselPrint_BandCallback() will not be run.
 *
 * If necessary, this can be called at any time. Any currently printing
 * band will be completed before the cancellation will take effect
 *
 * @param[in] picselContext     Set by AlienEvent_setPicselContext()
 * @param[in] pageData          Set earlier by PicselPrint_InitialiseCallback()
 */
void PicselPrint_printBandCancel(Picsel_Context       *picselContext,
                                 PicselPrint_PageData *pageData);

/**
 * Report the status of a print operation.
 *
 * If possible, the alien application should call this to report the progress
 * of a print operation while processing a
 * @ref PicselUserRequest_Type_PrintDocument request.
 *
 * @param[in] picselContext     Set by AlienEvent_setPicselContext()
 * @param[in] page              The current page being processed.  Page
 *                              numbers start at 1
 * @param[in] progress          The progress of the print operation (as a
 *                              percentage of the entire operation).
 *                              Suitable values are in the range 0 - 100
 */
void PicselPrint_printProgress(Picsel_Context *picselContext,
                               unsigned int    page,
                               unsigned int    progress);

/**
 * Search for printers on the local network
 *
 * The result of the search will be sent to the alien application as an
 * AlienInformation_SearchForPrintersResult event.  This operation may take
 * several seconds to complete.
 *
 * @param[in] picselContext     Set by AlienEvent_setPicselContext()
 *
 * @return  The queue status, normally 1. See @ref TgvAsync_Queue.
 */
int PicselPrint_searchForPrinters(Picsel_Context *picselContext);

/**
 * Print the current document to the printer with the supplied IP address
 *
 * This operation may take several seconds to complete.  This operation can
 * be cancelled with a call to @ref PicselPrint_cancel.
 *
 * This function should only be called after a document has been loaded (i.e.
 * an @ref AlienInformation_DocumentLoaded event has been received for this
 * document).
 *
 * While processing this request, the Picsel library will send @ref
 * AlienInformation_PrintDocumentProgress events to indicate the progress of
 * the operation.  When the operation completes, the Picsel library will send
 * an @ref AlienInformation_PrintDocumentResult with the result of the
 * operation.
 *
 * @param[in] picselContext     Set by AlienEvent_setPicselContext()
 * @param[in] ip                The IP address of the printer to print to
 * @param[in] emulation         The printer language to use, as reported by
 *                              the Picsel library in the
 *                              @ref AlienInformation_SearchForPrintersResult
 *                              event
 * @param[in] pages             An array of page numbers to be printed.  Page
 *                              numbers start at 1.  If NULL, all pages will
 *                              be printed.
 * @param[in] numPages          The number of entries in the 'pages' array
 * @param[in] numCopies         The number of copies to print
 * @param[in] quality           The desired print quality
 * @param[in] paperSize         The desired paper size
 * @param[in] scale             The scale method to use
 * @param[in] flags             Flags describing the print output.  See @ref
 *                              PicselPrint_OutputFlags more information.
 *
 * @return  The queue status, normally 1. See @ref TgvAsync_Queue.
 */
int PicselPrint_printDocument(Picsel_Context          *picselContext,
                              const char              *ip,
                              PicselPrint_Emulation    emulation,
                              unsigned int            *pages,
                              unsigned int             numPages,
                              unsigned int             numCopies,
                              PicselPrint_Quality      quality,
                              PicselPrint_PaperSize    paperSize,
                              PicselPrint_Scale        scale,
                              PicselPrint_OutputFlags  flags);

/**
 * Cancel a print operation which was scheduled with a call to @ref
 * PicselPrint_printDocument
 *
 * If possible, the print operation will be cancelled.  It may not be
 * possible to cancel the operation if it is almost complete.
 *
 * When a print job is successfully cancelled, an @ref
 * AlienInformation_PrintDocumentResult event with the @ref
 * AlienInformation_PrintDocumentResult_Error_Cancelled error code will be
 * send to the application.
 *
 * @param[in] picselContext     Set by AlienEvent_setPicselContext()
 *
 * @return  The queue status, normally 1. See @ref TgvAsync_Queue.
 */
int PicselPrint_cancel(Picsel_Context *picselContext);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PICSEL_PRINT_H */
