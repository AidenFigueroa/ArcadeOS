#include "music.h"
#include "sound.h"
#include "timer.h"

#include <stdint.h>


/*
 * One entry in a song.
 *
 * frequency:
 *     Musical pitch in hertz.
 *     Zero means silence.
 *
 * duration_ticks:
 *     How long the note lasts according to the OS timer.
 */
typedef struct
{
    uint32_t frequency;
    uint32_t duration_ticks;
} music_note_t;


/*
 * Musical note frequencies.
 */
#define NOTE_REST 0U

#define NOTE_G3   196U
#define NOTE_A3   220U
#define NOTE_B3   247U

#define NOTE_C4   262U
#define NOTE_D4   294U
#define NOTE_E4   330U
#define NOTE_FS4  370U
#define NOTE_G4   392U
#define NOTE_A4   440U
#define NOTE_B4   494U

#define NOTE_C5   523U
#define NOTE_D5   587U
#define NOTE_E5   659U
#define NOTE_FS5  740U
#define NOTE_G5   784U


/*
 * The ArcadeOS timer currently runs at 100 Hz.
 */
#define SIXTEENTH_NOTE 9U
#define EIGHTH_NOTE    18U
#define QUARTER_NOTE   38U
#define NOTE_GAP       2U


/*
 * ArcadeOS title theme.
 */
static const music_note_t title_theme[] =
{
    /*
     * Opening phrase.
     */
    {NOTE_E4,   EIGHTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_G4,   EIGHTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_B4,   EIGHTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_E5,   QUARTER_NOTE},
    {NOTE_REST, NOTE_GAP},


    /*
     * Answering phrase.
     */
    {NOTE_D5,   EIGHTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_B4,   EIGHTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_G4,   EIGHTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_A4,   QUARTER_NOTE},
    {NOTE_REST, NOTE_GAP},


    /*
     * Rising phrase.
     */
    {NOTE_B4,   EIGHTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_D5,   EIGHTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_E5,   EIGHTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_G5,   QUARTER_NOTE},
    {NOTE_REST, NOTE_GAP},


    /*
     * Falling phrase.
     */
    {NOTE_FS5,  EIGHTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_E5,   EIGHTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_D5,   EIGHTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_B4,   QUARTER_NOTE},
    {NOTE_REST, NOTE_GAP},


    /*
     * Faster arcade section.
     */
    {NOTE_E4,   SIXTEENTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_G4,   SIXTEENTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_A4,   SIXTEENTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_B4,   SIXTEENTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_D5,   SIXTEENTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_E5,   SIXTEENTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_D5,   SIXTEENTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_B4,   SIXTEENTH_NOTE},
    {NOTE_REST, NOTE_GAP},


    /*
     * Second fast phrase.
     */
    {NOTE_G4,   SIXTEENTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_A4,   SIXTEENTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_B4,   SIXTEENTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_D5,   SIXTEENTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_E5,   SIXTEENTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_FS5,  SIXTEENTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_G5,   EIGHTH_NOTE},
    {NOTE_REST, NOTE_GAP},


    /*
     * Ending phrase.
     */
    {NOTE_E5,   EIGHTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_D5,   EIGHTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_B4,   EIGHTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_A4,   EIGHTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_G4,   EIGHTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_A4,   EIGHTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_B4,   EIGHTH_NOTE},
    {NOTE_REST, NOTE_GAP},

    {NOTE_E5,   QUARTER_NOTE},
    {NOTE_REST, 18U}
};


/*
 * Slower game-selection theme.
 *
 * The PC speaker does not provide real volume control,
 * so this theme sounds softer by using lower pitches,
 * longer notes, and more silence between phrases.
 */
