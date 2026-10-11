#include <Arduino.h>

#include "PulseCounter.h"

volatile uint32_t PulseCounter::s_count = 0;
uint32_t PulseCounter::s_lastPulseUs = 0;
uint32_t PulseCounter::s_minSpacingUs = 0;

void PulseCounter::begin(uint8_t pin, uint32_t minSpacingUs)
{
    s_minSpacingUs = minSpacingUs;
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
    uint32_t nowUs = micros();
    if (nowUs - s_lastPulseUs >= s_minSpacingUs)   // only count if enough time has passed
    {
        s_count++;
        s_lastPulseUs = nowUs;                      // restart the lockout from this accepted pulse
    }
}