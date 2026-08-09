#include <Arduino.h>


#include "screens.h"
#include "global.h"
#include "config.h"
#include "display.h"
#include "display_format.h"
#include "hardware.h"
#include "statemachine.h"
#include "persistence.h"
#include "manual_control.h"
#include "web.h"
#include "mqtt.h"
#include "recipe_timing.h"

static int y = 1;                                    //Übergabewert von x für Braumeisterruf
static int pause = 0;
static ManualTargetBeepState manualTargetBeep;
static unsigned long rufsignalzeit = 0;
static HopDeadlineState hopDeadlineState;
static uint8_t activeHopMask = 0;
static uint32_t hopReminderStartedAtSeconds = 0;

static void renderActiveHopReminder() {
    uint8_t first = 0;
    uint8_t last = 0;
    uint8_t count = 0;
    for (uint8_t index = 1; index <= MAX_HOP_DEADLINES; ++index) {
        if ((activeHopMask & static_cast<uint8_t>(1U << (index - 1))) == 0) continue;
        if (first == 0) first = index;
        last = index;
        ++count;
    }

    if (count == 1) {
        print_lcdP(PSTR("                    "), LEFT, 2);
        print_lcdP(PSTR("Gabe "), LEFT, 2);
        printNumI_lcd(first, 5, 2);
        print_lcdP(PSTR(" jetzt"), 6, 2);
        return;
    }

    bool contiguous = true;
    for (uint8_t index = first; index <= last; ++index) {
        if ((activeHopMask & static_cast<uint8_t>(1U << (index - 1))) == 0) {
            contiguous = false;
            break;
        }
    }
    print_lcdP(PSTR("                    "), LEFT, 2);
    if (contiguous) {
        print_lcdP(PSTR("Gaben "), LEFT, 2);
        printNumI_lcd(first, 6, 2);
        print_lcdP(PSTR("-"), 7, 2);
        printNumI_lcd(last, 8, 2);
        print_lcdP(PSTR(" jetzt"), 9, 2);
    } else {
        printNumI_lcd(count, LEFT, 2);
        print_lcdP(PSTR(" Gaben jetzt"), 1, 2);
    }
}
void enterBraumeisterRufalarm(RUFALARM_REASON reason) {
    rufalarmReason = reason;
    modus = BRAUMEISTERRUFALARM;
    mqttPublishRufalarm();
}


// ---- enriched recipe input rendering -------------------------------------
// These render the whole mash/boil plan on the 20x4 LCD with a '>' cursor on
// the item being edited, so the input screens double as a recipe overview.

// The mash plan is one scrolling list. Positions (1-indexed):
//   1            -> Einmaischen (maischtemp)
//   2..rasten+1  -> rest (pos-1): temperature + time
//   rasten+2     -> Abmaischen (endtemp)
// field: 0 = editing temperature, 1 = editing time (rests only).

// Einmaischen/Abmaischen row: "label .... 64°C", bracketed when it's the cursor.
static void render_plan_named_row(int dy, const char *labelP, int value, bool cursor) {
    char b[DISPLAY_SIZE_X + 1];
    memset(b, ' ', DISPLAY_SIZE_X);
    b[DISPLAY_SIZE_X] = '\0';
    b[0] = cursor ? '>' : ' ';
    char lab[DISPLAY_SIZE_X + 1];
    strcpy_P(lab, labelP);
    for (int i = 0; lab[i] && i + 1 < DISPLAY_SIZE_X; i++) b[i + 1] = lab[i];
    char t[4];
    snprintf(t, sizeof(t), "%3d", value);
    b[14] = t[0]; b[15] = t[1]; b[16] = t[2];  // b[17] = degree glyph "°C"
    if (cursor) { b[13] = '['; b[18] = ']'; }
    print_lcd(b, LEFT, dy);
    print_lcd_deg(17, dy);
}

