// #include "stdio.h"
#include <stdlib.h>

#include "System.h"
#include "DataManager.h"
#include "Sprite.h"
#include "Prim2D.h"

#include "common.h"
#include "main.h"
#include "game.h"
#include "drunkard.h"
#include "beer.h"

/* [x,y] barman coordinates */
struct barman_pos barmanPosition[4] = {
	{330, 95},
	{360, 190},
	{395, 290},
	{425, 385}
};

// sprite [x,y] position into the TIM
struct Frame barman_frames[7] = {
	{90,    0},	// BARMAN_BUSY_LEGS
	{0,     0},	// IDLE FRAME 0
	{30,    0},	// IDLE FRAME 1
	{60,    0},	// DESPERATE
	{115,   0},	// FILLING_IDLE
	{115,  33},	// FILLING_OPERATING
	{0,   113}	// DRAG
};

/* Tap animation frame when used by the barman */
struct Frame tap_frames[3] = {
	{141, 0},
	{141, 29},
	{141, 58}
};

/* Positioning for 4 taps */
struct Frame tapPos[4] = {
	{362,  92},
	{395, 188},
	{425, 285},
	{458, 380}
};

struct Frame beerTap_frames[5] = {
	{0,   81},	// BEERTAP_EMPTY
	{29,  81},	// BEERTAP_025
	{58,  81},	// BEERTAP_050
	{87,  81},	// BEERTAP_100
	{116, 81}	// BEERTAP_FRO
};

struct delays gameDelays = { 
	0,	// barman_breath_wait
	0,	// input_wait
	0,	// filling_lag
	0,	// drunk_gen_wait
	0	// menu_blink_wait
};

struct Game *currGame;

struct GAME_PARAMS gameSetups[3] = {
	{"easy",
		200,	// drunkard_gen_time
		5,		// drunkard_speed
		50,		// drunkard_weight
		10,		// drunkard_drinking_time
		5},		// avail_lives
	{"normal", 70,  2, 3, 20, 3},
	{"hard",   35,  3, 1, 20, 1}
};

struct table_coords tables[] = {
	{110, 330, 80},	// [x_start, x_finish, y] first table (from top)
	{90, 360, 190},	// second table
	{60, 395, 290},	// third table
	{25, 425, 385}	// fourth table
};

bool barman_is_active;

