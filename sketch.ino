#include "Device.h"

Device device;

void setup() {
    Serial.begin(115200);
    device.setup();
}
void loop() {
    void Device::updateLedStatus(bool isOn) {
        ledState = isOn;
    }

    void Device::sendAlert(const String &message) {
        Serial.println("ALERTA: " + message);
    }


    void Device::updateReadings() {
        String alertMessages = "";
        bool alertSent = false;
        bool panicButtonActivated = isPanicButtonPressed();

        if (panicButtonActivated) {
            updateLedStatus(true);
            sendAlert("¡ALERTA DE PÁNICO ACTIVADA!");
            client.PATCH("{\"Status/Alerts/panicButton\":\"true\"}");
            lcdClear();
            lcdSetCursor(0, 0);
            lcdPrint("PANIC ALERT!");
            lcdSetCursor(0, 1);
            lcdPrint("LLAMAR AYUDA!");
            analogWrite(Ledpot, 255);
            delay(1000);
            return;
        }

        // Manejo de estado del LED y envío de alerta si hay alguna
        if (alertSent) {
            updateLedStatus(true);
            analogWrite(Ledpot, 255);
            sendAlert(alertMessages);
        } else {
            updateLedStatus(false);
            analogWrite(Ledpot, 0);
        }

    }

    bool Device::isPanicButtonPressed() {
        return digitalRead(panicButtonPin) == LOW;
    }
}