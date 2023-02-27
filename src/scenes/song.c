#include "tutorial.h"

#include "../game.h"
#include "../song_player.h"
#include "../song.h"

void song_on_start(void* game_data, void* song_data) {
  
}

void song_on_update(void* game_data, void* song_data) {
  GameData* game = (GameData*)game_data;
    
	sp_update(game, &game->song_player);
	song_update(game);
	song_draw(game);
}

void song_on_end(void* game_data, void* song_data) {
  
}

