#include "keyboard.h"
#include "io.h"


static unsigned char key_down[KEY_COUNT];
static unsigned char key_pressed[KEY_COUNT];

static int extended_prefix = 0;


/*
 * Basic US keyboard character table.
 */
static const char scancode_map[128] =
{
    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',
    [0x06] = '5',
    [0x07] = '6',
    [0x08] = '7',
    [0x09] = '8',
    [0x0A] = '9',
    [0x0B] = '0',

    [0x10] = 'q',
    [0x11] = 'w',
    [0x12] = 'e',
    [0x13] = 'r',
    [0x14] = 't',
    [0x15] = 'y',
    [0x16] = 'u',
    [0x17] = 'i',
    [0x18] = 'o',
    [0x19] = 'p',

    [0x1C] = '\n',

    [0x1E] = 'a',
    [0x1F] = 's',
    [0x20] = 'd',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',

    [0x2C] = 'z',
    [0x2D] = 'x',
    [0x2E] = 'c',
    [0x2F] = 'v',
    [0x30] = 'b',
    [0x31] = 'n',
    [0x32] = 'm',

    [0x39] = ' '
};


static keyboard_key_t keyboard_translate_scancode(
    unsigned char scancode,
    int extended
)
{
    if (extended)
    {
        switch (scancode)
        {
            case 0x48:
                return KEY_UP;

            case 0x50:
                return KEY_DOWN;

            case 0x4B:
                return KEY_LEFT;

            case 0x4D:
                return KEY_RIGHT;

            case 0x1C:
                return KEY_ENTER;

            default:
                return KEY_NONE;
        }
    }


    switch (scancode)
    {
        case 0x01:
            return KEY_ESCAPE;

        case 0x1C:
            return KEY_ENTER;

        case 0x11:
            return KEY_W;

        case 0x1F:
            return KEY_S;

        /*
         * A and D continue working as menu controls.
         */
        case 0x1E:
            return KEY_LEFT;

        case 0x20:
            return KEY_RIGHT;

        default:
            return KEY_NONE;
    }
}


void keyboard_update(void)
{
    /*
     * Read every scan code waiting in the keyboard
     * controller.
     */
    while ((inb(0x64) & 0x01) != 0)
    {
        unsigned char raw_scancode =
            inb(0x60);


        /*
         * Arrow keys begin with the 0xE0 prefix.
         */
        if (raw_scancode == 0xE0)
        {
            extended_prefix = 1;
            continue;
        }


        int released =
            (raw_scancode & 0x80) != 0;

        unsigned char scancode =
            raw_scancode & 0x7F;


        keyboard_key_t key =
            keyboard_translate_scancode(
                scancode,
                extended_prefix
            );


        extended_prefix = 0;


        if (
            key <= KEY_NONE ||
            key >= KEY_COUNT
        )
        {
            continue;
        }


        if (released)
        {
            key_down[key] = 0;
        }
        else
        {
            /*
             * Only register a new press when the key
             * was not already held.
             */
            if (!key_down[key])
            {
                key_pressed[key] = 1;
            }

            key_down[key] = 1;
        }
    }
}


int keyboard_is_key_down(
    keyboard_key_t key
)
{
    keyboard_update();

    if (
        key <= KEY_NONE ||
        key >= KEY_COUNT
    )
    {
        return 0;
    }

    return key_down[key] != 0;
}


int keyboard_was_key_pressed(
    keyboard_key_t key
)
{
    keyboard_update();

    if (
        key <= KEY_NONE ||
        key >= KEY_COUNT
    )
    {
        return 0;
    }

    if (!key_pressed[key])
    {
        return 0;
    }

    key_pressed[key] = 0;

    return 1;
}


keyboard_key_t keyboard_poll_key(void)
{
    keyboard_update();


    static const keyboard_key_t order[] =
    {
        KEY_ESCAPE,
        KEY_ENTER,

        KEY_LEFT,
        KEY_RIGHT,
        KEY_UP,
        KEY_DOWN,

        KEY_W,
        KEY_S
    };


    unsigned int count =
        sizeof(order) /
        sizeof(order[0]);


    for (
        unsigned int index = 0;
        index < count;
        index++
    )
    {
        keyboard_key_t key =
            order[index];

        if (key_pressed[key])
        {
            key_pressed[key] = 0;

            return key;
        }
    }


    return KEY_NONE;
}


int keyboard_enter_pressed(void)
{
    return keyboard_was_key_pressed(
        KEY_ENTER
    );
}


char keyboard_getchar(void)
{
    while (1)
    {
        if ((inb(0x64) & 0x01) == 0)
        {
            continue;
        }

        unsigned char scancode =
            inb(0x60);


        if (
            scancode == 0xE0 ||
            (scancode & 0x80) != 0
        )
        {
            continue;
        }


        if (
            scancode < sizeof(scancode_map) &&
            scancode_map[scancode] != '\0'
        )
        {
            return scancode_map[scancode];
        }
    }
}

void keyboard_clear_pressed_events(void)
{
    int key;

    /*
     * Read any waiting keyboard scancodes first.
     */
    keyboard_update();

    /*
     * Remove all stored one-time press events.
     *
     * Physical held-key states remain inside key_down.
     */
    for (key = 0; key < KEY_COUNT; key++)
    {
        key_pressed[key] = 0;
    }
}