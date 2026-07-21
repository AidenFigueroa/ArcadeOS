#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "multiboot.h"


int graphics_initialize(
    const multiboot_info_t *multiboot_info
);

uint32_t graphics_rgb(
    uint8_t red,
    uint8_t green,
    uint8_t blue
);

void graphics_put_pixel(
    uint32_t x,
    uint32_t y,
    uint32_t color
);

void graphics_clear(
    uint32_t color
);

void graphics_draw_rectangle(
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height,
    uint32_t color
);

uint32_t graphics_get_width(void);
uint32_t graphics_get_height(void);


/*
 * Copies the visible framebuffer into a hidden buffer
 * and sends future drawing operations to that buffer.
 */
int graphics_enable_double_buffering(void);


/*
 * Copies the completed hidden frame onto the display.
 */
void graphics_present(void);


/*
 * Returns drawing to the visible framebuffer.
 */
void graphics_disable_double_buffering(void);


#endif