// Rest row: ">1: 65°C   30 min", active field bracketed when it's the cursor.
static void render_plan_rest_row(int dy, int idx, bool cursor, int field) {
    char b[DISPLAY_SIZE_X + 1];
    memset(b, ' ', DISPLAY_SIZE_X);
    b[DISPLAY_SIZE_X] = '\0';
    b[0] = cursor ? '>' : ' ';
    b[1] = static_cast<char>('0' + idx);
    b[2] = ':';
    char t[4];
    snprintf(t, sizeof(t), "%3d", rastTemp[idx]);
    b[4] = t[0]; b[5] = t[1]; b[6] = t[2];  // b[7] = degree glyph "°C"
    if (cursor && field == 0) { b[3] = '['; b[8] = ']'; }
    char z[8];
    formatMinutes(z, sizeof(z), rastZeit[idx]);  // "XXX min" (7 chars)
    for (int i = 0; i < 7; i++) b[12 + i] = z[i];
    if (cursor && field == 1) { b[11] = '['; b[19] = ']'; }
    print_lcd(b, LEFT, dy);
    print_lcd_deg(7, dy);
}

static void render_plan_row(int dy, int pos, int activePos, int field) {
    int count = rasten + 2;
    bool cursor = (pos == activePos);
    if (pos < 1 || pos > count) {
        char blank[DISPLAY_SIZE_X + 1];
        memset(blank, ' ', DISPLAY_SIZE_X);
        blank[DISPLAY_SIZE_X] = '\0';
        print_lcd(blank, LEFT, dy);
    } else if (pos == 1) {
        render_plan_named_row(dy, PSTR("Einmaischen"), maischtemp, cursor);
    } else if (pos == count) {
        render_plan_named_row(dy, PSTR("Abmaischen"), endtemp, cursor);
    } else {
        render_plan_rest_row(dy, pos - 1, cursor, field);
    }
}

// Header + 2-row scrolling window over the whole mash plan, cursor on the item
// being edited. Row 3 is owned by the live-temperature overlay (loop()).
static void render_mash_plan(int activePos, int field) {
    print_lcdP(PSTR("Auto"), LEFT, 0);  // every frame: first print after clear glitches on slow clones

    int count = rasten + 2;
    // Only the value at the cursor changes between frames; skip the LCD writes
    // when nothing changed so the display isn't rewritten every loop (flicker).
    int val = (activePos == 1)     ? maischtemp
            : (activePos == count) ? endtemp
            : (field == 0)         ? rastTemp[activePos - 1]
                                   : rastZeit[activePos - 1];
    static int lp = -1, lf = -1, lv = -1, lc = -1;
    if (!lcdNeedsRedraw && activePos == lp && field == lf && val == lv && count == lc) {
        return;
    }
    lp = activePos; lf = field; lv = val; lc = count;
    lcdNeedsRedraw = false;

    print_lcdP(PSTR("Maischen"), RIGHT, 0);

    int start = listWindowStart(activePos, count, 2);
    for (int dy = 1; dy <= 2; dy++) {
        render_plan_row(dy, start + dy - 1, activePos, field);
    }
}

// Header + 3-row scrolling window over the hop additions, cursor on hop `x`.
static void render_hop_list() {
    print_lcdP(PSTR("Kochen"), LEFT, 0);  // every frame: first print after clear glitches on slow clones

    static int lx = -1, lv = -1, ln = -1;
    if (!lcdNeedsRedraw && x == lx && hopfenZeit[x] == lv && hopfenanzahl == ln) {
        return;  // unchanged -> skip writes (prevents flicker)
    }
    lx = x; lv = hopfenZeit[x]; ln = hopfenanzahl;
    lcdNeedsRedraw = false;

    char hb[DISPLAY_SIZE_X + 1];
    snprintf(hb, sizeof(hb), "Gabe %d von %d", x, hopfenanzahl);
    print_lcd(hb, RIGHT, 0);

    // rows 1-2 only; row 3 is owned by the live-temperature overlay (loop()).
    int start = listWindowStart(x, hopfenanzahl, 2);
    for (int dy = 1; dy <= 2; dy++) {
        int idx = start + dy - 1;
        char b[DISPLAY_SIZE_X + 1];
        memset(b, ' ', DISPLAY_SIZE_X);
        b[DISPLAY_SIZE_X] = '\0';
        if (idx >= 1 && idx <= hopfenanzahl) {
            bool cur = (idx == x);
            b[0] = cur ? '>' : ' ';
            b[1] = static_cast<char>('0' + idx);
            b[2] = ':';
            memcpy(b + 4, "nach", 4);
            char z[8];
            formatMinutes(z, sizeof(z), hopfenZeit[idx]);
            for (int i = 0; i < 7; i++) b[12 + i] = z[i];
            if (cur) { b[11] = '['; b[19] = ']'; }
        }
        print_lcd(b, LEFT, dy);
    }
}

