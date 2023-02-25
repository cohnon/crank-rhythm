#include "game.h"

#include "pd_api/pd_api_gfx.h"
#include "song.h"
#include "song_player.h"
#include <stdio.h>

float disk_current_angle;

struct GameData data;

const char* fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";

int map_select;
int map_index = 1;
float map_select_range;
char map_ids[50][100] = {"Tutorial"};

void game_setup_pd(PlaydateAPI* playdate) {
	data.playdate = playdate;
}

static void get_song(const char* filename, void* userdata) {
	strcpy(map_ids[map_index], filename);

	int i = 0;
	while (1) {
		if (i > 50) {
			break;
		}
		if (map_ids[map_index][i] == '/') {
			map_ids[map_index][i] = 0;
			break;
		}
		if (map_ids[map_index][i] == 0) {
			break;
		}
		i += 1;
	}
	
	map_index += 1;	
}

static void update_tutorial() {
	if (data.first_update) {
		data.first_update = 0;
		data.playdate->graphics->clear(kColorWhite);
		// data.playdate->graphics->drawText("Tutorial", 10, kASCIIEncoding, 5, 5);
		data.playdate->graphics->drawText("Spin the crank to spin the disk", 50, kASCIIEncoding, 5, 25);
		data.playdate->graphics->drawText("Allign the disk color with the incoming notes", 50, kASCIIEncoding, 5, 55);
		data.playdate->graphics->drawEllipse(5, 85, 16, 16, 2, 0.0f, 0.0f, kColorBlack);
		data.playdate->graphics->fillEllipse(8, 88, 10, 10, 0.0f, 0.0f, kColorBlack);
		data.playdate->graphics->drawText("Click any button when collidoing with disk", 50, kASCIIEncoding, 25, 85);
		data.playdate->graphics->drawBitmap(data.black_x_bitmap, 5, 115, kBitmapUnflipped);
		data.playdate->graphics->drawText("Don't let X's touch the same colour", 50, kASCIIEncoding, 30, 115);
		data.playdate->graphics->drawText("Click any button to continue...", 50, kASCIIEncoding, 5, 145);
	}
	
	PDButtons buttons;
	data.playdate->system->getButtonState(NULL, &buttons, NULL);
	if (buttons > 0) {
		data.state = GAME_STATE_SONG_LIST;
		data.first_update = 1;
	}
}

static void update_main_menu() {
	if (data.first_update) {
		data.first_update = 0;
		data.playdate->graphics->tileBitmap(data.bg_tile_bitmap, 0, 0, 400, 240, kBitmapUnflipped);
		data.playdate->graphics->drawText("4Tune", 5, kASCIIEncoding, 200, 120);
		data.playdate->graphics->drawText("press any button", 16, kASCIIEncoding, 200, 150);
	}
	
	if (data.frame % 4 == 0) {
		int offset = -32 + ((data.frame >> 2) % 32);
		data.playdate->graphics->tileBitmap(data.bg_tile_bitmap, offset, offset, 432, 262, kBitmapUnflipped);
		data.playdate->graphics->drawText("4Tune", 5, kASCIIEncoding, 200, 120);
		data.playdate->graphics->drawText("press any button", 16, kASCIIEncoding, 200, 150);
	}
	
  PDButtons buttons;
  data.playdate->system->getButtonState(NULL, &buttons, NULL);
	
	if (buttons > 0) {
		data.state = GAME_STATE_SONG_LIST;
		data.first_update = 1;
	}
}

static void update_song_list() {
	float crank_angle = data.playdate->system->getCrankAngle();
	
	if (data.first_update) {
		data.playdate->graphics->clear(kColorWhite);
		data.first_update = 0;
		for (int i = 0; i < map_index; ++i) {
			data.playdate->graphics->drawText(map_ids[i], 100, kASCIIEncoding, 13, 20 + 20 * i);
		}
	
		data.playdate->graphics->fillRect(
			10, 19 + 20 * map_select,
			380, 20,
			kColorXOR
		);
		
		map_select_range = crank_angle;
	}
	
  PDButtons pressed;
  data.playdate->system->getButtonState(NULL, &pressed, NULL);
	
	int prev = map_select;
	
	// scroll with crank
	int scrolled_with_crank = 0;
	float crank_dist = map_select_range + 37 - crank_angle;
	if (crank_dist < 0.0f) {
		crank_dist += 360.0f;
	}
	if (crank_dist > 37.0f * 2.0f) {
		if (data.playdate->system->getCrankChange() > 0) {		
			map_select += 1;
			if (map_select == map_index) {
				map_select = map_index - 1;
			}
		} else {
			map_select -= 1;
			if (map_select < 0) {
				map_select = 0;
			}
		}
		scrolled_with_crank = 1;
		map_select_range = crank_angle;
	}
	
	if (pressed & kButtonDown) {
		map_select += 1;
		if (map_select == map_index) {
			map_select = map_index - 1;
		}
	}
	if (pressed & kButtonUp) {
		map_select -= 1;
		if (map_select < 0) {
			map_select = 0;
		}
	}
	if (pressed & kButtonA) {
		if (map_select == 0) {
			data.state = GAME_STATE_TUTORIAL;
			data.first_update = 1;
			return;
		}
		song_open(data.playdate, &data.song_player, map_ids[map_select]);
		data.state = GAME_STATE_SONG;
		data.first_update = 1;
	}
	if (pressed & kButtonB) {
		data.state = GAME_STATE_MAIN_MENU;
		data.first_update = 1;
	}
	
	if (pressed > 0 || scrolled_with_crank) {
		data.playdate->graphics->fillRect(
			10, 19 + 20 * prev,
			380, 20,
			kColorXOR
		);
	
		data.playdate->graphics->fillRect(
			10, 19 + 20 * map_select,
			380, 20,
			kColorXOR
		);
	}
}

