#ifndef DRAWING_H
#define DRAWING_H

#include "game.h"

void draw_disk(const struct GameData* data, const float pos_x, const float pos_y, const float angle, const int big);
void draw_note(const struct GameData* data, const float pos_x, const float pos_y, const uint8_t type, const uint8_t color);

#endif