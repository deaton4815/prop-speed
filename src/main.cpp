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

  // ---- Throttle profile -------------------------------------------------
  // Started with 's', cancelled with 'x' or by typing a throttle value.
  // Edit the table to change the test. Every value is still clamped to
  // THROTTLE_MIN_US..THROTTLE_MAX_US.
  struct ProfileStep
  {
    uint16_t throttleUs;
    uint32_t durationMs;
  };

  constexpr ProfileStep PROFILE[] = {
    {1000,  5000},   // idle baseline
    {1170, 15000},
    {1220, 15000},
    {1270, 15000},
    {1220, 15000},
    {1170, 15000},
    {1000,  5000},   // spin down
  };
  constexpr uint8_t PROFILE_STEPS = sizeof(PROFILE) / sizeof(PROFILE[0]);

  bool     profileRunning     = false;
  uint8_t  profileIndex       = 0;
  uint32_t profileStepStartMs = 0;

  // ---- Helpers ----------------------------------------------------------
  // Lines starting with '#' are messages, not data. Load the CSV with
  // comment='#' (pandas) or delete those lines.

  void printHeader()
  {
    Serial.println("time_ms,rpm,throttle_us");
  }

  void setThrottle(uint16_t us)
  {
    throttleUs = constrain(us, THROTTLE_MIN_US, THROTTLE_MAX_US);
  }

  void startProfile()
  {
    profileRunning     = true;
    profileIndex       = 0;
    profileStepStartMs = millis();
    setThrottle(PROFILE[0].throttleUs);
    Serial.println("# Profile started");
  }

  void stopMotor()
  {
    profileRunning = false;
    setThrottle(THROTTLE_MIN_US);
    Serial.println("# Motor stopped");
  }

  // call every loop() pass; moves to the next profile step when the time is up
  void updateProfile()
  {
    if (!profileRunning)
    {
      return;
    }

    if (millis() - profileStepStartMs >= PROFILE[profileIndex].durationMs)
    {
      profileIndex++;
      if (profileIndex >= PROFILE_STEPS)
      {
        profileRunning = false;
        setThrottle(THROTTLE_MIN_US);
        Serial.println("# Profile finished");
        return;
      }
      profileStepStartMs = millis();
      setThrottle(PROFILE[profileIndex].throttleUs);
    }
  }

  // Commands:  1000-1300 = throttle in us (1000 = stop)
  //            s = run the throttle profile   x = stop   h = print CSV header
  void handleSerial()
  {
    while (Serial.available() > 0)
    {
      int c = Serial.peek();

      if (c == 's')
      {
        Serial.read();
        startProfile();
      }
      else if (c == 'x')
      {
        Serial.read();
        stopMotor();
      }
      else if (c == 'h')
      {
        Serial.read();
        printHeader();
      }
      else if (c >= '0' && c <= '9')
      {
        long requested = Serial.parseInt();
        if (requested >= THROTTLE_MIN_US)    // ignore junk
        {
          profileRunning = false;            // a typed value cancels the profile
          setThrottle(requested);
          Serial.print("# Throttle: ");
          Serial.print(throttleUs);
          Serial.println(" us");
        }
      }
      else
      {
        Serial.read();                       // discard newline / stray characters
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(10);                    // keep parseInt() from stalling loop()

  esc.attach(ESC_PIN, 1000, 2000);
  esc.writeMicroseconds(THROTTLE_MIN_US);
  delay(ARM_DELAY_MS);                      // let the ESC arm at idle

  Serial.println("# ESC armed. Commands: 1000-1300 = throttle us (1000 = stop), s = run profile, x = stop, h = header");

  pulseCounter.begin(IR_PIN, MIN_PULSE_SPACING_US, DEBUG_PIN);

  printHeader();

  lastWindowMs = millis();                  // first window starts now, after the arming delay
}


void loop() {

  handleSerial();
  updateProfile();
  esc.writeMicroseconds(throttleUs);

  if (millis() - lastWindowMs >= WINDOW_MS)
  {
    lastWindowMs += WINDOW_MS;

    uint32_t pulses = pulseCounter.takeCount();

    // revolutions in this window, scaled up to revolutions per minute
    float rpm = (static_cast<float>(pulses) / PULSES_PER_REV) * (60000.0f / WINDOW_MS);

    // one CSV row: time, measured RPM, commanded throttle
    Serial.print(millis());
    Serial.print(",");
    Serial.print(rpm, 0);
    Serial.print(",");
    Serial.println(throttleUs);
  }
}
