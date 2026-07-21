#ifndef KEYBOARD_H
#define KEYBOARD_H

typedef enum
{
    KEY_NONE = 0,

    KEY_ENTER,
    KEY_ESCAPE,

    KEY_LEFT,
    KEY_RIGHT,
    KEY_UP,
    KEY_DOWN,

    KEY_W,
    KEY_S,

    KEY_COUNT

} keyboard_key_t;


/*
 * Reads all pending keyboard scan codes and updates
 * ArcadeOS's record of which keys are held.
 */
void keyboard_update(void);


/*
 * Returns 1 while a key is being held down.
 */
int keyboard_is_key_down(
    keyboard_key_t key
);


/*
 * Returns 1 once when a key is initially pressed.
 */
int keyboard_was_key_pressed(
    keyboard_key_t key
);


/*
 * Returns one newly pressed key.
 *
 * Used by menus.
 */
keyboard_key_t keyboard_poll_key(void);


/*
 * Compatibility function for the title screen.
 */
int keyboard_enter_pressed(void);


/*
 * Older blocking text-input function.
 */
char keyboard_getchar(void);

void keyboard_clear_pressed_events(void);

#endif

