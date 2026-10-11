#include <Arduino.h>
#include <Servo.h>

#include "PulseCounter.h"
#include "FastIO.h"

namespace
{
  Servo esc;

  constexpr uint8_t  ESC_PIN         = 9;
  constexpr uint16_t THROTTLE_MIN_US = 1000;  // idle / armed
  constexpr uint16_t THROTTLE_MAX_US = 1300;  // test limit; raise later with care
  constexpr uint32_t ARM_DELAY_MS    = 3000;

  constexpr uint8_t IR_PIN = 2;
  constexpr uint32_t MIN_PULSE_SPACING_US = 1000;   // ignore edges closer than 1 ms

  constexpr uint32_t WINDOW_MS = 1000;
  uint32_t lastWindowMs = 0;

  constexpr uint8_t DEBUG_PIN = 7;

  constexpr uint8_t PULSES_PER_REV = 2; 

  uint16_t throttleUs = THROTTLE_MIN_US;

  PulseCounter pulseCounter;
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(10);                    // keep parseInt() from stalling loop()

  esc.attach(ESC_PIN, 1000, 2000);
  esc.writeMicroseconds(THROTTLE_MIN_US);
  delay(ARM_DELAY_MS);                      // let the ESC arm at idle

  Serial.println("ESC armed. Enter throttle in us (1000-1300), 1000 = stop.");

  pulseCounter.begin(IR_PIN, MIN_PULSE_SPACING_US, DEBUG_PIN);

  lastWindowMs = millis();                  // first window starts now, after the arming delay
}


void loop() {

  if (millis() - lastWindowMs >= WINDOW_MS)
  {
    lastWindowMs += WINDOW_MS;

    uint32_t pulses = pulseCounter.takeCount();

    // revolutions in this window, scaled up to revolutions per minute
    float rpm = (static_cast<float>(pulses) / PULSES_PER_REV) * (60000.0f / WINDOW_MS);

    Serial.print(millis());
    Serial.print(",");
    Serial.println(rpm, 0);
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

}