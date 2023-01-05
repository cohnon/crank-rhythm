#include "game.h"

#include "song_player.h"
#include <stdio.h>

#define DISK_SIZE 64
#define DISK_SIZE_MAX 74

#define NOTE_NORMAL 0
#define NOTE_CLICK 1
#define NOTE_HELD 2
#define NOTE_DANGER 3
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

struct Note {
  uint8_t type;
  uint8_t position;
	uint8_t color;
  float beat_time;
	float time;
  float end; // for held notes, spin notes, etc
};

struct Song {
	char name[26];
	int index;
	int miss_count;
	int ok_count;
	int good_count;
	int perfect_count;
	int note_count;
	struct Note notes[1000];
};

struct Song level;
struct SongPlayer song_player;

PlaydateAPI* pd;

const char* fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";
LCDFont* font = NULL;

LCDBitmap* clear_bitmap = NULL;
LCDBitmap* grey_bitmap = NULL;
LCDBitmap* stripe_bitmap = NULL;
LCDBitmap* x_bitmap = NULL;
LCDBitmap* note_bitmap = NULL;

int score = 0;

static void draw_disk();
static void load_song(const char* path);
static void draw_notes();

void game_setup_pd(PlaydateAPI* playdate) {
  pd = playdate;
}

void game_init() {
	sp_init(&song_player, pd);
		
	const char* err;
	font = pd->graphics->loadFont(fontpath, &err);
	pd->graphics->setFont(font);

	grey_bitmap = pd->graphics->loadBitmap("grey.png", &err);
	stripe_bitmap = pd->graphics->loadBitmap("stripe.png", &err);
	clear_bitmap = pd->graphics->newBitmap(32, 32, kColorWhite);
	x_bitmap = pd->graphics->loadBitmap("x.png", &err);
	note_bitmap = pd->graphics->loadBitmap("note.png", &err);
	
  load_song("song.4t");
	pd->display->setRefreshRate(0);
}

void game_update() {
	pd->graphics->clear(kColorWhite);
  
  PDButtons pressed;
  pd->system->getButtonState(NULL, &pressed, NULL);
  
	if (pressed & kButtonA) {
		sp_play(&song_player);
	}
	
	sp_update(&song_player);
	draw_notes();	
  draw_disk();
	
	// ui
	// name
	pd->graphics->drawText(level.name, 26, kASCIIEncoding, (400 / 2) - pd->graphics->getTextWidth(font, level.name, 26, kASCIIEncoding, 0) / 2, 0);

	// HACK DEBUG
	char score_display[100];
	int title_width = pd->graphics->getTextWidth(font, level.name, 26, kASCIIEncoding, 0);
	float completion = song_player.time / song_player.length;
	float max = (200 - title_width / 2.0f);
	if (completion < 0.5f) {
		max *= completion * 2.0f;
	}
	pd->graphics->drawLine(0, 10, max, 10, 5, kColorBlack);
	if (completion > 0.5f) {		
		pd->graphics->drawLine(
			(200 + title_width / 2), 10,
			(200 + title_width / 2) + (200 - title_width / 2) * (((completion - 0.5f) * 2.0f)), 10,
			5, kColorBlack
		);
	}
	sprintf(score_display, "%d %d %d %.2f", score, level.miss_count, level.ok_count, song_player.time);
	pd->graphics->drawText(score_display, 20, kASCIIEncoding, (400 / 2) - pd->graphics->getTextWidth(font, score_display, 20, kASCIIEncoding, 0) / 2, 240 - pd->graphics->getFontHeight(font));

	pd->system->drawFPS(0,20);
}

