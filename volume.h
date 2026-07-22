#ifndef VOLUME_H
#define VOLUME_H

#include "keyboard.h"


#define VOLUME_MIN_LEVEL 0
#define VOLUME_MAX_LEVEL 10


/*
 * Resets the software volume to its starting level.
 */
void volume_initialize(void);


/*
 * Handles global F1 and F2 volume controls.
 */
void volume_handle_key_press(
    keyboard_key_t key
);


/*
 * Reserved for updating or hiding the volume overlay.
 */
void volume_update_overlay(void);


/*
 * Returns the current software volume from 0 through 10.
 */
int volume_get_level(void);


/*
 * Returns 1 when ArcadeOS is muted, otherwise 0.
 */
int volume_is_muted(void);


#endif