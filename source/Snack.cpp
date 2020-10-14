#include <nds.h>
#include <nds/arm9/sprite.h>

#include "Snack.h"
#include "Snacks.h"

Snack::Snack(u8 x, u8 y, u8 id) {
	sprite.x = x;
	sprite.y = y;
	sprite.id = id;
	sprite.rot_id = 0;
	sprite.pal_offset = 48;
	sprite.gfx_ptr = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_256Color);
	//oamSetGfx(&oamMain, sprite.id, SpriteSize_32x32, SpriteColorFormat_256Color, sprite.gfx_ptr);
	dmaCopy(mouseSGfxTiles, sprite.gfx_ptr, mouseSGfxTilesLen);
	dmaCopy(mouseSGfxPal, SPRITE_PALETTE + 48, mouseSGfxPalLen);
	sprite.setAppropriatePal();
	draw();
}
Snack::~Snack() {
	oamFreeGfx(&oamMain, sprite.gfx_ptr);
}
void Snack::draw() {
	oamSet(&oamMain, sprite.id, sprite.x, sprite.y, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color, sprite.gfx_ptr, sprite.rot_id, false, false, false, false, false);
	oamUpdate(&oamMain);
}
// set the new position x, y from range 1 - 7
void Snack::newPos(u8 x, u8 y) {
	sprite.x = x * 32;
	sprite.y = y * 32;
	draw();
}
void Snack::die() {
	if (sprite.gfx_ptr)
		oamFreeGfx(&oamMain, (const void*)sprite.gfx_ptr);
	sprite.rot_id = -1;
	oamSetAffineIndex(&oamMain, sprite.id, -1, false);
	oamSetHidden(&oamMain, sprite.id, true);
	oamSet(&oamMain, sprite.id, sprite.x, sprite.y, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color, sprite.gfx_ptr, sprite.rot_id, false, true, false, false, false);
	oamUpdate(&oamMain);
}