bool setupGame()
{
	DataManager_Files game_data[] =
	{
		{ "bg.TIM", 0 },
		{ "barman.TIM", 0 },
		{ "beer_st.TIM", 0 },
		{ "beer_sh.TIM", 0 },
		{ "tip.TIM", 0 },
		{ "FONT8X8.TIM", 0 },
		// { "pad.TIM", 0 },
		{ "d_sheet.tim", 0 },
		{ "d_drink.tim", 0 },
		{ NULL, 0 }
	};

	enum SPRITES
	{
		BG,
		BARMAN,
		BEER_ST,
		BEER,
		TIP,
		FONT,
		// PAD,
		DRUNKARDS,
		DRINK
	};

	int i;

	// if current setup is the right one, skip
	if (curr_setup == S_PLAY)
		return true;

// #warning game lvl3
	currGameSetupIdx = LVL1;
	currGame->points = 0;
	currGame->difficulty_lvl = currGameSetupIdx;
	currGame->lives = gameSetups[currGameSetupIdx].avail_lives;

	// init array of drunkards
	for (i = 0; i < MAX_DRUNKARDS; ++i)
	{
		if (NULL == (d_array[i] = malloc3(sizeof(struct Drunkard))))
			fail(ERR, "Malloc failed while generating Drunkard object");
		d_array[i]->status = D_EMPTY;
	}

	// init array of beers
	for (i = 0; i < MAX_BEERS; ++i)
	{
		if (NULL == (b_array[i] = malloc3(sizeof(struct Beer))))
			fail(ERR, "Malloc failed while generating Beer object");
		b_array[i]->status = B_EMPTY;
	}

	// init array of tips
	for (i = 0; i < MAX_TIPS; ++i)
	{
		if (NULL == (t_array[i] = malloc3(sizeof(struct Tip))))
			fail(ERR, "Malloc failed while generating Tip object");
		t_array[i]->enabled = false;
	}

	DataManager_LoadDatas("DATA", game_data);

	// Setup sprites
	{
		/* Load background */
		Tim_Load(&bgImg, game_data[BG].address);
		
		/* Load barman */
		Tim_Load(&barmanImg, game_data[BARMAN].address);

		/* barman idle sprites */
		Sprite_Init(&barmanIdleSprt, &barmanImg,
			SPRITE_NORMAL,
			barman_frames[IDLE_0].x, barman_frames[IDLE_0].y,
			BARMAN_SPRT_IDLE_W, BARMAN_SPRT_IDLE_H);
		Sprite_SetPosition(&barmanIdleSprt,
			barmanPosition[IDLE_0].x,
			barmanPosition[IDLE_0].y);

		// barman busy legs sprite
		Sprite_Init(&barmanTapLegsSprt, &barmanImg,
			SPRITE_NORMAL,
			barman_frames[BARMAN_BUSY_LEGS].x, barman_frames[BARMAN_BUSY_LEGS].y,
			BARMAN_SPRT_BEER_LEGS_W, BARMAN_SPRT_BEER_LEGS_H);

		// tap sprite and position
		for (i = 0; i < 4; ++i)
		{
			Sprite_Init(&tapSprt[i], &barmanImg,
				SPRITE_NORMAL,
				tap_frames[TAP_NORMAL].x, tap_frames[TAP_NORMAL].y,
				TAP_SPRT_W, TAP_SPRT_H);
			Sprite_SetPosition(&tapSprt[i],
				tapPos[i].x - (TAP_SPRT_W /2),
				tapPos[i].y - (TAP_SPRT_H /2));
		}

		// beer on tap: position it beneath a tap sprite
		Sprite_Init(&barmanBeerSprt, &barmanImg,
			SPRITE_NORMAL,
			beerTap_frames[BEERTAP_EMPTY].x, beerTap_frames[BEERTAP_EMPTY].y,
			BEER_TAP_SPRT_W, BEER_TAP_SPRT_H);
		Sprite_SetPosition(&barmanBeerSprt,
			tapPos[0].x,
			tapPos[0].y + TAP_SPRT_H);

		// barman busy torso sprite
		Sprite_Init(&barmanTapTorsoSprt, &barmanImg,
			SPRITE_NORMAL,
			barman_frames[FILLING_IDLE].x, barman_frames[FILLING_IDLE].y,
			BARMAN_SPRT_BEER_TORSO_W, BARMAN_SPRT_BEER_TORSO_H);

		// barman fail sprite
		Sprite_Init(&barmanFailSprt, &barmanImg,
			SPRITE_NORMAL,
			barman_frames[DESPERATE].x, barman_frames[DESPERATE].y,
			BARMAN_SPRT_IDLE_W, BARMAN_SPRT_IDLE_H);

		// barman being dragged over the table
		// half height because this sprite is horizontal
		Sprite_Init(&barmanDragged, &barmanImg,
			SPRITE_NORMAL,
			barman_frames[DRAG].x, barman_frames[DRAG].y,
			BARMAN_SPRT_IDLE_W*2, BARMAN_SPRT_IDLE_H/2);

		printf(">>> w=%d\n", barmanDragged.w);

		// Beer image (on table) (sprites will be allocated when needed)
		Tim_Load(&beerImg, game_data[BEER].address);

		// Stats: lives
		Tim_Load(&beerStatImg, game_data[BEER_ST].address);
		for (i = 0; i < currGame->lives; ++i)
		{
			Sprite_Init(&livesSprt[i], &beerStatImg,
				SPRITE_NORMAL, 0, 0, beerStatImg.pw*2, beerStatImg.ph);
		}

		// Stats: points
		Tim_Load(&fontImg, game_data[FONT].address);
		Sprite_Init(&letterSprt, &fontImg,
			SPRITE_NORMAL, 0, 0, LETTER_W, LETTER_H);

		// Joypad buttons sprite (not needed)
		/*
		Tim_Load(&padImg, game_data[PAD].address);
		Sprite_Init(&padSprt, &padImg,
			SPRITE_NORMAL, 0, 0, padImg.pw*2, padImg.ph);
		*/

		// Tip image (sprites will be allocated when needed)
		Tim_Load(&tipImg, game_data[TIP].address);

		// Load drunkards images (NUM_AVAIL_DRUNKARDS)
		// (sprites will be allocated when needed)
		Tim_Load(&drunkardImg, game_data[DRUNKARDS].address);
		Tim_Load(&drinkImg, game_data[DRINK].address);
	}

	curr_setup = S_PLAY;
	return true;
}

