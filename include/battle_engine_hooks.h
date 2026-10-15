#ifndef GUARD_BATTLE_ENGINE_HOOKS_H
#define GUARD_BATTLE_ENGINE_HOOKS_H

#include "global.h"
#include "window.h"

// ---------------------------------------------------------------------------
// ROM-internal (static in the decomp) functions/data that our code calls.
// Their addresses come from pokefirered.sym and are exported in BPRE.ld.
// ---------------------------------------------------------------------------
void PlayerBufferExecCompleted(void);
void OakOldManBufferExecCompleted(void);
void SafariBufferExecCompleted(void);
void MoveSelectionDisplayPpString(void);
void MoveSelectionDisplayPpNumber(void);
extern const u8 sTargetIdentities[4];

// ---------------------------------------------------------------------------
// Replacement functions. Each one is installed over the ROM function named in
// the comment by hooks/<ROM>/hooks.s.
// ---------------------------------------------------------------------------

// battle_bg.c
extern const struct WindowTemplate gBattleWindowTemplates_Engine[]; // data: replaces sStandardBattleWindowTemplates
void Hook_LoadBattleMenuWindowGfx(void);                 // LoadBattleMenuWindowGfx
void Hook_LoadBattleTextboxAndBackground(void);          // LoadBattleTextboxAndBackground
void BattleMenu_LightSelection(void);

// battle_controller_player.c
void Hook_Player_HandleInputChooseAction(void);          // HandleInputChooseAction (player)
void Hook_Player_HandleInputChooseTarget(void);          // HandleInputChooseTarget
void Hook_Player_HandleMoveSwitching(void);              // HandleMoveSwitching
void Hook_Player_MoveSelectionDisplayMoveNames(void);    // MoveSelectionDisplayMoveNames
void Hook_Player_MoveSelectionDisplayMoveType(void);     // MoveSelectionDisplayMoveType (draws type + category icons)
void Hook_Player_InitMoveSelectionsVarsAndStrings(void); // InitMoveSelectionsVarsAndStrings
void Hook_Player_MoveSelectionDisplayPpString(void);     // MoveSelectionDisplayPpString
void Hook_Player_MoveSelectionDisplayPpNumber(void);     // MoveSelectionDisplayPpNumber
void Hook_ActionSelectionCreateCursorAt(u8 cursorPos, u8 baseTileNum);  // ActionSelectionCreateCursorAt
void Hook_ActionSelectionDestroyCursorAt(u8 cursorPos);                 // ActionSelectionDestroyCursorAt

void Hook_MoveSelectionCreateCursorAt(u8 cursorPos, u8 baseTileNum);   // MoveSelectionCreateCursorAt
void Hook_MoveSelectionDestroyCursorAt(u8 cursorPos);                  // MoveSelectionDestroyCursorAt

// battle_controller_oak_oldman.c
void Hook_OakOldMan_HandleInputChooseAction(void);       // HandleInputChooseAction (oak/old man)
void Hook_OakOldMan_SimulateInputChooseAction(void);     // SimulateInputChooseAction
void Hook_OakOldMan_PrintTextWithMainBgDarkened(const u8 *text, u8 delay); // PrintOakTextWithMainBgDarkened
void Hook_BtlCtrl_DrawVoiceoverMessageFrame(void);       // BtlCtrl_DrawVoiceoverMessageFrame
void Hook_BtlCtrl_RemoveVoiceoverMessageFrame(void);     // BtlCtrl_RemoveVoiceoverMessageFrame

// battle_controller_safari.c
void Hook_Safari_HandleInputChooseAction(void);          // HandleInputChooseAction (safari)
void Hook_Safari_HandleChooseActionAfterDma3(void);      // HandleChooseActionAfterDma3 (safari)

#endif // GUARD_BATTLE_ENGINE_HOOKS_H
