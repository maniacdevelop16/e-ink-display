#include "UI.h"

#include <Arduino.h>

#include "Display.h"
#include "Input.h"
#include "Quotes.h"

namespace
{
uint8_t currentQuote = 0;
uint8_t selectedApp = 0;
constexpr uint16_t POMODORO_DURATION_SECONDS = 25 * 60;
constexpr uint16_t POMODORO_EXTENSION_SECONDS = 5 * 60;
constexpr uint16_t POMODORO_MAX_SECONDS = 90 * 60;
uint16_t pomodoroRemainingSeconds = POMODORO_DURATION_SECONDS;
unsigned long pomodoroLastTick = 0;
bool pomodoroRunning = false;
bool pomodoroComplete = false;

enum Screen
{
    MENU,
    QUOTE_DISPLAY,
    POMODORO,
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

void displayPomodoro()
{
    showPomodoro(pomodoroRemainingSeconds, pomodoroRunning, pomodoroComplete);
}

void resetPomodoro()
{
    pomodoroRemainingSeconds = POMODORO_DURATION_SECONDS;
    pomodoroRunning = false;
    pomodoroComplete = false;
    displayPomodoro();
}

void updatePomodoroClock()
{
    if (!pomodoroRunning)
    {
        return;
    }

    const unsigned long elapsedSeconds = (millis() - pomodoroLastTick) / 1000;
    if (elapsedSeconds == 0)
    {
        return;
    }

    pomodoroLastTick += elapsedSeconds * 1000UL;
    if (elapsedSeconds >= pomodoroRemainingSeconds)
    {
        pomodoroRemainingSeconds = 0;
        pomodoroRunning = false;
        pomodoroComplete = true;
        displayPomodoro();
    }
    else
    {
        pomodoroRemainingSeconds -= elapsedSeconds;
        // E-paper panels should not be fully refreshed every second.
        if (pomodoroRemainingSeconds % 10 == 0)
        {
            displayPomodoro();
        }
    }
}

void openSelectedApp()
{
    if (selectedApp == 0)
    {
        currentScreen = QUOTE_DISPLAY;
        displayCurrentQuote();
    }
    else if (selectedApp == 1)
    {
        currentScreen = POMODORO;
        displayPomodoro();
    }
    else
    {
        currentScreen = DEMO_THREE;
        showDemo(3);
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
            returnToMenu();
        }
        else if (event == INPUT_LONG_PRESS)
        {
            returnToMenu();
        }
        return;
    }

    if (currentScreen == POMODORO)
    {
        updatePomodoroClock();

        if (event == INPUT_NEXT)
        {
            resetPomodoro();
        }
        else if (event == INPUT_PREVIOUS)
        {
            const uint16_t availableSeconds = POMODORO_MAX_SECONDS - pomodoroRemainingSeconds;
            if (POMODORO_EXTENSION_SECONDS >= availableSeconds)
            {
                pomodoroRemainingSeconds = POMODORO_MAX_SECONDS;
            }
            else
            {
                pomodoroRemainingSeconds += POMODORO_EXTENSION_SECONDS;
            }
            pomodoroComplete = false;
            displayPomodoro();
        }
        else if (event == INPUT_PRESS)
        {
            if (pomodoroComplete)
            {
                resetPomodoro();
            }
            else
            {
                pomodoroRunning = !pomodoroRunning;
                pomodoroLastTick = millis();
                displayPomodoro();
            }
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
