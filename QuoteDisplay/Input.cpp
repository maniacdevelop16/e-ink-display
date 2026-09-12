#include "Input.h"

#include <Arduino.h>

#include "Config.h"

namespace
{
bool wasPressed(uint8_t pin)
{
    if (digitalRead(pin) != LOW)
    {
        return false;
    }

    delay(DEBOUNCE_TIME_MS);
    if (digitalRead(pin) != LOW)
    {
        return false;
    }

    while (digitalRead(pin) == LOW)
    {
        delay(10);
    }
    delay(DEBOUNCE_TIME_MS);
    return true;
}
}

void initializeInput()
{
    pinMode(ROTARY_UP, INPUT_PULLUP);
    pinMode(ROTARY_DOWN, INPUT_PULLUP);
    pinMode(ROTARY_PRESS, INPUT_PULLUP);
}

InputEvent readInputEvent()
{
    if (wasPressed(ROTARY_UP))
    {
        return INPUT_NEXT;
    }
    if (wasPressed(ROTARY_DOWN))
    {
        return INPUT_PREVIOUS;
    }
    if (wasPressed(ROTARY_PRESS))
    {
        return INPUT_PRESS;
    }
    return INPUT_NONE;
}
