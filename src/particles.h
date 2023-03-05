// *****************
//  Particle System
// *****************
//
// A particle system where you can create custom emitters.
//
// NOTE: I am adding functionality as I need it so it is very bare bones.
//
// Author: Coherent Nonsense

#ifndef PARTICLES_H
#define PARTICLES_H

#include <pd_api.h>

// TODO: I should use generational IDs
typedef int emitter_id;
typedef struct ParticleSystem ParticleSystem;

// typedef enum EmitionType {
  // kEmitCircle,
  // kEmitCone,
// } ParticleEmitionType;

// typedef enum ParticleType {
  // kParticleCircle,
  // kParticleSquare,
  // kParticleTriangle,
  // kParticleBitmap,
// } ParticleType;

// NOTE: Commented fields are ideas.
// I will implement them as I need them.
typedef struct ParticleConfig {
  
  unsigned int max_particles;
  unsigned int max_emitters;

  // Emmiter
  // int        emit_type;
  float      emit_rate;
  float      emit_lifetime;
  // float      emit_angle;
  // float      emit_rotation;
  
  // int        particle_type;
  // float      particle_rotation_speed;
  // LCDBitmap* particle_bitmap;
  float      particle_start_velocity;
  float      particle_end_velocity;
  float      particle_lifetime;
  // float      particle_start_angle;
  // float      particle_end_angle;
  float      particle_start_size;
  float      particle_end_size;
  int        particle_color;
  // int        particle_end_color;

} ParticleConfig;


void particles_set_pd_ptr(PlaydateAPI* pd);

// Creates a new particle system.
// You are responsible for freeing it with 'particles_freeSystem'
ParticleSystem* particles_newSystem(const ParticleConfig config);

// Frees the memory for a particle system.
void particles_freeSystem(ParticleSystem* particles);

// Updates the particle system.
void particles_update(ParticleSystem* particles);


  // ********* //
 //  Emitter  //
// ********  //

// Creates an emitter for a particle system.
// Pass a reference to an x and y position which the
// particle system will spawn particles from.
// x and y must exist for the lifetime of the emitter
emitter_id particles_createEmitter(ParticleSystem* particles);

// Destroys an emitter.
void particles_destroyEmitter(ParticleSystem* particles, emitter_id emitter_id);

// Moves an emitter to an (x, y) position on the screen.
void particles_moveEmitter(ParticleSystem* particles, emitter_id emitter_id, float x, float y);

// Starts emitting particles from the emitter's location
void particles_startEmitter(ParticleSystem* particles, emitter_id emitter_id);

// Stops emitting particles from the emitter's location
void particles_stopEmitter(ParticleSystem* particles, emitter_id emitter_id);

#endif