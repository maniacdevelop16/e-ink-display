#include "Settings.h"

#include "Config.h"

namespace
{
GlobalSettings settings = { FONT_MONO_COMFORT, REFRESH_GENTLE, true, true };
}

void initializeSettings()
{
    resetSettings();
}

const GlobalSettings& getSettings()
{
    return settings;
}

void cycleFontChoice()
{
    settings.fontChoice = static_cast<FontChoice>((settings.fontChoice + 1) % 5);
}

void cycleRefreshStyle()
{
    settings.refreshStyle = settings.refreshStyle == REFRESH_GENTLE ? REFRESH_RESPONSIVE : REFRESH_GENTLE;
}

void togglePartialRefresh()
{
    settings.partialRefreshEnabled = !settings.partialRefreshEnabled;
}

void toggleNavigationHints()
{
    settings.showNavigationHints = !settings.showNavigationHints;
}

void resetSettings()
{
    settings = { FONT_MONO_COMFORT, REFRESH_GENTLE, true, true };
}

const char* fontChoiceLabel()
{
    const char* labels[] = {
        "MONO COMPACT",
        "MONO COMFORT",
        "SANS COMPACT",
        "SANS COMFORT",
        "SERIF READER"
    };
    return labels[settings.fontChoice];
}

const char* refreshStyleLabel()
{
    return settings.refreshStyle == REFRESH_GENTLE ? "GENTLE" : "RESPONSIVE";
}

bool usePartialRefresh()
{
    return settings.partialRefreshEnabled;
}

uint16_t pomodoroRefreshIntervalSeconds()
{
    return settings.refreshStyle == REFRESH_GENTLE ? POMODORO_REFRESH_INTERVAL_SECONDS : 10;
}

uint8_t partialRefreshLimit(uint8_t defaultLimit)
{
    return settings.refreshStyle == REFRESH_GENTLE ? defaultLimit : defaultLimit * 2;
}
