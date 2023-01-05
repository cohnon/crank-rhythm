#include "song_player.h"
#include <stdio.h>

void sp_init(struct SongPlayer* player, PlaydateAPI* playdate) {
  player->pd = playdate;
}

void sp_load(struct SongPlayer* player, FilePlayer* song, float bpm, float offset) {  
  player->current_song = song;
  player->bpm = bpm;
  player->sec_per_beat = 60 / bpm;
  player->offset = offset;
  player->length = player->pd->sound->fileplayer->getLength(player->current_song) / 2.0f - player->offset;
}

void sp_play(struct SongPlayer* player, FilePlayer* song, float bpm, float offset) {
  player->pd->sound->fileplayer->play(player->current_song, 1);
  player->start_tick = (float)player->pd->sound->getCurrentTime();
}

void sp_pause(struct SongPlayer* player) {
  player->pd->sound->fileplayer->pause(player->current_song);
}

void sp_stop(struct SongPlayer* player) {
}

void sp_update(struct SongPlayer* player) {
  if (!player->pd->sound->fileplayer->isPlaying(player->current_song)) {
    return;
  }
  player->time = ((float)player->pd->sound->getCurrentTime() - player->start_tick) / 44100.0f - player->offset;
  player->beat_time = player->time / player->sec_per_beat;
}
