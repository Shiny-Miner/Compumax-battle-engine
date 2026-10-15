#include "global.h"
#include "gflib.h"
#include "data.h"
#include "decompress.h"
#include "item.h"
#include "string_util.h"
#include "item_menu.h"
#include "link.h"
#include "text.h"
#include "m4a.h"
#include "menu.h"
#include "party_menu.h"
#include "pokeball.h"
#include "strings.h"
#include "pokemon_special_anim.h"
#include "task.h"
#include "util.h"
#include "graphics.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_controllers.h"
#include "battle_interface.h"
#include "battle_message.h"
#include "battle_script_commands.h"
#include "battle_engine_hooks.h"
#include "reshow_battle_screen.h"
#include "constants/battle_anim.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/songs.h"

#define MOVE_CATEGORY_PHYSICAL 0
#define MOVE_CATEGORY_SPECIAL  1
#define MOVE_CATEGORY_STATUS   2
// Icon ids passed to BlitMoveInfoIcon: types use (type + 1), categories use
// (category + MOVE_CATEGORY_ICON_BASE). The vanilla icon table is shorter than
// this, so the category icons need the extended table/graphics.
#define MOVE_CATEGORY_ICON_BASE 50

static u8 GetMoveCategory(u16 move);
static void MoveSelectionDisplayTypeIcon(void);
static void MoveSelectionDisplayCategoryIcon(void);

// ---------------------------------------------------------------------------
// Move-selection drawing
//
// The new frame has no tilemap arrow cursor and the vanilla text helpers pick
// their colours from ROM tables that point at palette slots this engine does
// not load. So the move names, the PP label and the PP numbers are drawn here
// with explicit colour indices into palette 15 (text.pal + the four slots
// written by Hook_LoadBattleMenuWindowGfx).
//
//   index 14 (0xFE) white        - window background, matches the frame fill
//   index 13 (0xFD) dark grey    - normal text
//   index 15 (0xFF) light grey   - text shadow
//   index  4 (text.pal) red      - selected move / low PP
//   index  6 (text.pal) green    - move picked up for swapping
//
// Colour arrays are { background, foreground, shadow }.
// ---------------------------------------------------------------------------
#define BTLTXT_FONT          2
#define BTLTXT_BG            14

// Window ids from gBattleWindowTemplates_Engine
#define WIN_MOVE_NAME_1      3
#define WIN_PP_LABEL         7
#define WIN_PP_NUMBER        8

#define NO_SWAP_SOURCE       0xFF

static const u8 sColors_Normal[]   = {BTLTXT_BG, 13, 15};
static const u8 sColors_Selected[] = {BTLTXT_BG, 4, 15};
static const u8 sColors_SwapSrc[]  = {BTLTXT_BG, 6, 15};
static const u8 sColors_LowPp[]    = {BTLTXT_BG, 4, 15};

static const u8 sText_PP[] = _("PP");

// Move being carried while the SELECT swap is active.
static u8 sSwapSource = NO_SWAP_SOURCE;

static void PrintBattleText(u8 windowId, u8 x, u8 y, const u8 *colors, const u8 *str)
{
    FillWindowPixelBuffer(windowId, PIXEL_FILL(BTLTXT_BG));
    AddTextPrinterParameterized3(windowId, BTLTXT_FONT, x, y, colors, 0, str);
    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_BOTH);
}

// Redraws all four move names. `selected` is shown in red, `swapSource` (if
// any) in green. The same function serves normal selection and swap mode.
static void DrawMoveNames(u8 selected, u8 swapSource)
{
    s32 i;
    struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleBufferA[gActiveBattler][4]);

    gNumberOfMovesToChoose = 0;
    for (i = 0; i < MAX_MON_MOVES; ++i)
    {
        const u8 *colors = sColors_Normal;

        if (i == swapSource)
            colors = sColors_SwapSrc;
        else if (i == selected)
            colors = sColors_Selected;
        PrintBattleText(WIN_MOVE_NAME_1 + i, 0, 1, colors, gMoveNames[moveInfo->moves[i]]);
        if (moveInfo->moves[i] != MOVE_NONE)
            ++gNumberOfMovesToChoose;
    }
}



void Hook_Player_HandleInputChooseAction(void)
{
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
        PlayerBufferExecCompleted();
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
            PlayerBufferExecCompleted();
        }
    }
    else if (JOY_NEW(START_BUTTON))
    {
        SwapHpBarsWithHpText();
    }
}

