#include "stdio.h"
#include <stdlib.h>

#include "System.h"
#include "Sprite.h"
#include "DataManager.h"

#ifdef PXFINDER
#include "pxfinder.h"
#endif
float move_offset = 0.5;

#include "common.c"

#include "beer.h"
#include "drunkard.h"
#include "main.h"
#include "menu.h"
#include "game.h"

bool freeAll()
{
	int i;
	free3(currGame); currGame = NULL;

	for (i = 0; i < MAX_BEERS; ++i)
	{
		free3(b_array[i]); b_array[i] = NULL;
	}

	for (i = 0; i < MAX_DRUNKARDS; ++i)
	{
		free3(d_array[i]); d_array[i] = NULL;
	}

	return true;
}

int main(void)
{
	u32 heap_base[4];
	int cputime, gputime;
	unsigned long _ramsize = 0x00200000;	// 2mb RAM
	unsigned long _stacksize = 0x00004000;	// 16kb stack

#ifdef DEBUG
	long psxstats = 0;	// various stats for PSX
#endif

	// Initialize heap
	InitHeap3((u32*)heap_base, STACK-(u32)heap_base);

	// TODO seed initial pseudo-random generator
	// http://ffhacktics.com/wiki/Random_Number_Generator
	// http://www.gamefaqs.com/ps3/927750-playstation-3/faqs/50212
	// see: Beatmania code
	// or: read an interrupt
	// or: allocate chunks of memory and XOR their location
	// or: #define	rnd(max)	((rand()*(max))/(32768)+1)
	srand(SystemGetTimeRef());

	System_Init(SCREEN_MODE, VMODE_16BITS, SCREEN_WIDTH, SCREEN_HEIGHT, GsINTER, 0);
	System_SetBackgroundColor(0, 64, 0);

	// Init DataManager & load TIM Images in VRAM from our loaded data files
	DataManager_Init();

#ifdef PXFINDER
	// Load before any other sprite
	pxfinder_init();
#endif


#ifdef DEBUG
	FntLoad(960, 256);
#endif

	// Init application
	if (NULL == (currGame = malloc3(sizeof(struct Game))))
	{
		fail(ERR, "Malloc failed while generating the Game!");
	}

	if (NULL == (err_cond = malloc3(sizeof(struct err_condition))))
	{
		fail(ERR, "Malloc failed while generating the err_condition struct!");
	}

	// Set initial game status
	// setGameStatus(MENU);
#warning Setting game to GAME_PLAYING
	setGameStatus(GAME_PLAYING);

#ifdef DEBUG
	/*
		long x, long y,		Display start location
		long w, long h,		Display area
		long isbg,			Automatic clearing of background
							0: Don't clear background to (0, 0, 0) when display is performed
							1: Clear background to (0, 0, 0) when display is performed
		long n 				Maximum number of characters
	*/
	psxstats = FntOpen((SCREEN_WIDTH - 50 - 70), 20, 256, 200, 0, 512);
#endif

	while (1)
	{
		ot = System_InitFrame();

#ifdef PXFINDER
		pxfinder_draw(ot[0], 1); // 1 = flush
#endif

		switch(getGameStatus())
		{
			case MENU:
			{
				/* Load menu items: TIM files, fonts, sprites */
				setupMenu();
				/* Show graphic items and menu choices */
				showMenu();
			}; break;

			case GAME_PLAYING:
			{
				/* Load game data items */
				setupGame();

				/*
					This timer isn't really necessary to allow "loading"
					but I need it to put some distance between the moment
					the player clicks the pad in the menu and the next main
					iteration otherwise that click will be catched in game
				*/
				if (gameDelays.waiting_screen != 0)
				{
					printMessage("Loading...",
						(SCREEN_WIDTH / 2) - (10 / 2),
						(SCREEN_HEIGHT / 2) + 20);
				}
				else
				{
					/* Move sprites etc. but draws nothing */
					playGame();

					/* check collisions between beers and drunkards */
					checkCollisions(b_array, d_array);

					// Draw everything
					// this must be the first (there's the BG, then beers and drunkards)
					drawPlayGame();
					// then draw the rest
					drawRest();
					printStatsOnScreen();
				}

			}; break;

			case GAME_OVER:
			{
				if (gameDelays.waiting_screen != 0)
				{
					printMessage("Game over!",
						(SCREEN_WIDTH / 2) - (10 / 2),
						(SCREEN_HEIGHT / 2) + 20);
				}
				else
				{
					// clear screen
					clearBeers(b_array);
					clearDrunkards(d_array);

					// unlock the game
					setGameStatus(MENU);
				}
			}; break;

			case GAME_LOCKED:
			{
				drawPlayGame();
				drawRest();
				printStatsOnScreen();

				switch (err_cond->err_code)
				{
					case ERR_BEER_0:
					case ERR_BEER_1:
					{
						if (gameDelays.falling_lag != 0)
						{
							fall_beer();
						}
						// FIXME this code block is duplicated
						else
						{
							if (getLives() == 0)
							{
								setGameStatus(GAME_OVER);
								gameDelays.waiting_screen = WAITING_SCREEN;
								break;
							}

							if (gameDelays.waiting_screen != 0)
							{
								printMessage("Get Ready!",
									(SCREEN_WIDTH / 2) - (10 / 2),
									(SCREEN_HEIGHT / 2) + 20);
							}
							else
							{
								// clear screen
								clearBeers(b_array);
								clearDrunkards(d_array);

								// unlock the game
								setGameStatus(GAME_PLAYING);
							}
						}
					}; break;

					case ERR_DRUNKARD:
					{
						// false = the barman has reached the end
						// of the table
						if (false == drawRageDrunkard())
						{
							if (getLives() == 0)
							{
								setGameStatus(GAME_OVER);
								gameDelays.waiting_screen = WAITING_SCREEN;
								break;
							}

							if (gameDelays.waiting_screen != 0)
							{
								printMessage("Get Ready!",
									(SCREEN_WIDTH / 2) - (10 / 2),
									(SCREEN_HEIGHT / 2) + 20);
							}
							else
							{
								// clear screen
								clearBeers(b_array);
								clearDrunkards(d_array);

								// unlock the game
								setGameStatus(GAME_PLAYING);
							}
						}

					}; break;
				} // end of err->code switch
			}; break; // end GAME_LOCKED

			default:
				printf("Could not determine game status!\n");
		}

		// Finally draw the whole picture frame
		System_DrawFrame(ot, &cputime, &gputime);

#if DEBUG
		FntPrint(psxstats, "cpu=%d, gpu=%d", cputime, gputime);
		FntFlush(psxstats);
#endif

		// Calculate delays
		calcDelays();
	}

	System_Exit();
	freeAll();
	return true;
}