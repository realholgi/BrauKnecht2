#include <unity.h>

#include "temp_filter.h"

void setUp(void) {}
void tearDown(void) {}

void test_stable_reading_passes_through(void) {
    int n = 0;
    TEST_ASSERT_EQUAL_FLOAT(20.0f, filterTemp(20.0f, 20.0f, n));
    TEST_ASSERT_EQUAL_INT(0, n);
}

void test_glitch_debounced_then_accepted(void) {
    int n = 0;
    float v = 20.0f;
    for (int i = 0; i < 5; i++) {           // 5 differing reads are rejected
        v = filterTemp(v, 80.0f, n);
        TEST_ASSERT_EQUAL_FLOAT(20.0f, v);
    }
    TEST_ASSERT_EQUAL_INT(5, n);
    v = filterTemp(v, 80.0f, n);            // the 6th persists -> accepted
    TEST_ASSERT_EQUAL_FLOAT(80.0f, v);
    TEST_ASSERT_EQUAL_INT(0, n);
}

void test_matching_reading_resets_counter(void) {
    int n = 3;
    TEST_ASSERT_EQUAL_FLOAT(20.0f, filterTemp(20.0f, 20.0f, n));
    TEST_ASSERT_EQUAL_INT(0, n);
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_stable_reading_passes_through);
    RUN_TEST(test_glitch_debounced_then_accepted);
    RUN_TEST(test_matching_reading_resets_counter);
    return UNITY_END();
}
