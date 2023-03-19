#include "stdio.h"
#include <stdlib.h>

#include "System.h"
#include "Sprite.h"

#include "common.h"
#include "main.h"
#include "game.h"
#include "drunkard.h"

struct Frame drunkard_frames[DRUNKARD_SPRITE_NUM_FRAMES] =
{
	{0,   0},	// WALKING_0
	{32,  0},	// WALKING_1
	{64,  0},	// YELLING_0
	{96,  0},	// YELLING_0

	{128,  0},	// GRABBING
	{160,  0},	// DRINKING
	{192,  0},	// BURPING
	{224,  0}	// WATCHING
};

// Drunkard animation while in D_MOVING|D_YELLING: mapping frame sequence
// these indexes are defined in drunkard_frames and stored in frame_idx
int drunkard_animation[DRUNKARK_WALKING_FRAMES] =
{
	0,1,0,1,0,1,0,1, 	// WALKING_0/1
	2,3,2,3,2,3,2,3,2,3	// YELLING_0/1
};

struct Frame drinking_frames[4] = 
{
	{0,   0},	// DA_GRABBING
	{32,  0},	// DA_DRINKING_0
	{64,  0},	// DA_DRINKING_1 (this is just the beer decreasing)
	{192, 0}	// DA_SENDBACK
};

bool addDrunkard(int pic_idx, struct Drunkard *d_array[])
{
	int i;
	struct Drunkard **d = d_array;
	int table_idx = rand() % 4;	// choose a table

	for (i = 0; i < MAX_DRUNKARDS; ++i)
	{
		// printf("[addDrunkard] Testing item %d %p\n", i, l[i]->d_sprt);
		// Skip already allocated objects
		if (d[i]->status != D_EMPTY)
			continue;

		// printf("Getting avatar at [%d,%d]\n", 0, DRUNKARD_SPRITE_H * pic_idx);
		Sprite_Init(&d[i]->d_sprt, &drunkardImg,
			SPRITE_NORMAL,
			0,								// x = 0
			DRUNKARD_SPRITE_H * pic_idx,	// y = [row] of avatar
			DRUNKARD_SPRITE_W, DRUNKARD_SPRITE_H);

		d[i]->frame_idx = 0;
		d[i]->d_sprt.x = tables[table_idx].x_start;
		d[i]->d_sprt.y = tables[table_idx].y;
		d[i]->table_idx = table_idx;
		d[i]->speed = gameSetups[currGameSetupIdx].drunkard_speed;
		d[i]->weight = gameSetups[currGameSetupIdx].drunkard_weight;
		d[i]->forward = true;
		d[i]->gsimage_row = pic_idx;
		d[i]->drinking_time = gameSetups[currGameSetupIdx].drunkard_drinking_time;
		d[i]->status = D_MOVING;
		d[i]->movement_pause = 0;

		printf("[addDrunkard] Created drunkard at table=%d\n", table_idx);
		return true;
	}

	// I couldn't work on any element of the array, expand by 25%
	// enlargeDrunkardArray(l);

	return true;
}

bool clearDrunkards(struct Drunkard *d_array[])
{
	int i;
	for (i = 0; i < MAX_DRUNKARDS; ++i)
	{
		if (d_array[i]->status == D_EMPTY)
			continue;
		remDrunkard(d_array[i]);
	}

	return true;
}

bool remDrunkard(struct Drunkard *item)
{
	item->status = D_EMPTY;
	item->d_sprt.w = 0;
	item->d_sprt.h = 0;
	return true;
}

