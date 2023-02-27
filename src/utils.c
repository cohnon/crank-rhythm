#include "utils.h"

// This isn't exposed in the API for some reason.
static const int ESCAPE_KEYCODE = (0x1 << 6);

int any_button(const PDButtons buttons) {
  return (buttons > 0) && !(buttons & ESCAPE_KEYCODE);
}