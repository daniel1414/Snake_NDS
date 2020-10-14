#include <nds.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/trig_lut.h>

#include "Snake.h"
#include "snakeParts.h"

Snake::Snake(u8 x, u8 y) : dir(UP), _isAlive(true), body_parts_count(2), move_tail(true) {
	// fill the id of every sprite of the snake
	for (int i = 1; i < 47; i++) {
		body_parts[i].id = i;
		body_parts[i].rot_id = 0;
		body_parts[i].x = 1;
		body_parts[i].y = 1;
		body_parts[i].gfx_ptr = nullptr;
		body_parts[i].hidden = true;
		body_parts[i].dir = NONE;
		body_parts[i].pal_offset = 16;
	}
	// position of the head int the beginning
	body_parts[0].id = 0;
	body_parts[0].rot_id = 0;
	body_parts[0].x = x;
	body_parts[0].y = y;
	body_parts[0].pal_offset = 0;
	body_parts[0].hidden = false;
	body_parts[0].dir = UP;
	// position of the tail in the beginning
	body_parts[47].id = 47;
	body_parts[47].rot_id = 0;
	body_parts[47].x = x;
	body_parts[47].y = y + 32;
	body_parts[47].pal_offset = 32;
	body_parts[47].hidden = false;
	body_parts[47].dir = UP;
	// rotations 
	oamRotateScale(&oamMain, 0, degreesToAngle(0), intToFixed(1, 8), intToFixed(1, 8));
	oamRotateScale(&oamMain, 1, degreesToAngle(270), intToFixed(1, 8), intToFixed(1, 8));
	oamRotateScale(&oamMain, 2, degreesToAngle(180), intToFixed(1, 8), intToFixed(1, 8));
	oamRotateScale(&oamMain, 3, degreesToAngle(90), intToFixed(1, 8), intToFixed(1, 8));
	//head graphics
	body_parts[0].gfx_ptr = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_256Color);
	oamSetGfx(&oamMain, body_parts[0].id, SpriteSize_32x32, SpriteColorFormat_256Color, body_parts[0].gfx_ptr);
	dmaCopy(snakeGfxTiles, body_parts[0].gfx_ptr, snakeGfxTilesLen);
	// tail praghics
	body_parts[47].gfx_ptr = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_256Color);
	oamSetGfx(&oamMain, body_parts[47].id, SpriteSize_32x32, SpriteColorFormat_256Color, body_parts[47].gfx_ptr);
	dmaCopy(tailUpGfxTiles, body_parts[47].gfx_ptr, tailUpGfxTilesLen);
	// palettes
	dmaCopy(snakeGfxPal, SPRITE_PALETTE, snakeGfxPalLen);
	dmaCopy(bodyVGfxPal, SPRITE_PALETTE + 16, bodyVGfxPalLen);
	dmaCopy(tailUpGfxPal, SPRITE_PALETTE + 32, tailUpGfxPalLen);

	body_parts[0].setAppropriatePal();
	body_parts[47].setAppropriatePal();
	draw();
}
Snake::~Snake() {
	for (int i = 0; i < body_parts_count - 1; i++) {
		oamFreeGfx(&oamMain, body_parts[i].gfx_ptr);
	}
	oamFreeGfx(&oamMain, body_parts[47].gfx_ptr);
}
bool Snake::move() {
	// check if player is dead
	if ((dir == UP && body_parts[0].y == 0) ||
		(dir == RIGHT && body_parts[0].x == 224)  ||
		(dir == DOWN && body_parts[0].y == 160) ||
		(dir == LEFT && body_parts[0].x == 0)) {
		// the player hit the wall
		return false;
	}
	// the tail
	if (move_tail) {
		// get the tail pos and rot_id from the last segment
		body_parts[47].x = body_parts[body_parts_count - 2].x;
		body_parts[47].y = body_parts[body_parts_count - 2].y;
		body_parts[47].rot_id = body_parts[body_parts_count - 2].rot_id;
	} else {
		move_tail = true;
	}
	// the body
	for (int i = body_parts_count - 1; i > 0; i--) {
		// take the position and rot_id from the previous segment
		body_parts[i].x = body_parts[i - 1].x;
		body_parts[i].y = body_parts[i - 1].y;
		body_parts[i].rot_id = body_parts[i - 1].rot_id;
	}
	// move and check if the player fucked up
	switch (dir) {
	case UP:
		if (positionFree(body_parts[0].x, body_parts[0].y - 32, false)) {
			body_parts[0].y -= 32;
			body_parts[0].rot_id = 0;
		}
		else
			return false;
		break;
	case RIGHT:
		if (positionFree(body_parts[0].x + 32, body_parts[0].y, false)) {
			body_parts[0].rot_id = 1;
			body_parts[0].x += 32;
		}
		else
			return false;
		break;
	case DOWN:
		if (positionFree(body_parts[0].x, body_parts[0].y + 32, false)) {
			body_parts[0].y += 32;
			body_parts[0].rot_id = 2;
		}
		else
			return false;
		break;
	case LEFT:
		if (positionFree(body_parts[0].x - 32, body_parts[0].y, false)) {
			body_parts[0].x -= 32;
			body_parts[0].rot_id = 3;
		}
		else
			return false;
		break;
	default:
		break;
	}
	draw();
	return true;
}
void Snake::moveOnePixel() {
	for (int i = 0; i < body_parts_count - 1; i++) {
		moveSegOnePixel(i, body_parts[i].dir);
	}
	if (move_tail)
		moveSegOnePixel(47, body_parts[47].dir);
	draw();
}
void Snake::moveSegOnePixel(u8 seg, DIRECTION d) {
	switch (d) {
	case UP:
		body_parts[seg].y -= 1;
		break;
	case RIGHT:
		body_parts[seg].x += 1;
		break;
	case DOWN:
		body_parts[seg].y += 1;
		break;
	case LEFT:
		body_parts[seg].x -= 1;
		break;
	default:
		break;
	}
}
bool Snake::update(DIRECTION newHeadDir) {
	// set new directions
	body_parts[47].dir = body_parts[body_parts_count - 2].dir;
	body_parts[47].rot_id = body_parts[body_parts_count - 2].rot_id;

	for (int i = body_parts_count - 1; i > 0; i--) {
		body_parts[i].dir = body_parts[i - 1].dir;
		body_parts[i].rot_id = body_parts[i - 1].rot_id;
	}
	switch (newHeadDir) {
	case UP:
		if (positionFree(body_parts[0].x, body_parts[0].y - 32, false) && body_parts[0].y != 0) {
			body_parts[0].rot_id = 0;
			body_parts[0].dir = newHeadDir;
		}
		else
			return false;
		break;
	case RIGHT:
		if (positionFree(body_parts[0].x + 32, body_parts[0].y, false) && body_parts[0].x != 224) {
			body_parts[0].rot_id = 1;
			body_parts[0].dir = newHeadDir;
		}
		else
			return false;
		break;
	case DOWN:
		if (positionFree(body_parts[0].x, body_parts[0].y + 32, false) && body_parts[0].y != 160) {
			body_parts[0].rot_id = 2;
			body_parts[0].dir = newHeadDir;
		}
		else
			return false;
		break;
	case LEFT:
		if (positionFree(body_parts[0].x - 32, body_parts[0].y, false) && body_parts[0].x != 0) {
			body_parts[0].rot_id = 3;
			body_parts[0].dir = newHeadDir;
		}
		else
			return false;
		break;
	default:
		break;
	}
	// draw();
	move_tail = true;
	return true;
}
void Snake::grow() {
	// allocate graphics in the graphics pointer -- is ok and unnecessary
	body_parts[body_parts_count - 1].gfx_ptr = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_256Color);
	oamSetGfx(&oamMain, body_parts[body_parts_count - 1].id, SpriteSize_32x32, SpriteColorFormat_256Color, body_parts[body_parts_count - 1].gfx_ptr);
	dmaCopy(bodyVGfxTiles, body_parts[body_parts_count - 1].gfx_ptr, bodyVGfxTilesLen);
	// set the rot_id from the tail segment
	body_parts[body_parts_count - 1].rot_id = body_parts[47].rot_id;
	body_parts[body_parts_count - 1].dir= body_parts[47].dir;
	body_parts[body_parts_count - 1].hidden = false;
	// get the position from the tail position
	body_parts[body_parts_count - 1].x = body_parts[47].x;
	body_parts[body_parts_count - 1].y = body_parts[47].y;
	// set the right palette offset
	body_parts[body_parts_count - 1].pal_offset = 16;
	body_parts[body_parts_count - 1].setAppropriatePal();
	// don't move the tail in the next iteration
	move_tail = false;
	body_parts_count++;
	draw();
}
void Snake::draw() {
	// draw the head and body parts
	for (int i = 0; i < body_parts_count - 1; i++) {
		oamSet(&oamMain, body_parts[i].id, body_parts[i].x, body_parts[i].y, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color,
			body_parts[i].gfx_ptr, body_parts[i].rot_id, false, body_parts[i].hidden, false, false, false);
	}
	// draw the tail
	oamSet(&oamMain, body_parts[47].id, body_parts[47].x, body_parts[47].y, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color,
		body_parts[47].gfx_ptr, body_parts[47].rot_id, false, body_parts[47].hidden, false, false, false);

	oamUpdate(&oamMain);
}
bool Snake::positionFree(u8 x, u8 y, bool scaled) const {
	if (scaled) {
		for (const auto& p : body_parts) {
			if (p.x == x * 32 && p.y == y * 32) {
				return false;
			}
		}
		return true;
	}
	else {
		for (const auto& p : body_parts) {
			if (p.x == x && p.y == y) {
				return false;
			}
		}
		return true;
	}
}
void Snake::setDirection(DIRECTION d) {
	switch (d) {
	case UP:
		if (dir != DOWN)
			dir = d;
		break;
	case RIGHT:
		if (dir != LEFT)
			dir = d;
		break;
	case DOWN:
		if (dir != UP)
			dir = d;
		break;
	case LEFT:
		if (dir != RIGHT)
			dir = d;
		break;
	case NONE:
		break;
	}
}
void Snake::reset(u8 x, u8 y) {
	// free all the body parts except head and tail
	for (int i =  1; i <  47; i++) {
		if(body_parts[i].gfx_ptr)
			oamFreeGfx(&oamMain, (const void*)body_parts[i].gfx_ptr);
		body_parts[i].x = 1;
		body_parts[i].y = 1;
		body_parts[i].rot_id = -1;
		oamSetAffineIndex(&oamMain, body_parts[i].id, -1, false);
		oamSetHidden(&oamMain, body_parts[i].id, true);
		body_parts[i].gfx_ptr = nullptr;
	}
	// set new position and rot_id of the head
	body_parts[0].x = x;
	body_parts[0].y = y;
	body_parts[0].rot_id = 0;
	// same for the tail
	body_parts[47].x = x;
	body_parts[47].y = y + 32;
	body_parts[47].rot_id = 0;
	
	dir = UP;
	body_parts_count = 2;

	body_parts[0].hidden = true;
	body_parts[47].hidden = true;

	draw();
}

