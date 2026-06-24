#include <unity.h>
#include <string.h>
#include <ArduinoJson.h>

#include "recipe.h"
#include "recipe_parse.h"

// Mirrors "import samples/Zero Day Session IPA.json": 3 mash steps
// (Einmaischen 70 / Verzuckerung 70 60min / Abmaischen 70), Kochdauer 30,
// 4 boil hops at KBH "Zeit" 20/0/0/0 (minutes before end). Extra fields are
// included to confirm the filter strips them without breaking positional logic.
static const char *KBH_SAMPLE = R"json({
  "Sud": { "Sudname": "Zero Day Session IPA", "Kochdauer": 30, "IBU": 15 },
  "Maischplan": [
    { "Name": "Einmaischen",    "TempRast": 70, "DauerRast": 0,  "Typ": 0 },
    { "Name": "verzuckerung",   "TempRast": 70, "DauerRast": 60, "Typ": 1 },
    { "Name": "Abmaischen",     "TempRast": 70, "DauerRast": 0,  "Typ": 1 }
  ],
  "Hopfengaben": [
    { "Name": "Motueka",       "Zeit": 20 },
    { "Name": "Talus",         "Zeit": 0 },
    { "Name": "Motueka",       "Zeit": 0 },
    { "Name": "Nelson Sauvin", "Zeit": 0 }
  ]
})json";

void setUp(void) {}
void tearDown(void) {}

static bool parseSample(Recipe &r) {
    StaticJsonDocument<512> filter;
    buildKbhFilter(filter);
    StaticJsonDocument<2048> doc;
    if (deserializeJson(doc, KBH_SAMPLE, DeserializationOption::Filter(filter))) {
        return false;
    }
    return parseKbhDoc(doc, r);
}

void test_kbh_name_and_boil(void) {
    Recipe r;
    TEST_ASSERT_TRUE(parseSample(r));
    TEST_ASSERT_EQUAL_STRING("Zero Day Session IPA", r.name);
    TEST_ASSERT_EQUAL_INT(30, r.kochzeit);
}

void test_kbh_mash_steps(void) {
    Recipe r;
    TEST_ASSERT_TRUE(parseSample(r));
    TEST_ASSERT_EQUAL_INT(70, r.maischtemp);   // first step
    TEST_ASSERT_EQUAL_INT(70, r.endtemp);      // last step
    TEST_ASSERT_EQUAL_INT(1, r.rasten);        // one middle step
    TEST_ASSERT_EQUAL_INT(70, r.rastTemp[1]);
    TEST_ASSERT_EQUAL_INT(60, r.rastZeit[1]);
}

void test_kbh_hops_converted_and_merged(void) {  // hopfenZeit = kochzeit - Zeit; gleiche Zeiten zusammengefasst
    Recipe r;
    TEST_ASSERT_TRUE(parseSample(r));
    // Zeit 20,0,0,0 -> 10,30,30,30 -> drei 30er zu einer Gabe zusammengefasst
    TEST_ASSERT_EQUAL_INT(2, r.hopfenanzahl);
    TEST_ASSERT_EQUAL_INT(10, r.hopfenZeit[1]);
    TEST_ASSERT_EQUAL_INT(30, r.hopfenZeit[2]);
}

void test_kbh_rejects_non_recipe(void) {
    StaticJsonDocument<128> doc;
    deserializeJson(doc, "{\"foo\":1}");
    Recipe r;
    TEST_ASSERT_FALSE(parseKbhDoc(doc, r));
}

void test_recipe_round_trip(void) {
    Recipe in;
    TEST_ASSERT_TRUE(parseSample(in));

    char buf[512];
    size_t n = recipeToJson(in, buf, sizeof(buf));
    TEST_ASSERT_GREATER_THAN(0, n);

    Recipe out;
    TEST_ASSERT_TRUE(recipeFromJson(buf, out));
    TEST_ASSERT_EQUAL_STRING(in.name, out.name);
    TEST_ASSERT_EQUAL_INT(in.maischtemp, out.maischtemp);
    TEST_ASSERT_EQUAL_INT(in.rasten, out.rasten);
    TEST_ASSERT_EQUAL_INT(in.rastTemp[1], out.rastTemp[1]);
    TEST_ASSERT_EQUAL_INT(in.rastZeit[1], out.rastZeit[1]);
    TEST_ASSERT_EQUAL_INT(in.endtemp, out.endtemp);
    TEST_ASSERT_EQUAL_INT(in.kochzeit, out.kochzeit);
    TEST_ASSERT_EQUAL_INT(in.hopfenanzahl, out.hopfenanzahl);
    TEST_ASSERT_EQUAL_INT(in.hopfenZeit[2], out.hopfenZeit[2]);
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_kbh_name_and_boil);
    RUN_TEST(test_kbh_mash_steps);
    RUN_TEST(test_kbh_hops_converted_and_merged);
    RUN_TEST(test_kbh_rejects_non_recipe);
    RUN_TEST(test_recipe_round_trip);
    return UNITY_END();
}
