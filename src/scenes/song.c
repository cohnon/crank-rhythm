#include "song.h"

#include "../game.h"
#include "../song_player.h"
#include "../drawing.h"
#include "../beatmap.h"

#include <stdio.h>
#include <assert.h>

// TODO Make sightreading based on time not beat time
static const float SIGHTREAD_DISTANCE = 3.0f / 170.0f;

// HACK i'll get rid of these soon
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

static void menu_home_callback(void* userdata) {
  GameData* game = (GameData*)userdata;
  scene_transition(game->scene_manager, game->song_list_scene);
}

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

// static int load_song_old(GameData* game, SongData* song, struct SongPlayer* song_player, const char* path) {
//   char dir_path[100] = "songs/";
//   strcat(dir_path, path);
  
//   char beatmap_path[100];
//   strcpy(beatmap_path, dir_path);
//   strcat(beatmap_path, "/beatmap.txt");

//   char beatmap_text[50];
//   int file_text_offset;
//   SDFile* file = playdate->file->open(beatmap_path, kFileRead);
//   int file_read_result = playdate->file->read(file, beatmap_text, 50);
  
//   if (file_read_result == -1) {
//     playdate->system->logToConsole("Failed to read %s", beatmap_path);
//     return 0;
//   }

//   int version = 0;
//   char song_name[26];
//   float bpm = 0;
//   float offset = 0;
//   file_text_offset = parse_header(beatmap_text, &version, song_name, &bpm, &offset);  

//   char header_display_buffer[20];
//   sprintf(header_display_buffer, "%d %d.%d %d.%d", version, (int)bpm, (int)((bpm - (int)bpm)*10.0f + 0.5f), (int)offset, (int)((offset - (int)offset)*10.0f + 0.5f));
    
//   char song_full_path[100];
//   strcpy(song_full_path, dir_path);
//   strcat(song_full_path, "/audio");
  
//   int sp_load_result = sp_load(game, song_player, song_full_path, bpm, offset);
  
//   if (!sp_load_result) {
//     playdate->system->logToConsole("Failed to load audio file");
//     return 0;
//   }
  
//   strcpy(song->name, song_name);
//   song->note_count = 0;
  
//   // bpm of brain power is 170 (2.371 offset)
//   float last_beat_time = -999.0f; // make sure each note is larger than the last
  
//   int type;
//   int color;
//   int position;
//   float beat_time;
//   int read;
//   int file_read;
//   while (song->note_count < 1000) {
//     playdate->file->seek(file, file_text_offset, SEEK_SET);
//     file_read = playdate->file->read(file, beatmap_text, 50);
//     read = parse_line(beatmap_text, &type, &color, &position, &beat_time);

//     if (file_read == 0) {
//       break;
//     }

//     song->notes[song->note_count].type = type;
//     song->notes[song->note_count].color = color;
//     song->notes[song->note_count].position = position;
//     song->notes[song->note_count].beat_time = beat_time;
//     song->notes[song->note_count].time = beat_time * 60 / bpm;

//     assert(beat_time > last_beat_time);

//     file_text_offset += read;
//     song->note_count += 1;
//   }

    
//   char buffer[15];
//   sprintf(buffer, "#notes: %d", song->note_count);
    
//   return 1;
// }

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

// HACK how can i not use a global here? the callback has no userdata >:(
static int finished;
static void reset(SongData* song) {  
  song->score = 0;
  display_score = 0;
  song->miss_count = 0;
  song->ok_count = 0;
  song->good_count = 0;
  song->perfect_count = 0;
  song->normal_hit = 0;
  song->normal_miss = 0;
  song->danger_hit = 0;
  song->danger_miss = 0;
  song->combo = 0;
  song->index = 0;
  song->health = MAX_HEALTH;
  finished = 0;
  particle_end = 0;
  particle_start = 0;
  failed_to_load_song = 0;
}

static void song_finish_callback(SoundSource* source) {
  finished = 1;
}

// HACK too lazy to put this in game.h
PDMenuItem* quit_menu_item;
void song_on_start(void* game_data, void* song_data) {
  GameData* game = (GameData*)game_data;
  SongData* song = (SongData*)song_data;
  
  quit_menu_item = playdate->system->addMenuItem("Quit Song", menu_home_callback, game_data);

  reset(song);

  // int load_song_result = load_song(game, song, &game->song_player, game->song_path);
  int load_header_result = beatmap_load_header(&song->header, game->song_path);
  int load_song_result = beatmap_load(&song->beatmap, &song->header);
  if (!load_song_result) {
    song->health = 0;
    failed_to_load_song = 1;
    return;
  }
  sp_play(game, &game->song_player);
  playdate->sound->fileplayer->setFinishCallback(game->song_player.current_song, song_finish_callback);
}

