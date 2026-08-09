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
#include "persistence.h"
#include "recipe.h"
#include "input.h"
#include "manual_control.h"
#include "build_info.h"
#include "brew_status.h"

const char *ap_ssid = APSSID;

ESP8266WebServer HTTP(80);

constexpr unsigned long WIFI_AP_POLL_MS = 1000;
static bool accessPointEnabled = false;
static bool configuredStationConnectedKnown = false;
static bool configuredStationConnected = false;

void handle_http() {
    HTTP.handleClient();
    MDNS.update();
}

static void handleConfigGet();
static void handleConfigPost();
static void handleRecipeGet();
static void handleRecipeDone();
static void handleRecipeUpload();
static void handleHistory();
static void handleManualPost();
static void handleAccessPointPost();

void setupWebserver() {
    HTTP.collectHeaders("X-BrauKnecht-Action");
    HTTP.on("/", handleRoot);
    HTTP.on("/history", HTTP_GET, handleHistory);
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
    HTTP.on("/manual", HTTP_POST, handleManualPost);
    HTTP.on("/access-point", HTTP_POST, handleAccessPointPost);

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

    JsonDocument json;

    json["title"] = title;
    json["title2"] = title2;
    json["temp_ist"] = isttemp;
    json["temp_soll"] = sollwert;
    json["heizung"] = heizung ? "an" : "aus";

    json["data"] = data;

    json["modus"] = static_cast<int>(modus);
    json["rufmodus"] = static_cast<int>(rufmodus);
    json["alarm"] = isRufalarmMode(modus);
    json["regelung"] = static_cast<int>(regelung);

    const bool hold = modus == BRAUVORGANG_HALT;
    const unsigned long elapsedSeconds = hold
        ? holdElapsedSeconds
        : static_cast<unsigned long>(minuten) * 60UL + static_cast<unsigned long>(sekunden);
    const BrewStatus activeStep = brewStatus({
        modus, holdReturnModus, x, holdReturnX, rasten, hopfenanzahl,
        rastZeit, kochzeit, elapsedSeconds, hold
    });
    if (activeStep.activeStepIndex >= 0) {
        json["active_step_index"] = activeStep.activeStepIndex;
        json["active_step_label"] = activeStep.activeStepLabel;
        json["active_step_elapsed_seconds"] = activeStep.elapsedSeconds;
        json["active_step_total_seconds"] = activeStep.totalSeconds;
        json["active_step_remaining_seconds"] = activeStep.remainingSeconds;
    } else {
        json["active_step_index"] = nullptr;
        json["active_step_label"] = nullptr;
        json["active_step_elapsed_seconds"] = nullptr;
        json["active_step_total_seconds"] = nullptr;
        json["active_step_remaining_seconds"] = nullptr;
    }
    json["active_step_timed"] = activeStep.timed;
    json["hold"] = hold;
    json["alarm_reason"] = rufalarmReasonName(
        isRufalarmMode(modus) ? rufalarmReason : RUFALARM_REASON_NONE);
    json["alarm_action"] = rufalarmActionName(rufalarmReason, isRufalarmMode(modus));

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

    json["firmware_version"] = firmwareVersion();
    json["build_hash"] = buildGitHash();
    json["build_time"] = buildTime();
    json["build_env"] = buildEnvironment();

    Recipe cur = currentRecipe();
    json["recipe_name"] = cur.name;
    json["recipe_maischtemp"] = cur.maischtemp;
    JsonArray recipeRests = json["recipe_rasten"].to<JsonArray>();
    for (int i = 1; i <= cur.rasten && i <= 7; i++) {
        JsonObject rest = recipeRests.add<JsonObject>();
        rest["temp"] = cur.rastTemp[i];
        rest["zeit"] = cur.rastZeit[i];
    }
    json["recipe_endtemp"] = cur.endtemp;
    json["recipe_kochzeit"] = cur.kochzeit;
    JsonArray recipeHops = json["recipe_hopfen"].to<JsonArray>();
    for (int i = 1; i <= cur.hopfenanzahl && i <= 6; i++) {
        recipeHops.add(cur.hopfenZeit[i]);
    }

    String message = "";
    serializeJson(json, message);

    HTTP.send(200, "application/json;charset=utf-8", message);
}

void handleRoot() {
    HTTP.send_P(200, "text/html", PAGE_Dashboard);
}

static void handleHistory() {
    HTTP.send_P(200, "text/html", PAGE_History);
}

