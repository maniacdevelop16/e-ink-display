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
// These mappings intentionally swap the physical UP and DOWN buttons.
constexpr uint8_t ROTARY_UP = 4;
constexpr uint8_t ROTARY_DOWN = 6;
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

// Shared UI layout
constexpr int HEADER_HEIGHT = 48;
constexpr int MENU_LEFT_MARGIN = 22;
constexpr int MENU_ITEM_HEIGHT = 32;

// Pomodoro behavior. The periodic refresh is intentionally conservative for e-paper.
constexpr uint16_t POMODORO_DURATION_SECONDS = 25 * 60;
constexpr uint16_t POMODORO_EXTENSION_SECONDS = 5 * 60;
constexpr uint16_t POMODORO_MAX_SECONDS = 90 * 60;
constexpr uint16_t POMODORO_REFRESH_INTERVAL_SECONDS = 30;
constexpr uint8_t POMODORO_PARTIALS_BEFORE_FULL_REFRESH = 8;
constexpr int POMODORO_DYNAMIC_Y = 72;
constexpr int POMODORO_DYNAMIC_HEIGHT = 132;
constexpr int POMODORO_PROGRESS_X = 45;
constexpr int POMODORO_PROGRESS_Y = 184;
constexpr int POMODORO_PROGRESS_WIDTH = 310;
constexpr int POMODORO_PROGRESS_HEIGHT = 14;

constexpr unsigned long DEBOUNCE_TIME_MS = 80;
constexpr unsigned long LONG_PRESS_TIME_MS = 800;
