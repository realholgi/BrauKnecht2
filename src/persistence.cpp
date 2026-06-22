#include <EEPROM.h>

#include "persistence.h"
#include "global.h"

void readEepromData() {
    EEPROM.begin(512);

    hysteresespeicher = EEPROM.read(HYSTERESE_MEM);
    if (hysteresespeicher > 40 || hysteresespeicher == 0) { hysteresespeicher = HYSTERESE_DEFAULT; };
    hysterese = hysteresespeicher / 10;

    kschwelle = EEPROM.read(KOCHSCHWELLE_MEM);
    if (kschwelle > 100 || kschwelle == 0) { kschwelle = KOCHSCHWELLE_DEFAULT; };
}

void writeEepromData() {
    EEPROM.begin(512);

    EEPROM.write(HYSTERESE_MEM, hysteresespeicher);
    EEPROM.write(KOCHSCHWELLE_MEM, kschwelle);
    EEPROM.commit();
}
