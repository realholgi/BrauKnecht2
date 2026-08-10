#pragma once

#include <stddef.h>
#include <stdint.h>

constexpr size_t RECIPE_NAME_CAPACITY = 40;
constexpr int RECIPE_MASH_TEMP_MIN = 10;
constexpr int RECIPE_MASH_TEMP_MAX = 105;
constexpr int RECIPE_REST_COUNT_MIN = 1;
constexpr int RECIPE_REST_COUNT_MAX = 7;
constexpr int RECIPE_REST_TEMP_MIN = 10;
constexpr int RECIPE_REST_TEMP_MAX = 105;
constexpr int RECIPE_REST_DURATION_MIN = 1;
constexpr int RECIPE_REST_DURATION_MAX = 99;
constexpr int RECIPE_MASH_OUT_TEMP_MIN = 10;
constexpr int RECIPE_MASH_OUT_TEMP_MAX = 80;
constexpr int RECIPE_BOIL_DURATION_MIN = 20;
constexpr int RECIPE_BOIL_DURATION_MAX = 180;
constexpr int RECIPE_HOP_COUNT_MIN = 1;
constexpr int RECIPE_HOP_COUNT_MAX = 6;
constexpr int RECIPE_HOP_TIME_MIN = 0;

// A brewing recipe — mirrors the brewing globals in global.h.
// Arrays are 1-indexed (index 0 unused), matching rastTemp[]/rastZeit[]/hopfenZeit[].
struct Recipe {
    char name[RECIPE_NAME_CAPACITY];
    int  maischtemp;        // Einmaischen temperature
    int  rasten;            // number of rests (1..7)
    int  rastTemp[RECIPE_REST_COUNT_MAX + 1];       // [1..rasten]
    int  rastZeit[RECIPE_REST_COUNT_MAX + 1];       // [1..rasten], minutes
    int  endtemp;           // Abmaischen temperature
    int  kochzeit;          // boil time, minutes
    int  hopfenanzahl;      // number of boil hop additions (1..6)
    int  hopfenZeit[RECIPE_HOP_COUNT_MAX + 1];      // [1..hopfenanzahl], minutes AFTER boil start
};

enum class RecipeValidationError : uint8_t {
    None,
    EmptyName,
    InvalidName,
    MashTemperature,
    RestCount,
    RestTemperature,
    RestDuration,
    MashOutTemperature,
    BoilDuration,
    HopCount,
    HopTime
};

struct RecipeValidationResult {
    RecipeValidationError error;
    uint8_t index;
};

RecipeValidationResult validateRecipe(const Recipe &recipe);

class Stream;

void applyRecipe(const Recipe &r);          // write into the brewing globals
Recipe currentRecipe();                     // read the brewing globals back into a Recipe
bool parseKbhStream(Stream &in, Recipe &r);     // Kleiner Brauhelfer JSON from a file/stream
bool parseBeerXmlStream(Stream &in, Recipe &r); // BeerXML from a file/stream
bool saveRecipe(const Recipe &r);           // -> /recipe.json
bool loadRecipe(Recipe &r);                 // <- /recipe.json
