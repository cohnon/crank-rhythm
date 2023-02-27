#ifndef SONG_LIST_H
#define SONG_LIST_H

#include <stdint.h>


#define MAX_SONGS 10

typedef struct SongData {
  char name[32];
  char artist[32];
  char creator[32];
  char path[50];
  uint16_t bpm;
} SongData;

typedef struct SongListData {
  int16_t map_select;
  int16_t map_index;
  float map_select_range;
  SongData song[MAX_SONGS];
  
  // HACK
  char map_ids[MAX_SONGS][32];
} SongListData;

void song_list_on_start(void* game_data, void* song_list_data);
void song_list_on_update(void* game_data, void* song_list_data);
void song_list_on_end(void* game_data, void* song_list_data);

#endif