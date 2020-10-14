#pragma once

#include "Sprite.h"

class Snack {
	Sprite sprite;
public:
	Snack(u8 x, u8 y, u8 id);
	~Snack();

	void draw();
	const Sprite* getPos() const { return (const Sprite*)&sprite; }
	void newPos(u8 x, u8 y);
	void die();
};