void update_song() {	
  PDButtons pressed;
  data.playdate->system->getButtonState(NULL, &pressed, NULL);
		
	data.playdate->graphics->clear(kColorWhite);
			
	sp_update(&data.song_player);
	song_update(&data);
	song_draw(&data);
}

static void menu_home_callback(void* userdata) {
	data.state = GAME_STATE_SONG_LIST;
	data.first_update = 1;
	song_close(&data);
}

static void menu_debug_callback(void* userdata) {
	data.debug = data.playdate->system->getMenuItemValue(data.debug_menu);
	data.playdate->system->logToConsole("%d", data.debug);
}

void game_init() {
	sp_init(&data.song_player, data.playdate);
	song_set_data_ptr(&data);
		
	data.playdate->system->addMenuItem("Quit Song", menu_home_callback, NULL);
	data.debug_menu = data.playdate->system->addCheckmarkMenuItem("Debug", 0, menu_debug_callback, NULL);
	
	data.state = GAME_STATE_MAIN_MENU;
	data.first_update = 1;
			
	const char* err;
	data.font = data.playdate->graphics->loadFont(fontpath, &err);
	data.playdate->graphics->setFont(data.font);

	data.black_x_bitmap = data.playdate->graphics->loadBitmap("x.png", &err);
	data.white_x_bitmap = data.playdate->graphics->loadBitmap("white_x.png", &err);
	data.bg_tile_bitmap = data.playdate->graphics->loadBitmap("bg-tiles.png", &err);
	data.light_grey_bitmap = data.playdate->graphics->loadBitmap("light-grey.png", &err);
	data.clear_bitmap = data.playdate->graphics->loadBitmap("clear.png", &err);
			
	data.playdate->file->listfiles("songs", get_song, NULL, 0);
  
	// song_open(pd, &song_player, "song.4t");
	data.playdate->display->setRefreshRate(50);
}

void game_update() {
	// data.playdate->graphics->clear(kColorWhite);
  
	switch (data.state) {
		case GAME_STATE_MAIN_MENU:
			update_main_menu();
			break;
		case GAME_STATE_SONG_LIST:
			update_song_list();
			break;
		case GAME_STATE_SONG:
			if (data.first_update) {			
				data.playdate->graphics->clear(kColorWhite);
				data.first_update = 0;
			}
			update_song();
			break;
		case GAME_STATE_TUTORIAL:
			update_tutorial();
			break;
		default:
			break;
	}
	
	data.frame += 1;

	if (data.debug) {
		float width = 100;
		float rows = 3;
		data.playdate->system->drawFPS(0, 0);

		data.playdate->graphics->fillRect(400 - width, rows * 18 + 4, width, 240 - (rows * 18 + 4), kColorWhite);
		data.playdate->graphics->drawRect(400 - width, rows * 18 + 4, width, 240 - (rows * 18 + 4), kColorBlack);

		for (int i = data.debug_log_start; i != data.debug_log_end; i = (i + 1) % 10) {
			data.playdate->graphics->drawText(data.debug_log[i], 15, kASCIIEncoding, 400 - width + 2, (rows * 18 + 4) + (18 * ((i - data.debug_log_start + 10) % 10)));
		}

		char buffer[20];
		data.playdate->graphics->fillRect(400 - width, 0, width, rows * 18 + 4, kColorWhite);
		data.playdate->graphics->drawRect(400 - width, 0, width, rows * 18 + 4, kColorBlack);
		if (data.song_player.beat_time > 0.0f) {		
			sprintf(buffer, "time: %d.%d", (int)data.song_player.beat_time, (int)((data.song_player.beat_time - (int)data.song_player.beat_time) * 10.0f + 0.5f));
		} else {
			sprintf(buffer, "time: 0.0");
		}
		data.playdate->graphics->drawText(buffer, 20, kASCIIEncoding, 400 - width + 2, 2 + 18 * 0);
		
		sprintf(buffer, "next: %d", (int)data.debug_next_note);
		data.playdate->graphics->drawText(buffer, 20, kASCIIEncoding, 400 - width + 2, 2 + 18 * 1);

		sprintf(buffer, "pos: %d", data.debug_next_note_position);
		data.playdate->graphics->drawText(buffer, 20, kASCIIEncoding, 400 - width + 2, 2 + 18 * 2);
	}
}

void debug_log(const char* msg) {
	int i = 0;
	for (; i < 14; ++i) {
		if (msg[i] == '\n') {
			break;
		}
		data.debug_log[data.debug_log_end][i] = msg[i];
	}
	data.debug_log[data.debug_log_end][i] = 0;

	data.debug_log_end = (data.debug_log_end + 1) % 10;
	if (data.debug_log_end == data.debug_log_start) {
		data.debug_log_start += 1;
	}
}
