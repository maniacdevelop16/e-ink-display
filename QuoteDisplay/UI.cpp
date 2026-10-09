#include "UI.h"

#include <Arduino.h>
#include <driver/rtc_io.h>
#include <esp_attr.h>
#include <esp_sleep.h>
#include <time.h>

#include "Config.h"
#include "Display.h"
#include "Input.h"
#include "Quotes.h"
#include "Settings.h"

namespace
{
constexpr uint32_t RETAINED_QUOTE_MAGIC = 0x51555445;
constexpr uint32_t DAILY_SCHEDULE_MAGIC = 0x4D4F524E;
enum TimerWakeReason : uint8_t
{
    TIMER_WAKE_NONE,
    TIMER_WAKE_AUTO_QUOTE,
    TIMER_WAKE_DAILY_QUOTE
};

RTC_DATA_ATTR uint32_t retainedQuoteMagic = 0;
RTC_DATA_ATTR uint8_t retainedQuote = 0;
RTC_DATA_ATTR bool retainedQuoteScreensaverMode = false;
RTC_DATA_ATTR uint32_t dailyScheduleMagic = 0;
RTC_DATA_ATTR time_t nextDailyQuoteAt = 0;
RTC_DATA_ATTR uint8_t retainedTimerWakeReason = TIMER_WAKE_NONE;

uint8_t currentQuote = 0;
uint8_t selectedApp = 0;
uint8_t selectedSetting = 0;
uint8_t selectedQuoteSetting = 0;
bool quoteScreensaverMode = false;
unsigned long lastUserActivityAt = 0;
unsigned long quoteLastChangedAt = 0;
unsigned long quoteLastActivityAt = 0;
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
uint8_t breathingVisualStep = 0;
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

void displayCurrentQuote(bool persistView = true)
{
    showQuote(currentQuote, quoteScreensaverMode);
    quoteLastChangedAt = millis();
    quoteLastActivityAt = quoteLastChangedAt;
    if (persistView)
    {
        saveCurrentView();
    }
}

void displayQuoteSettings()
{
    showQuoteSettings(selectedQuoteSetting);
    quoteSettingsPartialRefreshCount = 0;
}

void advanceQuoteForward(uint32_t steps = 1)
{
    while (steps-- > 0)
    {
        const uint8_t count = quoteCount(getQuoteDisplaySettings().category);
        if (currentQuote + 1 < count)
        {
            ++currentQuote;
        }
        else
        {
            currentQuote = 0;
            cycleQuoteCategory();
        }
    }
}

void advanceQuoteBackward()
{
    if (currentQuote > 0)
    {
        --currentQuote;
        return;
    }

    cycleQuoteCategoryBackward();
    currentQuote = quoteCount(getQuoteDisplaySettings().category) - 1;
}

uint32_t advanceDailyQuoteIfDue(bool forceUpdate = false)
{
    if (!dailyMorningEnabled() || dailyScheduleMagic != DAILY_SCHEDULE_MAGIC)
    {
        return 0;
    }

    const time_t now = time(nullptr);
    if (!forceUpdate && now < nextDailyQuoteAt)
    {
        return 0;
    }

    uint32_t changes = 1;
    if (now > nextDailyQuoteAt)
    {
        changes += static_cast<uint32_t>(
            (now - nextDailyQuoteAt) / DAILY_QUOTE_INTERVAL_SECONDS);
    }
    advanceQuoteForward(changes);
    nextDailyQuoteAt += static_cast<time_t>(changes) * DAILY_QUOTE_INTERVAL_SECONDS;
    retainedQuote = currentQuote;
    return changes;
}

void toggleDailyMorningSchedule()
{
    if (dailyMorningEnabled())
    {
        setDailyMorningEnabled(false);
        dailyScheduleMagic = 0;
        nextDailyQuoteAt = 0;
        return;
    }

    setDailyMorningEnabled(true);
    dailyScheduleMagic = DAILY_SCHEDULE_MAGIC;
    nextDailyQuoteAt = time(nullptr) + DAILY_QUOTE_INTERVAL_SECONDS;
}

void serviceDailyMorningSchedule()
{
    if (advanceDailyQuoteIfDue() == 0)
    {
        return;
    }

    saveCurrentView();
    if (currentScreen == QUOTE_DISPLAY)
    {
        displayCurrentQuote(false);
    }
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
    advanceQuoteForward(changes);
    quoteLastChangedAt += changes * intervalMs;
    showQuote(currentQuote, quoteScreensaverMode);
    quoteLastActivityAt = millis();
}

void enterQuoteDeepSleep()
{
    retainedQuoteMagic = RETAINED_QUOTE_MAGIC;
    retainedQuote = currentQuote;
    retainedQuoteScreensaverMode = quoteScreensaverMode;
    flushSettingsPersistence();

    rtc_gpio_pullup_en(static_cast<gpio_num_t>(MENU_BUTTON));
    rtc_gpio_pulldown_dis(static_cast<gpio_num_t>(MENU_BUTTON));
    esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(MENU_BUTTON), 0);

