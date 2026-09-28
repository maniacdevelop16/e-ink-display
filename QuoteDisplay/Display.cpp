#include "Display.h"

#include <SPI.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSerif18pt7b.h>

#include "Config.h"
#include "Settings.h"

namespace
{
GxEPD2_BW<GxEPD2_420_GYE042A87, GxEPD2_420_GYE042A87::HEIGHT> display(
    GxEPD2_420_GYE042A87(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

void drawCenteredText(const char* text, int y)
{
    if (text == nullptr || text[0] == '\0')
    {
        return;
    }

    int16_t x1;
    int16_t y1;
    uint16_t width;
    uint16_t height;
    display.getTextBounds(text, 0, y, &x1, &y1, &width, &height);
    display.setCursor((DISPLAY_WIDTH - width) / 2, y);
    display.println(text);
}

void setReadingFont()
{
    switch (getSettings().fontChoice)
    {
        case FONT_MONO_COMPACT:
            display.setFont(&FreeMonoBold9pt7b);
            break;
        case FONT_MONO_COMFORT:
            display.setFont(&FreeMonoBold12pt7b);
            break;
        case FONT_SANS_COMPACT:
            display.setFont(&FreeSans9pt7b);
            break;
        case FONT_SANS_COMFORT:
            display.setFont(&FreeSans12pt7b);
            break;
        case FONT_SERIF_READER:
            display.setFont(&FreeSerif12pt7b);
            break;
    }
}

void setPrimaryFont()
{
    switch (getSettings().fontChoice)
    {
        case FONT_MONO_COMPACT:
            display.setFont(&FreeMonoBold12pt7b);
            break;
        case FONT_MONO_COMFORT:
            display.setFont(&FreeMonoBold18pt7b);
            break;
        case FONT_SANS_COMPACT:
            display.setFont(&FreeSans12pt7b);
            break;
        case FONT_SANS_COMFORT:
            display.setFont(&FreeSans18pt7b);
            break;
        case FONT_SERIF_READER:
            display.setFont(&FreeSerif18pt7b);
            break;
    }
}

void drawMenuItem(const char* label, int y, bool selected)
{
    int16_t x1;
    int16_t y1;
    uint16_t width;
    uint16_t height;
    display.getTextBounds(label, 0, y, &x1, &y1, &width, &height);
    const int x = (DISPLAY_WIDTH - width) / 2;
    if (selected)
    {
        display.fillCircle(x - 18, y - (height / 2), 4, GxEPD_BLACK);
    }

    display.setCursor(x, y);
    display.println(label);
}

void drawMenuContent(uint8_t selectedApp)
{
    const char* apps[] = { "QUOTE DISPLAY", "POMODORO", "BOX BREATHING", "GLOBAL SETTINGS" };
    const char* descriptions[] = {
        "Browse daily quotes",
        "Focus timer: 25 minutes",
        "Guided 4-4-4-4 breathing",
        "Fonts, refresh, and hint options"
    };

    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    drawMenuItem(apps[0], 108, selectedApp == 0);
    drawMenuItem(apps[1], 138, selectedApp == 1);
    drawMenuItem(apps[2], 168, selectedApp == 2);
    drawMenuItem(apps[3], 198, selectedApp == 3);
    drawCenteredText(descriptions[selectedApp], 230);
}

void drawPomodoroTimer(uint16_t remainingSeconds, uint16_t totalSeconds, const char* status)
{
    const uint16_t minutes = remainingSeconds / 60;
    const uint8_t seconds = remainingSeconds % 60;
    char timeText[10];
    snprintf(timeText, sizeof(timeText), "%02u:%02u", static_cast<unsigned int>(minutes), static_cast<unsigned int>(seconds));

    setPrimaryFont();
    drawCenteredText(timeText, 128);
    setReadingFont();
    drawCenteredText(status, 158);

    display.drawRect(POMODORO_PROGRESS_X, POMODORO_PROGRESS_Y,
                     POMODORO_PROGRESS_WIDTH, POMODORO_PROGRESS_HEIGHT, GxEPD_BLACK);
    const uint16_t elapsedSeconds = totalSeconds > remainingSeconds ? totalSeconds - remainingSeconds : 0;
    const int fillWidth = totalSeconds == 0
        ? 0
        : (static_cast<uint32_t>(POMODORO_PROGRESS_WIDTH - 2) * elapsedSeconds) / totalSeconds;
    if (fillWidth > 0)
    {
        display.fillRect(POMODORO_PROGRESS_X + 1, POMODORO_PROGRESS_Y + 1,
                         fillWidth, POMODORO_PROGRESS_HEIGHT - 2, GxEPD_BLACK);
    }

    char progressText[22];
    const uint16_t progressPercent = totalSeconds == 0 ? 0 : (elapsedSeconds * 100UL) / totalSeconds;
    snprintf(progressText, sizeof(progressText), "%u%% COMPLETE", static_cast<unsigned int>(progressPercent));
    drawCenteredText(progressText, 216);
}

void drawBreathingSquare(uint8_t phase)
{
    const int sizes[] = { 36, 54, 78, 54 };
    const int size = sizes[phase];
    const int x = (DISPLAY_WIDTH - size) / 2;
    const int y = 158 - (size / 2);

    display.drawRect(x, y, size, size, GxEPD_BLACK);
    if (phase == 0)
    {
        display.fillRect((DISPLAY_WIDTH - 14) / 2, 151, 14, 14, GxEPD_BLACK);
    }
}

void drawBreathingContent(uint8_t phase, uint8_t currentRound, uint8_t totalRounds,
                          const char* phaseLabel, const char* status)
{
    char roundText[20];
    snprintf(roundText, sizeof(roundText), "ROUND %u OF %u",
             static_cast<unsigned int>(currentRound), static_cast<unsigned int>(totalRounds));

    setPrimaryFont();
    drawCenteredText(phaseLabel, 86);
    setReadingFont();
    drawCenteredText(status, 108);
    drawBreathingSquare(phase);
    drawCenteredText(roundText, 214);

    constexpr int STEP_SIZE = 12;
    constexpr int STEP_GAP = 18;
    constexpr int FIRST_STEP_X = (DISPLAY_WIDTH - (BREATHING_PHASE_COUNT * STEP_SIZE +
        (BREATHING_PHASE_COUNT - 1) * STEP_GAP)) / 2;
    for (uint8_t step = 0; step < BREATHING_PHASE_COUNT; ++step)
    {
        const int x = FIRST_STEP_X + step * (STEP_SIZE + STEP_GAP);
        if (step == phase)
        {
            display.fillRect(x, 222, STEP_SIZE, STEP_SIZE, GxEPD_BLACK);
        }
        else
        {
            display.drawRect(x, 222, STEP_SIZE, STEP_SIZE, GxEPD_BLACK);
        }
    }
}
}

void initializeDisplay()
{
    pinMode(EPD_POWER, OUTPUT);
    digitalWrite(EPD_POWER, HIGH);
    delay(1000);

    SPI.begin(EPD_SCK, -1, EPD_MOSI, EPD_CS);
    delay(500);
    display.init(115200);
}

void showQuote(uint8_t index)
{
    const Quote& quote = getQuote(index);

    display.setRotation(0);
    display.setFullWindow();
    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);

        display.setFont(&FreeMonoBold9pt7b);
        drawCenteredText("A QUIET THOUGHT", 36);
        display.drawLine(82, 56, DISPLAY_WIDTH - 82, 56, GxEPD_BLACK);

        setReadingFont();
        display.drawLine(58, 84, 58, 186, GxEPD_BLACK);
        display.fillCircle(58, 84, 3, GxEPD_BLACK);

        drawCenteredText(quote.line1, QUOTE_Y1);
        drawCenteredText(quote.line2, QUOTE_Y2);
        drawCenteredText(quote.line3, QUOTE_Y3);

        char authorText[50];
        snprintf(authorText, sizeof(authorText), "- %s", quote.author);
        display.setFont(&FreeMonoBold9pt7b);
        drawCenteredText(authorText, 214);
        display.drawLine(104, 234, DISPLAY_WIDTH - 104, 234, GxEPD_BLACK);

        char counter[22];
        snprintf(counter, sizeof(counter), "THOUGHT %u OF %u",
                 static_cast<unsigned int>(index + 1), static_cast<unsigned int>(NUM_QUOTES));
        drawCenteredText(counter, 258);
        if (getSettings().showNavigationHints)
        {
            drawCenteredText("UP / DOWN  TO BROWSE", 279);
            drawCenteredText("PRESS  FOR MENU", 297);
        }
    } while (display.nextPage());

