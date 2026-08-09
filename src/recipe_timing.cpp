#include "recipe_timing.h"

namespace {
uint8_t validHopMask(int hopCount) {
    if (hopCount <= 0) return 0;

    const uint8_t count = hopCount > MAX_HOP_DEADLINES
        ? MAX_HOP_DEADLINES
        : static_cast<uint8_t>(hopCount);
    return static_cast<uint8_t>((1U << count) - 1U);
}
}  // namespace

void startBrewClock(BrewClockState &state, uint32_t nowMs) {
    state.accumulatedMs = 0;
    state.runningSinceMs = nowMs;
    state.running = true;
}

void pauseBrewClock(BrewClockState &state, uint32_t nowMs) {
    if (!state.running) return;

    state.accumulatedMs += nowMs - state.runningSinceMs;
    state.running = false;
}

void resumeBrewClock(BrewClockState &state, uint32_t nowMs) {
    if (state.running) return;

    state.runningSinceMs = nowMs;
    state.running = true;
}

void resetBrewClock(BrewClockState &state) {
    state.accumulatedMs = 0;
    state.runningSinceMs = 0;
    state.running = false;
}

uint32_t brewElapsedSeconds(const BrewClockState &state, uint32_t nowMs) {
    uint32_t elapsedMs = state.accumulatedMs;
    if (state.running) elapsedMs += nowMs - state.runningSinceMs;
    return elapsedMs / 1000UL;
}

void resetHopDeadlines(HopDeadlineState &state) {
    state.emittedMask = 0;
}

uint8_t collectDueHops(HopDeadlineState &state, const int *hopMinutes,
                       int hopCount, uint32_t elapsedSeconds) {
    if (hopMinutes == nullptr) return 0;

    const uint8_t availableMask = validHopMask(hopCount);
    const uint32_t elapsedMinutes = elapsedSeconds / 60UL;
    uint8_t dueMask = 0;

    for (uint8_t index = 1; index <= MAX_HOP_DEADLINES; ++index) {
        const uint8_t bit = static_cast<uint8_t>(1U << (index - 1));
        if ((availableMask & bit) == 0 || (state.emittedMask & bit) != 0) continue;
        if (hopMinutes[index] <= static_cast<int>(elapsedMinutes)) dueMask |= bit;
    }

    state.emittedMask |= dueMask;
    return dueMask;
}

uint8_t nextPendingHopIndex(const HopDeadlineState &state, int hopCount) {
    const uint8_t availableMask = validHopMask(hopCount);
    for (uint8_t index = 1; index <= MAX_HOP_DEADLINES; ++index) {
        const uint8_t bit = static_cast<uint8_t>(1U << (index - 1));
        if ((availableMask & bit) != 0 && (state.emittedMask & bit) == 0) return index;
    }
    return 0;
}
