#include "game_select.h"
#include "graphics.h"
#include "font.h"
#include "keyboard.h"
#include "timer.h"
#include "music.h"


#define GAME_COUNT 3

#define GAME_PONG_INDEX      0
#define GAME_SNAKE_INDEX     1
#define GAME_BREAKOUT_INDEX  2


/*
 * Returns the width of text drawn with the 5-by-7 font.
 */
static uint32_t menu_text_width(
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


/*
 * Draws text centered inside a horizontal area.
 */
static void menu_draw_centered_text(
    const char *text,
    uint32_t area_x,
    uint32_t area_width,
    uint32_t y,
    uint32_t scale,
    uint32_t color
)
{
    uint32_t width =
        menu_text_width(text, scale);

    uint32_t x =
        area_x;

    if (width < area_width)
    {
        x +=
            (area_width - width) / 2;
    }

    font_draw_text(
        text,
        x,
        y,
        scale,
        color
    );
}


/*
 * Small replacement for the standard absolute-value
 * function. ArcadeOS has no C standard library.
 */
static int menu_absolute(int value)
{
    if (value < 0)
    {
        return -value;
    }

    return value;
}


/*
 * Draws a line using Bresenham's line algorithm.
 */
static void menu_draw_line(
    int x0,
    int y0,
    int x1,
    int y1,
    uint32_t color
)
{
    int dx =
        menu_absolute(x1 - x0);

    int sx =
        x0 < x1 ? 1 : -1;

    int dy =
        -menu_absolute(y1 - y0);

    int sy =
        y0 < y1 ? 1 : -1;

    int error =
        dx + dy;


    while (1)
    {
        if (
            x0 >= 0 &&
            y0 >= 0
        )
        {
            graphics_put_pixel(
                (uint32_t)x0,
                (uint32_t)y0,
                color
            );
        }


        if (
            x0 == x1 &&
            y0 == y1
        )
        {
            break;
        }


        int doubled_error =
            error * 2;


        if (doubled_error >= dy)
        {
            error += dy;
            x0 += sx;
        }


        if (doubled_error <= dx)
        {
            error += dx;
            y0 += sy;
        }
    }
}


/*
 * Draws a small left-facing selection arrow.
 */
static void menu_draw_left_arrow(
    uint32_t x,
    uint32_t y,
    uint32_t color
)
{
    for (int offset = 0; offset < 3; offset++)
    {
        menu_draw_line(
            (int)x + 14 + offset,
            (int)y,
            (int)x + offset,
            (int)y + 12,
            color
        );

        menu_draw_line(
            (int)x + offset,
            (int)y + 12,
            (int)x + 14 + offset,
            (int)y + 24,
            color
        );
    }
}


/*
 * Draws a small right-facing selection arrow.
 */
static void menu_draw_right_arrow(
    uint32_t x,
    uint32_t y,
    uint32_t color
)
{
    for (int offset = 0; offset < 3; offset++)
    {
        menu_draw_line(
            (int)x + offset,
            (int)y,
            (int)x + 14 + offset,
            (int)y + 12,
            color
        );

        menu_draw_line(
            (int)x + 14 + offset,
            (int)y + 12,
            (int)x + offset,
            (int)y + 24,
            color
        );
    }
}


/*
 * Draws the perspective grid in the background.
 */
static void menu_draw_grid(
    uint32_t width,
    uint32_t height,
    uint32_t color
)
{
    uint32_t horizon =
        height * 58 / 100;


    /*
     * Horizontal grid lines.
     */
    for (
        uint32_t y = horizon;
        y < height - 45;
        y += 24
    )
    {
        graphics_draw_rectangle(
            26,
            y,
            width - 52,
            1,
            color
        );
    }


    /*
     * Lines moving toward the center create the
     * fake three-dimensional perspective.
     */
    int center_x =
        (int)(width / 2);

    int horizon_y =
        (int)horizon;

    for (
        int bottom_x = 25;
        bottom_x < (int)width - 25;
        bottom_x += 60
    )
    {
        menu_draw_line(
            center_x,
            horizon_y,
            bottom_x,
            (int)height - 46,
            color
        );
    }
}


/*
 * Draws the non-changing parts of the screen.
 */
static void menu_draw_background(void)
{
    uint32_t width =
        graphics_get_width();

    uint32_t height =
        graphics_get_height();


    uint32_t background =
        graphics_rgb(2, 4, 15);

    uint32_t dark_panel =
        graphics_rgb(7, 11, 28);

    uint32_t cyan =
        graphics_rgb(0, 232, 255);

    uint32_t magenta =
        graphics_rgb(255, 35, 200);

    uint32_t purple =
        graphics_rgb(94, 62, 210);

    uint32_t muted =
        graphics_rgb(40, 55, 92);

    uint32_t white =
        graphics_rgb(238, 246, 255);

    uint32_t gold =
        graphics_rgb(255, 197, 55);


    graphics_clear(background);


    /*
     * Small stars in the background.
     */
    for (uint32_t index = 0; index < 42; index++)
    {
        uint32_t star_x =
            (
                index * 97 + 53
            ) % width;

        uint32_t star_y =
            20 +
            (
                index * 47
            ) % 310;

        uint32_t star_size =
            index % 7 == 0 ? 2 : 1;

        graphics_draw_rectangle(
            star_x,
            star_y,
            star_size,
            star_size,
            white
        );
    }


    menu_draw_grid(
        width,
        height,
        muted
    );


    /*
     * Outer cabinet-style frame.
     */
    graphics_draw_rectangle(
        12,
        12,
        width - 24,
        height - 24,
        magenta
    );

    graphics_draw_rectangle(
        17,
        17,
        width - 34,
        height - 34,
        cyan
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
        34,
        width - 68,
        84,
        purple
    );

    graphics_draw_rectangle(
        39,
        39,
        width - 78,
        74,
        dark_panel
    );


    font_draw_text(
        "ARCADEOS",
        55,
        53,
        3,
        cyan
    );


    menu_draw_centered_text(
        "GAME SELECT",
        0,
        width,
        55,
        4,
        magenta
    );


    uint32_t credit_width =
        menu_text_width(
            "CREDIT READY",
            2
        );

    font_draw_text(
        "CREDIT READY",
        width - credit_width - 54,
        65,
        2,
        gold
    );


    /*
     * Bottom control instructions.
     */
    font_draw_text(
        "MOVE WITH A D OR ARROWS",
        55,
        height - 37,
        2,
        white
    );


    uint32_t enter_width =
        menu_text_width(
            "ENTER TO PLAY",
            2
        );

    font_draw_text(
        "ENTER TO PLAY",
        width - enter_width - 55,
        height - 37,
        2,
        gold
    );


    /*
     * Subtle scanlines.
     */
    for (
        uint32_t y = 123;
        y < height - 45;
        y += 5
    )
    {
        graphics_draw_rectangle(
            24,
            y,
            width - 48,
            1,
            graphics_rgb(3, 5, 16)
        );
    }
}


/*
 * Draws the Pong preview inside its card.
 */
static void menu_draw_pong_preview(
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height
)
{
    uint32_t preview_background =
        graphics_rgb(4, 8, 22);

    uint32_t cyan =
        graphics_rgb(0, 232, 255);

    uint32_t white =
        graphics_rgb(240, 248, 255);

    uint32_t muted =
        graphics_rgb(64, 88, 125);


    graphics_draw_rectangle(
        x,
        y,
        width,
        height,
        preview_background
    );


    uint32_t center_x =
        x + width / 2;


    for (
        uint32_t dash_y = y + 10;
        dash_y < y + height - 10;
        dash_y += 16
    )
    {
        graphics_draw_rectangle(
            center_x - 1,
            dash_y,
            2,
            8,
            muted
        );
    }


    graphics_draw_rectangle(
        x + 17,
        y + height / 2 - 21,
        7,
        42,
        cyan
    );


    graphics_draw_rectangle(
        x + width - 24,
        y + height / 2 - 21,
        7,
        42,
        cyan
    );


    graphics_draw_rectangle(
        center_x + 13,
        y + height / 2 - 4,
        8,
        8,
        white
    );
}


/*
 * Draws the Snake preview.
 */
static void menu_draw_snake_preview(
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height
)
{
    uint32_t preview_background =
        graphics_rgb(4, 8, 22);

    uint32_t green =
        graphics_rgb(65, 255, 155);

    uint32_t magenta =
        graphics_rgb(255, 35, 200);

    uint32_t muted =
        graphics_rgb(25, 40, 70);


    graphics_draw_rectangle(
        x,
        y,
        width,
        height,
        preview_background
    );


    for (
        uint32_t grid_y = y + 8;
        grid_y < y + height;
        grid_y += 16
    )
    {
        graphics_draw_rectangle(
            x,
            grid_y,
            width,
            1,
            muted
        );
    }


    for (
        uint32_t grid_x = x + 8;
        grid_x < x + width;
        grid_x += 16
    )
    {
        graphics_draw_rectangle(
            grid_x,
            y,
            1,
            height,
            muted
        );
    }


    uint32_t block =
        12;

    uint32_t snake_x =
        x + width / 2 - 30;

    uint32_t snake_y =
        y + height / 2;


    graphics_draw_rectangle(
        snake_x,
        snake_y,
        block,
        block,
        green
    );

    graphics_draw_rectangle(
        snake_x + block,
        snake_y,
        block,
        block,
        green
    );

    graphics_draw_rectangle(
        snake_x + block * 2,
        snake_y,
        block,
        block,
        green
    );

    graphics_draw_rectangle(
        snake_x + block * 2,
        snake_y - block,
        block,
        block,
        green
    );

    graphics_draw_rectangle(
        snake_x + block * 3,
        snake_y - block,
        block,
        block,
        green
    );


    graphics_draw_rectangle(
        x + width - 28,
        y + 24,
        8,
        8,
        magenta
    );
}


/*
 * Draws the Breakout preview.
 */
static void menu_draw_breakout_preview(
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height
)
{
    uint32_t preview_background =
        graphics_rgb(4, 8, 22);

    uint32_t cyan =
        graphics_rgb(0, 232, 255);

    uint32_t magenta =
        graphics_rgb(255, 35, 200);

    uint32_t gold =
        graphics_rgb(255, 197, 55);

    uint32_t white =
        graphics_rgb(240, 248, 255);


    graphics_draw_rectangle(
        x,
        y,
        width,
        height,
        preview_background
    );


    uint32_t brick_gap =
        3;

    uint32_t brick_width =
        (width - 16 - brick_gap * 4) / 5;

    uint32_t brick_height =
        11;


    for (uint32_t row = 0; row < 3; row++)
    {
        uint32_t row_color;

        if (row == 0)
        {
            row_color = magenta;
        }
        else if (row == 1)
        {
            row_color = gold;
        }
        else
        {
            row_color = cyan;
        }


        for (uint32_t column = 0; column < 5; column++)
        {
            graphics_draw_rectangle(
                x + 8
                    + column
                    * (
                        brick_width
                        + brick_gap
                    ),
                y + 9
                    + row
                    * (
                        brick_height
                        + brick_gap
                    ),
                brick_width,
                brick_height,
                row_color
            );
        }
    }


    graphics_draw_rectangle(
        x + width / 2 - 28,
        y + height - 19,
        56,
        6,
        cyan
    );


    graphics_draw_rectangle(
        x + width / 2 + 15,
        y + height - 42,
        7,
        7,
        white
    );
}


/*
 * Draws one cabinet card.
 */
static void menu_draw_game_card(
    uint32_t game_index,
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height,
    int selected,
    int pulse,
    int locked_flash
)
{
    uint32_t panel =
        graphics_rgb(10, 15, 36);

    uint32_t inner_panel =
        graphics_rgb(6, 10, 26);

    uint32_t cyan =
        graphics_rgb(0, 232, 255);

    uint32_t magenta =
        graphics_rgb(255, 35, 200);

    uint32_t purple =
        graphics_rgb(105, 72, 230);

    uint32_t white =
        graphics_rgb(240, 248, 255);

    uint32_t muted =
        graphics_rgb(112, 127, 160);

    uint32_t red =
        graphics_rgb(255, 70, 105);

    uint32_t gold =
        graphics_rgb(255, 197, 55);


    uint32_t border =
        purple;

    if (selected)
    {
        border =
            pulse ? cyan : magenta;
    }


    if (selected)
    {
        graphics_draw_rectangle(
            x - 4,
            y - 4,
            width + 8,
            height + 8,
            border
        );

        graphics_draw_rectangle(
            x,
            y,
            width,
            height,
            panel
        );
    }
    else
    {
        graphics_draw_rectangle(
            x,
            y,
            width,
            height,
            border
        );

        graphics_draw_rectangle(
            x + 3,
            y + 3,
            width - 6,
            height - 6,
            panel
        );
    }


    graphics_draw_rectangle(
        x + 8,
        y + 8,
        width - 16,
        height - 16,
        inner_panel
    );


    uint32_t preview_x =
        x + 13;

    uint32_t preview_y =
        y + 13;

    uint32_t preview_width =
        width - 26;

    uint32_t preview_height =
        120;


    if (game_index == GAME_PONG_INDEX)
    {
        menu_draw_pong_preview(
            preview_x,
            preview_y,
            preview_width,
            preview_height
        );
    }
    else if (game_index == GAME_SNAKE_INDEX)
    {
        menu_draw_snake_preview(
            preview_x,
            preview_y,
            preview_width,
            preview_height
        );
    }
    else
    {
        menu_draw_breakout_preview(
            preview_x,
            preview_y,
            preview_width,
            preview_height
        );
    }


    graphics_draw_rectangle(
        x + 13,
        preview_y + preview_height + 10,
        width - 26,
        2,
        border
    );


    const char *title;
    const char *category;
    const char *status;


    if (game_index == GAME_PONG_INDEX)
    {
        title =
            "PONG";

        category =
            "ARCADE CLASSIC";

        status =
            "READY TO PLAY";
    }
    else if (game_index == GAME_SNAKE_INDEX)
    {
        title =
            "SNAKE";

        category =
            "NEON GRID";

        status =
            "COMING SOON";
    }
    else
    {
        title =
            "BREAKOUT";

        category =
            "BRICK ATTACK";

        status =
            "COMING SOON";
    }


    menu_draw_centered_text(
        title,
        x,
        width,
        preview_y + preview_height + 24,
        3,
        selected ? white : muted
    );


    menu_draw_centered_text(
        category,
        x,
        width,
        preview_y + preview_height + 57,
        2,
        muted
    );


    uint32_t status_color =
        game_index == GAME_PONG_INDEX
        ? gold
        : muted;


    if (
        selected &&
        game_index != GAME_PONG_INDEX
    )
    {
        status_color =
            locked_flash ? red : magenta;
    }


    menu_draw_centered_text(
        status,
        x,
        width,
        y + height - 36,
        2,
        status_color
    );


    if (selected)
    {
        menu_draw_left_arrow(
            x - 25,
            y + height / 2 - 12,
            white
        );

        menu_draw_right_arrow(
            x + width + 10,
            y + height / 2 - 12,
            white
        );
    }
}


/*
 * Draws the description panel beneath the cards.
 */
static void menu_draw_description(
    uint32_t selected_game,
    int locked_flash
)
{
    uint32_t width =
        graphics_get_width();

    uint32_t height =
        graphics_get_height();

    uint32_t panel =
        graphics_rgb(8, 13, 31);

    uint32_t cyan =
        graphics_rgb(0, 232, 255);

    uint32_t magenta =
        graphics_rgb(255, 35, 200);

    uint32_t white =
        graphics_rgb(240, 248, 255);

    uint32_t muted =
        graphics_rgb(128, 145, 180);

    uint32_t red =
        graphics_rgb(255, 70, 105);


    uint32_t panel_x =
        52;

    uint32_t panel_y =
        height - 112;

    uint32_t panel_width =
        width - 104;

    uint32_t panel_height =
        64;


    graphics_draw_rectangle(
        panel_x,
        panel_y,
        panel_width,
        panel_height,
        cyan
    );

    graphics_draw_rectangle(
        panel_x + 3,
        panel_y + 3,
        panel_width - 6,
        panel_height - 6,
        panel
    );


    if (selected_game == GAME_PONG_INDEX)
    {
        font_draw_text(
            "PONG",
            panel_x + 18,
            panel_y + 13,
            3,
            white
        );

        font_draw_text(
            "A MODERN TAKE ON A TWO PLAYER CLASSIC",
            panel_x + 150,
            panel_y + 19,
            2,
            muted
        );
    }
    else
    {
        const char *title =
            selected_game == GAME_SNAKE_INDEX
            ? "SNAKE"
            : "BREAKOUT";


        font_draw_text(
            title,
            panel_x + 18,
            panel_y + 13,
            3,
            white
        );


        font_draw_text(
            "THIS CABINET IS STILL UNDER CONSTRUCTION",
            panel_x + 180,
            panel_y + 19,
            2,
            locked_flash ? red : magenta
        );
    }
}


/*
 * Draws all three game cards.
 */
static void menu_draw_cards(
    uint32_t selected_game,
    int pulse,
    int locked_flash
)
{
    uint32_t width =
        graphics_get_width();

    uint32_t height =
        graphics_get_height();

    uint32_t background =
        graphics_rgb(2, 4, 15);


    uint32_t side_margin =
        58;

    uint32_t gap =
        18;

    uint32_t available_width =
        width
        - side_margin * 2
        - gap * 2;

    uint32_t card_width =
        available_width / 3;

    uint32_t card_y =
        145;

    uint32_t description_y =
        height - 112;

    uint32_t card_height =
        description_y
        - card_y
        - 18;


    /*
     * Clear the old card region so previous selection
     * arrows and glowing borders do not remain.
     */
    graphics_draw_rectangle(
        side_margin - 30,
        card_y - 8,
        width - (side_margin - 30) * 2,
        card_height + 16,
        background
    );


    for (
        uint32_t game = 0;
        game < GAME_COUNT;
        game++
    )
    {
        uint32_t card_x =
            side_margin
            + game
            * (
                card_width
                + gap
            );


        menu_draw_game_card(
            game,
            card_x,
            card_y,
            card_width,
            card_height,
            game == selected_game,
            pulse,
            locked_flash
        );
    }


    menu_draw_description(
        selected_game,
        locked_flash
    );
}


/*
 * Runs the interactive game selector.
 */
game_selection_t game_select_run(void)
{
    uint32_t selected_game =
        GAME_PONG_INDEX;

    uint32_t locked_flash_until =
        0;

    int locked_flash_was_visible =
        0;


    /*
     * Discard input events inherited from the
     * previous screen.
     */
    keyboard_clear_pressed_events();


    /*
     * Draw the screen once when the selector starts.
     */
    menu_draw_background();

    menu_draw_cards(
        selected_game,
        1,
        0
    );


    /*
     * Start the slower selector melody.
     */
    music_play_game_select_theme();


    while (1)
    {
        uint32_t current_tick =
            timer_get_ticks();

        keyboard_key_t key =
            keyboard_poll_key();

        int needs_redraw =
            0;


        /*
         * Keep the melody moving without pausing
         * menu input or animation.
         */
        music_update();


        /*
         * Move to the previous game.
         */
        if (key == KEY_LEFT)
        {
            if (selected_game == 0)
            {
                selected_game =
                    GAME_COUNT - 1;
            }
            else
            {
                selected_game--;
            }

            locked_flash_until =
                0;

            music_play_menu_move_sound();

            needs_redraw =
                1;
        }


        /*
         * Move to the next game.
         */
        else if (key == KEY_RIGHT)
        {
            selected_game =
                (
                    selected_game + 1
                ) % GAME_COUNT;

            locked_flash_until =
                0;

            music_play_menu_move_sound();

            needs_redraw =
                1;
        }


        /*
         * Return to the title screen.
         */
        else if (key == KEY_ESCAPE)
        {
            /*
             * Play the back sound before returning to the
             * title screen.
             */
            music_play_menu_back_sound();

            while (music_effect_is_playing())
            {
                music_update();
                __asm__ volatile ("hlt");
            }

            music_stop();

            return GAME_SELECTION_BACK;
        }


        /*
         * Launch the selected game.
         */
        else if (key == KEY_ENTER)
        {
            if (
                selected_game
                == GAME_PONG_INDEX
            )
            {
                /*
                 * Play the game-launch confirmation before
                 * Pong takes ownership of the PC speaker.
                 */
                music_play_game_enter_sound();

                while (music_effect_is_playing())
                {
                    music_update();
                    __asm__ volatile ("hlt");
                }

                music_stop();

                return GAME_SELECTION_PONG;
            }


            /*
             * Snake and Breakout are not ready yet.
             * Display the warning temporarily.
             */
            locked_flash_until =
                current_tick + 45;

            needs_redraw =
                1;
        }


        int locked_flash_visible =
            current_tick
            < locked_flash_until;


        /*
         * Redraw once when the temporary warning ends.
         */
        if (
            locked_flash_was_visible &&
            !locked_flash_visible
        )
        {
            needs_redraw =
                1;
        }


        /*
         * Only redraw after an actual change.
         */
        if (needs_redraw)
        {
            menu_draw_cards(
                selected_game,
                1,
                locked_flash_visible
            );
        }


        locked_flash_was_visible =
            locked_flash_visible;


        /*
         * Sleep until the timer wakes the processor.
         */
        __asm__ volatile ("hlt");
    }
}