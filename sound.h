#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>

/*
 * Prepare the PC speaker for use.
 * This also makes sure the speaker begins turned off.
 */
void sound_initialize(void);

/*
 * Begin playing a tone at the requested frequency.
 *
 * Examples:
 *     440 Hz = musical note A4
 *     523 Hz = musical note C5
 */
void sound_play_tone(uint32_t frequency);

/*
 * Stop the currently playing tone.
 */
void sound_stop(void);

#endif