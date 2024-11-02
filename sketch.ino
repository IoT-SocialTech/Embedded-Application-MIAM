#include "Device.h"

Device device;

#define WIFI_SSID "Wokwi-GUEST"
#define PASSWORD ""
#define DATABASE_URL "https://fir-embedded-miam-default-rtdb.firebaseio.com/.json"

void setup() {
    Serial.begin(115200);
    device.setup();

    device.lcdSetCursor(0, 0);
    device.lcdPrint("Connecting...");
    device.connectToWiFi(WIFI_SSID, PASSWORD);
    device.connectToFirebase(DATABASE_URL);

    configTime(-9000, -9000, "1.south-america.pool.ntp.org");
}
void loop() {
    device.updateReadings();
    float pulse = device.getPulse();
    float temperature = device.getTemperature();
    float distance = device.getDistance();

    device.updateFirebase(pulse, temperature, distance);
    device.lcdClear();

    device.lcdSetCursor(1, 0);
    device.lcdPrint("Pulso cardiaco");
    device.lcdSetCursor(6, 1);
    device.lcdPrint(String(pulse));
    delay(333);
    device.lcdClear();

    device.lcdSetCursor(0, 0);
    device.lcdPrint("Distancia:");
    device.lcdSetCursor(0, 1);
    device.lcdPrint(String(distance) + " cm");
    delay(333);

    device.lcdSetCursor(0, 0);
    device.lcdPrint("Distancia:");
    device.lcdSetCursor(0, 1);
    device.lcdPrint(String(distance) + " cm");
    delay(333);
}
}