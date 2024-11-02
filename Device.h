#ifndef DEVICE_H
#define DEVICE_H

#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
#include <WiFi.h>

class Device {
public:
  Device();
  void setup();
  void updateReadings();
  void connectToWiFi(const char* ssid, const char* password);
  void connectToFirebase(const String& url);
  void updateFirebase(float pulse, float temperature, float distance);
  void updateLedStatus(bool isOn);
  void sendAlert(const String &message);
  String getCurrentTime();
  float getPulse();
  float getDistance();
  float getTemperature();
  bool isPanicButtonPressed();

  void lcdSetCursor(int col, int row);
  void lcdPrint(const String &text);
  void lcdClear();

private:
  const int Ledpot = 4;
  const int panicButtonPin = 5;
  const int tri = 12;
  const int eco = 14;
  const int pulse = 35;
  const int temperature = 32;

  const float Res = 4095.0;
  const float Vmax = 3.3;
  const int minPulse = 60;
  const int maxPulse = 100;
  const float minTemp = 34.0;
  const float maxTemp = 38.0;
  const float proximityThreshold = 30.0;
  const float dangerousThreshold = 100.0;

  LiquidCrystal_I2C lcd;
  HTTPClient client;
  String databaseUrl;
  bool ledState = false;
};

#endif