bool playGame()
{
	int pic_idx; // id for drunkard image
	int beer_idx; // id for beer image
	static int barman_frame_idx;// idx for barman frame idling
	static int beer_frame_idx; 	// idx for beer frame filling
	static short barman_table_pos_idx;

	if (gameDelays.drunk_gen_wait == 0)
	{
		// listDrunkards(d_array);
		pic_idx = rand() % NUM_AVAIL_DRUNKARDS;	// one of the available avatars
// # warning DRUNKARDS DISABLED
#if 1 
		if (addDrunkard(pic_idx, d_array) == false)
			fail(ERR, "Could not add a new drunkard!");
		gameDelays.drunk_gen_wait = gameSetups[currGameSetupIdx].drunkard_gen_time;
#endif
	}

	// if moveBeers returns false, a beer has crashed
	if (moveBeers(b_array, barman_table_pos_idx) == false)
	{
		remLife();
		setGameStatus(GAME_LOCKED);

		// trigger barman failing sprite
		Sprite_SetPosition(&barmanFailSprt,
			barmanPosition[barman_table_pos_idx].x,
			barmanPosition[barman_table_pos_idx].y);

		// Init timer for beer falling
		gameDelays.falling_lag = FALLING_LAG;

		return false;
	}

	// listDrunkards(d_array);
	// if moveDrunkards return false, someone reached
	// the end of the table
	if (moveDrunkards(d_array) == false)
	{
		remLife();
		setGameStatus(GAME_LOCKED);

		// XXX hide the current barman (?) sprite
		// item->b_sprt.w = 0;
		// item->b_sprt.h = 0;

		// TODO trigger drunkard rage animation
		// trigger barman sprite being dragged
		// position him completely on the table
		Sprite_SetPosition(&barmanDragged,
			barmanPosition[barman_table_pos_idx].x -
							barmanDragged.w,
			barmanPosition[barman_table_pos_idx].y);

		return false;
	}

	// check for beers filling in this iteration
	beer_idx = haveBeersFilling(b_array);
	// printf("beers filling? %d\n", beer_idx);

	/***** Barman movement between tables *****/
	// barman can only move if the timer is over
	if (gameDelays.input_wait == 0)
	{
		// ... and he's not filling a mug
		if (beer_idx == -1)
		{
			if (IsPadPress(Pad1Down))
			{
				gameDelays.input_wait = INPUT_LAG;
				if (barman_table_pos_idx < 3)
				{
					barman_table_pos_idx++;
				}
			}

			if (IsPadPress(Pad1Up))
			{
				gameDelays.input_wait = INPUT_LAG;
				if (barman_table_pos_idx > 0)
				{
					barman_table_pos_idx--;
				}
			}

			// idle barman sprite positioning on screen
			Sprite_SetPosition(&barmanIdleSprt,
				barmanPosition[barman_table_pos_idx].x,
				barmanPosition[barman_table_pos_idx].y);

			// busy barman torso sprite positioning on screen
			Sprite_SetPosition(&barmanTapTorsoSprt,
				barmanPosition[barman_table_pos_idx].x,
				barmanPosition[barman_table_pos_idx].y);

			// busy barman legs sprite positioning on screen
			Sprite_SetPosition(&barmanTapLegsSprt,
				barmanPosition[barman_table_pos_idx].x,
				barmanPosition[barman_table_pos_idx].y + BARMAN_SPRT_BEER_TORSO_H);

			if (IsPadPress(Pad1Cross))
			{
				// generate a new beer and add to the list of active beers
				if (gameDelays.filling_lag <= 0)
				{
					// initialize filling animation
					gameDelays.filling_lag = FILLING_LAG;
					barman_frame_idx = 0;

					addBeer(
						barman_table_pos_idx, // table idx
						b_array);
					// listBeers(b_array);
				}
			}
		} // end of beer being filled

#if DEBUG
		if (IsPadPress(Pad1Triangle))
		{
			listBeers(b_array);
			listDrunkards(d_array);
		}
#endif

	} // end barman movevent between tables

	/**** BARMAN IS FILLING A BEER ****/
	// calculate the right barman frame
	if (beer_idx != -1)
	{
		// advance frame while the player is pressing Cross
		// (and I can count down the filling_lag)
		// otherwise stick to the last frame FILLING_OPERATING_4
		// even if the player keeps pressing Cross
		float fill_time = 0.0;
		if (IsPadPress(Pad1Cross))
		{
			barman_is_active = true;

			fill_time = (100 * gameDelays.filling_lag) / FILLING_LAG;
			if (fill_time <= 5.0)
			{
				barman_frame_idx = FILLING_OPERATING;
				beer_frame_idx = BEERTAP_FRO;
			}
			else if (fill_time <= 25.0)
			{
				barman_frame_idx = FILLING_OPERATING;
				beer_frame_idx = BEERTAP_100;
			}
			else if (fill_time <= 50.0)
			{
				barman_frame_idx = FILLING_OPERATING;
				beer_frame_idx = BEERTAP_050;
			}
			else if (fill_time <= 75.0)
			{	
				barman_frame_idx = FILLING_OPERATING;
				beer_frame_idx = BEERTAP_025;
			}
			else if (fill_time <= 100.0)
			{
				barman_frame_idx = FILLING_OPERATING;
				beer_frame_idx = BEERTAP_EMPTY;
			}
		}
		else
		{
			barman_is_active = false;

			if (fill_time <= 25.0)
			{
				barman_frame_idx = FILLING_IDLE;
				beer_frame_idx = BEERTAP_100;
			}
			else if (fill_time <= 50.0)
			{
				barman_frame_idx = FILLING_IDLE;
				beer_frame_idx = BEERTAP_050;
			}
			else if (fill_time <= 75.0)
			{
				barman_frame_idx = FILLING_IDLE;
				beer_frame_idx = BEERTAP_025;
			}
			else if (fill_time <= 100.0)
			{
				barman_frame_idx = FILLING_IDLE;
				beer_frame_idx = BEERTAP_EMPTY;
			}
		}

		// set barman frame
		Sprite_Change(&barmanTapTorsoSprt, &barmanImg,
			barman_frames[barman_frame_idx].x,
			barman_frames[barman_frame_idx].y);

		// set filling beer sprite
		Sprite_Init(&beerTapSprt, &barmanImg,
			SPRITE_NORMAL,
			beerTap_frames[beer_frame_idx].x,
			beerTap_frames[beer_frame_idx].y,
			BEER_TAP_SPRT_W, BEER_TAP_SPRT_W);

		Sprite_SetPosition(&beerTapSprt,
			tapSprt[barman_table_pos_idx].x,
			tapSprt[barman_table_pos_idx].y + BEER_TAP_SPRT_H);
	}
	else
	{
		/**** BARMAN IS IDLE ****/
		if (gameDelays.barman_breath_wait == 0)
		{
			gameDelays.barman_breath_wait = BARMAN_BREATH_WAIT;
			if ((barman_frame_idx % 2) == 0)
			{
				barman_frame_idx = IDLE_0;
				Sprite_Change(&barmanIdleSprt, &barmanImg,
					barman_frames[IDLE_0].x,
					barman_frames[IDLE_0].y);
			}
			else
			{
				barman_frame_idx = IDLE_1;
				Sprite_Change(&barmanIdleSprt, &barmanImg,
					barman_frames[IDLE_1].x,
					barman_frames[IDLE_1].y);
			}
		}
	}

	// where is the barman and what is he doing?
	// draw the tap sprites accordingly!
	{
		int i;
		for (i = 0; i < 4; ++i)
		{
			// this tap is the one next to the barman
			if (i == barman_table_pos_idx)
			{
				if (beer_idx != -1 && barman_is_active == true)
				{
					// barman is filling a beer
					Sprite_Change(&tapSprt[i], &barmanImg,
						tap_frames[TAP_BARMAN_1].x,
						tap_frames[TAP_BARMAN_1].y);
				}
				else if (beer_idx != -1 && barman_is_active == false)
				{
					// barman is idle at the tap
					Sprite_Change(&tapSprt[i], &barmanImg,
						tap_frames[TAP_BARMAN_0].x,
						tap_frames[TAP_BARMAN_0].y);
				}
				else
				{
					// barman is doing nothing
					Sprite_Change(&tapSprt[i], &barmanImg,
						tap_frames[TAP_NORMAL].x,
						tap_frames[TAP_NORMAL].y);
				}
			}
			else
			{
				// tap is not where the barman stands
				Sprite_Change(&tapSprt[i], &barmanImg,
					tap_frames[TAP_NORMAL].x,
					tap_frames[TAP_NORMAL].y);
			}
		}
	}

	// listBeers(b_array);

	// Finished filling a beer? Position the beer
	// on the table and launch it!
	// NOTE: there must be at most one beer in B_BEINGFILLED status.
	// NOTE1: when the wait is over, the beer can be launched.
	// NOTE2: "filling_lag" can be global, because we only have one barman
	// 			and he can fill one beer mug at a time!
	if (
		(beer_idx != -1)  &&
		(gameDelays.filling_lag <= 0) &&
		(!IsPadPress(Pad1Cross))
	)
	{
		setBeerStatus(b_array[beer_idx], B_MOVING);
	}

#if 0 // DEBUG
		for (i = 0; i < MAX_BEERS; ++i)
		{
			if (b_array[i]->dbgstats != 0)
			{
				FntPrint(b_array[i]->dbgstats, "%p", b_array[i]);
				FntFlush(b_array[i]->dbgstats);
			}
		}
#endif

	err_cond->err_table = -1;
	err_cond->err_code = ERR_NONE;
	return true;
}

