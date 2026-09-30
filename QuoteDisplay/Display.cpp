#include "Display.h"

#include <SPI.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
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
    applyGlobalFont(GLOBAL_FONT_BODY);
}

void setPrimaryFont()
{
    applyGlobalFont(GLOBAL_FONT_PRIMARY);
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
        "Fonts and refresh options"
    };

    display.setTextColor(GxEPD_BLACK);
    applyGlobalFont(GLOBAL_FONT_SMALL);
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

void drawQuoteLines(const Quote& quote, int firstLineY, int lineSpacing)
{
    setReadingFont();
    drawCenteredText(quote.line1, firstLineY);
    drawCenteredText(quote.line2, firstLineY + lineSpacing);
    drawCenteredText(quote.line3, firstLineY + (lineSpacing * 2));
}

void drawQuietQuote(const Quote& quote, uint8_t index, uint8_t count, const char* category)
{
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    applyGlobalFont(GLOBAL_FONT_SMALL);
    drawCenteredText(category, 36);
    display.drawLine(82, 56, DISPLAY_WIDTH - 82, 56, GxEPD_BLACK);
    display.drawLine(58, 84, 58, 186, GxEPD_BLACK);
    display.fillCircle(58, 84, 3, GxEPD_BLACK);
    drawQuoteLines(quote, 118, 34);

    char authorText[50];
    snprintf(authorText, sizeof(authorText), "- %s", quote.author);
    applyGlobalFont(GLOBAL_FONT_SMALL);
    drawCenteredText(authorText, 230);
    display.drawLine(104, 246, DISPLAY_WIDTH - 104, 246, GxEPD_BLACK);

    char counter[22];
    snprintf(counter, sizeof(counter), "THOUGHT %u OF %u",
             static_cast<unsigned int>(index + 1), static_cast<unsigned int>(count));
    drawCenteredText(counter, 276);
}

void drawMinimalQuote(const Quote& quote, uint8_t index, uint8_t count, const char* category)
{
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    applyGlobalFont(GLOBAL_FONT_SMALL);
    drawCenteredText(category, 34);
    display.drawLine(120, 50, DISPLAY_WIDTH - 120, 50, GxEPD_BLACK);
    drawQuoteLines(quote, 116, 34);

    char authorText[50];
    snprintf(authorText, sizeof(authorText), "- %s", quote.author);
    applyGlobalFont(GLOBAL_FONT_SMALL);
    drawCenteredText(authorText, 232);

    char counter[22];
    snprintf(counter, sizeof(counter), "%u / %u", static_cast<unsigned int>(index + 1),
             static_cast<unsigned int>(count));
    drawCenteredText(counter, 276);
}

void drawFocusQuote(const Quote& quote, uint8_t index, uint8_t count, const char* category)
{
    display.fillScreen(GxEPD_WHITE);
    display.fillRect(0, 0, DISPLAY_WIDTH, 44, GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);
    applyGlobalFont(GLOBAL_FONT_SMALL);
    drawCenteredText(category, 30);

    display.setTextColor(GxEPD_BLACK);
    display.drawLine(70, 72, DISPLAY_WIDTH - 70, 72, GxEPD_BLACK);
    drawQuoteLines(quote, 116, 34);

    char authorText[50];
    snprintf(authorText, sizeof(authorText), "- %s", quote.author);
    applyGlobalFont(GLOBAL_FONT_SMALL);
    drawCenteredText(authorText, 230);
    display.drawLine(104, 246, DISPLAY_WIDTH - 104, 246, GxEPD_BLACK);

    char counter[22];
    snprintf(counter, sizeof(counter), "THOUGHT %u OF %u",
             static_cast<unsigned int>(index + 1), static_cast<unsigned int>(count));
    drawCenteredText(counter, 276);
}

