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

// CrowPanel controls. Inputs use INPUT_PULLUP and are active LOW.
// These mappings intentionally swap the physical UP and DOWN buttons.
constexpr uint8_t ROTARY_UP = 4;
constexpr uint8_t ROTARY_DOWN = 6;
constexpr uint8_t ROTARY_PRESS = 5;
constexpr uint8_t MENU_BUTTON = 2;
constexpr uint8_t EXIT_BUTTON = 1;

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
constexpr uint8_t MENU_PARTIALS_BEFORE_FULL_REFRESH = 8;
constexpr int MENU_DYNAMIC_Y = 94;
constexpr int MENU_DYNAMIC_HEIGHT = 148;
constexpr uint8_t NUM_LAUNCHER_APPS = 4;
constexpr uint8_t NUM_SETTINGS_ITEMS = 8;
constexpr uint8_t NUM_QUOTE_SETTINGS_ITEMS = 5;
constexpr uint8_t SETTINGS_PARTIALS_BEFORE_FULL_REFRESH = 8;
constexpr int SETTINGS_DYNAMIC_Y = 92;
constexpr int SETTINGS_DYNAMIC_HEIGHT = 180;

// Pomodoro behavior. The periodic refresh is intentionally conservative for e-paper.
constexpr uint16_t POMODORO_DURATION_SECONDS = 25 * 60;
constexpr uint16_t POMODORO_EXTENSION_SECONDS = 5 * 60;
constexpr uint16_t POMODORO_MAX_SECONDS = 90 * 60;
constexpr uint16_t POMODORO_REFRESH_INTERVAL_SECONDS = 15;
constexpr uint16_t POMODORO_RESPONSIVE_REFRESH_INTERVAL_SECONDS = 5;
constexpr uint8_t POMODORO_PARTIALS_BEFORE_FULL_REFRESH = 12;
constexpr int POMODORO_DYNAMIC_Y = 55;
constexpr int POMODORO_DYNAMIC_HEIGHT = 220;

// Breathing-bloom behavior. One round is inhale, hold, exhale, rest.
constexpr uint8_t BREATHING_PHASE_COUNT = 4;
constexpr uint8_t BREATHING_DEFAULT_ROUNDS = 4;
constexpr uint8_t BREATHING_MAX_ROUNDS = 8;
constexpr unsigned long BREATHING_PHASE_DURATION_MS = 4000;
constexpr unsigned long BREATHING_ANIMATION_STEP_MS = 1000;
constexpr uint8_t BREATHING_PARTIALS_BEFORE_FULL_REFRESH = 24;
constexpr int BREATHING_DYNAMIC_Y = 55;
constexpr int BREATHING_DYNAMIC_HEIGHT = 245;

constexpr unsigned long DEBOUNCE_TIME_MS = 80;
constexpr unsigned long PERSISTENCE_SAVE_DELAY_MS = 3000;
constexpr uint32_t DAILY_QUOTE_INTERVAL_SECONDS = 24UL * 60UL * 60UL;
