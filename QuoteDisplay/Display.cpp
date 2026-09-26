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
    constexpr int MENU_LEFT = 22;
    constexpr int MENU_WIDTH = DISPLAY_WIDTH - (MENU_LEFT * 2);
    const int top = y - height - 8;

    if (selected)
    {
        display.fillRect(MENU_LEFT, top, MENU_WIDTH, height + 16, GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);
    }
    else
    {
        display.drawRect(MENU_LEFT, top, MENU_WIDTH, height + 16, GxEPD_BLACK);
    }

    display.setCursor(x, y);
    display.println(label);
    display.setTextColor(GxEPD_BLACK);
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

    display.setRotation(0);
    display.setFullWindow();
    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold12pt7b);
        display.fillRect(0, 0, DISPLAY_WIDTH, 48, GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);
        drawCenteredText("APP LAUNCHER", 34);
        display.setTextColor(GxEPD_BLACK);
        drawCenteredText("SELECT AN APP", 76);
        drawMenuItem(apps[0], 118, selectedApp == 0);
        drawMenuItem(apps[1], 166, selectedApp == 1);
        drawMenuItem(apps[2], 214, selectedApp == 2);
        drawCenteredText("UP/DOWN SELECT", 258);
        drawCenteredText("PRESS OPEN", 286);
    } while (display.nextPage());
}

void showPomodoro(uint16_t remainingSeconds, bool running, bool complete)
{
    const uint16_t minutes = remainingSeconds / 60;
    const uint8_t seconds = remainingSeconds % 60;
    char timeText[10];
    snprintf(timeText, sizeof(timeText), "%02u:%02u", static_cast<unsigned int>(minutes), static_cast<unsigned int>(seconds));

    display.setRotation(0);
    display.setFullWindow();
    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold12pt7b);
        display.fillRect(0, 0, DISPLAY_WIDTH, 48, GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);
        drawCenteredText("POMODORO", 34);
        display.setTextColor(GxEPD_BLACK);
        drawCenteredText(timeText, 125);
        drawCenteredText(complete ? "SESSION COMPLETE" : (running ? "FOCUS IN PROGRESS" : "READY TO FOCUS"), 170);
        drawCenteredText("PRESS START/PAUSE", 220);
        drawCenteredText("UP RESET  DOWN +5 MIN", 252);
        drawCenteredText("LONG PRESS: MENU", 282);
    } while (display.nextPage());
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
