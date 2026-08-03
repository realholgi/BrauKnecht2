#include <ArduinoJson.h>
#include <string.h>

#include "settings.h"

void settingsDefaults(Settings &s) {
    s.sta_ssid[0] = '\0';
    s.sta_pass[0] = '\0';
    s.mqtt_host[0] = '\0';
    s.mqtt_user[0] = '\0';
    s.mqtt_pass[0] = '\0';
    s.mqtt_port = 1883;
}

bool settingsFromJson(const char *json, Settings &s) {
    settingsDefaults(s);

    JsonDocument doc;
    if (deserializeJson(doc, json)) {
        return false;
    }

    strlcpy(s.sta_ssid, doc["sta_ssid"] | "", sizeof(s.sta_ssid));
    strlcpy(s.sta_pass, doc["sta_pass"] | "", sizeof(s.sta_pass));
    strlcpy(s.mqtt_host, doc["mqtt_host"] | "", sizeof(s.mqtt_host));
    strlcpy(s.mqtt_user, doc["mqtt_user"] | "", sizeof(s.mqtt_user));
    strlcpy(s.mqtt_pass, doc["mqtt_pass"] | "", sizeof(s.mqtt_pass));
    s.mqtt_port = doc["mqtt_port"] | 1883;
    return true;
}

size_t settingsToJson(const Settings &s, char *buf, size_t n) {
    JsonDocument doc;
    doc["sta_ssid"] = s.sta_ssid;
    doc["sta_pass"] = s.sta_pass;
    doc["mqtt_host"] = s.mqtt_host;
    doc["mqtt_user"] = s.mqtt_user;
    doc["mqtt_pass"] = s.mqtt_pass;
    doc["mqtt_port"] = s.mqtt_port;
    return serializeJson(doc, buf, n);
}
