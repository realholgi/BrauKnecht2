#pragma GCC diagnostic ignored "-Wwrite-strings"

#include <Arduino.h>
#include <Encoder.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TimeLib.h>
#include <EEPROM.h>

#include "global.h"
#include "config.h"
#include "display.h"
#include "web.h"
#include "proto.h"

OneWire oneWire(oneWirePin);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;
Encoder encoder(encoderPinA, encoderPinB);

#ifdef DEBUG
unsigned long serwartezeit = 0;
#endif

volatile int oldnumber = 0;
bool ButtonPressed = false;
int drehen = 0;
int fuenfmindrehen = 0;
bool anfang = true;
unsigned long altsekunden;
REGEL_MODE regelung = REGL_AUS;
bool sensorfehler = false;
float hysterese;
byte hysteresespeicher = HYSTERESE_DEFAULT;
long wartezeit = -60000;
float sensorwert;
unsigned long rufsignalzeit = 0;
int y = 1;                                            //Übergabewert von x für Braumeisterruf
int n = 0;                                            //Counter Messungserhöhung zur Fehlervermeidung
int pause = 0;
unsigned long abbruchtaste;
byte kschwelle = KOCHSCHWELLE_DEFAULT;
bool einmaldruck = false;

// -------------- external start
int sekunden = 0;
int minuten = 0;
int hendi_on_start = 0;
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

// -------------- external end

void hendi_special_handling();

void setup() {
#ifdef DEBUG
    Serial.begin(115200);
    Serial.println("BK Start");
#endif

    lcd_init();

    print_lcdP(PSTR("BK V2.5 - LC2004"), LEFT, 0);
    print_lcdP(PSTR("Arduino"), LEFT, 1);
    print_lcdP(PSTR(":)"), RIGHT, 2);
    print_lcdP(PSTR("realholgi & fg100"), RIGHT, 3);
    delay(500);

    drehen = sollwert;

    pinMode(heizungPin, OUTPUT);
    pinMode(beeperPin, OUTPUT);

    heizungOnOff(false);
    beeperOnOff(false);

    pinMode(tasterPin, INPUT_PULLUP);
    digitalWrite(tasterPin, HIGH);  // Turn on internal pullup resistor

#ifndef DEBUG
    for (x = 1; x <= 3; x++) {
      beeperOn(true);
      delay(200);
      beeperOn(false);
      delay(200);
    }
#endif

    x = 1;
    lcd_clear();

    sensors.getAddress(insideThermometer, 0);
    sensors.setResolution(insideThermometer, 12);   // set the resolution to 9 bit

    readEepromData();

    watchdogSetup();

    setupWebserver();
    setupWIFI();
}

