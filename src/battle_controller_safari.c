#include "global.h"
#include "gflib.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_controllers.h"
#include "battle_interface.h"
#include "battle_message.h"
#include "data.h"
#include "item_menu.h"
#include "link.h"
#include "main.h"
#include "pokeball.h"
#include "util.h"
#include "graphics.h"
#include "strings.h"
#include "constants/songs.h"
#include "constants/battle_anim.h"

static void SafariLightSelectionMenu(void);

static void HandleInputChooseAction(void)
{
    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);

        switch (gActionSelectionCursor[gActiveBattler])
        {
        case 0:
            BtlController_EmitTwoReturnValues(1, B_ACTION_SAFARI_BALL, 0);
            break;
        case 1:
            BtlController_EmitTwoReturnValues(1, B_ACTION_SAFARI_BAIT, 0);
            break;
        case 2:
            BtlController_EmitTwoReturnValues(1, B_ACTION_SAFARI_GO_NEAR, 0);
            break;
        case 3:
            BtlController_EmitTwoReturnValues(1, B_ACTION_SAFARI_RUN, 0);
            break;
        }
        SafariBufferExecCompleted();
    }
    else if (JOY_NEW(DPAD_LEFT))
    {
        if (gActionSelectionCursor[gActiveBattler] & 1)
        {
            PlaySE(SE_SELECT);
            gActionSelectionCursor[gActiveBattler] ^= 1;
			SafariLightSelectionMenu();
        }
    }
    else if (JOY_NEW(DPAD_RIGHT))
    {
        if (!(gActionSelectionCursor[gActiveBattler] & 1))
        {
            PlaySE(SE_SELECT);
            gActionSelectionCursor[gActiveBattler] ^= 1;
			SafariLightSelectionMenu();
        }
    }
    else if (JOY_NEW(DPAD_UP))
    {
        if (gActionSelectionCursor[gActiveBattler] & 2)
        {
            PlaySE(SE_SELECT);
            gActionSelectionCursor[gActiveBattler] ^= 2;
			SafariLightSelectionMenu();
        }
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        if (!(gActionSelectionCursor[gActiveBattler] & 2))
        {
            PlaySE(SE_SELECT);
            gActionSelectionCursor[gActiveBattler] ^= 2;
            SafariLightSelectionMenu();
        }
    }
}

static void HandleChooseActionAfterDma3(void)
{
    if (!IsDma3ManagerBusyWithBgCopy())
    {
        gBattle_BG0_X = 0;
        gBattle_BG0_Y = 160;
		SafariLightSelectionMenu();
        gBattlerControllerFuncs[gActiveBattler] = HandleInputChooseAction;
    }
}

static void SafariLightSelectionMenu(void)
{
	u16 palette[1] = {RGB( 31, 31, 31)};
	
	LoadPalette(gBattleFramesPalette, 0xE0, 0x20);
	switch (gActionSelectionCursor[gActiveBattler])
	{
	case 0:
		LoadPalette(&palette, 0xEC, 2);
		break;
	case 1:
		LoadPalette(&palette, 0xED, 2);
		break;
	case 2:
		LoadPalette(&palette, 0xEE, 2);
		break;
	case 3:
		LoadPalette(&palette, 0xEF, 2);
		break;
	}
}
