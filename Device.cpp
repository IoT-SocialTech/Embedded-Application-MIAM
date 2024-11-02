#include "Device.h"
#include <Arduino.h>
#include <time.h>

Device::Device() : lcd(0x27, 16, 2) {}

void Device::setup() {
    pinMode(Ledpot, OUTPUT);
    lcd.init();
    lcd.backlight();
    pinMode(panicButtonPin, INPUT_PULLUP);
}