#include "title_screen.h"
#include "graphics.h"
#include "font.h"
#include "keyboard.h"
#include "timer.h"
#include "music.h"

#include <stdint.h>


#define FONT_CHARACTER_WIDTH 5
#define FONT_CHARACTER_GAP   1


/*
 * Calculates the approximate width of one line of text.
 */
static uint32_t title_text_width(
    const char *text,
    uint32_t scale
)
{
    uint32_t length;

    length = 0;

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
        length *
        (
            FONT_CHARACTER_WIDTH +
            FONT_CHARACTER_GAP
        ) -
        FONT_CHARACTER_GAP
    ) * scale;
}


/*
 * Returns the x coordinate needed to center text.
 */
static uint32_t title_center_text(
    const char *text,
    uint32_t scale
)
{
    uint32_t screen_width;
    uint32_t text_width;

    screen_width =
        graphics_get_width();

    text_width =
        title_text_width(
            text,
            scale
        );

    if (text_width >= screen_width)
    {
        return 0;
    }

    return (
        screen_width -
        text_width
    ) / 2;
}


/*
 * Draws the permanent parts of the title screen.
 */
static void title_draw_cabinet(void)
{
    uint32_t width;
    uint32_t height;

    uint32_t background;
    uint32_t cabinet;
    uint32_t cabinet_dark;
    uint32_t magenta;
    uint32_t cyan;
    uint32_t purple;
    uint32_t gold;
    uint32_t scanline;
    uint32_t white;

    uint32_t marquee_x;
    uint32_t marquee_y;
    uint32_t marquee_width;
    uint32_t marquee_height;

    uint32_t deck_x;
    uint32_t deck_y;
    uint32_t deck_width;
    uint32_t deck_height;


    width =
        graphics_get_width();

    height =
        graphics_get_height();


    /*
     * Modern-retro color palette.
     */
    background =
        graphics_rgb(2, 4, 16);

    cabinet =
        graphics_rgb(10, 13, 34);

    cabinet_dark =
        graphics_rgb(5, 7, 20);

    magenta =
        graphics_rgb(255, 30, 210);

    cyan =
        graphics_rgb(0, 235, 255);

    purple =
        graphics_rgb(105, 40, 210);

    gold =
        graphics_rgb(255, 190, 45);

    scanline =
        graphics_rgb(3, 5, 18);

    white =
        graphics_rgb(245, 250, 255);


    /*
     * Clear the entire display.
     */
    graphics_clear(background);


    /*
     * Cabinet exterior.
     */
    graphics_draw_rectangle(
        18,
        18,
        width - 36,
        height - 36,
        magenta
    );

    graphics_draw_rectangle(
        24,
        24,
        width - 48,
        height - 48,
        cyan
    );

    graphics_draw_rectangle(
        30,
        30,
        width - 60,
        height - 60,
        cabinet
    );


    /*
     * Vertical neon cabinet rails.
     */
    graphics_draw_rectangle(
        42,
        52,
        6,
        height - 104,
        purple
    );

    graphics_draw_rectangle(
        width - 48,
        52,
        6,
        height - 104,
        purple
    );


    /*
     * Retro scanline effect.
     */
    for (
        uint32_t y = 34;
        y < height - 34;
        y += 4
    )
    {
        graphics_draw_rectangle(
            48,
            y,
            width - 96,
            1,
            scanline
        );
    }


    /*
     * Top marquee measurements.
     */
    marquee_x =
        width / 10;

    marquee_y =
        height / 10;

    marquee_width =
        width * 8 / 10;

    marquee_height =
        height / 4;


    /*
     * Top marquee frame.
     */
    graphics_draw_rectangle(
        marquee_x,
        marquee_y,
        marquee_width,
        marquee_height,
        magenta
    );

    graphics_draw_rectangle(
        marquee_x + 6,
        marquee_y + 6,
        marquee_width - 12,
        marquee_height - 12,
        cyan
    );

    graphics_draw_rectangle(
        marquee_x + 12,
        marquee_y + 12,
        marquee_width - 24,
        marquee_height - 24,
        cabinet_dark
    );


    /*
     * Main ArcadeOS logo.
     */
    font_draw_text(
        "THE ARCADE",
        title_center_text(
            "THE ARCADE",
            7
        ),
        marquee_y + 35,
        7,
        cyan
    );


    /*
     * Subtitle.
     */
    font_draw_text(
        "YOUR ARCADE ADVENTURE STARTS HERE",
        title_center_text(
            "YOUR ARCADE ADVENTURE STARTS HERE",
            2
        ),
        marquee_y +
            marquee_height -
            38,
        2,
        white
    );


    /*
     * Lower control-deck measurements.
     */
    deck_x =
        width / 4;

    deck_y =
        height * 62 / 100;

    deck_width =
        width / 2;

    deck_height =
        height / 4;


    /*
     * Lower control-deck panel.
     */
    graphics_draw_rectangle(
        deck_x,
        deck_y,
        deck_width,
        deck_height,
        purple
    );

    graphics_draw_rectangle(
        deck_x + 6,
        deck_y + 6,
        deck_width - 12,
        deck_height - 12,
        cabinet_dark
    );


    /*
     * Coin-slot housing.
     */
    graphics_draw_rectangle(
        width / 2 - 44,
        deck_y + 22,
        88,
        28,
        gold
    );


    /*
     * Dark opening in the coin slot.
     */
    graphics_draw_rectangle(
        width / 2 - 32,
        deck_y + 31,
        64,
        10,
        background
    );
}


