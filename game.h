#ifndef _GAME_H_
#define _GAME_H_

#define BARMAN_SPRT_IDLE_W 30
#define BARMAN_SPRT_IDLE_H 60

// higher half of the barman
#define BARMAN_SPRT_BEER_TORSO_W 25
#define BARMAN_SPRT_BEER_TORSO_H 33

// tap
#define TAP_SPRT_W 29
#define TAP_SPRT_H 27

// beer on tap
#define BEER_TAP_SPRT_W 29
#define BEER_TAP_SPRT_H 32

// lower half of the barman
#define BARMAN_SPRT_BEER_LEGS_W 21
#define BARMAN_SPRT_BEER_LEGS_H 32

#define NUM_AVAIL_DRUNKARDS 4	// how many drunkard types we have in the GsIMAGE
#warning DBG reduced num of drunkards
#define MAX_DRUNKARDS 1 // 15		// MAX num drunkards on screen
#define MAX_BEERS 15
#define MAX_TIPS 10

#define BARMAN_BREATH_WAIT 15	// timing for barman at rest animation
#define INPUT_LAG 10 			// barman movement
#define FILLING_LAG 20			// beer mug fill time between frames
#define FALLING_LAG 120+100		// beer mug falling (100 = wait before screen appear)
#define DRUNK_GEN_WAIT 100		// wait time before generating a drunkard (will be configurable)
#define MENU_BLINK_WAIT 30		// blink timing for menu list items
#define WAITING_SCREEN 80		// timer to have the player wait for a bit

#define BARMAN_DRAGGING_SP 2	// how many pixel per frame the barman will go when
								// dragged over the table

#define LETTER_W 8 // sprite letter width
#define LETTER_H 8 // sprite letter height

enum currSetup
{
	S_MENU = 1,
	S_PLAY,
	S_HIGH
};
int curr_setup;

// FIXME wat??? -> How many seconds between a new drunkard generation
enum DIFFICULTY_LEVELS
{
	LVL1 = 0,
	LVL2,
	LVL3
};

struct Game
{
	char *player_name;
	int difficulty_lvl;
	int status; // see GAME_STATUS
	int points;
	int lives;
};
extern struct Game *currGame;	// current game

struct barman_pos
{
	int x;
	int y;
};
extern struct barman_pos barmanPosition[4];

extern struct Frame barman_frames[7];
extern struct Frame tap_frames[3];
extern struct Frame beerTap_frames[5];
extern struct Frame tapPos[4];

enum barman_frames
{
	BARMAN_BUSY_LEGS,
	IDLE_0,
	IDLE_1,
	DESPERATE,
	FILLING_IDLE,
	FILLING_OPERATING,
	DRAG
};

enum tap_frames
{
	TAP_NORMAL,
	TAP_BARMAN_0,
	TAP_BARMAN_1
};

extern struct GAME_PARAMS gameSetups[3];
extern struct table_coords tables[4];
int currGameSetupIdx;

// various delays
struct delays
{
	int barman_breath_wait;
	int input_wait;
	int filling_lag;
	int falling_lag; // beer falling
	int drunk_gen_wait;
	int menu_blink_wait;
	int beer_movement_wait;
	int drunkard_movement_wait;
	int waiting_screen;
};
extern struct delays gameDelays;

// TODO see if we can move some definitions in other files
GsIMAGE	bgImg, barmanImg, beerImg, beerStatImg, tipImg,
		fontImg, drunkardImg, drinkImg, padImg;

GsSPRITE barmanIdleSprt, barmanTapTorsoSprt, barmanTapLegsSprt,
			tapSprt[4], barmanBeerSprt, barmanFailSprt, barmanDragged;

GsSPRITE letterSprt, livesSprt[5], padSprt;

struct Tip
{
	GsSPRITE t_sprt;
	int table_idx;		// table it will be placed on
	int amount;			// how much will the drunkard leave
	bool enabled;		// visible / not visible
};

struct Drunkard *d_array[MAX_DRUNKARDS];
struct Beer *b_array[MAX_BEERS];
struct Tip *t_array[MAX_TIPS];

bool setupGame();
bool playGame();
bool calcDelays();
bool remLife();
int getLives(); // return remaining lives
bool addPoints(int amount);
bool leaveTip(int table_idx, int table_x, int table_y);
bool triggerEndGame();
int getGameStatus();
bool setGameStatus(int status);
bool restartGame();
bool checkCollisions(struct Beer *b_array[], struct Drunkard *d_array[]);
bool printStatsOnScreen();
bool printMessage(char *txt, int pos_x, int pos_y);
bool drawPlayGame();
bool drawRest();

// animation for the beer returning back
bool returnBeer(struct Beer *item, int x, int y);

// animation for barman dragged over the table
bool drawRageDrunkard();


#endif /* _GAME_H_ */