void song_on_update(void* game_data, void* song_data) {
  GameData* game = (GameData*)game_data;
  SongData* song = (SongData*)song_data;
    
  sp_update(game, &game->song_player);
  
  PDButtons pressed;
  playdate->system->getButtonState(NULL, &pressed, NULL);
      
  if (song->health < 1 || finished) {
    sp_stop(game, &game->song_player);
    if (pressed > 0) {
      scene_transition(game->scene_manager, game->song_list_scene);
    }
  }
  
  particles_update();
  
  struct Note* note;
  for (int i = song->index; i < song->note_count; ++i) {
    note = &song->beatmap.notes[i];
    if (note->beat_time - game->song_player.bpm * SIGHTREAD_DISTANCE > game->song_player.beat_time) {
      break;
    }
    
    if (note->type == NOTE_CLICK) {
      // too late
      if (note->time + 0.11f < game->song_player.time) {
        song->miss_count += 1;
        song->index += 1;
        song->health -= 15;
        prompt_show = 1;
        missed = 1;
        prompt_ease = 1.0f;
        prompt = 0;
        song->combo = 0;
      }
  
      // playdate->graphics->setStencilImage(clear_bitmap, 1);
      if (pressed > 0 && song->index == i) {
        float diff = note->time - game->song_player.time;
        float diff_2 = diff * diff;
        
        // inside clickable range
        if (diff_2 < 0.11f * 0.11f && note->color == angle_to_color(playdate->system->getCrankAngle(), note_position_to_angle(note->position))) {
          song->index += 1;
          song->health += 5;
          prompt_show = 1;
          prompt_ease = 1.0f;
          song->combo += 1;
          float progress = 1.0f - (note->beat_time - game->song_player.beat_time) / (game->song_player.bpm * SIGHTREAD_DISTANCE);
          int x, y;
          get_note_position(note->position, progress, &x, &y);
          particles_make(x, y);
          if (diff_2 < 0.045f * 0.045f) {
            song->perfect_count += 1;
            song->score += 100;
            prompt = 3;
          } else if (diff_2 < 0.07f * 0.07f) {
            song->good_count += 1;
            song->score += 30;
            prompt = 2;
          } else {
            song->ok_count += 1;
            song->score += 10;
            prompt = 1;
          }
        }
      }
    } else if (note->type == NOTE_NORMAL) {
      if (note->time < game->song_player.time) {
        song->index += 1;
        float note_angle = note_position_to_angle(note->position);    
        if (note->color == angle_to_color(playdate->system->getCrankAngle(), note_angle)) {
          float progress = 1.0f - (note->beat_time - game->song_player.beat_time) / (game->song_player.bpm * SIGHTREAD_DISTANCE);
          int x, y;
          get_note_position(note->position, progress, &x, &y);
          particles_make(x, y);
          song->score += 50;
          song->health += 3;
          song->combo += 1;
          song->normal_hit += 1;
        } else {
          song->combo = 0;
          missed = 1;
          song->health -= 5;
          song->normal_miss += 1;
        }
      }
    } else if (note->type == NOTE_DANGER) {
      if (note->time < game->song_player.time) {
        song->index += 1;
        float note_angle = note_position_to_angle(note->position);
        if (note->color == angle_to_color(playdate->system->getCrankAngle(), note_angle)) {
          song->health -= 20;
          missed = 1;
          song->combo = 0;
          song->danger_miss += 1;
        } else {
          song->score += 25;
          song->health += 10;
          song->combo += 1;
          song->danger_hit += 1;
        }
      }
    }
  }
  
  if (song->health > MAX_HEALTH) {
    song->health = MAX_HEALTH;
  }
  
  if (song->ok_count + song->good_count + song->perfect_count + song->miss_count > 0) {
    song->accuracy = (float)song->ok_count * 10.0f + (float)song->good_count * 30.0f + (float)song->perfect_count * 100.0f + (float)song->danger_hit * 25.0f + (float)song->normal_hit * 50.0f;
    song->accuracy /= (float)(song->ok_count + song->good_count + song->perfect_count + song->miss_count) * 100.0f + (float)(song->danger_hit + song->danger_miss) * 25.0f + (float)(song->normal_hit + song->normal_miss) * 50.0f;
  } else {
    song->accuracy = 0.0f;
  }

  // Draw
  playdate->graphics->clear(kColorWhite);
  if (song->health < 1 || finished) {
    playdate->graphics->drawText("Game Over", 9, kASCIIEncoding, 200, 120);
    char buffer[100];
    sprintf(buffer, "Score: %d", song->score);
    playdate->graphics->drawText(buffer, 99, kASCIIEncoding, 200, 150);
    if (failed_to_load_song) {
      playdate->graphics->drawText("Failed to open song", 99, kASCIIEncoding, 0, 10);
    }
    
    return;
  }
  
  int x, y;
  float progress;
  for (int i = song->index; i < song->note_count; ++i) {
    note = &song->beatmap.notes[i];
    // not yet
    if (note->beat_time - game->song_player.bpm * SIGHTREAD_DISTANCE > game->song_player.beat_time) {
      break;
    }
    
    // The lerped values are precalculated
    progress = 1.0f - (note->beat_time - game->song_player.beat_time) / (game->song_player.bpm * SIGHTREAD_DISTANCE);
    get_note_position(note->position, progress, &x, &y);
    
    draw_note(game, x, y, note->type, note->color);
  }

  playdate->system->getButtonState(&pressed, NULL, NULL);  
  draw_disk(game, 200, 120, playdate->system->getCrankAngle(), pressed > 0);
  
  particles_draw(game);
  
  // UI
  // --
  // score
  char display_buffer[50];
  if (display_score < song->score) {
    int diff = song->score - display_score;
    display_score += (diff + 1) >> 1;
  }
  sprintf(display_buffer, "%d", display_score);
  playdate->graphics->drawText(display_buffer, 50, kASCIIEncoding, 200 - playdate->graphics->getTextWidth(game->font, display_buffer, 26, kASCIIEncoding, 0) / 2.0f, 200);
  
  // name
  playdate->graphics->drawText(song->beatmap.name, 26, kASCIIEncoding, 200 - playdate->graphics->getTextWidth(game->font, song->beatmap.name, 26, kASCIIEncoding, 0) / 2.0f, 5);
  
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
  playdate->graphics->fillRect(200 - 50 + 2 + offset, 240 - 18 - 3 + 2, 96 * song->health / 100, 14, kColorBlack);
  
  // timer
  playdate->graphics->drawEllipse(200 - 50 - 8 - 18 + offset, 240 - 3 - 18, 18, 18, 1, 0.0f, 0.0f, kColorBlack);
  playdate->graphics->fillEllipse(200 - 50 - 8 - 18 + offset, 240 - 3 - 18, 18, 18, 0.0f, 360.0f * game->song_player.percentage, kColorBlack);
  
  // accuracy
  float accuracy = song->accuracy * 100.0f;
  sprintf(display_buffer, "%d.%d%%", (int)accuracy, (int)((accuracy - (int)(accuracy)) * 100.0f + 0.5f));
  // playdate->graphics->setFont(game->accuracy_font);
  playdate->graphics->drawText(display_buffer, 50, kASCIIEncoding, 200 + 50 + 5 + offset, 240 - playdate->graphics->getFontHeight(game->font));
  // playdate->graphics->setFont(game->font);
    
  // prompt
  int prompt_y = (int)(prompt_ease * 10.0f);
  prompt_ease *= 0.9f;
  int height = 50;
  if (prompt_ease < 0.1f) {
    prompt_ease = 0.0f;
  } else {
    switch (prompt) {
      case 0:
    playdate->graphics->drawText("Miss", 4, kASCIIEncoding, 200 - playdate->graphics->getTextWidth(game->font, "Miss", 4, kASCIIEncoding, 0) / 2, height - prompt_y);
    break;
      case 1:
    playdate->graphics->drawText("OK", 2, kASCIIEncoding, 200 - playdate->graphics->getTextWidth(game->font, "OK", 2, kASCIIEncoding, 0) / 2, height - prompt_y);
    break;
      case 2:
    playdate->graphics->drawText("Good", 4, kASCIIEncoding, 200 - playdate->graphics->getTextWidth(game->font, "Good", 4, kASCIIEncoding, 0) / 2, height - prompt_y);
    break;
      case 3:
    playdate->graphics->drawText("Perfect", 7, kASCIIEncoding, 200 - playdate->graphics->getTextWidth(game->font, "Perfect", 7, kASCIIEncoding, 0) / 2, height - prompt_y);
    break;
      default:
    break;
    }
  }
  
  // combo
  if (song->combo > 0) {
    sprintf(display_buffer, "Combo %d", song->combo);
    playdate->graphics->drawText(display_buffer, 50, kASCIIEncoding, 200 - playdate->graphics->getTextWidth(game->font, display_buffer, 50, kASCIIEncoding, 0) / 2, height - 20 - (prompt_y >> 1));
  }
}

void song_on_end(void* game_data, void* song_data) {
  GameData* game = (GameData*)game_data;

  playdate->system->removeMenuItem(quit_menu_item);

  sp_stop(game, &game->song_player);
}

