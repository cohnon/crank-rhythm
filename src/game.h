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
  LCDBitmap* bg_tile_bitmap;
  LCDBitmap* light_grey_bitmap;
  LCDBitmap* clear_bitmap;
  LCDFont* font;
  struct SongPlayer song_player;
  PDMenuItem* debug_menu;
  int debug;
  float debug_next_note;
  int debug_next_note_position;
  int debug_total_notes;
  char debug_log[10][10];
  int debug_log_start;
  int debug_log_end;
};

void game_setup_pd(PlaydateAPI* playdate);
void game_init();
void game_update();

void debug_log(const char* msg);

#endif