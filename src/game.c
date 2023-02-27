#include "game.h"

#include "drawing.h"
#include "pd_api.h"
#include "scene.h"
#include "song.h"
#include "song_player.h"
#include "scenes/menu.h"
#include "scenes/song_list.h"
#include "scenes/tutorial.h"
#include "scenes/song.h"

#include <pd_api/pd_api_gfx.h>
#include <stdio.h>


PlaydateAPI* playdate; // Global

GameData data;

const char* fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";

// HACK
int map_select;
int map_index = 1;
float map_select_range;
char map_ids[50][100] = {"Tutorial"};

void game_setup_pd(PlaydateAPI* pd) {
  playdate = pd;
}


// Callback for reading directory contents
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

static void menu_home_callback(void* userdata) {
  song_close(&data);
  scene_transition(data.scene_manager, data.song_list_scene);
}

static void menu_debug_callback(void* userdata) {
  data.debug = playdate->system->getMenuItemValue(data.debug_menu);
}

void game_init() {
  song_set_data_ptr(&data);
        
  playdate->system->addMenuItem("Quit Song", menu_home_callback, NULL);
  data.debug_menu = playdate->system->addCheckmarkMenuItem("Debug", 0, menu_debug_callback, NULL);
  
  data.fileplayer = playdate->sound->fileplayer->newPlayer();
  data.sound_effect = playdate->sound->sampleplayer->newPlayer();
  // HACK: this never frees the memory for start.wav
  playdate->sound->sampleplayer->setSample(data.sound_effect, playdate->sound->sample->load("audio/start"));

  const char* err;
  data.font = playdate->graphics->loadFont(fontpath, &err);
  data.basic_font = playdate->graphics->loadFont("fonts/Basic.pft", &err);
  playdate->graphics->setFont(data.font);

  data.title_bitmaps[0] = playdate->graphics->loadBitmap("images/title1.png", &err);
  data.title_bitmaps[1] = playdate->graphics->loadBitmap("images/title2.png", &err);
  data.black_x_bitmap = playdate->graphics->loadBitmap("x.png", &err);
  data.white_x_bitmap = playdate->graphics->loadBitmap("white_x.png", &err);
  data.bg_tile_bitmap = playdate->graphics->loadBitmap("bg-tiles.png", &err);
  data.light_grey_bitmap = playdate->graphics->loadBitmap("light-grey.png", &err);
  data.clear_bitmap = playdate->graphics->loadBitmap("clear.png", &err);
      
  playdate->file->listfiles("songs", get_song, NULL, 0);
  
  // song_open(pd, &song_player, "song.4t");
  playdate->display->setRefreshRate(50);
  
  // Setup Scenes
  data.scene_manager = scene_new(&data);
  data.menu_scene = scene_add(data.scene_manager, sizeof(MenuData), menu_on_start, menu_on_update, menu_on_end);
  data.song_list_scene = scene_add(data.scene_manager, sizeof(SongListData), song_list_on_start, song_list_on_update, song_list_on_end);
  data.tutorial_scene = scene_add(data.scene_manager, 0, tutorial_on_start, tutorial_on_update, tutorial_on_end);
  data.song_scene = scene_add(data.scene_manager, 0, song_on_start, song_on_update, song_on_end);
  scene_transition(data.scene_manager, data.menu_scene);
}

void game_update() {
  scene_update(data.scene_manager);	

  data.frame += 1;

  // HACK
  #ifdef DEBUG
  if (data.debug) {
    float width = 100;
    float rows = 3;
    playdate->system->drawFPS(0, 0);

    playdate->graphics->fillRect(400 - width, rows * 18 + 4, width, 240 - (rows * 18 + 4), kColorWhite);
    playdate->graphics->drawRect(400 - width, rows * 18 + 4, width, 240 - (rows * 18 + 4), kColorBlack);

    for (int i = data.debug_log_start; i != data.debug_log_end; i = (i + 1) % 10) {
      playdate->graphics->drawText(data.debug_log[i], 15, kASCIIEncoding, 400 - width + 2, (rows * 18 + 4) + (18 * ((i - data.debug_log_start + 10) % 10)));
    }

    char buffer[20];
    playdate->graphics->fillRect(400 - width, 0, width, rows * 18 + 4, kColorWhite);
    playdate->graphics->drawRect(400 - width, 0, width, rows * 18 + 4, kColorBlack);
    if (data.song_player.beat_time > 0.0f) {		
      sprintf(buffer, "time: %d.%d", (int)data.song_player.beat_time, (int)((data.song_player.beat_time - (int)data.song_player.beat_time) * 10.0f + 0.5f));
    } else {
      sprintf(buffer, "time: 0.0");
    }
    playdate->graphics->drawText(buffer, 20, kASCIIEncoding, 400 - width + 2, 2 + 18 * 0);
    
    sprintf(buffer, "next: %d", (int)data.debug_next_note);
    playdate->graphics->drawText(buffer, 20, kASCIIEncoding, 400 - width + 2, 2 + 18 * 1);

    sprintf(buffer, "pos: %d", data.debug_next_note_position);
    playdate->graphics->drawText(buffer, 20, kASCIIEncoding, 400 - width + 2, 2 + 18 * 2);
  }
  #endif
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
