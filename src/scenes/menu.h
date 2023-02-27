#ifndef MENU_H
#define MENU_H

#include <stdint.h>

#define MENU_NOTES_LENGTH 15

typedef struct MenuNote {
  float x;
  uint8_t y;
  uint8_t type;
  uint8_t color;
  uint8_t sin_offset;
} MenuNote;

typedef struct MenuData {
  MenuNote bg_notes[MENU_NOTES_LENGTH];
} MenuData;

void menu_on_start(void* game_data, void* menu_data);
void menu_on_update(void* game_data, void* menu_data);
void menu_on_end(void* game_data, void* menu_data);

#endif