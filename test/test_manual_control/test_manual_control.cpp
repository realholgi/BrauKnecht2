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

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_manual_setpoint_clamps_to_lower_limit);
    RUN_TEST(test_manual_setpoint_accepts_valid_range);
    RUN_TEST(test_manual_setpoint_clamps_to_upper_limit);
    return UNITY_END();
}
