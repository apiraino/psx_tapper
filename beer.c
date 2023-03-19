#include "stdio.h"
#include <stdlib.h>

#include "System.h"
#include "Sprite.h"

#include "common.h"
#include "main.h"
#include "game.h"
#include "beer.h"

struct Frame beer_frames[7] = {
	{0,   0},	// RUNNING_0
	{0,  32},	// RUNNING_1
	{0,  64},	// FALLING
	{0,  96},	// CRASHING_0
	{0, 128},	// CRASHING_1
	{0, 160},	// RETURNING
	{0, 192}	// FALLING AFTER RETURNING
};

bool addBeer(int table_idx, struct Beer *b_array[])
{
	int i;
	struct Beer **b = b_array;

	for (i = 0; i < MAX_BEERS; ++i)
	{
		// skip all beer mugs that are not empty
		if (b[i]->status != B_EMPTY)
			continue;

		printf("[addBeer] Adding a beer (%p) on table %d\n", b[i], table_idx);
		Sprite_Init(&b[i]->b_sprt, &beerImg,
			SPRITE_NORMAL,
			beer_frames[RUNNING_0].x,
			beer_frames[RUNNING_0].y,
			BEER_SPRT_W, BEER_SPRT_H);

		// place beer on this table at these coordinates
		b[i]->table_idx = table_idx;
		b[i]->b_sprt.w = beerImg.pw*2;   // 8bit image
		b[i]->b_sprt.x = tables[table_idx].x_finish;
		b[i]->b_sprt.y = tables[table_idx].y + 10;
		b[i]->status = B_BEINGFILLED;
		b[i]->forward = true;

		return true;
	}

	// moar beers!
	// enlargeBeersArray

	return false;
}

bool clearBeers(struct Beer *b_array[])
{
	int i;
	for (i = 0; i < MAX_BEERS; ++i)
		remBeer(b_array[i]);

	return true;
}

bool remBeer(struct Beer *item)
{
	item->status = B_EMPTY;
	item->b_sprt.w = 0;
	item->b_sprt.h = 0;
	return true;
}

bool haveBeersRunning(struct Beer *b_array[])
{
	// TODO check if in the list we have at least
	// one beer RUNNING
	int i;
	for (i = 0; i < MAX_BEERS; ++i)
	{
		if (b_array[i]->status == B_MOVING)
		{
			// printf("[haveBeersRunning] Found a beer RUNNNING!\n");
			return true;
		}
	}
	return false;
}

int haveBeersFilling(struct Beer *b_array[])
{
	int i;
	for (i = 0; i < MAX_BEERS; ++i)
	{
		if ( (b_array[i]->status > B_EMPTY) && (b_array[i]->status < B_MOVING) )
		{
			// printf("[haveBeersFilling] Found a beer FILLING!\n");
			return i;
		}
	}
	return -1;
}

bool setBeerStatus(struct Beer *item, int new_status)
{
	item->status = new_status;
	return true;
}

void listBeers(struct Beer *b_array[])
{
	int i;
	struct Beer **b = b_array;
	printf("---\nlistBeers\n");
	for (i = 0; i < MAX_BEERS; ++i)
	{
		printf("- beer 0x%p: status=%d, pos_x=%d, pos_y=%d\n",
			b[i], b[i]->status, b[i]->b_sprt.x, b[i]->b_sprt.y);
	}
}

bool moveBeers(struct Beer *b_array[], int barman_pos_idx)
{
	int i;
	static int beer_frame_idx; // idx for beer frame running
	struct Beer **b = b_array;

	/*
		0 increment beers position
		1 whether a beer crashes at the end of the table
		1a whether a beer crashes at the beginning of the table
		2 none if nothing happens
	*/

	// Increment beers position
	for (i = 0; i < MAX_BEERS; ++i)
	{
		/*
		// only move B_MOVING and B_RETURNING beers!
		if (b[i]->status != B_MOVING ||
			b[i]->status != B_RETURNING)
			continue;
		*/
		
		/* 1 */
		if (b[i]->status == B_MOVING)
		{
			if (b[i]->b_sprt.x <= tables[b[i]->table_idx].x_start)
			{
				/* Beer has crashed at the end of the table */
				printf("Beer 0x%p has crashed on table %d\n", b[i], b[i]->table_idx);
				err_cond->err_table = b[i]->table_idx;
				err_cond->err_code = ERR_BEER_0;
				remBeer(b[i]);
#ifndef DONTBLOCK
				return false;
#endif
			}

			// alternate movement sprites
			if (gameDelays.beer_movement_wait == 0)
			{
				gameDelays.beer_movement_wait = BEER_MOVEMENT_WAIT;
				if ((beer_frame_idx % 2) == 0)
				{
					beer_frame_idx = 1;
					Sprite_Change(&b[i]->b_sprt, &beerImg,
						beer_frames[RUNNING_1].x,
						beer_frames[RUNNING_1].y);
				}
				else
				{
					beer_frame_idx = 0;
					Sprite_Change(&b[i]->b_sprt, &beerImg,
						beer_frames[RUNNING_0].x,
						beer_frames[RUNNING_0].y);
				}
			}

		}
		/* 1a */
		else if (b[i]->status == B_RETURNING)
		{
			// check whether the returning beer will crash on the ground
			// of will be catched by the barman
			// printf("[moveBeers] Got a beer returning %p\n", b[i]);
			if (b[i]->b_sprt.x >= tables[b[i]->table_idx].x_finish)
			{
				// remove beer from table
				remBeer(b[i]);

#ifndef DONTBLOCK
				/* Beer has crashed at the beginning of the table */
				if (barman_pos_idx != b[i]->table_idx)
				{
					printf("Beer 0x%p has crashed on table %d [R]\n", b[i], b[i]->table_idx);
					err_cond->err_table = b[i]->table_idx;
					err_cond->err_code = ERR_BEER_1;
					return false;
				}
#endif
			}
		}
		else
		{
			// printf("[moveBeers] This beer %p will be ignored\n", b[i]);
			// only move B_MOVING and B_RETURNING beers!
			continue;
		}

		// now move it!
		if (b[i]->forward == true)
			b[i]->b_sprt.x -= 2;
		else
			b[i]->b_sprt.x += 2;			

		// printf("[moveBeers] Positioning beer @[%d,%d]\n",
		// 	b[i]->b_sprt.x, b[i]->b_sprt.y);

		Sprite_SetPosition(&b[i]->b_sprt,
			b[i]->b_sprt.x, b[i]->b_sprt.y);
	}

	// printf("[moveBeers] Finished moving all beers\n");

	/* 2 */
	return true;
}