// Recipe chooser (Start / Bearbeiten) shown before the mash or boil edit chain.
static void render_rezept_frage(const char *titleP, int sel) {
    // Title every frame: the first print right after lcd_clear() corrupts its
    // first char on slow LCD clones, so a once-only title would stay "0aischen".
    print_lcdP(titleP, LEFT, 0);

    static int ls = -1;
    if (!lcdNeedsRedraw && sel == ls) {
        return;  // only the selection changes between frames
    }
    ls = sel;
    lcdNeedsRedraw = false;

    char nm[DISPLAY_SIZE_X + 1];
    snprintf(nm, sizeof(nm), "%-20s", recipeName);  // pad to clear leftovers
    print_lcd(nm, LEFT, 1);

    // Both options on row 2 ('>' marks the selection); row 3 stays free for the
    // live-temperature overlay drawn in loop().
    char b[DISPLAY_SIZE_X + 1];
    memset(b, ' ', DISPLAY_SIZE_X);
    b[DISPLAY_SIZE_X] = '\0';
    b[0] = (sel == 0) ? '>' : ' ';
    memcpy(b + 1, "Start", 5);
    const int p2 = DISPLAY_SIZE_X - 10;  // "Bearbeiten" right-aligned, cols 10..19
    if (sel == 1) b[p2 - 1] = '>';
    memcpy(b + p2, "Bearbeiten", 10);
    print_lcd(b, LEFT, 2);
}

void funktion_hauptschirm() {
    if (anfang) {
        lcd_clear();
        drehen = 0;
        anfang = false;
        print_lcdP(PSTR("Maischautomatik"), 1, 0);
        print_lcdP(PSTR("Maischen manuell"), 1, 1);
        print_lcdP(PSTR("Kochen"), 1, 2);
        print_lcdP(PSTR("Setup"), 1, 3);
    }

    drehen = constrain(drehen, 0, 3);

    menu_zeiger(drehen);
    switch (drehen) {
        case 0:
            rufmodus = AUTOMATIK_FRAGE;
            break;
        case 1:
            rufmodus = MANUELL;
            break;
        case 2:
            rufmodus = KOCHEN_FRAGE;
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
        print_lcdP(PSTR("Kochschwelle"), 1, 0);
        print_lcdP(PSTR("AP ein/aus"), 1, 1);
    }

    drehen = constrain(drehen, 0, 1);

    menu_zeiger(drehen);
    rufmodus = drehen == 0 ? SETUP_KOCHSCHWELLE : SETUP_AP;

    if (warte_und_weiter(rufmodus)) {
        lcd_clear();
    }
}

// Chooser shown when entering Maischautomatik / Kochen: Start the loaded recipe
// straight away, or Bearbeiten to step through the edit chain as before.
void funktion_rezeptfrage(const char *titleP, MODUS startModus, MODUS editModus) {
    if (anfang) {
        lcd_clear();
        drehen = 0;
        anfang = false;
    }

    drehen = constrain(drehen, 0, 1);
    render_rezept_frage(titleP, drehen);
    rufmodus = (drehen == 0) ? startModus : editModus;

    if (warte_und_weiter(rufmodus)) {
        lcd_clear();
        x = 1;  // the edit chain normally initialises the rest/hop index
    }
}

