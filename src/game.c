#include "game.h"

#include "song_player.h"
#include <stdio.h>

#define DISK_SIZE 64
#define DISK_SIZE_MAX 74

#define NOTE_NORMAL 0
#define NOTE_HELD 1
#define NOTE_MOVE 2
#define NOTE_DANGER 3
#define NOTE_SLOWDOWN 4

#define NOTE_POSITION_TOP_LEFT 0
#define NOTE_POSITION_TOP_RIGHT 1
#define NOTE_POSITION_BOTTOM_LEFT 2
#define NOTE_POSITION_BOTTOM_RIGHT 3

struct Note {
  uint8_t type;
  uint8_t position;
  float beat_time;
  float end; // for held notes
  float speed;
};

struct Song {
	char name[26];
	int index;
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

int score = 0;

static void draw_disk();
static void load_song(const char* path);

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
	
	// score
	sp_update(&song_player);
	char score_display[20];

	if (level.index < 4 && song_player.beat_time > level.notes[level.index].beat_time) {
	 level.index += 1;
	}
	
	float x, y;
	float progress;
	struct Note note;
	for (int i = 0; i < 1000; ++i) {
		note = level.notes[i];
		if (note.beat_time > song_player.beat_time + 2 || note.beat_time < song_player.beat_time) {
			continue;
		}
		
		// pd->graphics->setStencilImage(stripe_bitmap, 1);
		progress = 1.0f - (note.beat_time - song_player.beat_time) / 2.0f;
		x = 400.0f * (1 - progress) + 223.0f * progress;
		y = 97 * progress;
		pd->graphics->drawBitmap(x_bitmap, (int)x - 11, (int)y - 11, kBitmapUnflipped);
		// pd->graphics->setStencilImage(clear_bitmap, 1);	
	}
	
  draw_disk();
	
	// Name
	pd->graphics->drawText(level.name, 26, kASCIIEncoding, (400 / 2) - pd->graphics->getTextWidth(font, level.name, 26, kASCIIEncoding, 0) / 2, 0);

	// HACK DEBUG		
	if (song_player.beat_time > 0) {
		sprintf(score_display, "%d", (int)song_player.beat_time);
		pd->graphics->drawText(score_display, 20, kASCIIEncoding, (400 / 2) - pd->graphics->getTextWidth(font, score_display, 20, kASCIIEncoding, 0) / 2, 240 - pd->graphics->getFontHeight(font));
	}

	pd->system->drawFPS(0,0);
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

	strcpy_s(level.name, 26, song_name);
	  
	// int type;
	// int position;
	// float beat_time;
	// int read;
	// int i = 0;
	// while (sscanf_s(file_text + file_text_offset, "%d%d%f%n", &type, &position, &beat_time, &read) != 0) {		
	// 	level.notes[i].type = type;
	// 	level.notes[i].position = position;
	// 	level.notes[i].beat_time = beat_time;
		
	// 	pd->system->logToConsole("(read %d) %d %d %f", read, type, position, beat_time);
		
	// 	file_text_offset += read;
	// 	i += 1;
	// }
	for (int i = 0; i < 1000; ++i) {
		level.notes[i].beat_time = (float)i;
	}

  sp_load(&song_player, song, bpm, offset);
}
