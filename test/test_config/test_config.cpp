#include <unity.h>
#include <string.h>
#include <stdio.h>

#include "settings.h"

void setUp(void) {}
void tearDown(void) {}

void test_defaults(void) {
    Settings s;
    settingsDefaults(s);
    TEST_ASSERT_EQUAL_STRING("", s.sta_ssid);
    TEST_ASSERT_EQUAL_STRING("", s.mqtt_host);
    TEST_ASSERT_EQUAL_UINT16(1883, s.mqtt_port);
}

void test_round_trip(void) {
    Settings in;
    settingsDefaults(in);
    strcpy(in.sta_ssid, "HomeNet");
    strcpy(in.sta_pass, "secret123");
    strcpy(in.mqtt_host, "192.168.1.10");
    strcpy(in.mqtt_user, "brau");
    strcpy(in.mqtt_pass, "pw");
    in.mqtt_port = 8883;

    char buf[512];
    size_t n = settingsToJson(in, buf, sizeof(buf));
    TEST_ASSERT_GREATER_THAN(0, n);

    Settings out;
    TEST_ASSERT_TRUE(settingsFromJson(buf, out));
    TEST_ASSERT_EQUAL_STRING("HomeNet", out.sta_ssid);
    TEST_ASSERT_EQUAL_STRING("secret123", out.sta_pass);
    TEST_ASSERT_EQUAL_STRING("192.168.1.10", out.mqtt_host);
    TEST_ASSERT_EQUAL_STRING("brau", out.mqtt_user);
    TEST_ASSERT_EQUAL_STRING("pw", out.mqtt_pass);
    TEST_ASSERT_EQUAL_UINT16(8883, out.mqtt_port);
}

void test_empty_object_first_boot(void) {  // valid JSON, no fields -> defaults, ok
    Settings s;
    TEST_ASSERT_TRUE(settingsFromJson("{}", s));
    TEST_ASSERT_EQUAL_STRING("", s.sta_ssid);
    TEST_ASSERT_EQUAL_UINT16(1883, s.mqtt_port);
}

void test_malformed_returns_false(void) {  // garbage -> false, but s stays at defaults
    Settings s;
    strcpy(s.sta_ssid, "stale");
    TEST_ASSERT_FALSE(settingsFromJson("not json", s));
    TEST_ASSERT_EQUAL_STRING("", s.sta_ssid);   // defaults applied even on failure
    TEST_ASSERT_EQUAL_UINT16(1883, s.mqtt_port);
}

void test_oversized_field_truncates(void) {  // strlcpy must not overflow fixed buffers
    char json[256];
    snprintf(json, sizeof(json),
             "{\"sta_ssid\":\"%s\"}",
             "0123456789012345678901234567890123456789");  // 40 chars, buffer is 33
    Settings s;
    TEST_ASSERT_TRUE(settingsFromJson(json, s));
    TEST_ASSERT_EQUAL_UINT(sizeof(s.sta_ssid) - 1, strlen(s.sta_ssid));
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_defaults);
    RUN_TEST(test_round_trip);
    RUN_TEST(test_empty_object_first_boot);
    RUN_TEST(test_malformed_returns_false);
    RUN_TEST(test_oversized_field_truncates);
    return UNITY_END();
}
