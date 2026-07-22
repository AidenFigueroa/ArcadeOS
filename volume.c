#include "volume.h"
#include "graphics.h"
#include "font.h"
#include "sound.h"

#include <stdint.h>


/*
 * Start at a comfortable software level.
 */
static int current_volume_level = VOLUME_MAX_LEVEL;


/*
 * Converts the current 0-10 level into a tiny string without relying on
 * snprintf(), which is unavailable in ArcadeOS's freestanding kernel.
 */
static void volume_make_level_text(char text[4])
{
    if (current_volume_level == VOLUME_MIN_LEVEL)
    {
        text[0] = 'O';
        text[1] = 'F';
        text[2] = 'F';
        text[3] = '\0';
    }
    else
    {
        text[0] = 'O';
        text[1] = 'N';
        text[2] = '\0';
    }
}


/*
 * Draws a compact retro volume panel in the upper-right corner.
 *
 * The complete panel is redrawn whenever F1 or F2 is pressed, so the old
 * slider fill is replaced rather than being left behind.
 */
static void volume_draw_overlay(void)
{
    uint32_t screen_width =
        graphics_get_width();

    uint32_t panel_width = 250;
    uint32_t panel_height = 78;
    uint32_t panel_x;
    uint32_t panel_y = 28;

    uint32_t black =
        graphics_rgb(2, 4, 15);

    uint32_t dark =
        graphics_rgb(8, 12, 30);

    uint32_t cyan =
        graphics_rgb(0, 232, 255);

    uint32_t magenta =
        graphics_rgb(255, 35, 200);

    uint32_t muted =
        graphics_rgb(47, 60, 92);

    uint32_t white =
        graphics_rgb(240, 248, 255);

    uint32_t bar_x;
    uint32_t bar_y;
    uint32_t bar_width = 168;
    uint32_t bar_height = 14;
    uint32_t fill_width;

    char level_text[4];


    if (screen_width > panel_width + 28)
    {
        panel_x =
            screen_width - panel_width - 28;
    }
    else
    {
        panel_x = 0;
        panel_width = screen_width;
    }


    /* Neon outer frame. */
    graphics_draw_rectangle(
        panel_x,
        panel_y,
        panel_width,
        panel_height,
        magenta
    );

    graphics_draw_rectangle(
        panel_x + 3,
        panel_y + 3,
        panel_width - 6,
        panel_height - 6,
        cyan
    );

    graphics_draw_rectangle(
        panel_x + 6,
        panel_y + 6,
        panel_width - 12,
        panel_height - 12,
        dark
    );


    font_draw_text(
    current_volume_level == VOLUME_MIN_LEVEL
        ? "SOUND MUTED"
        : "SOUND ON",
    panel_x + 16,
    panel_y + 14,
    2,
    white
);


    bar_x =
        panel_x + 16;

    bar_y =
        panel_y + 48;


    /* Slider track. */
    graphics_draw_rectangle(
        bar_x,
        bar_y,
        bar_width,
        bar_height,
        black
    );

    graphics_draw_rectangle(
        bar_x + 2,
        bar_y + 2,
        bar_width - 4,
        bar_height - 4,
        muted
    );


    fill_width =
        (bar_width - 4) *
        (uint32_t)current_volume_level /
        VOLUME_MAX_LEVEL;

    if (fill_width > 0)
    {
        graphics_draw_rectangle(
            bar_x + 2,
            bar_y + 2,
            fill_width,
            bar_height - 4,
            current_volume_level <= 2
                ? magenta
                : cyan
        );
    }


    volume_make_level_text(level_text);

    font_draw_text(
        level_text,
        panel_x + 204,
        panel_y + 44,
        2,
        white
    );
}


void volume_initialize(void)
{
    current_volume_level = VOLUME_MAX_LEVEL;
}

void volume_handle_key_press(keyboard_key_t key)
{
    if (key == KEY_F1)
    {
        /*
         * F1 mutes ArcadeOS and immediately stops
         * any tone currently playing.
         */
        current_volume_level = VOLUME_MIN_LEVEL;
        sound_stop();
    }
    else if (key == KEY_F2)
    {
        /*
         * F2 unmutes ArcadeOS.
         *
         * The next tone requested by the music system
         * will be allowed to play.
         */
        current_volume_level = VOLUME_MAX_LEVEL;
    }
    else
    {
        return;
    }

    volume_draw_overlay();
}

    /*
     * Zero is a real mute. The classic PC speaker does not expose normal
     * amplitude control, so intermediate levels currently represent the
     * OS setting and slider state rather than separate hardware amplitudes.
     */
    


void volume_update_overlay(void)
{
    /*
     * Reserved for a future timed fade-out. The panel is currently redrawn
     * immediately when F1 or F2 is pressed and remains until the active
     * screen replaces that region.
     */
}


int volume_get_level(void)
{
    return current_volume_level;
}


int volume_is_muted(void)
{
    return current_volume_level == 0;
}