#pragma once

#include <Arduino.h>

// E-paper pins
constexpr uint8_t EPD_CS = 45;
constexpr uint8_t EPD_DC = 46;
constexpr uint8_t EPD_RST = 47;
constexpr uint8_t EPD_BUSY = 48;
constexpr uint8_t EPD_SCK = 12;
constexpr uint8_t EPD_MOSI = 11;
constexpr uint8_t EPD_POWER = 7;

// Rotary/button pins. Inputs use INPUT_PULLUP and are active LOW.
constexpr uint8_t ROTARY_UP = 6;
constexpr uint8_t ROTARY_DOWN = 4;
constexpr uint8_t ROTARY_PRESS = 5;

// Display layout
constexpr int DISPLAY_WIDTH = 400;
constexpr int DISPLAY_HEIGHT = 300;
constexpr int QUOTE_Y1 = 105;
constexpr int QUOTE_Y2 = 135;
constexpr int QUOTE_Y3 = 165;
constexpr int AUTHOR_Y = 215;
constexpr int COUNTER_RIGHT_MARGIN = 15;
constexpr int COUNTER_Y = 285;

constexpr unsigned long DEBOUNCE_TIME_MS = 80;
