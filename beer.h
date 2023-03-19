#ifndef _BEER_H_
#define _BEER_H_

struct Beer
{
	GsSPRITE b_sprt;
	// GsSPRITE b_image;
	int table_idx;
	int status;		// see BEER_STATUS
	// long dbgstats;
	bool forward;		// direction of movement
};

#define BEER_SPRT_W 32
#define BEER_SPRT_H 32

#define BEER_MOVEMENT_WAIT 15	// timing for running beers

#define FALLING_INCREMENT_Y 2	// falling beer "speed"

enum BEER_STATUS
{
	B_EMPTY = 0,
	B_BEINGFILLED,
	B_MOVING,
	B_BEINGDRANK,
	B_RETURNING
};

enum beer_frames
{
	RUNNING_0,	// running 0
	RUNNING_1,	// running 1
	FALLING,	// beer falling
	CRASHING_0,	// beer crashing frame 0
	CRASHING_1,	// beer crashing frame 1
	RETURNING,	// beer returning towards the barman
	FALLING_R,	// beer falling
};

enum beer_tap_frames
{
	BEERTAP_EMPTY,
	BEERTAP_025,
	BEERTAP_050,
	BEERTAP_100,
	BEERTAP_FRO
};

GsSPRITE beerFallingSprt, beerTapSprt;

bool addBeer(int table, struct Beer *b_array[]);
// return the ID of the beer being filled (at most one!)
int haveBeersFilling(struct Beer *b_array[]);
bool haveBeersRunning(struct Beer *b_array[]);
bool setBeerStatus(struct Beer *item, int new_status);
bool remBeer(struct Beer *item);
bool moveBeers(struct Beer *b_array[], int barman_pos_idx);
bool drawBeers();
bool fall_beer();
void listBeers(struct Beer *b_array[]);
bool clearBeers(struct Beer *b_array[]);
bool returnBeer(struct Beer *item, int x, int y);


#endif /* _BEER_H_ */