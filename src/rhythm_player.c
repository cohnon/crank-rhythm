#include "rhythm_player.h"

#include <assert.h>
#include <stdint.h>

static const int SAMPLE_RATE = 44100;
static const float INVERSE_SAMPLE_RATE = 1.0f / 44100.0f;

static const PlaydateAPI* playdate;

typedef struct RhythmPlayer {
  FilePlayer* fileplayer;
  float bps;
  float offset;
  float time;
  float length_inverse;
  int audio_start;
  uint8_t is_loaded;
  uint8_t is_playing;
  uint8_t is_started;
} RhythmPlayer;

void rhythm_set_pd_ptr(PlaydateAPI* pd) {
  playdate = pd;
}

RhythmPlayer* rhythm_newPlayer(const float bpm, const float offset) {
  assert(bpm > 0.0f && offset >= 0.0f);

  RhythmPlayer* rhythm = playdate->system->realloc(NULL, sizeof(RhythmPlayer));

  rhythm->offset = offset;
  rhythm->bps= bpm / 60.0f;
  rhythm->is_loaded = 0;
  rhythm->is_playing = 0;
  rhythm->is_started = 0;

  return rhythm;
}

void rhythm_freePlayer(RhythmPlayer* rhythm) {
  if (rhythm->is_loaded) {
    playdate->sound->fileplayer->freePlayer(rhythm->fileplayer);
  }

  playdate->system->realloc(rhythm, 0);
}


int rhythm_load(RhythmPlayer* rhythm, const char* path) {
  rhythm->fileplayer = playdate->sound->fileplayer->newPlayer();
  playdate->sound->fileplayer->setBufferLength(rhythm->fileplayer, 5.0f);
  int fileplayer_result = playdate->sound->fileplayer->loadIntoPlayer(rhythm->fileplayer, path);

  if (fileplayer_result == 0) {
    playdate->sound->fileplayer->freePlayer(rhythm->fileplayer);
    return 0;
  }

  rhythm->length_inverse = 1.0f / (playdate->sound->fileplayer->getLength(rhythm->fileplayer) - rhythm->offset);
  rhythm->is_loaded = 1;

  return 1;
}

void rhythm_play(RhythmPlayer* rhythm) {
  if (!rhythm->is_loaded) {
    return;
  }

  playdate->sound->fileplayer->play(rhythm->fileplayer, 1);
  rhythm->audio_start = playdate->sound->getCurrentTime();
  rhythm->is_started = 1;
  rhythm->is_playing = 1;
}

void rhythm_playDelay(RhythmPlayer* rhythm, float delay) {
  assert(delay >= 0.0f);
  
  if (!rhythm->is_loaded) {
    return;
  }

  rhythm->audio_start = playdate->sound->getCurrentTime() + (int)delay * SAMPLE_RATE;

  rhythm->is_started = 1;
}

void rhythm_stop(RhythmPlayer* rhythm) {
  if (!rhythm->is_loaded) {
    return;
  }

  playdate->sound->fileplayer->stop(rhythm->fileplayer);
  rhythm->is_started = 0;
  rhythm->is_playing = 0;
}

int rhythm_isPlaying(RhythmPlayer* rhythm) {
  return rhythm->is_playing;
}

float rhythm_getTime(RhythmPlayer* rhythm) {
  if (!rhythm->is_loaded || !rhythm->is_started) {
    return 0.0f;
  }

  int current_time = playdate->sound->getCurrentTime();
  if (!rhythm->is_playing && current_time > rhythm->audio_start) {
    playdate->sound->fileplayer->play(rhythm->fileplayer, 1);
    rhythm->audio_start = current_time;
    rhythm->is_playing = 1;
  }

  return ((float)(current_time - rhythm->audio_start) * INVERSE_SAMPLE_RATE) - rhythm->offset;
}

float rhythm_getBeatTime(RhythmPlayer* rhythm) {
  return rhythm_getTime(rhythm) * rhythm->bps;
}

float rhythm_getProgress(RhythmPlayer* rhythm) {
  return rhythm_getTime(rhythm) * rhythm->length_inverse;
}