bool drawBeers()
{
	struct Beer **b = b_array;
	int i;
	for (i = 0; i < MAX_BEERS; ++i)
	{
		if (b[i]->status != B_BEINGFILLED)
		{
			Sprite_DrawFast(&b[i]->b_sprt, ot);
		}
	}
	return true;
}

bool fall_beer()
{
	int table_idx = err_cond->err_table;
	static int falling_frame_idx = 0;
	static int falling_frame_pos_y;
	int fall_spr;

	if (&beerFallingSprt != NULL)
	{
		Sprite_Init(&beerFallingSprt, &beerImg,
			SPRITE_NORMAL,
			beer_frames[falling_frame_idx].x,
			beer_frames[falling_frame_idx].y,
			BEER_SPRT_W, BEER_SPRT_H);
	}

	// choose sprite for falling beer according
	// to the error code.
	if (err_cond->err_code == ERR_BEER_0)
		fall_spr = FALLING;
	else
		fall_spr = FALLING_R;

	if (gameDelays.falling_lag <= 5)
	{
		// about to end the animation
		// prepare the timer 
		gameDelays.waiting_screen = WAITING_SCREEN;
	}
	else if (gameDelays.falling_lag <= 120)
		falling_frame_idx = CRASHING_0;
	else if (gameDelays.falling_lag <= 140)
		falling_frame_idx = CRASHING_1;
	else if (gameDelays.falling_lag <= 160)
		falling_frame_idx = CRASHING_0;
	else if (gameDelays.falling_lag <= 180)
	{
		falling_frame_idx = fall_spr;
		falling_frame_pos_y += FALLING_INCREMENT_Y;
	}
	else if (gameDelays.falling_lag <= 200)
	{
		falling_frame_idx = fall_spr;
		falling_frame_pos_y += FALLING_INCREMENT_Y;
	}
	else if (gameDelays.falling_lag <= 220)
	{
		falling_frame_idx = fall_spr;
		falling_frame_pos_y = tables[table_idx].y;
	}

	Sprite_Change(&beerFallingSprt, &beerImg,
		beer_frames[falling_frame_idx].x,
		beer_frames[falling_frame_idx].y);

	if (err_cond->err_code == ERR_BEER_0)
	{
		Sprite_SetPosition(&beerFallingSprt,
			tables[table_idx].x_start,
			falling_frame_pos_y);
	}
	else
	{
		Sprite_SetPosition(&beerFallingSprt,
			tables[table_idx].x_finish,
			falling_frame_pos_y);
	}
	
	// if (err_cond->err_code == ERR_BEER_0)
	Sprite_DrawFast(&beerFallingSprt, ot);
	// else
	// 	Sprite_Flip(&beerFallingSprt, ot);

	return true;
}

bool returnBeer(struct Beer *item, int x, int y)
{
	struct Beer *b = item;
	int i;
	// printf("[returnBeer] Got beer %p\n", b);

	for (i = 0; i < MAX_BEERS; ++i)
	{
		// find the beer in the array that must be set
		// as B_RETURNING
		if (b_array[i] == b)
		{
			// printf("[returnBeer] Beer %p will be set to B_RETURNING @%d,%d\n",
			//	b_array[i],	x, y);

			b_array[i]->forward = false;
			b_array[i]->status = B_RETURNING;

			b_array[i]->b_sprt.w = beerImg.pw*2;
			b_array[i]->b_sprt.h = BEER_SPRT_H;
		
			Sprite_Change(&b_array[i]->b_sprt, &beerImg,
				beer_frames[RETURNING].x,
				beer_frames[RETURNING].y);

			Sprite_SetPosition(&b_array[i]->b_sprt,
				x,
				y);

			break;
		}
	}


	return true;
}