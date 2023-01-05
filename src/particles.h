#ifndef PARTICLES_H
#define PARTICLES_H

#include "pd_api.h"

struct Particle {
  float x;
  float y;
  float velocity_x;
  float velocity_y;
};

void particles_spawn(int x, int y, float rotation);
void particles_update();
void particles_draw(PlaydateAPI* playdate);

#endif