bool checkCollisions(struct Beer *b_array[], struct Drunkard *d_array[])
{
	struct Beer **b = b_array;
	struct Drunkard **d = d_array;
	int i, j;
	bool goto_next_beer = false;

	/*
		For each beer, loop all drunkards and test for collisions.
	*/

	for (i = 0; i < MAX_BEERS; i++)
	{
		// printf("[checkCollisions] iterate beer %d", i);
		// only check running beers
		goto_next_beer = false;
		if (b[i]->status == B_MOVING)
		{
			// printf("[checkCollisions] Starting\n");
			for (j = 0; j < MAX_DRUNKARDS; j++)
			{
				// Only check D_MOVING and D_YELLING drunkards
				if (d[j]->status != D_MOVING && d[j]->status != D_YELLING)
				{
					continue;
				}

				if (goto_next_beer == true)
				{
					printf("[checkCollisions] Breaking bad\n");
					break;
				}

				// printf("[checkCollisions] iterate Drunkard %d", j);
				// if they're on the same table
				if (d[j]->table_idx == b[i]->table_idx)
				{
					// if the drunkard X coord touches/overlaps the beer
					if ( (d[j]->d_sprt.x + d[j]->d_sprt.w) >= b[i]->b_sprt.x)
					{
						printf("[checkCollisions] Beer 0x%p and drunkard 0x%p met on table=%d\n",
							b[i], d[j], d[j]->table_idx);

						addPoints(DRUNKARD_BEER_PRIZE);

						// if (NULL == (d[i]->beer = malloc3(sizeof(struct Beer))))
						// 	fail(ERR, "Malloc failed while generating Drunkard->beer object");

						d[j]->beer = b[i];
						retreatDrunkard(d[j]);

						// A beer meets a drunkark
						// Hide the beer while the drunkark is drinking
						// the make it appear again to return to the barman
						printf("[checkCollisions] Hiding beer %d (%p)\n", i, b[i]);
						remBeer(b[i]);

						// found a collision, we can skip the remaining drunkards
						// and check the next beer
						goto_next_beer = true;
					}
				}
			}
		}
	}

	return true;
}

