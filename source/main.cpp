#include <nds.h>
#include <stdio.h>
#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/system.h>
#include <nds/arm9/input.h>
#include <nds/timers.h>
#include <nds/interrupts.h>
#include <maxmod9.h>

#include <random>

#include "Snake.h"
#include "Snack.h"
#include "Sprite.h"
#include "OKLogo.h"

#include "soundbank.h"
#include "soundbank_bin.h"

bool waiting_0;
bool endGame = false;
void timerCallback();
bool gameOver();
void displayLogo();
void mainMenu();
int playGame();

int main(void) {
	// hardware initialization
	videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);
	vramSetBankB(VRAM_B_MAIN_SPRITE);
	vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
	oamInit(&oamMain, SpriteMapping_1D_128, false);

	displayLogo();
	lcdMainOnBottom();
	consoleDemoInit();

	mmInitDefaultMem((mm_addr)soundbank_bin);
	mmLoadEffect(SFX_HAPS);

	// main menu
	while (!endGame) {
		mainMenu();
		if (endGame)
			continue;
		playGame();
	}
	return 0;
}
void timerCallback() {
	waiting_0 = false;
}
bool gameOver() {
	consoleClear();
	iprintf("\x1b[12;2HChcesz grac jeszcze raz? A/B");
	while (1) {
		scanKeys();
		if (keysDown() & KEY_A)
			return false;
		if (keysDown() & KEY_B)
			return true;
	}
	return true;
}
void displayLogo() {
	int i = -16;
	setBrightness(1, i);
	int bg = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	dmaCopy(OKLogoBitmap, (void*)bgGetGfxPtr(bg), OKLogoBitmapLen);
	// loop up
	while (i < 1) {
		waiting_0 = true;
		timerStart(0, ClockDivider_1024, UINT16_MAX - 3000, timerCallback);
		while (waiting_0) {
			scanKeys();
		}
		setBrightness(1, i);
		i++;
	}
	waiting_0 = true;
	timerStart(0, ClockDivider_1024, 0, timerCallback);
	while (waiting_0) {
		scanKeys();
	}
	while (i > -17) {
		waiting_0 = true;
		timerStart(0, ClockDivider_1024, UINT16_MAX - 3000, timerCallback);
		while (waiting_0) {
			scanKeys();
		}
		setBrightness(1, i);
		i--;
	}
	videoBgDisable(bg);
	setBrightness(1, 0);
}
void mainMenu() {
	lcdMainOnTop();
	consoleClear();
	iprintf("\x1b[2;0H  ***   Welcome in Snake   ***");
	iprintf("\x1b[6;2H 1. Play!");			// 1
	iprintf("\x1b[8;2H 2. Exit :(");		// 2
	int choice = 1;
	iprintf("\x1b[12;2H Your choice: %d", choice);
	while (!(keysDown() & KEY_A)) {
		scanKeys();
		if ((keysDown() & KEY_UP) && (choice > 1))
			choice--;
		if((keysDown() & KEY_DOWN) && (choice < 2))
			choice++;
		iprintf("\x1b[12;2H Your choice: %d", choice);
	}
	if (choice == 2)
		endGame = true;

	consoleClear();
	lcdMainOnBottom();
}
int playGame() {
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> distX(0, 7);
	std::uniform_int_distribution<int> distY(0, 5);
	
	mm_sound_effect haps = {
		{SFX_HAPS},
		(int)(1.0f * (1 << 10)),
		0, 255, 255 / 2,
	};

	Snake player(2 * 32, 3 * 32);
	Snack mouse(32, 32, 48);
	int keys;
	int slowness = 2000;
	DIRECTION nextDir = player.getHead()->dir;
	int score = 0;
	// wait for the timer to overflow
	while (1) {
		// 32 times = 32 pixels
		for (int i = 0; i < 32; i++) {
			waiting_0 = true;
			timerStart(0, ClockDivider_256, UINT16_MAX - (slowness / 2), timerCallback);
			while (waiting_0) {
				scanKeys();
				keys = keysDown();
				if (keys & KEY_DOWN && player.getHead()->dir != UP)
					nextDir = DOWN;
				if (keys & KEY_UP && player.getHead()->dir != DOWN)
					nextDir = UP;
				if (keys & KEY_LEFT && player.getHead()->dir != RIGHT)
					nextDir = LEFT;
				if (keys & KEY_RIGHT && player.getHead()->dir != LEFT)
					nextDir = RIGHT;
			}
			player.moveOnePixel();
		}
		// the player fucked up
		if (!player.update(nextDir)) {

			while (1) {
				scanKeys();
				if (keysDown() & KEY_B)
					break;
			}
			player.reset(0, 0);
			mouse.die();
			
			return score;
		}
		consoleClear();
		// check if player ate a snack
		if (player.getHead()->x == mouse.getPos()->x && player.getHead()->y == mouse.getPos()->y) {
			// faster!!
			mmEffectEx(&haps);
			score++;
			slowness -= 30;
			// expand the snake
			player.grow();
			// choose a new location for the Snack
			u8 x;
			u8 y;
			do {
				x = distX(mt);
				y = distY(mt);
			} while (!player.positionFree(x, y));
			mouse.newPos(x, y);
		}
		iprintf("\x1b[3;10H Score: %d", score);
		swiWaitForVBlank();
	}
	return score;
}