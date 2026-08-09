#pragma once

#include <stdint.h>

constexpr uint8_t MAX_HOP_DEADLINES = 6;

struct BrewClockState {
    uint32_t accumulatedMs = 0;
    uint32_t runningSinceMs = 0;
    bool running = false;
};

struct HopDeadlineState {
    uint8_t emittedMask = 0;
};

void startBrewClock(BrewClockState &state, uint32_t nowMs);
void pauseBrewClock(BrewClockState &state, uint32_t nowMs);
void resumeBrewClock(BrewClockState &state, uint32_t nowMs);
void resetBrewClock(BrewClockState &state);
uint32_t brewElapsedSeconds(const BrewClockState &state, uint32_t nowMs);

void resetHopDeadlines(HopDeadlineState &state);
uint8_t collectDueHops(HopDeadlineState &state, const int *hopMinutes,
                       int hopCount, uint32_t elapsedSeconds);
uint8_t nextPendingHopIndex(const HopDeadlineState &state, int hopCount);