void funktion_temperatur() {
    if (anfang) {
        lcd_clear();
        armManualTargetBeep(manualTargetBeep, drehen);
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

    if (modus == MANUELL) {
        const bool wasActive = manualTargetBeep.active;
        const bool beepOn = updateManualTargetBeep(
            manualTargetBeep, sollwert, isttemp, static_cast<uint32_t>(millis()));

        beeperOnOff(beepOn);
        if (beepOn) {
            print_lcdP(PSTR("RUF"), LEFT, 3);
        } else if (manualTargetBeep.active || wasActive) {
            print_lcdP(PSTR("   "), LEFT, 3);
        }
    }
}


void funktion_rastanzahl() {
    static int presetApplied = 0;  // rest count whose preset template is loaded
    if (anfang) {
        lcd_clear();
        drehen = rasten;
        presetApplied = rasten;  // keep current/imported/edited values on (re)entry
        anfang = false;
        print_lcdP(PSTR("Auto"), LEFT, 0);
        print_lcdP(PSTR("Eingabe"), RIGHT, 0);
        print_lcdP(PSTR("Rasten"), LEFT, 1);
    }

    drehen = constrain(drehen, 1, 5);
    rasten = drehen;

    // Load preset temps/times only when the count actually changes, so stepping
    // back into this screen (or importing a recipe) doesn't clobber edits.
    if (drehen != presetApplied) {
        presetApplied = drehen;
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
    }

    printNumI_lcd(rasten, 19, 1);

    warte_und_weiter(EINGABE_MAISCHTEMP);
}

void funktion_maischtemperatur() {
    if (anfang) {
        lcd_clear();
        drehen = maischtemp;
        anfang = false;
    }

    drehen = constrain(drehen, 10, 105);
    maischtemp = drehen;

    render_mash_plan(1, 0);  // cursor on Einmaischen (first item)

    warte_und_weiter(EINGABE_RAST_TEMP);
}

void funktion_rasteingabe() {
    if (anfang) {
        lcd_clear();
        drehen = rastTemp[x];
        anfang = false;
    }

    drehen = constrain(drehen, 9, 105);
    rastTemp[x] = drehen;

    render_mash_plan(x + 1, 0);  // cursor on rest x, editing temperature

    warte_und_weiter(EINGABE_RAST_ZEIT);
}

void funktion_zeiteingabe() {
    if (anfang) {
        lcd_clear();
        drehen = rastZeit[x];
        anfang = false;
    }

    drehen = constrain(drehen, 1, 99);
    rastZeit[x] = drehen;

    render_mash_plan(x + 1, 1);  // cursor on rest x, editing time

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
    }

    drehen = constrain(drehen, 10, 80);
    endtemp = drehen;

    render_mash_plan(rasten + 2, 0);  // cursor on Abmaischen (last item)

    warte_und_weiter(AUTO_START);
}

void funktion_startabfrage(MODUS naechsterModus, const char *title) {
    if (anfang) {
        lcd_clear();
        anfang = false;
        altsekunden = millis();
    }

    // Redraw every frame: the first print right after lcd_clear() corrupts its
    // first char on slow LCD clones, and a once-only title would keep the glitch.
    print_lcd(title, LEFT, 0);

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
        enterBraumeisterRufalarm(RUFALARM_REASON_MAISCHSTART);
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
        print_lcdP(PSTR("Set Time"), LEFT, 3);
        if (holdReturnModus == AUTO_RAST_ZEIT) {
            holdReturnModus = HAUPTSCHIRM;
        } else {
            startBrewClock(brewClock, static_cast<uint32_t>(millis()));
        }
        print_lcdP(PSTR("            "), 0, 3);
        anfang = false;
        print_lcdP(PSTR("00:00"), LEFT, 2);
    }

    drehen = constrain(drehen, 10, 105);
    rastZeit[x] = drehen;

    const uint32_t elapsedSeconds =
        brewElapsedSeconds(brewClock, static_cast<uint32_t>(millis()));

    static uint32_t lastElapsedSeconds = UINT32_MAX;
    static int lastZeit = -1;
    if (lcdNeedsRedraw || elapsedSeconds != lastElapsedSeconds || rastZeit[x] != lastZeit) {
        lastElapsedSeconds = elapsedSeconds;
        lastZeit = rastZeit[x];
        lcdNeedsRedraw = false;

        char elapsedText[12];
        snprintf(elapsedText, sizeof(elapsedText), "%02lu:%02lu",
                 static_cast<unsigned long>(elapsedSeconds / 60UL),
                 static_cast<unsigned long>(elapsedSeconds % 60UL));
        print_lcd_minutes(rastZeit[x], RIGHT, 2);
        print_lcd(elapsedText, LEFT, 2);
    }

    if (elapsedSeconds >= static_cast<uint32_t>(rastZeit[x]) * 60UL) {
        resetBrewClock(brewClock);
        anfang = true;
        y = x;
        if (x < rasten) {
            modus = AUTO_RAST_TEMP;
            x++;
        } else {
            x = 1;
            modus = AUTO_ENDTEMP;
        }

        if (braumeister[y] != BM_ALARM_AUS) {
            rufmodus = modus;
            enterBraumeisterRufalarm(RUFALARM_REASON_RASTENDE);
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

    }

    drehen = constrain(drehen, 10, 105);
    endtemp = drehen;
    sollwert = endtemp;

    if (isttemp >= sollwert) {
        regelung = REGL_AUS;
        heizung = false;
        y = 0;
        braumeister[y] = BM_ALARM_WAIT;
        enterBraumeisterRufalarm(RUFALARM_REASON_MAISCHENDE);
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
    anfang = false;

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
        rufalarmReason = RUFALARM_REASON_NONE;
        delay(500);     // kurze Wartezeit, damit nicht durch unbeabsichtigtes Drehen der nächste Vorgabewert verstellt wird
    }
}

