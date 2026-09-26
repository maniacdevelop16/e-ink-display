#include "UI.h"

#include <Arduino.h>

#include "Display.h"
#include "Input.h"
#include "Quotes.h"

namespace
{
uint8_t currentQuote = 0;
uint8_t selectedApp = 0;

enum Screen
{
    MENU,
    QUOTE_DISPLAY,
    DEMO_TWO,
    DEMO_THREE
};

Screen currentScreen = MENU;

void displayCurrentQuote()
{
    showQuote(currentQuote);
}

void displayMenu()
{
    showMainMenu(selectedApp);
}

void openSelectedApp()
{
    currentScreen = static_cast<Screen>(selectedApp + 1);
    if (currentScreen == QUOTE_DISPLAY)
    {
        displayCurrentQuote();
    }
    else
    {
        showDemo(selectedApp + 1);
    }
}

void returnToMenu()
{
    currentScreen = MENU;
    displayMenu();
}
}

void initializeUI()
{
    displayMenu();
}

void updateUI()
{
    const InputEvent event = readInputEvent();

    if (currentScreen == MENU)
    {
        if (event == INPUT_NEXT)
        {
            selectedApp = (selectedApp + 1) % 3;
            displayMenu();
        }
        else if (event == INPUT_PREVIOUS)
        {
            selectedApp = (selectedApp + 2) % 3;
            displayMenu();
        }
        else if (event == INPUT_PRESS)
        {
            openSelectedApp();
        }
        return;
    }

    if (currentScreen == QUOTE_DISPLAY)
    {
        if (event == INPUT_NEXT)
        {
            currentQuote = (currentQuote + 1) % NUM_QUOTES;
            displayCurrentQuote();
        }
        else if (event == INPUT_PREVIOUS)
        {
            currentQuote = (currentQuote + NUM_QUOTES - 1) % NUM_QUOTES;
            displayCurrentQuote();
        }
        else if (event == INPUT_PRESS)
        {
            displayCurrentQuote();
        }
        else if (event == INPUT_LONG_PRESS)
        {
            returnToMenu();
        }
        return;
    }

    if (event == INPUT_PRESS || event == INPUT_LONG_PRESS)
    {
        returnToMenu();
    }
}
