#include "Settings.h"

#include <Preferences.h>

#include "Config.h"

namespace
{
GlobalSettings settings = { FONT_MONO_COMFORT, FONT_SIZE_MEDIUM, REFRESH_GENTLE, true };
QuoteDisplaySettings quoteSettings = { 0, QUOTE_THEME_QUIET, QUOTE_CATEGORY_DEFAULT };
Preferences preferences;
bool preferencesReady = false;

void saveGlobalSettings()
{
    if (!preferencesReady)
    {
        return;
    }

    preferences.putUChar("font", settings.fontChoice);
    preferences.putUChar("fontSize", settings.fontSize);
    preferences.putUChar("refresh", settings.refreshStyle);
    preferences.putBool("partial", settings.partialRefreshEnabled);
}

void saveQuoteSettings()
{
    if (!preferencesReady)
    {
        return;
    }

    preferences.putULong("quoteTime", quoteSettings.autoChangeIntervalSeconds);
    preferences.putUChar("quoteTheme", quoteSettings.theme);
    preferences.putUChar("quoteCat", quoteSettings.category);
}
}

void initializeSettings()
{
    preferencesReady = preferences.begin("quietspace", false);
    if (!preferencesReady)
    {
        return;
    }

    const uint8_t savedFont = preferences.getUChar("font", FONT_MONO_COMFORT);
    const uint8_t savedFontSize = preferences.getUChar("fontSize", FONT_SIZE_MEDIUM);
    const uint8_t savedRefresh = preferences.getUChar("refresh", REFRESH_GENTLE);
    const uint8_t savedTheme = preferences.getUChar("quoteTheme", QUOTE_THEME_QUIET);
    const uint8_t savedCategory = preferences.getUChar("quoteCat", QUOTE_CATEGORY_DEFAULT);
    settings.fontChoice = static_cast<FontChoice>(savedFont < 5 ? savedFont : FONT_MONO_COMFORT);
    settings.fontSize = static_cast<FontSize>(savedFontSize < 3 ? savedFontSize : FONT_SIZE_MEDIUM);
    settings.refreshStyle = static_cast<RefreshStyle>(savedRefresh < 2 ? savedRefresh : REFRESH_GENTLE);
    settings.partialRefreshEnabled = preferences.getBool("partial", true);
    quoteSettings.autoChangeIntervalSeconds = preferences.getULong("quoteTime", 0);
    quoteSettings.theme = static_cast<QuoteTheme>(savedTheme < 3 ? savedTheme : QUOTE_THEME_QUIET);
    quoteSettings.category = static_cast<QuoteCategory>(
        savedCategory < QUOTE_CATEGORY_COUNT ? savedCategory : QUOTE_CATEGORY_DEFAULT);
}

const GlobalSettings& getSettings()
{
    return settings;
}

void cycleFontChoice()
{
    settings.fontChoice = static_cast<FontChoice>((settings.fontChoice + 1) % 5);
    saveGlobalSettings();
}

void cycleFontSize()
{
    settings.fontSize = static_cast<FontSize>((settings.fontSize + 1) % 3);
    saveGlobalSettings();
}

void cycleRefreshStyle()
{
    settings.refreshStyle = settings.refreshStyle == REFRESH_GENTLE ? REFRESH_RESPONSIVE : REFRESH_GENTLE;
    saveGlobalSettings();
}

void togglePartialRefresh()
{
    settings.partialRefreshEnabled = !settings.partialRefreshEnabled;
    saveGlobalSettings();
}

void resetSettings()
{
    settings = { FONT_MONO_COMFORT, FONT_SIZE_MEDIUM, REFRESH_GENTLE, true };
    saveGlobalSettings();
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

const char* fontSizeLabel()
{
    const char* labels[] = { "SMALL", "MEDIUM", "LARGE" };
    return labels[settings.fontSize];
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
    return settings.refreshStyle == REFRESH_GENTLE
        ? POMODORO_REFRESH_INTERVAL_SECONDS
        : POMODORO_RESPONSIVE_REFRESH_INTERVAL_SECONDS;
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
    saveQuoteSettings();
}

void cycleQuoteTheme()
{
    quoteSettings.theme = static_cast<QuoteTheme>((quoteSettings.theme + 1) % 3);
    saveQuoteSettings();
}

void cycleQuoteCategory()
{
    quoteSettings.category = static_cast<QuoteCategory>(
        (quoteSettings.category + 1) % QUOTE_CATEGORY_COUNT);
    saveQuoteSettings();
}

void resetQuoteDisplaySettings()
{
    quoteSettings = { 0, QUOTE_THEME_QUIET, QUOTE_CATEGORY_DEFAULT };
    saveQuoteSettings();
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

uint8_t savedScreen()
{
    return preferencesReady ? preferences.getUChar("screen", 0) : 0;
}

uint8_t savedQuote()
{
    return preferencesReady ? preferences.getUChar("quote", 0) : 0;
}

uint8_t savedLauncherSelection()
{
    return preferencesReady ? preferences.getUChar("launcher", 0) : 0;
}

void saveResumeState(uint8_t screen, uint8_t quote, uint8_t launcherSelection)
{
    if (!preferencesReady)
    {
        return;
    }

    preferences.putUChar("screen", screen);
    preferences.putUChar("quote", quote);
    preferences.putUChar("launcher", launcherSelection);
}
