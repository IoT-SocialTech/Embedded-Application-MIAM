#include "Device.h"
#include <Arduino.h>
#include <time.h>


Device::Device() : lcd(0x27, 16, 2) {}

void Device::setup() {
    pinMode(Ledpot, OUTPUT);
    lcd.init();
    lcd.backlight();
    pinMode(tri, OUTPUT);
    pinMode(eco, INPUT);
    pinMode(panicButtonPin, INPUT_PULLUP);
    digitalWrite(tri, LOW);
}
void Device::updateLedStatus(bool isOn) {
    ledState = isOn;
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
    }


    float currentTemp = getTemperature();
    float currentDistance = getDistance();

    if (currentDistance <= proximityThreshold) {
        alertMessages += "Objeto muy cerca! Distancia: " + String(currentDistance) + " cm ";
        alertSent = true;
    }

    if (alertSent) {
        updateLedStatus(true);
        analogWrite(Ledpot, 255);
        sendAlert(alertMessages);
    } else {
        updateLedStatus(false);
        analogWrite(Ledpot, 0);
    }

    Serial.println("--------- Monitor Serie ---------");
    Serial.println("Distancia: " + String(currentDistance) + " cm");
    Serial.println("-----------------------------");
}

float Device::getTemperature() {
    int bodyTemperature = ((analogRead(temperature) * (5 / 4095.0)) / 3.3) * 100;
    return bodyTemperature;
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

bool Device::isPanicButtonPressed() {
    return digitalRead(panicButtonPin) == LOW;
}