void Hook_Player_HandleInputChooseTarget(void)
{
    s32 i;
    u8 identities[4];

    memcpy(identities, sTargetIdentities, NELEMS(sTargetIdentities));
    DoBounceEffect(gMultiUsePlayerCursor, BOUNCE_HEALTHBOX, 15, 1);
    i = 0;
    if (gBattlersCount != 0)
    {
        do
        {
            if (i != gMultiUsePlayerCursor)
                EndBounceEffect(i, BOUNCE_HEALTHBOX);
            ++i;
        }
        while (i < gBattlersCount);
    }
    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = SpriteCb_HideAsMoveTarget;
        BtlController_EmitTwoReturnValues(1, 10, gMoveSelectionCursor[gActiveBattler] | (gMultiUsePlayerCursor << 8));
        EndBounceEffect(gMultiUsePlayerCursor, BOUNCE_HEALTHBOX);
        PlayerBufferExecCompleted();
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = SpriteCb_HideAsMoveTarget;
        gBattlerControllerFuncs[gActiveBattler] = HandleInputChooseMove;
        DoBounceEffect(gActiveBattler, BOUNCE_HEALTHBOX, 7, 1);
        DoBounceEffect(gActiveBattler, BOUNCE_MON, 7, 1);
        EndBounceEffect(gMultiUsePlayerCursor, BOUNCE_HEALTHBOX);
    }
    else if (JOY_NEW(DPAD_LEFT | DPAD_UP))
    {
        PlaySE(SE_SELECT);
        gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = SpriteCb_HideAsMoveTarget;

        do
        {
            u8 currSelIdentity = GetBattlerPosition(gMultiUsePlayerCursor);

            for (i = 0; i < MAX_BATTLERS_COUNT; ++i)
                if (currSelIdentity == identities[i])
                    break;
            do
            {
                if (--i < 0)
                    i = MAX_BATTLERS_COUNT; // UB: array out of range
                gMultiUsePlayerCursor = GetBattlerAtPosition(identities[i]);
            }
            while (gMultiUsePlayerCursor == gBattlersCount);
            i = 0;
            switch (GetBattlerPosition(gMultiUsePlayerCursor))
            {
            case B_POSITION_PLAYER_LEFT:
            case B_POSITION_PLAYER_RIGHT:
                if (gActiveBattler != gMultiUsePlayerCursor)
                    ++i;
                else if (gBattleMoves[GetMonData(&gPlayerParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_MOVE1 + gMoveSelectionCursor[gActiveBattler])].target & MOVE_TARGET_USER_OR_SELECTED)
                    ++i;
                break;
            case B_POSITION_OPPONENT_LEFT:
            case B_POSITION_OPPONENT_RIGHT:
                ++i;
                break;
            }
            if (gAbsentBattlerFlags & gBitTable[gMultiUsePlayerCursor])
                i = 0;
        }
        while (i == 0);
        gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = SpriteCb_ShowAsMoveTarget;
    }
    else if (JOY_NEW(DPAD_RIGHT | DPAD_DOWN))
    {
        PlaySE(SE_SELECT);
        gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = SpriteCb_HideAsMoveTarget;

        do
        {
            u8 currSelIdentity = GetBattlerPosition(gMultiUsePlayerCursor);

            for (i = 0; i < MAX_BATTLERS_COUNT; ++i)
                if (currSelIdentity == identities[i])
                    break;
            do
            {
                if (++i > 3)
                    i = 0;
                gMultiUsePlayerCursor = GetBattlerAtPosition(identities[i]);
            }
            while (gMultiUsePlayerCursor == gBattlersCount);
            i = 0;
            switch (GetBattlerPosition(gMultiUsePlayerCursor))
            {
            case B_POSITION_PLAYER_LEFT:
            case B_POSITION_PLAYER_RIGHT:
                if (gActiveBattler != gMultiUsePlayerCursor)
                    ++i;
                else if (gBattleMoves[GetMonData(&gPlayerParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_MOVE1 + gMoveSelectionCursor[gActiveBattler])].target & MOVE_TARGET_USER_OR_SELECTED)
                    ++i;
                break;
            case B_POSITION_OPPONENT_LEFT:
            case B_POSITION_OPPONENT_RIGHT:
                ++i;
                break;
            }
            if (gAbsentBattlerFlags & gBitTable[gMultiUsePlayerCursor])
                i = 0;
        }
        while (i == 0);
        gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = SpriteCb_ShowAsMoveTarget;
    }
}

