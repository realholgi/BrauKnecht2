#pragma once

#include "global.h"

struct BrewStatusInput {
    MODUS mode;
    MODUS holdReturnMode;
    int x;
    int holdReturnX;
    int rasten;
    int hopfenanzahl;
    const int *rastZeit;
    int kochzeit;
    unsigned long elapsedSeconds;
    bool hold;
};

struct BrewStatus {
    int activeStepIndex;
    const char *activeStepLabel;
    bool timed;
    unsigned long elapsedSeconds;
    unsigned long totalSeconds;
    unsigned long remainingSeconds;
};

BrewStatus brewStatus(const BrewStatusInput &input);
