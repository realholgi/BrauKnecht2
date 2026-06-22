#include <Arduino.h>

#include "input.h"
#include "config.h"
#include "global.h"

ClickEncoder encoder1 = ClickEncoder(encoderPinA, encoderPinB, tasterPin, ENCODER_STEPS_PER_NOTCH);

void ICACHE_RAM_ATTR encoderTicker() {
    encoder1.service();
    drehen += encoder1.getValue();
}

bool getButton() {
    ButtonPressed = false;

    ClickEncoder::Button b = encoder1.getButton();
    if (b != ClickEncoder::Open) {
        switch (b) {
            case ClickEncoder::Held:
                modus = ABBRUCH;
                break;

            case ClickEncoder::Clicked:
                ButtonPressed = true;
                break;

            default:
                ButtonPressed = false;
                break;
        }
    }

    return ButtonPressed;
}
