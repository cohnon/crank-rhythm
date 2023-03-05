#include "particles.h"
#include "pd_api.h"
#include "pd_api/pd_api_gfx.h"
#include <stdint.h>
#include <string.h>


#define RNG_DIRECTION_LENGTH (40)

// Random direction lookup table
static int rng_direction_index;
static const float RNG_DIRECTION[RNG_DIRECTION_LENGTH * 2] = {
  0.841259265433149,   -0.5406318972488384, -0.5349408018299417, -0.8448895422110153,
  0.6823677095838336,   0.7310091031699353,  0.5375892696013794,  -0.843206841296639,
  0.8069759365772174,   0.590584318946349,   0.5937166381637259,  -0.8046741909416278,
  -0.12650644582844384,-0.9919657852788346,  0.9411482263501566,  -0.3379941065136407,
  0.25303865751916693,  0.9674561684132764,  0.2676089791887744,  -0.9635275991156362,
  0.607939139776964,    0.7939836285007676, -0.7144826618289678, -0.6996531468847923,
  -0.6560140528758291,  0.7547486750100518,  0.9565361561090179,   0.29161375491595803,
  -0.8718449774667018,  0.48978192623461125, 0.4699429678779538,  -0.8826967808608234,
  -0.9467703928570526,  0.32190965069302024, 0.7317655183098842,   0.6815564732380336,
  -0.6589854194636962,  0.7521557132231705, -0.4441104765780634, -0.8959720333769382,
  -0.7540581539254659, -0.6568076586783368,  0.9758767619337619, 0.21832211412881564,
  0.5064530410405741, -0.862267543875307,   -0.9804470414616216, -0.19678312653566885,
  0.020035795805328604, -0.9997992632956114,-0.9486761245827259, -0.3162492856066247,
  0.3181758189395624, -0.9480317232256201,  -0.5152017729607454, 0.8570689197130559,
  0.5419336447692151, 0.8404212780904313,   -0.97765103753426, -0.21023427125039557,
  -0.8979844677715709, 0.44002715329966685,  0.3914545773270262, -0.9201974320164772,
  0.5411707749650121, 0.8409127138554681,   -0.5591836814832464, 0.8290437927895263,
  0.9964945169006336, 0.08365810054604737,   0.9041822344407833, -0.4271469149153163,
  0.9716516484439374, 0.23641716112875422,   0.6382235165663452, -0.7698511173608102,
  -0.9342965888382285, -0.35649668173105115,-0.8023379374672804, 0.5968700311631924, 
};

PlaydateAPI* playdate;

typedef struct Particle {
  int state;
  float x;
  float y;
  unsigned int lifetime;
  float dir_x;
  float dir_y;
} Particle;

typedef struct Emitter {
  int16_t id;
  int16_t state;
  unsigned int lifetime;
  unsigned int timer;
  float x;
  float y;
} Emitter;

typedef struct ParticleSystem {
  int ms_per_particle;
  int ms_lifetime;
  int ms_emit_lifetime;
  ParticleConfig config;
  Particle* particles;
  int particles_length;
  
  uint8_t* particles_free;
  uint8_t particles_free_length;

  Emitter* emitters;
  int emitters_length;

  emitter_id* emitters_free;
  uint8_t emitters_free_length;
} ParticleSystem;

void particles_set_pd_ptr(PlaydateAPI* pd) {
  playdate = pd;
}

// Creates a new particle system.
// You are responsible for freeing it with 'particles_freeSystem'
ParticleSystem* particles_newSystem(const ParticleConfig config) {
  ParticleSystem* particles = (ParticleSystem*)playdate->system->realloc(NULL, sizeof(ParticleSystem));
  
  // Default values
  particles->config = (ParticleConfig){
    .max_particles = 100,
    .max_emitters = 20,
    .emit_rate = 10.0f,
    .emit_lifetime = config.particle_lifetime,
    .particle_start_velocity = 5.0f,
    .particle_end_velocity = 5.0f,
    .particle_lifetime = 1.0f,
    .particle_start_size = 10,
    .particle_end_size = 10,
    .particle_color = config.particle_color,
  };
  
  // Apply config
  if (config.max_particles > 0) { particles->config.max_particles = config.max_particles; }
  if (config.max_emitters > 0) { particles->config.max_emitters = config.max_emitters; }
  if (config.emit_rate > 0.0f) { particles->config.emit_rate = config.emit_rate; }
  if (config.emit_lifetime > 0.0f) { particles->config.emit_lifetime = config.emit_lifetime; }
  if (config.particle_start_velocity > 0.0f) { particles->config.particle_start_velocity = config.particle_start_velocity; }
  if (config.particle_end_velocity > 0.0f) { particles->config.particle_end_velocity = config.particle_end_velocity; }
  if (config.particle_lifetime > 0.0f) { particles->config.particle_lifetime = config.particle_lifetime; }
  if (config.particle_start_size > 0.0f) { particles->config.particle_start_size = config.particle_start_size; }
  if (config.particle_end_size > 0.0f) { particles->config.particle_end_size = config.particle_end_size; }

  particles->particles = (Particle*)playdate->system->realloc(NULL, sizeof(Particle) * particles->config.max_particles);
  particles->particles_free = (uint8_t*)playdate->system->realloc(NULL, sizeof(uint8_t) * particles->config.max_particles);
  particles->emitters = (Emitter*)playdate->system->realloc(NULL, sizeof(Emitter) * particles->config.max_emitters);
  particles->emitters_free = (emitter_id*)playdate->system->realloc(NULL, sizeof(emitter_id) * particles->config.max_emitters);
  particles->ms_per_particle = (int)(1000.0f / particles->config.emit_rate);
  particles->ms_lifetime = (int)(1000.0f * particles->config.particle_lifetime);
  particles->ms_emit_lifetime = (int)(1000.0f * particles->config.emit_lifetime);  
  particles->particles_length = 0;
  particles->particles_free_length = 0;
  particles->emitters_length = 0;
  particles->emitters_free_length = 0;  

  return particles;
}

