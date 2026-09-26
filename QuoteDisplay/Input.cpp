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

InputEvent readPressEvent()
{
    if (digitalRead(ROTARY_PRESS) != LOW)
    {
        return INPUT_NONE;
    }

    delay(DEBOUNCE_TIME_MS);
    if (digitalRead(ROTARY_PRESS) != LOW)
    {
        return INPUT_NONE;
    }

    const unsigned long pressedAt = millis();
    while (digitalRead(ROTARY_PRESS) == LOW)
    {
        delay(10);
    }

    const bool isLongPress = millis() - pressedAt >= LONG_PRESS_TIME_MS;
    delay(DEBOUNCE_TIME_MS);
    return isLongPress ? INPUT_LONG_PRESS : INPUT_PRESS;
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
    return readPressEvent();
}
