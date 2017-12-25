#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>

#include "global.h"
#include "config.h"
#include "html.h"
#include "web.h"

ESP8266WebServer HTTP(80);

void handle_http() {
    HTTP.handleClient();
    MDNS.update();
}

void setupWebserver() {
    HTTP.on("/", handleRoot);
    HTTP.on("/data.json", HTTP_GET, [&]() {
        HTTP.sendHeader("Connection", "close");
        HTTP.sendHeader("Access-Control-Allow-Origin", "*");
        return handleDataJson();
    });

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
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();

    String title;
    String data;
    String title2 = "Details";

    char jetzt[10];
    sprintf(jetzt, "%02i:%02i", (stunden * 60) + minuten, sekunden);

    switch (modus) {
        case HAUPTSCHIRM:
            title = F("Hauptmenu");
            break;

        case MAISCHEN:
            title = F("Maischmenu");
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

        case EINGABE_BRAUMEISTERRUF:
            title = F("Maisch-Automatik");
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

            data += "<li>Ruf:  ";
            switch (braumeister[x]) {
                case BM_ALARM_AUS:
                    data += F("nein");
                    break;

                case BM_ALARM_WAIT:
                    data += F("anhalten");
                    break;

                case BM_ALARM_SIGNAL:
                    data += F("Signal");
                    break;

                default:
                    data += braumeister[x];
            }
            data += F("</li>");
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
            data += F("&deg;C<li>");
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

    json["title"] = title;
    json["title2"] = title2;
    json["temp_ist"] = isttemp;
    json["temp_soll"] = sollwert;
    json["heizung"] = heizung ? "an" : "aus";

    json["data"] = data;

    json["modus"] = (int) modus;
    json["rufmodus"] = (int) rufmodus;

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

//    JsonArray &all_rast_temp = json.createNestedArray("all_rast_temp");
//    all_rast_temp.copyFrom(rastTemp);
//    all_rast_temp.remove(0);
//
//    JsonArray &all_rast_zeit = json.createNestedArray("all_rast_zeit");
//    all_rast_zeit.copyFrom(rastZeit);
//    all_rast_zeit.remove(0);
//
//    JsonArray &all_hopfen_zeit = json.createNestedArray("all_hopfen_zeit");
//    all_hopfen_zeit.copyFrom(hopfenZeit);
//    all_hopfen_zeit.remove(0);

    String message = "";
    json.printTo(message);
    HTTP.send(200, "application/json;charset=utf-8", message);
}

void handleRoot() {
    HTTP.send_P(200, "text/html", PAGE_Kochen);
}

bool setupWIFI() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASS);
    MDNS.begin("bk");

    Serial.println(F("Enabling WIFI"));
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(F("WiFi connected"));
        Serial.print(F("IP address: "));
        Serial.println(WiFi.localIP());

        Serial.print("signal strength (RSSI):");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
        return true;
    } else {
        Serial.println(F("Can not connect to WiFi station. Go into AP mode."));
        WiFi.mode(WIFI_AP);

        delay(10);

        WiFi.softAP("bk", "bk");

        Serial.print(F("IP address: "));
        Serial.println(WiFi.softAPIP());
    }
    return false;
}