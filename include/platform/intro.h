#pragma once

#include <stdbool.h>

/* Plays the packaged Pixel Electronics Studio intro once before the game menu.
 * Missing/invalid intro assets are treated as non-fatal so development builds
 * can still boot straight to FNaF 3. */
bool intro_play(void);
