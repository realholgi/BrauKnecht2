#pragma once

constexpr int MANUAL_SETPOINT_MIN = 10;
constexpr int MANUAL_SETPOINT_MAX = 100;

int clampManualSetpoint(int value);