    uint64_t nextWakeDelayUs = 0;
    TimerWakeReason wakeReason = TIMER_WAKE_NONE;
    const uint32_t intervalSeconds = getQuoteDisplaySettings().autoChangeIntervalSeconds;
    if (intervalSeconds > 0)
    {
        const unsigned long intervalMs = intervalSeconds * 1000UL;
        const unsigned long elapsedMs = millis() - quoteLastChangedAt;
        const unsigned long remainingMs = elapsedMs < intervalMs
            ? intervalMs - elapsedMs
            : 1;
        nextWakeDelayUs = static_cast<uint64_t>(remainingMs) * 1000ULL;
        wakeReason = TIMER_WAKE_AUTO_QUOTE;
    }

    if (dailyMorningEnabled() && dailyScheduleMagic == DAILY_SCHEDULE_MAGIC)
    {
        const time_t now = time(nullptr);
        const uint64_t remainingSeconds = nextDailyQuoteAt > now
            ? static_cast<uint64_t>(nextDailyQuoteAt - now)
            : 1ULL;
        const uint64_t dailyWakeDelayUs = remainingSeconds * 1000000ULL;
        if (nextWakeDelayUs == 0 || dailyWakeDelayUs <= nextWakeDelayUs)
        {
            nextWakeDelayUs = dailyWakeDelayUs;
            wakeReason = TIMER_WAKE_DAILY_QUOTE;
        }
    }

    retainedTimerWakeReason = wakeReason;
    if (nextWakeDelayUs > 0)
    {
        esp_sleep_enable_timer_wakeup(nextWakeDelayUs);
    }

    prepareDisplayForSleep();
    Serial.flush();
    esp_deep_sleep_start();
}

void sleepQuoteDisplayWhenIdle()
{
    const uint16_t timeoutSeconds = deepSleepTimeoutSeconds();
    if (currentScreen == QUOTE_DISPLAY && timeoutSeconds > 0 &&
        millis() - quoteLastActivityAt >= timeoutSeconds * 1000UL)
    {
        enterQuoteDeepSleep();
    }
}

