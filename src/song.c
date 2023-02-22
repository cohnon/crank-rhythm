#include "song.h"
#include "pd_api/pd_api_gfx.h"
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

#define DISK_SIZE 64
#define DISK_SIZE_MAX 74

// TODO Make sightreading based on time not beat time
#define SIGHTREAD_DISTANCE (3.0f / 170.0f)

struct Song level;

int display_score;

float prompt_ease;
int prompt_show;
int prompt;

// sin values [0, 2pi]
float health_shake[11] = {0, 2, 4, 4, 2, 0, -3, -5, -5, -3, 0};
int missed;
int health_shake_index = 10;

int failed_to_load_song = 0;

struct Particle {
	uint16_t x;
	uint16_t y;
	uint32_t life;
};

int particle_start;
int particle_end;
struct Particle particles[10];
int pulses[10];

static void particles_update() {
	for (int i = particle_start; i != particle_end; i = (i + 1) % 10) {
		particles[i].life += 1;
		pulses[i] += 1;
		if (particles[i].life > 15) {
			particle_start += 1;
			particle_start %= 10;
		}
	}
}

static void particles_draw(struct GameData* data) {
	for (int i = particle_start; i != particle_end; i = (i + 1) % 10) {
		// struct Particle* p = &particles[i];
		// data->playdate->graphics->drawRect(p->x - p->life / 2, p->y - p->life / 2, p->life, p->life, kColorBlack);
		// data->playdate->graphics->drawRect(p->x - p->life / 2 - 1, p->y - p->life / 2 - 1, p->life + 2, p->life + 2, kColorWhite);
		data->playdate->graphics->drawEllipse(200 - 32 - pulses[i] / 2, 120 - 32 - pulses[i] / 2, 64 + pulses[i], 64 + pulses[i], 1, 0.0f, 0.0f, kColorBlack);
	}
}

static void particles_make(uint16_t x, uint16_t y) {
	particles[particle_end] = (struct Particle){ x, y, 0 };
	pulses[particle_end] = 0;
	particle_end += 1;
	particle_end %= 10;
	
}

static float lerp(float x1, float x2, float t) {
	// return x1 + t * (x2 - x1);
	return x1 * (1 - t) + x2 * t;
}

static void get_note_position(int position, float progress, int* x, int* y) {
	switch (position) {
		case NOTE_POS_L1:
			*x = lerp(0.0f, 177.0f, progress);
			*y = lerp(0.0f, 97.0f, progress);
      break;
		case NOTE_POS_L2:
      *x = lerp(0, 170.0f, progress);
      *y = lerp(60.0f, 108.0f, progress);
      break;
		case NOTE_POS_L3:
      *x = lerp(0, 168.0f, progress);
      *y = lerp(120.0f, 120.0f, progress);
			break;
		case NOTE_POS_L4:
      *x = lerp(0, 170.0f, progress);
      *y = lerp(180.0f, 132.0f, progress);
      break;
		case NOTE_POS_L5:
			*x = lerp(0.0, 177.0f, progress);
			*y = lerp(240.0f, 143.0f, progress);
			break;
		case NOTE_POS_R1:
			*x = lerp(400.0f, 223.0f, progress);
			*y = lerp(0.0f, 97.0f, progress);
			break;
		case NOTE_POS_R2:
			*x = lerp(400.0f, 212.0f, progress);
			*y = lerp(60.0f, 108.0f, progress);
			break;
		case NOTE_POS_R3:
			*x = lerp(400.0f, 232.0f, progress);
			*y = lerp(120.0f, 120.0f, progress);
			break;
		case NOTE_POS_R4:
			*x = lerp(400.0f, 212.0f, progress);
			*y = lerp(180.0f, 132.0f, progress);
			break;
		case NOTE_POS_R5:
			*x = lerp(400.0f, 223.0f, progress);
			*y = lerp(240.0f, 143.0f, progress);
			break;
		default:
      *x = 0.0f;
      *y = 0.0f;
			break;
	}
}

