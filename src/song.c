#include "song.h"
#include <math.h>
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

	char beatmap_text[5000];
	int file_text_offset;
	SDFile* file = playdate->file->open(beatmap_path, kFileRead);
	playdate->file->read(file, beatmap_text, 5000);
	
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

static float note_position_to_angle(int position) {	
  float note_angle;
	switch (position) {
		case NOTE_POS_L1:
		case NOTE_POS_R5:
			note_angle = 135.0f;
			break;
		case NOTE_POS_L2:
		case NOTE_POS_R4:
			note_angle = 112.5f;
			break;
		case NOTE_POS_L3:
		case NOTE_POS_R3:
			note_angle = 90.0f;
			break;
		case NOTE_POS_L4:
		case NOTE_POS_R2:
			note_angle = 67.5f;
			break;
		case NOTE_POS_L5:
		case NOTE_POS_R1:
			note_angle = 45.0f;
			break;
		default:
			note_angle = 0.0f;
	}
	
	return note_angle;
}

static int angle_to_color(float crank_angle, float note_angle) {		
	// since disk is rotationally symmetrical, we can clamp it to 180 degrees
	float lower_angle = crank_angle;
	if (lower_angle > 180.0f) {
		lower_angle -= 180.0f;
	}
	
	float crank_dist = note_angle - lower_angle;
	
	// modulus
	if (crank_dist < 0) {
		crank_dist += 180.0f;
	} else if (crank_dist > 180.0f) {
		crank_dist -= 180.0f;
	}
	
  if (crank_dist < 90.0f) {
		return NOTE_WHITE;
  } else {
		return NOTE_BLACK;
	}
}

void song_open(struct PlaydateAPI* playdate, struct SongPlayer* song_player, const char* path) {
  load_song(playdate, song_player, path);
}

void song_update(struct PlaydateAPI* playdate, struct SongPlayer* song_player) {
  PDButtons pressed;
  playdate->system->getButtonState(NULL, &pressed, NULL);
  float angle = playdate->system->getCrankAngle();
	
	struct Note* note;
	for (int i = level.index; i < level.note_count; ++i) {
		note = &level.notes[i];
		
		if (note->beat_time - SIGHTREAD_DISTANCE > song_player->beat_time) {
			break;
		}
		
		if (note->type == NOTE_CLICK) {
			// too late
			if (note->time + 0.11f < song_player->time) {
				level.miss_count += 1;
				level.index += 1;
			}
	
			// playdate->graphics->setStencilImage(clear_bitmap, 1);
			if (pressed > 0) {
				float diff = note->time - song_player->time;
				float diff_2 = diff * diff;
				
				// inside clickable range
				if (diff_2 < 0.11f * 0.11f && note->color == angle_to_color(playdate->system->getCrankAngle(), note_position_to_angle(note->position))) {
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
				level.index += 1;
        float note_angle = note_position_to_angle(note->position);				
        if (note->color == angle_to_color(playdate->system->getCrankAngle(), note_angle)) {
  				score += 50;
        }
			}
		} else if (note->type == NOTE_DANGER) {
			if (note->time < song_player->time) {
				level.index += 1;
				float note_angle = note_position_to_angle(note->position);
				if (note->color == angle_to_color(playdate->system->getCrankAngle(), note_angle)) {
					score -= 100;
				}
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
				
    // The lerped values are precalculated
		progress = 1.0f - (note->beat_time - data->song_player.beat_time) / SIGHTREAD_DISTANCE;
		switch (note->position) {
			case NOTE_POS_L1:
				x = lerp(0.0f, 177.0f, progress);
				y = lerp(0.0f, 97.0f, progress);
        break;
			case NOTE_POS_L2:
        x = lerp(0, 170.0f, progress);
        y = lerp(60.0f, 108.0f, progress);
        break;
			case NOTE_POS_L3:
        x = lerp(0, 168.0f, progress);
        y = lerp(120.0f, 120.0f, progress);
				break;
			case NOTE_POS_L4:
        x = lerp(0, 170.0f, progress);
        y = lerp(180.0f, 132.0f, progress);
        break;
			case NOTE_POS_L5:
				x = lerp(0.0, 177.0f, progress);
				y = lerp(240.0f, 143.0f, progress);
				break;
			case NOTE_POS_R1:
				x = lerp(400.0f, 223.0f, progress);
				y = lerp(0.0f, 97.0f, progress);
				break;
			case NOTE_POS_R2:
				x = lerp(400.0f, 212.0f, progress);
				y = lerp(60.0f, 108.0f, progress);
				break;
			case NOTE_POS_R3:
				x = lerp(400.0f, 232.0f, progress);
				y = lerp(120.0f, 120.0f, progress);
				break;
			case NOTE_POS_R4:
				x = lerp(400.0f, 212.0f, progress);
				y = lerp(180.0f, 132.0f, progress);
				break;
			case NOTE_POS_R5:
				x = lerp(400.0f, 223.0f, progress);
				y = lerp(240.0f, 143.0f, progress);
				break;
			default:
        x = 0.0f;
        y = 0.0f;
				break;
		}		
		
		// playdate->graphics->drawBitmap(note_bitmap, (int)x - 12, (int)y - 12, kBitmapUnflipped);

		if (note->type == NOTE_DANGER) {
			LCDBitmap* bitmap = note->color == NOTE_BLACK ? data->black_x_bitmap : data->white_x_bitmap;
			data->playdate->graphics->drawBitmap(bitmap, x - 12, y - 12, kBitmapUnflipped);
		} else {
			int note_size = 16;
	    if (note->color == NOTE_WHITE) {
	      data->playdate->graphics->drawEllipse(x - (note_size >> 1), y - (note_size >> 1), note_size, note_size, 1, 0.0f, 0.0f, kColorBlack);
	    } else {      
	  		data->playdate->graphics->fillEllipse(x - (note_size >> 1), y - (note_size >> 1), note_size, note_size, 0.0f, 0.0f, kColorBlack);
	    }
    
	    if (note->type == NOTE_CLICK) {
			  data->playdate->graphics->drawEllipse(x - (note_size >> 1) - 6, y - (note_size >> 1) - 6, note_size + 12, note_size + 12, 2, 0.0f, 0.0f, kColorBlack);
			}
		}
    
	}
  
	data->playdate->graphics->drawText(level.name, 26, kASCIIEncoding, (400 / 2), 0);
	char score_display[100];
	sprintf(score_display, "%d %d %d %.2f", score, level.miss_count, level.ok_count, data->song_player.time);
	data->playdate->graphics->drawText(score_display, 20, kASCIIEncoding, (400 / 2), 225);
}
