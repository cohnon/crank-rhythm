#include "drawing.h"
#include "game.h"
#include "scenes/song.h"

#include "pd_api/pd_api_gfx.h"

#define DISK_SIZE 64
#define DISK_SIZE_MAX 74

void draw_disk(const struct GameData* data, float pos_x, float pos_y, float angle, int big) {  
  int size = DISK_SIZE + big * (DISK_SIZE_MAX - DISK_SIZE);
  
  int offset = (DISK_SIZE_MAX - size) >> 1;
  int bounds_x = pos_x - (DISK_SIZE_MAX >> 1);
  int bounds_y = pos_y - (DISK_SIZE_MAX >> 1);

  playdate->graphics->fillEllipse(bounds_x + offset, bounds_y + offset, size, size, 0.0f, 0.0f, kColorWhite);

  // white
  playdate->graphics->drawEllipse(bounds_x + offset, bounds_y + offset, size, size, 2, angle + 0.0f, angle + 90.0f, kColorBlack);
  playdate->graphics->drawEllipse(bounds_x + offset, bounds_y + offset, size, size, 2, angle + 180.0f, angle + 270.0f, kColorBlack);
  // black
  playdate->graphics->fillEllipse(bounds_x + offset, bounds_y + offset, size, size, angle + 270.0f, angle + 360.0f, kColorBlack);
  playdate->graphics->fillEllipse(bounds_x + offset, bounds_y + offset, size, size, angle + 90.0f, angle + 180.0f, kColorBlack);
  
  playdate->graphics->drawEllipse(bounds_x + offset - 3, bounds_y + offset - 3, size + 6, size + 6, 1, 0.0f, 360.0f, kColorBlack);
}

void draw_note(const struct GameData* data, const float pos_x, const float pos_y, const uint8_t type, const uint8_t color) {
  const struct playdate_graphics* graphics = playdate->graphics;
  
  switch (type) {
    case NOTE_NORMAL: {
      int note_size = 14;
      if (color == NOTE_WHITE) {
        graphics->fillEllipse(pos_x - (note_size >> 1), pos_y - (note_size >> 1), note_size, note_size, 0.0f, 0.0f, kColorBlack);
        graphics->fillEllipse(pos_x - (note_size >> 1) + 1, pos_y - (note_size >> 1) + 1, note_size - 2, note_size - 2, 0.0f, 0.0f, kColorWhite);
      } else {
        graphics->fillEllipse(pos_x - (note_size >> 1), pos_y - (note_size >> 1), note_size, note_size, 0.0f, 0.0f, kColorBlack);
      }
      break;
    }
    case NOTE_CLICK: {
      int note_size = 16;
      if (color == NOTE_WHITE) {
        graphics->fillEllipse(pos_x - (note_size >> 1), pos_y - (note_size >> 1), note_size, note_size, 0.0f, 0.0f, kColorBlack);
        graphics->fillEllipse(pos_x - (note_size >> 1) + 1, pos_y - (note_size >> 1) + 1, note_size - 2, note_size - 2, 0.0f, 0.0f, kColorWhite);
      } else {
        graphics->fillEllipse(pos_x - (note_size >> 1), pos_y - (note_size >> 1), note_size, note_size, 0.0f, 0.0f, kColorBlack);
      }
      
      graphics->drawEllipse(pos_x - (note_size >> 1) - 6, pos_y - (note_size >> 1) - 6, note_size + 12, note_size + 12, 2, 0.0f, 0.0f, kColorBlack);
      break;
    }
    case NOTE_DANGER:
      if (color == NOTE_WHITE) {
        graphics->drawBitmap(data->white_x_bitmap, pos_x - 12, pos_y - 12, kBitmapUnflipped);
      } else {
        graphics->drawBitmap(data->black_x_bitmap, pos_x - 12, pos_y - 12, kBitmapUnflipped);
      }
      break;
    default:
      break;
  }
}
