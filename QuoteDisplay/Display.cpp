#include "Display.h"

#include <SPI.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>

#include "Config.h"

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
    const char* apps[] = { "QUOTE DISPLAY", "POMODORO", "BOX BREATHING" };
    const char* descriptions[] = { "Browse daily quotes", "Focus timer: 25 minutes", "Guided 4-4-4-4 breathing" };

    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    drawMenuItem(apps[0], 118, selectedApp == 0);
    drawMenuItem(apps[1], 156, selectedApp == 1);
    drawMenuItem(apps[2], 194, selectedApp == 2);
    drawCenteredText(descriptions[selectedApp], 230);
}

void drawPomodoroTimer(uint16_t remainingSeconds, uint16_t totalSeconds, const char* status)
{
    const uint16_t minutes = remainingSeconds / 60;
    const uint8_t seconds = remainingSeconds % 60;
    char timeText[10];
    snprintf(timeText, sizeof(timeText), "%02u:%02u", static_cast<unsigned int>(minutes), static_cast<unsigned int>(seconds));

    display.setFont(&FreeMonoBold18pt7b);
    drawCenteredText(timeText, 128);
    display.setFont(&FreeMonoBold12pt7b);
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

    display.setFont(&FreeMonoBold18pt7b);
    drawCenteredText(phaseLabel, 86);
    display.setFont(&FreeMonoBold12pt7b);
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
        display.setFont(&FreeMonoBold12pt7b);

        drawCenteredText("PRESS: MENU", 30);

        drawCenteredText(quote.line1, QUOTE_Y1);
        drawCenteredText(quote.line2, QUOTE_Y2);
        drawCenteredText(quote.line3, QUOTE_Y3);

        char authorText[50];
        snprintf(authorText, sizeof(authorText), "- %s", quote.author);
        drawCenteredText(authorText, AUTHOR_Y);

        char counter[10];
        snprintf(counter, sizeof(counter), "%d / %d", index + 1, NUM_QUOTES);

        int16_t x1;
        int16_t y1;
        uint16_t width;
        uint16_t height;
        display.getTextBounds(counter, 0, 0, &x1, &y1, &width, &height);
        display.setCursor(DISPLAY_WIDTH - width - COUNTER_RIGHT_MARGIN, COUNTER_Y);
        display.println(counter);
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
        display.setFont(&FreeMonoBold12pt7b);
        drawCenteredText("YOUR QUIET SPACE", 50);
        display.drawLine(80, 72, DISPLAY_WIDTH - 80, 72, GxEPD_BLACK);
        drawMenuContent(selectedApp);
        display.setFont(&FreeMonoBold9pt7b);
        drawCenteredText("UP / DOWN  TO EXPLORE", 266);
        drawCenteredText("PRESS  TO BEGIN", 288);
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

void showPomodoro(uint16_t remainingSeconds, uint16_t totalSeconds, const char* status, const char* action)
{
    display.setRotation(0);
    display.setFullWindow();
    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold12pt7b);
        display.fillRect(0, 0, DISPLAY_WIDTH, HEADER_HEIGHT, GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);
        drawCenteredText("POMODORO", 34);
        display.setTextColor(GxEPD_BLACK);
        drawPomodoroTimer(remainingSeconds, totalSeconds, status);
        drawCenteredText(action, 232);
        drawCenteredText("UP: RESET    DOWN: +5 MIN", 260);
        drawCenteredText("HOLD: MENU", 286);
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
        display.setFont(&FreeMonoBold12pt7b);
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
        display.setFont(&FreeMonoBold12pt7b);
        display.fillRect(0, 0, DISPLAY_WIDTH, HEADER_HEIGHT, GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);
        drawCenteredText("BOX BREATHING", 34);
        display.setTextColor(GxEPD_BLACK);
        drawBreathingContent(phase, currentRound, totalRounds, phaseLabel, status);
        drawCenteredText(action, 254);
        drawCenteredText("UP: RESET    DOWN: ROUNDS", 275);
        drawCenteredText("HOLD: MENU", 296);
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
        display.setFont(&FreeMonoBold12pt7b);
        drawBreathingContent(phase, currentRound, totalRounds, phaseLabel, status);
    } while (display.nextPage());

    display.setFullWindow();
}
