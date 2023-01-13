#include "song_player.h"
#include <stdio.h>

void sp_init(struct SongPlayer* player, PlaydateAPI* playdate) {
  player->pd = playdate;
  player->playing = 0;
}

void sp_load(struct SongPlayer* player, FilePlayer* song, float bpm, float offset, float length) {
  player->current_song = song;
  player->bpm = bpm;
  player->sec_per_beat = 60 / bpm;
  player->offset = offset;
  // manually setting length because fileplayer->getlength seems broken
  // its only used to show progress bar so it doesnt have to be that accurate
  // player->length = player->pd->sound->fileplayer->getLength(player->current_song) - player->offset;
  player->length = length;
}

void sp_play(struct SongPlayer* player) {
  player->time = -3;
  player->start_tick = (float)player->pd->system->getCurrentTimeMilliseconds(); 
  // player->pd->sound->fileplayer->play(player->current_song, 1);
  // player->start_tick = (float)player->pd->sound->getCurrentTime();
}

void sp_pause(struct SongPlayer* player) {
  player->pd->sound->fileplayer->pause(player->current_song);
}

void sp_stop(struct SongPlayer* player) {
  player->playing = 0;
  player->pd->sound->fileplayer->stop(player->current_song);
}

void sp_update(struct SongPlayer* player) {
  if (!player->playing) {
    player->time = -3.0f - player->offset + 0.001 * ((float)player->pd->system->getCurrentTimeMilliseconds() - player->start_tick);
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
