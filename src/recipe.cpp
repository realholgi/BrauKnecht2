#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#include "recipe.h"
#include "recipe_parse.h"
#include "global.h"

void applyRecipe(const Recipe &r) {
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

bool parseKbhStream(Stream &in, Recipe &r) {
    StaticJsonDocument<512> filter;
    buildKbhFilter(filter);

    DynamicJsonDocument doc(2048);
    if (deserializeJson(doc, in, DeserializationOption::Filter(filter))) {
        return false;
    }
    return parseKbhDoc(doc, r);
}

bool saveRecipe(const Recipe &r) {
    char buf[512];
    size_t n = recipeToJson(r, buf, sizeof(buf));
    File f = LittleFS.open("/recipe.json", "w");
    if (!f) {
        return false;
    }
    f.write(reinterpret_cast<const uint8_t *>(buf), n);
    f.close();
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
