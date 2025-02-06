#include "global.h"
#include "gflib.h"
#include "graphics.h"
#include "task.h"
#include "pokeball.h"
#include "party_menu.h"
#include "util.h"
#include "m4a.h"
#include "link.h"
#include "item.h"
#include "item_menu.h"
#include "strings.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_controllers.h"
#include "battle_interface.h"
#include "battle_message.h"
#include "reshow_battle_screen.h"
#include "battle_string_ids.h"
#include "constants/songs.h"
#include "constants/items.h"

static void PrintOakTextWithMainBgDarkened(const u8 *text, u8 a1);
static void LightSelectionMenu(void);
static void HandleInputChooseAction(void);


static void HandleInputChooseAction(void)
{
    // Like player, but specifically for Rival in Oak's Lab
    u16 itemId = gBattleBufferA[gActiveBattler][2] | (gBattleBufferA[gActiveBattler][3] << 8);

    DoBounceEffect(gActiveBattler, BOUNCE_HEALTHBOX, 7, 1);
    DoBounceEffect(gActiveBattler, BOUNCE_MON, 7, 1);
    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);

        switch (gActionSelectionCursor[gActiveBattler])
        {
        case 0:
            BtlController_EmitTwoReturnValues(1, B_ACTION_USE_MOVE, 0);
            break;
        case 1:
            BtlController_EmitTwoReturnValues(1, B_ACTION_USE_ITEM, 0);
            break;
        case 2:
            BtlController_EmitTwoReturnValues(1, B_ACTION_SWITCH, 0);
            break;
        case 3:
            BtlController_EmitTwoReturnValues(1, B_ACTION_RUN, 0);
            break;
        }
        OakOldManBufferExecCompleted();
    }
    else if (JOY_NEW(DPAD_LEFT))
    {
        if (gActionSelectionCursor[gActiveBattler] & 1) // if is B_ACTION_USE_ITEM or B_ACTION_RUN
        {
            PlaySE(SE_SELECT);
            gActionSelectionCursor[gActiveBattler] ^= 1;
			LightSelectionMenu();
        }
    }
    else if (JOY_NEW(DPAD_RIGHT))
    {
        if (!(gActionSelectionCursor[gActiveBattler] & 1)) // if is B_ACTION_USE_MOVE or B_ACTION_SWITCH
        {
            PlaySE(SE_SELECT);
            gActionSelectionCursor[gActiveBattler] ^= 1;
			LightSelectionMenu();
        }
    }
    else if (JOY_NEW(DPAD_UP))
    {
        if (gActionSelectionCursor[gActiveBattler] & 2) // if is B_ACTION_SWITCH or B_ACTION_RUN
        {
            PlaySE(SE_SELECT);
            gActionSelectionCursor[gActiveBattler] ^= 2;
			LightSelectionMenu();
        }
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        if (!(gActionSelectionCursor[gActiveBattler] & 2)) // if is B_ACTION_USE_MOVE or B_ACTION_USE_ITEM
        {
            PlaySE(SE_SELECT);
            gActionSelectionCursor[gActiveBattler] ^= 2;
			LightSelectionMenu();
        }
    }
    else if (JOY_NEW(B_BUTTON))
    {
        if ((gBattleTypeFlags & BATTLE_TYPE_DOUBLE)
         && GetBattlerPosition(gActiveBattler) == B_POSITION_PLAYER_RIGHT
         && !(gAbsentBattlerFlags & gBitTable[GetBattlerAtPosition(B_POSITION_PLAYER_LEFT)])
         && !(gBattleTypeFlags & BATTLE_TYPE_MULTI))
        {
            if (gBattleBufferA[gActiveBattler][1] == B_ACTION_USE_ITEM)
            {
                // Add item to bag if it is a ball
                if (itemId <= ITEM_PREMIER_BALL)
                    AddBagItem(itemId, 1);
                else
                    return;
            }
            PlaySE(SE_SELECT);
            BtlController_EmitTwoReturnValues(1, B_ACTION_CANCEL_PARTNER, 0);
            OakOldManBufferExecCompleted();
        }
    }
    else if (JOY_NEW(START_BUTTON))
    {
        SwapHpBarsWithHpText();
    }
}
static void SimulateInputChooseAction(void)
{
    // Old Man
    switch (gBattleStruct->simulatedInputState[0])
    {
    case 0:
        gBattleStruct->simulatedInputState[2] = 64;
        ++gBattleStruct->simulatedInputState[0];
        // fall through
    case 1:
        if (--gBattleStruct->simulatedInputState[2] == 0)
        {
            // Move cursor to BAG
            PlaySE(SE_SELECT);
			gActionSelectionCursor[gActiveBattler] ^= 1;
			LightSelectionMenu();
            gBattleStruct->simulatedInputState[2] = 64;
            ++gBattleStruct->simulatedInputState[0];
        }
        break;
    case 2:
        if (--gBattleStruct->simulatedInputState[2] == 0)
        {
            // Open bag
            PlaySE(SE_SELECT);
            BtlController_EmitTwoReturnValues(1, B_ACTION_USE_ITEM, 0);
            OakOldManBufferExecCompleted();
        }
        break;
    }
}
static void PrintOakTextWithMainBgDarkened(const u8 *text, u8 a2)
{
    // If delay is 0, it's treated as 256.
    switch (gBattleStruct->simulatedInputState[0])
    {
    case 0:
        if (!IsTextPrinterActive(0))
        {
            ++gBattleStruct->simulatedInputState[0];
        }
        break;
    case 1:
        if (--gBattleStruct->simulatedInputState[3] == 0)
        {
             BeginNormalPaletteFade(0xFFFF3FFF,
                                   4, 
                                   0, 
                                   8, 
                                   RGB_BLACK);
            ++gBattleStruct->simulatedInputState[0];
        }
        break;
    case 2:
        if (!gPaletteFade.active)
        {
            BtlCtrl_DrawVoiceoverMessageFrame();
            ++gBattleStruct->simulatedInputState[0];
        }
        break;
    case 3:
        BattleStringExpandPlaceholdersToDisplayedString(text);
        BattlePutTextOnWindow(gDisplayedStringBattle, 24);
        ++gBattleStruct->simulatedInputState[0];
        break;
    case 4:
        if (!IsTextPrinterActive(24))
        {
                BeginNormalPaletteFade(0xFFFF3FFF,
                                   4,
                                   8,
                                   0,
                                   RGB_BLACK);
            ++gBattleStruct->simulatedInputState[0];
        }
        break;
    case 5:
        if (!gPaletteFade.active)
        {if (GetBattlerSide(gActiveBattler) == B_SIDE_PLAYER)
                OakOldManBufferExecCompleted();
            else
                OpponentBufferExecCompleted();
            gBattleCommunication[MSG_DISPLAY] = 0;
            gBattleStruct->simulatedInputState[0] = 0;
        }
        break;
    }
}

void BtlCtrl_DrawVoiceoverMessageFrame(void)
{
    	FillBgTilemapBufferRect(0, 7, 	1, 	 15, 	28,  4, 	14);
}
static void LightSelectionMenu(void)
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
