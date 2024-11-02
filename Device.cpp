#include "Device.h"

Device::Device() : lcd(0x27, 16, 2) {}

void Device::setup() {
    pinMode(Ledpot, OUTPUT);
    lcd.init();
    lcd.backlight();
    pinMode(tri, OUTPUT);
    pinMode(eco, INPUT);
    digitalWrite(tri, LOW);
}

void Device::sendAlert(const String &message) {
    Serial.println("ALERTA: " + message);
}

void Device::updateLedStatus(bool isOn) {
    ledState = isOn;
}


void Device::updateReadings() {

    String alertMessages = "";
    bool alertSent = false;

    float currentDistance = getDistance();


    if (currentDistance <= proximityThreshold) {
        alertMessages += "Objeto muy cerca! Distancia: " + String(currentDistance) + " cm ";
        alertSent = true;
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

    // Mostrar lecturas en el monitor serie
    Serial.println("--------- Monitor Serie ---------");
    Serial.println("Distancia: " + String(currentDistance) + " cm");
    Serial.println("-----------------------------");
}

float Device::getDistance() {
    digitalWrite(tri, LOW);
    delayMicroseconds(2);
    digitalWrite(tri, HIGH);
    delayMicroseconds(10);
    digitalWrite(tri, LOW);
    long tiempo = pulseIn(eco, HIGH);
    return (tiempo / 29.35) / 2;
}

void Device::lcdSetCursor(int col, int row) {
    lcd.setCursor(col, row);
}

void Device::lcdPrint(const String &text) {
    lcd.print(text);
}

void Device::lcdClear() {
    lcd.clear();
}