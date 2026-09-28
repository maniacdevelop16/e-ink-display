#pragma once

#include <Arduino.h>
#include "Quotes.h"

void initializeDisplay();
void showQuote(uint8_t index);
void showMainMenu(uint8_t selectedApp);
void updateMainMenuSelection(uint8_t selectedApp);
void showSettings(uint8_t selectedSetting);
void showPomodoro(uint16_t remainingSeconds, uint16_t totalSeconds, const char* status, const char* action);
void updatePomodoroTimer(uint16_t remainingSeconds, uint16_t totalSeconds);
void showBreathing(uint8_t phase, uint8_t currentRound, uint8_t totalRounds,
                   const char* phaseLabel, const char* status, const char* action);
void updateBreathingPhase(uint8_t phase, uint8_t currentRound, uint8_t totalRounds,
                          const char* phaseLabel, const char* status);
