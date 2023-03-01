#include "scene.h"
#include "game.h"

#include <assert.h>
#include <string.h>

static const uint8_t SCENE_ID_NONE = 0xff;

typedef struct Scene {
  void* data;
  uint32_t data_size;
  SceneOnStart on_start;
  SceneOnUpdate on_update;
  SceneOnEnd on_end;
} Scene;

typedef struct SceneManager {
  void* game_data;
  Scene scenes[MAX_SCENES];
  uint8_t scenes_length;
  scene_id current_scene_id;
  scene_id next_scene_id;
} SceneManager;

SceneManager* scene_new(void* game_data) {
  SceneManager* manager = (SceneManager*)playdate->system->realloc(NULL, sizeof(SceneManager));
  
  manager->game_data = game_data;
  manager->scenes_length = 0;
  
  manager->current_scene_id = SCENE_ID_NONE;
  manager->next_scene_id = 0;
  
  return manager;
}

void scene_delete(SceneManager* manager) {
  playdate->system->realloc((void*)manager, 0);
}

scene_id scene_add(SceneManager* manager, const int scene_data_size, SceneOnStart on_start, SceneOnUpdate on_update, SceneOnEnd on_end) {
  scene_id id = manager->scenes_length;
  Scene* new_scene = &manager->scenes[id];
  new_scene->data_size = scene_data_size;
  new_scene->on_start = on_start;
  new_scene->on_update = on_update;
  new_scene->on_end = on_end;
  
  manager->scenes_length += 1;
  
  return id;
}

void scene_transition(SceneManager* manager, scene_id id) {
  assert(id < manager->scenes_length);
  
  manager->next_scene_id = id;
}

void scene_update(SceneManager* manager) {
  assert(manager->scenes_length > 0);

  if (manager->next_scene_id != manager->current_scene_id) {    
    // End old scene
    if (manager->current_scene_id != SCENE_ID_NONE) {
      Scene* old_scene = &manager->scenes[manager->current_scene_id];
      if (old_scene->on_end != NULL) {
        old_scene->on_end(manager->game_data, old_scene->data);      
      }
      playdate->system->realloc(manager->scenes[manager->current_scene_id].data, 0);  
    }

    // Create new scene
    manager->current_scene_id = manager->next_scene_id;
    Scene* scene = &manager->scenes[manager->current_scene_id];
    scene->data = playdate->system->realloc(NULL, scene->data_size);
    memset(scene->data, 0, scene->data_size);
    
    if (scene->on_start != NULL) {
      scene->on_start(manager->game_data, scene->data);    
    }
  }

  Scene* scene = &manager->scenes[manager->current_scene_id];
  
  if (scene->on_update != NULL) {
    scene->on_update(manager->game_data, scene->data);  
  }
}
