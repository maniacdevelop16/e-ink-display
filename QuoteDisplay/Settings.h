#pragma once

#include <Arduino.h>

enum TextSize
{
    TEXT_COMPACT,
    TEXT_COMFORTABLE
};

enum RefreshStyle
{
    REFRESH_GENTLE,
    REFRESH_RESPONSIVE
};

struct GlobalSettings
{
    TextSize textSize;
    RefreshStyle refreshStyle;
    bool showNavigationHints;
};

void initializeSettings();
const GlobalSettings& getSettings();
void cycleTextSize();
void cycleRefreshStyle();
void toggleNavigationHints();
void resetSettings();

const char* textSizeLabel();
const char* refreshStyleLabel();
uint16_t pomodoroRefreshIntervalSeconds();
uint8_t partialRefreshLimit(uint8_t defaultLimit);
