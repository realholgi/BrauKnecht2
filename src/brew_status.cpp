#include "brew_status.h"

#include <stdio.h>

namespace {
constexpr BrewStatus inactiveStatus() {
    return {-1, "", false, 0, 0, 0};
}

BrewStatus timedStatus(int index, const char *label, unsigned long elapsed, unsigned long total) {
    return {index, label, true, elapsed, total, elapsed >= total ? 0 : total - elapsed};
}

const char *restLabel(int index) {
    static char label[16];
    snprintf(label, sizeof(label), "%d. Rast", index);
    return label;
}
}

BrewStatus brewStatus(const BrewStatusInput &input) {
    BrewStatus status = inactiveStatus();

    if (input.mode == AUTO_RAST_ZEIT && input.x >= 1 && input.x <= input.rasten && input.rastZeit) {
        const int minutes = input.rastZeit[input.x];
        if (minutes >= 0) {
            status = timedStatus(input.x, restLabel(input.x), input.elapsedSeconds,
                                 static_cast<unsigned long>(minutes) * 60UL);
        }
    } else if (input.mode == KOCHEN_AUTO_LAUF && input.kochzeit >= 0) {
        status = timedStatus(input.rasten + 2, "Kochen", input.elapsedSeconds,
                             static_cast<unsigned long>(input.kochzeit) * 60UL);
    }

    if (!input.hold) return status;

    BrewStatusInput heldInput = input;
    heldInput.mode = input.holdReturnMode;
    heldInput.x = input.holdReturnX;
    heldInput.hold = false;
    heldInput.elapsedSeconds = input.elapsedSeconds;
    const BrewStatus held = brewStatus(heldInput);
    static char label[24];
    snprintf(label, sizeof(label), "HOLD: %s", held.activeStepLabel);
    return {held.activeStepIndex, label, false, input.elapsedSeconds, 0, 0};
}
