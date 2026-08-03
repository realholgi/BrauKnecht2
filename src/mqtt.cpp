#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "mqtt.h"
#include "settings.h"
#include "global.h"

static WiFiClient espClient;
static PubSubClient client(espClient);

static char nodeId[20];          // brauknecht_aabbcc
static char stateTopic[40];      // brauknecht/<nodeId>/state
static char availTopic[44];      // brauknecht/<nodeId>/availability
static unsigned long lastPublish = 0;
static unsigned long lastReconnect = 0;

constexpr unsigned long PUBLISH_INTERVAL = 3000;
constexpr unsigned long RECONNECT_INTERVAL = 5000;

void mqttSetup() {
    snprintf(nodeId, sizeof(nodeId), "brauknecht_%06x", ESP.getChipId());
    snprintf(stateTopic, sizeof(stateTopic), "brauknecht/%s/state", nodeId);
    snprintf(availTopic, sizeof(availTopic), "brauknecht/%s/availability", nodeId);
    client.setBufferSize(512);   // HA discovery payloads exceed the 256-byte default
    client.setSocketTimeout(1);  // keep a failed connect well under the 2s watchdog
    client.setServer(settings.mqtt_host, settings.mqtt_port);
}

// One Home-Assistant MQTT-discovery config message (retained) per entity.
static void publishConfig(const char *component, const char *key, const char *name,
                          const char *unit, const char *devClass) {
    char topic[100];
    snprintf(topic, sizeof(topic), "homeassistant/%s/%s/%s/config", component, nodeId, key);

    char uniq[40];
    snprintf(uniq, sizeof(uniq), "%s_%s", nodeId, key);
    char tmpl[40];
    snprintf(tmpl, sizeof(tmpl), "{{ value_json.%s }}", key);

    JsonDocument doc;
    doc["name"] = name;
    doc["uniq_id"] = uniq;
    doc["stat_t"] = stateTopic;
    doc["avty_t"] = availTopic;   // HA marks the entity unavailable on LWT
    doc["val_tpl"] = tmpl;
    if (unit) {
        doc["unit_of_meas"] = unit;
    }
    if (devClass) {
        doc["dev_cla"] = devClass;
    }
    JsonObject dev = doc["dev"].to<JsonObject>();
    dev["ids"].to<JsonArray>().add(nodeId);
    dev["name"] = "BrauKnecht";
    dev["mdl"] = "BrauKnecht2";
    dev["mf"] = "realholgi";

    char buf[512];
    size_t len = serializeJson(doc, buf, sizeof(buf));
    client.publish(topic, reinterpret_cast<const uint8_t *>(buf), len, true);
}

static void publishDiscovery() {
    publishConfig("sensor", "isttemp", "Ist-Temperatur", "°C", "temperature");
    publishConfig("sensor", "sollwert", "Soll-Temperatur", "°C", "temperature");
    publishConfig("binary_sensor", "heizung", "Heizung", nullptr, "heat");
    publishConfig("sensor", "modus", "Modus", nullptr, nullptr);
}

static void publishState() {
    JsonDocument doc;
    doc["isttemp"] = isttemp;
    doc["sollwert"] = sollwert;
    doc["heizung"] = heizung ? "ON" : "OFF";
    doc["modus"] = static_cast<int>(modus);

    char buf[128];
    size_t len = serializeJson(doc, buf, sizeof(buf));
    client.publish(stateTopic, reinterpret_cast<const uint8_t *>(buf), len);
}

static void reconnect() {
    const char *user = strlen(settings.mqtt_user) ? settings.mqtt_user : nullptr;
    const char *pass = strlen(settings.mqtt_user) ? settings.mqtt_pass : nullptr;
    // Last-Will: broker publishes "offline" if we drop, so HA marks us away.
    bool ok = client.connect(nodeId, user, pass, availTopic, 0, true, "offline");
    if (ok) {
        client.publish(availTopic, "online", true);   // birth message (retained)
        publishDiscovery();
        publishState();
    }
}

void mqttLoop() {
    if (WiFi.status() != WL_CONNECTED || !strlen(settings.mqtt_host)) {
        return;
    }

    if (!client.connected()) {
        unsigned long now = millis();
        if (now - lastReconnect >= RECONNECT_INTERVAL) {
            lastReconnect = now;
            reconnect();
        }
        return;
    }

    client.loop();

    unsigned long now = millis();
    if (now - lastPublish >= PUBLISH_INTERVAL) {
        lastPublish = now;
        publishState();
    }
}
