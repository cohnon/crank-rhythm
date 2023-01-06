#include "game.h"

#include "pd_api/pd_api_gfx.h"
#include "song.h"
#include "song_player.h"
#include <stdio.h>

#define DISK_SIZE 64
#define DISK_SIZE_MAX 74

float disk_current_angle;

struct GameData data;

const char* fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";
LCDFont* font = NULL;

LCDBitmap* x_bitmap = NULL;

int map_select;
int map_index;
char map_ids[50][100];

static void draw_disk();

void game_setup_pd(PlaydateAPI* playdate) {
	data.playdate = playdate;
}

static void get_song(const char* filename, void* userdata) {
	strcpy_s(map_ids[map_index++], 50, filename);
}

static void update_main_menu() {
	if (data.first_update) {
		data.first_update = 0;
		data.playdate->graphics->drawText("4Tune", 5, kASCIIEncoding, 200, 120);
		data.playdate->graphics->drawText("press any button", 16, kASCIIEncoding, 200, 150);
	}
	
  PDButtons buttons;
  data.playdate->system->getButtonState(&buttons, NULL, NULL);
	
	if (buttons > 0) {
		data.state = GAME_STATE_SONG_LIST;
		data.first_update = 1;
	}
}

static void update_song_list() {
	if (data.first_update) {
		data.playdate->graphics->clear(kColorWhite);
		data.first_update = 0;
		for (int i = 0; i < map_index; ++i) {
			data.playdate->graphics->drawText(map_ids[i], 100, kASCIIEncoding, 40, 40 + 20 * i);
		}
	
		data.playdate->graphics->fillRect(
			40, 40 + 20 * map_select,
			200, 20,
			kColorXOR
		);
	}
	
  PDButtons pressed;
  data.playdate->system->getButtonState(NULL, &pressed, NULL);
	
	int prev = map_select;
	if (pressed & kButtonDown) {
		map_select += 1;
		if (map_select == map_index) {
			map_select = 0;
		}
	}
	if (pressed & kButtonUp) {
		map_select -= 1;
		if (map_select < 0) {
			map_select = map_index - 1;
		}
	}
	if (pressed & kButtonA) {
		int i = 0;
		while (1) {
			if (i > 50) {
				break;
			}
			if (map_ids[map_select][i] == '/') {
				map_ids[map_select][i] = 0;
				break;
			}
			if (map_ids[map_select][i] == 0) {
				break;
			}
			i += 1;
		}
		song_open(data.playdate, &data.song_player, map_ids[map_select]);
		data.state = GAME_STATE_SONG;
		data.first_update = 1;
	}
	
	if (pressed > 0) {
		data.playdate->graphics->fillRect(
			40, 40 + 20 * prev,
			200, 20,
			kColorXOR
		);
	
		data.playdate->graphics->fillRect(
			40, 40 + 20 * map_select,
			200, 20,
			kColorXOR
		);
	}
}

void update_song() {	
  PDButtons pressed;
  data.playdate->system->getButtonState(NULL, &pressed, NULL);
	
	if (pressed & kButtonA) {
		sp_play(&data.song_player);
	}
	
	data.playdate->graphics->clear(kColorWhite);
			
	sp_update(&data.song_player);
	song_update(data.playdate, &data.song_player);
	song_draw(&data);
  draw_disk();
	
	// HACK DEBUG
	// int title_width = data.playdate->graphics->getTextWidth(font, level.name, 26, kASCIIEncoding, 0);
	// float completion = song_player.time / song_player.length;
	// float max = (200 - title_width / 2.0f);
	// if (completion < 0.5f) {
	// 	max *= completion * 2.0f;
	// }
	// data.playdate->graphics->drawLine(0, 10, max, 10, 5, kColorBlack);
	// if (completion > 0.5f) {		
	// 	data.playdate->graphics->drawLine(
	// 		(200 + title_width / 2), 10,
	// 		(200 + title_width / 2) + (200 - title_width / 2) * (((completion - 0.5f) * 2.0f)), 10,
	// 		5, kColorBlack
	// 	);
	// }
}

void game_init() {
	data.state = GAME_STATE_MAIN_MENU;
	data.first_update = 1;
	
	sp_init(&data.song_player, data.playdate);
			
	const char* err;
	font = data.playdate->graphics->loadFont(fontpath, &err);
	data.playdate->graphics->setFont(font);

	data.grey_bitmap = data.playdate->graphics->loadBitmap("grey.png", &err);
	data.stripe_bitmap = data.playdate->graphics->loadBitmap("stripe.png", &err);
	data.clear_bitmap = data.playdate->graphics->newBitmap(32, 32, kColorWhite);
	x_bitmap = data.playdate->graphics->loadBitmap("x.png", &err);
	
	if (data.grey_bitmap == NULL || data.stripe_bitmap == NULL || data.clear_bitmap == NULL) {
		data.playdate->system->logToConsole("UH OH");
	}
	
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
		default:
			if (data.first_update) {			
				data.playdate->graphics->clear(kColorWhite);
				data.first_update = 0;
			}
			update_song();
			break;
	}	

	data.playdate->system->drawFPS(0,20);
}

static void draw_disk() {
	float target_angle = data.playdate->system->getCrankAngle();

	// 0 -> 300
	// 350 -> 10
	float angle;
	float smooth = 0.4f;
	if (disk_current_angle + 360 - target_angle < target_angle - disk_current_angle) {
		angle = (disk_current_angle + smooth * (target_angle - 360 - disk_current_angle));
		if (angle < 0) {
			angle += 360.0f;
		}
		if (angle > 360) {
			angle -= 360.0f;
		}
	} else if (target_angle + 360 - disk_current_angle < disk_current_angle - target_angle) {
		angle = (disk_current_angle + smooth * (target_angle + 360 - disk_current_angle));
		if (angle < 0) {
			angle += 360.0f;
		}
		if (angle > 360) {
			angle -= 360.0f;
		}
	} else {
		angle = (disk_current_angle + smooth * (target_angle - disk_current_angle));
	}
	disk_current_angle = angle;
	
	int size = DISK_SIZE;
	PDButtons buttons;
	data.playdate->system->getButtonState(&buttons, NULL, NULL);
  
	if (buttons > 0) {
		size = DISK_SIZE_MAX;
	}
  
	int offset = (DISK_SIZE_MAX - size) / 2;
	int bounds_x = 400 / 2 - DISK_SIZE_MAX / 2;
	int bounds_y = 240 / 2 - DISK_SIZE_MAX / 2;
	  
	// clear
	data.playdate->graphics->fillEllipse(bounds_x + offset, bounds_y + offset, size, size, 0.0f, 0.0f, kColorWhite);
	// stripe
	data.playdate->graphics->setStencilImage(data.grey_bitmap, 1);
	data.playdate->graphics->fillEllipse(bounds_x + offset, bounds_y + offset, size, size, angle + 0.0f, angle + 90.0f, kColorBlack);
	// grey
	data.playdate->graphics->setStencilImage(data.stripe_bitmap, 1);
	data.playdate->graphics->fillEllipse(bounds_x + offset, bounds_y + offset, size, size, angle + 180.0f, angle + 270.0f, kColorBlack);
	// white
	data.playdate->graphics->setStencilImage(data.clear_bitmap, 1);
	data.playdate->graphics->drawEllipse(bounds_x + offset, bounds_y + offset, size, size, 2, angle + 90.0f, angle + 180.0f, kColorBlack);
	// black
	data.playdate->graphics->fillEllipse(bounds_x + offset, bounds_y + offset, size, size, angle + 270.0f, angle + 360.0f, kColorBlack);
}

