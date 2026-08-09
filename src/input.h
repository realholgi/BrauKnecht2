#pragma once

#include <ClickEncoder.h>

extern ClickEncoder encoder1;

void encoderTicker();
bool getButton();

// timer1 drives encoderTicker. Pause it around flash reads, writes, and erases:
// the ISR would otherwise run library code from flash while the cache is
// disabled and crash. Pause/Resume are no-ops until encoderTimerSetup() has run.
void encoderTimerSetup();
void encoderTimerPause();
void encoderTimerResume();

// RAII helper — pause for the lifetime of a flash-access scope.
struct EncoderTimerGuard {
    EncoderTimerGuard() { encoderTimerPause(); }
    ~EncoderTimerGuard() { encoderTimerResume(); }
};
