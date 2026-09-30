#include "UI.h"

#include <Arduino.h>

#include "Config.h"
#include "Display.h"
#include "Input.h"
#include "Quotes.h"
#include "Settings.h"

namespace
{
uint8_t currentQuote = 0;
uint8_t selectedApp = 0;
uint8_t selectedSetting = 0;
uint8_t selectedQuoteSetting = 0;
unsigned long quoteLastChangedAt = 0;
uint16_t pomodoroRemainingSeconds = POMODORO_DURATION_SECONDS;
uint16_t pomodoroTotalSeconds = POMODORO_DURATION_SECONDS;
uint16_t pomodoroLastRenderedSeconds = POMODORO_DURATION_SECONDS;
unsigned long pomodoroLastTick = 0;
uint8_t pomodoroPartialRefreshCount = 0;
uint8_t menuPartialRefreshCount = 0;
uint8_t quoteSettingsPartialRefreshCount = 0;
uint8_t settingsPartialRefreshCount = 0;
uint8_t breathingPhase = 0;
uint8_t breathingCurrentRound = 1;
uint8_t breathingTotalRounds = BREATHING_DEFAULT_ROUNDS;
unsigned long breathingPhaseStartedAt = 0;
uint8_t breathingPartialRefreshCount = 0;

enum Screen
{
    MENU,
    QUOTE_DISPLAY,
    QUOTE_SETTINGS,
    POMODORO,
    BREATHING,
    SETTINGS
};

Screen currentScreen = MENU;

void saveCurrentView()
{
    saveResumeState(static_cast<uint8_t>(currentScreen), currentQuote, selectedApp);
}

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
    quoteLastChangedAt = millis();
    saveCurrentView();
}

void displayQuoteSettings()
{
    showQuoteSettings(selectedQuoteSetting);
    quoteSettingsPartialRefreshCount = 0;
}

void updateQuoteChangeTimer()
{
    const uint32_t intervalSeconds = getQuoteDisplaySettings().autoChangeIntervalSeconds;
    if (intervalSeconds == 0)
    {
        quoteLastChangedAt = millis();
        return;
    }

    const unsigned long intervalMs = intervalSeconds * 1000UL;
    const unsigned long elapsedMs = millis() - quoteLastChangedAt;
    if (elapsedMs < intervalMs)
    {
        return;
    }

    const uint32_t changes = elapsedMs / intervalMs;
    currentQuote = (currentQuote + changes) % NUM_QUOTES;
    quoteLastChangedAt += changes * intervalMs;
    showQuote(currentQuote);
    saveCurrentView();
}

void displayMenu()
{
    showMainMenu(selectedApp);
    menuPartialRefreshCount = 0;
}

void displaySettings()
{
    showSettings(selectedSetting);
    settingsPartialRefreshCount = 0;
}

void refreshQuoteSettings()
{
    if (!usePartialRefresh() || quoteSettingsPartialRefreshCount >=
        partialRefreshLimit(SETTINGS_PARTIALS_BEFORE_FULL_REFRESH))
    {
        displayQuoteSettings();
        return;
    }

    updateQuoteSettingsSelection(selectedQuoteSetting);
    ++quoteSettingsPartialRefreshCount;
}

void refreshSettings()
{
    if (!usePartialRefresh() || settingsPartialRefreshCount >=
        partialRefreshLimit(SETTINGS_PARTIALS_BEFORE_FULL_REFRESH))
    {
        displaySettings();
        return;
    }

    updateSettingsSelection(selectedSetting);
    ++settingsPartialRefreshCount;
}

void refreshMenuSelection()
{
    if (!usePartialRefresh())
    {
        displayMenu();
        return;
    }

    if (menuPartialRefreshCount >= partialRefreshLimit(MENU_PARTIALS_BEFORE_FULL_REFRESH))
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

    if (pomodoroState == POMODORO_RUNNING)
    {
        status = "FOCUS IN PROGRESS";
    }
    else if (pomodoroState == POMODORO_PAUSED)
    {
        status = "PAUSED";
    }
    else if (pomodoroState == POMODORO_COMPLETE)
    {
        status = "SESSION COMPLETE";
    }

    showPomodoro(pomodoroRemainingSeconds, pomodoroTotalSeconds, status);
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
    if (!usePartialRefresh())
    {
        displayPomodoro();
        return;
    }

    if (pomodoroPartialRefreshCount >= partialRefreshLimit(POMODORO_PARTIALS_BEFORE_FULL_REFRESH))
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
        if (pomodoroLastRenderedSeconds - pomodoroRemainingSeconds >= pomodoroRefreshIntervalSeconds())
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

void getBreathingDisplayText(const char*& phaseLabel, const char*& status)
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
    if (breathingState == BREATHING_PAUSED)
    {
        status = "PAUSED - TAKE YOUR TIME";
    }
    else if (breathingState == BREATHING_COMPLETE)
    {
        phaseLabel = "COMPLETE";
        status = "ALL ROUNDS FINISHED";
    }
}

void displayBreathing()
{
    const char* phaseLabel;
    const char* status;
    getBreathingDisplayText(phaseLabel, status);

    showBreathing(breathingPhase, breathingCurrentRound, breathingTotalRounds,
                  phaseLabel, status);
    breathingPartialRefreshCount = 0;
}

