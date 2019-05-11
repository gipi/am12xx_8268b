/**
 * Functions for selecting which Picsel product to launch. The Alien
 * application must not call these functions directly, but must pass them as
 * function pointer parameters to PicselApp_start().
 *
 * @file
 * $Id: picsel-entrypoint.h,v 1.70 2012/06/25 12:33:51 hugh Exp $
 *
 */
/* Copyright (C) Picsel, 2005-2008. All Rights Reserved. */
/**
 * @addtogroup TgvInitialisation
 * @{
 */

#ifndef PICSEL_ENTRYPOINT_H
#define PICSEL_ENTRYPOINT_H

#include "alien-types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct PicselEvent;

/**
 * Prototype function that will be called to launch any specific product.
 *
 * The Alien application can safely ignore the argument descriptions below.
 *
 * @param[in] picselContext Set by AlienEvent_setPicselContext().
 * @param[in] event         The start event from the Alien application.
 *
 * @return                  1 if the function succeeds; 0 otherwise.
 */
typedef int (*Picsel_initialiseFn)(             Picsel_Context *picselContext,
                                   const struct PicselEvent    *event);


/**
 * Pass a pointer to this function, as a parameter to PicselApp_start(), to
 * start the @ref TgvFileViewer product; allows viewing of PDFs, images, HTML
 * and Microsoft Office files. Do not call this function directly.
 */
Picsel_initialiseFn Picsel_EntryPoint_FileViewer(void);

/**
 * Pass a pointer to this function, as a parameter to PicselApp_start(), to
 * start the Text Dump product; allows dumping text from of PDFs, HTML,
 * text, WMF and Microsoft Office files using functions including
 * PicselTextDump_requestSectionText(). Do not call this function directly.
 */
Picsel_initialiseFn Picsel_EntryPoint_TextDump(void);

/**
 * Pass a pointer to this function, as a parameter to PicselApp_start(), to
 * start the @ref AttachmentViewer product; thin client for viewing email
 * attachments when used with a Picsel Server. Do not call this function
 * directly.
 */
Picsel_initialiseFn Picsel_EntryPoint_Avc(void);

/**
 * Pass a pointer to this function, as a parameter to PicselApp_start(), to
 * start the AVC (Live Attachment Viewer Client) product; networked demo of
 * @ref AttachmentViewer. See Picsel_EntryPoint_Avc(). Do not call this
 * function directly.
 */
Picsel_initialiseFn Picsel_EntryPoint_AvcLive(void);


/**
 * Do not use; for Picsel Internal Testing Only. Do not call this function
 * directly.
 */
Picsel_initialiseFn Picsel_EntryPoint_videoplayer(void);

/**
 * Do not use; for Picsel Internal Testing Only. Do not call this function
 * directly.
 */
Picsel_initialiseFn Picsel_EntryPoint_comicViewer(void);

/**
 * Select a container that is suitable for clients who use
 * PicselApp_addEmbedded(); the container app has no user interface so you
 * must add at least one embedded app if you use this entrypoint.
 *
 * Do not use; for Picsel Internal Testing Only. Do not call this function
 * directly.
 */
Picsel_initialiseFn Picsel_EntryPoint_embeddedContainer(void);

/**
 * Do not use; for Picsel Internal Testing Only. Do not call this function
 * directly.
 */
Picsel_initialiseFn Picsel_EntryPoint_default(void);

/**
 * Pass a pointer to this function, as a parameter to PicselApp_start(), to
 * start the @ref Browser product; provides a full Internet browser. Do not
 * call this function directly.
 */
Picsel_initialiseFn Picsel_EntryPoint_browser(void);

/**
 * Do not use; for Picsel Internal Testing Only. Do not call this function
 * directly.
 */
Picsel_initialiseFn Picsel_EntryPoint_contentPlayer(void);

/**
 * Do not use; for Picsel Internal Testing Only. Do not call this function
 * directly.
 */
Picsel_initialiseFn Picsel_EntryPoint_TestApp(void);

/**
 * Pass a pointer to this function, as a parameter to PicselApp_start(), to
 * start the TGV test app; test sets used for validating an OEM port. Do not
 * call this function directly.
 */
Picsel_initialiseFn Picsel_EntryPoint_tgvTestApp(void);

/**
 * Do not use; for Picsel Internal Testing Only. Do not call this function
 * directly.
 */
Picsel_initialiseFn Picsel_EntryPoint_fxTest(void);

/**
 * Pass a pointer to this function, as a parameter to PicselApp_start(), to
 * start the @ref ThumbnailGenerator product; generates thumbnails for each
 * page of a document, allowing rich native user interface page selection.
 * Do not call this function directly.
 */
Picsel_initialiseFn Picsel_EntryPoint_ThumbGenerator(void);

/**
 * Pass a pointer to this function, as a parameter to PicselApp_start(), to
 * start the @ref TgvMediaViewer "Media Viewer" product; generate and
 * manage thumbnails for large sets of multimedia files. Do not call this
 * function directly.
 */
Picsel_initialiseFn Picsel_EntryPoint_mediaViewer(void);

/**
 * Pass a pointer to this function, as a parameter to PicselApp_start(), to
 * start the UE2 client application, for communicating with the UE2
 * display manager to display UI controls on screen. Do not call this
 * function directly.
 */
Picsel_initialiseFn Picsel_EntryPoint_ue2Client(void);

/**
 * Pass a pointer to this function, as a parameter to PicselApp_start(), to
 * start the UE2 client combined Lomond display manager.
 * Do not call this function directly.
 */
Picsel_initialiseFn Picsel_EntryPoint_dispmanUe2client(void);

 /**
 * Pass a pointer to this function, as a parameter to PicselApp_start(), to
 * start the UE2 client combined Lomond display manager.
 * Do not call this function directly.
 */
Picsel_initialiseFn Picsel_EntryPoint_dispmanUe2ClientLite(void);

/**
 * Pass a pointer to this function, as a parameter to PicselApp_start(), to
 * start the Thumbnailer app; generate thumbnails for large sets of
 * multimedia files. Do not call this function directly.
 */
Picsel_initialiseFn Picsel_EntryPoint_thumbnailer(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PICSEL_ENTRYPOINT_H */
