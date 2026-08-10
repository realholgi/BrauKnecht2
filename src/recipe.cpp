#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#include "recipe.h"
#include "recipe_parse.h"
#include "global.h"
#include "input.h"

void applyRecipe(const Recipe &r) {
    snprintf(recipeName, RECIPE_NAME_CAPACITY, "%s", r.name);
    maischtemp = r.maischtemp;
    rasten = r.rasten;
    for (int i = 1; i <= r.rasten; i++) {
        rastTemp[i] = r.rastTemp[i];
        rastZeit[i] = r.rastZeit[i];
    }
    endtemp = r.endtemp;
    kochzeit = r.kochzeit;
    hopfenanzahl = r.hopfenanzahl;
    for (int i = 1; i <= r.hopfenanzahl; i++) {
        hopfenZeit[i] = r.hopfenZeit[i];
    }
}

// Inverse of applyRecipe: pack the brewing globals into a Recipe (for the web view).
Recipe currentRecipe() {
    Recipe r{};
    snprintf(r.name, sizeof(r.name), "%s", recipeName);
    r.maischtemp = maischtemp;
    r.rasten = rasten;
    for (int i = 1; i <= rasten && i <= 7; i++) {
        r.rastTemp[i] = rastTemp[i];
        r.rastZeit[i] = rastZeit[i];
    }
    r.endtemp = endtemp;
    r.kochzeit = kochzeit;
    r.hopfenanzahl = hopfenanzahl;
    for (int i = 1; i <= hopfenanzahl && i <= 6; i++) {
        r.hopfenZeit[i] = hopfenZeit[i];
    }
    return r;
}

bool parseKbhStream(Stream &in, Recipe &r) {
    JsonDocument filter;
    buildKbhFilter(filter);

    JsonDocument doc;
    if (deserializeJson(doc, in, DeserializationOption::Filter(filter))) {
        return false;
    }
    return parseKbhDoc(doc, r);
}

bool parseBeerXmlStream(Stream &in, Recipe &r) {
    // ponytail: buffer the file (a few KB) then scan in memory — far smaller
    // than a DOM, and lets the scanner stay a pure, testable string function.
    String xml;
    xml.reserve(2048);
    char buf[257];
    while (in.available() && xml.length() < 16384) {
        int got = in.readBytes(buf, sizeof(buf) - 1);
        if (got <= 0) break;
        buf[got] = '\0';
        xml += buf;
    }
    return parseBeerXmlString(xml.c_str(), r);
}

bool saveRecipe(const Recipe &r) {
    EncoderTimerGuard guard;  // pause timer1 ISR during the flash write
    const char *temporaryPath = "/recipe.json.tmp";
    char buf[512];
    const size_t len = recipeToJson(r, buf, sizeof(buf));
    if (len == 0) {
        LittleFS.remove(temporaryPath);
        return false;
    }

    LittleFS.remove(temporaryPath);
    File f = LittleFS.open(temporaryPath, "w");
    if (!f) {
        return false;
    }
    const size_t written = f.write(reinterpret_cast<const uint8_t *>(buf), len);
    f.close();
    if (written != len) {
        LittleFS.remove(temporaryPath);
        return false;
    }
    if (!LittleFS.rename(temporaryPath, "/recipe.json")) {
        LittleFS.remove(temporaryPath);
        return false;
    }
    return true;
}

bool loadRecipe(Recipe &r) {
    File f = LittleFS.open("/recipe.json", "r");
    if (!f) {
        return false;
    }
    char buf[512];
    size_t len = f.readBytes(buf, sizeof(buf) - 1);
    buf[len] = '\0';
    f.close();
    return recipeFromJson(buf, r);
}
