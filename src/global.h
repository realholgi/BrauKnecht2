#pragma once

#include <stdint.h>

enum MODUS {
    HAUPTSCHIRM = 0,
    MANUELL,
    MANUELL_HALTEN,
    SETUP_MENU,
    SETUP_HYSTERESE,
    SETUP_KOCHSCHWELLE,
    EINGABE_RAST_ANZ,
    AUTOMATIK = EINGABE_RAST_ANZ,
    EINGABE_MAISCHTEMP,
    EINGABE_RAST_TEMP,
    EINGABE_RAST_ZEIT,
    EINGABE_ENDTEMP,
    AUTO_START,
    AUTO_MAISCHTEMP,
    AUTO_RAST_TEMP,
    AUTO_RAST_ZEIT,
    AUTO_ENDTEMP,
    BRAUMEISTERRUFALARM,
    BRAUMEISTERRUF,
    KOCHEN,
    EINGABE_HOPFENGABEN_ANZAHL,
    EINGABE_HOPFENGABEN_ZEIT,
    KOCHEN_START_FRAGE,
    KOCHEN_AUFHEIZEN,
    KOCHEN_AUTO_LAUF,
    ABBRUCH,
    ALARMTEST
};
enum REGEL_MODE {
    REGL_AUS = 0,
    REGL_MAISCHEN,
    REGL_KOCHEN
};
enum BM_ALARM_MODE {
    BM_ALARM_AUS = 0,
    BM_ALARM_MIN = BM_ALARM_AUS,
    BM_ALARM_WAIT,
    BM_ALARM_SIGNAL,
    BM_ALARM_MAX = BM_ALARM_SIGNAL
};

constexpr int HYSTERESE_MEM = 0;
constexpr int HYSTERESE_DEFAULT = 5;

constexpr int KOCHSCHWELLE_DEFAULT = 98;
constexpr int KOCHSCHWELLE_MEM = 25;

constexpr int HENDI_MAX_RUNTIME = 85;

constexpr int ENCODER_STEPS_PER_NOTCH = 4;   // Change this depending on which encoder is used

constexpr char APSSID[] = "BrauKnecht";
constexpr char APPSK[] = "brauknecht";

extern int sekunden;
extern int minuten;
extern int minutenwert;
extern int stunden;
extern MODUS modus;
extern MODUS rufmodus;
extern int sollwert;
extern int maischtemp;
extern int rasten;
extern int rastTemp[];
extern int rastZeit[];
extern BM_ALARM_MODE braumeister[];
extern int endtemp;
extern int kochzeit;
extern int hopfenanzahl;
extern int hopfenZeit[];
extern char recipeName[];
extern int timer;
extern float isttemp;
extern bool heizung;
extern REGEL_MODE regelung;
extern int x;

// shared mutable state (definitions in Brauknecht.cpp)
extern bool ButtonPressed;
extern volatile int drehen;
extern bool anfang;
extern unsigned long altsekunden;
extern bool sensorfehler;
extern float hysterese;
extern uint8_t hysteresespeicher;
extern long wartezeit;
extern int n;
extern uint8_t kschwelle;
extern bool einmaldruck;