void refreshBreathingPhase()
{
    if (!usePartialRefresh())
    {
        displayBreathing();
        return;
    }

    if (breathingPartialRefreshCount >= partialRefreshLimit(BREATHING_PARTIALS_BEFORE_FULL_REFRESH))
    {
        displayBreathing();
        return;
    }

    const char* phaseLabel;
    const char* status;
    getBreathingDisplayText(phaseLabel, status);
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
    else if (selectedApp == 2)
    {
        currentScreen = BREATHING;
        displayBreathing();
    }
    else
    {
        currentScreen = SETTINGS;
        displaySettings();
    }
    saveCurrentView();
}

void returnToMenu()
{
    currentScreen = MENU;
    displayMenu();
    saveCurrentView();
}

void goBack()
{
    if (currentScreen == QUOTE_SETTINGS)
    {
        currentScreen = QUOTE_DISPLAY;
        displayCurrentQuote();
    }
    else if (currentScreen != MENU)
    {
        returnToMenu();
    }
}
}

void initializeUI()
{
    initializeSettings();
    currentQuote = savedQuote() < NUM_QUOTES ? savedQuote() : 0;
    selectedApp = savedLauncherSelection() < NUM_LAUNCHER_APPS ? savedLauncherSelection() : 0;
    const uint8_t previousScreen = savedScreen();
    currentScreen = previousScreen <= SETTINGS ? static_cast<Screen>(previousScreen) : MENU;

    if (currentScreen == QUOTE_DISPLAY)
    {
        displayCurrentQuote();
    }
    else if (currentScreen == QUOTE_SETTINGS)
    {
        displayQuoteSettings();
    }
    else if (currentScreen == POMODORO)
    {
        displayPomodoro();
    }
    else if (currentScreen == BREATHING)
    {
        displayBreathing();
    }
    else if (currentScreen == SETTINGS)
    {
        displaySettings();
    }
    else
    {
        displayMenu();
    }
}

void updateUI()
{
    const InputEvent event = readInputEvent();

    if (event == INPUT_MENU)
    {
        if (currentScreen != MENU)
        {
            returnToMenu();
        }
        return;
    }

    if (event == INPUT_EXIT)
    {
        goBack();
        return;
    }

    if (currentScreen == MENU)
    {
        if (event == INPUT_NEXT)
        {
            selectedApp = (selectedApp + 1) % NUM_LAUNCHER_APPS;
            refreshMenuSelection();
            saveCurrentView();
        }
        else if (event == INPUT_PREVIOUS)
        {
            selectedApp = (selectedApp + NUM_LAUNCHER_APPS - 1) % NUM_LAUNCHER_APPS;
            refreshMenuSelection();
            saveCurrentView();
        }
        else if (event == INPUT_PRESS)
        {
            openSelectedApp();
        }
        return;
    }

    if (currentScreen == QUOTE_DISPLAY)
    {
        updateQuoteChangeTimer();

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
            currentScreen = QUOTE_SETTINGS;
            saveCurrentView();
            displayQuoteSettings();
        }
        return;
    }

    if (currentScreen == QUOTE_SETTINGS)
    {
        if (event == INPUT_NEXT)
        {
            selectedQuoteSetting = (selectedQuoteSetting + 1) % NUM_QUOTE_SETTINGS_ITEMS;
            refreshQuoteSettings();
        }
        else if (event == INPUT_PREVIOUS)
        {
            selectedQuoteSetting = (selectedQuoteSetting + NUM_QUOTE_SETTINGS_ITEMS - 1) % NUM_QUOTE_SETTINGS_ITEMS;
            refreshQuoteSettings();
        }
        else if (event == INPUT_PRESS)
        {
            if (selectedQuoteSetting == 0)
            {
                cycleQuoteChangeInterval();
            }
            else if (selectedQuoteSetting == 1)
            {
                cycleQuoteTheme();
            }
            else
            {
                resetQuoteDisplaySettings();
            }
            quoteLastChangedAt = millis();
            refreshQuoteSettings();
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
        return;
    }

    if (currentScreen == SETTINGS)
    {
        if (event == INPUT_NEXT)
        {
            selectedSetting = (selectedSetting + 1) % NUM_SETTINGS_ITEMS;
            refreshSettings();
        }
        else if (event == INPUT_PREVIOUS)
        {
            selectedSetting = (selectedSetting + NUM_SETTINGS_ITEMS - 1) % NUM_SETTINGS_ITEMS;
            refreshSettings();
        }
        else if (event == INPUT_PRESS)
        {
            if (selectedSetting == 0)
            {
                cycleFontChoice();
            }
            else if (selectedSetting == 1)
            {
                cycleFontSize();
            }
            else if (selectedSetting == 2)
            {
                togglePartialRefresh();
            }
            else if (selectedSetting == 3)
            {
                cycleRefreshStyle();
            }
            else
            {
                resetSettings();
            }
            if (selectedSetting == 0 || selectedSetting == 1 || selectedSetting == 4)
            {
                displaySettings();
            }
            else
            {
                refreshSettings();
            }
        }
        return;
    }
}
