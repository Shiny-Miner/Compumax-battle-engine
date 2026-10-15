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
#include "battle_engine_hooks.h"
#include "constants/songs.h"
#include "constants/items.h"



void Hook_OakOldMan_HandleInputChooseAction(void)
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
BattleMenu_LightSelection();
        }
    }
    else if (JOY_NEW(DPAD_RIGHT))
    {
        if (!(gActionSelectionCursor[gActiveBattler] & 1)) // if is B_ACTION_USE_MOVE or B_ACTION_SWITCH
        {
            PlaySE(SE_SELECT);
            gActionSelectionCursor[gActiveBattler] ^= 1;
BattleMenu_LightSelection();
        }
    }
    else if (JOY_NEW(DPAD_UP))
    {
        if (gActionSelectionCursor[gActiveBattler] & 2) // if is B_ACTION_SWITCH or B_ACTION_RUN
        {
            PlaySE(SE_SELECT);
            gActionSelectionCursor[gActiveBattler] ^= 2;
BattleMenu_LightSelection();
        }
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        if (!(gActionSelectionCursor[gActiveBattler] & 2)) // if is B_ACTION_USE_MOVE or B_ACTION_USE_ITEM
        {
            PlaySE(SE_SELECT);
            gActionSelectionCursor[gActiveBattler] ^= 2;
BattleMenu_LightSelection();
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
void Hook_OakOldMan_SimulateInputChooseAction(void)
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
BattleMenu_LightSelection();
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
void Hook_OakOldMan_PrintTextWithMainBgDarkened(const u8 *text, u8 delay)
{
    // If delay is 0, it's treated as 256.
    switch (gBattleStruct->simulatedInputState[0])
    {
    case 0:
        if (!IsTextPrinterActive(0))
        {
            gBattleStruct->simulatedInputState[3] = delay;
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
            Hook_BtlCtrl_DrawVoiceoverMessageFrame();
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
            Hook_BtlCtrl_RemoveVoiceoverMessageFrame();
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
        {
            if (GetBattlerSide(gActiveBattler) == B_SIDE_PLAYER)
                OakOldManBufferExecCompleted();
            else
                OpponentBufferExecCompleted();
            gBattleCommunication[MSG_DISPLAY] = 0;
            gBattleStruct->simulatedInputState[0] = 0;
        }
        break;
    }
}

void Hook_BtlCtrl_DrawVoiceoverMessageFrame(void)
{
    FillBgTilemapBufferRect(0, 7, 	1, 	 15, 	28,  4, 	14);
}

// Redraws the engine's own battle message frame (tiles 3-0xD, palette 14, rows
// 14-19, columns 0-29 - identical to gBattleFramesTilemap).
// The ROM's BtlCtrl_RemoveVoiceoverMessageFrame restores the *vanilla* frame
// tiles, which show up as garbage on the bottom row / right column with the
// engine's tileset. PrintOakText_ForPetesSake / PrintOakText_KeepAnEyeOnHP still
// call it from the ROM, so it has to be replaced as well.
void Hook_BtlCtrl_RemoveVoiceoverMessageFrame(void)
{
    // top row
    FillBgTilemapBufferRect(0, 3,    0, 14,  1, 1, 14);
    FillBgTilemapBufferRect(0, 4,    1, 14, 28, 1, 14);
    FillBgTilemapBufferRect(0, 5,   29, 14,  1, 1, 14);
    // left / right edges
    FillBgTilemapBufferRect(0, 6,    0, 15,  1, 3, 14);
    FillBgTilemapBufferRect(0, 8,   29, 15,  1, 3, 14);
    FillBgTilemapBufferRect(0, 9,    0, 18,  1, 1, 14);
    FillBgTilemapBufferRect(0, 10,  29, 18,  1, 1, 14);
    // interior
    FillBgTilemapBufferRect(0, 7,    1, 15, 28, 4, 14);
    // bottom row
    FillBgTilemapBufferRect(0, 11,   0, 19,  1, 1, 14);
    FillBgTilemapBufferRect(0, 12,   1, 19, 28, 1, 14);
    FillBgTilemapBufferRect(0, 13,  29, 19,  1, 1, 14);
    CopyBgTilemapBufferToVram(0);
}
