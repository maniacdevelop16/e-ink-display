#include "UI.h"

#include <Arduino.h>

#include "Config.h"
#include "Display.h"
#include "Input.h"
#include "Quotes.h"

namespace
{
uint8_t currentQuote = 0;
uint8_t selectedApp = 0;
uint16_t pomodoroRemainingSeconds = POMODORO_DURATION_SECONDS;
uint16_t pomodoroTotalSeconds = POMODORO_DURATION_SECONDS;
uint16_t pomodoroLastRenderedSeconds = POMODORO_DURATION_SECONDS;
unsigned long pomodoroLastTick = 0;
uint8_t pomodoroPartialRefreshCount = 0;
uint8_t menuPartialRefreshCount = 0;
uint8_t breathingPhase = 0;
uint8_t breathingCurrentRound = 1;
uint8_t breathingTotalRounds = BREATHING_DEFAULT_ROUNDS;
unsigned long breathingPhaseStartedAt = 0;
uint8_t breathingPartialRefreshCount = 0;

enum Screen
{
    MENU,
    QUOTE_DISPLAY,
    POMODORO,
    BREATHING
};

Screen currentScreen = MENU;

enum PomodoroState
{
    POMODORO_READY,
    POMODORO_RUNNING,
    POMODORO_PAUSED,
    POMODORO_COMPLETE
};

PomodoroState pomodoroState = POMODORO_READY;

enum BreathingState
{
    BREATHING_READY,
    BREATHING_RUNNING,
    BREATHING_PAUSED,
    BREATHING_COMPLETE
};

BreathingState breathingState = BREATHING_READY;

void displayCurrentQuote()
{
    showQuote(currentQuote);
}

void displayMenu()
{
    showMainMenu(selectedApp);
    menuPartialRefreshCount = 0;
}

void refreshMenuSelection()
{
    if (menuPartialRefreshCount >= MENU_PARTIALS_BEFORE_FULL_REFRESH)
    {
        displayMenu();
        return;
    }

    updateMainMenuSelection(selectedApp);
    ++menuPartialRefreshCount;
}

void displayPomodoro()
{
    const char* status = "READY TO FOCUS";
    const char* action = "PRESS TO START";

    if (pomodoroState == POMODORO_RUNNING)
    {
        status = "FOCUS IN PROGRESS";
        action = "PRESS TO PAUSE";
    }
    else if (pomodoroState == POMODORO_PAUSED)
    {
        status = "PAUSED";
        action = "PRESS TO RESUME";
    }
    else if (pomodoroState == POMODORO_COMPLETE)
    {
        status = "SESSION COMPLETE";
        action = "PRESS TO RESET";
    }

    showPomodoro(pomodoroRemainingSeconds, pomodoroTotalSeconds, status, action);
    pomodoroLastRenderedSeconds = pomodoroRemainingSeconds;
    pomodoroPartialRefreshCount = 0;
}

void resetPomodoro()
{
    pomodoroRemainingSeconds = POMODORO_DURATION_SECONDS;
    pomodoroTotalSeconds = POMODORO_DURATION_SECONDS;
    pomodoroState = POMODORO_READY;
    displayPomodoro();
}

void refreshPomodoroTimer()
{
    if (pomodoroPartialRefreshCount >= POMODORO_PARTIALS_BEFORE_FULL_REFRESH)
    {
        displayPomodoro();
        return;
    }

    updatePomodoroTimer(pomodoroRemainingSeconds, pomodoroTotalSeconds);
    pomodoroLastRenderedSeconds = pomodoroRemainingSeconds;
    ++pomodoroPartialRefreshCount;
}

void updatePomodoroClock()
{
    if (pomodoroState != POMODORO_RUNNING)
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
        pomodoroState = POMODORO_COMPLETE;
        displayPomodoro();
    }
    else
    {
        pomodoroRemainingSeconds -= elapsedSeconds;
        if (pomodoroLastRenderedSeconds - pomodoroRemainingSeconds >= POMODORO_REFRESH_INTERVAL_SECONDS)
        {
            refreshPomodoroTimer();
        }
    }
}

void addPomodoroTime()
{
    if (pomodoroState == POMODORO_COMPLETE)
    {
        pomodoroTotalSeconds = POMODORO_EXTENSION_SECONDS;
        pomodoroRemainingSeconds = POMODORO_EXTENSION_SECONDS;
        pomodoroState = POMODORO_READY;
        displayPomodoro();
        return;
    }

    const uint16_t availableSeconds = POMODORO_MAX_SECONDS - pomodoroTotalSeconds;
    const uint16_t addedSeconds = POMODORO_EXTENSION_SECONDS >= availableSeconds
        ? availableSeconds
        : POMODORO_EXTENSION_SECONDS;
    pomodoroTotalSeconds += addedSeconds;
    pomodoroRemainingSeconds += addedSeconds;
    displayPomodoro();
}

