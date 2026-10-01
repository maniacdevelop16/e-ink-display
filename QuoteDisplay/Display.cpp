#include "Display.h"

#include <math.h>
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
    const char* apps[] = { "QUOTE DISPLAY", "POMODORO", "BREATHING BLOOM", "GLOBAL SETTINGS" };
    const char* descriptions[] = {
        "Browse daily quotes",
        "Focus timer: 25 minutes",
        "A calm guided breathing rhythm",
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

void drawPomodoroProgressRing(uint16_t remainingSeconds, uint16_t totalSeconds)
{
    constexpr int CENTER_X = DISPLAY_WIDTH / 2;
    constexpr int CENTER_Y = 157;
    constexpr int OUTER_RADIUS = 96;
    constexpr int INNER_RADIUS = 79;
    constexpr int ARC_RADIUS = 88;
    constexpr int ARC_HALF_WIDTH = 8;
    constexpr float START_ANGLE = -1.5707963f;
    constexpr float DEGREES_TO_RADIANS = 0.0174532925f;
    const uint16_t elapsedSeconds = totalSeconds > remainingSeconds ? totalSeconds - remainingSeconds : 0;
    uint16_t progressDegrees = totalSeconds == 0
        ? 0
        : (static_cast<uint32_t>(elapsedSeconds) * 360UL) / totalSeconds;
    if (remainingSeconds == 0 && totalSeconds > 0)
    {
        progressDegrees = 360;
    }

    display.drawCircle(CENTER_X, CENTER_Y, OUTER_RADIUS, GxEPD_BLACK);
    display.drawCircle(CENTER_X, CENTER_Y, INNER_RADIUS, GxEPD_BLACK);

    if (progressDegrees > 0)
    {
        for (uint16_t degree = 0; degree <= progressDegrees; degree += 2)
        {
            const float angle = START_ANGLE + degree * DEGREES_TO_RADIANS;
            const int x = CENTER_X + static_cast<int>(cosf(angle) * ARC_RADIUS);
            const int y = CENTER_Y + static_cast<int>(sinf(angle) * ARC_RADIUS);
            display.fillCircle(x, y, ARC_HALF_WIDTH, GxEPD_BLACK);
        }

        // Re-cut the center and borders so the filled arc stays smooth and circular.
        display.fillCircle(CENTER_X, CENTER_Y, INNER_RADIUS - 1, GxEPD_WHITE);
        display.drawCircle(CENTER_X, CENTER_Y, INNER_RADIUS, GxEPD_BLACK);
        display.drawCircle(CENTER_X, CENTER_Y, OUTER_RADIUS, GxEPD_BLACK);
    }
}

void drawPomodoroTimer(uint16_t remainingSeconds, uint16_t totalSeconds, const char* status)
{
    const uint16_t minutes = remainingSeconds / 60;
    const uint8_t seconds = remainingSeconds % 60;
    char timeText[10];
    snprintf(timeText, sizeof(timeText), "%02u:%02u", static_cast<unsigned int>(minutes), static_cast<unsigned int>(seconds));

    drawPomodoroProgressRing(remainingSeconds, totalSeconds);

    setPrimaryFont();
    drawCenteredText(timeText, 153);
    applyGlobalFont(GLOBAL_FONT_SMALL);
    drawCenteredText(status, 188);
}

void drawBreathingBloom(uint8_t phase, uint8_t phaseStep)
{
    constexpr int CENTER_X = DISPLAY_WIDTH / 2;
    constexpr int CENTER_Y = 170;
    constexpr uint8_t PETAL_COUNT = 6;
    constexpr float FULL_CIRCLE_RADIANS = 6.28318530718f;
    const uint8_t step = phaseStep > 3 ? 3 : phaseStep;

    int petalDistance = 20;
    int petalRadius = 10;
    if (phase == 0)
    {
        petalDistance = 20 + step * 8;
        petalRadius = 10 + step;
    }
    else if (phase == 1)
    {
        petalDistance = 44;
        petalRadius = 13;
    }
    else if (phase == 2)
    {
        petalDistance = 44 - step * 8;
        petalRadius = 13 - step;
    }

    for (uint8_t petal = 0; petal < PETAL_COUNT; ++petal)
    {
        const float angle = -FULL_CIRCLE_RADIANS / 4.0f +
            FULL_CIRCLE_RADIANS * petal / PETAL_COUNT;
        const int x = CENTER_X + static_cast<int>(cosf(angle) * petalDistance);
        const int y = CENTER_Y + static_cast<int>(sinf(angle) * petalDistance);
        display.drawCircle(x, y, petalRadius, GxEPD_BLACK);
        display.fillCircle(x, y, 2, GxEPD_BLACK);
    }

    display.fillCircle(CENTER_X, CENTER_Y, 6, GxEPD_BLACK);
    if (phase == 1)
    {
        display.drawCircle(CENTER_X, CENTER_Y, 61 + (step % 2) * 2, GxEPD_BLACK);
    }
    else if (phase == 3)
    {
        display.drawCircle(CENTER_X, CENTER_Y, 28 + (step % 2) * 2, GxEPD_BLACK);
    }
}

void drawBreathingContent(uint8_t phase, uint8_t phaseStep, uint8_t currentRound,
                          uint8_t totalRounds, const char* phaseLabel)
{
    char roundText[12];
    snprintf(roundText, sizeof(roundText), "%u / %u",
             static_cast<unsigned int>(currentRound), static_cast<unsigned int>(totalRounds));

    setPrimaryFont();
    drawCenteredText(phaseLabel, 91);
    drawBreathingBloom(phase, phaseStep);
    applyGlobalFont(GLOBAL_FONT_SMALL);
    drawCenteredText(roundText, 278);
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
        drawPomodoroTimer(remainingSeconds, totalSeconds, "FOCUS");
    } while (display.nextPage());

    display.setFullWindow();
}

void showBreathing(uint8_t phase, uint8_t phaseStep, uint8_t currentRound,
                   uint8_t totalRounds, const char* phaseLabel)
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
        drawCenteredText("BREATHE", 34);
        display.setTextColor(GxEPD_BLACK);
        drawBreathingContent(phase, phaseStep, currentRound, totalRounds, phaseLabel);
    } while (display.nextPage());
}

void updateBreathingPhase(uint8_t phase, uint8_t phaseStep, uint8_t currentRound,
                          uint8_t totalRounds, const char* phaseLabel)
{
    display.setRotation(0);
    display.setPartialWindow(0, BREATHING_DYNAMIC_Y, DISPLAY_WIDTH, BREATHING_DYNAMIC_HEIGHT);
    display.firstPage();

    do
    {
        display.fillRect(0, BREATHING_DYNAMIC_Y, DISPLAY_WIDTH, BREATHING_DYNAMIC_HEIGHT, GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        setReadingFont();
        drawBreathingContent(phase, phaseStep, currentRound, totalRounds, phaseLabel);
    } while (display.nextPage());

    display.setFullWindow();
}
