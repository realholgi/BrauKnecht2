#include <Arduino.h>
#include <TimeLib.h>

#include "hardware.h"
#include "config.h"
#include "global.h"

static int hendi_on_start = 0;

static void hendi_special_handling();

void beeperOnOff(bool value) {
    if (value) {
        digitalWrite(beeperPin, HIGH); // einschalten
    } else {
        digitalWrite(beeperPin, LOW); // ausschalten
    }
}

void schalte_heizung_an() {
    digitalWrite(heizungPin, LOW);   // einschalten
}

void schalte_heizung_aus() {
    digitalWrite(heizungPin, HIGH);   // ausschalten
}

void heizungOnOff(bool value) {
    if (value) {
        hendi_special_handling();
        schalte_heizung_an();
    } else {
        schalte_heizung_aus();
        hendi_on_start = 0;
    }
}

// HENDI-Spezial, welche nach 90 min abschaltet. Also vorher mal kurz ein-/ausschalten
static void hendi_special_handling() {
    int hendi_is_on_duration = (hour() * 60) + minute() - hendi_on_start;
    if (hendi_is_on_duration >= HENDI_MAX_RUNTIME) {
        schalte_heizung_aus();
        delay(1000);
        schalte_heizung_an();
        hendi_on_start = 0;
    }
    if (hendi_on_start == 0) {
        hendi_on_start = (hour() * 60) + minute();
    }
}