void funktion_brauvorgang_halt() {
    if (anfang) {
        lcd_clear();
        drehen = 0;
        anfang = false;
    }

    drehen = constrain(drehen, 0, 1);
    print_lcdP(PSTR("Vorgang HALT"), CENTER, 0);
    print_lcdP(drehen == 0 ? PSTR(">Fortsetzen") : PSTR(" Fortsetzen"), LEFT, 1);
    print_lcdP(drehen == 1 ? PSTR(">Abbrechen") : PSTR(" Abbrechen"), LEFT, 2);

    if (!ButtonPressed) return;
    if (drehen == 1) {
        funktion_abbruch();
        return;
    }

    const MODUS resumedModus = holdReturnModus;
    if (resumedModus == AUTO_RAST_ZEIT || resumedModus == KOCHEN_AUTO_LAUF) {
        resumeBrewClock(brewClock, static_cast<uint32_t>(millis()));
    }
    modus = resumedModus;
    x = holdReturnX;
    sollwert = holdTarget;
    anfang = true;
}


void funktion_ap() {
    if (anfang) {
        lcd_clear();
        anfang = false;
        print_lcdP(PSTR("AP ein/aus"), LEFT, 0);
        print_lcdP(PSTR("Eingabe"), RIGHT, 0);

        drehen = isAccessPointEnabled() ? 1 : 0;
    }

    drehen = constrain(drehen, 0, 1);
    print_lcdP(drehen ? PSTR("ein") : PSTR("aus"), RIGHT, 1);

    if (warte_und_weiter(SETUP_MENU)) {
        setAccessPointEnabled(drehen == 1);
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

    if (warte_und_weiter(EINGABE_HOPFENGABEN_ZEIT)) {
        x = 1;  // start at the first hop (the zeit screen keeps x so back-step works)
    }
}

void funktion_hopfengaben() {
    if (anfang) {
        drehen = hopfenZeit[x];  // x is set on entry (anzahl screen) / back-step
        anfang = false;
        lcd_clear();
    }

    // Untergrenze = vorige Gabe + 5 min Mindestabstand; Obergrenze 5 min vor
    // Kochende. Bei enger Kochzeit Untergrenze kappen, damit der Bereich nie
    // degeneriert (min > max), sonst flackert der Wert.
    int lo = hopfenZeit[x - 1] + 5;
    int hi = kochzeit - 5;
    if (lo > hi) {
        lo = hi;
    }
    drehen = constrain(drehen, lo, hi);
    hopfenZeit[x] = drehen;

    render_hop_list();

    if (warte_und_weiter(modus)) {
        if (x < hopfenanzahl) {
            x++;
            drehen = hopfenZeit[x];
            anfang = false; // nicht auf Anfang zurück, nur Cursor weiter
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
        const bool resume = holdReturnModus == KOCHEN_AUTO_LAUF;
        if (resume) {
            holdReturnModus = HAUPTSCHIRM;
        } else {
            x = 1;
            startBrewClock(brewClock, static_cast<uint32_t>(millis()));
            resetHopDeadlines(hopDeadlineState);
            activeHopMask = 0;
            hopReminderStartedAtSeconds = 0;
        }
        lcd_clear();
        print_lcdP(PSTR("Kochen"), LEFT, 0);
        print_lcd_minutes(kochzeit, RIGHT, 0);
        print_lcdP(PSTR("00:00"), 11, 1);
        anfang = false;
    }

    const uint32_t elapsedSeconds =
        brewElapsedSeconds(brewClock, static_cast<uint32_t>(millis()));
    const uint8_t newHopMask =
        collectDueHops(hopDeadlineState, hopfenZeit, hopfenanzahl, elapsedSeconds);
    if (newHopMask != 0) {
        activeHopMask |= newHopMask;
        hopReminderStartedAtSeconds = elapsedSeconds;
    }

    const uint8_t nextHop = nextPendingHopIndex(hopDeadlineState, hopfenanzahl);
    x = nextHop == 0 ? MAX_HOP_DEADLINES + 1 : nextHop;

    static uint32_t lastElapsedSeconds = UINT32_MAX;
    static int lastX = -1;
    if (lcdNeedsRedraw || elapsedSeconds != lastElapsedSeconds || x != lastX) {
        lastElapsedSeconds = elapsedSeconds;
        lastX = x;
        lcdNeedsRedraw = false;

        if (activeHopMask == 0 && x <= hopfenanzahl && x <= MAX_HOP_DEADLINES) {
            printNumI_lcd(x, LEFT, 2);
            print_lcdP(PSTR(". Gabe bei "), 1, 2);
            print_lcd_minutes(hopfenZeit[x], RIGHT, 2);
        } else if (activeHopMask == 0) {
            print_lcdP(PSTR("                    "), LEFT, 2);
        }

        char elapsedText[12];
        const int elapsedTextLength = snprintf(
            elapsedText, sizeof(elapsedText), "%02lu:%02lu",
            static_cast<unsigned long>(elapsedSeconds / 60UL),
            static_cast<unsigned long>(elapsedSeconds % 60UL));
        print_lcdP(PSTR("min"), RIGHT, 1);
        print_lcd(elapsedText, 16 - elapsedTextLength, 1);
    }

    if (activeHopMask != 0) {
        renderActiveHopReminder();
        if (millis() >= (altsekunden + 1000)) {
            print_lcdP(PSTR("   "), LEFT, 3);
            beeperOnOff(false);
            if (millis() >= (altsekunden + 1500)) {
                altsekunden = millis();
                pause++;
            }
        } else {
            print_lcdP(PSTR("RUF"), LEFT, 3);
            if (pause <= 4) beeperOnOff(true);
            if (pause > 8) pause = 0;
        }

        if (warte_und_weiter(modus) ||
            elapsedSeconds - hopReminderStartedAtSeconds >= 60UL) {
            print_lcdP(PSTR("   "), LEFT, 3);
            pause = 0;
            beeperOnOff(false);
            activeHopMask = 0;
            anfang = false;
        }
    }

    if (elapsedSeconds >= static_cast<uint32_t>(kochzeit) * 60UL) {
        resetBrewClock(brewClock);
        anfang = true;
        rufmodus = ABBRUCH;
        regelung = REGL_AUS;
        heizung = false;
        y = 0;
        braumeister[y] = BM_ALARM_WAIT;
        enterBraumeisterRufalarm(RUFALARM_REASON_KOCHENDE);
    }
}


void funktion_abbruch() {
    regelung = REGL_AUS;
    heizung = false;
    heizungOnOff(false);
    beeperOnOff(false);
    resetBrewClock(brewClock);
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