static const music_note_t game_select_theme[] =
{
    /*
     * Original neutral menu phrase.
     */
    {NOTE_E4,   10U},
    {NOTE_REST, 16U},

    {NOTE_D4,   10U},
    {NOTE_REST, 16U},

    {NOTE_B3,   14U},
    {NOTE_REST, 34U},


    /*
     * Original lower answering phrase.
     */
    {NOTE_D4,   10U},
    {NOTE_REST, 16U},

    {NOTE_C4,   10U},
    {NOTE_REST, 16U},

    {NOTE_A3,   14U},
    {NOTE_REST, 34U},


    /*
     * Original low ending phrase.
     */
    {NOTE_C4,   10U},
    {NOTE_REST, 16U},

    {NOTE_B3,   10U},
    {NOTE_REST, 16U},

    {NOTE_G3,   18U},
    {NOTE_REST, 34U},


    /*
     * Continuation one.
     *
     * This stays in the same restrained descending style
     * instead of turning into a new, more energetic song.
     */
    {NOTE_E4,   10U},
    {NOTE_REST, 16U},

    {NOTE_C4,   10U},
    {NOTE_REST, 16U},

    {NOTE_A3,   14U},
    {NOTE_REST, 34U},


    /*
     * Continuation two.
     */
    {NOTE_D4,   10U},
    {NOTE_REST, 16U},

    {NOTE_B3,   10U},
    {NOTE_REST, 16U},

    {NOTE_G3,   18U},
    {NOTE_REST, 34U},


    /*
     * Brief middle variation.
     */
    {NOTE_C4,   10U},
    {NOTE_REST, 16U},

    {NOTE_D4,   10U},
    {NOTE_REST, 16U},

    {NOTE_B3,   14U},
    {NOTE_REST, 34U},


    /*
     * Return to the lower range.
     */
    {NOTE_B3,   10U},
    {NOTE_REST, 16U},

    {NOTE_A3,   10U},
    {NOTE_REST, 16U},

    {NOTE_G3,   18U},
    {NOTE_REST, 34U},


    /*
     * Final phrase leading naturally back to the opening.
     */
    {NOTE_D4,   10U},
    {NOTE_REST, 16U},

    {NOTE_C4,   10U},
    {NOTE_REST, 16U},

    {NOTE_A3,   14U},
    {NOTE_REST, 34U},

    {NOTE_E4,   10U},
    {NOTE_REST, 16U},

    {NOTE_D4,   10U},
    {NOTE_REST, 16U},

    {NOTE_B3,   18U},
    {NOTE_REST, 52U}
};


/*
 * Short interface sounds.
 *
 * These temporarily take control of the PC speaker and
 * then return control to whichever menu melody was active.
 */
static const music_note_t menu_move_sound[] =
{
    {NOTE_C5,   2U},
    {NOTE_E5,   2U},
    {NOTE_REST, 1U}
};


static const music_note_t mode_move_sound[] =
{
    {NOTE_B4,   2U},
    {NOTE_D5,   2U},
    {NOTE_REST, 1U}
};


static const music_note_t mode_confirm_sound[] =
{
    {NOTE_G4,   3U},
    {NOTE_C5,   5U},
    {NOTE_REST, 2U}
};


static const music_note_t menu_back_sound[] =
{
    {NOTE_C5,   3U},
    {NOTE_G4,   5U},
    {NOTE_REST, 2U}
};


static const music_note_t game_enter_sound[] =
{
    {NOTE_E4,   3U},
    {NOTE_B4,   3U},
    {NOTE_E5,   7U},
    {NOTE_REST, 2U}
};


#define TITLE_THEME_NOTE_COUNT \
    ((uint32_t)(sizeof(title_theme) / sizeof(title_theme[0])))

#define GAME_SELECT_THEME_NOTE_COUNT \
    ((uint32_t)(sizeof(game_select_theme) / sizeof(game_select_theme[0])))

#define MENU_MOVE_SOUND_NOTE_COUNT \
    ((uint32_t)(sizeof(menu_move_sound) / sizeof(menu_move_sound[0])))

#define MODE_MOVE_SOUND_NOTE_COUNT \
    ((uint32_t)(sizeof(mode_move_sound) / sizeof(mode_move_sound[0])))

