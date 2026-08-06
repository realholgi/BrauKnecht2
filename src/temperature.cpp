#include <OneWire.h>
#include <DallasTemperature.h>

#include "temperature.h"
#include "temp_filter.h"
#include "config.h"
#include "global.h"

static OneWire oneWire(oneWirePin);
static DallasTemperature sensors(&oneWire);
static DeviceAddress insideThermometer;

constexpr unsigned long CONVERSION_MS = 750;  // 12-bit DS18B20 conversion time
static unsigned long lastRequest = 0;

void temperatureSetup() {
    sensors.begin();
    sensors.getAddress(insideThermometer, 0);
    sensors.setResolution(insideThermometer, 12);
    sensors.setWaitForConversion(false);   // non-blocking: poll by elapsed time
    sensors.requestTemperatures();          // kick the first conversion
    lastRequest = millis();
}

// Non-blocking: returns immediately while a conversion is in flight, so the
// main loop (web, MQTT, LCD) keeps running instead of stalling ~750 ms.
void readTemperature() {
    if (millis() - lastRequest < CONVERSION_MS) {
        return;
    }
    float sensorwert = sensors.getTempC(insideThermometer);
    isttemp = filterTemp(isttemp, sensorwert, n);
    sensors.requestTemperatures();          // start the next conversion
    lastRequest = millis();
}
