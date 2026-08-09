#include <Arduino.h>

#include "input.h"
#include "config.h"
#include "global.h"
#include "statemachine.h"
#include "recipe_timing.h"

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
    // Double-click = "back one step". The window is a tradeoff: every single
    // click is delayed this long (to tell it apart from a double), but too short
    // and a relaxed double-click reads as two single clicks. 280 ms is the
    // chosen balance.
    encoder1.setDoubleClickEnabled(true);
    encoder1.setDoubleClickTime(280);

    timer1_isr_init();
    timer1_attachInterrupt(encoderTicker);
    timerReady = true;
    encoderTimerResume();
}

bool getButton() {
    ButtonPressed = false;

    ClickEncoder::Button b = encoder1.getButton();
    switch (b) {
        case ClickEncoder::Held:
            if ((regelung == REGL_MAISCHEN || regelung == REGL_KOCHEN) && brewIsActive(modus)) {
                holdReturnModus = modus;
                holdReturnX = x;
                holdTarget = sollwert;
                if (modus == AUTO_RAST_ZEIT || modus == KOCHEN_AUTO_LAUF) {
                    pauseBrewClock(brewClock, static_cast<uint32_t>(millis()));
                }
                holdWasHeating = heizung;
                modus = BRAUVORGANG_HALT;
                anfang = true;
            } else if (modus != BRAUVORGANG_HALT) {
                modus = ABBRUCH;
            }
            break;

        case ClickEncoder::DoubleClicked:
            goBackOneStep();       // double-click: step back one screen (input only)
            break;

        case ClickEncoder::Clicked:
            ButtonPressed = true;
            break;

        default:
            break;
    }

    return ButtonPressed;
}
