#include "terminal.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static volatile unsigned char *video_memory =
    (volatile unsigned char *)0xB8000;

static int terminal_row = 0;
static int terminal_column = 0;
static unsigned char terminal_color = 0x0F;


/*
 * Calculates the video-memory index for a screen position.
 *
 * Each screen position uses two bytes:
 *   character
 *   color
 */
static int terminal_index(int row, int column)
{
    return (row * VGA_WIDTH + column) * 2;
}


/*
 * Places one character at a specific screen position.
 */
static void terminal_putentryat(
    char character,
    unsigned char color,
    int row,
    int column
)
{
    int index = terminal_index(row, column);

    video_memory[index] = character;
    video_memory[index + 1] = color;
}


/*
 * Moves every screen row upward by one row.
 */
static void terminal_scroll(void)
{
    for (int row = 1; row < VGA_HEIGHT; row++)
    {
        for (int column = 0; column < VGA_WIDTH; column++)
        {
            int source_index =
                terminal_index(row, column);

            int destination_index =
                terminal_index(row - 1, column);

            video_memory[destination_index] =
                video_memory[source_index];

            video_memory[destination_index + 1] =
                video_memory[source_index + 1];
        }
    }

    /*
     * Clear the final row.
     */
    for (int column = 0; column < VGA_WIDTH; column++)
    {
        terminal_putentryat(
            ' ',
            terminal_color,
            VGA_HEIGHT - 1,
            column
        );
    }

    terminal_row = VGA_HEIGHT - 1;
}


/*
 * Moves the writing position to the next line.
 */
static void terminal_newline(void)
{
    terminal_column = 0;
    terminal_row++;

    if (terminal_row >= VGA_HEIGHT)
    {
        terminal_scroll();
    }
}


/*
 * Clears the screen and resets the writing position.
 */
void terminal_initialize(void)
{
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = 0x0F;

    for (int row = 0; row < VGA_HEIGHT; row++)
    {
        for (int column = 0; column < VGA_WIDTH; column++)
        {
            terminal_putentryat(
                ' ',
                terminal_color,
                row,
                column
            );
        }
    }
}


/*
 * Writes one character to the screen.
 */
void terminal_putchar(char character)
{
    if (character == '\n')
    {
        terminal_newline();
        return;
    }

    terminal_putentryat(
        character,
        terminal_color,
        terminal_row,
        terminal_column
    );

    terminal_column++;

    if (terminal_column >= VGA_WIDTH)
    {
        terminal_newline();
    }
}


/*
 * Writes a null-terminated string to the screen.
 */
void terminal_write(const char *text)
{
    int index = 0;

    while (text[index] != '\0')
    {
        terminal_putchar(text[index]);
        index++;
    }
}