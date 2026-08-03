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
