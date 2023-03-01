#ifndef PARTICLES_H
#define PARTICLES_H

#include <pd_api.h>


typedef struct ParticleSystem ParticleSystem;

typedef enum EmitionType {
  kEmitCircle,
  kEmitCone,
} ParticleEmitionType;

typedef enum ParticleType {
  kParticleCircle,
  kParticleSquare,
  kParticleTriangle,
  kParticleBitmap,
} ParticleType;

typedef struct ParticleConfig {
  int emit_type;
  float emit_rate;
  float emit_angle;
  float emit_rotation;
  
  int particle_type;
  float particle_angle;
  // shape
  LCDBitmap* bitmap;
  int particle_size;
  int particle_color;
  float particle_rotation;
} ParticleConfig;


void particles_set_pd_ptr(PlaydateAPI* pd);

// Creates a new particle system.
// You are responsible for freeing it with 'particles_freeSystem'
ParticleSystem* particles_newSystem(ParticleConfig config) {
  return NULL;
}

// Frees the memory for a particle system.
void particles_freeSystem(ParticleSystem* system);

// Updates the particle system.
void particles_update(ParticleSystem* system);

#endif