#include "game.h"

#include "rhythm_player.h"
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


  // ********************* //
 //  Game Initialisation  //
// ********************* //
void game_init(void) {
  
  // Playdate setup
  playdate->display->setRefreshRate(50);

  data.sound_effect = playdate->sound->sampleplayer->newPlayer();
  // HACK: this never frees the memory for start.wav
  playdate->sound->sampleplayer->setSample(data.sound_effect, playdate->sound->sample->load("audio/start"));

  // Initialise systems  
  rhythm_set_pd_ptr(playdate);

  // Load Assets
  const char* err;
  const struct playdate_graphics* graphics = playdate->graphics;
  data.font              = graphics->loadFont  (fontpath,            &err);
  data.basic_font        = graphics->loadFont  ("fonts/Basic.pft",   &err);
  data.title_bitmaps[0]  = graphics->loadBitmap("images/title1.png", &err);
  data.title_bitmaps[1]  = graphics->loadBitmap("images/title2.png", &err);
  data.black_x_bitmap    = graphics->loadBitmap("images/x.png",             &err);
  data.white_x_bitmap    = graphics->loadBitmap("images/white_x.png",       &err);
  data.clear_bitmap      = graphics->loadBitmap("images/clear.png",         &err);
  data.rhythmplayer      = rhythm_newPlayer();
  
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
void game_update(void) {
  
  // Timing
  float current_time = (float)playdate->system->getCurrentTimeMilliseconds() / 1000.0f;
  data.delta_time = current_time - data.time;  
  data.time = current_time;
  

  scene_update(data.scene_manager);

  
  data.frame += 1;

  // Debug
  playdate->system->drawFPS(0, 0);
}
