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

    if (selected)
    {
        display.fillRect(x - 12, y - height - 5, width + 24, height + 10, GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);
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
    const char* apps[] = { "App 1: Quote Display", "App 2: Demo Screen", "App 3: Demo Screen" };

    display.setRotation(0);
    display.setFullWindow();
    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold12pt7b);
        drawCenteredText("START MENU", 55);
        drawMenuItem(apps[0], 115, selectedApp == 0);
        drawMenuItem(apps[1], 160, selectedApp == 1);
        drawMenuItem(apps[2], 205, selectedApp == 2);
        drawCenteredText("UP/DOWN: SELECT", 250);
        drawCenteredText("PRESS: OPEN", 280);
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
