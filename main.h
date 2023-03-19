#ifndef _TAPPER_H_
#define _TAPPER_H_

#define STACK (0x80000000 + _ramsize - _stacksize)

// declared global so I can reach the Ordering Table anywhere
// and use Sprite_DrawFast at will
GsOT *ot;

enum APP_STATUS
{
	INTRO = 0,
	MENU,			// show menu
	GAME_PLAYING,	// game playing
	GAME_LOCKED,	// game stopped
	GAME_OVER,
	GAME_SUCCESS,	// nobody will ever reach this point :^)
	HIGHSCORE,
	MEMCARD
};

bool freeAll();

#endif /* _TAPPER_H_ */