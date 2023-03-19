#ifndef _DRUNKARD_H_
#define _DRUNKARD_H_

#define DRUNKARD_SPRITE_NUM_FRAMES 8 // total number of drunkard frames
#define DRUNKARD_SPRITE_W 32
#define DRUNKARD_SPRITE_H 32

#define DRUNKARD_BEER_PRIZE 100
#define DRUNKARD_BEER_TIP_MIN 300
#define DRUNKARD_BEER_TIP_MAX 500
#define DRUNKARK_TIP_PROBABILITY 4 // 1 out of x

#define DRUNKARD_MOVEMENT_WAIT 2 // 7	// timing for moving drunkards

/*
struct drunkardFrame
{
	int x;
	int y;
};
*/

enum drunkard_frames
{
	WALKING_0,	// drunkard advancing
	WALKING_1,

	YELLING_0,	// stop for yelling
	YELLING_1,

	GRABBING,	// grab the beer
	DRINKING,
	BURPING,	// after drinking
	WATCHING	// watch the show
};

enum DRUNKARD_STATUS
{
	D_EMPTY,
	D_MOVING,
	D_YELLING,
	D_GRABBING,
	D_DRINKING
};

enum drinking_frames
{
	DA_GRABBING,
	DA_DRINKING_0,	// drinking the beer (beer is at 75%)
	DA_DRINKING_1,	// drinking the beer (beer is at 0%)
	DA_SENDBACK
};

#define DRUNKARK_WALKING_FRAMES 16+2
extern int drunkard_animation[DRUNKARK_WALKING_FRAMES];

struct Drunkard
{
	// GsIMAGE d_image;	// image obj
	GsSPRITE d_sprt;		// sprite obj
	GsSPRITE drinkingSprt;	// if the drunkard is drinking, this is the "companion" sprite
	struct Beer *beer;		// a beer object he is associated with while drinking

	int frame_idx;		// current sprite frame idx (from drunkard_animation)
	int table_idx;		// table it will be placed on
	int speed;			// advancement speed
	bool forward;		// direction of movement
	int weight;			// how much he/she will retreat with a beer
	int drinking_time;	// how long will it take to gulp a beer
	int status;			// see DRUNKARD_STATUS
	int movement_pause;	// how long he will stand before advancing
	int gsimage_row;	// row of GsImage object: says which Drunkard to choose
	long dbgstats;		// NOT USED
};

bool addDrunkard(int pic_idx, struct Drunkard *d_array[]);
bool remDrunkard(struct Drunkard *item);
bool moveDrunkards(struct Drunkard *d_array[]);
bool drawDrunkards();
void listDrunkards(struct Drunkard *d_array[]);
bool retreatDrunkard(struct Drunkard *item);
bool restoreDrunkard(struct Drunkard *item);
bool setDrunkardStatus(struct Drunkard *item, int new_status);
bool clearDrunkards(struct Drunkard *d_array[]);

#endif /* _DRUNKARD_H_ */