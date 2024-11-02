#include "Device.h"

Device device;

void setup() {
    Serial.begin(115200);
    device.setup();
}
void loop() {
    device.updateReadings();

    float temperature = device.getTemperature();

    device.lcdClear();

    device.lcdSetCursor(0, 0);
    device.lcdPrint("Distancia:");
    device.lcdSetCursor(0, 1);
    device.lcdPrint(String(distance) + " cm");
    delay(333);
}