// Frees the memory for a particle system.
void particles_freeSystem(ParticleSystem* particles) {
  playdate->system->realloc(particles->particles, 0);
  playdate->system->realloc(particles->emitters, 0);
  playdate->system->realloc(particles, 0);
}

emitter_id particles_createEmitter(ParticleSystem* particles) {
  emitter_id id;
  if (particles->emitters_free_length > 0) {
    // Pop from free stack
    particles->emitters_free_length -= 1;
    id = particles->emitters_free[particles->emitters_free_length];
    id = (((id >> 8) + 1) << 8) + (id & 0xff); // increment generation
  } else if (particles->emitters_length < particles->config.max_emitters) {
    // Add to list if available
    id = particles->emitters_length;
    particles->emitters_length += 1;
  } else {
    return -1;
  }
  
  int index = id & 0xff;
  Emitter* emitter = &particles->emitters[index];
  emitter->id = id;
  emitter->lifetime = playdate->system->getCurrentTimeMilliseconds() + particles->ms_emit_lifetime;
  emitter->state = 0;
  emitter->timer = 0;
  emitter->x = 200;
  emitter->y = 120;
  
  return id;
}

void particles_destroyEmitter(ParticleSystem* particles, emitter_id emitter_id) {
  int index = emitter_id & 0xff;
  Emitter* emitter = &particles->emitters[index];
    
  // If the emitter doesn't exist
  if (emitter->id != emitter_id || emitter->state == -1) {
    return;
  }
    
  emitter->state = -1;
  
  // Add to free stack
  particles->emitters_free[particles->emitters_free_length] = emitter_id;
  particles->emitters_free_length += 1;
}

void particles_moveEmitter(ParticleSystem* particles, emitter_id emitter_id, float x, float y) {
  int index = emitter_id & 0xff;
  Emitter* emitter = &particles->emitters[index];
  
  if (emitter->id != emitter_id) {
    return;
  }
  
  emitter->x = x;
  emitter->y = y;
}

void particles_startEmitter(ParticleSystem* particles, emitter_id emitter_id) {
  int index = emitter_id & 0xff;
  Emitter* emitter = &particles->emitters[index];
  
  if (emitter->id != emitter_id) {
    return;
  }
  
  emitter->state = 1;
}

void particles_stopEmitter(ParticleSystem* particles, emitter_id emitter_id) {
  int index = emitter_id & 0xff;
  Emitter* emitter = &particles->emitters[index];
  
  if (emitter->id != emitter_id) {
    return;
  }
  
  emitter->state = 0;
}

// Updates the particle system.
void particles_update(ParticleSystem* particles) {
  unsigned int current_time = playdate->system->getCurrentTimeMilliseconds();

  // Run emitters
  for (int i = 0; i < particles->emitters_length; ++i) {
    Emitter* emitter = &particles->emitters[i];

    // Check emitter lifetime
    if (particles->ms_emit_lifetime > 0 && current_time > emitter->lifetime) {
      particles_destroyEmitter(particles, emitter->id);
      continue;
    }
    
    // Spawn particles    
    if (emitter->state == 1 && current_time > emitter->timer) {
      emitter->timer = current_time + particles->ms_per_particle;
            
      // Add particle
      int particle_id;
      if (particles->particles_free_length > 0) {
        // Pop from free stack
        particles->particles_free_length -= 1;
        particle_id = particles->particles_free[particles->particles_free_length];
      } else if (particles->particles_length < particles->config.max_particles) {
        // Add to list if available
        particle_id = particles->particles_length;
        particles->particles_length += 1;
      } else {
        // We ran out of particles this update loop
        // so just give up :(
        return;
      }
      Particle* particle = &particles->particles[particle_id];
      particle->state = 1;
      particle->x = emitter->x;
      particle->y = emitter->y;
      particle->dir_y = RNG_DIRECTION[rng_direction_index + 0];
      particle->dir_x = RNG_DIRECTION[rng_direction_index + 1];
      rng_direction_index += 2;
      rng_direction_index %= RNG_DIRECTION_LENGTH;
      particle->lifetime = current_time + particles->ms_lifetime;
    }
  }
  
  // Run particles
  for (int particle_id = 0; particle_id < particles->particles_length; ++particle_id) {
    Particle* particle = &particles->particles[particle_id];
    if (particle->state == 1) {
      if (current_time > particle->lifetime) {
        particle->state = -1;

        particles->particles_free[particles->particles_free_length] = particle_id;
        particles->particles_free_length += 1;
        continue;
      }
            
      float progress = (float)(particle->lifetime - current_time) / (float)particles->ms_lifetime;

      float velocity = particles->config.particle_end_velocity + progress * (particles->config.particle_start_velocity - particles->config.particle_end_velocity);
      particle->x += particle->dir_x * velocity;
      particle->y += particle->dir_y * velocity;
      
      int size = (int)(particles->config.particle_end_size + progress * (particles->config.particle_start_size - particles->config.particle_end_size));

      playdate->graphics->fillEllipse((int)particle->x - (size >> 1), (int)particle->y - (size >> 1), size, size, 0.0f, 360.0f, particles->config.particle_color);
    }
  }
}
