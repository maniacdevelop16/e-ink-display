#include "Display.h"

#include <SPI.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold12pt7b.h>

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
    constexpr int MENU_WIDTH = DISPLAY_WIDTH - (MENU_LEFT_MARGIN * 2);
    const int top = y - height - 8;

    if (selected)
    {
        display.fillRect(MENU_LEFT_MARGIN, top, MENU_WIDTH, MENU_ITEM_HEIGHT, GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);
    }
    else
    {
        display.drawRect(MENU_LEFT_MARGIN, top, MENU_WIDTH, MENU_ITEM_HEIGHT, GxEPD_BLACK);
    }

    display.setCursor(x, y);
    display.println(label);
    display.setTextColor(GxEPD_BLACK);
}

void drawPomodoroTimer(uint16_t remainingSeconds, uint16_t totalSeconds, const char* status)
{
    const uint16_t minutes = remainingSeconds / 60;
    const uint8_t seconds = remainingSeconds % 60;
    char timeText[10];
    snprintf(timeText, sizeof(timeText), "%02u:%02u", static_cast<unsigned int>(minutes), static_cast<unsigned int>(seconds));

    drawCenteredText(timeText, 125);
    drawCenteredText(status, 165);

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
    const char* apps[] = { "1  QUOTE DISPLAY", "2  POMODORO", "3  DEMO SCREEN" };
    const char* descriptions[] = { "Browse daily quotes", "Focus timer: 25 minutes", "Reserved for your next app" };

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
        drawCenteredText("APP LAUNCHER", 34);
        display.setTextColor(GxEPD_BLACK);
        drawCenteredText(descriptions[selectedApp], 76);
        drawMenuItem(apps[0], 118, selectedApp == 0);
        drawMenuItem(apps[1], 166, selectedApp == 1);
        drawMenuItem(apps[2], 214, selectedApp == 2);
        drawCenteredText("UP/DOWN SELECT", 258);
        drawCenteredText("PRESS OPEN", 286);
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
        display.setFont(&FreeMonoBold12pt7b);
        display.fillRect(0, 0, DISPLAY_WIDTH, HEADER_HEIGHT, GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);
        drawCenteredText("POMODORO", 34);
        display.setTextColor(GxEPD_BLACK);
        drawPomodoroTimer(remainingSeconds, totalSeconds, status);
        drawCenteredText(action, 220);
        drawCenteredText("UP RESET  DOWN +5 MIN", 252);
        drawCenteredText("LONG PRESS: MENU", 282);
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

void showDemo(uint8_t appNumber)
{
    const char* title = appNumber == 2 ? "APP 2" : "APP 3";
    const char* message = appNumber == 2 ? "Demo screen ready" : "Another demo screen";

    display.setRotation(0);
    display.setFullWindow();
    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold12pt7b);
        drawCenteredText(title, 100);
        drawCenteredText(message, 150);
        drawCenteredText("PRESS: MENU", 250);
    } while (display.nextPage());
}
