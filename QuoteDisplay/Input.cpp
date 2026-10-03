#include "Input.h"

#include <Arduino.h>
#include <driver/rtc_io.h>

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
    rtc_gpio_deinit(static_cast<gpio_num_t>(MENU_BUTTON));
    pinMode(ROTARY_UP, INPUT_PULLUP);
    pinMode(ROTARY_DOWN, INPUT_PULLUP);
    pinMode(ROTARY_PRESS, INPUT_PULLUP);
    pinMode(MENU_BUTTON, INPUT_PULLUP);
    pinMode(EXIT_BUTTON, INPUT_PULLUP);
}

InputEvent readInputEvent()
{
    if (wasPressed(MENU_BUTTON))
    {
        return INPUT_MENU;
    }
    if (wasPressed(EXIT_BUTTON))
    {
        return INPUT_EXIT;
    }
    if (wasPressed(ROTARY_UP))
    {
        return INPUT_NEXT;
    }
    if (wasPressed(ROTARY_DOWN))
    {
        return INPUT_PREVIOUS;
    }
    return wasPressed(ROTARY_PRESS) ? INPUT_PRESS : INPUT_NONE;
}
