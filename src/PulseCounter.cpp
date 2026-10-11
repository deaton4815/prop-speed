#include <Arduino.h>

#include "PulseCounter.h"
#include "FastIO.h"

volatile uint32_t PulseCounter::s_count = 0;
uint32_t PulseCounter::s_lastPulseUs = 0;
uint32_t PulseCounter::s_minSpacingUs = 0;
uint8_t  PulseCounter::s_debugPin = 0;

void PulseCounter::begin(uint8_t pin, uint32_t minSpacingUs, uint8_t debugPin)
{
    s_minSpacingUs = minSpacingUs;
    s_debugPin = debugPin;
    pinMode(debugPin, OUTPUT);
    fastWrite(debugPin, false);          // idle LOW
    pinMode(pin, INPUT);
    attachInterrupt(digitalPinToInterrupt(pin), isr, RISING);
}

uint32_t PulseCounter::takeCount()
{
    noInterrupts();            // the copy and the reset must not be split by an interrupt
    uint32_t count = s_count;
    s_count = 0;
    interrupts();
    return count;
}

void PulseCounter::isr()
{
    fastWrite(s_debugPin, true);                    // scope marker: ISR entered

    uint32_t nowUs = micros();
    if (nowUs - s_lastPulseUs >= s_minSpacingUs)   // only count if enough time has passed
    {
        s_count++;
        s_lastPulseUs = nowUs;                      // restart the lockout from this accepted pulse
    }

    fastWrite(s_debugPin, false);                   // scope marker: ISR done
}