bool moveDrunkards(struct Drunkard *d_array[])
{
	int i;
	static int curr_frame_idx;
	struct Drunkard **d = d_array;

	/*
		Increment drunkard position.
		Collisions are detected in checkCollisions()
		Just check whether the drunkard reaches the tap 
		or the beginning of the table
	*/

	// Increment drunkard position
	for (i = 0; i < MAX_DRUNKARDS; ++i)
	{
		// Skip object (drunkards) not allocated or not allowed to move yet (i.e. yelling etc.)
		if (d[i]->status == D_EMPTY || d[i]->movement_pause != 0)
			continue;

		// first manage drinking buddies
		if (d[i]->status == D_DRINKING)
		{
			if (d[i]->drinking_time != 0)
			{
				float d_time = 0.0;
				d_time = ( d[i]->drinking_time * 100) / gameSetups[currGameSetupIdx].drunkard_drinking_time;

				// Add the other frame (beer being gulped)
				if (d_time < 50.0)
				{
					// DA_DRINKING_1
					Sprite_Init(&d[i]->drinkingSprt, &drinkImg,
						SPRITE_NORMAL,
						drinking_frames[DA_DRINKING_1].x,
						drinking_frames[DA_DRINKING_1].y,
						DRUNKARD_SPRITE_W, DRUNKARD_SPRITE_H);
				}
				else if (d_time < 100.0)
				{
					// DA_DRINKING_0
					Sprite_Init(&d[i]->drinkingSprt, &drinkImg,
						SPRITE_NORMAL,
						drinking_frames[DA_DRINKING_0].x,
						drinking_frames[DA_DRINKING_0].y,
							DRUNKARD_SPRITE_W, DRUNKARD_SPRITE_H);
				}

				// draw the avatar while he's drinking
				Sprite_Change(&d[i]->d_sprt, &drunkardImg,
					drunkard_frames[ DRINKING ].x,
					drunkard_frames[ DRINKING ].y +
						(d[i]->gsimage_row * DRUNKARD_SPRITE_H));

				// Place the "companion" sprite right after the drunkard one
				Sprite_SetPosition(&d[i]->drinkingSprt,
					d[i]->d_sprt.x + DRUNKARD_SPRITE_W,
					d[i]->d_sprt.y);

				d[i]->drinking_time--;
			}
			else
			{
				// leave tip: one out of four possibilites
				if ( (rand() % DRUNKARK_TIP_PROBABILITY) == 1)
				{
					// move down a bit (Y coord) the tip position
					// TODO will be more precise once graphics will be done
					leaveTip(d[i]->table_idx,
						d[i]->d_sprt.x,
						d[i]->d_sprt.y + 15);
				}

				// once finished drinking the beer, he will be craving
				// for another one!
				restoreDrunkard(d[i]);

				// replace beer sprite and invert movement
				returnBeer(d[i]->beer,
					d[i]->d_sprt.x,
					d[i]->d_sprt.y);
				
			}
		}

		curr_frame_idx = drunkard_animation[ d[i]->frame_idx ];
		// printf("[moveDrunkard] curr frame_idx=%d\n", drunkard_animation[ d[i]->frame_idx ]);
		if (curr_frame_idx == 2 || curr_frame_idx == 3)
			d[i]->status = D_YELLING;
		else
			d[i]->status = D_MOVING;

		// *** Now move the drunkards ***

		// Drunkard reached the end of the table
		if (d[i]->d_sprt.x >= tables[d[i]->table_idx].x_finish)
		{
			printf("[moveDrunkard] Drunkard 0x%p has reached the tap on table %d\n", d[i], d[i]->table_idx);
			err_cond->err_table = d[i]->table_idx;
			err_cond->err_code = ERR_DRUNKARD;
			remDrunkard(d[i]);
#ifndef DONTBLOCK
			return false;
#endif
		}
		else
		{
			// continue moving
			// d_array[i]->movement_pause = 0;

			// if the drunkard is retreating, go backward
			// otherwise go forward
			if (d[i]->forward == true)
			{
				// drunkark moving
				// I'm using frames for sprite movement (see drunkard_animation)
				if (d[i]->status == D_MOVING)
				{
					d[i]->d_sprt.x += d[i]->speed;
				}
				else
				{
					// printf("don't move fwd\n");
				}

				// set next animation frame or reset the array
				if (d[i]->frame_idx == (DRUNKARK_WALKING_FRAMES-1))
				{
					d[i]->frame_idx = drunkard_animation[0];
				}
				else
				{
					d[i]->frame_idx = drunkard_animation[ d[i]->frame_idx++ ];						
				}

				// shift down the image to load the correct drunkard avatar
				Sprite_Change(&d[i]->d_sprt, &drunkardImg,
					drunkard_frames[ drunkard_animation[d[i]->frame_idx] ].x,
					drunkard_frames[ drunkard_animation[d[i]->frame_idx] ].y +
						(d[i]->gsimage_row * DRUNKARD_SPRITE_H));
			}
			else
			{
				/*
					If the wall is met while retreating, stop at the wall.
					At the next round, destroy the drunkard
				*/
				if ( (d[i]->d_sprt.x - d[i]->speed) <= tables[d[i]->table_idx].x_start)
					remDrunkard(d[i]);
				else
				{					
					if (d[i]->weight > 0)
					{
						d[i]->status = D_GRABBING;
						d[i]->d_sprt.x -= d[i]->speed;
						d[i]->weight -= d[i]->speed;

						// draw the beer right after the drunkard
						// DA_GRABBING
						Sprite_Init(&d[i]->drinkingSprt, &drinkImg,
							SPRITE_NORMAL,
							drinking_frames[DA_GRABBING].x,
							drinking_frames[DA_GRABBING].y,
								DRUNKARD_SPRITE_W, DRUNKARD_SPRITE_H);

						Sprite_SetPosition(&d[i]->drinkingSprt,
							d[i]->d_sprt.x + DRUNKARD_SPRITE_W,
							d[i]->d_sprt.y + 10); // a bit lower than the drunkard
					}
					else
					{
						d[i]->status = D_DRINKING;
						d[i]->movement_pause = DRUNKARD_MOVEMENT_WAIT;
					}
				}
			}

			Sprite_SetPosition(&d[i]->d_sprt,
				d[i]->d_sprt.x, d[i]->d_sprt.y);
		}
	}

	// printf("[moveDrunkard] Finished moving all drunkards\n");

	return true;
}