//loop=============================================================
void loop() {
    handle_http();

    sekunden = second();
    minutenwert = minute();
    stunden = hour();

    long number = encoder.read();
    if (number != oldnumber) {
        {
            if (number > oldnumber) { // < > Zeichen ändern = Encoderdrehrichtung ändern
                ++drehen;
                fuenfmindrehen = fuenfmindrehen + 5;
            } else {
                --drehen;
                fuenfmindrehen = fuenfmindrehen - 5;
            }
            delay(100); // entprellen
            oldnumber = encoder.read();
        }
    }

    sensors.requestTemperatures();
    sensorwert = sensors.getTempC(insideThermometer);
    if ((sensorwert != isttemp) && (n < 5)) { // Messfehlervermeidung des Sensorwertes
        n++;
    } else {
        isttemp = sensorwert;
        n = 0;
    }


    // Sensorfehler -------------------------------------------------
    // Sensorfehler -127 => VCC fehlt
    // Sensorfehler 85.00 => interner Sensorfehler ggf. Leitung zu lang
    //                       nicht aktiviert
    // Sensorfehler 0.00 => Datenleitung oder GND fehlt

    if (regelung == REGL_MAISCHEN) {
        if ((int) isttemp == -127 || (int) isttemp == 0) {
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

void stateMachine() {
    switch (modus) {
        case HAUPTSCHIRM:
            regelung = REGL_AUS;
            funktion_hauptschirm();
            break;

        case MANUELL:
            regelung = REGL_MAISCHEN;
            funktion_temperatur();
            break;

        case MAISCHEN:
            regelung = REGL_AUS;
            funktion_maischmenue();
            break;

        case SETUP_MENU:
            funktion_setupmenu();
            break;

        case SETUP_HYSTERESE:
            funktion_hysterese();
            break;

        case SETUP_KOCHSCHWELLE:
            funktion_kochschwelle();
            break;

        case ALARMTEST:
            regelung = REGL_AUS;
            rufmodus = HAUPTSCHIRM;
            modus = BRAUMEISTERRUFALARM;
            print_lcdP(PSTR("Alarmtest"), RIGHT, 0);
            break;

        case EINGABE_RAST_ANZ:
            regelung = REGL_AUS;
            funktion_rastanzahl();
            break;

        case EINGABE_MAISCHTEMP:
            regelung = REGL_AUS;
            funktion_maischtemperatur();
            break;

        case EINGABE_RAST_TEMP:
            regelung = REGL_AUS;
            funktion_rasteingabe();
            break;

        case EINGABE_RAST_ZEIT:
            regelung = REGL_AUS;
            funktion_zeiteingabe();
            break;

        case EINGABE_BRAUMEISTERRUF:
            regelung = REGL_AUS;
            funktion_braumeister();
            break;

        case EINGABE_ENDTEMP:
            regelung = REGL_AUS;
            funktion_endtempeingabe();
            break;

        case AUTO_START:
            regelung = REGL_AUS;
            funktion_startabfrage(AUTO_MAISCHTEMP, "Auto");
            break;

        case AUTO_MAISCHTEMP:
            regelung = REGL_MAISCHEN;;
            funktion_maischtemperaturautomatik();
            break;

        case AUTO_RAST_TEMP:
            regelung = REGL_MAISCHEN;
            funktion_tempautomatik();
            break;

        case AUTO_RAST_ZEIT:
            regelung = REGL_MAISCHEN;
            funktion_zeitautomatik();
            break;

        case AUTO_ENDTEMP:
            regelung = REGL_MAISCHEN;
            funktion_endtempautomatik();
            break;

        case BRAUMEISTERRUFALARM:
            funktion_braumeisterrufalarm();
            break;

        case BRAUMEISTERRUF:
            funktion_braumeisterruf();
            break;

        case KOCHEN:
            funktion_kochzeit();
            break;

        case EINGABE_HOPFENGABEN_ANZAHL:
            funktion_anzahlhopfengaben();
            break;

        case EINGABE_HOPFENGABEN_ZEIT:
            funktion_hopfengaben();
            break;

        case KOCHEN_START_FRAGE:
            funktion_startabfrage(KOCHEN_AUFHEIZEN, "Kochen");
            break;

        case KOCHEN_AUFHEIZEN:
            regelung = REGL_KOCHEN;
            funktion_kochenaufheizen();
            break;

        case KOCHEN_AUTO_LAUF:
            regelung = REGL_KOCHEN;
            funktion_hopfenzeitautomatik();
            break;

        case ABBRUCH:
            funktion_abbruch();
            break;
    }
}

bool getButton() {
    int buttonVoltage = digitalRead(tasterPin);
    if (buttonVoltage == HIGH) {
        ButtonPressed = false;
        abbruchtaste = millis();
    } else if (buttonVoltage == LOW) {
        ButtonPressed = true;
        if (millis() >= (abbruchtaste + 2000)) {     //Taste 2 Sekunden drücken
            modus = ABBRUCH;
        }
    }
    return ButtonPressed;
}

bool warte_und_weiter(MODUS naechsterModus) {
    if (!ButtonPressed) {
        einmaldruck = true;
    }
    if (einmaldruck && ButtonPressed) {
        einmaldruck = false;
        modus = naechsterModus;
        anfang = true;
        return true;
    }
    return false;
}

void menu_zeiger(int pos) {
    int p;
    for (p = 0; p <= 3; p++) {
        if (p == pos) {
            print_lcdP(PSTR("=>"), LEFT, p);
        } else {
            print_lcdP(PSTR("  "), LEFT, p);
        }
    }
}

void funktion_hauptschirm() {
    if (anfang) {
        lcd_clear();
        drehen = 0;
        anfang = false;
        print_lcdP(PSTR("Maischen"), 2, 0);
        print_lcdP(PSTR("Kochen"), 2, 1);
        print_lcdP(PSTR("       "), 2, 2);
        print_lcdP(PSTR("Setup"), 2, 3);
    }

    drehen = constrain(drehen, 0, 3);

    menu_zeiger(drehen);
    switch (drehen) {
        case 0:
            rufmodus = MAISCHEN;
            break;
        case 1:
            rufmodus = KOCHEN;
            break;
        case 3:
            rufmodus = SETUP_MENU;
            break;
        default:
            rufmodus = ABBRUCH;
            break;
    }

    if (warte_und_weiter(rufmodus)) {
        lcd_clear();
    }
}

void funktion_maischmenue() {
    if (anfang) {
        lcd_clear();
        drehen = 0;
        anfang = false;
        print_lcdP(PSTR("Automatik"), 2, 0);
        print_lcdP(PSTR("Manuell"), 2, 1);
    }

    drehen = constrain(drehen, 0, 1);

    menu_zeiger(drehen);
    switch (drehen) {
        case 0:
            rufmodus = AUTOMATIK;
            break;
        case 1:
            rufmodus = MANUELL;
            break;

        default:
            rufmodus = ABBRUCH;
            break;
    }

    if (warte_und_weiter(rufmodus)) {
        if (modus == MANUELL) {
            //Übergabe an Modus1
            drehen = (int) isttemp + 10; // Vorgabewert 10°C über IstWert
        }
    }
}

void funktion_setupmenu() {
    if (anfang) {
        lcd_clear();
        drehen = 0;
        anfang = false;
        print_lcdP(PSTR("Kochschwelle"), 2, 0);
        print_lcdP(PSTR("Hysterese"), 2, 1);
    }

    drehen = constrain(drehen, 0, 1);

    menu_zeiger(drehen);
    switch (drehen) {
        case 0:
            rufmodus = SETUP_KOCHSCHWELLE;
            break;
        case 1:
            rufmodus = SETUP_HYSTERESE;
            break;
        default:
            rufmodus = ABBRUCH;
            break;
    }

    if (warte_und_weiter(rufmodus)) {
        lcd_clear();
    }
}

void funktion_temperatur() {
    if (anfang) {
        lcd_clear();
        anfang = false;
    }

    sollwert = drehen;
    switch (modus) {
        case MANUELL:
            print_lcdP(PSTR("Manuell"), LEFT, 0);
            break;

        default:
            break;
    }

    if (modus == MANUELL && isttemp >= sollwert) { // Manuell -> Sollwert erreicht
        rufmodus = MANUELL;                //Abbruch nach Rufalarm
        modus = BRAUMEISTERRUFALARM;
        regelung = REGL_AUS;
        heizung = false;
        y = 0;
        braumeister[y] = BM_ALARM_SIGNAL;
    }
}

void funktion_rastanzahl() {
    if (anfang) {
        lcd_clear();
        drehen = rasten;
        anfang = false;
        print_lcdP(PSTR("Auto"), LEFT, 0);
        print_lcdP(PSTR("Eingabe"), RIGHT, 0);
        print_lcdP(PSTR("Rasten"), LEFT, 1);
    }

    drehen = constrain(drehen, 1, 5);
    rasten = drehen;

    switch (drehen) {
        case 1:
            rastTemp[1] = 67;
            rastZeit[1] = 60;
            maischtemp = 65;
            break;

        case 2:
            rastTemp[1] = 62;
            rastZeit[1] = 30;
            rastTemp[2] = 72;
            rastZeit[2] = 35;
            maischtemp = 55;
            break;

        case 3:
            rastTemp[1] = 43;
            rastZeit[1] = 20;
            rastTemp[2] = 62;
            rastZeit[2] = 30;
            rastTemp[3] = 72;
            rastZeit[3] = 30;
            maischtemp = 45;
            break;

        case 4:
            rastTemp[1] = 45;
            rastZeit[1] = 10;
            rastTemp[2] = 52;
            rastZeit[2] = 10;
            rastTemp[3] = 65;
            rastZeit[3] = 30;
            rastTemp[4] = 72;
            rastZeit[4] = 30;
            maischtemp = 37;
            break;

        case 5:
            rastTemp[1] = 35;
            rastZeit[1] = 20;
            rastTemp[2] = 40;
            rastZeit[2] = 20;
            rastTemp[3] = 55;
            rastZeit[3] = 15;
            rastTemp[4] = 64;
            rastZeit[4] = 35;
            rastTemp[5] = 72;
            rastZeit[5] = 25;
            maischtemp = 30;
            break;

        default:;
    }

    printNumI_lcd(rasten, 19, 1);

    warte_und_weiter(EINGABE_MAISCHTEMP);
}

void funktion_maischtemperatur() {
    if (anfang) {
        lcd_clear();
        drehen = maischtemp;
        anfang = false;
        print_lcdP(PSTR("Auto"), LEFT, 0);
        print_lcdP(PSTR("Eingabe"), RIGHT, 0);
    }

    drehen = constrain(drehen, 10, 105);
    maischtemp = drehen;

    print_lcdP(PSTR("Maischtemp"), LEFT, 1);
    printNumF_lcd(maischtemp, 15, 1);
    print_lcd_deg(19, 1);

    warte_und_weiter(EINGABE_RAST_TEMP);
}

void funktion_rasteingabe() {
    if (anfang) {
        lcd_clear();
        drehen = rastTemp[x];
        anfang = false;
        print_lcdP(PSTR("Auto"), LEFT, 0);
        print_lcdP(PSTR("Eingabe"), RIGHT, 0);
    }

    drehen = constrain(drehen, 9, 105);
    rastTemp[x] = drehen;

    printNumI_lcd(x, LEFT, 1);
    print_lcdP(PSTR(". Rast"), 1, 1);
    printNumF_lcd(rastTemp[x], 15, 1);
    print_lcd_deg(19, 1);

    warte_und_weiter(EINGABE_RAST_ZEIT);
}

void funktion_zeiteingabe() {
    if (anfang) {
        fuenfmindrehen = rastZeit[x];
        anfang = false;
    }

    fuenfmindrehen = constrain(fuenfmindrehen, 1, 99);
    rastZeit[x] = fuenfmindrehen;

    print_lcd_minutes(rastZeit[x], RIGHT, 2);

    warte_und_weiter(EINGABE_BRAUMEISTERRUF);
}

void funktion_braumeister() {
    if (anfang) {
        drehen = (int) braumeister[x];
        anfang = false;
    }

    drehen = constrain(drehen, BM_ALARM_MIN, BM_ALARM_MAX);
    braumeister[x] = (BM_ALARM_MODE) drehen;

    print_lcdP(PSTR("Ruf"), LEFT, 2);

    switch (braumeister[x]) {
        case BM_ALARM_AUS:
            print_lcdP(PSTR("    Nein"), RIGHT, 2);
            break;

        case BM_ALARM_WAIT:
            print_lcdP(PSTR("Anhalten"), RIGHT, 2);
            break;

        case BM_ALARM_SIGNAL:
            print_lcdP(PSTR("  Signal"), RIGHT, 2);
            break;
    }

    if (warte_und_weiter(EINGABE_ENDTEMP)) {
        if (x < rasten) {
            x++;
            modus = EINGABE_RAST_TEMP; // Sprung zur Rasttemperatureingabe
        } else {
            x = 1;
            modus = EINGABE_ENDTEMP; // Sprung zur Rastzeiteingabe
        }
    }
}

void funktion_endtempeingabe() {
    if (anfang) {
        lcd_clear();
        drehen = endtemp;
        anfang = false;
        print_lcdP(PSTR("Auto"), LEFT, 0);
        print_lcdP(PSTR("Eingabe"), RIGHT, 0);
    }

    drehen = constrain(drehen, 10, 80);
    endtemp = drehen;

    print_lcdP(PSTR("Endtemperatur"), LEFT, 1);
    printNumF_lcd(endtemp, 15, 1);
    print_lcd_deg(19, 1);

    warte_und_weiter(AUTO_START);
}

void funktion_startabfrage(MODUS naechsterModus, char *title) {
    if (anfang) {
        lcd_clear();
        print_lcd(title, LEFT, 0);
        anfang = false;
        altsekunden = millis();
    }

    if (millis() >= (altsekunden + 1000)) {
        print_lcdP(PSTR("       "), CENTER, 2);
        if (millis() >= (altsekunden + 1500)) {
            altsekunden = millis();
        }
    } else {
        print_lcdP(PSTR("Start ?"), CENTER, 2);
    }

    warte_und_weiter(naechsterModus);
}

void funktion_maischtemperaturautomatik() {
    if (anfang) {
        lcd_clear();
        drehen = maischtemp;
        anfang = false;
        print_lcdP(PSTR("Auto"), LEFT, 0);
        print_lcdP(PSTR("Maischen"), RIGHT, 0);
    }

    drehen = constrain(drehen, 10, 105);
    maischtemp = drehen;
    sollwert = maischtemp;

    if (isttemp >= sollwert) {
        rufmodus = AUTO_RAST_TEMP;
        y = 0;
        braumeister[y] = BM_ALARM_WAIT;
        modus = BRAUMEISTERRUFALARM;
    }
}

void funktion_tempautomatik() {
    if (anfang) {
        lcd_clear();
        drehen = rastTemp[x];
        anfang = false;
        print_lcdP(PSTR("Auto"), LEFT, 0);
        printNumI_lcd(x, 13, 0);
        print_lcdP(PSTR(". Rast"), RIGHT, 0);

        heizung = true;
        //wartezeit = millis() + 60000;  // sofort aufheizen
    }

    drehen = constrain(drehen, 10, 105);

    rastTemp[x] = drehen;
    sollwert = rastTemp[x];

    if (isttemp >= sollwert) {
        modus = AUTO_RAST_ZEIT;
        anfang = true;
    }
}

void funktion_zeitautomatik() {
    if (anfang) {
        drehen = rastZeit[x];
        //wartezeit = millis() + 60000;  // sofort aufheizen
        heizung = true;
    }

    print_lcd_minutes(rastZeit[x], RIGHT, 2);

    // Zeitzählung---------------
    if (anfang) {
        print_lcdP(PSTR("Set Time"), LEFT, 3);
        setTime(00, 00, 00, 00, 01, 01); // Sekunden auf 0 stellen
        delay(400); //test

        sekunden = second();
        minutenwert = minute();
        stunden = hour();

        print_lcdP(PSTR("            "), 0, 3);
        anfang = false;
        print_lcdP(PSTR("00:00"), LEFT, 2);
    }

    if (sekunden < 10) {
        printNumI_lcd(sekunden, 4, 2);
        if (sekunden == 0) {
            print_lcdP(PSTR("0"), 3, 2);
        }
    } else {
        printNumI_lcd(sekunden, 3, 2);
    }

    minuten = (stunden * 60) + minutenwert;

    if (minuten < 10) {
        printNumI_lcd(minuten, 1, 2);
    } else {
        printNumI_lcd(minuten, 0, 2);
    }
    // Ende Zeitzählung---------------------

    drehen = constrain(drehen, 10, 105);
    rastZeit[x] = drehen; // Encoderzuordnung

    if (minuten >= rastZeit[x]) {
        anfang = true;
        y = x;
        if (x < rasten) {
            modus = AUTO_RAST_TEMP;
            x++;
        } else {
            x = 1; // Endtemperatur
            modus = AUTO_ENDTEMP;
        }

        if (braumeister[y] != BM_ALARM_AUS) {
            rufmodus = modus;
            modus = BRAUMEISTERRUFALARM;
        }
    }
}

void funktion_endtempautomatik() {
    if (anfang) {
        lcd_clear();
        anfang = false;
        drehen = endtemp;    // Zuordnung Encoder
        print_lcdP(PSTR("Auto"), LEFT, 0);
        print_lcdP(PSTR("Endtemp"), RIGHT, 0);

        //wartezeit = millis() + 60000;  // sofort aufheizen
        heizung = true;
    }

    drehen = constrain(drehen, 10, 105);
    endtemp = drehen;
    sollwert = endtemp;

    if (isttemp >= sollwert) {
        rufmodus = ABBRUCH;
        modus = BRAUMEISTERRUFALARM;
        regelung = REGL_AUS;
        heizung = false;
        y = 0;
        braumeister[y] = BM_ALARM_WAIT;
    }
}

void funktion_braumeisterrufalarm() {
    if (anfang) {
        rufsignalzeit = millis();
        anfang = false;
    }

    if (millis() >= (altsekunden + 1000)) { //Bliken der Anzeige und RUF
        print_lcdP(PSTR("          "), LEFT, 3);
        beeperOnOff(false);
        if (millis() >= (altsekunden + 1500)) {
            altsekunden = millis();
            pause++;
        }
    } else {
        print_lcdP(PSTR("RUF"), LEFT, 3);
        if (pause <= 4) {
            beeperOnOff(true);
        }
        if (pause > 8) {
            pause = 0;
        }
    }

    // 20 Sekunden Rufsignalisierung wenn "Ruf Signal"
    if (braumeister[y] == BM_ALARM_SIGNAL && millis() >= (rufsignalzeit + 20000)) {
        anfang = true;
        pause = 0;
        beeperOnOff(false);
        modus = rufmodus;
        einmaldruck = false;
    }

    if (warte_und_weiter(BRAUMEISTERRUF)) {
        pause = 0;
        beeperOnOff(false);
        if (braumeister[y] == BM_ALARM_SIGNAL) {
            print_lcdP(PSTR("   "), LEFT, 3);
            modus = rufmodus;
        }
    }
}

void funktion_braumeisterruf() {
    if (anfang) {
        anfang = false;
    }

    if (millis() >= (altsekunden + 1000)) {
        print_lcdP(PSTR("        "), LEFT, 3);
        if (millis() >= (altsekunden + 1500)) {
            altsekunden = millis();
        }
    } else {
        print_lcdP(PSTR("weiter ?"), LEFT, 3);
    }

    if (warte_und_weiter(rufmodus)) {
        print_lcdP(PSTR("        "), LEFT, 3);     // Text "weiter ?" löschen
        print_lcdP(PSTR("             "), RIGHT, 3); // Löscht Text bei Sensorfehler oder Alarmtest
        sensorfehler = false;
        delay(500);     // kurze Wartezeit, damit nicht durch unbeabsichtigtes Drehen der nächste Vorgabewert verstellt wird
    }
}

void funktion_hysterese() {
    if (anfang) {
        lcd_clear();
        anfang = false;
        print_lcdP(PSTR("Hysterese"), LEFT, 0);
        print_lcdP(PSTR("Eingabe"), RIGHT, 0);

        fuenfmindrehen = hysteresespeicher;
    }

    fuenfmindrehen = constrain(fuenfmindrehen, 0, 40); //max. 4,0 Sekunden Hysterese
    hysteresespeicher = static_cast<byte>(fuenfmindrehen);

    printNumF_lcd(float(hysteresespeicher) / 10, RIGHT, 1);

    if (warte_und_weiter(SETUP_MENU)) {
        hysterese = hysteresespeicher / 10;
        writeEepromData();
    }
}

void funktion_kochschwelle() {
    if (anfang) {
        lcd_clear();
        drehen = kschwelle;
        anfang = false;
        print_lcdP(PSTR("Kochschwelle"), LEFT, 0);
        print_lcdP(PSTR("Eingabe"), RIGHT, 0);
    }

    drehen = constrain(drehen, 20, 99);
    kschwelle = static_cast<byte>(drehen);

    printNumI_lcd(kschwelle, RIGHT, 1);

    if (warte_und_weiter(SETUP_MENU)) {
        writeEepromData();
    }
}

void funktion_kochzeit() {
    if (anfang) {
        lcd_clear();
        fuenfmindrehen = kochzeit;
        anfang = false;
        print_lcdP(PSTR("Kochen"), LEFT, 0);
        print_lcdP(PSTR("Eingabe"), RIGHT, 0);
        print_lcdP(PSTR("Zeit"), LEFT, 1);
    }

    fuenfmindrehen = constrain(fuenfmindrehen, 20, 180);
    kochzeit = fuenfmindrehen;

    print_lcd_minutes(kochzeit, RIGHT, 1);

    warte_und_weiter(EINGABE_HOPFENGABEN_ANZAHL);
}

void funktion_anzahlhopfengaben() {
    if (anfang) {
        lcd_clear();
        drehen = hopfenanzahl;
        anfang = false;
        print_lcdP(PSTR("Kochen"), LEFT, 0);
        print_lcdP(PSTR("Eingabe"), RIGHT, 0);
        print_lcdP(PSTR("Hopfengaben"), LEFT, 1);
    }

    drehen = constrain(drehen, 1, 5);
    hopfenanzahl = drehen;

    printNumI_lcd(hopfenanzahl, RIGHT, 1);

    warte_und_weiter(EINGABE_HOPFENGABEN_ZEIT);
}

void funktion_hopfengaben() {
    if (anfang) {
        x = 1;
        fuenfmindrehen = hopfenZeit[x];
        anfang = false;
        lcd_clear();
        print_lcdP(PSTR("Kochen"), LEFT, 0);
        print_lcdP(PSTR("Eingabe"), RIGHT, 0);
    }

    printNumI_lcd(x, LEFT, 1);
    print_lcdP(PSTR(". Hopfengabe"), 1, 1);
    print_lcdP(PSTR("nach"), LEFT, 2);

    fuenfmindrehen = constrain(fuenfmindrehen, hopfenZeit[(x - 1)] + 5, kochzeit - 5);
    hopfenZeit[x] = fuenfmindrehen;

    print_lcd_minutes(hopfenZeit[x], RIGHT, 2);

    if (warte_und_weiter(modus)) {
        if (x < hopfenanzahl) {
            x++;
            fuenfmindrehen = hopfenZeit[x];
            print_lcdP(PSTR("  "), LEFT, 1);
            print_lcdP(PSTR("   "), 13, 2);
            delay(400);
            anfang = false; // nicht auf Anfang zurück
        } else {
            x = 1;
            modus = KOCHEN_START_FRAGE;
        }
    }
}

void funktion_kochenaufheizen() {
    if (anfang) {
        lcd_clear();
        print_lcdP(PSTR("Kochen"), LEFT, 0);
        anfang = false;
    }

    sollwert = kschwelle;
    if (isttemp >= sollwert) {
        print_lcdP(PSTR("            "), RIGHT, 0);
        print_lcdP(PSTR("Kochbeginn"), CENTER, 1);
        beeperOnOff(true);
        delay(500);
        beeperOnOff(false);
        anfang = true;
        modus = KOCHEN_AUTO_LAUF;
    } else {
        print_lcdP(PSTR("Aufheizen"), RIGHT, 0);
    }
}

void funktion_hopfenzeitautomatik() {
    if (anfang) {
        x = 1;
        lcd_clear();
        print_lcdP(PSTR("Kochen"), LEFT, 0);
        setTime(00, 00, 00, 00, 01, 01); //.........Sekunden auf 0 stellen
        print_lcd_minutes(kochzeit, RIGHT, 0);

        sekunden = second();
        minutenwert = minute();
        stunden = hour();


        print_lcdP(PSTR("00:00"), 11, 1);
    }

    if (x <= hopfenanzahl) {
        printNumI_lcd(x, LEFT, 2);
        print_lcdP(PSTR(". Gabe bei "), 1, 2);
        print_lcd_minutes(hopfenZeit[x], RIGHT, 2);
    } else {
        print_lcdP(PSTR("                    "), 0, 2);
    }

    print_lcdP(PSTR("min"), RIGHT, 1);

    if (sekunden < 10) {
        printNumI_lcd(sekunden, 15, 1);
        if (sekunden) {
            print_lcdP(PSTR("0"), 14, 1);
        }
    } else {
        printNumI_lcd(sekunden, 14, 1);
    }

    minuten = (stunden * 60) + minutenwert;
    if (minuten < 10) {
        printNumI_lcd(minuten, 12, 1);
    }

    if ((minuten >= 10) && (minuten < 100)) {
        printNumI_lcd(minuten, 11, 1);
    }

    if (minuten >= 100) {
        printNumI_lcd(minuten, 10, 1);
    }

    if ((minuten == hopfenZeit[x]) && (x <= hopfenanzahl)) {  // Hopfengabe
        //Alarm -----
        if (millis() >= (altsekunden + 1000)) { //Blinken der Anzeige und RUF
            print_lcdP(PSTR("   "), LEFT, 3);
            beeperOnOff(false);
            if (millis() >= (altsekunden + 1500)) {
                altsekunden = millis();
                pause++;
            }
        } else {
            print_lcdP(PSTR("RUF"), LEFT, 3);
            if (pause <= 4) {
                beeperOnOff(true);
            }
            if (pause > 8) {
                pause = 0;
            }
        }

        if (warte_und_weiter(modus)) {
            anfang = false; // nicht zurücksetzen!!!
            _next_koch_step();
        }
    }

    if ((minuten > hopfenZeit[x]) && (x <= hopfenanzahl)) {  // Alarmende nach 1 Minute
        _next_koch_step();
    }

    if (minuten >= kochzeit) {   //Kochzeitende
        rufmodus = ABBRUCH;
        modus = BRAUMEISTERRUFALARM;
        regelung = REGL_AUS;
        heizung = false;
        y = 0;
        braumeister[y] = BM_ALARM_WAIT;
    }
}

void _next_koch_step() {
    print_lcdP(PSTR("   "), LEFT, 3);
    pause = 0;
    beeperOnOff(false);
    x++;
}

void funktion_abbruch() {
    regelung = REGL_AUS;
    heizung = false;
    wartezeit = -60000;
    heizungOnOff(false);
    beeperOnOff(false);
    anfang = true;
    lcd_clear();
    rufmodus = HAUPTSCHIRM;
    x = 1;
    y = 1;
    n = 0;
    einmaldruck = false;
    pause = 0;
    drehen = sollwert;

    if (millis() >= abbruchtaste + 5000) { // länger als 5 Sekunden drücken
        modus = SETUP_MENU;
    } else {
        modus = HAUPTSCHIRM;
    }
}
//------------------------------------------------------------------

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
void hendi_special_handling() {
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

void readEepromData() {
    EEPROM.begin(512);

    hysteresespeicher = EEPROM.read(HYSTERESE_MEM);
    if (hysteresespeicher > 40 || hysteresespeicher == 0) { hysteresespeicher = HYSTERESE_DEFAULT; };
    hysterese = hysteresespeicher / 10;

    kschwelle = EEPROM.read(KOCHSCHWELLE_MEM);
    if (kschwelle > 100 || kschwelle == 0) { kschwelle = KOCHSCHWELLE_DEFAULT; };
}

void writeEepromData() {
    EEPROM.begin(512);

    EEPROM.write(HYSTERESE_MEM, hysteresespeicher);
    EEPROM.write(KOCHSCHWELLE_MEM, kschwelle);
    EEPROM.commit();
}

void watchdogSetup() {
    wdt_enable(WDTO_2S);
}

extern "C" void custom_crash_callback(struct rst_info *rst_info, uint32_t stack, uint32_t stack_end) {
    heizungOnOff(false);
    beeperOnOff(true); // beeeeeeeeeeep
    while (true);
}
