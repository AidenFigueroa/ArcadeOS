#include "graphics.h"
#include "io.h"


/*
 * Supports double buffering through 1024x768x32.
 *
 * At 800x600x32, ArcadeOS uses about 1.9 MB.
 */
#define GRAPHICS_MAX_BUFFER_SIZE \
    (1024U * 768U * 4U)


static volatile uint8_t *framebuffer =
    (void *)0;


/*
 * Hidden screen used while preparing a complete frame.
 *
 * It lives in the kernel's BSS instead of the small
 * kmalloc heap.
 */
static uint8_t back_buffer[
    GRAPHICS_MAX_BUFFER_SIZE
] __attribute__((aligned(16)));


static uint32_t screen_width = 0;
static uint32_t screen_height = 0;
static uint32_t screen_pitch = 0;
static uint32_t screen_buffer_size = 0;

static uint8_t bits_per_pixel = 0;
static uint8_t bytes_per_pixel = 0;

static uint8_t red_position = 0;
static uint8_t red_mask_size = 0;

static uint8_t green_position = 0;
static uint8_t green_mask_size = 0;

static uint8_t blue_position = 0;
static uint8_t blue_mask_size = 0;

static int double_buffering_enabled = 0;


static void graphics_debug(
    const char *text
)
{
    uint32_t index = 0;

    while (text[index] != '\0')
    {
        outb(
            0xE9,
            (uint8_t)text[index]
        );

        index++;
    }
}


static void graphics_debug_number(
    uint32_t number
)
{
    char digits[11];
    int index = 0;

    if (number == 0)
    {
        outb(0xE9, '0');
        return;
    }

    while (number > 0)
    {
        digits[index] =
            '0' + number % 10;

        number /= 10;
        index++;
    }

    while (index > 0)
    {
        index--;

        outb(
            0xE9,
            (uint8_t)digits[index]
        );
    }
}


static void graphics_debug_information(
    const multiboot_info_t *multiboot_info
)
{
    graphics_debug(
        "Framebuffer address: "
    );

    graphics_debug_number(
        (uint32_t)
        multiboot_info->framebuffer_address
    );

    graphics_debug(
        "\nFramebuffer width: "
    );

    graphics_debug_number(
        multiboot_info->framebuffer_width
    );

    graphics_debug(
        "\nFramebuffer height: "
    );

    graphics_debug_number(
        multiboot_info->framebuffer_height
    );

    graphics_debug(
        "\nFramebuffer pitch: "
    );

    graphics_debug_number(
        multiboot_info->framebuffer_pitch
    );

    graphics_debug(
        "\nBits per pixel: "
    );

    graphics_debug_number(
        multiboot_info
            ->framebuffer_bits_per_pixel
    );

    graphics_debug(
        "\nFramebuffer type: "
    );

    graphics_debug_number(
        multiboot_info->framebuffer_type
    );

    graphics_debug(
        "\nRed position: "
    );

    graphics_debug_number(
        multiboot_info
            ->framebuffer_red_position
    );

    graphics_debug(
        "\nRed mask size: "
    );

    graphics_debug_number(
        multiboot_info
            ->framebuffer_red_mask_size
    );

    graphics_debug(
        "\nGreen position: "
    );

    graphics_debug_number(
        multiboot_info
            ->framebuffer_green_position
    );

    graphics_debug(
        "\nGreen mask size: "
    );

    graphics_debug_number(
        multiboot_info
            ->framebuffer_green_mask_size
    );

    graphics_debug(
        "\nBlue position: "
    );

    graphics_debug_number(
        multiboot_info
            ->framebuffer_blue_position
    );

    graphics_debug(
        "\nBlue mask size: "
    );

    graphics_debug_number(
        multiboot_info
            ->framebuffer_blue_mask_size
    );

    graphics_debug("\n");
}


static uint32_t scale_color_channel(
    uint8_t value,
    uint8_t mask_size
)
{
    uint32_t maximum_value =
        (1U << mask_size) - 1U;

    return (
        (
            (uint32_t)value *
            maximum_value
        ) + 127U
    ) / 255U;
}


/*
 * Returns whichever buffer drawing operations should
 * currently modify.
 */
static volatile uint8_t *
graphics_get_drawing_buffer(void)
{
    if (double_buffering_enabled)
    {
        return back_buffer;
    }

    return framebuffer;
}


