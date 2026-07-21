#include "pong.h"
#include "graphics.h"
#include "font.h"
#include "keyboard.h"
#include "timer.h"
#include "sound.h"
#include "music.h"


#define PONG_MODE_ONE_PLAYER 0
#define PONG_MODE_TWO_PLAYER 1
#define PONG_MODE_BACK       2

#define PONG_WINNING_SCORE   7

/*
 * Ball-speed progression.
 *
 * ArcadeOS's timer runs at 100 ticks per second,
 * so 300 ticks is approximately three seconds.
 */
#define PONG_START_BALL_SPEED       8
#define PONG_MAX_BALL_SPEED         15
#define PONG_SPEED_UP_INTERVAL      300

/*
 * Pong draws only the small regions that change directly
 * to video memory. This avoids copying an entire 800-by-600
 * framebuffer during every animation frame.
 */


/*
 * Short, nonblocking PC-speaker effects used by Pong.
 */
typedef struct
{
    uint32_t frequency;
    uint32_t duration_ticks;
} pong_sound_step_t;


static const pong_sound_step_t pong_wall_sound[] =
{
    {280, 2},
    {0,   1}
};


static const pong_sound_step_t pong_paddle_sound[] =
{
    {620, 2},
    {820, 2},
    {0,   1}
};


static const pong_sound_step_t pong_score_sound[] =
{
    {220, 4},
    {330, 4},
    {440, 7},
    {0,   2}
};


static const pong_sound_step_t pong_win_sound[] =
{
    {523, 4},
    {659, 4},
    {784, 5},
    {1047, 10},
    {0,   2}
};


static const pong_sound_step_t *pong_active_sound;
static uint32_t pong_active_sound_length;
static uint32_t pong_active_sound_index;
static uint32_t pong_sound_step_end_tick;
static int pong_sound_is_playing;


/*
 * Sends the current effect step to the PC speaker.
 */
static void pong_output_sound_step(void)
{
    uint32_t frequency =
        pong_active_sound[
            pong_active_sound_index
        ].frequency;

    if (frequency == 0)
    {
        sound_stop();
    }
    else
    {
        sound_play_tone(frequency);
    }
}


/*
 * Starts or replaces the current Pong sound effect.
 */
static void pong_play_sound_effect(
    const pong_sound_step_t *sound,
    uint32_t sound_length
)
{
    if (
        sound == 0 ||
        sound_length == 0
    )
    {
        return;
    }

    pong_active_sound = sound;
    pong_active_sound_length = sound_length;
    pong_active_sound_index = 0;
    pong_sound_is_playing = 1;

    pong_output_sound_step();

    pong_sound_step_end_tick =
        timer_get_ticks() +
        pong_active_sound[0].duration_ticks;
}


/*
 * Advances the active sound effect without pausing Pong.
 */
static void pong_update_sound(void)
{
    uint32_t current_tick;

    if (!pong_sound_is_playing)
    {
        return;
    }

    current_tick =
        timer_get_ticks();

    while (
        (int32_t)(
            current_tick -
            pong_sound_step_end_tick
        ) >= 0
    )
    {
        pong_active_sound_index++;

        if (
            pong_active_sound_index >=
            pong_active_sound_length
        )
        {
            pong_sound_is_playing = 0;
            sound_stop();
            return;
        }

        pong_sound_step_end_tick +=
            pong_active_sound[
                pong_active_sound_index
            ].duration_ticks;

        pong_output_sound_step();
    }
}


/*
 * Stops any effect that is still playing.
 */
static void pong_stop_sound(void)
{
    pong_sound_is_playing = 0;
    pong_active_sound = 0;
    pong_active_sound_length = 0;
    pong_active_sound_index = 0;
    pong_sound_step_end_tick = 0;

    sound_stop();
}


#define PONG_SOUND_LENGTH(sound_array) \
    ((uint32_t)(sizeof(sound_array) / sizeof(sound_array[0])))


static uint32_t pong_text_width(
    const char *text,
    uint32_t scale
)
{
    uint32_t length = 0;

    while (
        text[length] != '\0' &&
        text[length] != '\n'
    )
    {
        length++;
    }

    if (length == 0)
    {
        return 0;
    }

    return (
        length * 6 - 1
    ) * scale;
}


