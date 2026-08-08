
#include <Arduino.h>
#include <TimeLib.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

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
#include "recipe.h"
#include "build_info.h"
#include "temperature_control.h"
#include "screens.h"

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
static TemperatureControlState temperatureControl;
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
int maischtemp = 68;
int rasten = 1;
int rastTemp[8] = {0, 66};   // nur Index 1 = Standardrezept; 2..7 füllen Presets/Import
int rastZeit[8] = {0, 60};
BM_ALARM_MODE braumeister[8] = {};   // alle BM_ALARM_AUS; pro Rast im Edit-Screen gesetzt
int endtemp = 78;
int kochzeit = 60;
int hopfenanzahl = 2;
int hopfenZeit[] = {0, 10, 50, 80, 80, 40, 40};
char recipeName[40] = "Standardrezept";

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

    char splashTitle[DISPLAY_SIZE_X + 1];
    snprintf(splashTitle, sizeof(splashTitle), "BrauKnecht %s", firmwareVersion());
    print_lcd(splashTitle, LEFT, 0);
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

    Recipe rcp;
    if (loadRecipe(rcp)) {
        applyRecipe(rcp);
    }

    watchdogSetup();

    setupWIFI();

    // OTA over WiFi (flash via `pio run -e d1_mini_ota -t upload`). Pause the
    // timer1 encoder ISR while flashing so it can't run library code from flash
    // mid-erase. mDNS is already up (setupWIFI), so just advertise the service.
    ArduinoOTA.setHostname("bk");
    ArduinoOTA.onStart([]() { encoderTimerPause(); });
    ArduinoOTA.begin(false);
    MDNS.enableArduino(8266);

    setupWebserver();
    mqttSetup();

    // Service the encoder from hardware timer1 (a real interrupt). The previous
    // Ticker/os_timer runs in the SDK task context and gets starved by WiFi
    // traffic, which made the encoder stutter and skip once STA + MQTT were on.
    encoderTimerSetup();
}

//loop=============================================================
void loop() {
    ArduinoOTA.handle();
    serviceWiFiAp();
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
                enterBraumeisterRufalarm();
            }
        } else {
            sensorfehler = false;
        }
    }

    // Temperaturanzeige Istwert — nur neu zeichnen bei Wertänderung (0.1°C) oder
    // nach einem Clear, sonst flackert die Zeile durch dauerndes Neuschreiben.
    {
        static int lastShown = -100000;
        int shown = sensorfehler ? -99999 : static_cast<int>(isttemp * 10);
        if (overlaysDirty || shown != lastShown) {
            lastShown = shown;
            if (!sensorfehler) {
                print_lcdP(PSTR("ist "), 10, 3);
                printNumF_lcd(isttemp, 15, 3);
                print_lcd_deg(19, 3);
            } else {
                print_lcdP(PSTR("   ERR"), RIGHT, 3);
            }
        }
    }

    // Heizregelung
    if (regelung == REGL_MAISCHEN && !sensorfehler) {
        static int lastSoll = -1000;
        if (overlaysDirty || sollwert != lastSoll) {
            lastSoll = sollwert;
            print_lcdP(PSTR("soll "), 9, 1);
            printNumF_lcd(sollwert, 15, 1);
            print_lcd_deg(19, 1);
        }

        heizung = updateTemperatureControl(
            temperatureControl, isttemp, sollwert, static_cast<uint32_t>(millis()));
    } else if (regelung == REGL_KOCHEN) {
        resetTemperatureControl(temperatureControl);
        heizung = true;
    } else {
        resetTemperatureControl(temperatureControl);
        heizung = false;
    }

    // Heizanzeige nur beim Brauen; sonst gehört Zeile 3 / Spalte 0 dem
    // Menücursor bzw. dem Screen (sonst flackert z. B. das ">" bei "Setup").
    if (regelung != REGL_AUS) {
        print_lcdP(heizung ? PSTR("H") : PSTR(" "), LEFT, 3);
    }
    overlaysDirty = false;  // temp/soll haben das Flag gelesen

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
