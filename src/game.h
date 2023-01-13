#ifndef GAME_H
#define GAME_H

#include "pd_api.h"
#include "song_player.h"
#include <stdint.h>

#define GAME_STATE_MAIN_MENU 0
#define GAME_STATE_SONG_LIST 1
#define GAME_STATE_SETTINGS 2
#define GAME_STATE_SONG 3
#define GAME_STATE_TUTORIAL 4

struct GameData {
  uint8_t state;
  uint8_t first_update;
  uint32_t frame;
  PlaydateAPI* playdate;
  LCDBitmap* black_x_bitmap;
  LCDBitmap* white_x_bitmap;
  LCDFont* font;
  LCDFont* accuracy_font;
  struct SongPlayer song_player;
};

void game_setup_pd(PlaydateAPI* playdate);
void game_init();
void game_update();

#endif