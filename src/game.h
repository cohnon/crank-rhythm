#ifndef GAME_H
#define GAME_H

#include "scene.h"
#include "song_player.h"
#include <pd_api.h>
#include <stdint.h>


typedef struct GameData {
  // Data
  uint32_t frame;

  // Scenes
  SceneManager* scene_manager;
  scene_id menu_scene;
  scene_id song_list_scene;
  scene_id tutorial_scene;
  scene_id song_scene;

  // Assets  
  LCDBitmap* title_bitmaps[2];
  LCDBitmap* black_x_bitmap;
  LCDBitmap* white_x_bitmap;
  LCDBitmap* bg_tile_bitmap;
  LCDBitmap* light_grey_bitmap;
  LCDBitmap* clear_bitmap;
  LCDFont* font;
  LCDFont* basic_font;
  SamplePlayer* sound_effect;
  FilePlayer* fileplayer;
  struct SongPlayer song_player;
  int song_count;
  
  // Debug
  char song_path[32];
  PDMenuItem* debug_menu;
  int debug;
  float debug_next_note;
  int debug_next_note_position;
  int debug_total_notes;
  char debug_log[10][15];
  int debug_log_start;
  int debug_log_end;
} GameData;

extern PlaydateAPI* playdate;

void game_setup_pd(PlaydateAPI* pd);
void game_init();
void game_update();

void debug_log(const char* msg);

#endif