#pragma once

// A brewing recipe — mirrors the brewing globals in global.h.
// Arrays are 1-indexed (index 0 unused), matching rastTemp[]/rastZeit[]/hopfenZeit[].
struct Recipe {
    char name[40];
    int  maischtemp;        // Einmaischen temperature
    int  rasten;            // number of rests (1..7)
    int  rastTemp[8];       // [1..rasten]
    int  rastZeit[8];       // [1..rasten], minutes
    int  endtemp;           // Abmaischen temperature
    int  kochzeit;          // boil time, minutes
    int  hopfenanzahl;      // number of boil hop additions (1..6)
    int  hopfenZeit[7];     // [1..hopfenanzahl], minutes AFTER boil start
};

class Stream;

void applyRecipe(const Recipe &r);          // write into the brewing globals
bool parseKbhStream(Stream &in, Recipe &r);     // Kleiner Brauhelfer JSON from a file/stream
bool parseBeerXmlStream(Stream &in, Recipe &r); // BeerXML from a file/stream
bool saveRecipe(const Recipe &r);           // -> /recipe.json
bool loadRecipe(Recipe &r);                 // <- /recipe.json