void Hook_Player_HandleMoveSwitching(void)
{
    u8 perMovePPBonuses[4];
    struct ChooseMoveStruct moveStruct;
    u8 totalPPBonuses;

    if (JOY_NEW(A_BUTTON | SELECT_BUTTON))
    {
        PlaySE(SE_SELECT);

        if (gMoveSelectionCursor[gActiveBattler] != gMultiUsePlayerCursor)
        {
            struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleBufferA[gActiveBattler][4]);
            s32 i;

            // swap moves and pp
            i = moveInfo->moves[gMoveSelectionCursor[gActiveBattler]];
            moveInfo->moves[gMoveSelectionCursor[gActiveBattler]] = moveInfo->moves[gMultiUsePlayerCursor];
            moveInfo->moves[gMultiUsePlayerCursor] = i;
            i = moveInfo->currentPp[gMoveSelectionCursor[gActiveBattler]];
            moveInfo->currentPp[gMoveSelectionCursor[gActiveBattler]] = moveInfo->currentPp[gMultiUsePlayerCursor];
            moveInfo->currentPp[gMultiUsePlayerCursor] = i;
            i = moveInfo->maxPp[gMoveSelectionCursor[gActiveBattler]];
            moveInfo->maxPp[gMoveSelectionCursor[gActiveBattler]] = moveInfo->maxPp[gMultiUsePlayerCursor];
            moveInfo->maxPp[gMultiUsePlayerCursor] = i;
            if (gDisableStructs[gActiveBattler].mimickedMoves & gBitTable[gMoveSelectionCursor[gActiveBattler]])
            {
                gDisableStructs[gActiveBattler].mimickedMoves &= (~gBitTable[gMoveSelectionCursor[gActiveBattler]]);
                gDisableStructs[gActiveBattler].mimickedMoves |= gBitTable[gMultiUsePlayerCursor];
            }
            for (i = 0; i < MAX_MON_MOVES; ++i)
                perMovePPBonuses[i] = (gBattleMons[gActiveBattler].ppBonuses & (3 << (i * 2))) >> (i * 2);
            totalPPBonuses = perMovePPBonuses[gMoveSelectionCursor[gActiveBattler]];
            perMovePPBonuses[gMoveSelectionCursor[gActiveBattler]] = perMovePPBonuses[gMultiUsePlayerCursor];
            perMovePPBonuses[gMultiUsePlayerCursor] = totalPPBonuses;
            totalPPBonuses = 0;
            for (i = 0; i < MAX_MON_MOVES; ++i)
                totalPPBonuses |= perMovePPBonuses[i] << (i * 2);

            gBattleMons[gActiveBattler].ppBonuses = totalPPBonuses;
            for (i = 0; i < MAX_MON_MOVES; ++i)
            {
                gBattleMons[gActiveBattler].moves[i] = moveInfo->moves[i];
                gBattleMons[gActiveBattler].pp[i] = moveInfo->currentPp[i];
            }
            if (!(gBattleMons[gActiveBattler].status2 & STATUS2_TRANSFORMED))
            {
                for (i = 0; i < MAX_MON_MOVES; ++i)
                {
                    moveStruct.moves[i] = GetMonData(&gPlayerParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_MOVE1 + i);
                    moveStruct.currentPp[i] = GetMonData(&gPlayerParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_PP1 + i);
                }

                totalPPBonuses = GetMonData(&gPlayerParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_PP_BONUSES);
                for (i = 0; i < MAX_MON_MOVES; ++i)
                    perMovePPBonuses[i] = (totalPPBonuses & (3 << (i * 2))) >> (i * 2);
                i = moveStruct.moves[gMoveSelectionCursor[gActiveBattler]];
                moveStruct.moves[gMoveSelectionCursor[gActiveBattler]] = moveStruct.moves[gMultiUsePlayerCursor];
                moveStruct.moves[gMultiUsePlayerCursor] = i;
                i = moveStruct.currentPp[gMoveSelectionCursor[gActiveBattler]];
                moveStruct.currentPp[gMoveSelectionCursor[gActiveBattler]] = moveStruct.currentPp[gMultiUsePlayerCursor];
                moveStruct.currentPp[gMultiUsePlayerCursor] = i;
                totalPPBonuses = perMovePPBonuses[gMoveSelectionCursor[gActiveBattler]];
                perMovePPBonuses[gMoveSelectionCursor[gActiveBattler]] = perMovePPBonuses[gMultiUsePlayerCursor];
                perMovePPBonuses[gMultiUsePlayerCursor] = totalPPBonuses;
                totalPPBonuses = 0;
                for (i = 0; i < MAX_MON_MOVES; ++i)
                    totalPPBonuses |= perMovePPBonuses[i] << (i * 2);
                for (i = 0; i < MAX_MON_MOVES; ++i)
                {
                    SetMonData(&gPlayerParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_MOVE1 + i, &moveStruct.moves[i]);
                    SetMonData(&gPlayerParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_PP1 + i, &moveStruct.currentPp[i]);
                }
                SetMonData(&gPlayerParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_PP_BONUSES, &totalPPBonuses);
            }
        }
        if (gBattleTypeFlags & BATTLE_TYPE_FIRST_BATTLE)
            gBattlerControllerFuncs[gActiveBattler] = OakOldManHandleInputChooseMove;
        else
            gBattlerControllerFuncs[gActiveBattler] = HandleInputChooseMove;
        gMoveSelectionCursor[gActiveBattler] = gMultiUsePlayerCursor;
        sSwapSource = NO_SWAP_SOURCE;
        DrawMoveNames(gMoveSelectionCursor[gActiveBattler], NO_SWAP_SOURCE);
        Hook_Player_MoveSelectionDisplayPpString();
        Hook_Player_MoveSelectionDisplayPpNumber();
        Hook_Player_MoveSelectionDisplayMoveType();
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (gBattleTypeFlags & BATTLE_TYPE_FIRST_BATTLE)
            gBattlerControllerFuncs[gActiveBattler] = OakOldManHandleInputChooseMove;
        else
            gBattlerControllerFuncs[gActiveBattler] = HandleInputChooseMove;
        sSwapSource = NO_SWAP_SOURCE;
        DrawMoveNames(gMoveSelectionCursor[gActiveBattler], NO_SWAP_SOURCE);
        Hook_Player_MoveSelectionDisplayPpString();
        Hook_Player_MoveSelectionDisplayPpNumber();
        Hook_Player_MoveSelectionDisplayMoveType();
    }
    if (JOY_NEW(DPAD_LEFT))
    {
        if (gMultiUsePlayerCursor & 1)
        {
            gMultiUsePlayerCursor ^= 1;
            PlaySE(SE_SELECT);
            DrawMoveNames(gMultiUsePlayerCursor, gMoveSelectionCursor[gActiveBattler]);
        }
    }
    if (JOY_NEW(DPAD_RIGHT))
    {
        if (!(gMultiUsePlayerCursor & 1) && (gMultiUsePlayerCursor ^ 1) < gNumberOfMovesToChoose)
        {
            gMultiUsePlayerCursor ^= 1;
            PlaySE(SE_SELECT);
            DrawMoveNames(gMultiUsePlayerCursor, gMoveSelectionCursor[gActiveBattler]);
        }
    }
    if (JOY_NEW(DPAD_UP))
    {
        if (gMultiUsePlayerCursor & 2)
        {
            gMultiUsePlayerCursor ^= 2;
            PlaySE(SE_SELECT);
            DrawMoveNames(gMultiUsePlayerCursor, gMoveSelectionCursor[gActiveBattler]);
        }
    }
    if (JOY_NEW(DPAD_DOWN))
    {
        if (!(gMultiUsePlayerCursor & 2) && (gMultiUsePlayerCursor ^ 2) < gNumberOfMovesToChoose)
        {
            gMultiUsePlayerCursor ^= 2;
            PlaySE(SE_SELECT);
            DrawMoveNames(gMultiUsePlayerCursor, gMoveSelectionCursor[gActiveBattler]);
        }
    }
}

