#include "Device.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
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

void Device::sendMACAddressToFirebase() {
    macAddress = WiFi.macAddress();
    Serial.println("MAC Address: " + macAddress);

    String payload = "{\"MacAddress\":\"" + macAddress + "\"}";

    int httpResponseCode = client.PATCH(payload);
    if (httpResponseCode > 0) {
        Serial.println("MAC Address enviado a Firebase: " + macAddress);
    } else {
        Serial.println("Error enviando MAC Address: " + String(httpResponseCode));
    }
}

void Device::updateFirebase(float pulse, float temperature, float distance) {
    String currentTime = getCurrentTime();
    client.PATCH("{\"Time\":\"" + currentTime + "\"}");
    client.PATCH("{\"Distance\":" + String(distance) + "}");
    client.PATCH("{\"HeartRate\":" + String(pulse) + "}");
    client.PATCH("{\"Temperature\":" + String(temperature) + "}");
}

void Device::authenticateWithServer() {
    HTTPClient httpClient;
    httpClient.begin("https://miam-edge-api.onrender.com/api/v1/auth/login");
    httpClient.addHeader("Content-Type", "application/json");

    // Construir el JSON para la solicitud
    JsonDocument dataRecord;
    dataRecord["id"] = macAddress;
    dataRecord["password"] = macAddress;
    String dataRecordResource;
    serializeJson(dataRecord, dataRecordResource);
    int httpResponseCode = httpClient.POST(dataRecordResource);

    if (httpResponseCode > 0) {
        String responseResource = httpClient.getString();
        StaticJsonDocument<512> response;
        DeserializationError error = deserializeJson(response, responseResource);
        if (!error) {
            String status = response["status"];
            if (status == "SUCCESS") {
                token = response["data"]["token"].as<String>();
                Serial.println("Token almacenado: " + token);
            } else {
                Serial.println("Error en autenticación: " + response["message"].as<String>());
            }
        } else {
            Serial.println("Error al parsear JSON: " + String(error.c_str()));
        }
    } else {
        Serial.println("Error en la solicitud HTTP: " + String(httpResponseCode));
    }
    httpClient.end();
}

void Device::fetchDeviceLimits() {
    if (token.isEmpty()) {
        Serial.println("Error: Token no disponible. Autentícate primero.");
        return;
    }
    // Construir la URL con el MAC Address
    String endpoint = "https://miam-edge-api.onrender.com/api/v1/miam-edge-api/device/" + macAddress;
    HTTPClient httpClient;
    httpClient.begin(endpoint);
    httpClient.addHeader("Authorization", "Bearer " + token);

    int httpResponseCode = httpClient.GET();

    if (httpResponseCode > 0) {
        String responseResource = httpClient.getString();
        //Serial.println("Respuesta del servidor: " + responseResource);
        StaticJsonDocument<512> response;
        DeserializationError error = deserializeJson(response, responseResource);
        if (!error) {
            String status = response["status"];
            if (status == "SUCCESS") {
                // Capturar los valores deseados
                maxPulse = response["data"]["limitHeartRate"];
                maxTemp = response["data"]["limitTemperature"];
                proximityThreshold = response["data"]["limitDistance"];
                Serial.println("Límites obtenidos:");
                Serial.println("Pulso: " + String(maxPulse));
                Serial.println("Temperatura: " + String(maxTemp));
                Serial.println("Distancia: " + String(proximityThreshold));
            } else {
                Serial.println("Error en la respuesta: " + response["message"].as<String>());
            }
        } else {
            Serial.println("Error al parsear JSON: " + String(error.c_str()));
        }
    } else {
        Serial.println("Error en la solicitud HTTP: " + String(httpResponseCode));
    }

    httpClient.end();
}

String Device::getToken() const {
    return token;
}

void Device::updateLedStatus(bool isOn) {
    ledState = String(isOn ? "on" : "off");
    client.PATCH("{\"Led\":\"" + String(isOn ? "on" : "off") + "\"}");
}

void Device::sendAlert(const String &message) {
    Serial.println("ALERTA: " + message);
    client.PATCH("{\"Alert\":\"" + message + "\"}");
}


void Device::sendMetricsToServer(const String& alert, float distance, float pulse, float temperature, const String& ledStatus, const String& panicButton, const String& currentTime) {
    if (token.isEmpty()) {
        Serial.println("Error: Token no disponible. Autentícate primero.");
        return;
    }
    String data = "{\"Alert\":\"" + alert + "\",\"Distance\":"+
                      String(distance) + ",\"HeartRate\":" + String(pulse) + ",\"Led\":\"" +
                      ledStatus + "\",\"MacAddress\":\"" + macAddress + "\",\"PanicButton\":\"" +
                      panicButton + "\",\"Temperature\":" + String(temperature) + ",\"Time\":\"" +
                      currentTime + "\"}";



    JsonDocument dataRecord;
    dataRecord["data"] = data;
    String dataRecordResource;
    serializeJson(dataRecord, dataRecordResource);
    Serial.println(dataRecordResource);
    // Configurar la solicitud HTTP
    HTTPClient httpClient;
    httpClient.begin("https://miam-edge-api.onrender.com/api/v1/miam-edge-api/metrics");
    httpClient.addHeader("Content-Type", "application/json");
    httpClient.addHeader("Authorization", "Bearer " + token);

    // Enviar la solicitud
    int httpResponseCode = httpClient.POST(dataRecordResource);

    if (httpResponseCode > 0) {
        String response = httpClient.getString();
        Serial.println("Respuesta del servidor: " + response);
    } else {
        Serial.println("Error en la solicitud HTTP: " + String(httpResponseCode));
    }
    httpClient.end();
}

void Device::updateReadings() {

    String alertMessages = "";
    bool alertSent = false;
    bool panicButtonActivated = isPanicButtonPressed();

    // Obtener lecturas de los sensores
    float currentPulse = getPulse();
    float currentTemp = getTemperature();
    float currentDistance = getDistance();

    if (panicButtonActivated) {
        updateLedStatus(true);
        alertMessages = "ALERTA DE PANICO ACTIVADA!";
        sendAlert(alertMessages);
        sendMetricsToServer(alertMessages, currentDistance, currentPulse, currentTemp, "on", "true", getCurrentTime());
        client.PATCH("{\"PanicButton\":\"true\"}");
        lcdClear();
        lcdSetCursor(0, 0);
        lcdPrint("PANIC ALERT!");
        lcdSetCursor(0, 1);
        lcdPrint("LLAMAR AYUDA!");
        analogWrite(Ledpot, 255);
        delay(1000);
        return;
    } else {
        client.PATCH("{\"PanicButton\":\"false\"}");
    }

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

    // Manejo de estado del LED y envío de alerta si hay alguna
    if (alertSent) {
        updateLedStatus(true);
        analogWrite(Ledpot, 255);
        sendAlert(alertMessages);
    } else {
        updateLedStatus(false);
        analogWrite(Ledpot, 0);
        client.PATCH("{\"Alert\":\"\"}");
    }

    sendMetricsToServer(alertMessages,currentDistance,currentPulse,currentTemp,ledState,"false",getCurrentTime());

    // Mostrar lecturas en el monitor serie
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