#pragma once

#include <Arduino.h>
#include <cstdint>

/*
fastWrite
Sets or clears one GPIO pin on the Uno R4 (Renesas RA4M1) by writing the port
set/reset registers directly. The pin must already be configured with
pinMode(pin, OUTPUT).

Adapted from the AVR "direct port manipulation" idea (PORTD |= ...), which does
not exist on this chip:
  POSR - writing a 1 to a bit sets that pin HIGH
  PORR - writing a 1 to a bit sets that pin LOW
g_pin_cfg[pin].pin holds the hardware location: high byte = port, low 4 bits = pin.
*/

namespace FastIO
{
    static R_PORT0_Type *const portTable[] = {R_PORT0, R_PORT1, R_PORT2, R_PORT3,
                                              R_PORT4, R_PORT5, R_PORT6, R_PORT7};
}

static inline void fastWrite(uint8_t pin, bool high)
{
    const uint16_t hw   = g_pin_cfg[pin].pin;
    R_PORT0_Type *port  = FastIO::portTable[hw >> 8];
    const uint16_t mask = static_cast<uint16_t>(1U << (hw & 0xF));

    if (high)
    {
        port->POSR = mask;
    }
    else
    {
        port->PORR = mask;
    }
}
