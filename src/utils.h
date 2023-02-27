#ifndef UTILS_H
#define UTILS_H

// This file contains a bunch of random functions that don't have a home anywhere else :(

#include <pd_api.h>


// Checks if any button has been hit
// This is for user prompts like "hit any button to continue"
// but it turns out that the menu button also activates this
// which is kind of annoying so I ignore the menu button.
int any_button(const PDButtons buttons);

#endif