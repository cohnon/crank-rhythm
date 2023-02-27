#include "song_list.h"

#include "../game.h"
#include "../song.h"


// Callback for reading directory contents
static void get_song(const char* filename, void* userdata) {
  SongListData* song_list = (SongListData*)userdata;
  
  strcpy(song_list->map_ids[song_list->map_index], filename);

  int i = 0;
  while (1) {
    if (i > 50) {
      break;
    }
    if (song_list->map_ids[song_list->map_index][i] == '/') {
      song_list->map_ids[song_list->map_index][i] = 0;
      break;
    }
    if (song_list->map_ids[song_list->map_index][i] == 0) {
      break;
    }
    i += 1;
  }
  
  song_list->map_index += 1;	
}

void song_list_on_start(void* game_data, void* song_list_data) {
  SongListData* song_list = (SongListData*)song_list_data;
  
  song_list->map_index = 1;
  song_list->map_select = 0;
  strcpy(song_list->map_ids[0], "Tutorial");
  playdate->file->listfiles("songs", get_song, song_list_data, 0);
  
  song_list->map_select_range = playdate->system->getCrankAngle();
}

void song_list_on_update(void* game_data, void* song_list_data) {
  GameData* game = (GameData*)game_data;
  SongListData* song_list = (SongListData*)song_list_data;
  
  float crank_angle = playdate->system->getCrankAngle();
  PDButtons pressed;
  playdate->system->getButtonState(NULL, &pressed, NULL);
    
  // scroll with crank
  float crank_dist = song_list->map_select_range + 37 - crank_angle;
  if (crank_dist < 0.0f) {
    crank_dist += 360.0f;
  }

  if (crank_dist > 37.0f * 2.0f) {
    if (playdate->system->getCrankChange() > 0) {
      song_list->map_select += 1;
      if (song_list->map_select == song_list->map_index) {
        song_list->map_select = song_list->map_index - 1;
      }
    } else {
      song_list->map_select -= 1;
      if (song_list->map_select < 0) {
        song_list->map_select = 0;
      }
    }
    song_list->map_select_range = crank_angle;
  }
      
  if (pressed & kButtonDown) {
    song_list->map_select += 1;
    if (song_list->map_select == song_list->map_index) {
      song_list->map_select = song_list->map_index - 1;
    }
  }
  if (pressed & kButtonUp) {
    song_list->map_select -= 1;
    if (song_list->map_select < 0) {
      song_list->map_select = 0;
    }
  }
  if (pressed & kButtonA) {
    if (song_list->map_select == 0) {
      scene_transition(game->scene_manager, game->tutorial_scene);
      return;
    }
    song_open(game, &game->song_player, song_list->map_ids[song_list->map_select]);
    playdate->sound->fileplayer->stop(game->fileplayer);
    scene_transition(game->scene_manager, game->song_scene);
  }
  
  if (pressed & kButtonB) {
    scene_transition(game->scene_manager, game->menu_scene);
  }
  
  // render
  playdate->graphics->clear(kColorWhite);
  int points[8] = {0, 0, 150, 0, 130, 240, 0, 240};
  playdate->graphics->fillPolygon(4, points, kColorBlack, kPolygonFillNonZero);
  playdate->graphics->setDrawMode(kDrawModeNXOR);
  for (int i = 0; i < song_list->map_index; ++i) {
    playdate->graphics->drawText(song_list->map_ids[i], 100, kASCIIEncoding, 13, 20 + 30 * i);
  }
  playdate->graphics->setDrawMode(kDrawModeCopy);

  int length = 150;
  playdate->graphics->fillRect(
    0, 19 + 30 * song_list->map_select,
    length + 15, 30,
    kColorXOR
  );
  
  playdate->graphics->fillEllipse(length, 19 + 30 * song_list->map_select, 30, 30, 0.0f, 360.0f, kColorBlack);
}

void song_list_on_end(void* game_data, void* song_list_data) {
  
}
