#pragma once

#include <stdint.h>
#include <stddef.h>

// Network / MQTT settings, persisted as /settings.json on LittleFS.
struct Settings {
    char sta_ssid[33];
    char sta_pass[65];
    char mqtt_host[65];
    char mqtt_user[33];
    char mqtt_pass[65];
    uint16_t mqtt_port;
};

// Pure helpers (no hardware) — unit-tested on native.
void   settingsDefaults(Settings &s);
bool   settingsFromJson(const char *json, Settings &s); // false on parse error (s left at defaults)
size_t settingsToJson(const Settings &s, char *buf, size_t n);

// File-backed (LittleFS) — implemented in persistence.cpp.
extern Settings settings;
bool loadSettings(Settings &s);
bool saveSettings(const Settings &s);
