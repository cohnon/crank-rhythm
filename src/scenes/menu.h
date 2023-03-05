#ifndef MENU_H
#define MENU_H

#include "../particles.h"
#include <pd_api.h>
#include <stdint.h>

#define MENU_NOTES_LENGTH 15

typedef struct MenuNote {
  float x;
  uint8_t y;
  uint8_t type;
  uint8_t color;
  uint8_t sin_offset;
  emitter_id emitter;
} MenuNote;

typedef struct MenuData {
  float progress;
  PDSynth* progress_synth;
  MenuNote bg_notes[MENU_NOTES_LENGTH];
  ParticleSystem* particles;
  // float waveform[9];
  // int waveform_counter;
  // LCDBitmap* waveform_bitmap;
} MenuData;

void menu_on_start(void* game_data, void* menu_data);
void menu_on_update(void* game_data, void* menu_data);
void menu_on_end(void* game_data, void* menu_data);

#endif