#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#include "global.h"
#include "config.h"
#include "html.h"
#include "web.h"
#include "settings.h"
#include "recipe.h"

const char *ap_ssid = APSSID;
const char *ap_password = APPSK;

ESP8266WebServer HTTP(80);

void handle_http() {
    HTTP.handleClient();
    MDNS.update();
}

static void handleConfigGet();
static void handleConfigPost();
static void handleRecipeGet();
static void handleRecipeDone();
static void handleRecipeUpload();

void setupWebserver() {
    HTTP.on("/", handleRoot);
    HTTP.on("/style.css", HTTP_GET, []() {
        HTTP.sendHeader("Cache-Control", "max-age=86400");
        HTTP.send_P(200, "text/css", STYLE_CSS);
    });
    HTTP.on("/data.json", HTTP_GET, [&]() {
        HTTP.sendHeader("Connection", "close");
        HTTP.sendHeader("Access-Control-Allow-Origin", "*");
        return handleDataJson();
    });
    HTTP.on("/config", HTTP_GET, handleConfigGet);
    HTTP.on("/config", HTTP_POST, handleConfigPost);
    HTTP.on("/recipe", HTTP_GET, handleRecipeGet);
    HTTP.on("/recipe", HTTP_POST, handleRecipeDone, handleRecipeUpload);

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

// Shared page shell — links the local /style.css so all pages match.
static String pageHead(const char *title) {
    String h = F("<!DOCTYPE html><html lang='de'><head><meta charset='utf-8'>"
                 "<meta name='viewport' content='width=device-width,initial-scale=1,viewport-fit=cover'>"
                 "<link rel='icon' href='data:,'><link rel='stylesheet' href='/style.css'><title>");
    h += title;
    h += F("</title></head><body><div class='wrap'><h1>");
    h += title;
    h += F("</h1>");
    return h;
}

static String pageFoot() {
    return F("<p style='text-align:center'><a href='/'>&larr; Status</a></p></div></body></html>");
}

static void handleConfigGet() {
    // Async scan only — a blocking WiFi.scanNetworks() can outlast the 2s
    // watchdog while the loop is stalled in this handler. We cache the last good
    // result so the list never blanks while a (re)scan is running.
    static String wifiCache;  // scanned <option>s, kept across reloads
    int n = WiFi.scanComplete();
    if (n == -2) {
        WiFi.scanNetworks(true);  // first scan; list fills on the next reload
    } else if (n >= 0) {
        String c;
        for (int i = 0; i < n; i++) {
            String s = WiFi.SSID(i);
            if (s.length() == 0 || s.indexOf('\'') >= 0) continue;  // skip empty/quote SSIDs
            if (s == APSSID || s == settings.sta_ssid) continue;   // own AP / already selected
            String tag = "value='" + s + "'";
            if (c.indexOf(tag) >= 0) continue;                     // same SSID on several APs/channels
            c += F("<option ");
            c += tag;
            c += F(">");
            c += s;
            c += F("</option>");
        }
        wifiCache = c;
        WiFi.scanNetworks(true);  // refresh for the next reload
    }
    // n == -1: scan still running -> keep showing the cached list

    // current SSID first (stays selected even if not in range right now)
    String opts;
    if (strlen(settings.sta_ssid)) {
        opts += F("<option selected value='");
        opts += settings.sta_ssid;
        opts += F("'>");
        opts += settings.sta_ssid;
        opts += F("</option>");
    } else {
        opts += F("<option value=''>&ndash; ausw&auml;hlen &ndash;</option>");
    }
    opts += wifiCache;

    String h = pageHead("Einstellungen");
    h += F("<form class='card' method='POST' action='/config'>"
           "<label>WLAN SSID</label><select name='sta_ssid'>");
    h += opts;
    h += F("</select><label>WLAN Passwort</label><input name='sta_pass' type='password' placeholder='unver&auml;ndert'>"
           "<label>MQTT Host</label><input name='mqtt_host' value='");
    h += settings.mqtt_host;
    h += F("'><label>MQTT Port</label><input name='mqtt_port' inputmode='numeric' value='");
    h += settings.mqtt_port;
    h += F("'><label>MQTT User</label><input name='mqtt_user' value='");
    h += settings.mqtt_user;
    h += F("'><label>MQTT Passwort</label><input name='mqtt_pass' type='password' placeholder='unver&auml;ndert'>"
           "<button class='btn' type='submit'>Speichern &amp; Neustart</button></form>");
    h += pageFoot();
    HTTP.send(200, "text/html; charset=utf-8", h);
}

static void handleConfigPost() {
    strlcpy(settings.sta_ssid, HTTP.arg("sta_ssid").c_str(), sizeof(settings.sta_ssid));
    strlcpy(settings.mqtt_host, HTTP.arg("mqtt_host").c_str(), sizeof(settings.mqtt_host));
    strlcpy(settings.mqtt_user, HTTP.arg("mqtt_user").c_str(), sizeof(settings.mqtt_user));
    settings.mqtt_port = HTTP.arg("mqtt_port").toInt();
    if (!settings.mqtt_port) settings.mqtt_port = 1883;  // blank/0 -> default
    // keep existing passwords if the field was left blank
    if (HTTP.arg("sta_pass").length()) {
        strlcpy(settings.sta_pass, HTTP.arg("sta_pass").c_str(), sizeof(settings.sta_pass));
    }
    if (HTTP.arg("mqtt_pass").length()) {
        strlcpy(settings.mqtt_pass, HTTP.arg("mqtt_pass").c_str(), sizeof(settings.mqtt_pass));
    }

    saveSettings(settings);

    HTTP.send(200, "text/html; charset=utf-8",
              pageHead("Neustart") + F("<div class='card'>Gespeichert. Neustart&hellip;</div>") + pageFoot());
    delay(1000);
    ESP.restart();
}

static File uploadFile;

static void handleRecipeGet() {
    String h = pageHead("Rezept-Import");
    h += F("<div class='card'><h2>Aktuelles Rezept</h2>Einmaischen ");
    h += maischtemp;
    h += F("&deg;C &middot; ");
    h += rasten;
    h += F(" Rasten &middot; ");
    h += kochzeit;
    h += F(" min Kochen</div>"
           "<form class='card' method='POST' action='/recipe' enctype='multipart/form-data'>"
           "<label>Kleiner-Brauhelfer JSON oder BeerXML</label>"
           "<input type='file' name='recipe' accept='.json,.xml'>"
           "<button class='btn' type='submit'>Importieren</button></form>");
    h += pageFoot();
    HTTP.send(200, "text/html; charset=utf-8", h);
}

// Streams the multipart upload to a temp file on LittleFS.
static void handleRecipeUpload() {
    HTTPUpload &up = HTTP.upload();
    if (up.status == UPLOAD_FILE_START) {
        uploadFile = LittleFS.open("/upload.tmp", "w");
    } else if (up.status == UPLOAD_FILE_WRITE) {
        if (uploadFile) {
            uploadFile.write(up.buf, up.currentSize);
        }
    } else if (up.status == UPLOAD_FILE_END) {
        if (uploadFile) {
            uploadFile.close();
        }
    }
}

// Runs after the upload completed: parse, apply, persist.
static void handleRecipeDone() {
    File f = LittleFS.open("/upload.tmp", "r");
    Recipe r;
    bool ok = false;
    if (f) {
        // sniff the first non-space byte: '<' = BeerXML, otherwise KBH JSON
        int c = ' ';
        while (f.available()) {
            c = f.peek();
            if (c != ' ' && c != '\t' && c != '\r' && c != '\n') break;
            f.read();
        }
        ok = (c == '<') ? parseBeerXmlStream(f, r) : parseKbhStream(f, r);
        f.close();
    }
    LittleFS.remove("/upload.tmp");

    if (!ok) {
        HTTP.send(400, "text/html; charset=utf-8",
                  pageHead("Import fehlgeschlagen") +
                      F("<div class='card'>Kein g&uuml;ltiges Kleiner-Brauhelfer JSON oder BeerXML.</div>") +
                      pageFoot());
        return;
    }

    applyRecipe(r);
    saveRecipe(r);

    String h = pageHead("Importiert");
    h += F("<div class='card'><h2>");
    h += r.name;
    h += F("</h2>");
    h += r.rasten;
    h += F(" Rasten &middot; ");
    h += r.kochzeit;
    h += F(" min Kochen &middot; ");
    h += r.hopfenanzahl;
    h += F(" Hopfengaben</div>");
    h += pageFoot();
    HTTP.send(200, "text/html; charset=utf-8", h);
}