    Serial.print("Quote ");
    Serial.print(index + 1);
    Serial.print("/");
    Serial.print(NUM_QUOTES);
    Serial.print(": ");
    Serial.println(quote.line1);
}

void showMainMenu(uint8_t selectedApp)
{
    display.setRotation(0);
    display.setFullWindow();
    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        setReadingFont();
        drawCenteredText("YOUR QUIET SPACE", 50);
        display.drawLine(80, 72, DISPLAY_WIDTH - 80, 72, GxEPD_BLACK);
        drawMenuContent(selectedApp);
        if (getSettings().showNavigationHints)
        {
            display.setFont(&FreeMonoBold9pt7b);
            drawCenteredText("UP / DOWN  TO EXPLORE", 266);
            drawCenteredText("PRESS  TO BEGIN", 288);
        }
    } while (display.nextPage());
}

void updateMainMenuSelection(uint8_t selectedApp)
{
    display.setRotation(0);
    display.setPartialWindow(0, MENU_DYNAMIC_Y, DISPLAY_WIDTH, MENU_DYNAMIC_HEIGHT);
    display.firstPage();

    do
    {
        display.fillRect(0, MENU_DYNAMIC_Y, DISPLAY_WIDTH, MENU_DYNAMIC_HEIGHT, GxEPD_WHITE);
        drawMenuContent(selectedApp);
    } while (display.nextPage());

    display.setFullWindow();
}

