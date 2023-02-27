#include "menu.h"

#include "../game.h"
#include "../drawing.h"
#include "../utils.h"
#include <stdint.h>


// precomputed values to move the background notes
#define RNG_Y_LENGTH    12
#define RNG_TYPE_LENGTH 5
#define SIN_VALS_LENGTH 11

static const uint16_t RNG_Y[RNG_Y_LENGTH] = {50, 70, 200, 210, 140, 190, 100, 8, 120, 30, 180, 120};
static const uint8_t RNG_TYPE[RNG_TYPE_LENGTH] = {0, 1, 0, 2, 0};
static const int8_t SIN_VALS[SIN_VALS_LENGTH] = {0, 2, 4, 4, 2, 0, -3, -5, -5, -3, 0};

void menu_on_start(void* game_data, void* menu_data) {
  GameData* game = (GameData*)game_data;
  MenuData* menu = (MenuData*)menu_data;
  
  playdate->sound->fileplayer->loadIntoPlayer(game->fileplayer, "audio/menu");
  playdate->sound->fileplayer->play(game->fileplayer, 0);

  for (int i = 0; i < 15; ++i) {
    MenuNote* note = &menu->bg_notes[i];
    note->y = (i * 2) % 12;
    note->type = (i * 3) % 5;
    note->color = (i * 7) % 2;
    note->x = i * 50;
    note->sin_offset = i * 3;
  }
}

void menu_on_update(void* game_data, void* menu_data) {
  GameData* game = (GameData*)game_data;
  MenuData* menu = (MenuData*)menu_data;
  
  playdate->graphics->clear(kColorWhite);
  for (int i = 0; i < 15; ++i) {
    MenuNote* note = &menu->bg_notes[i];
    note->x = (note->x + ((float)RNG_Y[note->y] / 80.0f) + 1.5f);
    if (note->x > 500) {
      note->x = 0;
      note->y = (note->y + 1) % 12;
      note->type = (note->type + 1) % 5;
      note->color = (note->color + 1) % 2;
    }
    draw_note(game, 420 - (int)note->x, RNG_Y[note->y] + SIN_VALS[((game->frame + note->sin_offset) / 10) % 11], RNG_TYPE[note->type], note->color);
  }  
  
  playdate->graphics->setDrawMode(kDrawModeNXOR);
  playdate->graphics->drawBitmap(game->title_bitmaps[(game->frame / 10) % 2], 200 - (196 / 2), 50, kBitmapUnflipped);
  playdate->graphics->setDrawMode(kDrawModeCopy);
  int left = 200 - (playdate->graphics->getTextWidth(game->basic_font, "CRANK TO START", 16, kASCIIEncoding, 0) / 2);
  int top = 230 - playdate->graphics->getFontHeight(game->basic_font) - ((game->frame / 20) % 2) * 2;
  playdate->graphics->setFont(game->basic_font);
  playdate->graphics->drawText("CRANK TO START", 16, kASCIIEncoding, left, top);
  playdate->graphics->setFont(game->font);
  
  PDButtons buttons;
  playdate->system->getButtonState(NULL, &buttons, NULL);
  
  if (any_button(buttons)) {
    playdate->sound->sampleplayer->play(game->sound_effect, 1, 1.0);
    scene_transition(game->scene_manager, game->song_list_scene);
  }
}

void menu_on_end(void* game_data, void* menu_data) {}