static void pong_draw_centered_text(
    const char *text,
    uint32_t y,
    uint32_t scale,
    uint32_t color
)
{
    uint32_t screen_width =
        graphics_get_width();

    uint32_t text_width =
        pong_text_width(
            text,
            scale
        );

    uint32_t x = 0;

    if (text_width < screen_width)
    {
        x =
            (
                screen_width -
                text_width
            ) / 2;
    }

    font_draw_text(
        text,
        x,
        y,
        scale,
        color
    );
}


static int pong_clamp(
    int value,
    int minimum,
    int maximum
)
{
    if (value < minimum)
    {
        return minimum;
    }

    if (value > maximum)
    {
        return maximum;
    }

    return value;
}


/*
 * Draws the one-player or two-player mode menu.
 */
static void pong_draw_mode_screen(
    int selected_mode
)
{
    uint32_t width =
        graphics_get_width();

    uint32_t height =
        graphics_get_height();


    uint32_t background =
        graphics_rgb(2, 4, 15);

    uint32_t panel =
        graphics_rgb(8, 13, 32);

    uint32_t cyan =
        graphics_rgb(0, 232, 255);

    uint32_t magenta =
        graphics_rgb(255, 35, 200);

    uint32_t purple =
        graphics_rgb(105, 72, 230);

    uint32_t white =
        graphics_rgb(240, 248, 255);

    uint32_t muted =
        graphics_rgb(125, 145, 185);

    uint32_t gold =
        graphics_rgb(255, 197, 55);


    graphics_clear(background);


    /*
     * Outer arcade frame.
     */
    graphics_draw_rectangle(
        20,
        20,
        width - 40,
        height - 40,
        magenta
    );

    graphics_draw_rectangle(
        26,
        26,
        width - 52,
        height - 52,
        cyan
    );

    graphics_draw_rectangle(
        32,
        32,
        width - 64,
        height - 64,
        background
    );


    pong_draw_centered_text(
        "PONG",
        65,
        8,
        white
    );

    pong_draw_centered_text(
        "SELECT GAME MODE",
        145,
        3,
        magenta
    );


    uint32_t card_width =
        280;

    uint32_t card_height =
        120;

    uint32_t card_x =
        (
            width -
            card_width
        ) / 2;

    uint32_t first_y =
        220;

    uint32_t second_y =
        365;


    /*
     * One-player option.
     */
    graphics_draw_rectangle(
        card_x,
        first_y,
        card_width,
        card_height,
        selected_mode ==
            PONG_MODE_ONE_PLAYER
            ? cyan
            : purple
    );

    graphics_draw_rectangle(
        card_x + 5,
        first_y + 5,
        card_width - 10,
        card_height - 10,
        panel
    );


    pong_draw_centered_text(
        "ONE PLAYER",
        first_y + 22,
        4,
        selected_mode ==
            PONG_MODE_ONE_PLAYER
            ? white
            : muted
    );

    pong_draw_centered_text(
        "PLAYER ONE VS CPU",
        first_y + 76,
        2,
        gold
    );


    /*
     * Two-player option.
     */
    graphics_draw_rectangle(
        card_x,
        second_y,
        card_width,
        card_height,
        selected_mode ==
            PONG_MODE_TWO_PLAYER
            ? cyan
            : purple
    );

    graphics_draw_rectangle(
        card_x + 5,
        second_y + 5,
        card_width - 10,
        card_height - 10,
        panel
    );


    pong_draw_centered_text(
        "TWO PLAYER",
        second_y + 22,
        4,
        selected_mode ==
            PONG_MODE_TWO_PLAYER
            ? white
            : muted
    );

    pong_draw_centered_text(
        "LOCAL ARCADE MATCH",
        second_y + 76,
        2,
        gold
    );


    pong_draw_centered_text(
        "UP DOWN TO SELECT",
        height - 75,
        2,
        muted
    );

    pong_draw_centered_text(
        "ENTER TO START   ESC TO RETURN",
        height - 45,
        2,
        white
    );
}