bool drawDrunkards()
{
	struct Drunkard **d = d_array;
	int i;
	for (i = 0; i < MAX_DRUNKARDS; ++i)
	{
		Sprite_DrawFast(&d[i]->d_sprt, ot);
		if (d[i]->status == D_DRINKING || d[i]->status == D_GRABBING)
			Sprite_DrawFast(&d[i]->drinkingSprt, ot);
	}

	return true;
}

void listDrunkards(struct Drunkard *d_array[])
{
	int i;
	for (i = 0; i < MAX_DRUNKARDS; ++i)
	{
		printf("[listDrunkards] [d=%d] %p x=%d status=%d\n", i,
			d_array[i], d_array[i]->d_sprt.x, d_array[i]->status);
	}
}

bool retreatDrunkard(struct Drunkard *item)
{
	item->forward = false;
	// item->movement_pause = 0;

	Sprite_Change(&item->d_sprt, &drunkardImg,
		drunkard_frames[GRABBING].x,
		drunkard_frames[GRABBING].y +
			(item->gsimage_row * DRUNKARD_SPRITE_H));

	return true;
}

bool restoreDrunkard(struct Drunkard *item)
{
	item->status = D_MOVING;
	item->movement_pause = 0;
	item->drinking_time = gameSetups[currGameSetupIdx].drunkard_drinking_time;
	item->forward = true;
	item->weight = gameSetups[currGameSetupIdx].drunkard_weight;

	// first frame of the sprite sheet
	Sprite_Change(&item->d_sprt, &drunkardImg,
		drunkard_frames[WALKING_0].x,
		drunkard_frames[WALKING_0].y +
			(item->gsimage_row * DRUNKARD_SPRITE_H));

	return true;
}

bool setDrunkardStatus(struct Drunkard *item, int new_status)
{
	item->status = new_status;
	return true;
}