#pragma once

#include <Arduino.h>
#include "Quotes.h"

enum GlobalFontRole
{
    GLOBAL_FONT_SMALL,
    GLOBAL_FONT_BODY,
    GLOBAL_FONT_PRIMARY
};

// Use this for every new page so the Global Settings font choice always applies.
void applyGlobalFont(GlobalFontRole role);

void initializeDisplay();
void showQuote(uint8_t index);
void showQuoteSettings(uint8_t selectedSetting);
void updateQuoteSettingsSelection(uint8_t selectedSetting);
void showMainMenu(uint8_t selectedApp);
void updateMainMenuSelection(uint8_t selectedApp);
void showSettings(uint8_t selectedSetting);
void updateSettingsSelection(uint8_t selectedSetting);
void showPomodoro(uint16_t remainingSeconds, uint16_t totalSeconds, const char* status);
void updatePomodoroTimer(uint16_t remainingSeconds, uint16_t totalSeconds);
void showBreathing(uint8_t phase, uint8_t phaseStep, uint8_t currentRound,
                   uint8_t totalRounds, const char* phaseLabel);
void updateBreathingPhase(uint8_t phase, uint8_t phaseStep, uint8_t currentRound,
                          uint8_t totalRounds, const char* phaseLabel);
