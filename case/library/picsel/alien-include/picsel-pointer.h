/**
 * Mouse Pointer, Stylus and Touch Input
 *
 * The file picsel-cursor.h provides a different and incompatible
 * type of mouse pointer which is less commonly used.
 *
 * The functions in this file are offered by the TGV library for use by
 * the Alien application. If these features are not required by the
 * application, it is not necessary to call these.
 *
 * $Id: picsel-pointer.h,v 1.11 2012/06/11 15:25:17 neilg Exp $
 * @file
 */
/* Copyright (C) Picsel, 2004-2008. All Rights Reserved. */
/**
 * @defgroup TgvDevicePointer Device Pointer Input
 * @ingroup TgvCommand
 *
 * The Picsel pointer API allows user interaction at specific screen
 * coordinates.
 *
 * This is typically achieved using a touchscreen, joystick or
 * mouse. Functions like PicselPointer_down() indicate user input and
 * can be used whether or not a pointer (arrow) is displayed.
 *
 * @ifnot cui
 * See @ref TgvEmulatedCursor, which provides a different and incompatible
 * type of emulated mouse pointer.
 * @endif
 * @{
 */

#ifndef PICSEL_POINTER_H
#define PICSEL_POINTER_H

#include "alien-types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 *  Enum for PicselPointer button Id.
 *  Used in notifications like PicselPointer_down().
 *  Values in the range 0 to PicselPointerButton_TouchBase are reserved.
 */
typedef enum PicselPointerButton
{
    /** Indicates that no button is pressed.  Used for tracking mouse or
     *  cursor movement across the screen.
     */
    PicselPointerButton_None   = 0,

    /** The normal mouse button, typically representing selection of the
     *  item under the pointer.
     */
    PicselPointerButton_Left   = 1,
    PicselPointerButton_Middle = 2,
    PicselPointerButton_Right  = 3,

    /** Base value for touch and multitouch inputs.  Button IDs >= this value
     *  will be interpreted as touch input.  In applications where multiple
     *  touch points are supported, each point's ID should be unique within
     *  the gesture.
     */
    PicselPointerButton_TouchBase = 32768,

    /** A base value for manufacturer-specified button values.
     * A manufacturer may extend the range of available button values by
     * defining PointerId values >= 65536
     * Manufacturer button Id values must be provided to application developers
     * as an addition to any Picsel SDK.
     */
    PicselPointerButton_CustomBase = 65536
}
PicselPointerButton;


/**
 * Bitfield for PicselPointer modifier keys.
 * Bits indicate the state of modifier keys at the time of the pointer
 * event.
 * Used in notifications like PicselPointer_down().
 */
typedef enum PicselPointerModifierState
{
    PicselPointerModifierState_Control      = 1<<0,
    PicselPointerModifierState_Shift        = 1<<1,
    PicselPointerModifierState_Alt          = 1<<2
}
PicselPointerModifierState;


/**
 * Notification that the device pointer's button or touchscreen has been
 * pressed by the user.
 *
 * The coordinates are based on 0, 0 being the top left corner of the screen,
 * with positive coordinates going right and down. Note that these
 * coordinates are not affected by the orientation of the Alien screen.
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 * @param timestamp     Timestamp for the event, in milliseconds
 * @param button        Which button has been pressed.
 *                      See @ref PicselPointerButton.
 * @param modifierState State of any modifier keys at the time of the
 *                      pointer event.
 *                      See @ref PicselPointerModifierState.
 * @param x             The x location of the pointer when the button
 *                      was pressed.
 * @param y             The y location of the pointer
 *
 * @return              The queue status, normally 1. See @ref TgvAsync_Queue.
 *
 * @see                 PicselPointer_up(), PicselPointer_move()
 */
int PicselPointer_down(Picsel_Context *picselContext,
                       unsigned int    timestamp,
                       int             button,
                       unsigned int    modifierState,
                       int             x,
                       int             y);

/**
 * Notification that the device's pointer has been moved by the user, or that
 * the device's touchscreen has detected a movement.
 *
 * The coordinates are based on 0, 0 being the top left corner of the screen,
 * with positive coordinates going right and down. Note that these
 * coordinates are not affected by the orientation of the Alien screen
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 * @param timestamp     Timestamp for the event, in milliseconds
 * @param button        The id of the pointer button generating the event.
 *                      This will be a @ref PicselPointerButton value, a
 *                      touch value in the touch id range, or a manufacturer
 *                      specific value in the custom id range.  If no button
 *                      is pressed, PicselPointerButton_None should be used
 *                      to indicate that the cursor is moving.
 * @param modifierState State of any modifier keys at the time of the
 *                      pointer event.
 *                      See @ref PicselPointerModifierState.
 * @param x             The x location of the pointer
 * @param y             The y location of the pointer
 *
 * @return              The queue status, normally 1. See @ref TgvAsync_Queue.
 */
int PicselPointer_move(Picsel_Context *picselContext,
                       unsigned int    timestamp,
                       int             button,
                       unsigned int    modifierState,
                       int             x,
                       int             y);

/**
 * Notification that the device's pointer button has been released by the
 * user, or that the device's touchscreen has detected a release.
 *
 * The coordinates are based on 0, 0 being the top left corner of the screen,
 * with positive coordinates going right and down. Note that these
 * coordinates are not affected by the orientation of the Alien screen
 *
 * @param picselContext Set by AlienEvent_setPicselContext()
 * @param timestamp     Timestamp for the event, in milliseconds
 * @param button        The id of the pointer button generating the event.
 *                      This will be a @ref PicselPointerButton value or a
 *                      manufacturer-specific value in the custom id range
 * @param modifierState State of any modifier keys at the time of the
 *                      pointer event.
 *                      See @ref PicselPointerModifierState.
 * @param x             The x location of the pointer
 * @param y             The y location of the pointer
 *
 * @return   The queue status, normally 1. See @ref TgvAsync_Queue.
 *
 * @see                 PicselPointer_down(),
 */
int PicselPointer_up(Picsel_Context *picselContext,
                     unsigned int    timestamp,
                     int             button,
                     unsigned int    modifierState,
                     int             x,
                     int             y);

/** @} */ /* End of Doxygen group */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PICSEL_POINTER_H */
