#ifndef SONG_LIST_H
#define SONG_LIST_H

#include "../beatmap.h"
#include <stdint.h>


#define MAX_SONGS 30

typedef struct SongListData {
  int16_t map_select;
  int16_t map_index;
  float map_select_range;
  float input_delay;
  BeatmapHeader song_headers[MAX_SONGS];
  
  // HACK
  // char map_ids[MAX_SONGS][32];
} SongListData;

void song_list_on_start(void* game_data, void* song_list_data);
void song_list_on_update(void* game_data, void* song_list_data);
void song_list_on_end(void* game_data, void* song_list_data);

#endif