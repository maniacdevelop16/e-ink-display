#include "Settings.h"

#include "Config.h"

namespace
{
GlobalSettings settings = { FONT_MONO_COMFORT, REFRESH_GENTLE, true, true };
QuoteDisplaySettings quoteSettings = { 0, QUOTE_THEME_QUIET };
}

void initializeSettings()
{
    resetSettings();
    resetQuoteDisplaySettings();
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

const QuoteDisplaySettings& getQuoteDisplaySettings()
{
    return quoteSettings;
}

void cycleQuoteChangeInterval()
{
    const uint32_t intervals[] = { 0, 60, 180, 300, 600, 1800, 3600 };
    uint8_t index = 0;
    while (index < 7 && intervals[index] != quoteSettings.autoChangeIntervalSeconds)
    {
        ++index;
    }
    if (index == 7)
    {
        index = 0;
    }
    quoteSettings.autoChangeIntervalSeconds = intervals[(index + 1) % 7];
}

void cycleQuoteTheme()
{
    quoteSettings.theme = static_cast<QuoteTheme>((quoteSettings.theme + 1) % 3);
}

void resetQuoteDisplaySettings()
{
    quoteSettings = { 0, QUOTE_THEME_QUIET };
}

const char* quoteChangeIntervalLabel()
{
    const char* labels[] = { "OFF", "1 MIN", "3 MIN", "5 MIN", "10 MIN", "30 MIN", "1 HOUR" };
    const uint32_t intervals[] = { 0, 60, 180, 300, 600, 1800, 3600 };
    for (uint8_t index = 0; index < 7; ++index)
    {
        if (intervals[index] == quoteSettings.autoChangeIntervalSeconds)
        {
            return labels[index];
        }
    }
    return "OFF";
}

const char* quoteThemeLabel()
{
    const char* labels[] = { "QUIET", "MINIMAL", "FOCUS" };
    return labels[quoteSettings.theme];
}
