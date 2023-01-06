#include "song.h"
#include <stdio.h>

// HACK allow player to set this
#define SIGHTREAD_DISTANCE 3

struct Song level;
int score;

static float lerp(float x1, float x2, float t) {
	// return x1 + t * (x2 - x1);
	return x1 * (1 - t) + x2 * t;
}

static void load_song(PlaydateAPI* playdate, struct SongPlayer* song_player, const char* path) {
  char dir_path[100] = "songs/";
  strcpy_s(dir_path + 6, 100 - 6, path);
  
  char beatmap_path[100];
  strcpy_s(beatmap_path, 100, dir_path);
  strcpy_s(beatmap_path + strlen(dir_path), 50, "/beatmap");

	char beatmap_text[500];
	int file_text_offset;
	SDFile* file = playdate->file->open(beatmap_path, kFileRead);
	playdate->file->read(file, beatmap_text, 500);
	
	if (file == NULL) {
		playdate->system->logToConsole("%s\nFailed to read %s", path, beatmap_path);
    return;
	}

	int version;
	char song_path[100];
	char song_name[26];
	float bpm;
	float offset;
	sscanf_s(beatmap_text, "%d%s%s%f%f%n", &version, song_name, 25, song_path, 49, &bpm, &offset, &file_text_offset);
	
  char song_full_path[300] = "Test";
  strcpy_s(song_full_path, 100, dir_path);
  int path_len = strlen(song_full_path);
  song_full_path[path_len] = '/';
  
  strcpy_s(song_full_path + path_len + 1, 50, song_path);
  FilePlayer* song = playdate->sound->fileplayer->newPlayer();
	int fileplayer_result = playdate->sound->fileplayer->loadIntoPlayer(song, song_full_path);
  
  if (fileplayer_result == 0) {
    playdate->system->logToConsole("Failed to load %s", song_full_path);
    return;
  }
  
  sp_load(song_player, song, bpm, offset);

	strcpy_s(level.name, 26, song_name);
	level.note_count = 0;
	
	// bpm of brain power is 170 (2.371 offset)
	int type;
	int position;
	int color;
	float beat_time;
	int read;
	while (sscanf_s(beatmap_text + file_text_offset, "%d%d%d%f%n", &type, &position, &color, &beat_time, &read) != 0) {		
		level.notes[level.note_count].type = type;
		level.notes[level.note_count].position = position;
		level.notes[level.note_count].color = color;
		level.notes[level.note_count].beat_time = beat_time;
		level.notes[level.note_count].time = level.notes[level.note_count].beat_time * 60 / bpm;
						
		file_text_offset += read;
		level.note_count += 1;
	}
}

void song_open(struct PlaydateAPI* playdate, struct SongPlayer* song_player, const char* path) {
  load_song(playdate, song_player, path);
}

void song_update(struct PlaydateAPI* playdate, struct SongPlayer* song_player) {
  PDButtons pressed;
  playdate->system->getButtonState(NULL, &pressed, NULL);
	
	struct Note* note;
	for (int i = level.index; i < level.note_count; ++i) {
		note = &level.notes[i];
		
		if (note->beat_time - SIGHTREAD_DISTANCE > song_player->beat_time) {
			break;
		}
		
		if (note->type == NOTE_CLICK) {
			// too late
			if (note->time + 0.11f < song_player->time) {
				playdate->system->logToConsole("%f %f", note->time, song_player->time);
				level.miss_count += 1;
				level.index += 1;
			}
	
			// playdate->graphics->setStencilImage(clear_bitmap, 1);
			if (pressed > 0) {
				float diff = note->time - song_player->time;
				float diff_2 = diff * diff;
				if (diff_2 < 0.11f * 0.11f) {
					level.index += 1;
					if (diff_2 < 0.045f * 0.045f) {
						level.perfect_count += 1;					
						score += 100;
					} else if (diff_2 < 0.07f * 0.07f) {
						level.good_count += 1;
						score += 30;
					} else {
						level.ok_count += 1;
						score += 10;
					}
				}
			}
		} else if (note->type == NOTE_NORMAL) {
			if (note->time < song_player->time) {
				score += 50;
				level.index += 1;
			}
		}
	}
}

void song_draw(struct GameData* data) {	
	float x, y;
	float progress;
	struct Note* note;
	for (int i = level.index; i < level.note_count; ++i) {
		note = &level.notes[i];
		// not yet
		if (note->beat_time - SIGHTREAD_DISTANCE > data->song_player.beat_time) {
			break;
		}
				
		// playdate->graphics->setStencilImage(stripe_bitmap, 1);
		progress = 1.0f - (note->beat_time - data->song_player.beat_time) / SIGHTREAD_DISTANCE;
		switch (note->position) {
			case NOTE_POS_L1:
			case NOTE_POS_L2:
			case NOTE_POS_L3:
				x = lerp(0.0f, 177.0f, progress);
				y = lerp(0.0f, 97.0f, progress);
				break;
			case NOTE_POS_L4:
			case NOTE_POS_L5:
				x = lerp(0.0, 177.0f, progress);
				y = lerp(240.0f, 143.0f, progress);
				break;
			case NOTE_POS_R1:
			case NOTE_POS_R2:
			case NOTE_POS_R3:
				x = lerp(400.0f, 223.0f, progress);
				y = lerp(0.0f, 97.0f, progress);
				break;
			case NOTE_POS_R4:
			case NOTE_POS_R5:
				x = lerp(400.0f, 223.0f, progress);
				y = lerp(240.0f, 143.0f, progress);
				break;
			default:
				break;
		}		
		
		// playdate->graphics->drawBitmap(note_bitmap, (int)x - 12, (int)y - 12, kBitmapUnflipped);

    switch (note->color) {
      case NOTE_STRIPES:
        data->playdate->graphics->setStencilImage(data->stripe_bitmap, 1);
        break;
      case NOTE_GREY:
        data->playdate->graphics->setStencilImage(data->grey_bitmap, 1);
        break;
      default:
        break;
    }
    
		int note_size = 16;
    if (note->color == NOTE_WHITE) {
      data->playdate->graphics->setStencilImage(data->clear_bitmap, 1);
      data->playdate->graphics->drawEllipse(x - (note_size >> 1), y - (note_size >> 1), note_size, note_size, 1, 0.0f, 0.0f, kColorBlack);
    } else {      
  		data->playdate->graphics->fillEllipse(x - (note_size >> 1), y - (note_size >> 1), note_size, note_size, 0.0f, 0.0f, kColorBlack);
    }
    
    data->playdate->graphics->setStencilImage(data->clear_bitmap, 1);
    if (note->type == NOTE_CLICK) {
		  data->playdate->graphics->drawEllipse(x - (note_size >> 1) - 6, y - (note_size >> 1) - 6, note_size + 12, note_size + 12, 2, 0.0f, 0.0f, kColorBlack);
		}
    
	}
  
	data->playdate->graphics->drawText(level.name, 26, kASCIIEncoding, (400 / 2), 0);
	char score_display[100];
	sprintf(score_display, "%d %d %d %.2f", score, level.miss_count, level.ok_count, data->song_player.time);
	data->playdate->graphics->drawText(score_display, 20, kASCIIEncoding, (400 / 2), 225);
}
