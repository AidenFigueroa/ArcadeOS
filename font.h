#ifndef FONT_H
#define FONT_H

#include "multiboot.h"

void font_draw_character(
    char character,
    uint32_t x,
    uint32_t y,
    uint32_t scale,
    uint32_t color
);

void font_draw_text(
    const char *text,
    uint32_t x,
    uint32_t y,
    uint32_t scale,
    uint32_t color
);

#endif