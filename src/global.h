#pragma once

#include <stdint.h>
struct BrewClockState;


enum MODUS {
    HAUPTSCHIRM = 0,
    MANUELL,
    SETUP_MENU = 3,
    SETUP_KOCHSCHWELLE = 5,
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
    ALARMTEST,
    // appended at the end so existing MODUS integer values (published over MQTT
    // and /data.json) stay stable:
    AUTOMATIK_FRAGE,        // Rezept-Chooser (Start/Bearbeiten) vor dem Maischen
    KOCHEN_FRAGE,           // Rezept-Chooser (Start/Bearbeiten) vor dem Kochen
    SETUP_AP,
    BRAUVORGANG_HALT
};

inline bool isRufalarmMode(MODUS mode) {
    return mode == BRAUMEISTERRUFALARM || mode == BRAUMEISTERRUF;
}

inline bool brewIsActive(MODUS mode) {
    return mode == AUTO_MAISCHTEMP || mode == AUTO_RAST_TEMP ||
           mode == AUTO_RAST_ZEIT || mode == AUTO_ENDTEMP ||
           mode == KOCHEN_AUFHEIZEN || mode == KOCHEN_AUTO_LAUF;
}

enum RUFALARM_REASON {
    RUFALARM_REASON_NONE,
    RUFALARM_REASON_MAISCHSTART,
    RUFALARM_REASON_RASTENDE,
    RUFALARM_REASON_MAISCHENDE,
    RUFALARM_REASON_KOCHENDE,
    RUFALARM_REASON_SENSORFEHLER,
    RUFALARM_REASON_ALARMTEST
};

inline const char *rufalarmReasonName(RUFALARM_REASON reason) {
    switch (reason) {
        case RUFALARM_REASON_MAISCHSTART: return "mash_start";
        case RUFALARM_REASON_RASTENDE: return "rest_complete";
        case RUFALARM_REASON_MAISCHENDE: return "mash_end";
        case RUFALARM_REASON_KOCHENDE: return "boil_end";
        case RUFALARM_REASON_SENSORFEHLER: return "sensor_fault";
        case RUFALARM_REASON_ALARMTEST: return "alarm_test";
        default: return "none";
    }
}

inline const char *rufalarmActionName(RUFALARM_REASON reason, bool alarm) {
    if (!alarm) return "none";
    return reason == RUFALARM_REASON_SENSORFEHLER
        ? "check_sensor_and_acknowledge_at_controller"
        : "acknowledge_at_controller";
}

inline const char *modeStatusName(MODUS mode) {
    switch (mode) {
        case HAUPTSCHIRM:
            return "Bereit";
        case MANUELL:
            return "Manuelles Maischen";
        case AUTO_START:
        case AUTO_MAISCHTEMP:
        case AUTO_RAST_TEMP:
        case AUTO_RAST_ZEIT:
        case AUTO_ENDTEMP:
            return "Automatisches Maischen";
        case BRAUMEISTERRUFALARM:
        case BRAUMEISTERRUF:
            return "Rufalarm";
        case KOCHEN_AUFHEIZEN:
        case KOCHEN_AUTO_LAUF:
            return "Kochen";
        default:
            return "Einrichtung";
    }
}

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

constexpr int KOCHSCHWELLE_DEFAULT = 98;

constexpr int KOCHSCHWELLE_MEM = 25;

constexpr int HENDI_MAX_RUNTIME = 85;

constexpr int ENCODER_STEPS_PER_NOTCH = 4;   // Change this depending on which encoder is used

constexpr char APSSID[] = "BrauKnecht";

extern BrewClockState brewClock;
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
extern RUFALARM_REASON rufalarmReason;
extern MODUS holdReturnModus;
extern int holdReturnX;
extern int holdTarget;
extern bool holdWasHeating;

// shared mutable state (definitions in Brauknecht.cpp)
extern bool ButtonPressed;
extern volatile int drehen;
extern bool anfang;
extern unsigned long altsekunden;
extern bool sensorfehler;
extern int n;
extern uint8_t kschwelle;
extern bool einmaldruck;
