#ifndef _COMMON_H_
#define _COMMON_H_

#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 480
#define SCREEN_MODE	MODE_NTSC

#define	rnd(max)			((rand()*(max))/(32768)+1)
#define	rnd_rng(min,max)	(rand()%(max-min))+min
#define MAX(a,b)			((a) > (b) ? a : b)
#define MIN(a,b)			((a) < (b) ? a : b)

// Reason to LOCK the game and rem a life
enum err_codes
{
	ERR_NONE,		// nothing bad happened
	ERR_BEER_0,		// beer reached beginning of table
	ERR_BEER_1,		// empty beer reached end of table
	ERR_DRUNKARD	// drunkard reached end of table
};

struct err_condition
{
	int err_table;		// where the condition happen
	int err_code;		// error code
};
struct err_condition *err_cond;

struct table_coords
{
	int x_start;
	int x_finish;
	int y;
};

struct Frame
{
	int x;
	int y;
};

struct GAME_PARAMS
{
	char *name;					// difficulty level label
	int drunkard_gen_time;		// waiting time before a new drunkard appearance
	int drunkard_speed;			// advance/retreat (in pixels) between animation frames
	int drunkard_weight;		// retreat (in pixels) when catching a beer
	int drunkard_drinking_time;	// waiting time while drinking a beer
	int avail_lives;			// available lives
};

// error message severity
enum MESSAGE_SEVERITY
{
	ERR = 0,
	WARN,
	DBG
};

void fail(int severity, char *msg); // output on stdout ERR,WARN,DBG messages

#endif /* _COMMON_H_ */