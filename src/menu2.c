#include "global.h"
#include "gflib.h"
#include "menu.h"
#include "menu_helpers.h"
#include "strings.h"
#include "text_window.h"
#include "constants/songs.h"
#include "graphics.h"

struct Menu
{
    u8 tilemapLeft;
    u8 tilemapTop;
    s8 cursorPos;
    s8 minCursorPos;
    s8 maxCursorPos;
    u8 windowId;
    u8 fontId;
    u8 optionWidth;
    u8 optionHeight;
    u8 columns;
    u8 rows;
    bool8 APressMuted;
};

static EWRAM_DATA struct Menu sMenu = {0};
static EWRAM_DATA u16 sTileNum = 0;
static EWRAM_DATA u8 sPaletteNum = 0;
static EWRAM_DATA u8 sYesNoWindowId = 0;
static EWRAM_DATA u8 sTopBarWindowId = 0;

const u32 gBattleTiles[] = INCBIN_U32("graphics/battle_interface/tiles.4bpp.lz");
const u16 gBattleFramesPalette[] = INCBIN_U16("graphics/battle_interface/frames.gbapal");
const u16 gBattleTextPalette[] = INCBIN_U16("graphics/battle_interface/text.gbapal");
const u32 gBattleFramesTilemap[] = INCBIN_U32("graphics/battle_interface/frames.bin.lz");
const u32 gBattleEngTilemap[] = INCBIN_U32("graphics/battle_interface/battle_eng.bin.lz");
const u32 gSafariEngTilemap[] = INCBIN_U32("graphics/battle_interface/safari_eng.bin.lz");
const u8 gText_BattleTextPrefix[] = _("{COLOR 4}");

static void WindowFunc_DrawDialogFrameWithCustomTileAndPalette(u8 bg, u8 tilemapLeft, u8 tilemapTop, u8 width, u8 height, u8 paletteNum)
{
    FillBgTilemapBufferRect(bg, sTileNum + 0, tilemapLeft - 2, tilemapTop - 1, 1, 1, sPaletteNum);
	FillBgTilemapBufferRect(bg, sTileNum + 1, tilemapLeft - 1, tilemapTop - 1, 1, 1, sPaletteNum);
	FillBgTilemapBufferRect(bg, sTileNum + 2, tilemapLeft, tilemapTop - 1, width, 1, sPaletteNum);
	FillBgTilemapBufferRect(bg, sTileNum + 3, tilemapLeft + width, tilemapTop - 1, 1, 1, sPaletteNum);
	FillBgTilemapBufferRect(bg, sTileNum + 4, tilemapLeft + width + 1, tilemapTop - 1, 1, 1, sPaletteNum);
	FillBgTilemapBufferRect(bg, sTileNum + 5, tilemapLeft - 2, tilemapTop, 1, 3, sPaletteNum);
	FillBgTilemapBufferRect(bg, sTileNum + 6, tilemapLeft - 1, tilemapTop, 1, 3, sPaletteNum);
	FillBgTilemapBufferRect(bg, sTileNum + 7, tilemapLeft, tilemapTop, 26, 4, sPaletteNum);
	FillBgTilemapBufferRect(bg, sTileNum + 8, tilemapLeft + width, tilemapTop, 1, 3, sPaletteNum);
	FillBgTilemapBufferRect(bg, sTileNum + 9, tilemapLeft + width + 1, tilemapTop, 1, 3, sPaletteNum);
	FillBgTilemapBufferRect(bg, sTileNum + 10, tilemapLeft - 2, tilemapTop + 3, 1, 1, sPaletteNum);
	FillBgTilemapBufferRect(bg, sTileNum + 11, tilemapLeft - 1, tilemapTop + 3, 1, 1, sPaletteNum);
	FillBgTilemapBufferRect(bg, sTileNum + 13, tilemapLeft + width, tilemapTop + 3, 1, 1, sPaletteNum);
	FillBgTilemapBufferRect(bg, sTileNum + 14, tilemapLeft + width + 1, tilemapTop + 3, 1, 1, sPaletteNum);
	FillBgTilemapBufferRect(bg, sTileNum + 15, tilemapLeft - 2, tilemapTop + 4, 1, 1, sPaletteNum);
	FillBgTilemapBufferRect(bg, sTileNum + 16, tilemapLeft - 1, tilemapTop + 4, 1, 1, sPaletteNum);
	FillBgTilemapBufferRect(bg, sTileNum + 17, tilemapLeft, tilemapTop + 4, width, 1, sPaletteNum);
	FillBgTilemapBufferRect(bg, BG_TILE_H_FLIP(sTileNum + 16), tilemapLeft + width, tilemapTop + 4, 1, 1, sPaletteNum);
	FillBgTilemapBufferRect(bg, BG_TILE_H_FLIP(sTileNum + 15), tilemapLeft + width + 1, tilemapTop + 4, 1, 1, sPaletteNum);
    }
    