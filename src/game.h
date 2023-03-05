#ifndef GAME_H
#define GAME_H

#include "beatmap.h"
#include "rhythm_player.h"
#include "scene.h"
#include <pd_api.h>
#include <stdint.h>


typedef struct GameData {
  // Data
  uint32_t frame;
  float time;
  float delta_time;
  
  // Scene Data
  BeatmapHeader header;

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
  LCDBitmap* clear_bitmap;
  LCDBitmap* grey_bitmap;
  LCDFont* font;
  LCDFont* basic_font;
  LCDFont* font_cuberick;
  SamplePlayer* sound_effect;
  RhythmPlayer* rhythmplayer;
  int song_count;
    
} GameData;

extern PlaydateAPI* playdate;

void game_setup_pd(PlaydateAPI* pd);
void game_init(void);
void game_update(void);

#endif