#pragma once

#include "Sprite.h"

class Snake {
	Sprite body_parts[48];
	DIRECTION dir;
	bool _isAlive;
	u8 body_parts_count;
	bool move_tail;
public:
	Snake(u8 x, u8 y);
	~Snake();
	bool move(); //return true if player is dead
	void moveOnePixel();
	void moveSegOnePixel(u8 seg, DIRECTION d);
	void grow();
	void reset(u8 x, u8 y);
	bool update(DIRECTION newHeadDir);

	bool isAlive() const { return _isAlive; }
	bool positionFree(u8 x, u8 y, bool scaled = true) const;
	void setDirection(DIRECTION d);
	const Sprite* getHead() const { return (const Sprite*)&body_parts[0]; }
private:
	void draw();
};