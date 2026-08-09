#include <unity.h>

#include "brew_status.h"

namespace {
int rests[] = {0, 30, 45};

BrewStatus status(MODUS mode, int x, unsigned long elapsed, bool hold = false,
                  MODUS holdMode = HAUPTSCHIRM, int holdX = 0, int restCount = 2,
                  int hops = 2, int boilMinutes = 60) {
    return brewStatus({mode, holdMode, x, holdX, restCount, hops, rests, boilMinutes, elapsed, hold});
}
}

void setUp(void) {}
void tearDown(void) {}

void test_timed_rest_uses_recipe_row_and_seconds(void) {
    const BrewStatus value = status(AUTO_RAST_ZEIT, 2, 14UL * 60UL + 32UL);
    TEST_ASSERT_EQUAL_INT(2, value.activeStepIndex);
    TEST_ASSERT_EQUAL_STRING("2. Rast", value.activeStepLabel);
    TEST_ASSERT_TRUE(value.timed);
    TEST_ASSERT_EQUAL_UINT32(2700, value.totalSeconds);
    TEST_ASSERT_EQUAL_UINT32(872, value.elapsedSeconds);
    TEST_ASSERT_EQUAL_UINT32(1828, value.remainingSeconds);
}

void test_timed_boil_uses_boil_row_not_hop_counter(void) {
    const BrewStatus value = status(KOCHEN_AUTO_LAUF, 2, 600);
    TEST_ASSERT_EQUAL_INT(4, value.activeStepIndex);
    TEST_ASSERT_EQUAL_STRING("Kochen", value.activeStepLabel);
    TEST_ASSERT_TRUE(value.timed);
    TEST_ASSERT_EQUAL_UINT32(3600, value.totalSeconds);
    TEST_ASSERT_EQUAL_UINT32(3000, value.remainingSeconds);
}

void test_non_timed_alarm_and_invalid_states_have_no_highlight(void) {
    TEST_ASSERT_EQUAL_INT(-1, status(AUTO_RAST_TEMP, 1, 0).activeStepIndex);
    TEST_ASSERT_EQUAL_INT(-1, status(BRAUMEISTERRUFALARM, 1, 0).activeStepIndex);
    TEST_ASSERT_EQUAL_INT(-1, status(AUTO_RAST_ZEIT, 3, 0).activeStepIndex);
}

void test_hold_keeps_captured_step_label_but_not_timer(void) {
    const BrewStatus value = status(BRAUVORGANG_HALT, 0, 120, true, AUTO_RAST_ZEIT, 1);
    TEST_ASSERT_EQUAL_INT(1, value.activeStepIndex);
    TEST_ASSERT_EQUAL_STRING("HOLD: 1. Rast", value.activeStepLabel);
    TEST_ASSERT_FALSE(value.timed);
}

void test_remaining_time_clamps_at_zero_and_zero_recipe_counts_work(void) {
    TEST_ASSERT_EQUAL_UINT32(0, status(AUTO_RAST_ZEIT, 1, 99999).remainingSeconds);
    TEST_ASSERT_EQUAL_INT(2, status(KOCHEN_AUTO_LAUF, 1, 0, false, HAUPTSCHIRM, 0, 0, 0).activeStepIndex);
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_timed_rest_uses_recipe_row_and_seconds);
    RUN_TEST(test_timed_boil_uses_boil_row_not_hop_counter);
    RUN_TEST(test_non_timed_alarm_and_invalid_states_have_no_highlight);
    RUN_TEST(test_hold_keeps_captured_step_label_but_not_timer);
    RUN_TEST(test_remaining_time_clamps_at_zero_and_zero_recipe_counts_work);
    return UNITY_END();
}
