#ifndef SONG_H
#define SONG_H

#include "game.h"
#include "song_player.h"

#include "pd_api.h"
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

struct Note {
  uint8_t type;
	uint8_t color;
  uint8_t position;
	uint8_t extra_data; // just to fill out the padding why not
  float beat_time;
	float time;
};

struct Song {
	int state;
	char name[26];
	int index;
	int score;
	int health;
	int miss_count;
	int ok_count;
	int good_count;
	int perfect_count;
	int danger_hit;
	int danger_miss;
	int normal_hit;
	int normal_miss;
	int note_count;
	int combo;
	float accuracy;
	struct Note* notes;
};

void song_set_data_ptr(GameData* data);
void song_open(GameData* data, struct SongPlayer* song_player, const char* path);
void song_close(GameData* data);
void song_update(GameData* data);
void song_draw(GameData* data);

#endif