#define MODE_CONFIRM_SOUND_NOTE_COUNT \
    ((uint32_t)(sizeof(mode_confirm_sound) / sizeof(mode_confirm_sound[0])))

#define MENU_BACK_SOUND_NOTE_COUNT \
    ((uint32_t)(sizeof(menu_back_sound) / sizeof(menu_back_sound[0])))

#define GAME_ENTER_SOUND_NOTE_COUNT \
    ((uint32_t)(sizeof(game_enter_sound) / sizeof(game_enter_sound[0])))


/*
 * Current looping-theme state.
 */
static const music_note_t *active_theme;
static uint32_t active_theme_note_count;
static uint32_t current_note_index;
static uint32_t current_note_end_tick;
static int music_is_playing;


/*
 * Current temporary interface-sound state.
 */
static const music_note_t *active_effect;
static uint32_t active_effect_note_count;
static uint32_t current_effect_note_index;
static uint32_t current_effect_note_end_tick;
static int effect_is_playing;


/*
 * Sends the active theme note to the PC speaker.
 */
static void music_output_current_theme_note(void)
{
    const music_note_t *note;

    if (
        !music_is_playing ||
        active_theme == 0 ||
        active_theme_note_count == 0
    )
    {
        sound_stop();
        return;
    }

    note =
        &active_theme[current_note_index];

    if (note->frequency == NOTE_REST)
    {
        sound_stop();
    }
    else
    {
        sound_play_tone(
            note->frequency
        );
    }
}


/*
 * Sends the active interface-sound note to the speaker.
 */
static void music_output_current_effect_note(void)
{
    const music_note_t *note;

    if (
        !effect_is_playing ||
        active_effect == 0 ||
        active_effect_note_count == 0
    )
    {
        return;
    }

    note =
        &active_effect[
            current_effect_note_index
        ];

    if (note->frequency == NOTE_REST)
    {
        sound_stop();
    }
    else
    {
        sound_play_tone(
            note->frequency
        );
    }
}


/*
 * Starts one looping theme.
 */
static void music_start_theme(
    const music_note_t *theme,
    uint32_t theme_note_count
)
{
    sound_stop();

    active_effect = 0;
    active_effect_note_count = 0;
    current_effect_note_index = 0;
    current_effect_note_end_tick = 0;
    effect_is_playing = 0;

    if (
        theme == 0 ||
        theme_note_count == 0
    )
    {
        active_theme = 0;
        active_theme_note_count = 0;
        current_note_index = 0;
        current_note_end_tick = 0;
        music_is_playing = 0;
        return;
    }

    active_theme =
        theme;

    active_theme_note_count =
        theme_note_count;

    current_note_index =
        0;

    music_is_playing =
        1;

    music_output_current_theme_note();

    current_note_end_tick =
        timer_get_ticks() +
        active_theme[
            current_note_index
        ].duration_ticks;
}


/*
 * Starts one temporary interface sound.
 *
 * A new effect replaces any interface effect that was
 * already playing. The looping theme keeps advancing in
 * the background and resumes when the effect finishes.
 */
static void music_start_effect(
    const music_note_t *effect,
    uint32_t effect_note_count
)
{
    if (
        effect == 0 ||
        effect_note_count == 0
    )
    {
        return;
    }

    active_effect =
        effect;

    active_effect_note_count =
        effect_note_count;

    current_effect_note_index =
        0;

    effect_is_playing =
        1;

    music_output_current_effect_note();

    current_effect_note_end_tick =
        timer_get_ticks() +
        active_effect[0].duration_ticks;
}


/*
 * Starts the ArcadeOS title-screen theme.
 */
void music_play_title_theme(void)
{
    music_start_theme(
        title_theme,
        TITLE_THEME_NOTE_COUNT
    );
}


/*
 * Starts the calmer game-selection theme.
 */
