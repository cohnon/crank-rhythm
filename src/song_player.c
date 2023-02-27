#include "song_player.h"

#include "game.h"

int sp_load(GameData* data, struct SongPlayer* player, const char* path, const float bpm, const float offset) {
  const struct playdate_sound* sound = playdate->sound;
  player->current_song = sound->fileplayer->newPlayer();
  sound->fileplayer->setBufferLength(player->current_song, 5.0f);
  int fileplayer_result = sound->fileplayer->loadIntoPlayer(player->current_song, path);

  if (fileplayer_result == 0) {
    playdate->system->logToConsole("Failed to load audio:  %s", path);
    
    playdate->sound->fileplayer->freePlayer(player->current_song);
    player->current_song = NULL;
    
    return 0;
  }
    
  player->bpm = bpm;
  player->sec_per_beat = 60 / bpm;
  player->offset = offset;
  player->length = playdate->sound->fileplayer->getLength(player->current_song) - player->offset;
  player->playing = 0;
  
  return 1;
}

void sp_play(GameData* data, struct SongPlayer* player) {
  if (player->current_song == NULL) {
    return;
  }
  
  player->time = -3;
  player->start_tick = (float)playdate->system->getCurrentTimeMilliseconds(); 
}

void sp_stop(GameData* data, struct SongPlayer* player) {
  if (player->current_song == NULL) {
    return;
  }
  playdate->system->logToConsole("STOPPING");
  playdate->sound->fileplayer->stop(player->current_song);
  playdate->sound->fileplayer->freePlayer(player->current_song);
  player->current_song = NULL;
}

void sp_update(GameData* data, struct SongPlayer* player) {
  if (player->current_song == NULL) {
    return;
  }

  if (!player->playing) {
    player->time = -3.0f - player->offset + 0.001f * ((float)playdate->system->getCurrentTimeMilliseconds() - player->start_tick);
    player->beat_time = player->time / player->sec_per_beat;
 
    if (player->time + player->offset > 0) {
      playdate->sound->fileplayer->play(player->current_song, 1);
      player->song_start = playdate->sound->getCurrentTime();
      player->playing = 1;
    }
            
    return;
  }
  
  player->time = (float)(playdate->sound->getCurrentTime() - player->song_start) / 44100.0f - player->offset;
  player->beat_time = player->time / player->sec_per_beat;
  player->percentage = player->time / player->length;
}