void Hook_Player_MoveSelectionDisplayMoveNames(void)
{
    DrawMoveNames(gMoveSelectionCursor[gActiveBattler], sSwapSource);
}

// Replaces vanilla MoveSelectionDisplayPpString.
void Hook_Player_MoveSelectionDisplayPpString(void)
{
    PrintBattleText(WIN_PP_LABEL, 0, 1, sColors_Normal, sText_PP);
}

// Replaces vanilla MoveSelectionDisplayPpNumber. The vanilla one recolours
// slots 91/92 of the palette buffer (palette 5), which no window uses here.
void Hook_Player_MoveSelectionDisplayPpNumber(void)
{
    u8 *txtPtr;
    u8 cur, max;
    const u8 *colors = sColors_Normal;
    struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleBufferA[gActiveBattler][4]);

    if (gBattleBufferA[gActiveBattler][2] == TRUE) // caller asked for no PP number
        return;

    cur = moveInfo->currentPp[gMoveSelectionCursor[gActiveBattler]];
    max = moveInfo->maxPp[gMoveSelectionCursor[gActiveBattler]];
    if (cur == 0 || cur * 4 <= max)
        colors = sColors_LowPp;

    txtPtr = ConvertIntToDecimalStringN(gDisplayedStringBattle, cur, STR_CONV_MODE_RIGHT_ALIGN, 2);
    *txtPtr++ = CHAR_SLASH;
    ConvertIntToDecimalStringN(txtPtr, max, STR_CONV_MODE_RIGHT_ALIGN, 2);
    PrintBattleText(WIN_PP_NUMBER, 0, 1, colors, gDisplayedStringBattle);
}

