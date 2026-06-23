#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>

#include "global.h"
#include "config.h"
#include "html.h"
#include "web.h"
#include "settings.h"

const char *ap_ssid = APSSID;
const char *ap_password = APPSK;

ESP8266WebServer HTTP(80);

void handle_http() {
    HTTP.handleClient();
    MDNS.update();
}

static void handleConfigGet();
static void handleConfigPost();

void setupWebserver() {
    HTTP.on("/", handleRoot);
    HTTP.on("/data.json", HTTP_GET, [&]() {
        HTTP.sendHeader("Connection", "close");
        HTTP.sendHeader("Access-Control-Allow-Origin", "*");
        return handleDataJson();
    });
    HTTP.on("/config", HTTP_GET, handleConfigGet);
    HTTP.on("/config", HTTP_POST, handleConfigPost);

    HTTP.onNotFound(handleNotFound);
    HTTP.begin();
    MDNS.addService("http", "tcp", 80);
}

void handleNotFound() {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += HTTP.uri();
    message += "\nMethod: ";
    message += (HTTP.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += HTTP.args();
    message += "\n";
    for (uint8_t i = 0; i < HTTP.args(); i++) {
        message += " " + HTTP.argName(i) + ": " + HTTP.arg(i) + "\n";
    }
    HTTP.send(404, "text/plain", message);
}

void handleDataJson() {
    String title;
    String data;
    String title2 = "Details";

    char jetzt[10];
    snprintf(jetzt, sizeof(jetzt), "%02i:%02i", (stunden * 60) + minuten, sekunden);

    switch (modus) {
        case HAUPTSCHIRM:
            title = F("Hauptmenu");
            break;

        case MANUELL:
            title = F("Maischen: manuell");
            break;

        case BRAUMEISTERRUFALARM:
        case BRAUMEISTERRUF:
            title = F("Rufalarm");
            break;

        case EINGABE_RAST_ANZ:
            title = F("Maisch-Automatik: Eingabe");
            title2 = F("Rasteneingabe");
            data = F("<li>Anzahl: ");
            data += rasten;
            data += F("</li>");
            break;

        case EINGABE_MAISCHTEMP:
            title = F("Maisch-Automatik: Eingabe");
            title2 = F("Maischetemperatur");
            data = F("<li>Einmaischen bei ");
            data += maischtemp;
            data += F("&deg;C</li>");
            break;

        case EINGABE_RAST_TEMP:
            title = F("Maisch-Automatik: Eingabe");
            title2 = F("Rast ");
            title2 += x;
            title2 += F(" von ");
            title2 += rasten;

            data = F("<li>Rasttemperatur: ");
            data += rastTemp[x];
            data += F("&deg;C</li>");
            break;

        case EINGABE_RAST_ZEIT:
            title = F("Maisch-Automatik: Eingabe");
            title2 = F("Rast ");
            title2 += x;
            title2 += F(" von ");
            title2 += rasten;

            data = F("<li>Rasttemperatur: ");
            data += rastTemp[x];
            data += F("&deg;C</li>");

            data += F("<li>Rastzeit: ");
            data += rastZeit[x];
            data += F(" min.</li>");
            break;

        case EINGABE_ENDTEMP:
            title = F("Maisch-Automatik: Eingabe");
            title2 = F("Endtemperatur: ");
            data = F("<li>Abmaischen bei ");
            data += endtemp;
            data += F("&deg;C</li>");
            break;

        case AUTO_START:
            title = F("Maisch-Automatik: Start?");
            break;

        case AUTO_MAISCHTEMP:
            title = F("Maisch-Automatik");
            title2 = F("Aufheizen bis zum Einmaischen");
            data = F("<li>Einmaischen bei ");
            data += maischtemp;
            data += F("&deg;C</li>");
            break;

        case AUTO_RAST_TEMP:
            title = F("Maisch-Automatik");  // x (rasten), rastTemp[x]
            title2 = F("Rast ");
            title2 += x;
            title2 += F(" von ");
            title2 += rasten;
            data += F("<li>Aufheizen auf ");
            data += rastTemp[x];
            data += F("&deg;C</li>");
            break;

        case AUTO_RAST_ZEIT:
            title = F("Maisch-Automatik"); // x (rasten), rastZeit[x], minuten, sekunden
            title2 = F("Rast ");
            title2 += x;
            title2 += F(" von ");
            title2 += rasten;
            data += F("<li>");
            data += jetzt;
            data += F(" von ");
            data += rastZeit[x];
            data += F(" min.</li>");
            break;

        case AUTO_ENDTEMP:
            title = F("Maisch-Automatik");
            title2 = F("Aufheizen bis zum Abmaischen");
            data = F("<li>Abmaischen bei ");
            data += endtemp;
            data += F("&deg;C</li>");
            break;

        case KOCHEN:
            title = F("Kochen: Eingabe Kochzeit ");
            title += kochzeit;
            title += " min.";
            break;

        case EINGABE_HOPFENGABEN_ANZAHL:
            title = F("Kochen: Eingabe Anzahl Hopfengaben ");
            title += hopfenanzahl;
            break;

        case EINGABE_HOPFENGABEN_ZEIT:
            title = F("Kochen: Eingabe Hopfenzeit");
            break;

        case KOCHEN_START_FRAGE:
            title = F("Kochen: Warten auf Start");
            break;

        case KOCHEN_AUFHEIZEN:
            title = F("Kochen: Aufheizen");
            break;

        case KOCHEN_AUTO_LAUF:// x (hopfenanzahl), hopfenZeit[x], minuten, sekunden, kochzeit
            title = F("Kochen");

            title2 = "Kochzeit gesamt: ";
            title2 += kochzeit;
            title2 += " min";

            data = "<li>";
            data += x;
            data += ". Hopfengabe bei ";
            data += hopfenZeit[x];
            data += " min</li>";

            data += "<li>Aktuell: ";
            data += jetzt;
            data += " min</li>";
            break;

        default:
            title = F("Modus: ");
            title += modus;
    }

    DynamicJsonDocument json(1024);

    json["title"] = title;
    json["title2"] = title2;
    json["temp_ist"] = isttemp;
    json["temp_soll"] = sollwert;
    json["heizung"] = heizung ? "an" : "aus";

    json["data"] = data;

    json["modus"] = static_cast<int>(modus);
    json["rufmodus"] = static_cast<int>(rufmodus);
    json["regelung"] = static_cast<int>(regelung);

    json["maischtemp"] = maischtemp;
    json["rast_anzahl"] = rasten;
    json["rast_nr"] = x;
    json["rast_temp"] = rastTemp[x];
    json["rast_zeit_soll"] = rastZeit[x];
    json["rast_zeit_ist"] = jetzt;

    json["timer_soll"] = timer;
    json["timer_ist"] = jetzt;

    json["kochzeit"] = kochzeit;
    json["hopfenanzahl"] = hopfenanzahl;

    String message = "";
    serializeJson(json, message);

    HTTP.send(200, "application/json;charset=utf-8", message);
}

void handleRoot() {
    HTTP.send_P(200, "text/html", PAGE_Kochen);
}

bool setupWIFI() {
    WiFi.mode(WIFI_AP_STA);

    WiFi.softAP(ap_ssid);                       // always on, for first-time setup

    if (strlen(settings.sta_ssid)) {            // join home network when configured
        WiFi.begin(settings.sta_ssid, settings.sta_pass);
    }

    MDNS.begin("bk");
    delay(10);

#ifdef DEBUG
    Serial.print(F("AP IP: "));
    Serial.println(WiFi.softAPIP());
    Serial.print(F("STA SSID: "));
    Serial.println(settings.sta_ssid);
#endif

    return false;
}

static void handleConfigGet() {
    String h = F("<!DOCTYPE html><html><head><meta charset='utf-8'>"
                 "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                 "<title>BrauKnecht Config</title></head><body><h2>Einstellungen</h2>"
                 "<form method='POST' action='/config'>"
                 "WLAN SSID:<br><input name='sta_ssid' value='");
    h += settings.sta_ssid;
    h += F("'><br>WLAN Passwort:<br><input name='sta_pass' type='password'>"
           "<br>MQTT Host:<br><input name='mqtt_host' value='");
    h += settings.mqtt_host;
    h += F("'><br>MQTT Port:<br><input name='mqtt_port' value='");
    h += settings.mqtt_port;
    h += F("'><br>MQTT User:<br><input name='mqtt_user' value='");
    h += settings.mqtt_user;
    h += F("'><br>MQTT Passwort:<br><input name='mqtt_pass' type='password'><br><br>"
           "<input type='submit' value='Speichern &amp; Neustart'></form></body></html>");
    HTTP.send(200, "text/html; charset=utf-8", h);
}

static void handleConfigPost() {
    strlcpy(settings.sta_ssid, HTTP.arg("sta_ssid").c_str(), sizeof(settings.sta_ssid));
    strlcpy(settings.mqtt_host, HTTP.arg("mqtt_host").c_str(), sizeof(settings.mqtt_host));
    strlcpy(settings.mqtt_user, HTTP.arg("mqtt_user").c_str(), sizeof(settings.mqtt_user));
    settings.mqtt_port = HTTP.arg("mqtt_port").toInt();
    // keep existing passwords if the field was left blank
    if (HTTP.arg("sta_pass").length()) {
        strlcpy(settings.sta_pass, HTTP.arg("sta_pass").c_str(), sizeof(settings.sta_pass));
    }
    if (HTTP.arg("mqtt_pass").length()) {
        strlcpy(settings.mqtt_pass, HTTP.arg("mqtt_pass").c_str(), sizeof(settings.mqtt_pass));
    }

    saveSettings(settings);

    HTTP.send(200, "text/html; charset=utf-8",
              F("<html><body>Gespeichert. Neustart&hellip;</body></html>"));
    delay(1000);
    ESP.restart();
}
