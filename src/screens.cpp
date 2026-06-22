#include <Arduino.h>
#include <TimeLib.h>

#include "screens.h"
#include "global.h"
#include "config.h"
#include "display.h"
#include "hardware.h"
#include "statemachine.h"
#include "persistence.h"

static int y = 1;                                    //Übergabewert von x für Braumeisterruf
static int pause = 0;
static unsigned long rufsignalzeit = 0;

static void _next_koch_step();

void funktion_hauptschirm() {
    if (anfang) {
        lcd_clear();
        drehen = 0;
        anfang = false;
        print_lcdP(PSTR("Maischautomatik"), 2, 0);
        print_lcdP(PSTR("Maischen manuell"), 2, 1);
        print_lcdP(PSTR("Kochen"), 2, 2);
        print_lcdP(PSTR("Setup"), 2, 3);
    }

    drehen = constrain(drehen, 0, 3);

    menu_zeiger(drehen);
    switch (drehen) {
        case 0:
            rufmodus = AUTOMATIK;
            break;
        case 1:
            rufmodus = MANUELL;
            break;
        case 2:
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
        if (modus == MANUELL) {
            //Übergabe an Modus1
            drehen = static_cast<int>(isttemp) + 10; // Vorgabewert 10°C über IstWert
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
        warte_und_weiter(MANUELL_HALTEN);
    }
}

void funktion_temperatur_halten() {
    sollwert = drehen;

    // warte_und_weiter(ABBRUCH); // TODO will man das wirklich?
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
            rastTemp[1] = 66;
            rastZeit[1] = 60;
            maischtemp = 67;
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
            rastZeit[1] = 15;
            rastTemp[2] = 62;
            rastZeit[2] = 20;
            rastTemp[3] = 72;
            rastZeit[3] = 35;
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
        drehen = rastZeit[x];
        anfang = false;
    }

    drehen = constrain(drehen, 1, 99);
    rastZeit[x] = drehen;

    print_lcd_minutes(rastZeit[x], RIGHT, 2);

    //warte_und_weiter(EINGABE_BRAUMEISTERRUF);

    braumeister[x] = BM_ALARM_AUS;

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

void funktion_startabfrage(MODUS naechsterModus, const char *title) {
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

        drehen = hysteresespeicher;
    }

    drehen = constrain(drehen, 0, 40); //max. 4,0 Sekunden Hysterese
    hysteresespeicher = static_cast<uint8_t>(drehen);

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
    kschwelle = static_cast<uint8_t>(drehen);

    printNumI_lcd(kschwelle, RIGHT, 1);

    if (warte_und_weiter(SETUP_MENU)) {
        writeEepromData();
    }
}

void funktion_kochzeit() {
    if (anfang) {
        lcd_clear();
        drehen = kochzeit;
        anfang = false;
        print_lcdP(PSTR("Kochen"), LEFT, 0);
        print_lcdP(PSTR("Eingabe"), RIGHT, 0);
        print_lcdP(PSTR("Zeit"), LEFT, 1);
    }

    drehen = constrain(drehen, 20, 180);
    kochzeit = drehen;

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
        drehen = hopfenZeit[x];
        anfang = false;
        lcd_clear();
        print_lcdP(PSTR("Kochen"), LEFT, 0);
        print_lcdP(PSTR("Eingabe"), RIGHT, 0);
    }

    printNumI_lcd(x, LEFT, 1);
    print_lcdP(PSTR(". Hopfengabe"), 1, 1);
    print_lcdP(PSTR("nach"), LEFT, 2);

    drehen = constrain(drehen, hopfenZeit[(x - 1)] + 5, kochzeit - 5);
    hopfenZeit[x] = drehen;

    print_lcd_minutes(hopfenZeit[x], RIGHT, 2);

    if (warte_und_weiter(modus)) {
        if (x < hopfenanzahl) {
            x++;
            drehen = hopfenZeit[x];
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

        anfang = false;
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

static void _next_koch_step() {
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

    modus = HAUPTSCHIRM;
}
