#include <Arduino.h>

#include "statemachine.h"
#include "global.h"
#include "config.h"
#include "display.h"
#include "input.h"
#include "screens.h"

void stateMachine() {
    encoder1.setAccelerationEnabled(false);
    switch (modus) {
        case HAUPTSCHIRM:
            regelung = REGL_AUS;
            funktion_hauptschirm();
            break;

        case MANUELL:
            regelung = REGL_MAISCHEN;
            encoder1.setAccelerationEnabled(true);
            funktion_temperatur();
            break;


        case SETUP_MENU:
            funktion_setupmenu();
            break;


        case SETUP_KOCHSCHWELLE:
            funktion_kochschwelle();
            break;

        case SETUP_AP:
            funktion_ap();
            break;

        case ALARMTEST:
            regelung = REGL_AUS;
            rufmodus = HAUPTSCHIRM;
            enterBraumeisterRufalarm();
            print_lcdP(PSTR("Alarmtest"), RIGHT, 0);
            break;

        case AUTOMATIK_FRAGE:
            regelung = REGL_AUS;
            funktion_rezeptfrage(PSTR("Maischen"), AUTO_START, EINGABE_RAST_ANZ);
            break;

        case KOCHEN_FRAGE:
            regelung = REGL_AUS;
            funktion_rezeptfrage(PSTR("Kochen"), KOCHEN_START_FRAGE, KOCHEN);
            break;

        case EINGABE_RAST_ANZ:
            regelung = REGL_AUS;
            funktion_rastanzahl();
            break;

        case EINGABE_MAISCHTEMP:
            encoder1.setAccelerationEnabled(true);
            regelung = REGL_AUS;
            funktion_maischtemperatur();
            break;

        case EINGABE_RAST_TEMP:
            regelung = REGL_AUS;
            encoder1.setAccelerationEnabled(true);
            funktion_rasteingabe();
            break;

        case EINGABE_RAST_ZEIT:
            regelung = REGL_AUS;
            encoder1.setAccelerationEnabled(true);
            funktion_zeiteingabe();
            break;

        case EINGABE_ENDTEMP:
            regelung = REGL_AUS;
            encoder1.setAccelerationEnabled(true);
            funktion_endtempeingabe();
            break;

        case AUTO_START:
            regelung = REGL_AUS;
            funktion_startabfrage(AUTO_MAISCHTEMP, "Maischen");
            break;

        case AUTO_MAISCHTEMP:
            regelung = REGL_MAISCHEN;
            encoder1.setAccelerationEnabled(true);
            funktion_maischtemperaturautomatik();
            break;

        case AUTO_RAST_TEMP:
            regelung = REGL_MAISCHEN;
            encoder1.setAccelerationEnabled(true);
            funktion_tempautomatik();
            break;

        case AUTO_RAST_ZEIT:
            regelung = REGL_MAISCHEN;
            encoder1.setAccelerationEnabled(true);
            funktion_zeitautomatik();
            break;

        case AUTO_ENDTEMP:
            regelung = REGL_MAISCHEN;
            encoder1.setAccelerationEnabled(true);
            funktion_endtempautomatik();
            break;

        case BRAUMEISTERRUFALARM:
            funktion_braumeisterrufalarm();
            break;

        case BRAUMEISTERRUF:
            funktion_braumeisterruf();
            break;

        case KOCHEN:
            encoder1.setAccelerationEnabled(true);
            funktion_kochzeit();
            break;

        case EINGABE_HOPFENGABEN_ANZAHL:
            funktion_anzahlhopfengaben();
            break;

        case EINGABE_HOPFENGABEN_ZEIT:
            encoder1.setAccelerationEnabled(true);
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

// Step back one screen in the linear input flow. Mirrors the forward
// transitions in warte_und_weiter()/the screen functions. Called on long-press
// only while not brewing (regelung == REGL_AUS); during a brew the long-press
// keeps its stop-and-abort meaning (see getButton()).
void goBackOneStep() {
    switch (modus) {
        case AUTOMATIK_FRAGE:
        case KOCHEN_FRAGE:
            modus = HAUPTSCHIRM;
            break;

        case EINGABE_RAST_ANZ:
            modus = AUTOMATIK_FRAGE;
            break;
        case EINGABE_MAISCHTEMP:
            modus = EINGABE_RAST_ANZ;
            break;
        case EINGABE_RAST_TEMP:
            if (x > 1) {
                x--;
                modus = EINGABE_RAST_ZEIT;  // back to the previous rest's time
            } else {
                modus = EINGABE_MAISCHTEMP;
            }
            break;
        case EINGABE_RAST_ZEIT:
            modus = EINGABE_RAST_TEMP;       // same rest, back to its temperature
            break;
        case EINGABE_ENDTEMP:
            x = rasten;
            modus = EINGABE_RAST_ZEIT;       // back to the last rest's time
            break;
        case AUTO_START:
            modus = EINGABE_ENDTEMP;
            break;

        case KOCHEN:  // boil-time entry
            modus = KOCHEN_FRAGE;
            break;
        case EINGABE_HOPFENGABEN_ANZAHL:
            modus = KOCHEN;
            break;
        case EINGABE_HOPFENGABEN_ZEIT:
            if (x > 1) {
                x--;                         // back to the previous hop, same screen
            } else {
                modus = EINGABE_HOPFENGABEN_ANZAHL;
            }
            break;
        case KOCHEN_START_FRAGE:
            x = hopfenanzahl;
            modus = EINGABE_HOPFENGABEN_ZEIT;
            break;

        case SETUP_MENU:
            modus = HAUPTSCHIRM;
            break;
        case SETUP_KOCHSCHWELLE:
        case SETUP_AP:
            modus = SETUP_MENU;
            break;

        default:
            return;  // HAUPTSCHIRM (nothing above it) or nothing to do
    }
    anfang = true;
    lcd_clear();
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
    for (int p = 0; p <= 3; p++) {
        print_lcdP(p == pos ? PSTR(">") : PSTR(" "), LEFT, p);  // cursor at col 0, items at col 1
    }
}
