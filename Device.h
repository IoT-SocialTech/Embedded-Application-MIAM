#ifndef DEVICE_H
#define DEVICE_H

#include <LiquidCrystal_I2C.h>


class Device {
  public:
    Device();
    void setup();
    void updateReadings();
    void updateLedStatus(bool isOn);
    void sendAlert(const String &message);

    float getPulse();
    float getDistance();
    float getTemperature();
    bool isPanicButtonPressed();
    void updateLedStatus(bool isOn);


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


    const float proximityThreshold = 30.0;
    bool ledState = false;

    LiquidCrystal_I2C lcd;
};



#endif
