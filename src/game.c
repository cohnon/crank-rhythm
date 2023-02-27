#include "game.h"

#include "scenes/menu.h"
#include "scenes/song_list.h"
#include "scenes/tutorial.h"
#include "scenes/song.h"


PlaydateAPI* playdate; // Global

GameData data;

// TODO: use a better font
//       also, why is this up here?
const char* fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";

void game_setup_pd(PlaydateAPI* pd) {
  playdate = pd;
}

// Playdate menu buttons
static void menu_home_callback(void* userdata) {
  scene_transition(data.scene_manager, data.song_list_scene);
}

static void menu_debug_callback(void* userdata) {
  data.debug = playdate->system->getMenuItemValue(data.debug_menu);
}


  // ********************* //
 //  Game Initialisation  //
// ********************* //
void game_init() {
  
  // Playdate setup        
  playdate->display->setRefreshRate(50);

  playdate->system->addMenuItem("Quit Song", menu_home_callback, NULL);
  data.debug_menu = playdate->system->addCheckmarkMenuItem("Debug", 0, menu_debug_callback, NULL);

  data.fileplayer = playdate->sound->fileplayer->newPlayer();
  data.sound_effect = playdate->sound->sampleplayer->newPlayer();
  // HACK: this never frees the memory for start.wav
  playdate->sound->sampleplayer->setSample(data.sound_effect, playdate->sound->sample->load("audio/start"));

  // Load Assets
  const char* err;
  const struct playdate_graphics* graphics = playdate->graphics;
  data.font              = graphics->loadFont  (fontpath,            &err);
  data.basic_font        = graphics->loadFont  ("fonts/Basic.pft",   &err);
  data.title_bitmaps[0]  = graphics->loadBitmap("images/title1.png", &err);
  data.title_bitmaps[1]  = graphics->loadBitmap("images/title2.png", &err);
  data.black_x_bitmap    = graphics->loadBitmap("x.png",             &err);
  data.white_x_bitmap    = graphics->loadBitmap("white_x.png",       &err);
  data.bg_tile_bitmap    = graphics->loadBitmap("bg-tiles.png",      &err);
  data.light_grey_bitmap = graphics->loadBitmap("light-grey.png",    &err);
  data.clear_bitmap      = graphics->loadBitmap("clear.png",         &err);
  
  graphics->setFont(data.font);  
  
  // Setup Scenes
  data.scene_manager   = scene_new(&data);
  data.menu_scene      = scene_add(data.scene_manager, sizeof(MenuData), menu_on_start, menu_on_update, menu_on_end);
  data.song_list_scene = scene_add(data.scene_manager, sizeof(SongListData), song_list_on_start, song_list_on_update, song_list_on_end);
  data.tutorial_scene  = scene_add(data.scene_manager, 0, tutorial_on_start, tutorial_on_update, tutorial_on_end);
  data.song_scene      = scene_add(data.scene_manager, sizeof(SongData), song_on_start, song_on_update, song_on_end);

  scene_transition(data.scene_manager, data.menu_scene);
}


  // ************* //
 //  Game Update  //
// ************* //
void game_update() {
  scene_update(data.scene_manager);

  data.frame += 1;

  // HACK make a better module
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