void showSettings(uint8_t selectedSetting)
{
    char fontRow[32];
    char partialRow[32];
    char refreshRow[32];
    char hintsRow[32];
    snprintf(fontRow, sizeof(fontRow), "FONT: %s", fontChoiceLabel());
    snprintf(partialRow, sizeof(partialRow), "PARTIAL REFRESH: %s",
             getSettings().partialRefreshEnabled ? "ON" : "OFF");
    snprintf(refreshRow, sizeof(refreshRow), "REFRESH: %s", refreshStyleLabel());
    snprintf(hintsRow, sizeof(hintsRow), "NAV HINTS: %s",
             getSettings().showNavigationHints ? "SHOW" : "HIDE");
    const char* rows[] = { fontRow, partialRow, refreshRow, hintsRow, "RESET DEFAULTS" };

    display.setRotation(0);
    display.setFullWindow();
    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        setReadingFont();
        drawCenteredText("GLOBAL SETTINGS", 44);
        display.setFont(&FreeMonoBold9pt7b);
        drawCenteredText("APPLIES TO EVERY APP", 70);
        display.drawLine(84, 84, DISPLAY_WIDTH - 84, 84, GxEPD_BLACK);
        drawMenuItem(rows[0], 106, selectedSetting == 0);
        drawMenuItem(rows[1], 130, selectedSetting == 1);
        drawMenuItem(rows[2], 154, selectedSetting == 2);
        drawMenuItem(rows[3], 178, selectedSetting == 3);
        drawMenuItem(rows[4], 202, selectedSetting == 4);
        drawCenteredText("UP / DOWN  SELECT", 250);
        drawCenteredText("PRESS  CHANGE", 274);
        drawCenteredText("HOLD  MENU", 296);
    } while (display.nextPage());
}

void showPomodoro(uint16_t remainingSeconds, uint16_t totalSeconds, const char* status, const char* action)
{
    display.setRotation(0);
    display.setFullWindow();
    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        setReadingFont();
        display.fillRect(0, 0, DISPLAY_WIDTH, HEADER_HEIGHT, GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);
        drawCenteredText("POMODORO", 34);
        display.setTextColor(GxEPD_BLACK);
        drawPomodoroTimer(remainingSeconds, totalSeconds, status);
        if (getSettings().showNavigationHints)
        {
            drawCenteredText(action, 232);
            drawCenteredText("UP: RESET    DOWN: +5 MIN", 260);
            drawCenteredText("HOLD: MENU", 286);
        }
    } while (display.nextPage());
}

void updatePomodoroTimer(uint16_t remainingSeconds, uint16_t totalSeconds)
{
    display.setRotation(0);
    display.setPartialWindow(0, POMODORO_DYNAMIC_Y, DISPLAY_WIDTH, POMODORO_DYNAMIC_HEIGHT);
    display.firstPage();

    do
    {
        display.fillRect(0, POMODORO_DYNAMIC_Y, DISPLAY_WIDTH, POMODORO_DYNAMIC_HEIGHT, GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        setReadingFont();
        drawPomodoroTimer(remainingSeconds, totalSeconds, "FOCUS IN PROGRESS");
    } while (display.nextPage());

    display.setFullWindow();
}

void showBreathing(uint8_t phase, uint8_t currentRound, uint8_t totalRounds,
                   const char* phaseLabel, const char* status, const char* action)
{
    display.setRotation(0);
    display.setFullWindow();
    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        setReadingFont();
        display.fillRect(0, 0, DISPLAY_WIDTH, HEADER_HEIGHT, GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);
        drawCenteredText("BOX BREATHING", 34);
        display.setTextColor(GxEPD_BLACK);
        drawBreathingContent(phase, currentRound, totalRounds, phaseLabel, status);
        if (getSettings().showNavigationHints)
        {
            drawCenteredText(action, 254);
            drawCenteredText("UP: RESET    DOWN: ROUNDS", 275);
            drawCenteredText("HOLD: MENU", 296);
        }
    } while (display.nextPage());
}

void updateBreathingPhase(uint8_t phase, uint8_t currentRound, uint8_t totalRounds,
                          const char* phaseLabel, const char* status)
{
    display.setRotation(0);
    display.setPartialWindow(0, BREATHING_DYNAMIC_Y, DISPLAY_WIDTH, BREATHING_DYNAMIC_HEIGHT);
    display.firstPage();

    do
    {
        display.fillRect(0, BREATHING_DYNAMIC_Y, DISPLAY_WIDTH, BREATHING_DYNAMIC_HEIGHT, GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        setReadingFont();
        drawBreathingContent(phase, currentRound, totalRounds, phaseLabel, status);
    } while (display.nextPage());

    display.setFullWindow();
}
