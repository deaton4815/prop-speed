#pragma once

#include <cstdint>

/*
PulseCounter
Counts rising edges on one interrupt pin.
The ISR increments the count; the main loop reads and clears it once per window.
Edges closer together than a minimum spacing are ignored (lockout against chatter).
*/
class PulseCounter
{
    public:
        // configure the pin and start counting
        // minSpacingUs: ignore edges closer together than this
        // debugPin: driven HIGH for the duration of the ISR, for scope timing
        void begin(uint8_t pin, uint32_t minSpacingUs, uint8_t debugPin);

        // return the pulses counted since the last call, and reset to 0
        uint32_t takeCount();

    private:
        static void isr();                     // static: attachInterrupt needs a plain function
        static volatile uint32_t s_count;      // shared between ISR and loop(), so volatile
        static uint32_t s_lastPulseUs;         // only touched inside the ISR
        static uint32_t s_minSpacingUs;        // set once in begin(), before the ISR is attached
        static uint8_t  s_debugPin;            // set once in begin(), before the ISR is attached
};