#include "song_list.h"

#include "../game.h"
#include <stdio.h>
#include <string.h>


// Callback for reading directory contents
static void get_song(const char* filename, void* userdata) {
  SongListData* song_list = (SongListData*)userdata;
  
  char buffer[50];
  strcpy(buffer, filename);

  // Remove ending '/' if it exists
  for (int i = 1; i < 50; ++i) {
    if (i == 50) {
      break;
    }
    if (buffer[i] == '/') {
      buffer[i] = 0;
      break;
    }
    if (buffer[i] == 0) {
      break;
    }
  }
  
  beatmap_load_header(&song_list->song_headers[song_list->map_index], buffer);
  
  song_list->map_index += 1;
}

static void move_selector(GameData* game, SongListData* song_list, int direction) {
  int start = song_list->map_select;
  if (direction > 0) {    
    song_list->map_select -= 1;
    if (song_list->map_select < 0) {
      song_list->map_select = 0;
    }
  } else {    
    song_list->map_select += 1;
    if (song_list->map_select == song_list->map_index) {
      song_list->map_select = song_list->map_index - 1;
    }
  }
  
  if (song_list->map_select == 0) {
    rhythm_stop(game->rhythmplayer);
  } else if (song_list->map_select != start) {
    FilePlayer* fileplayer = rhythm_getFileplayer(game->rhythmplayer);
    BeatmapHeader* header = &song_list->song_headers[song_list->map_select];
    rhythm_load(game->rhythmplayer, header->audio_path, header->bpm, header->offset);
    rhythm_skipTo(game->rhythmplayer, 34.0f);
    playdate->sound->fileplayer->setVolume(fileplayer, 0.0f, 0.0f);
    playdate->sound->fileplayer->fadeVolume(fileplayer, 1.0f, 1.0f, 2 * 44100, NULL);
    rhythm_play(game->rhythmplayer, 0);
  }
}

void song_list_on_start(void* game_data, void* song_list_data) {
  GameData* game = (GameData*)game_data;
  SongListData* song_list = (SongListData*)song_list_data;
  
  song_list->map_index = 1;
  song_list->map_select = 0;
  strcpy(song_list->song_headers[0].name, "Tutorial");
  playdate->file->listfiles("songs", get_song, song_list_data, 0);
  
  song_list->map_select_range = playdate->system->getCrankAngle();
  song_list->input_delay = game->time + 0.5f;
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

  if (game->time > song_list->input_delay && crank_dist > 37.0f * 2.0f) {
    if (playdate->system->getCrankChange() > 0) {
      move_selector(game, song_list, -1);
    } else {
      move_selector(game, song_list, 1);
    }
    song_list->map_select_range = crank_angle;
  }

  if (pressed & kButtonDown) {
    move_selector(game, song_list, -1);
  }
  if (pressed & kButtonUp) {
    move_selector(game, song_list, 1);
  }
  if (pressed & kButtonA) {
    if (song_list->map_select == 0) {
      scene_transition(game->scene_manager, game->tutorial_scene);
      return;
    }
    
    // TODO: is there a way to pass the song name to another scene without polluting GameData?
    memcpy(&game->header, &song_list->song_headers[song_list->map_select], sizeof(BeatmapHeader));
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
    playdate->graphics->drawText(song_list->song_headers[i].name, 100, kASCIIEncoding, 13, 25 + 30 * i);
  }
  playdate->graphics->setDrawMode(kDrawModeCopy);

  int length = 170;
  int y_pos = 19 + 30 * song_list->map_select;
  int selector_points[8] = {0, y_pos, length + 3, y_pos, length, y_pos + 30, 0, y_pos + 30};
  if (rhythm_isOnBeat(game->rhythmplayer, 0.2f)) {
    selector_points[1] -= 2;
    selector_points[3] -= 2;
    selector_points[5] += 2;
    selector_points[7] += 2;
    
    selector_points[2] += 5;
    selector_points[4] += 5;
  }
  playdate->graphics->fillPolygon(4, selector_points, kColorXOR, kPolygonFillNonZero);
  
  // Info Box
  playdate->graphics->fillRect(200, 20, 175, 200, kColorBlack);
  playdate->graphics->fillRect(203, 23, 169, 194, kColorWhite);
  
  BeatmapHeader* header = &song_list->song_headers[song_list->map_select];
  playdate->graphics->drawText(header->name, 20, kASCIIEncoding, 210, 30);
  
  if (song_list->map_select == 0) {
    playdate->graphics->drawText("Learn the rulez\n(this is temp i swer)", 40, kASCIIEncoding, 210, 50);
  } else {
    char buffer[20];
    sprintf(buffer, "BPM: %d", (int)header->bpm);
    playdate->graphics->drawText(buffer, 20, kASCIIEncoding, 210, 50);
    playdate->graphics->drawText("Some other stuff", 20, kASCIIEncoding, 210, 70);
  }
  
}

void song_list_on_end(void* game_data, void* song_list_data) {
  GameData* game = (GameData*)game_data;
  
  FilePlayer* fileplayer = rhythm_getFileplayer(game->rhythmplayer);
  playdate->sound->fileplayer->setVolume(fileplayer, 1.0f, 1.0f);    
  rhythm_stop(game->rhythmplayer);  
}