static void draw_disk() {  
	float angle = pd->system->getCrankAngle();
	
	int size = DISK_SIZE;
	PDButtons buttons;
	pd->system->getButtonState(&buttons, NULL, NULL);
  
	if (buttons > 0) {
		size = DISK_SIZE_MAX;
	}
  
	int offset = (DISK_SIZE_MAX - size) / 2;
	int bounds_x = 400 / 2 - DISK_SIZE_MAX / 2;
	int bounds_y = 240 / 2 - DISK_SIZE_MAX / 2;
	  
	// clear
	pd->graphics->fillEllipse(bounds_x + offset, bounds_y + offset, size, size, 0.0f, 0.0f, kColorWhite);
	// stripe
	pd->graphics->setStencilImage(grey_bitmap, 1);
	pd->graphics->fillEllipse(bounds_x + offset, bounds_y + offset, size, size, angle + 0.0f, angle + 90.0f, kColorBlack);
	// grey
	pd->graphics->setStencilImage(stripe_bitmap, 1);
	pd->graphics->fillEllipse(bounds_x + offset, bounds_y + offset, size, size, angle + 180.0f, angle + 270.0f, kColorBlack);
	// white
	pd->graphics->setStencilImage(clear_bitmap, 1);
	pd->graphics->drawEllipse(bounds_x + offset, bounds_y + offset, size, size, 2, angle + 90.0f, angle + 180.0f, kColorBlack);
	// black
	pd->graphics->fillEllipse(bounds_x + offset, bounds_y + offset, size, size, angle + 270.0f, angle + 360.0f, kColorBlack);
}

static float lerp(float x1, float x2, float t) {
	// return x1 + t * (x2 - x1);
	return x1 * (1 - t) + x2 * t;
}

static void draw_notes() {
  PDButtons pressed;
  pd->system->getButtonState(NULL, &pressed, NULL);
	
	float x, y;
	float progress;
	struct Note* note;
	for (int i = level.index; i < level.note_count; ++i) {
		note = &level.notes[i];
		// not yet
		if (note->beat_time - 2.0f > song_player.beat_time) {
			break;
		}
						
		if (note->type == NOTE_CLICK) {
			// too late
			if (note->time + 0.11f < song_player.time) {
				pd->system->logToConsole("%f %f", note->time, song_player.time);
				level.miss_count += 1;
				level.index += 1;
			}
	
			// pd->graphics->setStencilImage(clear_bitmap, 1);
			if (pressed > 0) {
				float diff = note->time - song_player.time;
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
			if (note->time < song_player.time) {
				score += 50;
				level.index += 1;
			}
		}
		
		// pd->graphics->setStencilImage(stripe_bitmap, 1);
		progress = 1.0f - (note->beat_time - song_player.beat_time) / 2.0f;
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
		
		// pd->graphics->drawBitmap(note_bitmap, (int)x - 12, (int)y - 12, kBitmapUnflipped);

		int note_size = 16;
		switch (note->type) {
			case NOTE_NORMAL:
				pd->graphics->fillEllipse(x - (note_size >> 1), y - (note_size >> 1), note_size, note_size, 0.0f, 0.0f, kColorBlack);
				break;
			case NOTE_CLICK:
				pd->graphics->fillEllipse(x - (note_size >> 1), y - (note_size >> 1), note_size, note_size, 0.0f, 0.0f, kColorBlack);
				pd->graphics->drawEllipse(x - (note_size >> 1) - 4, y - (note_size >> 1) - 4, note_size + 8, note_size + 8, 2, 0.0f, 0.0f, kColorBlack);
				break;
		}
	}
}

static void load_song(const char* path) {
	char file_text[500];
	int file_text_offset;
	SDFile* file = pd->file->open(path, kFileRead);
	pd->file->read(file, file_text, 500);
	
	if (file == NULL) {
		pd->system->logToConsole("FAILED");
	}
	
	int version;
	char song_path[50];
	char song_name[26];
	float bpm;
	float offset;
	sscanf_s(file_text, "%d%s%s%f%f%n", &version, song_name, 25, song_path, 49, &bpm, &offset, &file_text_offset);
	
	FilePlayer* song = pd->sound->fileplayer->newPlayer();
	pd->sound->fileplayer->loadIntoPlayer(song, song_path);

  sp_load(&song_player, song, bpm, offset);

	strcpy_s(level.name, 26, song_name);
	level.note_count = 0;
	
	// bpm of brain power is 170 (2.371 offset)
	int type;
	int position;
	int color;
	float beat_time;
	int read;
	while (sscanf_s(file_text + file_text_offset, "%d%d%d%f%n", &type, &position, &color, &beat_time, &read) != 0) {		
		level.notes[level.note_count].type = type;
		level.notes[level.note_count].position = position;
		level.notes[level.note_count].color = color;
		level.notes[level.note_count].beat_time = beat_time;
		level.notes[level.note_count].time = level.notes[level.note_count].beat_time * 60 / bpm;
						
		file_text_offset += read;
		level.note_count += 1;
	}
}
