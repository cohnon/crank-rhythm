#include "game.h"

#include "pd_api.h"

static int update(void* userdata);

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg) {
  (void)arg; // arg is currently only used for event = kEventKeyPressed
  
  if (event == kEventInit) {
    game_setup_pd(playdate);
    game_init();

    playdate->system->setUpdateCallback(update, playdate);
  }
  
  return 0;
}

static int update(void* userdata) {	
  game_update();

  return 1;
}

