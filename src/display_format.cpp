#include <stdio.h>

#include "display_format.h"
#include "display.h"

int alignX(int x, int len) {
    if (x == RIGHT) {
        x = DISPLAY_SIZE_X - len;
    }
    if (x == CENTER) {
        x = (DISPLAY_SIZE_X - len) / 2;
    }
    if (x < 0) {
        x = 0;
    }
    if (x > DISPLAY_SIZE_X - 1) {
        x = DISPLAY_SIZE_X - 1;
    }
    return x;
}

void formatMinutes(char *buf, size_t n, int value) {
    if (value < 0) {
        value = 0;
    }
    if (value > 999) {
        value = 999;
    }
    snprintf(buf, n, "%3d min", value);
}

int listWindowStart(int cursor, int count, int visible) {
    if (count <= visible) {
        return 1;  // everything fits, no scrolling
    }
    int start = cursor - visible / 2;  // keep the cursor roughly centred
    int maxStart = count - visible + 1;
    if (start < 1) {
        start = 1;
    }
    if (start > maxStart) {
        start = maxStart;
    }
    return start;
}
