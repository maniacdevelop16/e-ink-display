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
