#include "manual_control.h"

int clampManualSetpoint(int value) {
    if (value < MANUAL_SETPOINT_MIN) {
        return MANUAL_SETPOINT_MIN;
    }
    if (value > MANUAL_SETPOINT_MAX) {
        return MANUAL_SETPOINT_MAX;
    }
    return value;
}

bool canEnterManualMode(MODUS currentMode, REGEL_MODE currentRegulation) {
    return currentMode == MANUELL || currentRegulation == REGL_AUS;
}

void armManualTargetBeep(ManualTargetBeepState &state, int setpoint) {
    state.setpoint = setpoint;
    state.armed = true;
    state.active = false;
    state.startedAt = 0;
}

bool updateManualTargetBeep(ManualTargetBeepState &state, int setpoint,
                            float currentTemperature, uint32_t now) {
    if (state.setpoint != setpoint) {
        armManualTargetBeep(state, setpoint);
    }

    if (state.active) {
        const uint32_t elapsed = now - state.startedAt;
        if (elapsed >= MANUAL_TARGET_BEEP_DURATION_MS) {
            state.active = false;
            return false;
        }
        return elapsed % MANUAL_TARGET_BEEP_CYCLE_MS < MANUAL_TARGET_BEEP_ON_MS;
    }

    if (state.armed && currentTemperature >= setpoint) {
        state.startedAt = now;
        state.armed = false;
        state.active = true;
        return true;
    }

    return false;
}
