#include <algorithm>

#include "temperature_control.h"

namespace {
constexpr uint32_t SAMPLE_INTERVAL_MS = 5000;
constexpr uint32_t MINIMUM_RELAY_OFF_MS = 60000;
constexpr uint32_t MINIMUM_PREDICTIVE_ON_MS = 5000;
constexpr float TURN_ON_DEFICIT_C = 0.4F;
constexpr float HARD_OFF_EXCESS_C = 0.5F;
constexpr float DEEP_UNDERSHOOT_C = 3.0F;
constexpr float STARTUP_LEAD_C = 0.5F;
constexpr float HEATING_RATE_EMA_WEIGHT = 0.25F;
constexpr float MINIMUM_HEATING_RATE_C_PER_SECOND = 0.001F;
constexpr float MAXIMUM_HEATING_RATE_C_PER_SECOND = 0.25F;
constexpr float DEFAULT_COAST_SECONDS = 30.0F;
constexpr float MINIMUM_COAST_SECONDS = 5.0F;
constexpr float MAXIMUM_COAST_SECONDS = 120.0F;
constexpr float COAST_LEARNING_WEIGHT = 0.5F;
constexpr float COAST_PEAK_FALL_C = 0.0625F;
constexpr uint32_t COAST_OBSERVATION_TIMEOUT_MS = 180000;
constexpr float MINIMUM_PREDICTIVE_LEAD_C = 0.2F;
constexpr float MAXIMUM_PREDICTIVE_LEAD_C = 4.0F;

float clamp(float value, float minimum, float maximum) {
    return std::max(minimum, std::min(value, maximum));
}


float forecastTemperature(const TemperatureControlState &state,
                          float currentTemperature) {
    const float lead = state.hasHeatingRate
        ? clamp(state.heatingRateCPerSecond * state.coastSeconds,
                MINIMUM_PREDICTIVE_LEAD_C, MAXIMUM_PREDICTIVE_LEAD_C)
        : STARTUP_LEAD_C;
    return currentTemperature + lead;
}

void resetSampleAnchor(TemperatureControlState &state, float currentTemperature,
                       uint32_t now) {
    state.lastSampleC = currentTemperature;
    state.lastSampleMs = now;
}

void finishCoastObservation(TemperatureControlState &state) {
    if (!state.observingCoast) {
        return;
    }

    if (state.coastStartRateCPerSecond >= MINIMUM_HEATING_RATE_C_PER_SECOND) {
        const float observedCoastSeconds = clamp(
            (state.coastPeakC - state.coastStartC) /
                state.coastStartRateCPerSecond,
            MINIMUM_COAST_SECONDS, MAXIMUM_COAST_SECONDS);
        state.coastSeconds = state.coastSeconds * (1.0F - COAST_LEARNING_WEIGHT) +
                             observedCoastSeconds * COAST_LEARNING_WEIGHT;
    }
    state.observingCoast = false;
}

void startCoastObservation(TemperatureControlState &state,
                           float currentTemperature, uint32_t now) {
    state.observingCoast = true;
    state.coastStartedMs = now;
    state.coastStartC = currentTemperature;
    state.coastPeakC = currentTemperature;
    state.coastStartRateCPerSecond = state.hasHeatingRate
        ? state.heatingRateCPerSecond
        : 0.0F;
}

void switchHeaterOn(TemperatureControlState &state, float currentTemperature,
                    uint32_t now) {
    finishCoastObservation(state);
    state.heaterOn = true;
    state.lastSwitchMs = now;
    resetSampleAnchor(state, currentTemperature, now);
}

void switchHeaterOff(TemperatureControlState &state, float currentTemperature,
                     uint32_t now) {
    state.heaterOn = false;
    state.lastSwitchMs = now;
    startCoastObservation(state, currentTemperature, now);
}
}  // namespace

void resetTemperatureControl(TemperatureControlState &state) {
    state = TemperatureControlState{};
}

bool updateTemperatureControl(TemperatureControlState &state,
                              float currentTemperature, int setpoint,
                              uint32_t now) {
    if (!state.initialized) {
        state.initialized = true;
        state.setpoint = setpoint;
        state.coastSeconds = DEFAULT_COAST_SECONDS;
        state.lastSwitchMs = now;
        resetSampleAnchor(state, currentTemperature, now);
        state.heaterOn = currentTemperature <= setpoint - TURN_ON_DEFICIT_C;
        return state.heaterOn;
    }

    if (state.observingCoast) {
        state.coastPeakC = std::max(state.coastPeakC, currentTemperature);
        if (currentTemperature <= state.coastPeakC - COAST_PEAK_FALL_C ||
            static_cast<uint32_t>(now - state.coastStartedMs) >=
                COAST_OBSERVATION_TIMEOUT_MS) {
            finishCoastObservation(state);
        }
    }

    const bool targetChanged = state.setpoint != setpoint;
    if (targetChanged) {
        const int previousSetpoint = state.setpoint;
        state.observingCoast = false;
        state.setpoint = setpoint;
        resetSampleAnchor(state, currentTemperature, now);

        if (setpoint < previousSetpoint &&
            forecastTemperature(state, currentTemperature) >= setpoint) {
            if (state.heaterOn) {
                switchHeaterOff(state, currentTemperature, now);
            }
        } else if (setpoint > previousSetpoint &&
                   currentTemperature <= setpoint - TURN_ON_DEFICIT_C) {
            if (!state.heaterOn) {
                switchHeaterOn(state, currentTemperature, now);
            }
        }
        return state.heaterOn;
    }

    if (state.heaterOn &&
        static_cast<uint32_t>(now - state.lastSampleMs) >= SAMPLE_INTERVAL_MS) {
        const uint32_t elapsedMs = now - state.lastSampleMs;
        const float elapsedSeconds = static_cast<float>(elapsedMs) / 1000.0F;
        const float heatingRate = (currentTemperature - state.lastSampleC) /
                                  elapsedSeconds;
        resetSampleAnchor(state, currentTemperature, now);

        if (heatingRate >= MINIMUM_HEATING_RATE_C_PER_SECOND &&
            heatingRate <= MAXIMUM_HEATING_RATE_C_PER_SECOND) {
            if (state.hasHeatingRate) {
                state.heatingRateCPerSecond =
                    state.heatingRateCPerSecond * (1.0F - HEATING_RATE_EMA_WEIGHT) +
                    heatingRate * HEATING_RATE_EMA_WEIGHT;
            } else {
                state.heatingRateCPerSecond = heatingRate;
                state.hasHeatingRate = true;
            }
        }
    }

    if (state.heaterOn) {
        if (currentTemperature >= setpoint + HARD_OFF_EXCESS_C ||
            (static_cast<uint32_t>(now - state.lastSwitchMs) >=
                 MINIMUM_PREDICTIVE_ON_MS &&
             forecastTemperature(state, currentTemperature) >= setpoint)) {
            switchHeaterOff(state, currentTemperature, now);
        }
    } else if (currentTemperature <= setpoint - DEEP_UNDERSHOOT_C ||
               (currentTemperature <= setpoint - TURN_ON_DEFICIT_C &&
                static_cast<uint32_t>(now - state.lastSwitchMs) >=
                    MINIMUM_RELAY_OFF_MS)) {
        switchHeaterOn(state, currentTemperature, now);
    }

    return state.heaterOn;
}
