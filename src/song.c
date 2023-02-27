#include "song.h"
#include "drawing.h"

#include "pd_api/pd_api_gfx.h"
#include "scene.h"
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
    // playdate->graphics->drawRect(p->x - p->life / 2, p->y - p->life / 2, p->life, p->life, kColorBlack);
    // playdate->graphics->drawRect(p->x - p->life / 2 - 1, p->y - p->life / 2 - 1, p->life + 2, p->life + 2, kColorWhite);
    playdate->graphics->drawEllipse(200 - 32 - pulses[i] / 2, 120 - 32 - pulses[i] / 2, 64 + pulses[i], 64 + pulses[i], 1, 0.0f, 0.0f, kColorBlack);
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

// maybe scanf doesn't work??
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

static int load_song(GameData* data, struct SongPlayer* song_player, const char* path) {
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

  int version = 0;
  char song_name[26];
  float bpm = 0;
  float offset = 0;
  file_text_offset = parse_header(beatmap_text, &version, song_name, &bpm, &offset);  

  debug_log(song_name);
  char header_display_buffer[20];
  sprintf(header_display_buffer, "%d %d.%d %d.%d", version, (int)bpm, (int)((bpm - (int)bpm)*10.0f + 0.5f), (int)offset, (int)((offset - (int)offset)*10.0f + 0.5f));
  debug_log(header_display_buffer);
    
  char song_full_path[100];
  strcpy(song_full_path, dir_path);
  strcat(song_full_path, "/audio");
  
  int sp_load_result = sp_load(data, song_player, song_full_path, bpm, offset);
  
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
    read = parse_line(beatmap_text, &type, &color, &position, &beat_time);

    if (level.note_count == 0) {
      debug_log(beatmap_text);
    }

    beatmap_text[20] = 0;
    if (file_read == 0) {
      break;
    }

    if (level.note_count == 0) {
      char buffer[15];
      sprintf(buffer, "type: %d", type);
      debug_log(buffer);
      sprintf(buffer, "col: %d", color);
      debug_log(buffer);
      sprintf(buffer, "pos: %d", position);
      debug_log(buffer);
      sprintf(buffer, "time: %d.%d", (int)beat_time, (int)((beat_time - (int)beat_time) * 10.0f + 0.5f));
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

    
  char buffer[15];
  sprintf(buffer, "#notes: %d", level.note_count);
  debug_log(buffer);
    
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
  level.normal_hit = 0;
  level.normal_miss = 0;
  level.danger_hit = 0;
  level.danger_miss = 0;
  level.combo = 0;
  level.index = 0;
  level.health = MAX_HEALTH;
  particle_end = 0;
  particle_start = 0;
  failed_to_load_song = 0;
}

static void song_finish_callback(SoundSource* source) {
  level.health = 0;
}

void song_set_data_ptr(struct GameData* data) {
  level.notes = (struct Note*)playdate->system->realloc(NULL, sizeof(struct Note) * 1000);
}

void song_open(GameData* data, struct SongPlayer* song_player, const char* path) {
  reset();
  int load_song_result = load_song(data, song_player, path);
  if (!load_song_result) {
    level.health = 0;
    failed_to_load_song = 1;
    return;
  }
  sp_play(data, song_player);
  playdate->sound->fileplayer->setFinishCallback(song_player->current_song, song_finish_callback);
}

void song_close(struct GameData* data) {
  sp_stop(data, &data->song_player);
}

void song_update(struct GameData* data) {  
  PDButtons pressed;
  playdate->system->getButtonState(NULL, &pressed, NULL);
      
  if (level.health < 1) {
    sp_stop(data, &data->song_player);
    if (pressed > 0) {
      scene_transition(data->scene_manager, data->song_list_scene);
    }
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
    if (diff_2 < 0.11f * 0.11f && note->color == angle_to_color(playdate->system->getCrankAngle(), note_position_to_angle(note->position))) {
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
        if (note->color == angle_to_color(playdate->system->getCrankAngle(), note_angle)) {
      float progress = 1.0f - (note->beat_time - data->song_player.beat_time) / (data->song_player.bpm * SIGHTREAD_DISTANCE);
      int x, y;
      get_note_position(note->position, progress, &x, &y);
      particles_make(x, y);
      level.score += 50;
      level.health += 3;
      level.combo += 1;
      level.normal_hit += 1;
        } else {
      level.combo = 0;
      missed = 1;
      level.health -= 5;
      level.normal_miss += 1;
    }
      }
    } else if (note->type == NOTE_DANGER) {
      if (note->time < data->song_player.time) {
    level.index += 1;
    float note_angle = note_position_to_angle(note->position);
    if (note->color == angle_to_color(playdate->system->getCrankAngle(), note_angle)) {
      level.health -= 20;
      missed = 1;
      level.combo = 0;
      level.danger_miss += 1;
    } else {
      level.score += 25;
      level.health += 10;
      level.combo += 1;
      level.danger_hit += 1;
    }
      }
    }
  }
  
  if (level.health > MAX_HEALTH) {
    level.health = MAX_HEALTH;
  }
  
  if (level.ok_count + level.good_count + level.perfect_count + level.miss_count > 0) {
    level.accuracy = (float)level.ok_count * 10.0f + (float)level.good_count * 30.0f + (float)level.perfect_count * 100.0f + (float)level.danger_hit * 25.0f + (float)level.normal_hit * 50.0f;
    level.accuracy /= (float)(level.ok_count + level.good_count + level.perfect_count + level.miss_count) * 100.0f + (float)(level.danger_hit + level.danger_miss) * 25.0f + (float)(level.normal_hit + level.normal_miss) * 50.0f;
  } else {
    level.accuracy = 0.0f;
  }
}

