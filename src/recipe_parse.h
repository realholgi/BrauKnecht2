#pragma once

#include <ArduinoJson.h>

#include "recipe.h"

// Pure recipe logic — no hardware, unit-tested on native.

// ArduinoJson filter that keeps only the KBH fields we map, so the full export
// never has to sit in RAM when streaming from a file.
void buildKbhFilter(JsonDocument &filter);

// Map a (filtered or full) Kleiner Brauhelfer document into a Recipe.
// Mash plan: first step = Einmaischen (maischtemp), last = Abmaischen (endtemp),
// middle steps = rests. Hop times are converted from "minutes before boil end"
// (KBH) to "minutes after boil start" (BrauKnecht): hopfenZeit = kochzeit - Zeit.
bool parseKbhDoc(const JsonDocument &doc, Recipe &r);

// Map a BeerXML document (in-memory string) into a Recipe. Subset scanner:
// RECIPE/NAME, BOIL_TIME, MASH_STEP temp/time, HOP TIME where USE == Boil.
bool parseBeerXmlString(const char *xml, Recipe &r);

// Our own persistence format (round-trips through /recipe.json).
size_t recipeToJson(const Recipe &r, char *buf, size_t n);
bool   recipeFromJson(const char *json, Recipe &r);
