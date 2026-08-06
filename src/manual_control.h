#pragma once

#include <stdint.h>
#include "global.h"

constexpr int MANUAL_SETPOINT_MIN = 10;
constexpr int MANUAL_SETPOINT_MAX = 100;
constexpr uint32_t MANUAL_TARGET_BEEP_ON_MS = 1000;
constexpr uint32_t MANUAL_TARGET_BEEP_CYCLE_MS = 1500;
constexpr uint32_t MANUAL_TARGET_BEEP_DURATION_MS = 7500;

struct ManualTargetBeepState {
    int setpoint = MANUAL_SETPOINT_MIN;
    uint32_t startedAt = 0;
    bool armed = false;
    bool active = false;
};

int clampManualSetpoint(int value);
bool canEnterManualMode(MODUS currentMode, REGEL_MODE currentRegulation);
void armManualTargetBeep(ManualTargetBeepState &state, int setpoint);
bool updateManualTargetBeep(ManualTargetBeepState &state, int setpoint,
                            float currentTemperature, uint32_t now);
