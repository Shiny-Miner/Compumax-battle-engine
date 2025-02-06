#include "global.h"
#include "gflib.h"
#include "battle.h"
#include "battle_bg.h"
#include "battle_message.h"
#include "decompress.h"
#include "graphics.h"
#include "link.h"
#include "new_menu_helpers.h"
#include "overworld.h"
#include "text_window.h"
#include "trig.h"
#include "constants/maps.h"
#include "constants/songs.h"
#include "constants/trainer_classes.h"

static const struct WindowTemplate gUnknown_8248330[] = { //Fix baseBlock
    {	//Intro Texts
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 15,
        .width = 28,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 200
    }, { //What will\n{B_ACTIVE_NAME_WITH_PREFIX} do?
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 35,
        .width = 14,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 312
    }, { //Fight Bag\nPokémon Run
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 0,
        .height = 0,
        .paletteNum = 0,
        .baseBlock = 0
    }, { //1st Move
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 55,
        .width = 10,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 368
    }, { //2nd Move
        .bg = 0,
        .tilemapLeft = 11,
        .tilemapTop = 55,
        .width = 9,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 388
    }, { //3th Move
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 57,
        .width = 9,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 408
    }, { //4th Move
        .bg = 0,
        .tilemapLeft = 11,
        .tilemapTop = 57,
        .width = 9,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 428
    }, { //PP Text
        .bg = 0,
        .tilemapLeft = 23,
        .tilemapTop = 55,
        .width = 2,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 448
    }, { //PP Count
        .bg = 0,
        .tilemapLeft = 25,
        .tilemapTop = 55,
        .width = 4,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 452
    }, { //Type Icon
        .bg = 0,
        .tilemapLeft = 23,
        .tilemapTop = 57,
        .width = 4,
        .height = 2,
        .paletteNum = 13,
        .baseBlock = 460
    }, { //Category Icon
        .bg = 0,
        .tilemapLeft = 27,
        .tilemapTop = 57,
        .width = 2,
        .height = 2,
        .paletteNum = 13,
        .baseBlock = 468
    }, {
		.bg = 0,
        .tilemapLeft = 0, //21,
        .tilemapTop = 0, //55,
        .width = 0, //8,
        .height = 0, //4,
        .paletteNum = 0,//5,
        .baseBlock = 0,//0x2b0
    }, {// Stats al subir nivel
        .bg = 1,
        .tilemapLeft = 19,
        .tilemapTop = 8,
        .width = 10,
        .height = 11,
        .paletteNum = 15,
        .baseBlock = 472
    }, {
        .bg = 2,
        .tilemapLeft = 18,
        .tilemapTop = 0,
        .width = 12,
        .height = 3,
        .paletteNum = 15,
        .baseBlock = 578
    }, { //Yes/No
        .bg = 0,
        .tilemapLeft = 25,
        .tilemapTop = 9,
        .width = 4,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 472
    }, {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 3,
        .width = 7,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x020
    }, {
        .bg = 2,
        .tilemapLeft = 2,
        .tilemapTop = 3,
        .width = 7,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x040
    }, {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 2,
        .width = 7,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x020
    }, {
        .bg = 2,
        .tilemapLeft = 2,
        .tilemapTop = 2,
        .width = 7,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x040
    }, {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 6,
        .width = 7,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x060
    }, {
        .bg = 2,
        .tilemapLeft = 2,
        .tilemapTop = 6,
        .width = 7,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x080
    }, {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 0,
        .height = 0,
        .paletteNum = 0,
        .baseBlock = 200
    }, {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 0,
        .height = 0,
        .paletteNum = 0,
        .baseBlock = 200
    }, {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 0,
        .height = 0,
        .paletteNum = 0,
        .baseBlock = 216
    }, { // Tutorial Texts
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 15,
        .width = 28,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 200
    }, DUMMY_WIN_TEMPLATE
};

void LoadBattleMenuWindowGfx(void)
{    TextWindow_SetUserSelectedFrame(2, 0x022, 0xC0);
    gPlttBufferUnfaded[0xFC] = RGB( 9,  9,  9);
    gPlttBufferUnfaded[0xFD] = RGB( 9,  9,  9);
    gPlttBufferUnfaded[0xFE] = RGB(31, 31, 31);
    gPlttBufferUnfaded[0xFF] = RGB( 26,  26,  25);
    CpuCopy16(&gPlttBufferUnfaded[0xFC], &gPlttBufferFaded[0xFC], 8);
}
void LoadBattleTextboxAndBackground(void)
{    LZDecompressVram(gBattleTiles, (void *)BG_CHAR_ADDR(0));
	CopyToBgTilemapBuffer(0, gBattleFramesTilemap, 0, 0x000);
    if (gBattleTypeFlags & BATTLE_TYPE_SAFARI)
		{
			CopyToBgTilemapBuffer(0, gSafariEngTilemap, 0, 68);
		}
	else
		{
			CopyToBgTilemapBuffer(0, gBattleEngTilemap, 0, 68);
		}
    LoadPalette(gFireRedMenuElements2_Pal, 0xD0, 0x20);
    LoadPalette(gBattleFramesPalette, 0xE0, 0x20);
	LoadPalette(gBattleTextPalette, 0xF0, 0x20);
	CopyBgTilemapBufferToVram(0);
        LoadBattleMenuWindowGfx();
    DrawMainBattleBackground();
}
bool8 LoadChosenBattleElement(u8 caseId)
{
    bool8 ret = FALSE;
    u8 battleScene;
    switch (caseId)
    {
    case 0:		
    LoadBattleTextboxAndBackground();
		break;
    case 1:
        battleScene = GetBattleTerrainOverride();
        LZDecompressVram(sBattleTerrainTable[battleScene].tileset, (void *)BG_CHAR_ADDR(2));
        break;
    case 2:
        battleScene = GetBattleTerrainOverride();
        LZDecompressVram(sBattleTerrainTable[battleScene].tilemap, (void *)BG_SCREEN_ADDR(26));
        break;
    case 3:
        battleScene = GetBattleTerrainOverride();
        LoadCompressedPalette(sBattleTerrainTable[battleScene].palette, 0x20, 0x60);
        break;
    case 4:
        LoadBattleMenuWindowGfx();
        break;
    default:
        ret = TRUE;
        break;
    }
    return ret;
}
