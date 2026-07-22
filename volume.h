#ifndef VOLUME_H
#define VOLUME_H

#include "keyboard.h"

/*
 * ArcadeOS exposes ten software volume levels.
 *
 * The current PC-speaker driver has no hardware amplitude control, so
 * level zero provides real mute while levels one through ten provide the
 * operating-system volume state and on-screen display. A future audio
 * driver can use the same level without changing the user interface.
 */
#define VOLUME_MIN_LEVEL 0
#define VOLUME_MAX_LEVEL 10

void volume_initialize(void);
void volume_handle_key_press(keyboard_key_t key);
void volume_update_overlay(void);

int volume_get_level(void);
int volume_is_muted(void);

#endif
