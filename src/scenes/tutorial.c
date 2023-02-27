#include "tutorial.h"

#include "../game.h"
#include "../utils.h"

void tutorial_on_start(void* game_data, void* tutorial_data) {  
  GameData* game = (GameData*)game_data;
  
	playdate->graphics->clear(kColorWhite);
	playdate->graphics->drawText("Spin the crank to spin the disk", 50, kASCIIEncoding, 5, 25);
	playdate->graphics->drawText("Allign the disk color with the incoming notes", 50, kASCIIEncoding, 5, 55);
	playdate->graphics->drawEllipse(5, 85, 16, 16, 2, 0.0f, 0.0f, kColorBlack);
	playdate->graphics->fillEllipse(8, 88, 10, 10, 0.0f, 0.0f, kColorBlack);
	playdate->graphics->drawText("Click any button when collidoing with disk", 50, kASCIIEncoding, 25, 85);
	playdate->graphics->drawBitmap(game->black_x_bitmap, 5, 115, kBitmapUnflipped);
	playdate->graphics->drawText("Don't let X's touch the same colour", 50, kASCIIEncoding, 30, 115);
	playdate->graphics->drawText("Click any button to continue...", 50, kASCIIEncoding, 5, 145);
}

void tutorial_on_update(void* game_data, void* tutorial_data) { 
  GameData* game = (GameData*)game_data;
  
	PDButtons buttons;
	playdate->system->getButtonState(NULL, &buttons, NULL);
	if (any_button(buttons)) {
    scene_transition(game->scene_manager, game->song_list_scene);
	}
}

void tutorial_on_end(void* game_data, void* tutorial_data) {
  
}