static int pong_select_mode(void)
{
    int selected_mode =
        PONG_MODE_ONE_PLAYER;


    pong_draw_mode_screen(
        selected_mode
    );


    while (1)
    {
        keyboard_update();
        music_update();


        if (
            keyboard_was_key_pressed(KEY_UP) ||
            keyboard_was_key_pressed(KEY_W)
        )
        {
            if (
                selected_mode !=
                PONG_MODE_ONE_PLAYER
            )
            {
                selected_mode =
                    PONG_MODE_ONE_PLAYER;

                music_play_mode_move_sound();

                pong_draw_mode_screen(
                    selected_mode
                );
            }
        }


        if (
            keyboard_was_key_pressed(KEY_DOWN) ||
            keyboard_was_key_pressed(KEY_S)
        )
        {
            if (
                selected_mode !=
                PONG_MODE_TWO_PLAYER
            )
            {
                selected_mode =
                    PONG_MODE_TWO_PLAYER;

                music_play_mode_move_sound();

                pong_draw_mode_screen(
                    selected_mode
                );
            }
        }


        if (
            keyboard_was_key_pressed(
                KEY_ENTER
            )
        )
        {
            /*
             * Confirm the chosen one-player or two-player
             * mode before the match begins.
             */
            music_play_mode_confirm_sound();

            while (music_effect_is_playing())
            {
                music_update();
                __asm__ volatile ("hlt");
            }

            music_stop();

            return selected_mode;
        }


        if (
            keyboard_was_key_pressed(
                KEY_ESCAPE
            )
        )
        {
            music_play_menu_back_sound();

            while (music_effect_is_playing())
            {
                music_update();
                __asm__ volatile ("hlt");
            }

            music_stop();

            return PONG_MODE_BACK;
        }


        __asm__ volatile ("hlt");
    }
}


/*
 * Draws one small score digit.
 */
static void pong_draw_digit(
    unsigned int digit,
    uint32_t x,
    uint32_t y,
    uint32_t scale,
    uint32_t color
)
{
    static const unsigned char digits[10][5] =
    {
        { 7, 5, 5, 5, 7 },
        { 2, 6, 2, 2, 7 },
        { 7, 1, 7, 4, 7 },
        { 7, 1, 7, 1, 7 },
        { 5, 5, 7, 1, 1 },
        { 7, 4, 7, 1, 7 },
        { 7, 4, 7, 5, 7 },
        { 7, 1, 1, 1, 1 },
        { 7, 5, 7, 5, 7 },
        { 7, 5, 7, 1, 7 }
    };


    if (digit > 9)
    {
        return;
    }


    for (
        uint32_t row = 0;
        row < 5;
        row++
    )
    {
        for (
            uint32_t column = 0;
            column < 3;
            column++
        )
        {
            unsigned char bit =
                1U << (2 - column);

            if (
                digits[digit][row] &
                bit
            )
            {
                graphics_draw_rectangle(
                    x + column * scale,
                    y + row * scale,
                    scale,
                    scale,
                    color
                );
            }
        }
    }
}


static void pong_draw_center_line(
    int court_top,
    int court_bottom,
    uint32_t color
)
{
    uint32_t width =
        graphics_get_width();


    for (
        int y = court_top;
        y < court_bottom;
        y += 24
    )
    {
        graphics_draw_rectangle(
            width / 2 - 2,
            (uint32_t)y,
            4,
            12,
            color
        );
    }
}


static void pong_draw_scores(
    unsigned int left_score,
    unsigned int right_score
)
{
    uint32_t width =
        graphics_get_width();


    uint32_t background =
        graphics_rgb(2, 4, 15);

    uint32_t white =
        graphics_rgb(240, 248, 255);


    graphics_draw_rectangle(
        width / 2 - 110,
        42,
        220,
        40,
        background
    );


    pong_draw_digit(
        left_score,
        width / 2 - 70,
        42,
        7,
        white
    );

    pong_draw_digit(
        right_score,
        width / 2 + 48,
        42,
        7,
        white
    );
}


