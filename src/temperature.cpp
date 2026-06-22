#include <OneWire.h>
#include <DallasTemperature.h>

#include "temperature.h"
#include "config.h"
#include "global.h"

static OneWire oneWire(oneWirePin);
static DallasTemperature sensors(&oneWire);
static DeviceAddress insideThermometer;
static float sensorwert;

void temperatureSetup() {
    sensors.getAddress(insideThermometer, 0);
    sensors.setResolution(insideThermometer, 12);   // set the resolution to 9 bit
}

void readTemperature() {
    sensors.requestTemperatures();
    sensorwert = sensors.getTempC(insideThermometer);
    if ((sensorwert != isttemp) && (n < 5)) { // Messfehlervermeidung des Sensorwertes
        n++;
    } else {
        isttemp = sensorwert;
        n = 0;
    }
}
