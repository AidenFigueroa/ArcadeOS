#ifndef MULTIBOOT_H
#define MULTIBOOT_H


typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;


/*
 * GRUB places this value in EAX before entering
 * a Multiboot-compatible kernel.
 */
#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002


/*
 * Bit 12 means framebuffer information is available.
 */
#define MULTIBOOT_INFO_FRAMEBUFFER 0x00001000


#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
#define MULTIBOOT_FRAMEBUFFER_TYPE_TEXT    2


/*
 * Information passed to ArcadeOS by GRUB.
 *
 * The field order and offsets must exactly match
 * the Multiboot specification.
 */
typedef struct
{
    /*
     * Offset 0
     */
    uint32_t flags;

    /*
     * Offsets 4 and 8
     */
    uint32_t mem_lower;
    uint32_t mem_upper;

    /*
     * Offset 12
     */
    uint32_t boot_device;

    /*
     * Offset 16
     */
    uint32_t cmdline;

    /*
     * Offsets 20 and 24
     */
    uint32_t mods_count;
    uint32_t mods_addr;

    /*
     * Offsets 28 through 43
     */
    uint32_t symbols[4];

    /*
     * Offsets 44 and 48
     */
    uint32_t mmap_length;
    uint32_t mmap_addr;

    /*
     * Offsets 52 and 56
     */
    uint32_t drives_length;
    uint32_t drives_addr;

    /*
     * Offset 60
     */
    uint32_t config_table;

    /*
     * Offset 64
     */
    uint32_t boot_loader_name;

    /*
     * Offset 68
     */
    uint32_t apm_table;

    /*
     * Offsets 72 and 76
     */
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;

    /*
     * Offsets 80 through 87
     */
    uint16_t vbe_mode;
    uint16_t vbe_interface_segment;
    uint16_t vbe_interface_offset;
    uint16_t vbe_interface_length;

    /*
     * Offset 88
     */
    uint64_t framebuffer_address;

    /*
     * Offset 96
     */
    uint32_t framebuffer_pitch;

    /*
     * Offsets 100 and 104
     */
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;

    /*
     * Offsets 108 and 109
     */
    uint8_t framebuffer_bits_per_pixel;
    uint8_t framebuffer_type;

    /*
     * Offsets 110 and 111
     *
     * The Multiboot color-information union begins on
     * a four-byte boundary, so two padding bytes occur
     * before its contents.
     */
    uint8_t framebuffer_color_padding_before[2];

    /*
     * RGB information begins at offset 112.
     */
    uint8_t framebuffer_red_position;
    uint8_t framebuffer_red_mask_size;

    uint8_t framebuffer_green_position;
    uint8_t framebuffer_green_mask_size;

    uint8_t framebuffer_blue_position;
    uint8_t framebuffer_blue_mask_size;

    /*
     * Complete the eight-byte color-information union.
     */
    uint8_t framebuffer_color_padding_after[2];

} __attribute__((packed)) multiboot_info_t;


/*
 * Compile-time offset checks.
 *
 * Compilation fails if this structure stops matching
 * the layout supplied by GRUB.
 */
typedef char check_framebuffer_address_offset[
    __builtin_offsetof(
        multiboot_info_t,
        framebuffer_address
    ) == 88 ? 1 : -1
];

typedef char check_framebuffer_pitch_offset[
    __builtin_offsetof(
        multiboot_info_t,
        framebuffer_pitch
    ) == 96 ? 1 : -1
];

typedef char check_framebuffer_width_offset[
    __builtin_offsetof(
        multiboot_info_t,
        framebuffer_width
    ) == 100 ? 1 : -1
];

typedef char check_framebuffer_bpp_offset[
    __builtin_offsetof(
        multiboot_info_t,
        framebuffer_bits_per_pixel
    ) == 108 ? 1 : -1
];

typedef char check_framebuffer_type_offset[
    __builtin_offsetof(
        multiboot_info_t,
        framebuffer_type
    ) == 109 ? 1 : -1
];

typedef char check_red_position_offset[
    __builtin_offsetof(
        multiboot_info_t,
        framebuffer_red_position
    ) == 112 ? 1 : -1
];

typedef char check_green_position_offset[
    __builtin_offsetof(
        multiboot_info_t,
        framebuffer_green_position
    ) == 114 ? 1 : -1
];

typedef char check_blue_position_offset[
    __builtin_offsetof(
        multiboot_info_t,
        framebuffer_blue_position
    ) == 116 ? 1 : -1
];

typedef char check_multiboot_info_size[
    sizeof(multiboot_info_t) == 120 ? 1 : -1
];


#endif