static void pong_draw_court(
    int mode,
    int court_top,
    int court_bottom
)
{
    uint32_t width =
        graphics_get_width();

    uint32_t height =
        graphics_get_height();


    uint32_t background =
        graphics_rgb(2, 4, 15);

    uint32_t panel =
        graphics_rgb(7, 11, 29);

    uint32_t cyan =
        graphics_rgb(0, 232, 255);

    uint32_t magenta =
        graphics_rgb(255, 35, 200);

    uint32_t white =
        graphics_rgb(240, 248, 255);

    uint32_t muted =
        graphics_rgb(75, 95, 140);


    graphics_clear(background);


    /*
     * Permanent outer cabinet frame.
     */
    graphics_draw_rectangle(
        16,
        16,
        width - 32,
        height - 32,
        magenta
    );

    graphics_draw_rectangle(
        22,
        22,
        width - 44,
        height - 44,
        background
    );


    /*
     * Header panel.
     */
    graphics_draw_rectangle(
        34,
        32,
        width - 68,
        64,
        panel
    );

    font_draw_text(
        mode == PONG_MODE_ONE_PLAYER
            ? "CPU"
            : "PLAYER TWO",
        52,
        53,
        2,
        cyan
    );


    const char *right_label =
        "PLAYER ONE";

    uint32_t right_label_width =
        pong_text_width(
            right_label,
            2
        );

    font_draw_text(
        right_label,
        width -
            right_label_width -
            52,
        53,
        2,
        cyan
    );


    pong_draw_centered_text(
        "FIRST TO 7",
        82,
        1,
        muted
    );


    /*
     * Separate cyan court frame.
     */
    graphics_draw_rectangle(
        38,
        (uint32_t)court_top - 4,
        width - 76,
        (uint32_t)(
            court_bottom -
            court_top
        ) + 8,
        cyan
    );

    graphics_draw_rectangle(
        42,
        (uint32_t)court_top,
        width - 84,
        (uint32_t)(
            court_bottom -
            court_top
        ),
        background
    );


    pong_draw_center_line(
        court_top,
        court_bottom,
        muted
    );


    /*
     * Footer control panel.
     */
    graphics_draw_rectangle(
        34,
        height - 60,
        width - 68,
        34,
        panel
    );


    if (mode == PONG_MODE_ONE_PLAYER)
    {
        font_draw_text(
            "ARROWS MOVE",
            50,
            height - 51,
            2,
            white
        );

        const char *escape_text =
            "ESC MODE SELECT";

        uint32_t escape_width =
            pong_text_width(
                escape_text,
                2
            );

        font_draw_text(
            escape_text,
            width -
                escape_width -
                50,
            height - 51,
            2,
            white
        );
    }
    else
    {
        font_draw_text(
            "P2 W S",
            50,
            height - 51,
            2,
            white
        );

        pong_draw_centered_text(
            "P1 ARROWS",
            height - 51,
            2,
            cyan
        );

        const char *escape_text =
            "ESC EXIT";

        uint32_t escape_width =
            pong_text_width(
                escape_text,
                2
            );

        font_draw_text(
            escape_text,
            width -
                escape_width -
                50,
            height - 51,
            2,
            white
        );
    }
}


static int pong_show_winner(
    int mode,
    int right_player_won
)
{
    uint32_t width =
        graphics_get_width();

    uint32_t height =
        graphics_get_height();


    uint32_t panel =
        graphics_rgb(8, 13, 32);

    uint32_t cyan =
        graphics_rgb(0, 232, 255);

    uint32_t magenta =
        graphics_rgb(255, 35, 200);

    uint32_t white =
        graphics_rgb(240, 248, 255);


    graphics_draw_rectangle(
        width / 2 - 245,
        height / 2 - 105,
        490,
        210,
        magenta
    );

    graphics_draw_rectangle(
        width / 2 - 239,
        height / 2 - 99,
        478,
        198,
        panel
    );


    const char *winner;


    if (right_player_won)
    {
        winner =
            "PLAYER ONE WINS";
    }
    else if (
        mode ==
        PONG_MODE_ONE_PLAYER
    )
    {
        winner =
            "CPU WINS";
    }
    else
    {
        winner =
            "PLAYER TWO WINS";
    }


    pong_draw_centered_text(
        winner,
        height / 2 - 55,
        4,
        cyan
    );

    pong_draw_centered_text(
        "ENTER REMATCH",
        height / 2 + 15,
        2,
        white
    );

    pong_draw_centered_text(
        "ESC MODE SELECT",
        height / 2 + 50,
        2,
        white
    );

    /*
     * Pong uses direct rendering, so the winner panel is
     * already visible and no full-screen copy is required.
     */

    while (1)
    {
        keyboard_update();
        pong_update_sound();


        if (
            keyboard_was_key_pressed(
                KEY_ENTER
            )
        )
        {
            pong_stop_sound();
            return 1;
        }


        if (
            keyboard_was_key_pressed(
                KEY_ESCAPE
            )
        )
        {
            pong_stop_sound();

            music_play_menu_back_sound();

            while (music_effect_is_playing())
            {
                music_update();
                __asm__ volatile ("hlt");
            }

            music_stop();

            return 0;
        }


        __asm__ volatile ("hlt");
    }
}


