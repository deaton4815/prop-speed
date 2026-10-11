#include <Arduino.h>
#include <Servo.h>

#include "PulseCounter.h"

namespace
{
  Servo esc;

  constexpr uint8_t  ESC_PIN         = 9;
  constexpr uint16_t THROTTLE_MIN_US = 1000;  // idle / armed
  constexpr uint16_t THROTTLE_MAX_US = 1300;  // test limit; raise later with care
  constexpr uint32_t ARM_DELAY_MS    = 3000;

  constexpr uint8_t IR_PIN = 2;

  constexpr uint32_t WINDOW_MS = 1000;
  uint32_t lastWindowMs = 0;

  constexpr uint8_t DEBUG_PIN = 7;

  uint16_t throttleUs = THROTTLE_MIN_US;

  PulseCounter pulseCounter;

  bool lastState = false;
  bool newState = false;
}

void setup() {
  Serial.begin(115200);

  esc.attach(ESC_PIN, 1000, 2000);
  esc.writeMicroseconds(THROTTLE_MIN_US);
  delay(ARM_DELAY_MS);                      // let the ESC arm at idle

  Serial.println("ESC armed. Enter throttle in us (1000-1300), 1000 = stop.");

  pinMode(DEBUG_PIN, OUTPUT);

  pulseCounter.begin(IR_PIN);

}


void loop() {

  if (millis() - lastWindowMs >= WINDOW_MS)
  {
    lastWindowMs += WINDOW_MS;

    uint32_t pulses = pulseCounter.takeCount();

    Serial.print(millis());
    Serial.print(",");
    Serial.println(pulses);
  }

  if (Serial.available() > 0) {
    long requested = Serial.parseInt();
    if (requested >= THROTTLE_MIN_US) {     // ignore junk / stray newline (parses as 0)
      throttleUs = constrain(requested, THROTTLE_MIN_US, THROTTLE_MAX_US);
      Serial.print("Throttle: ");
      Serial.print(throttleUs);
      Serial.println(" us");
    }
  }
  esc.writeMicroseconds(throttleUs);

  newState = digitalRead(IR_PIN);
  if (newState != lastState) {
    lastState = newState;
    Serial.println("State Change!");
    Serial.println(lastState);
  }
  
}