#ifndef SCENE_H
#define SCENE_H

#include <stdint.h>

typedef uint8_t scene_id;

typedef struct Scene {
	void (*on_start)(void* data);
  void (*on_update)(void* data);
  void (*on_end)(void* data);
} Scene;

typedef struct SceneManager {
  scene_id current_scene;
  scene_id prev_scene;
  float transition_timer;
} SceneManager;

scene_id scene_add(Scene* scene);
void scene_transition(Scene* scene);

#endif