/*
 * Draws or hides the blinking coin prompt.
 */
static void title_draw_coin_prompt(int visible)
{
    uint32_t width;
    uint32_t height;

    uint32_t cabinet_dark;
    uint32_t gold;
    uint32_t white;

    uint32_t prompt_y;


    width =
        graphics_get_width();

    height =
        graphics_get_height();

    cabinet_dark =
        graphics_rgb(5, 7, 20);

    gold =
        graphics_rgb(255, 190, 45);

    white =
        graphics_rgb(245, 250, 255);

    prompt_y =
        height * 62 / 100 + 65;


    /*
     * Clear the prompt area before redrawing it.
     */
    graphics_draw_rectangle(
        width / 4 + 10,
        prompt_y,
        width / 2 - 20,
        82,
        cabinet_dark
    );


    /*
     * Keep the area empty when the prompt is hidden.
     */
    if (!visible)
    {
        return;
    }


    font_draw_text(
        "INSERT COIN",
        title_center_text(
            "INSERT COIN",
            4
        ),
        prompt_y + 2,
        4,
        gold
    );

    font_draw_text(
        "PRESS ENTER",
        title_center_text(
            "PRESS ENTER",
            2
        ),
        prompt_y + 47,
        2,
        white
    );
}


/*
 * Displays the confirmation shown after Enter.
 */
static void title_draw_credit_accepted(void)
{
    uint32_t width;
    uint32_t height;

    uint32_t cabinet_dark;
    uint32_t green;

    uint32_t prompt_y;


    width =
        graphics_get_width();

    height =
        graphics_get_height();

    cabinet_dark =
        graphics_rgb(5, 7, 20);

    green =
        graphics_rgb(80, 255, 135);

    prompt_y =
        height * 62 / 100 + 65;


    /*
     * Clear the blinking prompt.
     */
    graphics_draw_rectangle(
        width / 4 + 10,
        prompt_y,
        width / 2 - 20,
        82,
        cabinet_dark
    );


    /*
     * Draw the confirmation message.
     */
    font_draw_text(
        "CREDIT ACCEPTED",
        title_center_text(
            "CREDIT ACCEPTED",
            3
        ),
        prompt_y + 24,
        3,
        green
    );
}


/*
 * Runs the ArcadeOS title screen.
 */
void title_screen_run(void)
{
    uint32_t previous_blink_state;

    /*
     * Remove any input events left over from the
     * previous screen.
     */
    keyboard_clear_pressed_events();


    /*
     * Draw the complete title screen.
     */
    title_draw_cabinet();
    title_draw_coin_prompt(1);

    previous_blink_state = 0;


    /*
     * Start the title-screen music.
     */
    music_play_title_theme();


    while (1)
    {
        uint32_t blink_state;


        /*
         * Advance the title music.
         */
        music_update();


        /*
         * Toggle the prompt every half second.
         */
        blink_state =
            (
                timer_get_ticks() /
                50
            ) % 2;


        if (
            blink_state !=
            previous_blink_state
        )
        {
            title_draw_coin_prompt(
                blink_state == 0
            );

            previous_blink_state =
                blink_state;
        }


        /*
         * Detect and consume one new Enter press.
         *
         * Unlike keyboard_is_key_down(), this removes
         * the stored pressed event after reading it.
         */
        if (
            keyboard_was_key_pressed(
                KEY_ENTER
            )
        )
        {
            /*
             * Stop the music.
             */
            music_stop();


            /*
             * Show that the coin was accepted.
             */
            title_draw_credit_accepted();


            /*
             * Wait until this same Enter press has
             * physically been released.
             */
            while (
                keyboard_is_key_down(
                    KEY_ENTER
                )
            )
            {
                __asm__ volatile ("hlt");
            }


            /*
             * Remove the old Enter press before the
             * game selector starts.
             *
             * Without this, the selector can interpret
             * the title-screen Enter as "launch Pong."
             */
            keyboard_clear_pressed_events();


            /*
             * Return to kernel_main(), which opens
             * the game-selection screen.
             */
            return;
        }


        /*
         * Wake on the next hardware interrupt.
         */
        __asm__ volatile ("hlt");
    }
}