void drawQuoteSettingsContent(uint8_t selectedSetting)
{
    char categoryRow[34];
    char timerRow[32];
    char themeRow[32];
    snprintf(categoryRow, sizeof(categoryRow), "CATEGORY: %s",
             quoteCategoryLabel(getQuoteDisplaySettings().category));
    snprintf(timerRow, sizeof(timerRow), "AUTO CHANGE: %s", quoteChangeIntervalLabel());
    snprintf(themeRow, sizeof(themeRow), "THEME: %s", quoteThemeLabel());
    const char* rows[] = { categoryRow, timerRow, themeRow, "RESET QUOTE SETTINGS" };

    display.setTextColor(GxEPD_BLACK);
    applyGlobalFont(GLOBAL_FONT_SMALL);
    drawMenuItem(rows[0], 108, selectedSetting == 0);
    drawMenuItem(rows[1], 138, selectedSetting == 1);
    drawMenuItem(rows[2], 168, selectedSetting == 2);
    drawMenuItem(rows[3], 198, selectedSetting == 3);
}

void drawSettingsContent(uint8_t selectedSetting)
{
    char fontRow[32];
    char sizeRow[32];
    char partialRow[32];
    char refreshRow[32];
    snprintf(fontRow, sizeof(fontRow), "FONT: %s", fontChoiceLabel());
    snprintf(sizeRow, sizeof(sizeRow), "FONT SIZE: %s", fontSizeLabel());
    snprintf(partialRow, sizeof(partialRow), "PARTIAL REFRESH: %s",
             getSettings().partialRefreshEnabled ? "ON" : "OFF");
    snprintf(refreshRow, sizeof(refreshRow), "REFRESH: %s", refreshStyleLabel());
    const char* rows[] = { fontRow, sizeRow, partialRow, refreshRow, "RESET DEFAULTS" };

    display.setTextColor(GxEPD_BLACK);
    applyGlobalFont(GLOBAL_FONT_SMALL);
    drawMenuItem(rows[0], 108, selectedSetting == 0);
    drawMenuItem(rows[1], 133, selectedSetting == 1);
    drawMenuItem(rows[2], 158, selectedSetting == 2);
    drawMenuItem(rows[3], 183, selectedSetting == 3);
    drawMenuItem(rows[4], 208, selectedSetting == 4);
}
}

