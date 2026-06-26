#include <unity.h>

#include "global.h"

void setUp(void) {}
void tearDown(void) {}

void test_rufalarm_modes_are_alarm(void) {
    TEST_ASSERT_TRUE(isRufalarmMode(BRAUMEISTERRUFALARM));
    TEST_ASSERT_TRUE(isRufalarmMode(BRAUMEISTERRUF));
}

void test_non_alarm_modes_are_not_alarm(void) {
    TEST_ASSERT_FALSE(isRufalarmMode(HAUPTSCHIRM));
    TEST_ASSERT_FALSE(isRufalarmMode(KOCHEN_START_FRAGE));
    TEST_ASSERT_FALSE(isRufalarmMode(AUTO_RAST_TEMP));
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_rufalarm_modes_are_alarm);
    RUN_TEST(test_non_alarm_modes_are_not_alarm);
    return UNITY_END();
}
