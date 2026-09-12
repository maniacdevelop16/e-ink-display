#include "UI.h"

#include <Arduino.h>

#include "Display.h"
#include "Input.h"
#include "Quotes.h"

namespace
{
uint8_t currentQuote = 0;

void displayCurrentQuote()
{
    showQuote(currentQuote);
}
}

void initializeUI()
{
    displayCurrentQuote();
}

void updateUI()
{
    switch (readInputEvent())
    {
        case INPUT_NEXT:
            Serial.println("ROTARY: NEXT");
            currentQuote = (currentQuote + 1) % NUM_QUOTES;
            displayCurrentQuote();
            break;

        case INPUT_PREVIOUS:
            Serial.println("ROTARY: PREVIOUS");
            currentQuote = (currentQuote + NUM_QUOTES - 1) % NUM_QUOTES;
            displayCurrentQuote();
            break;

        case INPUT_PRESS:
            Serial.println("ROTARY: PRESS");
            displayCurrentQuote();
            break;

        case INPUT_NONE:
            break;
    }
}