int graphics_initialize(
    const multiboot_info_t *multiboot_info
)
{
    if (multiboot_info == (void *)0)
    {
        graphics_debug(
            "Graphics error: Multiboot information is null\n"
        );

        return 0;
    }

    if (
        (
            multiboot_info->flags &
            MULTIBOOT_INFO_FRAMEBUFFER
        ) == 0
    )
    {
        graphics_debug(
            "Graphics error: no framebuffer information\n"
        );

        return 0;
    }

    graphics_debug_information(
        multiboot_info
    );

    if (
        multiboot_info->framebuffer_type
        != MULTIBOOT_FRAMEBUFFER_TYPE_RGB
    )
    {
        graphics_debug(
            "Graphics error: framebuffer is not RGB\n"
        );

        return 0;
    }

    bits_per_pixel =
        multiboot_info
            ->framebuffer_bits_per_pixel;

    if (
        bits_per_pixel != 15 &&
        bits_per_pixel != 16 &&
        bits_per_pixel != 24 &&
        bits_per_pixel != 32
    )
    {
        graphics_debug(
            "Graphics error: unsupported bits per pixel\n"
        );

        return 0;
    }

    if (
        (
            multiboot_info
                ->framebuffer_address >> 32
        ) != 0
    )
    {
        graphics_debug(
            "Graphics error: framebuffer is above 4 GiB\n"
        );

        return 0;
    }

    if (
        multiboot_info
            ->framebuffer_width == 0 ||
        multiboot_info
            ->framebuffer_height == 0 ||
        multiboot_info
            ->framebuffer_pitch == 0
    )
    {
        graphics_debug(
            "Graphics error: invalid framebuffer dimensions\n"
        );

        return 0;
    }

    uint8_t current_red_position =
        multiboot_info
            ->framebuffer_red_position;

    uint8_t current_red_size =
        multiboot_info
            ->framebuffer_red_mask_size;

    uint8_t current_green_position =
        multiboot_info
            ->framebuffer_green_position;

    uint8_t current_green_size =
        multiboot_info
            ->framebuffer_green_mask_size;

    uint8_t current_blue_position =
        multiboot_info
            ->framebuffer_blue_position;

    uint8_t current_blue_size =
        multiboot_info
            ->framebuffer_blue_mask_size;

    if (
        current_red_size == 0 ||
        current_green_size == 0 ||
        current_blue_size == 0 ||

        current_red_size > 16 ||
        current_green_size > 16 ||
        current_blue_size > 16 ||

        current_red_position +
            current_red_size >
            bits_per_pixel ||

        current_green_position +
            current_green_size >
            bits_per_pixel ||

        current_blue_position +
            current_blue_size >
            bits_per_pixel
    )
    {
        graphics_debug(
            "Graphics error: invalid RGB mask layout\n"
        );

        return 0;
    }

    framebuffer =
        (volatile uint8_t *)
        (uint32_t)
        multiboot_info
            ->framebuffer_address;

    screen_width =
        multiboot_info
            ->framebuffer_width;

    screen_height =
        multiboot_info
            ->framebuffer_height;

    screen_pitch =
        multiboot_info
            ->framebuffer_pitch;

    bytes_per_pixel =
        (bits_per_pixel + 7) / 8;

    if (
        screen_pitch <
        screen_width * bytes_per_pixel
    )
    {
        graphics_debug(
            "Graphics error: framebuffer pitch is too small\n"
        );

        return 0;
    }

    if (
        screen_height >
        0xFFFFFFFFU / screen_pitch
    )
    {
        graphics_debug(
            "Graphics error: framebuffer size overflow\n"
        );

        return 0;
    }

    screen_buffer_size =
        screen_pitch * screen_height;

    red_position =
        current_red_position;

    red_mask_size =
        current_red_size;

    green_position =
        current_green_position;

    green_mask_size =
        current_green_size;

    blue_position =
        current_blue_position;

    blue_mask_size =
        current_blue_size;

    double_buffering_enabled = 0;

    graphics_debug(
        "ArcadeOS: graphics initialization succeeded\n"
    );

    return 1;
}


uint32_t graphics_rgb(
    uint8_t red,
    uint8_t green,
    uint8_t blue
)
{
    uint32_t converted_red =
        scale_color_channel(
            red,
            red_mask_size
        );

    uint32_t converted_green =
        scale_color_channel(
            green,
            green_mask_size
        );

    uint32_t converted_blue =
        scale_color_channel(
            blue,
            blue_mask_size
        );

    return (
        converted_red << red_position
    ) | (
        converted_green << green_position
    ) | (
        converted_blue << blue_position
    );
}


