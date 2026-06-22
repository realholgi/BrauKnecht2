#pragma once

#include <stddef.h>

// Pure display-formatting helpers — no hardware deps, unit-tested on native.

// Resolve a requested column (which may be LEFT/RIGHT/CENTER) for a string of
// `len` chars and clamp it to the visible width.
int alignX(int x, int len);

// Render `value` minutes right-justified in 3 digits followed by " min"
// (e.g. "  5 min", " 50 min", "500 min"). Clamps to 0..999.
void formatMinutes(char *buf, size_t n, int value);
