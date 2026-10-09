#include "Settings.h"

#include <Preferences.h>

#include "Config.h"

namespace
{
constexpr uint16_t SCREENSAVER_TIMEOUTS[] = { 0, 10, 20, 30, 60, 120 };
constexpr uint16_t DEEP_SLEEP_TIMEOUTS[] = { 0, 30, 60, 180, 300, 600 };
constexpr uint8_t TIMEOUT_OPTION_COUNT = 6;

GlobalSettings settings = {
    FONT_MONO_COMFORT, FONT_SIZE_MEDIUM, REFRESH_GENTLE, true, false, 20, 30
};
QuoteDisplaySettings quoteSettings = { 0, QUOTE_THEME_QUIET, QUOTE_CATEGORY_DEFAULT };
GlobalSettings persistedSettings = settings;
QuoteDisplaySettings persistedQuoteSettings = quoteSettings;
Preferences preferences;
bool preferencesReady = false;
bool globalSettingsDirty = false;
bool quoteSettingsDirty = false;
bool resumeStateDirty = false;
unsigned long persistenceChangedAt = 0;
uint8_t resumeScreen = 0;
uint8_t resumeQuote = 0;
uint8_t resumeLauncher = 0;
uint8_t persistedScreen = 0;
uint8_t persistedQuote = 0;
uint8_t persistedLauncher = 0;

void markPersistenceChanged()
{
    persistenceChangedAt = millis();
}

uint8_t timeoutIndex(const uint16_t* options, uint16_t value, uint8_t defaultIndex)
{
    for (uint8_t index = 0; index < TIMEOUT_OPTION_COUNT; ++index)
    {
        if (options[index] == value)
        {
            return index;
        }
    }
    return defaultIndex;
}

void saveGlobalSettings()
{
    globalSettingsDirty = true;
    markPersistenceChanged();
}

void saveQuoteSettings()
{
    quoteSettingsDirty = true;
    markPersistenceChanged();
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
    settings.navigationHintsEnabled = preferences.getBool("navHints", false);
    const uint16_t savedScreensaver = preferences.getUShort("screenTime", 20);
    const uint16_t savedDeepSleep = preferences.getUShort("sleepTime", 30);
    settings.screensaverTimeoutSeconds = SCREENSAVER_TIMEOUTS[
        timeoutIndex(SCREENSAVER_TIMEOUTS, savedScreensaver, 2)];
    settings.deepSleepTimeoutSeconds = DEEP_SLEEP_TIMEOUTS[
        timeoutIndex(DEEP_SLEEP_TIMEOUTS, savedDeepSleep, 1)];
    quoteSettings.autoChangeIntervalSeconds = preferences.getULong("quoteTime", 0);
    quoteSettings.theme = static_cast<QuoteTheme>(savedTheme < 3 ? savedTheme : QUOTE_THEME_QUIET);
    quoteSettings.category = static_cast<QuoteCategory>(
        savedCategory < QUOTE_CATEGORY_COUNT ? savedCategory : QUOTE_CATEGORY_DEFAULT);
    resumeScreen = preferences.getUChar("screen", 0);
    resumeQuote = preferences.getUChar("quote", 0);
    resumeLauncher = preferences.getUChar("launcher", 0);
    persistedSettings = settings;
    persistedQuoteSettings = quoteSettings;
    persistedScreen = resumeScreen;
    persistedQuote = resumeQuote;
    persistedLauncher = resumeLauncher;
}

void flushSettingsPersistence()
{
    if (!preferencesReady)
    {
        return;
    }

    if (globalSettingsDirty)
    {
        if (settings.fontChoice != persistedSettings.fontChoice)
        {
            preferences.putUChar("font", settings.fontChoice);
        }
        if (settings.fontSize != persistedSettings.fontSize)
        {
            preferences.putUChar("fontSize", settings.fontSize);
        }
        if (settings.refreshStyle != persistedSettings.refreshStyle)
        {
            preferences.putUChar("refresh", settings.refreshStyle);
        }
        if (settings.partialRefreshEnabled != persistedSettings.partialRefreshEnabled)
        {
            preferences.putBool("partial", settings.partialRefreshEnabled);
        }
        if (settings.navigationHintsEnabled != persistedSettings.navigationHintsEnabled)
        {
            preferences.putBool("navHints", settings.navigationHintsEnabled);
        }
        if (settings.screensaverTimeoutSeconds != persistedSettings.screensaverTimeoutSeconds)
        {
            preferences.putUShort("screenTime", settings.screensaverTimeoutSeconds);
        }
        if (settings.deepSleepTimeoutSeconds != persistedSettings.deepSleepTimeoutSeconds)
        {
            preferences.putUShort("sleepTime", settings.deepSleepTimeoutSeconds);
        }
        persistedSettings = settings;
        globalSettingsDirty = false;
    }

    if (quoteSettingsDirty)
    {
        if (quoteSettings.autoChangeIntervalSeconds !=
            persistedQuoteSettings.autoChangeIntervalSeconds)
        {
            preferences.putULong("quoteTime", quoteSettings.autoChangeIntervalSeconds);
        }
        if (quoteSettings.theme != persistedQuoteSettings.theme)
        {
            preferences.putUChar("quoteTheme", quoteSettings.theme);
        }
        if (quoteSettings.category != persistedQuoteSettings.category)
        {
            preferences.putUChar("quoteCat", quoteSettings.category);
        }
        persistedQuoteSettings = quoteSettings;
        quoteSettingsDirty = false;
    }

    if (resumeStateDirty)
    {
        if (resumeScreen != persistedScreen)
        {
            preferences.putUChar("screen", resumeScreen);
        }
        if (resumeQuote != persistedQuote)
        {
            preferences.putUChar("quote", resumeQuote);
        }
        if (resumeLauncher != persistedLauncher)
        {
            preferences.putUChar("launcher", resumeLauncher);
        }
        persistedScreen = resumeScreen;
        persistedQuote = resumeQuote;
        persistedLauncher = resumeLauncher;
        resumeStateDirty = false;
    }
}

void serviceSettingsPersistence()
{
    if ((!globalSettingsDirty && !quoteSettingsDirty && !resumeStateDirty) ||
        millis() - persistenceChangedAt < PERSISTENCE_SAVE_DELAY_MS)
    {
        return;
    }

    flushSettingsPersistence();
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

void toggleNavigationHints()
{
    settings.navigationHintsEnabled = !settings.navigationHintsEnabled;
    saveGlobalSettings();
}

void cycleScreensaverTimeout()
{
    const uint8_t index = timeoutIndex(
        SCREENSAVER_TIMEOUTS, settings.screensaverTimeoutSeconds, 2);
    settings.screensaverTimeoutSeconds =
        SCREENSAVER_TIMEOUTS[(index + 1) % TIMEOUT_OPTION_COUNT];
    saveGlobalSettings();
}

void cycleDeepSleepTimeout()
{
    const uint8_t index = timeoutIndex(
        DEEP_SLEEP_TIMEOUTS, settings.deepSleepTimeoutSeconds, 1);
    settings.deepSleepTimeoutSeconds =
        DEEP_SLEEP_TIMEOUTS[(index + 1) % TIMEOUT_OPTION_COUNT];
    saveGlobalSettings();
}

void resetSettings()
{
    settings = {
        FONT_MONO_COMFORT, FONT_SIZE_MEDIUM, REFRESH_GENTLE, true, false, 20, 30
    };
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

const char* screensaverTimeoutLabel()
{
    const char* labels[] = { "OFF", "10 SEC", "20 SEC", "30 SEC", "1 MIN", "2 MIN" };
    return labels[timeoutIndex(SCREENSAVER_TIMEOUTS, settings.screensaverTimeoutSeconds, 2)];
}

const char* deepSleepTimeoutLabel()
{
    const char* labels[] = { "OFF", "30 SEC", "1 MIN", "3 MIN", "5 MIN", "10 MIN" };
    return labels[timeoutIndex(DEEP_SLEEP_TIMEOUTS, settings.deepSleepTimeoutSeconds, 1)];
}

bool showNavigationHints()
{
    return settings.navigationHintsEnabled;
}

uint16_t screensaverTimeoutSeconds()
{
    return settings.screensaverTimeoutSeconds;
}

uint16_t deepSleepTimeoutSeconds()
{
    return settings.deepSleepTimeoutSeconds;
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

void cycleQuoteCategoryBackward()
{
    quoteSettings.category = static_cast<QuoteCategory>(
        (quoteSettings.category + QUOTE_CATEGORY_COUNT - 1) % QUOTE_CATEGORY_COUNT);
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
    return resumeScreen;
}

uint8_t savedQuote()
{
    return resumeQuote;
}

uint8_t savedLauncherSelection()
{
    return resumeLauncher;
}

void saveResumeState(uint8_t screen, uint8_t quote, uint8_t launcherSelection)
{
    if (screen == resumeScreen && quote == resumeQuote && launcherSelection == resumeLauncher)
    {
        return;
    }

    resumeScreen = screen;
    resumeQuote = quote;
    resumeLauncher = launcherSelection;
    resumeStateDirty = true;
    markPersistenceChanged();
}
