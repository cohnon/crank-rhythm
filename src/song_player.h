#ifndef SONG_PLAYER_H
#define SONG_PLAYER_H

#include "pd_api.h"

typedef struct GameData GameData;

struct SongPlayer {
  float bpm;
  float length;
  float sec_per_beat;
  FilePlayer* current_song;
  float start_tick;
  int song_start;
  float time;
  float beat_time;
  float percentage;
  float offset;
  int playing;
};

int sp_load(GameData* data, struct SongPlayer* player, const char* path, const float bpm, const float offset);
void sp_play(GameData* data, struct SongPlayer* player);
void sp_stop(GameData* data, struct SongPlayer* player);
void sp_update(GameData* data, struct SongPlayer* player);

#endif