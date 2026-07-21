#include "font.h"
#include "graphics.h"

#define FONT_WIDTH 5
#define FONT_HEIGHT 7

/*
 * Returns one row of pixel data for an uppercase letter.
 *
 * Each five-bit value represents one horizontal row.
 *
 * Example:
 *
 * 0b01110
 *
 * means:
 *
 *  .###.
 */
static uint8_t font_get_row(
    char character,
    uint32_t row
)
{
    static const uint8_t letters[26][FONT_HEIGHT] =
    {
        /* A */
        {
            0x0E,
            0x11,
            0x11,
            0x1F,
            0x11,
            0x11,
            0x11
        },

        /* B */
        {
            0x1E,
            0x11,
            0x11,
            0x1E,
            0x11,
            0x11,
            0x1E
        },

        /* C */
        {
            0x0E,
            0x11,
            0x10,
            0x10,
            0x10,
            0x11,
            0x0E
        },

        /* D */
        {
            0x1E,
            0x11,
            0x11,
            0x11,
            0x11,
            0x11,
            0x1E
        },

        /* E */
        {
            0x1F,
            0x10,
            0x10,
            0x1E,
            0x10,
            0x10,
            0x1F
        },

        /* F */
        {
            0x1F,
            0x10,
            0x10,
            0x1E,
            0x10,
            0x10,
            0x10
        },

        /* G */
        {
            0x0E,
            0x11,
            0x10,
            0x17,
            0x11,
            0x11,
            0x0F
        },

        /* H */
        {
            0x11,
            0x11,
            0x11,
            0x1F,
            0x11,
            0x11,
            0x11
        },

        /* I */
        {
            0x1F,
            0x04,
            0x04,
            0x04,
            0x04,
            0x04,
            0x1F
        },

        /* J */
        {
            0x01,
            0x01,
            0x01,
            0x01,
            0x11,
            0x11,
            0x0E
        },

        /* K */
        {
            0x11,
            0x12,
            0x14,
            0x18,
            0x14,
            0x12,
            0x11
        },

        /* L */
        {
            0x10,
            0x10,
            0x10,
            0x10,
            0x10,
            0x10,
            0x1F
        },

        /* M */
        {
            0x11,
            0x1B,
            0x15,
            0x15,
            0x11,
            0x11,
            0x11
        },

        /* N */
        {
            0x11,
            0x19,
            0x15,
            0x13,
            0x11,
            0x11,
            0x11
        },

        /* O */
        {
            0x0E,
            0x11,
            0x11,
            0x11,
            0x11,
            0x11,
            0x0E
        },

        /* P */
        {
            0x1E,
            0x11,
            0x11,
            0x1E,
            0x10,
            0x10,
            0x10
        },

        /* Q */
        {
            0x0E,
            0x11,
            0x11,
            0x11,
            0x15,
            0x12,
            0x0D
        },

        /* R */
        {
            0x1E,
            0x11,
            0x11,
            0x1E,
            0x14,
            0x12,
            0x11
        },

        /* S */
        {
            0x0F,
            0x10,
            0x10,
            0x0E,
            0x01,
            0x01,
            0x1E
        },

        /* T */
        {
            0x1F,
            0x04,
            0x04,
            0x04,
            0x04,
            0x04,
            0x04
        },

        /* U */
        {
            0x11,
            0x11,
            0x11,
            0x11,
            0x11,
            0x11,
            0x0E
        },

        /* V */
        {
            0x11,
            0x11,
            0x11,
            0x11,
            0x11,
            0x0A,
            0x04
        },

        /* W */
        {
            0x11,
            0x11,
            0x11,
            0x15,
            0x15,
            0x15,
            0x0A
        },

        /* X */
        {
            0x11,
            0x11,
            0x0A,
            0x04,
            0x0A,
            0x11,
            0x11
        },

        /* Y */
        {
            0x11,
            0x11,
            0x0A,
            0x04,
            0x04,
            0x04,
            0x04
        },

        /* Z */
        {
            0x1F,
            0x01,
            0x02,
            0x04,
            0x08,
            0x10,
            0x1F
        }
    };

    if (row >= FONT_HEIGHT)
    {
        return 0;
    }

    if (
        character >= 'a' &&
        character <= 'z'
    )
    {
        character =
            character - 'a' + 'A';
    }

    if (
        character < 'A' ||
        character > 'Z'
    )
    {
        return 0;
    }

    return letters[character - 'A'][row];
}


void font_draw_character(
    char character,
    uint32_t x,
    uint32_t y,
    uint32_t scale,
    uint32_t color
)
{
    if (scale == 0)
    {
        return;
    }

    for (
        uint32_t row = 0;
        row < FONT_HEIGHT;
        row++
    )
    {
        uint8_t row_pixels =
            font_get_row(character, row);

        for (
            uint32_t column = 0;
            column < FONT_WIDTH;
            column++
        )
        {
            uint8_t bit =
                1U << (FONT_WIDTH - 1 - column);

            if ((row_pixels & bit) != 0)
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


void font_draw_text(
    const char *text,
    uint32_t x,
    uint32_t y,
    uint32_t scale,
    uint32_t color
)
{
    uint32_t current_x = x;
    uint32_t current_y = y;

    for (
        uint32_t index = 0;
        text[index] != '\0';
        index++
    )
    {
        char character = text[index];

        if (character == '\n')
        {
            current_x = x;
            current_y +=
                (FONT_HEIGHT + 2) * scale;

            continue;
        }

        if (character == ' ')
        {
            current_x +=
                (FONT_WIDTH + 1) * scale;

            continue;
        }

        font_draw_character(
            character,
            current_x,
            current_y,
            scale,
            color
        );

        current_x +=
            (FONT_WIDTH + 1) * scale;
    }
}