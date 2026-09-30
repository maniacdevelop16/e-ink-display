#pragma once

#include <Arduino.h>

enum FontChoice
{
    FONT_MONO_COMPACT,
    FONT_MONO_COMFORT,
    FONT_SANS_COMPACT,
    FONT_SANS_COMFORT,
    FONT_SERIF_READER
};

enum RefreshStyle
{
    REFRESH_GENTLE,
    REFRESH_RESPONSIVE
};

enum FontSize
{
    FONT_SIZE_SMALL,
    FONT_SIZE_MEDIUM,
    FONT_SIZE_LARGE
};

struct GlobalSettings
{
    FontChoice fontChoice;
    FontSize fontSize;
    RefreshStyle refreshStyle;
    bool partialRefreshEnabled;
};

enum QuoteTheme
{
    QUOTE_THEME_QUIET,
    QUOTE_THEME_MINIMAL,
    QUOTE_THEME_FOCUS
};

struct QuoteDisplaySettings
{
    uint32_t autoChangeIntervalSeconds;
    QuoteTheme theme;
};

void initializeSettings();
const GlobalSettings& getSettings();
void cycleFontChoice();
void cycleFontSize();
void cycleRefreshStyle();
void togglePartialRefresh();
void resetSettings();

const char* fontChoiceLabel();
const char* fontSizeLabel();
const char* refreshStyleLabel();
// Future pages should use these helpers instead of local display preferences.
bool usePartialRefresh();
uint16_t pomodoroRefreshIntervalSeconds();
uint8_t partialRefreshLimit(uint8_t defaultLimit);

const QuoteDisplaySettings& getQuoteDisplaySettings();
void cycleQuoteChangeInterval();
void cycleQuoteTheme();
void resetQuoteDisplaySettings();
const char* quoteChangeIntervalLabel();
const char* quoteThemeLabel();

// Small device state is stored in the ESP32's built-in flash (NVS).
uint8_t savedScreen();
uint8_t savedQuote();
uint8_t savedLauncherSelection();
void saveResumeState(uint8_t screen, uint8_t quote, uint8_t launcherSelection);
