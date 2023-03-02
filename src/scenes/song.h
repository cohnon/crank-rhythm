#ifndef SONGSCENE_H
#define SONGSCENE_H

#include "../beatmap.h"
#include "../rhythm_player.h"
#include <stdint.h>


#define SONG_STATE_WON 0

#define NOTE_NORMAL 0
#define NOTE_CLICK 1
#define NOTE_DANGER 2
#define NOTE_SLIDER 3
#define NOTE_SPIN 4

#define NOTE_POS_L1 0x0
#define NOTE_POS_L2 0x1
#define NOTE_POS_L3 0x2
#define NOTE_POS_L4 0x3
#define NOTE_POS_L5 0x4
#define NOTE_POS_R1 0x5
#define NOTE_POS_R2 0x6
#define NOTE_POS_R3 0x7
#define NOTE_POS_R4 0x8
#define NOTE_POS_R5 0x9

#define NOTE_BLACK 0
#define NOTE_WHITE 1

#define MAX_HEALTH 100

typedef struct SongData {
  BeatmapHeader header;
  Beatmap beatmap;
  float countdown_timer;
  RhythmPlayer* rhythmplayer;
  PDMenuItem* quit_menu_item;
  int finished;

  int index;
  int score;
  int display_score;
  int health;
  int miss_count;
  int ok_count;
  int good_count;
  int perfect_count;
  int danger_hit;
  int danger_miss;
  int normal_hit;
  int normal_miss;
  int combo;
  float accuracy;

} SongData;

void song_on_start(void* game_data, void* song_data);
void song_on_update(void* game_data, void* song_data);
void song_on_end(void* game_data, void* song_data);

#endif