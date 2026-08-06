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

void test_mode_values_remain_published_contract(void) {
    TEST_ASSERT_EQUAL_INT(1, MANUELL);
    TEST_ASSERT_EQUAL_INT(3, SETUP_MENU);
    TEST_ASSERT_EQUAL_INT(16, BRAUMEISTERRUFALARM);
    TEST_ASSERT_EQUAL_INT(28, SETUP_AP);
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_rufalarm_modes_are_alarm);
    RUN_TEST(test_non_alarm_modes_are_not_alarm);
    RUN_TEST(test_mode_values_remain_published_contract);
    return UNITY_END();
}