bool showQuoteScreensaverWhenIdle()
{
    const bool supportsScreensaver = currentScreen == MENU ||
        currentScreen == SETTINGS || currentScreen == QUOTE_SETTINGS;
    const uint16_t timeoutSeconds = screensaverTimeoutSeconds();
    if (!supportsScreensaver || timeoutSeconds == 0 ||
        millis() - lastUserActivityAt < timeoutSeconds * 1000UL)
    {
        return false;
    }

    currentScreen = QUOTE_DISPLAY;
    quoteScreensaverMode = true;
    displayCurrentQuote();
    return true;
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
    const char* status = "READY";

    if (pomodoroState == POMODORO_RUNNING)
    {
        status = "FOCUS";
    }
    else if (pomodoroState == POMODORO_PAUSED)
    {
        status = "PAUSED";
    }
    else if (pomodoroState == POMODORO_COMPLETE)
    {
        status = "COMPLETE";
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

const char* getBreathingPhaseLabel()
{
    const char* phaseLabels[] = { "INHALE", "HOLD", "EXHALE", "REST" };
    if (breathingState == BREATHING_PAUSED)
    {
        return "PAUSED";
    }
    if (breathingState == BREATHING_COMPLETE)
    {
        return "COMPLETE";
    }
    if (breathingState == BREATHING_READY)
    {
        return "READY";
    }
    return phaseLabels[breathingPhase];
}

void displayBreathing()
{
    showBreathing(breathingPhase, breathingVisualStep, breathingCurrentRound,
                  breathingTotalRounds, getBreathingPhaseLabel());
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

    updateBreathingPhase(breathingPhase, breathingVisualStep, breathingCurrentRound,
                         breathingTotalRounds, getBreathingPhaseLabel());
    ++breathingPartialRefreshCount;
}

void resetBreathing()
{
    breathingPhase = 0;
    breathingCurrentRound = 1;
    breathingVisualStep = 0;
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

    const unsigned long elapsedMs = millis() - breathingPhaseStartedAt;
    unsigned long elapsedPhases = elapsedMs / BREATHING_PHASE_DURATION_MS;
    if (elapsedPhases == 0)
    {
        const uint8_t visualStep = elapsedMs / BREATHING_ANIMATION_STEP_MS;
        if (visualStep != breathingVisualStep)
        {
            breathingVisualStep = visualStep;
            refreshBreathingPhase();
        }
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

    breathingVisualStep = 0;

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
        quoteScreensaverMode = false;
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
    quoteScreensaverMode = false;
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
    if (dailyMorningEnabled() && dailyScheduleMagic != DAILY_SCHEDULE_MAGIC)
    {
        // A complete power loss clears the relative clock anchor; set it again in Settings.
        setDailyMorningEnabled(false);
    }
    const uint8_t activeQuoteCount = quoteCount(getQuoteDisplaySettings().category);
    const esp_sleep_wakeup_cause_t wakeCause = esp_sleep_get_wakeup_cause();
    const bool wokeFromQuoteSleep = retainedQuoteMagic == RETAINED_QUOTE_MAGIC &&
        (wakeCause == ESP_SLEEP_WAKEUP_EXT0 || wakeCause == ESP_SLEEP_WAKEUP_TIMER);
    const uint8_t restoredQuote = wokeFromQuoteSleep ? retainedQuote : savedQuote();
    currentQuote = restoredQuote < activeQuoteCount ? restoredQuote : 0;
    selectedApp = savedLauncherSelection() < NUM_LAUNCHER_APPS ? savedLauncherSelection() : 0;
    const uint8_t previousScreen = savedScreen();
    currentScreen = previousScreen <= SETTINGS ? static_cast<Screen>(previousScreen) : MENU;

    if (wokeFromQuoteSleep && wakeCause == ESP_SLEEP_WAKEUP_EXT0)
    {
        retainedQuoteMagic = 0;
        quoteScreensaverMode = false;
        currentScreen = MENU;
        displayMenu();
        saveCurrentView();
        lastUserActivityAt = millis();
        return;
    }

    if (wokeFromQuoteSleep && wakeCause == ESP_SLEEP_WAKEUP_TIMER)
    {
        currentScreen = QUOTE_DISPLAY;
        quoteScreensaverMode = retainedQuoteScreensaverMode;
        const bool dailyWake = retainedTimerWakeReason == TIMER_WAKE_DAILY_QUOTE;
        if (!dailyWake || advanceDailyQuoteIfDue(true) == 0)
        {
            advanceQuoteForward();
        }
        retainedTimerWakeReason = TIMER_WAKE_NONE;
        retainedQuote = currentQuote;
        displayCurrentQuote(false);
        lastUserActivityAt = millis();
        return;
    }

    retainedQuoteMagic = 0;
    quoteScreensaverMode = false;

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
    lastUserActivityAt = millis();
}

void updateUI()
{
    serviceSettingsPersistence();
    serviceDailyMorningSchedule();
    const InputEvent event = readInputEvent();

    if (event != INPUT_NONE)
    {
        lastUserActivityAt = millis();
    }

    if (event == INPUT_NONE && showQuoteScreensaverWhenIdle())
    {
        return;
    }

    if (currentScreen == QUOTE_DISPLAY && event != INPUT_NONE)
    {
        quoteScreensaverMode = false;
        quoteLastActivityAt = millis();
    }

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
            advanceQuoteForward();
            displayCurrentQuote();
        }
        else if (event == INPUT_PREVIOUS)
        {
            advanceQuoteBackward();
            displayCurrentQuote();
        }
        else if (event == INPUT_PRESS)
        {
            currentScreen = QUOTE_SETTINGS;
            saveCurrentView();
            displayQuoteSettings();
        }
        sleepQuoteDisplayWhenIdle();
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
                cycleQuoteCategory();
                currentQuote = 0;
                saveCurrentView();
            }
            else if (selectedQuoteSetting == 1)
            {
                cycleQuoteChangeInterval();
            }
            else if (selectedQuoteSetting == 2)
            {
                cycleQuoteTheme();
            }
            else
            {
                resetQuoteDisplaySettings();
                currentQuote = 0;
                saveCurrentView();
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
                breathingVisualStep = 0;
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
                toggleNavigationHints();
            }
            else if (selectedSetting == 3)
            {
                cycleScreensaverTimeout();
            }
            else if (selectedSetting == 4)
            {
                cycleDeepSleepTimeout();
            }
            else if (selectedSetting == 5)
            {
                toggleDailyMorningSchedule();
            }
            else if (selectedSetting == 6)
            {
                togglePartialRefresh();
            }
            else if (selectedSetting == 7)
            {
                cycleRefreshStyle();
            }
            else
            {
                resetSettings();
                dailyScheduleMagic = 0;
                nextDailyQuoteAt = 0;
            }
            if (selectedSetting == 0 || selectedSetting == 1 || selectedSetting == 2 ||
                selectedSetting == 8)
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
