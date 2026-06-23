#include <Arduino.h>

#include "input.h"
#include "config.h"
#include "global.h"

ClickEncoder encoder1 = ClickEncoder(encoderPinA, encoderPinB, tasterPin, ENCODER_STEPS_PER_NOTCH);

void IRAM_ATTR encoderTicker() {
    encoder1.service();
    drehen += encoder1.getValue();
}

static bool timerReady = false;

void encoderTimerResume() {
    if (!timerReady) return;
    timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
    timer1_write(5000);   // 80 MHz / 16 = 5 MHz -> 5000 ticks = 1 ms
}

void encoderTimerPause() {
    if (timerReady) timer1_disable();
}

void encoderTimerSetup() {
    timer1_isr_init();
    timer1_attachInterrupt(encoderTicker);
    timerReady = true;
    encoderTimerResume();
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
