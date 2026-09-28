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

struct GlobalSettings
{
    FontChoice fontChoice;
    RefreshStyle refreshStyle;
    bool partialRefreshEnabled;
    bool showNavigationHints;
};

void initializeSettings();
const GlobalSettings& getSettings();
void cycleFontChoice();
void cycleRefreshStyle();
void togglePartialRefresh();
void toggleNavigationHints();
void resetSettings();

const char* fontChoiceLabel();
const char* refreshStyleLabel();
uint16_t pomodoroRefreshIntervalSeconds();
uint8_t partialRefreshLimit(uint8_t defaultLimit);
