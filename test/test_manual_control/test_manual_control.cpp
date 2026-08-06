#include <unity.h>

#include "manual_control.h"

void setUp(void) {}
void tearDown(void) {}

void test_manual_setpoint_clamps_to_lower_limit(void) {
    TEST_ASSERT_EQUAL_INT(10, clampManualSetpoint(-5));
    TEST_ASSERT_EQUAL_INT(10, clampManualSetpoint(9));
}

void test_manual_setpoint_accepts_valid_range(void) {
    TEST_ASSERT_EQUAL_INT(10, clampManualSetpoint(10));
    TEST_ASSERT_EQUAL_INT(67, clampManualSetpoint(67));
    TEST_ASSERT_EQUAL_INT(100, clampManualSetpoint(100));
}

void test_manual_setpoint_clamps_to_upper_limit(void) {
    TEST_ASSERT_EQUAL_INT(100, clampManualSetpoint(101));
    TEST_ASSERT_EQUAL_INT(100, clampManualSetpoint(250));
}

void test_manual_target_beep_stays_armed_below_target(void) {
    ManualTargetBeepState state;
    armManualTargetBeep(state, 65);

    TEST_ASSERT_FALSE(updateManualTargetBeep(state, 65, 64.9F, 100));
    TEST_ASSERT_TRUE(state.armed);
    TEST_ASSERT_FALSE(state.active);
}

void test_manual_target_beep_pulses_for_five_cycles(void) {
    ManualTargetBeepState state;
    armManualTargetBeep(state, 65);

    TEST_ASSERT_TRUE(updateManualTargetBeep(state, 65, 65.0F, 100));
    TEST_ASSERT_TRUE(updateManualTargetBeep(state, 65, 65.0F, 1099));
    TEST_ASSERT_FALSE(updateManualTargetBeep(state, 65, 65.0F, 1100));
    TEST_ASSERT_FALSE(updateManualTargetBeep(state, 65, 65.0F, 1599));
    TEST_ASSERT_TRUE(updateManualTargetBeep(state, 65, 65.0F, 1600));
    TEST_ASSERT_FALSE(updateManualTargetBeep(state, 65, 65.0F, 7600));
    TEST_ASSERT_FALSE(state.active);
    TEST_ASSERT_FALSE(state.armed);
}

void test_manual_target_beep_does_not_retrigger_unchanged_target(void) {
    ManualTargetBeepState state;
    armManualTargetBeep(state, 65);

    TEST_ASSERT_TRUE(updateManualTargetBeep(state, 65, 65.0F, 100));
    TEST_ASSERT_FALSE(updateManualTargetBeep(state, 65, 65.0F, 7600));
    TEST_ASSERT_FALSE(updateManualTargetBeep(state, 65, 64.0F, 8000));
    TEST_ASSERT_FALSE(updateManualTargetBeep(state, 65, 65.0F, 9000));
}

void test_manual_target_beep_rearms_for_changed_target(void) {
    ManualTargetBeepState state;
    armManualTargetBeep(state, 65);

    TEST_ASSERT_TRUE(updateManualTargetBeep(state, 65, 65.0F, 100));
    TEST_ASSERT_FALSE(updateManualTargetBeep(state, 66, 65.0F, 200));
    TEST_ASSERT_TRUE(state.armed);
    TEST_ASSERT_TRUE(updateManualTargetBeep(state, 66, 66.0F, 300));
}

void test_manual_target_beep_handles_uint32_wraparound(void) {
    ManualTargetBeepState state;
    armManualTargetBeep(state, 65);

    TEST_ASSERT_TRUE(updateManualTargetBeep(state, 65, 65.0F, UINT32_MAX - 499));
    TEST_ASSERT_TRUE(updateManualTargetBeep(state, 65, 65.0F, 499));
    TEST_ASSERT_FALSE(updateManualTargetBeep(state, 65, 65.0F, 500));
    TEST_ASSERT_FALSE(updateManualTargetBeep(state, 65, 65.0F, 7000));
    TEST_ASSERT_FALSE(state.active);
}

void test_manual_mode_entry_rejects_active_automatic_and_cooking(void) {
    TEST_ASSERT_TRUE(canEnterManualMode(HAUPTSCHIRM, REGL_AUS));
    TEST_ASSERT_TRUE(canEnterManualMode(MANUELL, REGL_MAISCHEN));
    TEST_ASSERT_FALSE(canEnterManualMode(AUTO_RAST_TEMP, REGL_MAISCHEN));
    TEST_ASSERT_FALSE(canEnterManualMode(KOCHEN_AUTO_LAUF, REGL_KOCHEN));
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_manual_setpoint_clamps_to_lower_limit);
    RUN_TEST(test_manual_setpoint_accepts_valid_range);
    RUN_TEST(test_manual_setpoint_clamps_to_upper_limit);
    RUN_TEST(test_manual_target_beep_stays_armed_below_target);
    RUN_TEST(test_manual_target_beep_pulses_for_five_cycles);
    RUN_TEST(test_manual_target_beep_does_not_retrigger_unchanged_target);
    RUN_TEST(test_manual_target_beep_rearms_for_changed_target);
    RUN_TEST(test_manual_target_beep_handles_uint32_wraparound);
    RUN_TEST(test_manual_mode_entry_rejects_active_automatic_and_cooking);
    return UNITY_END();
}
