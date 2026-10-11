#include <Arduino.h>

#include "PulseCounter.h"

volatile uint32_t PulseCounter::s_count = 0;

void PulseCounter::begin(uint8_t pin)
{
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
    s_count++;                 // nothing else in the ISR: no Serial, no delay
}