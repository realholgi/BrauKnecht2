#include <unity.h>
#include <string.h>

#include "display.h"
#include "display_format.h"

void setUp(void) {}
void tearDown(void) {}

void test_alignX_left(void) {
    TEST_ASSERT_EQUAL_INT(0, alignX(LEFT, 5));
}
void test_alignX_explicit(void) {
    TEST_ASSERT_EQUAL_INT(3, alignX(3, 5));
}
void test_alignX_right(void) {
    TEST_ASSERT_EQUAL_INT(DISPLAY_SIZE_X - 5, alignX(RIGHT, 5));
}
void test_alignX_center(void) {
    TEST_ASSERT_EQUAL_INT((DISPLAY_SIZE_X - 6) / 2, alignX(CENTER, 6));
}
void test_alignX_clamp_low(void) {
    TEST_ASSERT_EQUAL_INT(0, alignX(-3, 2));
}
void test_alignX_clamp_high(void) {
    TEST_ASSERT_EQUAL_INT(DISPLAY_SIZE_X - 1, alignX(50, 2));
}
void test_alignX_right_overflow(void) {  // string wider than display -> clamp to 0
    TEST_ASSERT_EQUAL_INT(0, alignX(RIGHT, 30));
}

void test_formatMinutes_one_digit(void) {
    char b[8];
    formatMinutes(b, sizeof(b), 5);
    TEST_ASSERT_EQUAL_STRING("  5 min", b);
}
void test_formatMinutes_two_digit(void) {
    char b[8];
    formatMinutes(b, sizeof(b), 50);
    TEST_ASSERT_EQUAL_STRING(" 50 min", b);
}
void test_formatMinutes_three_digit(void) {
    char b[8];
    formatMinutes(b, sizeof(b), 500);
    TEST_ASSERT_EQUAL_STRING("500 min", b);
}
void test_formatMinutes_clamp_high(void) {
    char b[8];
    formatMinutes(b, sizeof(b), 1500);
    TEST_ASSERT_EQUAL_STRING("999 min", b);
}
void test_formatMinutes_clamp_low(void) {
    char b[8];
    formatMinutes(b, sizeof(b), -5);
    TEST_ASSERT_EQUAL_STRING("  0 min", b);
}

// window of 3 rows over the rest/hop list
void test_window_fits_no_scroll(void) {
    TEST_ASSERT_EQUAL_INT(1, listWindowStart(1, 3, 3));
    TEST_ASSERT_EQUAL_INT(1, listWindowStart(3, 3, 3));
    TEST_ASSERT_EQUAL_INT(1, listWindowStart(2, 2, 3));  // fewer items than window
}
void test_window_cursor_at_start(void) {
    TEST_ASSERT_EQUAL_INT(1, listWindowStart(1, 7, 3));
    TEST_ASSERT_EQUAL_INT(1, listWindowStart(2, 7, 3));
}
void test_window_cursor_middle(void) {
    TEST_ASSERT_EQUAL_INT(3, listWindowStart(4, 7, 3));  // window [3,4,5], cursor centred
}
void test_window_cursor_at_end(void) {
    TEST_ASSERT_EQUAL_INT(5, listWindowStart(7, 7, 3));  // clamped: window [5,6,7]
    TEST_ASSERT_EQUAL_INT(5, listWindowStart(6, 7, 3));
}
void test_window_cursor_always_visible(void) {
    for (int visible = 2; visible <= 3; visible++) {
        for (int count = 1; count <= 7; count++) {
            for (int cur = 1; cur <= count; cur++) {
                int s = listWindowStart(cur, count, visible);
                TEST_ASSERT_TRUE(cur >= s && cur <= s + visible - 1);  // cursor inside window
                TEST_ASSERT_TRUE(s >= 1);                              // never before the list
            }
        }
    }
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_alignX_left);
    RUN_TEST(test_alignX_explicit);
    RUN_TEST(test_alignX_right);
    RUN_TEST(test_alignX_center);
    RUN_TEST(test_alignX_clamp_low);
    RUN_TEST(test_alignX_clamp_high);
    RUN_TEST(test_alignX_right_overflow);
    RUN_TEST(test_formatMinutes_one_digit);
    RUN_TEST(test_formatMinutes_two_digit);
    RUN_TEST(test_formatMinutes_three_digit);
    RUN_TEST(test_formatMinutes_clamp_high);
    RUN_TEST(test_formatMinutes_clamp_low);
    RUN_TEST(test_window_fits_no_scroll);
    RUN_TEST(test_window_cursor_at_start);
    RUN_TEST(test_window_cursor_middle);
    RUN_TEST(test_window_cursor_at_end);
    RUN_TEST(test_window_cursor_always_visible);
    return UNITY_END();
}
