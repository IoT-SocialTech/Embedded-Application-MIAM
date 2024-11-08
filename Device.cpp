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

void Device::connectToWiFi(const char* ssid, const char* password) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(250);
    }
    Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
    lcdClear();
    lcdSetCursor(0, 0);
    lcdPrint("Online");
    delay(500);
}

void Device::connectToFirebase(const String& url) {
    databaseUrl = url;
    client.begin(databaseUrl);
    int httpResponseCode = client.GET();
    lcdClear();
    lcdSetCursor(0, 0);
    if (httpResponseCode > 0) {
        lcdPrint("Firebase Online");
        Serial.println("Firebase connected.");
    } else {
        lcdPrint("Firebase Error");
        Serial.println("Failed to connect to Firebase.");
    }
    delay(500);
}

void Device::updateFirebase(float pulse, float temperature, float distance) {
    String currentTime = getCurrentTime();
    client.PATCH("{\"Status/Sensors/time\":\"" + currentTime + "\"}");
    client.PATCH("{\"Status/Sensors/Distance\":" + String(distance) + "}");
    client.PATCH("{\"Status/Sensors/Pulse\":" + String(pulse) + "}");
    client.PATCH("{\"Status/Sensors/Temperature\":" + String(temperature) + "}");
}

void Device::updateLedStatus(bool isOn) {
    ledState = isOn;
    client.PATCH("{\"Status/Led\":\"" + String(isOn ? "on" : "off") + "\"}");
}

void Device::sendAlert(const String &message) {
    Serial.println("ALERTA: " + message);
    client.PATCH("{\"Status/Alerts/Message\":\"" + message + "\"}");
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
    } else {
        client.PATCH("{\"Status/Alerts/panicButton\":\"false\"}");
    }

    float currentPulse = getPulse();
    float currentTemp = getTemperature();
    float currentDistance = getDistance();

    // Evaluar alertas
    if (currentPulse < minPulse || currentPulse > maxPulse) {
        alertMessages += (currentPulse < minPulse) ? "Pulso muy bajo. " : "Pulso muy alto. ";
        alertSent = true;
    }

    if (currentTemp < minTemp || currentTemp > maxTemp) {
        alertMessages += (currentTemp < minTemp) ? "Temperatura muy baja. " : "Temperatura muy alta. ";
        alertSent = true;
    }

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
        client.PATCH("{\"Status/Alerts/Message\":\"\"}");
    }

    Serial.println("--------- Monitor Serie ---------");
    Serial.println("Voltaje de Pulso: " + String(currentPulse));
    Serial.println("Temperatura: " + String(currentTemp) + " °C");
    Serial.println("Distancia: " + String(currentDistance) + " cm");
    Serial.println("-----------------------------");
}

float Device::getPulse() {
    int BPM = ((analogRead(pulse) * (5 / 4095.0))/ 3.3) * 675;
    return BPM;
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

bool Device::isPanicButtonPressed() {
    return digitalRead(panicButtonPin) == LOW;
}

String Device::getCurrentTime() {
    struct tm timeInfo;
    if(!getLocalTime(&timeInfo)) {
        Serial.println("Failed to obtain time");
        return "";
    }
    char timeStringBuffer[20];
    strftime(timeStringBuffer, sizeof(timeStringBuffer), "%d/%m/%Y %H:%M", &timeInfo);
    return String(timeStringBuffer);
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