#include <Arduino.h>

#include "Mng_Mess1.h"

Mng_Mess1 mess1;

void setup() {
    Serial.begin(115200);
    pinMode(2, OUTPUT);
    pinMode(14, INPUT_PULLUP);

    mess1.setup();
}

void loop() {
    mess1.run();

    // Serial.println("IM HERE");
    // digitalWrite(2, !digitalRead(2));
    // delay(2000);
}