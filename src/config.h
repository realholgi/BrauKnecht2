#pragma once

// DEBUG (serial logging) is set by the build env, not here:
//   pio run -e d1_mini         -> release (silent, beeper self-test on boot)
//   pio run -e d1_mini_debug   -> serial debug on 115200

enum PinAssignments {
    encoderPinA = D5,
    encoderPinB = D6,
    tasterPin = D7,
    oneWirePin = D3,
    heizungPin = D4,
    beeperPin = D8,
};
