#ifndef GAME_SELECT_H
#define GAME_SELECT_H

typedef enum
{
    GAME_SELECTION_BACK = 0,
    GAME_SELECTION_PONG

} game_selection_t;


/*
 * Displays the game-selection screen.
 *
 * It remains active until a game is selected
 * or Escape is pressed.
 */
game_selection_t game_select_run(void);

#endif