void applyGlobalFont(GlobalFontRole role)
{
    const bool primary = role == GLOBAL_FONT_PRIMARY;
    const bool small = role == GLOBAL_FONT_SMALL;

    if (getSettings().fontSize == FONT_SIZE_SMALL)
    {
        switch (getSettings().fontChoice)
        {
            case FONT_MONO_COMPACT:
            case FONT_MONO_COMFORT:
                display.setFont(primary ? &FreeMonoBold12pt7b : &FreeMonoBold9pt7b);
                return;
            case FONT_SANS_COMPACT:
            case FONT_SANS_COMFORT:
                display.setFont(primary ? &FreeSans12pt7b : &FreeSans9pt7b);
                return;
            case FONT_SERIF_READER:
                display.setFont(primary ? &FreeSerif12pt7b : &FreeSerif9pt7b);
                return;
        }
    }

    if (getSettings().fontSize == FONT_SIZE_LARGE)
    {
        switch (getSettings().fontChoice)
        {
            case FONT_MONO_COMPACT:
            case FONT_MONO_COMFORT:
                display.setFont(primary || !small ? &FreeMonoBold18pt7b : &FreeMonoBold12pt7b);
                return;
            case FONT_SANS_COMPACT:
            case FONT_SANS_COMFORT:
                display.setFont(primary || !small ? &FreeSans18pt7b : &FreeSans12pt7b);
                return;
            case FONT_SERIF_READER:
                display.setFont(primary || !small ? &FreeSerif18pt7b : &FreeSerif12pt7b);
                return;
        }
    }

    switch (getSettings().fontChoice)
    {
        case FONT_MONO_COMPACT:
            display.setFont(primary ? &FreeMonoBold12pt7b : &FreeMonoBold9pt7b);
            break;
        case FONT_MONO_COMFORT:
            display.setFont(primary ? &FreeMonoBold18pt7b : &FreeMonoBold12pt7b);
            break;
        case FONT_SANS_COMPACT:
            display.setFont(primary ? &FreeSans12pt7b : &FreeSans9pt7b);
            break;
        case FONT_SANS_COMFORT:
            display.setFont(primary ? &FreeSans18pt7b : &FreeSans12pt7b);
            break;
        case FONT_SERIF_READER:
            display.setFont(primary ? &FreeSerif18pt7b : (small ? &FreeSerif9pt7b : &FreeSerif12pt7b));
            break;
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
    const QuoteCategory category = getQuoteDisplaySettings().category;
    const uint8_t count = quoteCount(category);
    const Quote& quote = getQuote(category, index);
    const char* categoryLabel = quoteCategoryLabel(category);

    display.setRotation(0);
    display.setFullWindow();
    display.firstPage();

    do
    {
        switch (getQuoteDisplaySettings().theme)
        {
            case QUOTE_THEME_QUIET:
                drawQuietQuote(quote, index, count, categoryLabel);
                break;
            case QUOTE_THEME_MINIMAL:
                drawMinimalQuote(quote, index, count, categoryLabel);
                break;
            case QUOTE_THEME_FOCUS:
                drawFocusQuote(quote, index, count, categoryLabel);
                break;
        }
    } while (display.nextPage());

    Serial.print("Quote ");
    Serial.print(index + 1);
    Serial.print("/");
    Serial.print(count);
    Serial.print(": ");
    Serial.println(quote.line1);
}

void showQuoteSettings(uint8_t selectedSetting)
{
    display.setRotation(0);
    display.setFullWindow();
    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        applyGlobalFont(GLOBAL_FONT_BODY);
        drawCenteredText("QUOTE SETTINGS", 44);
        applyGlobalFont(GLOBAL_FONT_SMALL);
        drawCenteredText("PERSONALIZE APP 1", 70);
        display.drawLine(84, 84, DISPLAY_WIDTH - 84, 84, GxEPD_BLACK);
        drawQuoteSettingsContent(selectedSetting);
    } while (display.nextPage());
}

void updateQuoteSettingsSelection(uint8_t selectedSetting)
{
    display.setRotation(0);
    display.setPartialWindow(0, SETTINGS_DYNAMIC_Y, DISPLAY_WIDTH, SETTINGS_DYNAMIC_HEIGHT);
    display.firstPage();

    do
    {
        display.fillRect(0, SETTINGS_DYNAMIC_Y, DISPLAY_WIDTH, SETTINGS_DYNAMIC_HEIGHT, GxEPD_WHITE);
        drawQuoteSettingsContent(selectedSetting);
    } while (display.nextPage());

    display.setFullWindow();
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
    display.setRotation(0);
    display.setFullWindow();
    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        setReadingFont();
        drawCenteredText("GLOBAL SETTINGS", 44);
        applyGlobalFont(GLOBAL_FONT_SMALL);
        drawCenteredText("APPLIES TO EVERY APP", 70);
        display.drawLine(84, 84, DISPLAY_WIDTH - 84, 84, GxEPD_BLACK);
        drawSettingsContent(selectedSetting);
    } while (display.nextPage());
}

void updateSettingsSelection(uint8_t selectedSetting)
{
    display.setRotation(0);
    display.setPartialWindow(0, SETTINGS_DYNAMIC_Y, DISPLAY_WIDTH, SETTINGS_DYNAMIC_HEIGHT);
    display.firstPage();

    do
    {
        display.fillRect(0, SETTINGS_DYNAMIC_Y, DISPLAY_WIDTH, SETTINGS_DYNAMIC_HEIGHT, GxEPD_WHITE);
        drawSettingsContent(selectedSetting);
    } while (display.nextPage());

    display.setFullWindow();
}

void showPomodoro(uint16_t remainingSeconds, uint16_t totalSeconds, const char* status)
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
                   const char* phaseLabel, const char* status)
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
