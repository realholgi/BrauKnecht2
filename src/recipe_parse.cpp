#include <string.h>

#include "recipe_parse.h"

void buildKbhFilter(JsonDocument &filter) {
    filter["Sud"]["Sudname"] = true;
    filter["Sud"]["Kochdauer"] = true;
    // [0] applies the sub-filter to every element of the array
    filter["Maischplan"][0]["TempRast"] = true;
    filter["Maischplan"][0]["DauerRast"] = true;
    filter["Hopfengaben"][0]["Zeit"] = true;
}

static int clampInt(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

bool parseKbhDoc(const JsonDocument &doc, Recipe &r) {
    JsonObjectConst sud = doc["Sud"];
    if (sud.isNull()) {
        return false; // not a Kleiner Brauhelfer recipe
    }

    memset(&r, 0, sizeof(r));
    strlcpy(r.name, sud["Sudname"] | "", sizeof(r.name));
    r.kochzeit = sud["Kochdauer"] | 0;

    // Mash plan: first = Einmaischen, last = Abmaischen, middle = rests.
    JsonArrayConst mp = doc["Maischplan"].as<JsonArrayConst>();
    int count = mp.size();
    int i = 0;
    for (JsonObjectConst step : mp) {
        int temp = step["TempRast"] | 0;
        int dauer = step["DauerRast"] | 0;
        if (i == 0) {
            r.maischtemp = temp;
        } else if (i == count - 1) {
            r.endtemp = temp;
        } else if (r.rasten < 7) {
            r.rasten++;
            r.rastTemp[r.rasten] = temp;
            r.rastZeit[r.rasten] = dauer;
        }
        i++;
    }
    // Degenerate plan (<3 steps): fall back to a single rest so the brew is runnable.
    if (r.rasten < 1) {
        r.rasten = 1;
        r.rastTemp[1] = r.maischtemp;
        r.rastZeit[1] = 0;
    }

    // Hops: KBH "Zeit" is minutes before boil end -> BrauKnecht counts up from start.
    JsonArrayConst hops = doc["Hopfengaben"].as<JsonArrayConst>();
    for (JsonObjectConst hop : hops) {
        if (r.hopfenanzahl >= 6) break;
        int zeit = hop["Zeit"] | 0;
        r.hopfenanzahl++;
        r.hopfenZeit[r.hopfenanzahl] = clampInt(r.kochzeit - zeit, 0, r.kochzeit);
    }
    if (r.hopfenanzahl < 1) {
        r.hopfenanzahl = 1;
        r.hopfenZeit[1] = 0;
    }

    return true;
}

size_t recipeToJson(const Recipe &r, char *buf, size_t n) {
    DynamicJsonDocument doc(1024);
    doc["name"] = r.name;
    doc["maischtemp"] = r.maischtemp;
    doc["rasten"] = r.rasten;
    doc["endtemp"] = r.endtemp;
    doc["kochzeit"] = r.kochzeit;
    doc["hopfenanzahl"] = r.hopfenanzahl;

    JsonArray rt = doc.createNestedArray("rastTemp");
    JsonArray rz = doc.createNestedArray("rastZeit");
    for (int i = 1; i <= r.rasten; i++) {
        rt.add(r.rastTemp[i]);
        rz.add(r.rastZeit[i]);
    }
    JsonArray hz = doc.createNestedArray("hopfenZeit");
    for (int i = 1; i <= r.hopfenanzahl; i++) {
        hz.add(r.hopfenZeit[i]);
    }
    return serializeJson(doc, buf, n);
}

bool recipeFromJson(const char *json, Recipe &r) {
    memset(&r, 0, sizeof(r));

    DynamicJsonDocument doc(1024);
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
