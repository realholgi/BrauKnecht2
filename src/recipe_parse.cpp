#include <string.h>
#include <stdlib.h>

#include "recipe_parse.h"

static int clampInt(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// --- shared mapping (used by both KBH and BeerXML) ---------------------------

// Mash plan -> globals: first step = Einmaischen (maischtemp), last = Abmaischen
// (endtemp), middle steps = rests.
static void mapMash(Recipe &r, const int *temps, const int *times, int count) {
    r.rasten = 0;
    for (int i = 0; i < count; i++) {
        if (i == 0) {
            r.maischtemp = temps[i];
        } else if (i == count - 1) {
            r.endtemp = temps[i];
        } else if (r.rasten < 7) {
            r.rasten++;
            r.rastTemp[r.rasten] = temps[i];
            r.rastZeit[r.rasten] = times[i];
        }
    }
    if (r.rasten < 1) {  // degenerate plan (<3 steps): one rest so the brew runs
        r.rasten = 1;
        r.rastTemp[1] = r.maischtemp;
        r.rastZeit[1] = 0;
    }
}

// Hop additions: input is "minutes before boil end"; BrauKnecht counts up from
// boil start, so hopfenZeit = kochzeit - beforeEnd. r.kochzeit must be set first.
static void mapHops(Recipe &r, const int *beforeEnd, int count) {
    r.hopfenanzahl = 0;
    for (int i = 0; i < count && r.hopfenanzahl < 6; i++) {
        int t = clampInt(r.kochzeit - beforeEnd[i], 0, r.kochzeit);
        // Gaben zur gleichen Zeit zusammenfassen: BrauKnecht kennt pro Gabe nur
        // einen Zeitpunkt (einen Alarm), doppelte Zeiten wären nur Wiederholungen.
        bool dup = false;
        for (int j = 1; j <= r.hopfenanzahl; j++) {
            if (r.hopfenZeit[j] == t) {
                dup = true;
                break;
            }
        }
        if (dup) {
            continue;
        }
        r.hopfenanzahl++;
        r.hopfenZeit[r.hopfenanzahl] = t;
    }
    if (r.hopfenanzahl < 1) {
        r.hopfenanzahl = 1;
        r.hopfenZeit[1] = 0;
    }
}

// --- Kleiner Brauhelfer JSON -------------------------------------------------

void buildKbhFilter(JsonDocument &filter) {
    filter["Sud"]["Sudname"] = true;
    filter["Sud"]["Kochdauer"] = true;
    // [0] applies the sub-filter to every element of the array
    filter["Maischplan"][0]["TempRast"] = true;
    filter["Maischplan"][0]["DauerRast"] = true;
    filter["Hopfengaben"][0]["Zeit"] = true;
}

bool parseKbhDoc(const JsonDocument &doc, Recipe &r) {
    JsonObjectConst sud = doc["Sud"];
    if (sud.isNull()) {
        return false; // not a Kleiner Brauhelfer recipe
    }

    memset(&r, 0, sizeof(r));
    strlcpy(r.name, sud["Sudname"] | "", sizeof(r.name));
    r.kochzeit = sud["Kochdauer"] | 0;

    int temps[16], times[16], tc = 0;
    for (JsonObjectConst step : doc["Maischplan"].as<JsonArrayConst>()) {
        if (tc >= 16) break;
        temps[tc] = step["TempRast"] | 0;
        times[tc] = step["DauerRast"] | 0;
        tc++;
    }
    mapMash(r, temps, times, tc);

    int beforeEnd[16], hc = 0;
    for (JsonObjectConst hop : doc["Hopfengaben"].as<JsonArrayConst>()) {
        if (hc >= 16) break;
        beforeEnd[hc++] = hop["Zeit"] | 0;
    }
    mapHops(r, beforeEnd, hc);
    return true;
}

// --- BeerXML -----------------------------------------------------------------
// Hand-rolled subset scanner (no DOM): we only need RECIPE/NAME, BOIL_TIME,
// each MASH_STEP's STEP_TEMP/STEP_TIME, and HOP TIME where USE == Boil.

static bool tagIs(const char *ns, size_t nlen, const char *name) {
    return strlen(name) == nlen && strncmp(ns, name, nlen) == 0;
}

static void copyText(const char *t, char *buf, size_t n) {
    size_t i = 0;
    while (t[i] && t[i] != '<' && i < n - 1) {
        buf[i] = t[i];
        i++;
    }
    buf[i] = '\0';
}

bool parseBeerXmlString(const char *xml, Recipe &r) {
    if (!strstr(xml, "<RECIPE")) {
        return false;
    }
    memset(&r, 0, sizeof(r));

    int boilTime = 0;
    int temps[16], times[16], stepCount = 0;
    int beforeEnd[16], hopCount = 0;
    bool gotName = false, inMashStep = false, inHop = false;
    int curTemp = 0, curTime = 0, curHopTime = 0;
    char curUse[16] = {0};

    const char *p = xml;
    while ((p = strchr(p, '<')) != nullptr) {
        p++;
        bool closing = (*p == '/');
        if (closing) {
            p++;
        }
        const char *ns = p;
        while (*p && *p != '>' && *p != ' ' && *p != '/') {
            p++;
        }
        size_t nlen = p - ns;
        const char *gt = strchr(p, '>');
        if (!gt) break;
        const char *text = gt + 1;

        if (tagIs(ns, nlen, "MASH_STEP")) {
            if (!closing) {
                inMashStep = true;
                curTemp = curTime = 0;
            } else {
                if (stepCount < 16) {
                    temps[stepCount] = curTemp;
                    times[stepCount] = curTime;
                    stepCount++;
                }
                inMashStep = false;
            }
        } else if (tagIs(ns, nlen, "HOP")) {
            if (!closing) {
                inHop = true;
                curHopTime = 0;
                curUse[0] = '\0';
            } else {
                // Boil additions only — First Wort and Dry Hop are intentionally
                // dropped (BrauKnecht only models timed boil additions).
                if (strcmp(curUse, "Boil") == 0 && hopCount < 16) {
                    beforeEnd[hopCount++] = curHopTime;
                }
                inHop = false;
            }
        } else if (!closing) {
            if (tagIs(ns, nlen, "NAME")) {
                if (!gotName) {
                    copyText(text, r.name, sizeof(r.name));
                    gotName = true;
                }
            } else if (tagIs(ns, nlen, "BOIL_TIME")) {
                boilTime = atoi(text);
            } else if (inMashStep && tagIs(ns, nlen, "STEP_TEMP")) {
                curTemp = atoi(text);
            } else if (inMashStep && tagIs(ns, nlen, "STEP_TIME")) {
                curTime = atoi(text);
            } else if (inHop && tagIs(ns, nlen, "USE")) {
                copyText(text, curUse, sizeof(curUse));
            } else if (inHop && tagIs(ns, nlen, "TIME")) {
                curHopTime = atoi(text);
            }
        }
        p = gt + 1;
    }

    if (!gotName && stepCount == 0) {
        return false; // didn't look like a recipe
    }

    r.kochzeit = boilTime;
    mapMash(r, temps, times, stepCount);
    mapHops(r, beforeEnd, hopCount);
    return true;
}

// --- our own persistence format ----------------------------------------------

size_t recipeToJson(const Recipe &r, char *buf, size_t n) {
    JsonDocument doc;
    doc["name"] = r.name;
    doc["maischtemp"] = r.maischtemp;
    doc["rasten"] = r.rasten;
    doc["endtemp"] = r.endtemp;
    doc["kochzeit"] = r.kochzeit;
    doc["hopfenanzahl"] = r.hopfenanzahl;

    JsonArray rt = doc["rastTemp"].to<JsonArray>();
    JsonArray rz = doc["rastZeit"].to<JsonArray>();
    for (int i = 1; i <= r.rasten; i++) {
        rt.add(r.rastTemp[i]);
        rz.add(r.rastZeit[i]);
    }
    JsonArray hz = doc["hopfenZeit"].to<JsonArray>();
    for (int i = 1; i <= r.hopfenanzahl; i++) {
        hz.add(r.hopfenZeit[i]);
    }
    return serializeJson(doc, buf, n);
}

bool recipeFromJson(const char *json, Recipe &r) {
    memset(&r, 0, sizeof(r));

    JsonDocument doc;
    if (deserializeJson(doc, json)) {
        return false;
    }

    strlcpy(r.name, doc["name"] | "", sizeof(r.name));
    r.maischtemp = doc["maischtemp"] | 0;
    r.rasten = clampInt(doc["rasten"] | 0, 0, 7);
    r.endtemp = doc["endtemp"] | 0;
    r.kochzeit = doc["kochzeit"] | 0;
    r.hopfenanzahl = clampInt(doc["hopfenanzahl"] | 0, 0, 6);

    JsonArrayConst rt = doc["rastTemp"];
    JsonArrayConst rz = doc["rastZeit"];
    for (int i = 0; i < (int) rt.size() && i < r.rasten; i++) {
        r.rastTemp[i + 1] = rt[i] | 0;
        r.rastZeit[i + 1] = rz[i] | 0;
    }
    JsonArrayConst hz = doc["hopfenZeit"];
    for (int i = 0; i < (int) hz.size() && i < r.hopfenanzahl; i++) {
        r.hopfenZeit[i + 1] = hz[i] | 0;
    }
    return true;
}
