#include "song_player.h"

void sp_init(struct SongPlayer* player, PlaydateAPI* playdate) {
  player->pd = playdate;
}

int sp_load(struct SongPlayer* player, const char* path, const float bpm, const float offset) {
  player->current_song = player->pd->sound->fileplayer->newPlayer();
	player->pd->sound->fileplayer->setBufferLength(player->current_song, 5.0f);
	int fileplayer_result = player->pd->sound->fileplayer->loadIntoPlayer(player->current_song, path);

  if (fileplayer_result == 0) {
    player->pd->system->logToConsole("Failed to load audio:  %s", path);
    
    player->pd->sound->fileplayer->freePlayer(player->current_song);
    player->current_song = NULL;
    
    return 0;
  }
  
  player->bpm = bpm;
  player->sec_per_beat = 60 / bpm;
  player->offset = offset;
  player->length = player->pd->sound->fileplayer->getLength(player->current_song) - player->offset;
  player->playing = 0;
  
  return 1;
}

void sp_play(struct SongPlayer* player) {
  if (player->current_song == NULL) {
    return;
  }
  
  player->time = -3;
  player->start_tick = (float)player->pd->system->getCurrentTimeMilliseconds(); 
}

void sp_stop(struct SongPlayer* player) {
  if (player->current_song == NULL) {
    return;
  }
  player->pd->system->logToConsole("STOPPING");
  player->pd->sound->fileplayer->stop(player->current_song);
  player->pd->sound->fileplayer->freePlayer(player->current_song);
  player->current_song = NULL;
}

void sp_update(struct SongPlayer* player) {
  if (player->current_song == NULL) {
    return;
  }
  
  if (!player->playing) {
    player->time = -3.0f - player->offset + 0.001f * ((float)player->pd->system->getCurrentTimeMilliseconds() - player->start_tick);
    player->beat_time = player->time / player->sec_per_beat;
 
    if (player->time + player->offset > 0) {
      player->pd->sound->fileplayer->play(player->current_song, 1);
      player->song_start = player->pd->sound->getCurrentTime();
      player->playing = 1;
    }
            
    return;
  }
  
  player->time = (float)(player->pd->sound->getCurrentTime() - player->song_start) / 44100.0f - player->offset;
  player->beat_time = player->time / player->sec_per_beat;
  player->percentage = player->time / player->length;
}
