// #include "stdio.h"
#include <stdlib.h>

#include "System.h"
#include "DataManager.h"
#include "Sprite.h"

#include "common.h"
#include "main.h"
#include "game.h"
#include "menu.h"

bool setupMenu()
{
	DataManager_Files menu_data[] =
	{
		{ "menu.TIM", 0 },
		{ "FONT8X8.TIM", 0 },
		{ NULL, 0 }
	};

	// if current setup is the right one, skip
	if (curr_setup == S_MENU)
		return true;

	DataManager_LoadDatas("DATA", menu_data);

	// background
	Tim_Load(&menu_bg, menu_data[0].address);
	
	// menu font
	Tim_Load(&menu_fnt, menu_data[1].address);

	Sprite_Init(&menu_item_sprt, &menu_fnt,
		SPRITE_NORMAL, 0, 0, menu_fnt.pw, menu_fnt.ph);

	curr_setup = S_MENU;
	return true;
}

bool showMenu()
{
	static int menu_pos_idx = 0;

	struct menu_list menu[] = {
		{"NEW GAME", 	360, 40}, /* [x,y] coordinates */
		{"HIGH SCORES",	380, 50},
		{"QUIT",		400, 60},
	};

	enum
	{
		NEWGAME,
		HIGHSCORE,
		QUIT
	};

	// Show BG
	TextureDraw(&menu_bg, 0, 0);

	// manage cursor: it can only move if the timer is over
	if (gameDelays.input_wait == 0)
	{
		if (IsPadPress(Pad1Down))
		{
			gameDelays.input_wait = INPUT_LAG;
			if (menu_pos_idx < 3)
			{
				menu_pos_idx++;
			}
		}

		if (IsPadPress(Pad1Up))
		{
			gameDelays.input_wait = INPUT_LAG;
			if (menu_pos_idx > 0)
			{
				menu_pos_idx--;
			}
		}

		if (IsPadPress(Pad1Cross))
		{
			gameDelays.waiting_screen = WAITING_SCREEN;
			switch(menu_pos_idx)
			{
				case NEWGAME:
				{
					setGameStatus(GAME_PLAYING);
				}; break;

				// TODO
				case HIGHSCORE:
				case QUIT:
					break;
			}
		}
	}

	printMenuItems(menu, menu_pos_idx);

	return true;
}

bool printMenuItems(struct menu_list menu[], int menu_pos_idx)
{
	short TextOffs;
	int str_len, color, i;
	static bool blink;
	int menu_len = sizeof(menu[0]) / sizeof(menu);

	for (i = 0; i < menu_len; ++i)
	{
		TextOffs = 0;
		str_len = strlen(menu[i].title);
		color = 0;

		if (menu_pos_idx == i)
		{
			if (blink == true && gameDelays.menu_blink_wait == 0)
			{
				gameDelays.menu_blink_wait = MENU_BLINK_WAIT;
				color = 255;
				blink = false;
			}
			else
			{
				color = 0;
				blink = true;
			}
		}
		else
		{
			color = 255;
		}

		for (TextOffs = 0; TextOffs < str_len; TextOffs++)
		{
			// TODO replace with my printMessage()
			menu_item_sprt.x = menu[i].start_x + (LETTER_W * TextOffs);
			menu_item_sprt.y = menu[i].start_y;
			menu_item_sprt.w = LETTER_W;
			menu_item_sprt.h = LETTER_H;
			menu_item_sprt.u = LETTER_W * (menu[i].title[TextOffs] % 16);
			menu_item_sprt.v = LETTER_H * (menu[i].title[TextOffs] / 16);
			menu_item_sprt.r = menu_item_sprt.g = menu_item_sprt.b = color;
			Sprite_DrawFast(&menu_item_sprt, ot);
		}
	}

	return true;
}