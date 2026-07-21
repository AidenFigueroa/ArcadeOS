#ifndef MUSIC_H
#define MUSIC_H

/*
 * Start the looping ArcadeOS title-screen theme.
 */
void music_play_title_theme(void);

/*
 * Start the slower game-selection theme.
 */
void music_play_game_select_theme(void);

/*
 * Play a short sound when the highlighted game changes.
 */
void music_play_menu_move_sound(void);

/*
 * Play a short sound when the highlighted Pong mode changes.
 */
void music_play_mode_move_sound(void);

/*
 * Play a confirmation sound after selecting a Pong mode.
 */
void music_play_mode_confirm_sound(void);

/*
 * Play a short descending sound when Escape goes back.
 */
void music_play_menu_back_sound(void);

/*
 * Play a short launch sound when entering a game.
 */
void music_play_game_enter_sound(void);

/*
 * Return nonzero while a temporary interface sound plays.
 */
int music_effect_is_playing(void);

/*
 * Advance the active melody and interface sound.
 */
void music_update(void);

/*
 * Stop all music and silence the PC speaker.
 */
void music_stop(void);

#endif