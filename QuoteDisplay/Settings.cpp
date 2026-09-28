#include "Settings.h"

#include "Config.h"

namespace
{
GlobalSettings settings = { TEXT_COMFORTABLE, REFRESH_GENTLE, true };
}

void initializeSettings()
{
    resetSettings();
}

const GlobalSettings& getSettings()
{
    return settings;
}

void cycleTextSize()
{
    settings.textSize = settings.textSize == TEXT_COMPACT ? TEXT_COMFORTABLE : TEXT_COMPACT;
}

void cycleRefreshStyle()
{
    settings.refreshStyle = settings.refreshStyle == REFRESH_GENTLE ? REFRESH_RESPONSIVE : REFRESH_GENTLE;
}

void toggleNavigationHints()
{
    settings.showNavigationHints = !settings.showNavigationHints;
}

void resetSettings()
{
    settings = { TEXT_COMFORTABLE, REFRESH_GENTLE, true };
}

const char* textSizeLabel()
{
    return settings.textSize == TEXT_COMPACT ? "COMPACT" : "COMFORT";
}

const char* refreshStyleLabel()
{
    return settings.refreshStyle == REFRESH_GENTLE ? "GENTLE" : "RESPONSIVE";
}

uint16_t pomodoroRefreshIntervalSeconds()
{
    return settings.refreshStyle == REFRESH_GENTLE ? POMODORO_REFRESH_INTERVAL_SECONDS : 10;
}

uint8_t partialRefreshLimit(uint8_t defaultLimit)
{
    return settings.refreshStyle == REFRESH_GENTLE ? defaultLimit : defaultLimit * 2;
}