/*
 * Runs one complete Pong match.
 *
 * Returns 1 for a rematch.
 * Returns 0 to return to mode selection.
 */
static int pong_run_match(int mode)
{
    pong_stop_sound();

    int screen_width =
        (int)graphics_get_width();

    int screen_height =
        (int)graphics_get_height();


    /*
     * Clean 4:3 court boundaries.
     */
    const int court_top =
        112;

    const int court_bottom =
        screen_height - 72;

    const int paddle_width =
        12;

    const int paddle_height =
        86;

    const int paddle_speed =
        4;


    /*
     * CPU difficulty settings.
     *
     * The CPU is intentionally not perfect:
     *
     * - It waits until the ball reaches the left half
     *   of the court before predicting the landing point.
     * - It only recalculates that prediction every
     *   cpu_think_interval timer ticks.
     * - Its movement speed is limited.
     *
     * These limits give steep and fast shots a real
     * chance of getting past it.
     */
    const int cpu_base_speed =
        4;

    const int cpu_max_speed =
        7;

    const int cpu_dead_zone =
        4;

    const uint32_t cpu_think_interval =
        10;

    const int ball_size =
        12;

    const int left_paddle_x =
        66;

    const int right_paddle_x =
        screen_width - 78;


    int left_paddle_y =
        (
            court_top +
            court_bottom -
            paddle_height
        ) / 2;

    int right_paddle_y =
        left_paddle_y;


    int ball_x =
        screen_width / 2 -
        ball_size / 2;

    int ball_y =
        (
            court_top +
            court_bottom
        ) / 2 -
        ball_size / 2;


    int ball_speed =
        PONG_START_BALL_SPEED;

    int ball_velocity_x =
        ball_speed;

    int ball_velocity_y =
        2;


    unsigned int left_score = 0;
    unsigned int right_score = 0;


    uint32_t background =
        graphics_rgb(2, 4, 15);

    uint32_t cyan =
        graphics_rgb(0, 232, 255);

    uint32_t white =
        graphics_rgb(240, 248, 255);

    uint32_t muted =
        graphics_rgb(75, 95, 140);


    /*
     * Draw the permanent court directly to the visible
     * framebuffer.
     */
    pong_draw_court(
        mode,
        court_top,
        court_bottom
    );

    pong_draw_scores(
        left_score,
        right_score
    );


    graphics_draw_rectangle(
        (uint32_t)left_paddle_x,
        (uint32_t)left_paddle_y,
        paddle_width,
        paddle_height,
        cyan
    );

    graphics_draw_rectangle(
        (uint32_t)right_paddle_x,
        (uint32_t)right_paddle_y,
        paddle_width,
        paddle_height,
        cyan
    );

    graphics_draw_rectangle(
        (uint32_t)ball_x,
        (uint32_t)ball_y,
        ball_size,
        ball_size,
        white
    );


    /*
     * Keep double buffering disabled during the match.
     * Only the old paddles and ball are erased, then their
     * new positions are drawn directly to video memory.
     */
    graphics_disable_double_buffering();


    uint32_t previous_tick =
        timer_get_ticks();

    uint32_t next_speed_up_tick =
        previous_tick +
        PONG_SPEED_UP_INTERVAL;


    /*
     * The CPU stores one target instead of calculating
     * a perfect new target on every game update.
     */
    uint32_t next_cpu_think_tick =
        previous_tick;

    int cpu_target_y =
        left_paddle_y;


    while (1)
    {
        keyboard_update();
        pong_update_sound();


        if (
            keyboard_was_key_pressed(
                KEY_ESCAPE
            )
        )
        {
            pong_stop_sound();

            music_play_menu_back_sound();

            while (music_effect_is_playing())
            {
                music_update();
                __asm__ volatile ("hlt");
            }

            music_stop();

            return 0;
        }


        uint32_t current_tick =
            timer_get_ticks();


        /*
         * Update once per timer tick.
         *
         * The PIT runs at 100 Hz, giving Pong up to
         * 100 updates per second instead of 50.
         */
        if (current_tick == previous_tick)
        {
            __asm__ volatile ("hlt");
            continue;
        }


        previous_tick =
            current_tick;


        /*
         * Increase horizontal speed every three seconds.
         */
        if (
            current_tick >=
            next_speed_up_tick
        )
        {
            if (
                ball_speed <
                PONG_MAX_BALL_SPEED
            )
            {
                ball_speed++;

                if (ball_velocity_x < 0)
                {
                    ball_velocity_x =
                        -ball_speed;
                }
                else
                {
                    ball_velocity_x =
                        ball_speed;
                }
            }

            next_speed_up_tick +=
                PONG_SPEED_UP_INTERVAL;
        }


        int old_left_y =
            left_paddle_y;

        int old_right_y =
            right_paddle_y;

        int old_ball_x =
            ball_x;

        int old_ball_y =
            ball_y;


        /*
         * Player One controls the right paddle.
         */
        if (
            keyboard_is_key_down(
                KEY_UP
            )
        )
        {
            right_paddle_y -=
                paddle_speed;
        }

        if (
            keyboard_is_key_down(
                KEY_DOWN
            )
        )
        {
            right_paddle_y +=
                paddle_speed;
        }


        /*
         * Player Two or CPU controls the left paddle.
         */
        if (
            mode ==
            PONG_MODE_TWO_PLAYER
        )
        {
            if (
                keyboard_is_key_down(
                    KEY_W
                )
            )
            {
                left_paddle_y -=
                    paddle_speed;
            }

            if (
                keyboard_is_key_down(
                    KEY_S
                )
            )
            {
                left_paddle_y +=
                    paddle_speed;
            }
        }
        else
        {
            /*
             * Recalculate the CPU's target only after
             * its reaction interval has elapsed.
             */
            if (
                (int32_t)(
                    current_tick -
                    next_cpu_think_tick
                ) >= 0
            )
            {
                /*
                 * The CPU does not begin predicting until
                 * the ball is moving left and has crossed
                 * into the left half of the court.
                 *
                 * This delay is the CPU's main weakness.
                 */
                if (
                    ball_velocity_x < 0 &&
                    ball_x <=
                        screen_width / 2
                )
                {
                    int horizontal_distance =
                        ball_x -
                        (
                            left_paddle_x +
                            paddle_width
                        );

                    int ticks_until_arrival;

                    int predicted_ball_y;

                    int minimum_ball_y =
                        court_top;

                    int maximum_ball_y =
                        court_bottom -
                        ball_size;


                    if (horizontal_distance < 0)
                    {
                        horizontal_distance = 0;
                    }


                    /*
                     * Estimate how many game updates remain
                     * before the ball reaches the CPU.
                     */
                    ticks_until_arrival =
                        horizontal_distance /
                        (-ball_velocity_x);


                    /*
                     * Predict the ball's vertical position.
                     */
                    predicted_ball_y =
                        ball_y +
                        ball_velocity_y *
                        ticks_until_arrival;


                    /*
                     * Reflect the predicted position when it
                     * passes through the ceiling or floor.
                     *
                     * This also handles multiple bounces.
                     */
                    while (
                        predicted_ball_y <
                            minimum_ball_y ||
                        predicted_ball_y >
                            maximum_ball_y
                    )
                    {
                        if (
                            predicted_ball_y <
                            minimum_ball_y
                        )
                        {
                            predicted_ball_y =
                                minimum_ball_y +
                                (
                                    minimum_ball_y -
                                    predicted_ball_y
                                );
                        }

                        if (
                            predicted_ball_y >
                            maximum_ball_y
                        )
                        {
                            predicted_ball_y =
                                maximum_ball_y -
                                (
                                    predicted_ball_y -
                                    maximum_ball_y
                                );
                        }
                    }


                    /*
                     * Center the CPU paddle on the predicted
                     * position of the ball.
                     */
                    cpu_target_y =
                        predicted_ball_y +
                        ball_size / 2 -
                        paddle_height / 2;
                }
                else
                {
                    /*
                     * While the ball is moving away, or has
                     * not crossed the center yet, the CPU
                     * returns toward the center.
                     */
                    cpu_target_y =
                        (
                            court_top +
                            court_bottom -
                            paddle_height
                        ) / 2;
                }


                /*
                 * Keep the saved target inside the court.
                 */
                cpu_target_y =
                    pong_clamp(
                        cpu_target_y,
                        court_top,
                        court_bottom -
                            paddle_height
                    );


                /*
                 * The CPU must wait before thinking again.
                 *
                 * At 100 timer ticks per second, 10 ticks
                 * is about one tenth of a second.
                 */
                next_cpu_think_tick =
                    current_tick +
                    cpu_think_interval;
            }


            /*
             * The CPU becomes somewhat faster as the ball
             * accelerates, but it still has a strict limit.
             */
            int cpu_move_speed =
                cpu_base_speed +
                (
                    ball_speed -
                    PONG_START_BALL_SPEED
                ) / 2;


            if (
                cpu_move_speed >
                cpu_max_speed
            )
            {
                cpu_move_speed =
                    cpu_max_speed;
            }


            /*
             * Move toward the most recently calculated
             * target. The dead zone prevents tiny jitter.
             */
            if (
                left_paddle_y <
                cpu_target_y -
                cpu_dead_zone
            )
            {
                left_paddle_y +=
                    cpu_move_speed;

                if (
                    left_paddle_y >
                    cpu_target_y
                )
                {
                    left_paddle_y =
                        cpu_target_y;
                }
            }
            else if (
                left_paddle_y >
                cpu_target_y +
                cpu_dead_zone
            )
            {
                left_paddle_y -=
                    cpu_move_speed;

                if (
                    left_paddle_y <
                    cpu_target_y
                )
                {
                    left_paddle_y =
                        cpu_target_y;
                }
            }
        }


        left_paddle_y =
            pong_clamp(
                left_paddle_y,
                court_top,
                court_bottom -
                    paddle_height
            );

        right_paddle_y =
            pong_clamp(
                right_paddle_y,
                court_top,
                court_bottom -
                    paddle_height
            );


        ball_x +=
            ball_velocity_x;

        ball_y +=
            ball_velocity_y;


        /*
         * Top wall.
         */
        if (ball_y <= court_top)
        {
            ball_y =
                court_top;

            ball_velocity_y =
                -ball_velocity_y;

            pong_play_sound_effect(
                pong_wall_sound,
                PONG_SOUND_LENGTH(
                    pong_wall_sound
                )
            );
        }


        /*
         * Bottom wall.
         */
        if (
            ball_y + ball_size >=
            court_bottom
        )
        {
            ball_y =
                court_bottom -
                ball_size;

            ball_velocity_y =
                -ball_velocity_y;

            pong_play_sound_effect(
                pong_wall_sound,
                PONG_SOUND_LENGTH(
                    pong_wall_sound
                )
            );
        }


        /*
         * Left paddle collision.
         */
        if (
            ball_velocity_x < 0 &&

            ball_x <=
                left_paddle_x +
                paddle_width &&

            ball_x + ball_size >=
                left_paddle_x &&

            ball_y + ball_size >=
                left_paddle_y &&

            ball_y <=
                left_paddle_y +
                paddle_height
        )
        {
            ball_x =
                left_paddle_x +
                paddle_width;

            ball_velocity_x =
                ball_speed;


            int impact =
                (
                    ball_y +
                    ball_size / 2
                ) -
                (
                    left_paddle_y +
                    paddle_height / 2
                );


            ball_velocity_y =
                pong_clamp(
                    impact / 7,
                    -6,
                    6
                );

            if (ball_velocity_y == 0)
            {
                ball_velocity_y = 1;
            }

            pong_play_sound_effect(
                pong_paddle_sound,
                PONG_SOUND_LENGTH(
                    pong_paddle_sound
                )
            );
        }


        /*
         * Right paddle collision.
         */
        if (
            ball_velocity_x > 0 &&

            ball_x + ball_size >=
                right_paddle_x &&

            ball_x <=
                right_paddle_x +
                paddle_width &&

            ball_y + ball_size >=
                right_paddle_y &&

            ball_y <=
                right_paddle_y +
                paddle_height
        )
        {
            ball_x =
                right_paddle_x -
                ball_size;

            ball_velocity_x =
                -ball_speed;


            int impact =
                (
                    ball_y +
                    ball_size / 2
                ) -
                (
                    right_paddle_y +
                    paddle_height / 2
                );


            ball_velocity_y =
                pong_clamp(
                    impact / 7,
                    -6,
                    6
                );

            if (ball_velocity_y == 0)
            {
                ball_velocity_y = -1;
            }

            pong_play_sound_effect(
                pong_paddle_sound,
                PONG_SOUND_LENGTH(
                    pong_paddle_sound
                )
            );
        }


        int scored = 0;
        int serve_direction = 0;


        /*
         * Player One scores.
         */
        if (ball_x + ball_size < 42)
        {
            right_score++;
            scored = 1;
            serve_direction = -1;
        }


        /*
         * CPU or Player Two scores.
         */
        if (ball_x > screen_width - 42)
        {
            left_score++;
            scored = 1;
            serve_direction = 1;
        }


        /*
         * Remove old objects from the hidden frame.
         */
        graphics_draw_rectangle(
            (uint32_t)left_paddle_x,
            (uint32_t)old_left_y,
            paddle_width,
            paddle_height,
            background
        );

        graphics_draw_rectangle(
            (uint32_t)right_paddle_x,
            (uint32_t)old_right_y,
            paddle_width,
            paddle_height,
            background
        );

        graphics_draw_rectangle(
            (uint32_t)old_ball_x,
            (uint32_t)old_ball_y,
            ball_size,
            ball_size,
            background
        );


        /*
         * Restore the center line in case the old ball
         * erased part of it.
         */
        pong_draw_center_line(
            court_top,
            court_bottom,
            muted
        );


        if (scored)
        {
            /*
             * Every point begins at the original ball speed.
             * The speed-up countdown also restarts here.
             */
            ball_speed =
                PONG_START_BALL_SPEED;

            ball_velocity_x =
                serve_direction *
                ball_speed;

            next_speed_up_tick =
                current_tick +
                PONG_SPEED_UP_INTERVAL;


            /*
             * Start the CPU from a neutral target after
             * every point instead of preserving an old
             * prediction from the previous rally.
             */
            cpu_target_y =
                (
                    court_top +
                    court_bottom -
                    paddle_height
                ) / 2;

            next_cpu_think_tick =
                current_tick +
                cpu_think_interval;


            ball_x =
                screen_width / 2 -
                ball_size / 2;

            ball_y =
                (
                    court_top +
                    court_bottom
                ) / 2 -
                ball_size / 2;


            ball_velocity_y =
                (
                    left_score +
                    right_score
                ) % 2
                ? 2
                : -2;


            pong_draw_scores(
                left_score,
                right_score
            );


            if (
                right_score >=
                    PONG_WINNING_SCORE ||
                left_score >=
                    PONG_WINNING_SCORE
            )
            {
                pong_play_sound_effect(
                    pong_win_sound,
                    PONG_SOUND_LENGTH(
                        pong_win_sound
                    )
                );
            }
            else
            {
                pong_play_sound_effect(
                    pong_score_sound,
                    PONG_SOUND_LENGTH(
                        pong_score_sound
                    )
                );
            }
        }


        /*
         * Draw the updated objects into the hidden
         * frame.
         */
        graphics_draw_rectangle(
            (uint32_t)left_paddle_x,
            (uint32_t)left_paddle_y,
            paddle_width,
            paddle_height,
            cyan
        );

        graphics_draw_rectangle(
            (uint32_t)right_paddle_x,
            (uint32_t)right_paddle_y,
            paddle_width,
            paddle_height,
            cyan
        );

        graphics_draw_rectangle(
            (uint32_t)ball_x,
            (uint32_t)ball_y,
            ball_size,
            ball_size,
            white
        );


        /*
         * The updated paddles and ball were written directly
         * to the physical framebuffer above. No full-screen
         * graphics_present() copy is needed here.
         */


        if (
            right_score >=
            PONG_WINNING_SCORE
        )
        {
            int rematch =
                pong_show_winner(
                    mode,
                    1
                );

            return rematch;
        }


        if (
            left_score >=
            PONG_WINNING_SCORE
        )
        {
            int rematch =
                pong_show_winner(
                    mode,
                    0
                );

            return rematch;
        }
    }
}

void pong_run(void)
{
    while (1)
    {
        int mode =
            pong_select_mode();


        if (mode == PONG_MODE_BACK)
        {
            pong_stop_sound();
            return;
        }


        /*
         * A return value of 1 requests a rematch.
         * A return value of 0 returns to mode select.
         */
        while (pong_run_match(mode))
        {
        }
    }
}