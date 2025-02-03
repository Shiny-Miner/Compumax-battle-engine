#ifndef GUARD_BATTLE_TRANSITION_H
#define GUARD_BATTLE_TRANSITION_H

#include "global.h"

enum
{
    MUGSHOT_LORELEI,
    MUGSHOT_BRUNO,
    MUGSHOT_AGATHA,
    MUGSHOT_LANCE,
    MUGSHOT_BLUE,
    MUGSHOTS_COUNT
};

#define B_TRANSITION_BLUR 0
#define B_TRANSITION_DISTORTED_WAVE 1
#define B_TRANSITION_HORIZONTAL_CORRUGATE 2
#define B_TRANSITION_BIG_POKEBALL 3
#define B_TRANSITION_SLIDING_POKEBALLS 4
#define B_TRANSITION_CLOCKWISE_BLACKFADE 5
#define B_TRANSITION_FULLSCREEN_WAVE 6
#define B_TRANSITION_BLACK_WAVE_TO_RIGHT 7
#define B_TRANSITION_SLICED_SCREEN 8
#define B_TRANSITION_WHITEFADE_IN_STRIPES 9
#define B_TRANSITION_GRID_SQUARES 10
#define B_TRANSITION_BLACK_DOODLES 11
#define B_TRANSITION_LORELEI 12
#define B_TRANSITION_BRUNO 13
#define B_TRANSITION_AGATHA 14
#define B_TRANSITION_LANCE 15
#define B_TRANSITION_BLUE 16
#define B_TRANSITION_ANTI_CLOCKWISE_SPIRAL 17

extern const struct SpritePalette sSpritePalette_SlidingPokeball;

bool8 BT_IsDone(void);
void BT_StartOnField(u8 transitionId);
bool8 FldEff_Pokeball(void);

#endif // GUARD_BATTLE_TRANSITION_H
