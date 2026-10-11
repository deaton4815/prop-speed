#pragma once

#include <cstdint>

/*
PulseCounter
Counts rising edges on one interrupt pin.
The ISR increments the count; the main loop reads and clears it once per window.
*/
class PulseCounter
{
    public:
        // configure the pin and start counting
        void begin(uint8_t pin);

        // return the pulses counted since the last call, and reset to 0
        uint32_t takeCount();

    private:
        static void isr();                     // static: attachInterrupt needs a plain function
        static volatile uint32_t s_count;      // shared between ISR and loop(), so volatile
};