void getBreathingDisplayText(const char*& phaseLabel, const char*& status, const char*& action)
{
    const char* phaseLabels[] = { "INHALE", "HOLD", "EXHALE", "HOLD" };
    const char* phaseMessages[] = {
        "BREATHE IN SLOWLY",
        "STAY STILL AND CALM",
        "BREATHE OUT SLOWLY",
        "REST IN THE STILLNESS"
    };
    phaseLabel = phaseLabels[breathingPhase];
    status = phaseMessages[breathingPhase];
    action = "PRESS: BEGIN";

    if (breathingState == BREATHING_RUNNING)
    {
        action = "PRESS: PAUSE";
    }
    else if (breathingState == BREATHING_PAUSED)
    {
        status = "PAUSED - TAKE YOUR TIME";
        action = "PRESS: RESUME";
    }
    else if (breathingState == BREATHING_COMPLETE)
    {
        phaseLabel = "COMPLETE";
        status = "ALL ROUNDS FINISHED";
        action = "PRESS: RESET";
    }
}

void displayBreathing()
{
    const char* phaseLabel;
    const char* status;
    const char* action;
    getBreathingDisplayText(phaseLabel, status, action);

    showBreathing(breathingPhase, breathingCurrentRound, breathingTotalRounds,
                  phaseLabel, status, action);
    breathingPartialRefreshCount = 0;
}

void refreshBreathingPhase()
{
    if (breathingPartialRefreshCount >= BREATHING_PARTIALS_BEFORE_FULL_REFRESH)
    {
        displayBreathing();
        return;
    }

    const char* phaseLabel;
    const char* status;
    const char* action;
    getBreathingDisplayText(phaseLabel, status, action);
    updateBreathingPhase(breathingPhase, breathingCurrentRound, breathingTotalRounds,
                         phaseLabel, status);
    ++breathingPartialRefreshCount;
}

void resetBreathing()
{
    breathingPhase = 0;
    breathingCurrentRound = 1;
    breathingState = BREATHING_READY;
    displayBreathing();
}

void selectBreathingRounds()
{
    breathingTotalRounds = breathingTotalRounds == BREATHING_MAX_ROUNDS
        ? 1
        : breathingTotalRounds + 1;
    resetBreathing();
}

void updateBreathing()
{
    if (breathingState != BREATHING_RUNNING)
    {
        return;
    }

    unsigned long elapsedPhases = (millis() - breathingPhaseStartedAt) / BREATHING_PHASE_DURATION_MS;
    if (elapsedPhases == 0)
    {
        return;
    }

    breathingPhaseStartedAt += elapsedPhases * BREATHING_PHASE_DURATION_MS;
    while (elapsedPhases > 0 && breathingState == BREATHING_RUNNING)
    {
        --elapsedPhases;
        if (breathingPhase == BREATHING_PHASE_COUNT - 1)
        {
            breathingPhase = 0;
            if (breathingCurrentRound == breathingTotalRounds)
            {
                breathingState = BREATHING_COMPLETE;
            }
            else
            {
                ++breathingCurrentRound;
            }
        }
        else
        {
            ++breathingPhase;
        }
    }

    if (breathingState == BREATHING_COMPLETE)
    {
        displayBreathing();
    }
    else
    {
        refreshBreathingPhase();
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
        currentScreen = BREATHING;
        displayBreathing();
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
            refreshMenuSelection();
        }
        else if (event == INPUT_PREVIOUS)
        {
            selectedApp = (selectedApp + 2) % 3;
            refreshMenuSelection();
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
            addPomodoroTime();
        }
        else if (event == INPUT_PRESS)
        {
            if (pomodoroState == POMODORO_COMPLETE)
            {
                resetPomodoro();
            }
            else
            {
                pomodoroState = pomodoroState == POMODORO_RUNNING
                    ? POMODORO_PAUSED
                    : POMODORO_RUNNING;
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

    if (currentScreen == BREATHING)
    {
        updateBreathing();

        if (event == INPUT_NEXT)
        {
            resetBreathing();
        }
        else if (event == INPUT_PREVIOUS && breathingState != BREATHING_RUNNING)
        {
            selectBreathingRounds();
        }
        else if (event == INPUT_PRESS)
        {
            if (breathingState == BREATHING_COMPLETE)
            {
                resetBreathing();
            }
            else
            {
                breathingState = breathingState == BREATHING_RUNNING
                    ? BREATHING_PAUSED
                    : BREATHING_RUNNING;
                breathingPhaseStartedAt = millis();
                displayBreathing();
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
