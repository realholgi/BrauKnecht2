#pragma GCC diagnostic ignored "-Wsign-compare"  // millis() vs signed wartezeit timing

#include <Arduino.h>
#include <TimeLib.h>
#include <Ticker.h>

#include "global.h"
#include "config.h"
#include "display.h"
#include "web.h"
#include "hardware.h"
#include "temperature.h"
#include "input.h"
#include "statemachine.h"
#include "persistence.h"
#include "settings.h"
#include "mqtt.h"

Ticker ticker;

void watchdogSetup();

#ifdef DEBUG
unsigned long serwartezeit = 0;
#endif

// shared mutable state (declared extern in global.h)
bool ButtonPressed = false;
volatile int drehen = 0;
bool anfang = true;
unsigned long altsekunden;
REGEL_MODE regelung = REGL_AUS;
bool sensorfehler = false;
float hysterese;
uint8_t hysteresespeicher = HYSTERESE_DEFAULT;
long wartezeit = -60000;
int n = 0;                                            //Counter Messungserhöhung zur Fehlervermeidung
uint8_t kschwelle = KOCHSCHWELLE_DEFAULT;
bool einmaldruck = false;

int sekunden = 0;
int minuten = 0;
int minutenwert = 0;
int stunden = 0;
MODUS modus = HAUPTSCHIRM;
MODUS rufmodus = HAUPTSCHIRM;
int sollwert = 20;
int maischtemp = 38;
int rasten = 1;
int rastTemp[] = {0, 50, 64, 72, 72, 72, 72, 72};
int rastZeit[] = {0, 40, 30, 20, 15, 20, 20, 20};
BM_ALARM_MODE braumeister[] = {BM_ALARM_AUS, BM_ALARM_AUS, BM_ALARM_AUS, BM_ALARM_AUS, BM_ALARM_SIGNAL, BM_ALARM_AUS,
                               BM_ALARM_AUS, BM_ALARM_AUS};
int endtemp = 78;
int kochzeit = 90;
int hopfenanzahl = 2;
int hopfenZeit[] = {0, 10, 80, 80, 80, 40, 40};

int timer = 10;
float isttemp = 20;
bool heizung = false;

int x = 1;                                            //aktuelle Rast Nummer

void setup() {
#ifdef DEBUG
    Serial.begin(115200);
    Serial.println("BK Start");
#endif

    lcd_init();

    print_lcdP(PSTR("BK V2.6 - LC2004"), LEFT, 0);
    print_lcdP(PSTR(""), LEFT, 1);
    print_lcdP(PSTR(":)"), RIGHT, 2);
    print_lcdP(PSTR("by realholgi"), LEFT, 3);
    delay(500);

    drehen = sollwert;

    pinMode(heizungPin, OUTPUT);
    pinMode(beeperPin, OUTPUT);

    heizungOnOff(false);
    beeperOnOff(false);

    encoder1.setButtonHeldEnabled(true);
    encoder1.setDoubleClickEnabled(false);
#ifndef DEBUG
    for (x = 1; x <= 3; x++) {
      beeperOnOff(true);
      delay(200);
      beeperOnOff(false);
      delay(200);
    }
#endif

    x = 1;
    lcd_clear();

    temperatureSetup();

    persistenceSetup();
    readEepromData();
    loadSettings(settings);

    watchdogSetup();

    setupWIFI();
    setupWebserver();
    mqttSetup();

    ticker.attach_ms(1, encoderTicker);
}

