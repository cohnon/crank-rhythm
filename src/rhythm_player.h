// ***************
//  Rhythm Player
// ***************
//
// A wrapper over a fileplayer to keep track of a song's beat and progress
// using BPM and an offset
//
// Author: Coherent Nonsense

#ifndef RHYTHM_PLAYER_H
#define RHYTHM_PLAYER_H

#include "pd_api.h"

typedef struct RhythmPlayer RhythmPlayer;


  // ************************ //
 //  Rhythm Player Creation  //
// ************************ //

// You must call this before any other function.
void rhythm_set_pd_ptr(PlaydateAPI* pd);

// Creates a new rhythm player.
// You are responsible for freeing it with 'rhythm_freePlayer'.
RhythmPlayer* rhythm_newPlayer();

// Frees the memory for a rhythm player.
void rhythm_freePlayer(RhythmPlayer* rhythm);

// Loads an MP3 or PDA at a specific path.
// returns 0 on failure and 1 on success.
int rhythm_load(RhythmPlayer* rhythm, const char* path, const float bpm, const float offset);


  // ************************ //
 //  Rhythm Player Controls  //
// ************************ //

void rhythm_start_timer(RhythmPlayer* rhythm);
void rhythm_check_timer(RhythmPlayer* rhythm);

// Plays audio count times.
// If count is 0, audio will loop until explicityly stopped.
void rhythm_play(RhythmPlayer* rhythm, const int count);

// Plays audio in [delay] seconds.
// If delay isn't 0, getTime and getBeatTime will return negative values until the audio starts.
// If delay isn't 0, the audio won't start until getTime or getDelayTime is called.
void rhythm_playDelay(RhythmPlayer* rhythm, const float delay);

// Stops audio
void rhythm_stop(RhythmPlayer* rhythm);

// Jumps to a specific time
void rhythm_skipTo(RhythmPlayer* rhythm, float beat_time);


  // *********************** //
 //  Rhythm Player Getters  //
// *********************** //

// Gets the fileplayer object.
// You should not modify the fileplayer's offset.
// Use rhythm_skipTo if you want to change the audio's offset.
FilePlayer* rhythm_getFileplayer(RhythmPlayer* rhythm);

// Returns whether the rhythm player is playing.
int rhythm_isPlaying(RhythmPlayer* rhythm);

// Gets the time since the audio has started.
float rhythm_getTime(RhythmPlayer* rhythm);

// Gets the current beat the audio is at.
float rhythm_getBeatTime(RhythmPlayer* rhythm);

// Gets the progress of the song in the range [0, 1].
// 0 -> Audio is at the start, 1 -> Audio is at the end.
float rhythm_getProgress(RhythmPlayer* rhythm);

// Whether we are currently at a whole note.
// Will return true for [threshold] seconds after the whole note.
int rhythm_isOnBeat(RhythmPlayer* rhythm, float threshold);

#endif