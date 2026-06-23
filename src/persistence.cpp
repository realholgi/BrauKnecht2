#include <EEPROM.h>
#include <LittleFS.h>

#include "persistence.h"
#include "settings.h"
#include "global.h"
#include "input.h"

Settings settings;

void persistenceSetup() {
    if (!LittleFS.begin()) {
        LittleFS.format();
        LittleFS.begin();
    }
}

bool loadSettings(Settings &s) {
    settingsDefaults(s);
    File f = LittleFS.open("/settings.json", "r");
    if (!f) {
        return false;
    }
    char buf[512];
    size_t len = f.readBytes(buf, sizeof(buf) - 1);
    buf[len] = '\0';
    f.close();
    return settingsFromJson(buf, s);
}

bool saveSettings(const Settings &s) {
    EncoderTimerGuard guard;  // pause timer1 ISR during the flash write
    char buf[512];
    size_t len = settingsToJson(s, buf, sizeof(buf));
    File f = LittleFS.open("/settings.json", "w");
    if (!f) {
        return false;
    }
    f.write(reinterpret_cast<const uint8_t *>(buf), len);
    f.close();
    return true;
}

void readEepromData() {
    EEPROM.begin(512);

    hysteresespeicher = EEPROM.read(HYSTERESE_MEM);
    if (hysteresespeicher > 40 || hysteresespeicher == 0) { hysteresespeicher = HYSTERESE_DEFAULT; };
    hysterese = hysteresespeicher / 10;

    kschwelle = EEPROM.read(KOCHSCHWELLE_MEM);
    if (kschwelle > 100 || kschwelle == 0) { kschwelle = KOCHSCHWELLE_DEFAULT; };
}

void writeEepromData() {
    EncoderTimerGuard guard;  // pause timer1 ISR during the EEPROM (flash) commit
    EEPROM.begin(512);

    EEPROM.write(HYSTERESE_MEM, hysteresespeicher);
    EEPROM.write(KOCHSCHWELLE_MEM, kschwelle);
    EEPROM.commit();
}