//loop=============================================================
void loop() {
    handle_http();
    mqttLoop();

    sekunden = second();
    minutenwert = minute();
    stunden = hour();

    readTemperature();

    // Sensorfehler -------------------------------------------------
    // Sensorfehler -127 => VCC fehlt
    // Sensorfehler 85.00 => interner Sensorfehler ggf. Leitung zu lang
    //                       nicht aktiviert
    // Sensorfehler 0.00 => Datenleitung oder GND fehlt

    if (regelung == REGL_MAISCHEN) {
        if (static_cast<int>(isttemp) == -127 || static_cast<int>(isttemp) == 0) {
            if (!sensorfehler) {
                rufmodus = modus;
                print_lcdP(PSTR("Sensorfehler"), RIGHT, 2);
                regelung = REGL_AUS;
                heizung = false;
                sensorfehler = true;
                modus = BRAUMEISTERRUFALARM;
            }
        } else {
            sensorfehler = false;
        }
    }

    // Temperaturanzeige Istwert
    if (!sensorfehler) {
        print_lcdP(PSTR("ist "), 10, 3);
        printNumF_lcd(isttemp, 15, 3);
        print_lcd_deg(19, 3);
    } else {
        print_lcdP(PSTR("   ERR"), RIGHT, 3);
    }

    // Heizregelung
    if (regelung == REGL_MAISCHEN) {
        print_lcdP(PSTR("soll "), 9, 1);
        printNumF_lcd(sollwert, 15, 1);
        print_lcd_deg(19, 1);

        /*
          Regelung beim Hochfahren: Heizung schaltet 0,5°C vor Sollwert aus
          nach einer Wartezeit schaltet es dann um auf hysteresefreie Regelung
          beim Umschalten zwischen ein und aus,
          D.h. nach dem Umschalten ist ein weiteres Schalten für 1 min gesperrt.
          Ausnahme ist die Überschreitung des Sollwertes um 0,5°C.
          Dann wird sofort ausgschaltet.
          Es soll dadurch das Relaisrattern durch Springen
          der Temperatur am Schaltpunkt verhindern.
        */

        // setzt Hysterese beim Hochfahren auf 0.5°C unter sollwert
        if ((isttemp <= (sollwert - 4)) && (heizung == 1)) {
            hysterese = hysteresespeicher;
            hysterese = hysterese / 10;
        }

        // Ausschalten wenn Sollwert-Hysterese erreicht und dann Wartezeit
        if (heizung && (isttemp >= (sollwert - hysterese)) && (millis() >= (wartezeit + 60000))) {
            heizung = false;
            hysterese = 0;
            wartezeit = millis();
        }

        // Einschalten wenn kleiner Sollwert und dann Wartezeit
        if ((!heizung) && (isttemp <= (sollwert - 0.5)) && (millis() >= (wartezeit + 60000))) {
            heizung = true;
            hysterese = 0;
            wartezeit = millis();
        }

        // Ausschalten vor der Wartezeit, wenn Sollwert um 0,5 überschritten
        if (heizung && (isttemp >= (sollwert + 0.5))) {
            heizung = false;
            hysterese = 0;
            wartezeit = millis();
        }

        // Wenn 3 Grad unter soll, dann alles egal und Heizung an
        if ((!heizung) && (isttemp <= (sollwert - 3))) {
            heizung = true;
            hysterese = 0;
            wartezeit = millis();
        }
    }

    //Kochen => dauernd ein----------------------------------------------
    if (regelung == REGL_KOCHEN) {
        heizung = true;
    }

    if (heizung && regelung != REGL_AUS) {
        print_lcdP(PSTR("H"), LEFT, 3);
    } else {
        print_lcdP(PSTR(" "), LEFT, 3);
    }

    heizungOnOff(heizung);

#ifdef DEBUG
    if (millis() >= (serwartezeit + 1000)) {
        Serial.print(millis());
        Serial.print("\t");
        Serial.print(isttemp);
        Serial.println("");
        serwartezeit = millis();
    }
#endif

    getButton();

    stateMachine();

    wdt_reset();
}

void watchdogSetup() {
    wdt_enable(WDTO_2S);
}

extern "C" void custom_crash_callback(struct rst_info *, uint32_t, uint32_t) {
    heizungOnOff(false);
    beeperOnOff(true); // beeeeeeeeeeep
    while (true);
}
