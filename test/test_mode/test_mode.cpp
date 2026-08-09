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
    TEST_ASSERT_FALSE(isRufalarmMode(BRAUVORGANG_HALT));
}

void test_mode_status_names_group_internal_states(void) {
    TEST_ASSERT_EQUAL_STRING("Bereit", modeStatusName(HAUPTSCHIRM));
    TEST_ASSERT_EQUAL_STRING("Manuelles Maischen", modeStatusName(MANUELL));
    TEST_ASSERT_EQUAL_STRING("Automatisches Maischen", modeStatusName(AUTO_RAST_ZEIT));
    TEST_ASSERT_EQUAL_STRING("Kochen", modeStatusName(KOCHEN_AUTO_LAUF));
    TEST_ASSERT_EQUAL_STRING("Rufalarm", modeStatusName(BRAUMEISTERRUFALARM));
    TEST_ASSERT_EQUAL_STRING("Einrichtung", modeStatusName(EINGABE_RAST_TEMP));
}

void test_mode_values_remain_published_contract(void) {
    TEST_ASSERT_EQUAL_INT(1, MANUELL);
    TEST_ASSERT_EQUAL_INT(3, SETUP_MENU);
    TEST_ASSERT_EQUAL_INT(5, SETUP_KOCHSCHWELLE);
    TEST_ASSERT_EQUAL_INT(16, BRAUMEISTERRUFALARM);
    TEST_ASSERT_EQUAL_INT(28, SETUP_AP);
}

void test_alarm_reason_wire_values_and_actions_are_stable(void) {
    TEST_ASSERT_EQUAL_STRING("none", rufalarmReasonName(RUFALARM_REASON_NONE));
    TEST_ASSERT_EQUAL_STRING("mash_start", rufalarmReasonName(RUFALARM_REASON_MAISCHSTART));
    TEST_ASSERT_EQUAL_STRING("rest_complete", rufalarmReasonName(RUFALARM_REASON_RASTENDE));
    TEST_ASSERT_EQUAL_STRING("mash_end", rufalarmReasonName(RUFALARM_REASON_MAISCHENDE));
    TEST_ASSERT_EQUAL_STRING("boil_end", rufalarmReasonName(RUFALARM_REASON_KOCHENDE));
    TEST_ASSERT_EQUAL_STRING("sensor_fault", rufalarmReasonName(RUFALARM_REASON_SENSORFEHLER));
    TEST_ASSERT_EQUAL_STRING("alarm_test", rufalarmReasonName(RUFALARM_REASON_ALARMTEST));
    TEST_ASSERT_EQUAL_STRING("acknowledge_at_controller",
                             rufalarmActionName(RUFALARM_REASON_MAISCHSTART, true));
    TEST_ASSERT_EQUAL_STRING("check_sensor_and_acknowledge_at_controller",
                             rufalarmActionName(RUFALARM_REASON_SENSORFEHLER, true));
    TEST_ASSERT_EQUAL_STRING("none", rufalarmActionName(RUFALARM_REASON_SENSORFEHLER, false));
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_rufalarm_modes_are_alarm);
    RUN_TEST(test_non_alarm_modes_are_not_alarm);
    RUN_TEST(test_mode_values_remain_published_contract);
    RUN_TEST(test_mode_status_names_group_internal_states);
    RUN_TEST(test_alarm_reason_wire_values_and_actions_are_stable);
    return UNITY_END();
}