static int load_song(PlaydateAPI* playdate, struct SongPlayer* song_player, const char* path) {
	
	debug_log("Loading");

  char dir_path[100] = "songs/";
  strcat(dir_path, path);
  
  char beatmap_path[100];
  strcpy(beatmap_path, dir_path);
  strcat(beatmap_path, "/beatmap.txt");

	char beatmap_text[50];
	int file_text_offset;
	SDFile* file = playdate->file->open(beatmap_path, kFileRead);
	int file_read_result = playdate->file->read(file, beatmap_text, 50);
	
	if (file_read_result == -1) {
		playdate->system->logToConsole("Failed to read %s", beatmap_path);
		debug_log("Fail1");
    return 0;
	}

	int version;
	char song_name[26];
	float bpm;
	float offset;
	sscanf(beatmap_text, "%d\n%s\n%f\n%f\n%n", &version, song_name, &bpm, &offset, &file_text_offset);

	debug_log(song_name);
		
  char song_full_path[100];
  strcpy(song_full_path, dir_path);
  strcat(song_full_path, "/audio");
  
  int sp_load_result = sp_load(song_player, song_full_path, bpm, offset);
	
	if (!sp_load_result) {
		playdate->system->logToConsole("Failed to load audio file");
		debug_log("Fail2");
		return 0;
	}
	
	strcpy(level.name, song_name);
	level.note_count = 0;
	
	// bpm of brain power is 170 (2.371 offset)
	float last_beat_time = -999.0f; // make sure each note is larger than the last
	
	int type;
	int color;
	int position;
	float beat_time;
	int read;
	int file_read;
  while (level.note_count < 1000) {
		playdate->file->seek(file, file_text_offset, SEEK_SET);
		file_read = playdate->file->read(file, beatmap_text, 50);
		sscanf(beatmap_text, "%d %d %d %f\n%n", &type, &color, &position, &beat_time, &read);

		beatmap_text[20] = 0;
		if (file_read == 0) {
			break;
		}

		if (level.note_count == 1) {
			debug_log("1st note");
			char buffer[10];
			snprintf(buffer, 10, "type:%d", type);
			debug_log(buffer);
			snprintf(buffer, 10, "col:%d", color);
			debug_log(buffer);
			snprintf(buffer, 10, "pos:%d", position);
			debug_log(buffer);
			snprintf(buffer, 10, "time:%d", (int)beat_time);
			debug_log(buffer);
		}

		level.notes[level.note_count].type = type;
		level.notes[level.note_count].color = color;
		level.notes[level.note_count].position = position;
		level.notes[level.note_count].beat_time = beat_time;
		level.notes[level.note_count].time = beat_time * 60 / bpm;

		assert(beat_time > last_beat_time);

		file_text_offset += read;
		level.note_count += 1;
	}

		
	char buffer[10];
	snprintf(buffer, 10, "tot:%d", level.note_count);
	debug_log(buffer);
	debug_log("Success");
		
	return 1;
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

static void reset() {	
	level.score = 0;
	display_score = 0;
	level.miss_count = 0;
	level.ok_count = 0;
	level.good_count = 0;
	level.perfect_count = 0;
	level.combo = 0;
	level.index = 0;
	level.health = MAX_HEALTH;
	particle_end = 0;
	particle_start = 0;
	failed_to_load_song = 0;
}

// HACK
struct GameData* d;

static void song_finish_callback(SoundSource* source) {
	level.health = 0;
}

void song_set_data_ptr(struct GameData* data) {
	d = data;
	level.notes = (struct Note*)data->playdate->system->realloc(NULL, sizeof(struct Note) * 1000);
}

void song_open(struct PlaydateAPI* playdate, struct SongPlayer* song_player, const char* path) {
	reset();
  int load_song_result = load_song(playdate, song_player, path);
	if (!load_song_result) {
		level.health = 0;
		failed_to_load_song = 1;
		return;
	}
	sp_play(song_player);
	playdate->sound->fileplayer->setFinishCallback(song_player->current_song, song_finish_callback);
}

void song_close(struct GameData* data) {
	sp_stop(&data->song_player);
}

void song_update(struct GameData* data) {	
  PDButtons pressed;
  data->playdate->system->getButtonState(NULL, &pressed, NULL);
			
	if (level.health < 1) {
		if (pressed > 0) {
			data->state = GAME_STATE_SONG_LIST;
			data->first_update = 1;
		}
		sp_stop(&data->song_player);
		return;
	}
	
	particles_update();
	
	// debug
	data->debug_next_note = level.notes[level.index].beat_time;
	data->debug_next_note_position = level.notes[level.index].position;
	data->debug_total_notes = level.note_count;

	struct Note* note;
	for (int i = level.index; i < level.note_count; ++i) {
		note = &level.notes[i];
		if (note->beat_time - data->song_player.bpm * SIGHTREAD_DISTANCE > data->song_player.beat_time) {
			break;
		}
		
		if (note->type == NOTE_CLICK) {
			// too late
			if (note->time + 0.11f < data->song_player.time) {
				level.miss_count += 1;
				level.index += 1;
				level.health -= 15;
				prompt_show = 1;
				missed = 1;
				prompt_ease = 1.0f;
				prompt = 0;
				level.combo = 0;
			}
	
			// playdate->graphics->setStencilImage(clear_bitmap, 1);
			if (pressed > 0 && level.index == i) {
				float diff = note->time - data->song_player.time;
				float diff_2 = diff * diff;
				
				// inside clickable range
				if (diff_2 < 0.11f * 0.11f && note->color == angle_to_color(data->playdate->system->getCrankAngle(), note_position_to_angle(note->position))) {
					level.index += 1;
					level.health += 5;
					prompt_show = 1;
					prompt_ease = 1.0f;
					level.combo += 1;
					float progress = 1.0f - (note->beat_time - data->song_player.beat_time) / (data->song_player.bpm * SIGHTREAD_DISTANCE);
					int x, y;
					get_note_position(note->position, progress, &x, &y);
					particles_make(x, y);
					if (diff_2 < 0.045f * 0.045f) {
						level.perfect_count += 1;
						level.score += 100;
						prompt = 3;
					} else if (diff_2 < 0.07f * 0.07f) {
						level.good_count += 1;
						level.score += 30;
						prompt = 2;
					} else {
						level.ok_count += 1;
						level.score += 10;
						prompt = 1;
					}
				}
			}
		} else if (note->type == NOTE_NORMAL) {
			if (note->time < data->song_player.time) {
				level.index += 1;
        float note_angle = note_position_to_angle(note->position);				
        if (note->color == angle_to_color(data->playdate->system->getCrankAngle(), note_angle)) {
					float progress = 1.0f - (note->beat_time - data->song_player.beat_time) / (data->song_player.bpm * SIGHTREAD_DISTANCE);
					int x, y;
					get_note_position(note->position, progress, &x, &y);
					particles_make(x, y);
  				level.score += 50;
					level.health += 3;
					level.combo += 1;
        } else {
					level.combo = 0;
					missed = 1;
					level.health -= 5;
				}
			}
		} else if (note->type == NOTE_DANGER) {
			if (note->time < data->song_player.time) {
				level.index += 1;
				float note_angle = note_position_to_angle(note->position);
				if (note->color == angle_to_color(data->playdate->system->getCrankAngle(), note_angle)) {
					level.health -= 20;
					missed = 1;
					level.combo = 0;
				} else {
					level.score += 25;
					level.health += 10;
					level.combo += 1;
				}
			}
		}
	}
	
	if (level.health > MAX_HEALTH) {
		level.health = MAX_HEALTH;
	}
	
	if (level.ok_count + level.good_count + level.perfect_count + level.miss_count > 0) {
		level.accuracy = (float)level.ok_count * 10.0f + (float)level.good_count * 30.0f + (float)level.perfect_count * 100.0f;
		level.accuracy /= (float)(level.ok_count + level.good_count + level.perfect_count + level.miss_count) * 100.0f;
	} else {
		level.accuracy = 0.0f;
	}
}

static void draw_disk(struct GameData* data) {
	float angle = data->playdate->system->getCrankAngle();

	int size = DISK_SIZE;
	PDButtons buttons;
	data->playdate->system->getButtonState(&buttons, NULL, NULL);
  
	if (buttons > 0) {
		size = DISK_SIZE_MAX;
	}
  
	int offset = (DISK_SIZE_MAX - size) / 2;
	int bounds_x = 400 / 2 - DISK_SIZE_MAX / 2;
	int bounds_y = 240 / 2 - DISK_SIZE_MAX / 2;

	data->playdate->graphics->fillEllipse(bounds_x + offset, bounds_y + offset, size, size, 0.0f, 0.0f, kColorWhite);

	// white
	data->playdate->graphics->drawEllipse(bounds_x + offset, bounds_y + offset, size, size, 2, angle + 0.0f, angle + 90.0f, kColorBlack);
	data->playdate->graphics->drawEllipse(bounds_x + offset, bounds_y + offset, size, size, 2, angle + 180.0f, angle + 270.0f, kColorBlack);
	// black
	data->playdate->graphics->fillEllipse(bounds_x + offset, bounds_y + offset, size, size, angle + 270.0f, angle + 360.0f, kColorBlack);
	data->playdate->graphics->fillEllipse(bounds_x + offset, bounds_y + offset, size, size, angle + 90.0f, angle + 180.0f, kColorBlack);
}


void song_draw(struct GameData* data) {
	const struct playdate_graphics* graphics = data->playdate->graphics;
	
	if (level.health < 1) {
		data->playdate->graphics->clear(kColorWhite);
		data->playdate->graphics->drawText("Game Over", 9, kASCIIEncoding, 200, 120);
		char buffer[100];
		snprintf(buffer, 100, "Score: %d", level.score);
		data->playdate->graphics->drawText(buffer, 99, kASCIIEncoding, 200, 150);
		if (failed_to_load_song) {
			data->playdate->graphics->drawText("Failed to open song", 99, kASCIIEncoding, 0, 10);
		}
		
		return;
	}
	
	int x, y;
	float progress;
	struct Note* note;
	for (int i = level.index; i < level.note_count; ++i) {
		note = &level.notes[i];
		// not yet
		if (note->beat_time - data->song_player.bpm * SIGHTREAD_DISTANCE > data->song_player.beat_time) {
			break;
		}
				
    // The lerped values are precalculated
		progress = 1.0f - (note->beat_time - data->song_player.beat_time) / (data->song_player.bpm * SIGHTREAD_DISTANCE);
		get_note_position(note->position, progress, &x, &y);
				
		// playdate->graphics->drawBitmap(note_bitmap, (int)x - 12, (int)y - 12, kBitmapUnflipped);

		switch (note->type) {
			case NOTE_NORMAL: {
				int note_size = 14;
				if (note->color == NOTE_WHITE) {
					graphics->drawEllipse(x - (note_size >> 1), y - (note_size >> 1), note_size, note_size, 1, 0.0f, 0.0f, kColorBlack);				
				} else {
					graphics->fillEllipse(x - (note_size >> 1), y - (note_size >> 1), note_size, note_size, 0.0f, 0.0f, kColorBlack);
				}
				break;
			}
			case NOTE_CLICK: {
				int note_size = 16;
				if (note->color == NOTE_WHITE) {
					graphics->drawEllipse(x - (note_size >> 1), y - (note_size >> 1), note_size, note_size, 1, 0.0f, 0.0f, kColorBlack);
				} else {
					graphics->fillEllipse(x - (note_size >> 1), y - (note_size >> 1), note_size, note_size, 0.0f, 0.0f, kColorBlack);
				}
				
				graphics->drawEllipse(x - (note_size >> 1) - 6, y - (note_size >> 1) - 6, note_size + 12, note_size + 12, 2, 0.0f, 0.0f, kColorBlack);
				break;
			}
			case NOTE_DANGER:
				if (note->color == NOTE_WHITE) {
					graphics->drawBitmap(data->white_x_bitmap, x - 12, y - 12, kBitmapUnflipped);
				} else {
					graphics->drawBitmap(data->black_x_bitmap, x - 12, y - 12, kBitmapUnflipped);
				}
				break;
			default:
				break;
		}		
	}
	
	draw_disk(data);
	
	particles_draw(data);
	
	// UI
	// --
	// score
	char display_buffer[50];
	if (display_score < level.score) {
		int diff = level.score - display_score;
		display_score += (diff + 1) >> 1;
	}
	snprintf(display_buffer, 50, "%d", display_score);
	data->playdate->graphics->drawText(display_buffer, 50, kASCIIEncoding, 200 - data->playdate->graphics->getTextWidth(data->font, display_buffer, 26, kASCIIEncoding, 0) / 2.0f, 200);
	
	// name
	data->playdate->graphics->drawText(level.name, 26, kASCIIEncoding, 200 - data->playdate->graphics->getTextWidth(data->font, level.name, 26, kASCIIEncoding, 0) / 2.0f, 5);
	
	// health
	if (missed) {
		missed = 0;
		health_shake_index = 0;
	}
	if (health_shake_index > 10) {
		health_shake_index = 10;
	}
	int offset = health_shake[health_shake_index];
	health_shake_index += 2;
	
	data->playdate->graphics->drawRect(200 - 50 + offset, 240 - 18 - 3, 100, 18, kColorBlack);
	data->playdate->graphics->fillRect(200 - 50 + 2 + offset, 240 - 18 - 3 + 2, 96 * level.health / 100, 14, kColorBlack);
	
	// timer
	data->playdate->graphics->drawEllipse(200 - 50 - 8 - 18 + offset, 240 - 3 - 18, 18, 18, 1, 0.0f, 0.0f, kColorBlack);
	data->playdate->graphics->fillEllipse(200 - 50 - 8 - 18 + offset, 240 - 3 - 18, 18, 18, 0.0f, 360.0f * data->song_player.percentage, kColorBlack);
	
	// accuracy
	snprintf(display_buffer, 50, "%.2f%%", level.accuracy * 100.0f);
	// data->playdate->graphics->setFont(data->accuracy_font);
	data->playdate->graphics->drawText(display_buffer, 50, kASCIIEncoding, 200 + 50 + 5 + offset, 240 - data->playdate->graphics->getFontHeight(data->font));
	// data->playdate->graphics->setFont(data->font);
		
	// prompt
	int prompt_y = (int)(prompt_ease * 10.0f);
	prompt_ease *= 0.9f;
	int height = 50;
	if (prompt_ease < 0.1f) {
		prompt_ease = 0.0f;
	} else {
		switch (prompt) {
			case 0:
				data->playdate->graphics->drawText("Miss", 4, kASCIIEncoding, 200 - data->playdate->graphics->getTextWidth(data->font, "Miss", 4, kASCIIEncoding, 0) / 2, height - prompt_y);
				break;
			case 1:
				data->playdate->graphics->drawText("OK", 2, kASCIIEncoding, 200 - data->playdate->graphics->getTextWidth(data->font, "OK", 2, kASCIIEncoding, 0) / 2, height - prompt_y);
				break;
			case 2:
				data->playdate->graphics->drawText("Good", 4, kASCIIEncoding, 200 - data->playdate->graphics->getTextWidth(data->font, "Good", 4, kASCIIEncoding, 0) / 2, height - prompt_y);
				break;
			case 3:
				data->playdate->graphics->drawText("Perfect", 7, kASCIIEncoding, 200 - data->playdate->graphics->getTextWidth(data->font, "Perfect", 7, kASCIIEncoding, 0) / 2, height - prompt_y);
				break;
			default:
				break;
		}
	}
	
	// combo
	if (level.combo > 0) {
		snprintf(display_buffer, 50, "Combo %d", level.combo);
		data->playdate->graphics->drawText(display_buffer, 50, kASCIIEncoding, 200 - data->playdate->graphics->getTextWidth(data->font, display_buffer, 50, kASCIIEncoding, 0) / 2, height - 20 - (prompt_y >> 1));
	}
}
