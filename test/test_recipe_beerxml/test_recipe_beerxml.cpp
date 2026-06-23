#include <unity.h>
#include <string.h>

#include "recipe.h"
#include "recipe_parse.h"

// Mirrors "import samples/Zero Day Session IPA.xml": same recipe as the KBH file.
// The <STYLE><NAME> after the recipe NAME checks first-NAME-wins; the Dry Hop
// (TIME 10080) must be excluded by the USE==Boil filter.
static const char *XML_SAMPLE = R"xml(<?xml version="1.0" encoding="UTF-8"?>
<RECIPES><RECIPE>
<NAME>Zero Day Session IPA</NAME>
<VERSION>1</VERSION>
<TYPE>All Grain</TYPE>
<BOIL_TIME>30</BOIL_TIME>
<STYLE><NAME>Session IPA</NAME></STYLE>
<HOPS>
<HOP><NAME>Motueka</NAME><USE>Boil</USE><TIME>20</TIME></HOP>
<HOP><NAME>Talus</NAME><USE>Boil</USE><TIME>0</TIME></HOP>
<HOP><NAME>Motueka</NAME><USE>Boil</USE><TIME>0</TIME></HOP>
<HOP><NAME>Nelson Sauvin</NAME><USE>Boil</USE><TIME>0</TIME></HOP>
<HOP><NAME>Motueka DH</NAME><USE>Dry Hop</USE><TIME>10080</TIME></HOP>
</HOPS>
<MASH><MASH_STEPS>
<MASH_STEP><NAME>Einmaischen</NAME><TYPE>Infusion</TYPE><STEP_TEMP>70</STEP_TEMP><STEP_TIME>0</STEP_TIME></MASH_STEP>
<MASH_STEP><NAME>verzuckerung</NAME><TYPE>Temperature</TYPE><STEP_TEMP>70</STEP_TEMP><STEP_TIME>60</STEP_TIME></MASH_STEP>
<MASH_STEP><NAME>Abmaischen</NAME><TYPE>Temperature</TYPE><STEP_TEMP>70</STEP_TEMP><STEP_TIME>0</STEP_TIME></MASH_STEP>
</MASH_STEPS></MASH>
</RECIPE></RECIPES>)xml";

void setUp(void) {}
void tearDown(void) {}

void test_xml_name_first_wins(void) {
    Recipe r;
    TEST_ASSERT_TRUE(parseBeerXmlString(XML_SAMPLE, r));
    TEST_ASSERT_EQUAL_STRING("Zero Day Session IPA", r.name);  // not "Session IPA"
    TEST_ASSERT_EQUAL_INT(30, r.kochzeit);
}

void test_xml_mash_steps(void) {
    Recipe r;
    TEST_ASSERT_TRUE(parseBeerXmlString(XML_SAMPLE, r));
    TEST_ASSERT_EQUAL_INT(70, r.maischtemp);
    TEST_ASSERT_EQUAL_INT(70, r.endtemp);
    TEST_ASSERT_EQUAL_INT(1, r.rasten);
    TEST_ASSERT_EQUAL_INT(70, r.rastTemp[1]);
    TEST_ASSERT_EQUAL_INT(60, r.rastZeit[1]);
}

void test_xml_boil_hops_only_converted(void) {  // dry hop excluded, times converted
    Recipe r;
    TEST_ASSERT_TRUE(parseBeerXmlString(XML_SAMPLE, r));
    TEST_ASSERT_EQUAL_INT(4, r.hopfenanzahl);   // 4 boil, not 5
    TEST_ASSERT_EQUAL_INT(10, r.hopfenZeit[1]); // 30 - 20
    TEST_ASSERT_EQUAL_INT(30, r.hopfenZeit[2]);
    TEST_ASSERT_EQUAL_INT(30, r.hopfenZeit[3]);
    TEST_ASSERT_EQUAL_INT(30, r.hopfenZeit[4]);
}

void test_xml_rejects_garbage(void) {
    Recipe r;
    TEST_ASSERT_FALSE(parseBeerXmlString("this is not xml", r));
    TEST_ASSERT_FALSE(parseBeerXmlString("<html><body>nope</body></html>", r));
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_xml_name_first_wins);
    RUN_TEST(test_xml_mash_steps);
    RUN_TEST(test_xml_boil_hops_only_converted);
    RUN_TEST(test_xml_rejects_garbage);
    return UNITY_END();
}
