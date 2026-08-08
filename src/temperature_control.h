#pragma once

#include <stdint.h>

struct TemperatureControlState {
    bool initialized = false;
    bool heaterOn = false;
    bool hasHeatingRate = false;
    bool observingCoast = false;
    int setpoint = 0;
    uint32_t lastSwitchMs = 0;
    uint32_t lastSampleMs = 0;
    uint32_t coastStartedMs = 0;
    float lastSampleC = 0.0F;
    float heatingRateCPerSecond = 0.0F;
    float coastSeconds = 30.0F;
    float coastStartC = 0.0F;
    float coastPeakC = 0.0F;
    float coastStartRateCPerSecond = 0.0F;
};

void resetTemperatureControl(TemperatureControlState &state);
bool updateTemperatureControl(TemperatureControlState &state,
                              float currentTemperature, int setpoint,
                              uint32_t now);
