#ifndef SCENE_H
#define SCENE_H

#include "pd_api.h"
#include <stdint.h>

#define MAX_SCENES 10

typedef uint8_t scene_id;

typedef void (*SceneOnStart)(void* game_data, void* scene_data);
typedef void (*SceneOnUpdate)(void* game_data, void* scene_data);
typedef void (*SceneOnEnd)(void* game_data, void* scene_data);

typedef struct Scene {
	void* data;
	uint32_t data_size;
	SceneOnStart on_start;
	SceneOnUpdate on_update;
	SceneOnEnd on_end;
} Scene;

typedef struct SceneManager {
	void* game_data;
  scene_id current_scene_id;
  Scene scenes[MAX_SCENES];
  uint8_t scenes_length;
} SceneManager;

SceneManager* scene_new(void* game_data);
void scene_delete(SceneManager* manager);
scene_id scene_add(SceneManager* manager, const int scene_data_size, SceneOnStart on_start, SceneOnUpdate on_update, SceneOnEnd on_end);
void scene_transition(SceneManager* manager, const scene_id scene);
void scene_update(SceneManager* manager);

#endif