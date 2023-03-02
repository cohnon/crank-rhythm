// ***************
//  Scene Manager
// ***************
//
// A simple scene manager for the playdate.
// A scene is made up of 3 callbacks: on_start, on_update, and on_end,
// as well as a custom struct which will automatically be created and destroyed.
//
// Note: Your custom data will be set to all 0 when initialised
//
// Author: Coherent Nonsense


#ifndef SCENE_H
#define SCENE_H

#include <pd_api.h>
#include <stdint.h>

#define MAX_SCENES 10

typedef uint8_t scene_id;

typedef void (*SceneOnStart)(void* game_data, void* scene_data);
typedef void (*SceneOnUpdate)(void* game_data, void* scene_data);
typedef void (*SceneOnEnd)(void* game_data, void* scene_data);

typedef struct Scene Scene;
typedef struct SceneManager SceneManager;


  // ************************ //
 //  Scene Manager Creation  //
// ************************ //

// Creates a new scene manager.
// game_data is an optional pointer to a struct which will be passed to every scene (global game state).
// At least 1 scene must be created.
// You are responsble for freeing it with 'scene_free'.
SceneManager* scene_new(void* game_data);

// Frees the memory for a scene manager.
void scene_free(SceneManager* manager);


  // ************************ //
 //  Scene Manager Functions //
// ************************ //

// Adds a scene to your game.
// scene_data_size is for any custom data you want automatically initialised for you.
// All callbacks are optional and can be set to NULL.
scene_id scene_add(SceneManager* manager, const int scene_data_size, SceneOnStart on_start, SceneOnUpdate on_update, SceneOnEnd on_end);

// Transitions to another scene on the next frame.
void scene_transition(SceneManager* manager, const scene_id scene);

// Updates the scene manager.
// Must be called once per frame.
void scene_update(SceneManager* manager);

#endif