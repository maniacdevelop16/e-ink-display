#pragma once

#include <Arduino.h>
#include "Quotes.h"

void initializeDisplay();
void showQuote(uint8_t index);
void showMainMenu(uint8_t selectedApp);
void showPomodoro(uint16_t remainingSeconds, bool running, bool complete);
void showDemo(uint8_t appNumber);
