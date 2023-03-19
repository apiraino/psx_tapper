#ifndef _MENU_H_
#define _MENU_H_

#define LETTER_W 8 // sprite letter width
#define LETTER_H 8 // sprite letter height

GsIMAGE menu_fnt, menu_bg;
GsSPRITE menu_item_sprt;

struct menu_list
{
	char *title;
	int start_x;
	int start_y;
};

bool setupMenu();
bool showMenu();
bool printMenuItems(struct menu_list menu[], int menu_pos_idx);

#endif _MENU_H_