static u8 GetMoveCategory(u16 move)
{
    const struct BattleMove *data = &gBattleMoves[move];

    // Gen 3 has no per-move category: status moves have no power, everything
    // else is physical/special depending on its type.
    if (data->power == 0)
        return MOVE_CATEGORY_STATUS;
    if (IS_TYPE_PHYSICAL(data->type))
        return MOVE_CATEGORY_PHYSICAL;
    return MOVE_CATEGORY_SPECIAL;
}

static void MoveSelectionDisplayTypeIcon(void)
{
    struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleBufferA[gActiveBattler][4]);
    u8 type = gBattleMoves[moveInfo->moves[gMoveSelectionCursor[gActiveBattler]]].type;

    FillWindowPixelRect(9, 0xF, 0, 0, 32, 16);
    BlitMoveInfoIcon(9, type + 1, 0, 3);
    PutWindowTilemap(9);
    CopyWindowToVram(9, COPYWIN_BOTH);
}

static void MoveSelectionDisplayCategoryIcon(void)
{
    struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleBufferA[gActiveBattler][4]);
    u8 category = GetMoveCategory(moveInfo->moves[gMoveSelectionCursor[gActiveBattler]]);

    FillWindowPixelRect(10, 0xF, 0, 0, 16, 16);
    BlitMoveInfoIcon(10, category + MOVE_CATEGORY_ICON_BASE, 0, 0);
    PutWindowTilemap(10);
    CopyWindowToVram(10, COPYWIN_BOTH);
}

// Replaces the vanilla MoveSelectionDisplayMoveType (which printed the type name).
void Hook_Player_MoveSelectionDisplayMoveType(void)
{
    MoveSelectionDisplayTypeIcon();
    MoveSelectionDisplayCategoryIcon();
}

void Hook_Player_InitMoveSelectionsVarsAndStrings(void)
{
    sSwapSource = NO_SWAP_SOURCE;
    DrawMoveNames(gMoveSelectionCursor[gActiveBattler], NO_SWAP_SOURCE); // also draws the "cursor"
    gMultiUsePlayerCursor = 0xFF;
    Hook_Player_MoveSelectionDisplayPpString();
    Hook_Player_MoveSelectionDisplayPpNumber();
    Hook_Player_MoveSelectionDisplayMoveType();
}

// The action menu no longer uses the tilemap arrow cursor; the selected entry
// is highlighted by recolouring its palette slot instead.
void Hook_ActionSelectionCreateCursorAt(u8 cursorPos, u8 baseTileNum)
{
    BattleMenu_LightSelection();
}

void Hook_ActionSelectionDestroyCursorAt(u8 cursorPos)
{
}

// The vanilla move cursor is a tilemap arrow written into BG0 next to the move
// names; with the new frame tileset those tiles render as black blocks. Instead
// the "cursor" is the colour of the move name.
//
// Vanilla calls this as:
//   (pos, 0)  - normal selection moved to pos (gMoveSelectionCursor already updated)
//   (pos, 29) - SELECT pressed: pos is the move being picked up for swapping
//   (pos, 27) - SELECT pressed: pos is the swap target cursor
void Hook_MoveSelectionCreateCursorAt(u8 cursorPos, u8 baseTileNum)
{
    if (baseTileNum == 29)
    {
        sSwapSource = cursorPos;
        DrawMoveNames(gMoveSelectionCursor[gActiveBattler], sSwapSource);
    }
    else if (baseTileNum == 27)
    {
        DrawMoveNames(cursorPos, sSwapSource);
    }
    else
    {
        sSwapSource = NO_SWAP_SOURCE;
        DrawMoveNames(cursorPos, NO_SWAP_SOURCE);
    }
}

void Hook_MoveSelectionDestroyCursorAt(u8 cursorPos)
{
}
