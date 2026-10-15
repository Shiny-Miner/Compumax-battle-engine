// ---------------------------------------------------------------------------
// Battle engine hooks - FireRed 1.0 (BPRE)
//
// Every hook overwrites the first 8 bytes of a ROM function with
//     ldr  r3, =<replacement>|1
//     bx   r3
//     .pool
// so any caller (bl, function-pointer tables, controller callbacks) lands in
// our C code. r3 is used because none of the hooked functions take more than
// two arguments. The original bodies are left in place, now unreachable.
//
// Addresses/names come from pokefirered.sym and are checked to be 4-byte
// aligned (required for the literal pool) and at least 8 bytes long.
// The replacement labels are exported by build/linked.o (.importobj in
// mainBPRE.asm), which is imported after this file is included.
// ---------------------------------------------------------------------------

.macro hook_thumb,addr,target
    .org addr
    ldr  r3,=target|1
    bx   r3
    .pool
.endmacro


// ---- Battle background / textbox ----
// LoadBattleMenuWindowGfx (0x0800F380, size 0x8C)
hook_thumb 0x0800F380,Hook_LoadBattleMenuWindowGfx
// LoadBattleTextboxAndBackground (0x0800F420, size 0x40)
hook_thumb 0x0800F420,Hook_LoadBattleTextboxAndBackground

// ---- Player controller ----
// HandleInputChooseAction (0x0802E438, size 0x206)
hook_thumb 0x0802E438,Hook_Player_HandleInputChooseAction
// HandleInputChooseTarget (0x0802E674, size 0x39C)
hook_thumb 0x0802E674,Hook_Player_HandleInputChooseTarget
// HandleMoveSwitching (0x0802EF58, size 0x6B8)
hook_thumb 0x0802EF58,Hook_Player_HandleMoveSwitching
// MoveSelectionDisplayMoveNames (0x08030830, size 0x78)
hook_thumb 0x08030830,Hook_Player_MoveSelectionDisplayMoveNames
// MoveSelectionDisplayMoveType (0x08030940, size 0x8C)
hook_thumb 0x08030940,Hook_Player_MoveSelectionDisplayMoveType
// MoveSelectionDisplayPpString (0x080308A8, size 0x24)
hook_thumb 0x080308A8,Hook_Player_MoveSelectionDisplayPpString
// MoveSelectionDisplayPpNumber (0x080308CC, size 0x74)
hook_thumb 0x080308CC,Hook_Player_MoveSelectionDisplayPpNumber
// ActionSelectionCreateCursorAt (0x08030A54, size 0x44)
hook_thumb 0x08030A54,Hook_ActionSelectionCreateCursorAt
// ActionSelectionDestroyCursorAt (0x08030A98, size 0x44)
hook_thumb 0x08030A98,Hook_ActionSelectionDestroyCursorAt
// InitMoveSelectionsVarsAndStrings (0x08032CB4, size 0x38)
hook_thumb 0x08032CB4,Hook_Player_InitMoveSelectionsVarsAndStrings

// ---- Safari controller ----
// HandleInputChooseAction (0x080DD5A4, size 0x184)
hook_thumb 0x080DD5A4,Hook_Safari_HandleInputChooseAction
// HandleChooseActionAfterDma3 (0x080DDC38, size 0x40)
hook_thumb 0x080DDC38,Hook_Safari_HandleChooseActionAfterDma3

// ---- Oak / Old Man controller ----
// HandleInputChooseAction (0x080E763C, size 0x206)
hook_thumb 0x080E763C,Hook_OakOldMan_HandleInputChooseAction
// SimulateInputChooseAction (0x080E7844, size 0x9A)
hook_thumb 0x080E7844,Hook_OakOldMan_SimulateInputChooseAction
// PrintOakTextWithMainBgDarkened (0x080E85D4, size 0x130)
hook_thumb 0x080E85D4,Hook_OakOldMan_PrintTextWithMainBgDarkened
// BtlCtrl_DrawVoiceoverMessageFrame (0x080EB30C, size 0x218)
hook_thumb 0x080EB30C,Hook_BtlCtrl_DrawVoiceoverMessageFrame
// BtlCtrl_RemoveVoiceoverMessageFrame (0x080EB524, size 0x132)
hook_thumb 0x080EB524,Hook_BtlCtrl_RemoveVoiceoverMessageFrame

// ---- Data pointer patch ----
// BattleInitBgsAndWindows (0x0800F324, size 0x28) ends with a literal pool:
//   0x0800F344 -> gBattleBgTemplates
//   0x0800F348 -> sStandardBattleWindowTemplates (0x08248330)
// Point the second entry at our 26-entry window template table instead.
// CHECK: file offset 0xF348 of the base ROM should read 30 83 24 08.
.org 0x0800F348
.word gBattleWindowTemplates_Engine

// ---- Move cursor (no-op: tilemap arrow replaced by the new frame) ----
// MoveSelectionCreateCursorAt (0x080309CC, size 0x48)
hook_thumb 0x080309CC,Hook_MoveSelectionCreateCursorAt
// MoveSelectionDestroyCursorAt (0x08030A14, size 0x40)
hook_thumb 0x08030A14,Hook_MoveSelectionDestroyCursorAt