void song_draw(struct GameData* data) {
  playdate->graphics->clear(kColorWhite);
  if (level.health < 1) {
    playdate->graphics->drawText("Game Over", 9, kASCIIEncoding, 200, 120);
    char buffer[100];
    sprintf(buffer, "Score: %d", level.score);
    playdate->graphics->drawText(buffer, 99, kASCIIEncoding, 200, 150);
    if (failed_to_load_song) {
      playdate->graphics->drawText("Failed to open song", 99, kASCIIEncoding, 0, 10);
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
    
    draw_note(data, x, y, note->type, note->color);
  }

  PDButtons pressed;
  playdate->system->getButtonState(&pressed, NULL, NULL);  
  draw_disk(data, 200, 120, playdate->system->getCrankAngle(), pressed > 0);
  
  particles_draw(data);
  
  // UI
  // --
  // score
  char display_buffer[50];
  if (display_score < level.score) {
    int diff = level.score - display_score;
    display_score += (diff + 1) >> 1;
  }
  sprintf(display_buffer, "%d", display_score);
  playdate->graphics->drawText(display_buffer, 50, kASCIIEncoding, 200 - playdate->graphics->getTextWidth(data->font, display_buffer, 26, kASCIIEncoding, 0) / 2.0f, 200);
  
  // name
  playdate->graphics->drawText(level.name, 26, kASCIIEncoding, 200 - playdate->graphics->getTextWidth(data->font, level.name, 26, kASCIIEncoding, 0) / 2.0f, 5);
  
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
  
  playdate->graphics->drawRect(200 - 50 + offset, 240 - 18 - 3, 100, 18, kColorBlack);
  playdate->graphics->fillRect(200 - 50 + 2 + offset, 240 - 18 - 3 + 2, 96 * level.health / 100, 14, kColorBlack);
  
  // timer
  playdate->graphics->drawEllipse(200 - 50 - 8 - 18 + offset, 240 - 3 - 18, 18, 18, 1, 0.0f, 0.0f, kColorBlack);
  playdate->graphics->fillEllipse(200 - 50 - 8 - 18 + offset, 240 - 3 - 18, 18, 18, 0.0f, 360.0f * data->song_player.percentage, kColorBlack);
  
  // accuracy
  float accuracy = level.accuracy * 100.0f;
  sprintf(display_buffer, "%d.%d%%", (int)accuracy, (int)((accuracy - (int)(accuracy)) * 100.0f + 0.5f));
  // playdate->graphics->setFont(data->accuracy_font);
  playdate->graphics->drawText(display_buffer, 50, kASCIIEncoding, 200 + 50 + 5 + offset, 240 - playdate->graphics->getFontHeight(data->font));
  // playdate->graphics->setFont(data->font);
    
  // prompt
  int prompt_y = (int)(prompt_ease * 10.0f);
  prompt_ease *= 0.9f;
  int height = 50;
  if (prompt_ease < 0.1f) {
    prompt_ease = 0.0f;
  } else {
    switch (prompt) {
      case 0:
    playdate->graphics->drawText("Miss", 4, kASCIIEncoding, 200 - playdate->graphics->getTextWidth(data->font, "Miss", 4, kASCIIEncoding, 0) / 2, height - prompt_y);
    break;
      case 1:
    playdate->graphics->drawText("OK", 2, kASCIIEncoding, 200 - playdate->graphics->getTextWidth(data->font, "OK", 2, kASCIIEncoding, 0) / 2, height - prompt_y);
    break;
      case 2:
    playdate->graphics->drawText("Good", 4, kASCIIEncoding, 200 - playdate->graphics->getTextWidth(data->font, "Good", 4, kASCIIEncoding, 0) / 2, height - prompt_y);
    break;
      case 3:
    playdate->graphics->drawText("Perfect", 7, kASCIIEncoding, 200 - playdate->graphics->getTextWidth(data->font, "Perfect", 7, kASCIIEncoding, 0) / 2, height - prompt_y);
    break;
      default:
    break;
    }
  }
  
  // combo
  if (level.combo > 0) {
    sprintf(display_buffer, "Combo %d", level.combo);
    playdate->graphics->drawText(display_buffer, 50, kASCIIEncoding, 200 - playdate->graphics->getTextWidth(data->font, display_buffer, 50, kASCIIEncoding, 0) / 2, height - 20 - (prompt_y >> 1));
  }
}