bool calcDelays()
{
	int i;

	// delay for barman breathing
	if (gameDelays.barman_breath_wait != 0)
		gameDelays.barman_breath_wait--;

	// delay for barman movement
	if (gameDelays.input_wait != 0)
		gameDelays.input_wait--;

	// delay for beer running
	if (gameDelays.beer_movement_wait != 0)
		gameDelays.beer_movement_wait--;

	// delay for moving drunkards
	for (i = 0; i < MAX_DRUNKARDS; ++i)
	{
		if (d_array[i]->status == D_EMPTY ||
			d_array[i]->status == D_MOVING ||
			d_array[i]->status == D_GRABBING
		)
		{
			continue;
		}

		// printf("[calcDelays] [%d] Status %d\n", SystemGetTimeRef(), d_array[i]->status);

		// see drunkard_animation
		// if (d_array[i]->frame_idx > 8)
		{
			if (d_array[i]->movement_pause != 0)
			{
				// printf("[calcDelays] [%d] Slowing down\n", SystemGetTimeRef());
				d_array[i]->movement_pause--;
			}
			else
			{
				// printf("Resetting movement_pause to FULL\n");
				d_array[i]->movement_pause = DRUNKARD_MOVEMENT_WAIT;
			}
		}
	}

	// delay for beer filling
	if (gameDelays.filling_lag != 0 && barman_is_active == true)
		gameDelays.filling_lag -= 2;

	// delay for drunkard generation
	if (gameDelays.drunk_gen_wait != 0)
		gameDelays.drunk_gen_wait--;

	// delay for menu blinking
	if (gameDelays.menu_blink_wait != 0)
		gameDelays.menu_blink_wait--;

	// delay for beer falling
	if (gameDelays.falling_lag != 0)
		gameDelays.falling_lag -= 2;

	// delay for a waiting screen
	if (gameDelays.waiting_screen != 0)
		gameDelays.waiting_screen -= 1;

	return true;
}