void graphics_put_pixel(
    uint32_t x,
    uint32_t y,
    uint32_t color
)
{
    if (
        x >= screen_width ||
        y >= screen_height
    )
    {
        return;
    }

    volatile uint8_t *buffer =
        graphics_get_drawing_buffer();

    volatile uint8_t *pixel =
        buffer
        + y * screen_pitch
        + x * bytes_per_pixel;

    if (bytes_per_pixel == 4)
    {
        *((volatile uint32_t *)pixel) =
            color;

        return;
    }

    for (
        uint8_t byte = 0;
        byte < bytes_per_pixel;
        byte++
    )
    {
        pixel[byte] =
            (
                color >>
                (byte * 8)
            ) & 0xFF;
    }
}


void graphics_clear(
    uint32_t color
)
{
    volatile uint8_t *buffer =
        graphics_get_drawing_buffer();

    /*
     * Fast path for the 32-bit framebuffer used by
     * ArcadeOS.
     */
    if (bytes_per_pixel == 4)
    {
        for (
            uint32_t y = 0;
            y < screen_height;
            y++
        )
        {
            volatile uint32_t *row =
                (volatile uint32_t *)
                (
                    buffer +
                    y * screen_pitch
                );

            for (
                uint32_t x = 0;
                x < screen_width;
                x++
            )
            {
                row[x] = color;
            }
        }

        return;
    }

    for (
        uint32_t y = 0;
        y < screen_height;
        y++
    )
    {
        for (
            uint32_t x = 0;
            x < screen_width;
            x++
        )
        {
            graphics_put_pixel(
                x,
                y,
                color
            );
        }
    }
}


void graphics_draw_rectangle(
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height,
    uint32_t color
)
{
    if (
        x >= screen_width ||
        y >= screen_height ||
        width == 0 ||
        height == 0
    )
    {
        return;
    }

    if (width > screen_width - x)
    {
        width =
            screen_width - x;
    }

    if (height > screen_height - y)
    {
        height =
            screen_height - y;
    }

    volatile uint8_t *buffer =
        graphics_get_drawing_buffer();

    if (bytes_per_pixel == 4)
    {
        for (
            uint32_t current_y = y;
            current_y < y + height;
            current_y++
        )
        {
            volatile uint32_t *row =
                (volatile uint32_t *)
                (
                    buffer +
                    current_y * screen_pitch
                );

            for (
                uint32_t current_x = x;
                current_x < x + width;
                current_x++
            )
            {
                row[current_x] =
                    color;
            }
        }

        return;
    }

    for (
        uint32_t current_y = y;
        current_y < y + height;
        current_y++
    )
    {
        for (
            uint32_t current_x = x;
            current_x < x + width;
            current_x++
        )
        {
            graphics_put_pixel(
                current_x,
                current_y,
                color
            );
        }
    }
}


int graphics_enable_double_buffering(void)
{
    if (
        screen_buffer_size == 0 ||
        screen_buffer_size >
            GRAPHICS_MAX_BUFFER_SIZE
    )
    {
        graphics_debug(
            "Graphics error: framebuffer is too large for back buffer\n"
        );

        return 0;
    }

    /*
     * Copy the current screen into the hidden buffer,
     * allowing Pong to continue from the already drawn
     * court.
     */
    uint32_t word_count =
        screen_buffer_size / 4;

    volatile uint32_t *source =
        (volatile uint32_t *)
        framebuffer;

    uint32_t *destination =
        (uint32_t *)
        back_buffer;

    for (
        uint32_t index = 0;
        index < word_count;
        index++
    )
    {
        destination[index] =
            source[index];
    }

    uint32_t copied_bytes =
        word_count * 4;

    for (
        uint32_t index = copied_bytes;
        index < screen_buffer_size;
        index++
    )
    {
        back_buffer[index] =
            framebuffer[index];
    }

    double_buffering_enabled = 1;

    return 1;
}


void graphics_present(void)
{
    if (!double_buffering_enabled)
    {
        return;
    }

    uint32_t word_count =
        screen_buffer_size / 4;

    volatile uint32_t *destination =
        (volatile uint32_t *)
        framebuffer;

    uint32_t *source =
        (uint32_t *)
        back_buffer;

    for (
        uint32_t index = 0;
        index < word_count;
        index++
    )
    {
        destination[index] =
            source[index];
    }

    uint32_t copied_bytes =
        word_count * 4;

    for (
        uint32_t index = copied_bytes;
        index < screen_buffer_size;
        index++
    )
    {
        framebuffer[index] =
            back_buffer[index];
    }
}


void graphics_disable_double_buffering(void)
{
    if (double_buffering_enabled)
    {
        graphics_present();
    }

    double_buffering_enabled = 0;
}


uint32_t graphics_get_width(void)
{
    return screen_width;
}


uint32_t graphics_get_height(void)
{
    return screen_height;
}