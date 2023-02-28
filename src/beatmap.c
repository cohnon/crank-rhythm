#include "beatmap.h"

#include "game.h"
#include "pd_api/pd_api_file.h"
#include <string.h>
#include <assert.h>

static int parse_int(const char* string, int* val) {
  int index = 0;
  *val = 0;
  while (string[index] != ' ' && string[index] != '\n' && string[index] != 0) {
    *val *= 10;
    *val += string[index] - '0';

    index += 1;
  }

  return index + 1;
}

static float parse_float(const char* string, float* val) {
  int index = 0;
  int decimal_index = -1;
  *val = 0.0f;
  while (string[index] != ' ' && string[index] != '\n' && string[index] != 0) {
    if (decimal_index == -1 && string[index] == '.') {
      decimal_index = index;
      index += 1;
      continue;
    }
    
    if (decimal_index != -1) {
      float decimal = powf(10.0f, (float)(index - decimal_index));
      *val += (float)(string[index] - '0') / decimal;
    } else {
      *val *= 10.0f;
      *val += (float)(string[index] - '0');
    }
    
    index += 1;
  }
  return index + 1;
}

static int parse_string(const char* string, char* val) {
  int index = 0;
  while (string[index] != '\n' && string[index] != 0) {
    val[index] = string[index];

    index += 1;
  }
  
  val[index] = 0;
  
  return index + 1;
}

static int parse_header(const char* input, int* version, char* name, float* bpm, float* offset) {
  int length = 0;
  length += parse_int(input, version);
  length += parse_string(input + length, name);
  length += parse_float(input + length, bpm);  
  length += parse_float(input + length, offset);
  
  return length;
}

static int parse_line(const char* line, int* type, int* color, int* position, float* beat_time) {
  int length = 0;
  length += parse_int(line + length, type);
  length += parse_int(line + length, color);
  length += parse_int(line + length, position);
  length += parse_float(line + length, beat_time);

  return length;
}

int beatmap_load_header(BeatmapHeader* header, const char* path) {
  strcpy(header->path, "songs/");
  
  strcat(header->path, path);
  header->path_length = strlen(header->path);

  strcat(header->path, "/beatmap.txt");
  
  char beatmap_text[50];
  
  SDFile* file = playdate->file->open(header->path, kFileRead);
  
  if (file == NULL) {
    return BEATMAP_LOAD_FAIL;
  }

  int file_read_result = playdate->file->read(file, beatmap_text, 50);
  
  if (file_read_result == -1) {
    return BEATMAP_LOAD_FAIL;
  }
  
  playdate->file->close(file);

  int version; // useless rn but it could be useful
  header->length = parse_header(beatmap_text, &version, header->name, &header->bpm, &header->offset);
  
  // remove /beatmap.txt from path
  header->path[header->path_length] = 0;
  
  return BEATMAP_LOAD_SUCCESS;
}

int beatmap_load(Beatmap* beatmap, BeatmapHeader* header) {
  strcpy(beatmap->name, header->name);
  beatmap->bpm = header->bpm;
  beatmap->offset = header->offset;

  
  // int sp_load_result = sp_load(game, song_player, song_full_path, bpm, offset);
    
    
  char beatmap_buffer[50];
  int file_read_total = header->length;
  strcat(header->path, "/beatmap.txt");
  SDFile* file = playdate->file->open(header->path, kFileRead);

  int type;
  int color;
  int position;
  float beat_time;
  int read;

  int file_read;
  beatmap->notes_length = 0;
  float last_beat_time = -999.0f; // make sure each note is larger than the last

  while (beatmap->notes_length< 1000) {
    playdate->file->seek(file, file_read_total, SEEK_SET);
    file_read = playdate->file->read(file, beatmap_buffer, 50);
    read = parse_line(beatmap_buffer, &type, &color, &position, &beat_time);

    if (file_read == 0) {
      break;
    }

    beatmap->notes[beatmap->notes_length].type = type;
    beatmap->notes[beatmap->notes_length].color = color;
    beatmap->notes[beatmap->notes_length].position = position;
    beatmap->notes[beatmap->notes_length].beat_time = beat_time;
    beatmap->notes[beatmap->notes_length].time = beat_time * 60 / header->bpm;

    assert(beat_time > last_beat_time);

    file_read_total += read;
    beatmap->notes_length += 1;
  }
  
  playdate->file->close(file);

  return BEATMAP_LOAD_SUCCESS;
}