bool remLife()
{
	currGame->lives -= 1;
	return true;
}

int getLives()
{
	return currGame->lives;
}

bool addPoints(int amount)
{
	currGame->points += amount;
	return true;
}

bool leaveTip(int table_idx, int table_x, int table_y)
{
	int i;
	for (i = 0; i < MAX_TIPS; ++i)
	{
		if (t_array[i]->enabled == false)
		{
			Sprite_Init(&t_array[i]->t_sprt, &tipImg,
				SPRITE_NORMAL, 0, 0, tipImg.pw*2, tipImg.ph);

			Sprite_SetPosition(&t_array[i]->t_sprt,
				table_x, table_y + 10);

			t_array[i]->amount = rnd_rng(DRUNKARD_BEER_TIP_MIN, DRUNKARD_BEER_TIP_MAX);
			t_array[i]->table_idx = table_idx;
			t_array[i]->enabled = true;

			printf("[leaveTip] Leaving %d\n", t_array[i]->amount);
			return true;
		}
	}

	// FIXME: no more tips allowed
	return false;
}

bool triggerEndGame()
{
	// TODO open high score screen
	return true;
}

int getGameStatus()
{
	return currGame->status;
}

bool setGameStatus(int status)
{
	currGame->status = status;
	return true;
}

bool restartGame()
{
	// TODO clear any overlay screen I may have
	setGameStatus(GAME_PLAYING);
	return true;
}

