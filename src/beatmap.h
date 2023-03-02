#ifndef BEATMAP_H
#define BEATMAP_H

#include <stdint.h>

#define BEATMAP_LOAD_FAIL 0
#define BEATMAP_LOAD_SUCCESS 1

#define BEATMAP_MAX_STRING 32
#define BEATMAP_MAX_NOTES 1000

typedef struct BeatmapHeader {
  char name[BEATMAP_MAX_STRING];
  char artist[BEATMAP_MAX_STRING];
  char creator[BEATMAP_MAX_STRING];
  int difficulty;
  
  int highscore;
  
  float bpm;
  float offset;
  
  int length;
  char path[50];
  char audio_path[50];
  int path_length;
} BeatmapHeader;

typedef struct Note {
  uint8_t type;
  uint8_t color;
  uint8_t position;
  uint8_t extra_data; // just to fill out the padding why not
  float beat_time;
  float time;
} Note;

typedef struct Beatmap {
  char name[BEATMAP_MAX_STRING];
  float bpm;
  float offset;
  int notes_length;
  Note notes[BEATMAP_MAX_NOTES];
  char audio_path[50];
} Beatmap;

int beatmap_load_header(BeatmapHeader* header, const char* path);

// Creates a beatmap from a BeatmapHeader
int beatmap_load(Beatmap* beatmap, BeatmapHeader* header);

#endif