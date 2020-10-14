#pragma once

enum DIRECTION {
	UP, RIGHT, DOWN, LEFT, NONE
};

struct Sprite {
	u8 x;
	u8 y;
	u8 id;
	u8 rot_id;
	u16* gfx_ptr;
	u8 pal_offset;
	bool hidden;
	DIRECTION dir;
	void setAppropriatePal() {
		u8* ptr = (u8*)gfx_ptr;
		for (int i = 0; i < 32 * 32; i++) {
			*ptr += pal_offset;
			ptr++;
		}
		
	}
};
