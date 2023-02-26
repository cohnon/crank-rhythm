#ifndef GAME_H
#define GAME_H

#include "pd_api.h"
#include "song_player.h"
#include <stdint.h>

typedef uint8_t game_state;

#define GAME_STATE_MAIN_MENU 0
#define GAME_STATE_SONG_LIST 1
#define GAME_STATE_SETTINGS 2
#define GAME_STATE_SONG 3
#define GAME_STATE_TUTORIAL 4

typedef struct SongData {
  char name[32];
  char artist[32];
  char creator[32];
  char path[50];
  uint16_t bpm;
} SongData;

typedef struct GameData {
  game_state state;
  uint8_t first_update;
  uint32_t frame;
  PlaydateAPI* playdate;
  LCDBitmap* title_bitmaps[2];
  LCDBitmap* black_x_bitmap;
  LCDBitmap* white_x_bitmap;
  LCDBitmap* bg_tile_bitmap;
  LCDBitmap* light_grey_bitmap;
  LCDBitmap* clear_bitmap;
  LCDFont* font;
  SamplePlayer* sound_effect;
  PDMenuItem* debug_menu;
  int debug;
  float debug_next_note;
  int debug_next_note_position;
  int debug_total_notes;
  char debug_log[10][15];
  int debug_log_start;
  int debug_log_end;
  FilePlayer* fileplayer;
  LCDFont* basic_font;
  struct SongPlayer song_player;
  SongData songs[10];
  int song_count;
} GameData;

void game_setup_pd(PlaydateAPI* playdate);
void game_init();
void game_update();

void game_change_state(game_state state);

void debug_log(const char* msg);

#endif