bool printStatsOnScreen()
{
	// draw stats: lives, points
	int i, lives_spacing = 0;

	// I don't think anyone will play beyong 9999 points
	char points[4+1]; points[0] = '\0';
	sprintf(points, "%d", currGame->points);
	printMessage(points, 5, 50);

	for (i = 0; i < currGame->lives; ++i)
	{
		Sprite_SetPosition(&livesSprt[i],
			0 + lives_spacing,
			15);
		Sprite_DrawFast(&livesSprt[i], ot);
		lives_spacing += 20;
	}

	return true;
}

bool drawRest()
{
	// show tips (if any)
	int i;
	for (i = 0; i < MAX_TIPS; ++i)
	{
		if (t_array[i]->enabled == true)
			Sprite_DrawFast(&t_array[i]->t_sprt, ot);
	}

	// draw four taps
	for (i = 0; i < 4; ++i)
	{
		Sprite_DrawFast(&tapSprt[i], ot);
	}

	return true;
}

bool printMessage(char *txt, int pos_x, int pos_y)
{
	short TextOffs;
	int str_len;
	char msg[ strlen(txt) +1]; msg[0] = '\0';
	str_len = strlen(txt);
	sprintf(msg, "%s", txt);

	for (TextOffs = 0; TextOffs < str_len; TextOffs++)
	{	
		Sprite_Change(&letterSprt, &fontImg,
			LETTER_W * (msg[TextOffs] % 16),
			LETTER_H * (msg[TextOffs] / 16)
		);

		Sprite_SetPosition(&letterSprt,
			pos_x + (LETTER_W * TextOffs),
			pos_y);
		Sprite_DrawFast(&letterSprt, ot);
	}

	return true;
}

bool drawPlayGame()
{
	// Draw background
	TextureDraw(&bgImg, 0, 0);

	// what the name says...
	drawBeers();
	drawDrunkards();

// #warning DGB not drawing barman
// 	return 0;

	// draw barman
	if (
		(err_cond->err_code == ERR_BEER_0) ||
		(err_cond->err_code == ERR_BEER_1)
		)
	{
		Sprite_DrawFast(&barmanFailSprt, ot);
		return true;
	}
	else if (err_cond->err_code == ERR_DRUNKARD)
	{
		Sprite_DrawFast(&barmanDragged, ot);
		return true;
	}

	if (haveBeersFilling(b_array) != -1)
	{
		Sprite_DrawFast(&barmanTapTorsoSprt, ot);
		Sprite_DrawFast(&barmanTapLegsSprt, ot);
		Sprite_DrawFast(&beerTapSprt, ot);
	}
	else
	{
		Sprite_DrawFast(&barmanIdleSprt, ot);
	}

	return true;
}

bool drawRageDrunkard()
{
	// move him onto the table until the sprite
	// doesn't completely disappear behind the end of
	// the table

	printf(">>> problem occurs at table %d x=%d, w=%d tbl=%d\n", err_cond->err_table,
		barmanDragged.x, barmanDragged.w,
		tables[err_cond->err_table].x_start);

	// check whether we can stop the animation
	if ( (barmanDragged.x + barmanDragged.w)
		< tables[err_cond->err_table].x_start)
	{
		return false;
	}

	Sprite_SetPosition(&barmanDragged,
		barmanDragged.x - BARMAN_DRAGGING_SP,
		barmanDragged.y);

	return true;
}