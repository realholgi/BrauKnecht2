#ifndef CONFIG_H
#define CONFIG_H

#define DEBUG

#define SSID "eedv"
#define PASS "mega%dude"

enum PinAssignments {
    encoderPinA = D5,
    encoderPinB = D6,
    tasterPin = D7,
    oneWirePin = D3,
    heizungPin = D4,
    beeperPin = D8,
};

#endif //CONFIG_H