void music_play_game_select_theme(void)
{
    music_start_theme(
        game_select_theme,
        GAME_SELECT_THEME_NOTE_COUNT
    );
}


/*
 * Plays when the highlighted game changes.
 */
void music_play_menu_move_sound(void)
{
    music_start_effect(
        menu_move_sound,
        MENU_MOVE_SOUND_NOTE_COUNT
    );
}


/*
 * Plays when the highlighted Pong mode changes.
 */
void music_play_mode_move_sound(void)
{
    music_start_effect(
        mode_move_sound,
        MODE_MOVE_SOUND_NOTE_COUNT
    );
}


/*
 * Plays after confirming one-player or two-player mode.
 */
void music_play_mode_confirm_sound(void)
{
    music_start_effect(
        mode_confirm_sound,
        MODE_CONFIRM_SOUND_NOTE_COUNT
    );
}


/*
 * Plays when Escape returns to the previous screen.
 */
void music_play_menu_back_sound(void)
{
    music_start_effect(
        menu_back_sound,
        MENU_BACK_SOUND_NOTE_COUNT
    );
}


/*
 * Plays when Enter launches a game from the selector.
 */
void music_play_game_enter_sound(void)
{
    music_start_effect(
        game_enter_sound,
        GAME_ENTER_SOUND_NOTE_COUNT
    );
}


/*
 * Reports whether a temporary interface sound is active.
 */
int music_effect_is_playing(void)
{
    return effect_is_playing;
}


/*
 * Advances the looping theme and temporary interface sound.
 */
void music_update(void)
{
    uint32_t current_tick =
        timer_get_ticks();

    int theme_changed =
        0;

    int effect_changed =
        0;

    int effect_finished =
        0;


    /*
     * Keep the looping melody's timeline moving even while
     * an interface sound temporarily owns the speaker.
     */
    if (
        music_is_playing &&
        active_theme != 0 &&
        active_theme_note_count != 0
    )
    {
        while (
            (int32_t)(
                current_tick -
                current_note_end_tick
            ) >= 0
        )
        {
            current_note_index++;

            if (
                current_note_index >=
                active_theme_note_count
            )
            {
                current_note_index =
                    0;
            }

            current_note_end_tick +=
                active_theme[
                    current_note_index
                ].duration_ticks;

            theme_changed =
                1;
        }
    }


    /*
     * Temporary interface sounds have priority over music.
     */
    if (
        effect_is_playing &&
        active_effect != 0 &&
        active_effect_note_count != 0
    )
    {
        while (
            effect_is_playing &&
            (int32_t)(
                current_tick -
                current_effect_note_end_tick
            ) >= 0
        )
        {
            current_effect_note_index++;

            if (
                current_effect_note_index >=
                active_effect_note_count
            )
            {
                effect_is_playing =
                    0;

                active_effect =
                    0;

                active_effect_note_count =
                    0;

                current_effect_note_index =
                    0;

                current_effect_note_end_tick =
                    0;

                effect_finished =
                    1;

                break;
            }

            current_effect_note_end_tick +=
                active_effect[
                    current_effect_note_index
                ].duration_ticks;

            effect_changed =
                1;
        }
    }


    if (effect_is_playing)
    {
        if (effect_changed)
        {
            music_output_current_effect_note();
        }

        return;
    }


    /*
     * Resume the current melody note after an effect ends.
     */
    if (
        effect_finished ||
        theme_changed
    )
    {
        music_output_current_theme_note();
    }
}


/*
 * Stops all music and temporary sounds.
 */
void music_stop(void)
{
    music_is_playing =
        0;

    active_theme =
        0;

    active_theme_note_count =
        0;

    current_note_index =
        0;

    current_note_end_tick =
        0;

    effect_is_playing =
        0;

    active_effect =
        0;

    active_effect_note_count =
        0;

    current_effect_note_index =
        0;

    current_effect_note_end_tick =
        0;

    sound_stop();
}