static void handleManualPost() {
    if (!HTTP.hasHeader("X-BrauKnecht-Action") || HTTP.header("X-BrauKnecht-Action") != "manual") {
        HTTP.send(403, "application/json;charset=utf-8", "{\"error\":\"forbidden\"}");
        return;
    }

    if (!canEnterManualMode(modus, regelung)) {
        HTTP.send(409, "application/json;charset=utf-8", "{\"error\":\"manual_unavailable\"}");
        return;
    }

    int raw = sollwert;
    if (HTTP.hasArg("soll")) {
        raw = HTTP.arg("soll").toInt();
    } else if (HTTP.hasArg("temp_soll")) {
        raw = HTTP.arg("temp_soll").toInt();
    }

    int value = clampManualSetpoint(raw);
    sollwert = value;
    drehen = value;
    modus = MANUELL;
    anfang = true;

    JsonDocument json;
    json["temp_soll"] = sollwert;
    json["modus"] = static_cast<int>(modus);

    String message;
    serializeJson(json, message);
    HTTP.send(200, "application/json;charset=utf-8", message);
}

bool setupWIFI() {
    WiFi.mode(WIFI_AP_STA);

    accessPointEnabled = WiFi.softAP(ap_ssid);  // initial recovery AP

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

bool isAccessPointEnabled() {
    return accessPointEnabled;
}

bool setAccessPointEnabled(bool enabled) {
    if (accessPointEnabled == enabled) {
        return true;
    }

    bool changed = enabled ? WiFi.softAP(ap_ssid) : WiFi.softAPdisconnect(true);
    if (changed) {
        accessPointEnabled = enabled;
    }
    return changed;
}

void serviceWiFiAp() {
    static unsigned long lastPoll = 0;
    unsigned long now = millis();
    if (now - lastPoll < WIFI_AP_POLL_MS) {
        return;
    }
    lastPoll = now;

    bool connected = strlen(settings.sta_ssid) && WiFi.status() == WL_CONNECTED &&
                     WiFi.SSID() == settings.sta_ssid;
    if (!configuredStationConnectedKnown) {
        if (setAccessPointEnabled(!connected)) {
            configuredStationConnected = connected;
            configuredStationConnectedKnown = true;
        }
        return;
    }

    if (connected != configuredStationConnected && setAccessPointEnabled(!connected)) {
        configuredStationConnected = connected;
    }
}

// Shared page shell: all generated pages use the same app shell as the static
// dashboard/history pages.
static String htmlEscape(const char *value) {
    String out;
    for (const char *p = value; *p; p++) {
        switch (*p) {
            case '&': out += F("&amp;"); break;
            case '<': out += F("&lt;"); break;
            case '>': out += F("&gt;"); break;
            case '"': out += F("&quot;"); break;
            case '\'': out += F("&#39;"); break;
            default: out += *p; break;
        }
    }
    return out;
}

static void appendNavLink(String &h, const char *href, const char *label, const char *icon, const char *active) {
    h += F("<a class='nav-link");
    if (strcmp(active, label) == 0) {
        h += F(" active");
    }
    h += F("' href='");
    h += href;
    h += F("'>");
    h += icon;
    h += F("<span>");
    h += label;
    h += F("</span></a>");
}

static void appendNav(String &h, const char *active, bool bottom) {
    static const char *dashIcon = "<svg viewBox='0 0 24 24'><path class='nav-fill' d='M3 11.5 12 4l9 7.5-1.4 1.7-1.1-.9V21h-5v-6h-3v6h-5v-8.7l-1.1.9Z'/></svg>";
    static const char *histIcon = "<svg viewBox='0 0 24 24'><path d='M4 19h16'/><path d='M5 15l5-5 4 3 5-7'/></svg>";
    static const char *recipeIcon = "<svg viewBox='0 0 24 24'><path d='M7 3h7l4 4v14H7z'/><path d='M14 3v5h5'/><path d='M10 13h5'/><path d='M10 17h5'/></svg>";
    static const char *settingsIcon = "<svg viewBox='0 0 24 24'><path d='M12 8.5a3.5 3.5 0 1 0 0 7 3.5 3.5 0 0 0 0-7Z'/><path d='M19.4 15a1.7 1.7 0 0 0 .34 1.88l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06a1.7 1.7 0 0 0-1.88-.34 1.7 1.7 0 0 0-1.03 1.56V21a2 2 0 1 1-4 0v-.09A1.7 1.7 0 0 0 8.97 19.4a1.7 1.7 0 0 0-1.88.34l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06A1.7 1.7 0 0 0 4.6 15 1.7 1.7 0 0 0 3.04 14H3a2 2 0 1 1 0-4h.09A1.7 1.7 0 0 0 4.6 8.97a1.7 1.7 0 0 0-.34-1.88l-.06-.06A2 2 0 1 1 7.03 4.2l.06.06A1.7 1.7 0 0 0 8.97 4.6 1.7 1.7 0 0 0 10 3.04V3a2 2 0 1 1 4 0v.09a1.7 1.7 0 0 0 1.03 1.51 1.7 1.7 0 0 0 1.88-.34l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06a1.7 1.7 0 0 0-.34 1.88A1.7 1.7 0 0 0 20.96 10H21a2 2 0 1 1 0 4h-.09A1.7 1.7 0 0 0 19.4 15Z'/></svg>";
    h += bottom ? F("<nav class='bottom-nav' aria-label='Navigation'>") : F("<nav class='side-nav' aria-label='Navigation'>");
    appendNavLink(h, "/", "Dashboard", dashIcon, active);
    appendNavLink(h, "/history", "Verlauf", histIcon, active);
    appendNavLink(h, "/recipe", "Rezept", recipeIcon, active);
    appendNavLink(h, "/config", "Einstellungen", settingsIcon, active);
    h += F("</nav>");
}

static String pageHead(const char *title, const char *active) {
    String h = F("<!DOCTYPE html><html lang='de'><head><meta charset='utf-8'>"
                 "<meta name='viewport' content='width=device-width,initial-scale=1,viewport-fit=cover'>"
                 "<link rel='icon' href='data:,'><link rel='stylesheet' href='/style.css'><title>");
    h += title;
    h += F("</title></head><body><main class='app'><div class='topbar'><div class='brand'>BrauKnecht</div></div><div class='layout'>");
    appendNav(h, active, false);
    h += F("<section class='grid'><h1 class='page-title'>");
    h += title;
    h += F("</h1>");
    return h;
}

static String pageFoot(const char *active) {
    String h = F("</section></div></main>");
    appendNav(h, active, true);
    h += F("</body></html>");
    return h;
}

static void appendDeviceInfoCard(String &h) {
    char buildTimeText[24];
    formatBuildTime(buildTimeText, sizeof(buildTimeText), buildTime());

    h += F("<article class='card settings-info-card'><div class='section-title'>"
           "<svg viewBox='0 0 24 24'><circle cx='12' cy='12' r='10'/><path d='M12 16v-4'/><path d='M12 8h.01'/></svg>"
           "<span>Ger&auml;teinfo</span></div><ul class='rows'>"
           "<li><span class='row-label'>Firmware</span><span class='row-value'>");
    h += firmwareVersion();
    h += F("</span></li><li><span class='row-label'>Build</span><span class='row-value'>");
    h += buildGitHash();
    h += F("</span></li><li><span class='row-label'>Build-Zeit</span><span class='row-value'>");
    h += buildTimeText;
    h += F("</span></li><li><span class='row-label'>IP</span><span class='row-value'>");
    h += WiFi.localIP().toString();
    h += F("</span></li><li><span class='row-label'>AP-IP</span><span class='row-value'>");
    if (isAccessPointEnabled()) {
        h += WiFi.softAPIP().toString();
    } else {
        h += F("aus");
    }
    h += F("</span></li>");
    if (strlen(settings.mqtt_host)) {
        h += F("<li><span class='row-label'>MQTT</span><span class='row-value'>");
        h += htmlEscape(settings.mqtt_host);
        h += F(":");
        h += settings.mqtt_port;
        h += F("</span></li>");
    }
    h += F("</ul></article>");
}

static void handleConfigGet() {
    // Async scan only — a blocking WiFi.scanNetworks() can outlast the 2s
    // watchdog while the loop is stalled in this handler. We cache the last good
    // result so the list never blanks while a (re)scan is running.
    static String wifiCache;  // scanned <option>s, kept across reloads
    int scanCount = WiFi.scanComplete();
    if (scanCount == -2) {
        WiFi.scanNetworks(true);  // first scan; list fills on the next reload
    } else if (scanCount >= 0) {
        String c;
        for (int i = 0; i < scanCount; i++) {
            String s = WiFi.SSID(i);
            if (s.length() == 0) continue;
            if (s == APSSID || s == settings.sta_ssid) continue;   // own AP / already selected
            String escaped = htmlEscape(s.c_str());
            String tag = "value=\"" + escaped + "\"";
            if (c.indexOf(tag) >= 0) continue;                     // same SSID on several APs/channels
            c += F("<option ");
            c += tag;
            c += F(">");
            c += escaped;
            c += F("</option>");
        }
        wifiCache = c;
        WiFi.scanNetworks(true);  // refresh for the next reload
    }
    // scanCount == -1: scan still running -> keep showing the cached list

    // current SSID first (stays selected even if not in range right now)
    String opts;
    if (strlen(settings.sta_ssid)) {
        String ssid = htmlEscape(settings.sta_ssid);
        opts += F("<option selected value=\"");
        opts += ssid;
        opts += F("\">");
        opts += ssid;
        opts += F("</option>");
    } else {
        opts += F("<option value=\"\">&ndash; ausw&auml;hlen &ndash;</option>");
    }
    opts += wifiCache;

    String h = pageHead("Einstellungen", "Einstellungen");
    h += F("<form class='settings-form' method='POST' action='/config'>"
           "<article class='card'><div class='section-title'>"
           "<svg viewBox='0 0 24 24'><path d='M5 12.5a10 10 0 0 1 14 0'/><path d='M8.5 16a5 5 0 0 1 7 0'/><path d='M12 20h.01'/></svg>"
           "<span>WLAN</span></div><div class='field-grid two'><div><label>SSID</label><select name='sta_ssid'>");
    h += opts;
    h += F("</select></div><div><label>Passwort</label><input name='sta_pass' type='password' placeholder='unver&auml;ndert'></div></div>"
           "<p class='small'>AP wird nach erfolgreicher WLAN-Verbindung automatisch ausgeschaltet und kann am Ger&auml;t oder hier tempor&auml;r geschaltet werden.</p></article>"
           "<article class='card'><div class='section-title'>"
           "<svg viewBox='0 0 24 24'><circle cx='12' cy='5' r='2'/><circle cx='5' cy='19' r='2'/><circle cx='19' cy='19' r='2'/><path d='M12 7v4'/><path d='M12 11 5 17'/><path d='M12 11l7 6'/></svg>"
           "<span>MQTT</span></div><div class='field-grid three'><div><label>Host</label><input name='mqtt_host' value=\"");
    h += htmlEscape(settings.mqtt_host);
    h += F("\"></div><div><label>Port</label><input name='mqtt_port' inputmode='numeric' value='");
    h += settings.mqtt_port;
    h += F("'></div><div><label>User</label><input name='mqtt_user' value=\"");
    h += htmlEscape(settings.mqtt_user);
    h += F("</div><label>Passwort</label><input name='mqtt_pass' type='password' placeholder='unver&auml;ndert'></article>"
           "<article class='card'><div class='section-title'>"
           "<svg viewBox='0 0 24 24'><path d='M12 3v18'/><path d='M5 7h14'/><path d='M5 17h14'/><path d='M7 3h10v18H7z'/></svg>"
           "<span>Brauen</span></div><label>Kochschwelle (&deg;C)</label><input name='kschwelle' type='number' inputmode='numeric' min='20' max='99' value='");
    h += kschwelle;
    h += F("'><p class='small'>Die Kochschwelle entspricht der Einstellung unter Setup am Ger&auml;t.</p>"
           "<div class='section-title'><span>Access Point</span></div><p class='small'>Aktuell ");
    h += isAccessPointEnabled() ? F("eingeschaltet.") : F("ausgeschaltet.");
    h += F("</p><button class='btn full' type='submit' formaction='/access-point' name='access_point' value='");
    h += isAccessPointEnabled() ? F("off'>Access Point ausschalten") : F("on'>Access Point einschalten");
    h += F("</button></article>");
    appendDeviceInfoCard(h);
    h += F("<div class='settings-actions'><button class='btn full' type='submit'>Speichern &amp; Neustart</button></div></form>");
    h += pageFoot("Einstellungen");
    HTTP.send(200, "text/html; charset=utf-8", h);
}

static void handleAccessPointPost() {
    const String requested = HTTP.arg("access_point");
    if (requested != F("on") && requested != F("off")) {
        HTTP.send(400, "text/plain; charset=utf-8", "Ungültige Access-Point-Einstellung");
        return;
    }

    if (!setAccessPointEnabled(requested == F("on"))) {
        HTTP.send(503, "text/plain; charset=utf-8", "Access Point konnte nicht geändert werden");
        return;
    }

    HTTP.sendHeader("Location", "/config");
    HTTP.send(303, "text/plain", "");
}

static void handleConfigPost() {
    int requestedKochschwelle = HTTP.arg("kschwelle").toInt();
    kschwelle = static_cast<uint8_t>(constrain(requestedKochschwelle, 20, 99));
    writeEepromData();
    strlcpy(settings.sta_ssid, HTTP.arg("sta_ssid").c_str(), sizeof(settings.sta_ssid));
    strlcpy(settings.mqtt_host, HTTP.arg("mqtt_host").c_str(), sizeof(settings.mqtt_host));
    strlcpy(settings.mqtt_user, HTTP.arg("mqtt_user").c_str(), sizeof(settings.mqtt_user));
    long port = HTTP.arg("mqtt_port").toInt();
    settings.mqtt_port = (port > 0 && port <= 65535) ? static_cast<uint16_t>(port) : 1883;
    // keep existing passwords if the field was left blank
    if (HTTP.arg("sta_pass").length()) {
        strlcpy(settings.sta_pass, HTTP.arg("sta_pass").c_str(), sizeof(settings.sta_pass));
    }
    if (HTTP.arg("mqtt_pass").length()) {
        strlcpy(settings.mqtt_pass, HTTP.arg("mqtt_pass").c_str(), sizeof(settings.mqtt_pass));
    }

    saveSettings(settings);

    HTTP.send(200, "text/html; charset=utf-8",
              pageHead("Neustart", "Einstellungen") + F("<div class='card'>Gespeichert. Neustart&hellip;</div>") + pageFoot("Einstellungen"));
    delay(1000);
    ESP.restart();
}

static File uploadFile;

// Full recipe view: a vertical-schedule timeline that lists every rest
// (temp + time) and every hop addition. Used by both the import page and the
// import-success page.
static void appendRecipeCard(String &h, const Recipe &r) {
    String name = htmlEscape(r.name);
    const char *mashIcon = "<svg viewBox='0 0 24 24' aria-hidden='true'><path d='M5 19c8 0 14-6 14-14C11 5 5 11 5 19Z'/><path d='M5 19c4-5 8-8 14-14'/></svg>";
    const char *mashChipIcon = "<svg viewBox='0 0 24 24' aria-hidden='true'><path d='M8 19c-2-3 2-4 0-7'/><path d='M12 19c-2-3 2-4 0-7'/><path d='M16 19c-2-3 2-4 0-7'/></svg>";
    const char *mashoutIcon = "<svg viewBox='0 0 24 24' aria-hidden='true'><path d='M12 3s6 7 6 11a6 6 0 0 1-12 0c0-4 6-11 6-11Z'/><path d='M9.5 15a2.5 2.5 0 0 0 5 0'/></svg>";
    const char *boilIcon = "<svg viewBox='0 0 24 24' aria-hidden='true'><path d='M8 9h8v9a2 2 0 0 1-2 2h-4a2 2 0 0 1-2-2Z'/><path d='M6 12h12'/><path d='M9 5c-1 1.2 1 2.2 0 3.4'/><path d='M13 4c-1 1.2 1 2.2 0 3.4'/><path d='M17 5c-1 1.2 1 2.2 0 3.4'/></svg>";
    const char *hopIcon = "<svg viewBox='0 0 24 24' aria-hidden='true'><path d='M12 3c4 3 6 7 6 11a6 6 0 0 1-12 0c0-4 2-8 6-11Z'/><path d='M12 3v18'/><path d='M8 10l4 3 4-3'/><path d='M7 14l5 3 5-3'/></svg>";

    h += F("<article class='card'><div class='recipe-summary'><div><h2>");
    h += name;
    h += F(" <span class='pill'>Aktiv</span></h2><div class='chip-row'><span class='chip'>");
    h += mashChipIcon;
    h += F("Maischen</span><span class='chip'><svg viewBox='0 0 24 24'><path d='M8 6h13'/><path d='M8 12h13'/><path d='M8 18h13'/><path d='M3 6h.01'/><path d='M3 12h.01'/><path d='M3 18h.01'/></svg>");
    h += r.rasten;
    h += r.rasten == 1 ? F(" Rast</span><span class='chip'>") : F(" Rasten</span><span class='chip'>");
    h += hopIcon;
    h += r.hopfenanzahl;
    h += r.hopfenanzahl == 1 ? F(" Hopfengabe</span>") : F(" Hopfengaben</span>");
    h += F("<span class='chip'><svg viewBox='0 0 24 24'><path d='M8 7h8'/><path d='M9 7v13h6V7'/><path d='M7 11h10'/><path d='M12 3v4'/></svg>Kochen ");
    h += r.kochzeit;
    h += F(" min</span></div></div><span class='icon-btn' aria-hidden='true'>"
           "<svg viewBox='0 0 24 24'><path d='M12 5v.01'/><path d='M12 12v.01'/><path d='M12 19v.01'/></svg>"
           "</span></div></article>");

    h += F("<article class='card'><div class='recipe-timeline'>"
           "<div class='timeline-row'><span class='timeline-icon mash'>");
    h += mashIcon;
    h += F("</span><span class='timeline-name'>Einmaischen</span><span class='timeline-temp'>");
    h += r.maischtemp;
    h += F("&deg;C</span><span class='timeline-time'></span></div>");

    for (int i = 1; i <= r.rasten; i++) {
        h += F("<div class='timeline-row'><span class='timeline-icon'>");
        h += i;
        h += F("</span><span class='timeline-name'>");
        h += i;
        h += F(". Rast</span><span class='timeline-temp'>");
        h += r.rastTemp[i];
        h += F("&deg;C</span><span class='timeline-time'>");
        h += r.rastZeit[i];
        h += F(" min</span></div>");
    }

    h += F("<div class='timeline-row'><span class='timeline-icon mashout'>");
    h += mashoutIcon;
    h += F("</span><span class='timeline-name'>Abmaischen</span><span class='timeline-temp'>");
    h += r.endtemp;
    h += F("&deg;C</span><span class='timeline-time'></span></div>"
           "<div class='timeline-row'><span class='timeline-icon boil'>");
    h += boilIcon;
    h += F("</span><span class='timeline-name'>Kochen</span><span class='timeline-temp'></span><span class='timeline-time'>");
    h += r.kochzeit;
    h += F(" min</span></div>");

    for (int i = 1; i <= r.hopfenanzahl; i++) {
        h += F("<div class='timeline-row'><span class='timeline-icon hop'>");
        h += hopIcon;
        h += F("</span><span class='timeline-name'>");
        h += i;
        h += F(". nach ");
        h += r.hopfenZeit[i];
        h += F(" min</span><span class='timeline-temp'></span><span class='timeline-time'></span></div>");
    }

    h += F("<div class='timeline-row'><span class='timeline-icon boil'>");
    h += boilIcon;
    h += F("</span><span class='timeline-name'>Kochende</span><span class='timeline-temp'></span><span class='timeline-time'>");
    h += r.kochzeit;
    h += F(" min</span></div>");
    h += F("</div></article>");
}

static void handleRecipeGet() {
    String h = pageHead("Rezept", "Rezept");
    Recipe cur = currentRecipe();
    appendRecipeCard(h, cur);
    h += F("<form class='card' method='POST' action='/recipe' enctype='multipart/form-data'>"
           "<div class='import-title'><svg viewBox='0 0 24 24'><path d='M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z'/><path d='M14 2v6h6'/><path d='M12 18v-8'/><path d='M8 14l4-4 4 4'/></svg><span>Rezept importieren</span></div>"
           "<label>JSON oder BeerXML</label>"
           "<input type='file' name='recipe' accept='.json,.xml'>"
           "<button class='btn full' type='submit'>Importieren</button></form>");
    h += pageFoot("Rezept");
    HTTP.send(200, "text/html; charset=utf-8", h);
}

// Streams the multipart upload to a temp file on LittleFS.
static void handleRecipeUpload() {
    EncoderTimerGuard guard;  // pause timer1 ISR during the flash writes
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
    Recipe r;
    bool ok = false;
    {
        // Reading the upload does raw SPI-flash access (cache disabled) just like
        // writing — the timer1 encoder ISR must not run flash-resident code
        // meanwhile, or it faults (WDT reset). Pause it across the whole read.
        EncoderTimerGuard guard;
        File f = LittleFS.open("/upload.tmp", "r");
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
    }

    if (!ok) {
        HTTP.send(400, "text/html; charset=utf-8",
                  pageHead("Import fehlgeschlagen", "Rezept") +
                      F("<div class='card'>Kein g&uuml;ltiges Kleiner-Brauhelfer JSON oder BeerXML.</div>") +
                      pageFoot("Rezept"));
        return;
    }

    applyRecipe(r);
    saveRecipe(r);

    String h = pageHead("Importiert", "Rezept");
    appendRecipeCard(h, r);
    h += pageFoot("Rezept");
    HTTP.send(200, "text/html; charset=utf-8", h);
}
