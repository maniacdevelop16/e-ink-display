#pragma once

#include <Arduino.h>
#include "Quotes.h"

void initializeDisplay();
void showQuote(uint8_t index);
void showMainMenu(uint8_t selectedApp);
void showPomodoro(uint16_t remainingSeconds, uint16_t totalSeconds, const char* status, const char* action);
void updatePomodoroTimer(uint16_t remainingSeconds, uint16_t totalSeconds);
void showDemo(uint8_t appNumber);
