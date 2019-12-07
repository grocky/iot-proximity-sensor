#include <Arduino.h>

static const uint8  PIN_SENSOR_1 = D5;

void setup() {
    pinMode(PIN_SENSOR_1, INPUT_PULLUP);
    Serial.begin(9600);
}

bool isWaterHigh(int sensorPin) {
    return digitalRead(sensorPin) == 1;
}

void loop() {
    if (isWaterHigh(PIN_SENSOR_1)) {
        Serial.println("Water good");
    } else {
        Serial.println("Water low") ;
    }
    delay(1000);
}
