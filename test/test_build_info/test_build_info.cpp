#include <unity.h>
#include <string.h>

#include "build_info.h"

void setUp(void) {}
void tearDown(void) {}

void test_firmware_version_comes_from_manual_version_header(void) {
    TEST_ASSERT_EQUAL_STRING(VERSION_STRING, firmwareVersion());
}

void test_build_info_strings_are_always_present(void) {
    TEST_ASSERT_NOT_NULL(buildGitHash());
    TEST_ASSERT_NOT_NULL(buildTime());
    TEST_ASSERT_NOT_NULL(buildEnvironment());
    TEST_ASSERT_GREATER_THAN(0, strlen(buildGitHash()));
    TEST_ASSERT_GREATER_THAN(0, strlen(buildTime()));
    TEST_ASSERT_GREATER_THAN(0, strlen(buildEnvironment()));
}

void test_build_time_formats_iso_timestamp_for_display(void) {
    char out[24];
    formatBuildTime(out, sizeof(out), "2026-06-26T09:32:23Z");
    TEST_ASSERT_EQUAL_STRING("26.06.2026 09:32 UTC", out);
}

void test_build_time_keeps_unknown_fallback(void) {
    char out[16];
    formatBuildTime(out, sizeof(out), "unknown");
    TEST_ASSERT_EQUAL_STRING("unknown", out);
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_firmware_version_comes_from_manual_version_header);
    RUN_TEST(test_build_info_strings_are_always_present);
    RUN_TEST(test_build_time_formats_iso_timestamp_for_display);
    RUN_TEST(test_build_time_keeps_unknown_fallback);
    return UNITY_END();
}
