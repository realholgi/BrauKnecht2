#ifndef GLOBAL_H
#define GLOBAL_H

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

#define HYSTERESE_MEM 0
#define HYSTERESE_DEFAULT 5

#define KOCHSCHWELLE_DEFAULT 98
#define KOCHSCHWELLE_MEM 25

#define HENDI_MAX_RUNTIME 85

#define ENCODER_STEPS_PER_NOTCH    4   // Change this depending on which encoder is used

#define APSSID "BrauKnecht"
#define APPSK "brauknecht"

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
extern int timer;
extern float isttemp;
extern bool heizung;
extern int x;

#endif //GLOBAL_H
