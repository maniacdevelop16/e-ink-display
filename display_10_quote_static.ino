#include <SPI.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold12pt7b.h>

// =================================================
// E-PAPER PINS
// =================================================

#define EPD_CS     45
#define EPD_DC     46
#define EPD_RST    47
#define EPD_BUSY   48

#define EPD_SCK    12
#define EPD_MOSI   11
#define EPD_POWER  7

// =================================================
// ROTARY BUTTON PINS
// =================================================

#define ROTARY_UP      6
#define ROTARY_DOWN    4
#define ROTARY_PRESS   5

// =================================================
// DISPLAY
// =================================================

GxEPD2_BW<GxEPD2_420_GYE042A87,
          GxEPD2_420_GYE042A87::HEIGHT> display(
    GxEPD2_420_GYE042A87(
        EPD_CS,
        EPD_DC,
        EPD_RST,
        EPD_BUSY
    )
);

// =================================================
// QUOTES
// =================================================
//
// Format:
// { quote line 1,
//   quote line 2,
//   quote line 3,
//   author }
//
// Keep quote text relatively short so it looks
// elegant on the 400 x 300 display.
//

struct Quote
{
    const char* line1;
    const char* line2;
    const char* line3;
    const char* author;
};

Quote quotes[] =
{
    {
        "Know thyself.",
        "",
        "",
        "Socrates"
    },

    {
        "The unexamined life",
        "is not worth living.",
        "",
        "Socrates"
    },

    {
        "I think, therefore",
        "I am.",
        "",
        "Descartes"
    },

    {
        "He who has a why",
        "to live can bear",
        "almost any how.",
        "Nietzsche"
    },

    {
        "We are what we",
        "repeatedly do.",
        "",
        "Aristotle"
    },

    {
        "Happiness depends",
        "upon ourselves.",
        "",
        "Aristotle"
    },

    {
        "The obstacle is",
        "the way.",
        "",
        "Marcus Aurelius"
    },

    {
        "What you seek",
        "is seeking you.",
        "",
        "Rumi"
    },

    {
        "No man is free",
        "who is not master",
        "of himself.",
        "Epictetus"
    },

    {
        "Be here now.",
        "",
        "",
        "Ram Dass"
    }
};

const int NUM_QUOTES = sizeof(quotes) / sizeof(quotes[0]);

int currentQuote = 0;

// =================================================
// DISPLAY SETTINGS
// =================================================

const int QUOTE_X = 200;

const int QUOTE_Y1 = 105;
const int QUOTE_Y2 = 135;
const int QUOTE_Y3 = 165;

const int AUTHOR_Y = 215;

const int DEBOUNCE_TIME = 80;


// =================================================
// HELPER: CENTER TEXT
// =================================================

void drawCenteredText(const char* text, int y)
{
    if (text == nullptr || text[0] == '\0')
        return;

    int16_t x1;
    int16_t y1;
    uint16_t w;
    uint16_t h;

    display.getTextBounds(
        text,
        0,
        y,
        &x1,
        &y1,
        &w,
        &h
    );

    int x = (400 - w) / 2;

    display.setCursor(x, y);
    display.println(text);
}


// =================================================
// DISPLAY QUOTE
// =================================================

void showQuote(int index)
{
    display.setRotation(0);
    display.setFullWindow();

    display.firstPage();

    do
    {
        // -----------------------------
        // Clear screen
        // -----------------------------

        display.fillScreen(GxEPD_WHITE);

        // -----------------------------
        // Quote font
        // -----------------------------

        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold12pt7b);

        // -----------------------------
        // Quote
        // -----------------------------

        drawCenteredText(
            quotes[index].line1,
            QUOTE_Y1
        );

        drawCenteredText(
            quotes[index].line2,
            QUOTE_Y2
        );

        drawCenteredText(
            quotes[index].line3,
            QUOTE_Y3
        );

        // -----------------------------
        // Author
        // -----------------------------

        if (quotes[index].author[0] != '\0')
        {
            drawCenteredText(
                "- ",
                AUTHOR_Y
            );

            // Draw author manually with "- "
            char authorText[50];

            snprintf(
                authorText,
                sizeof(authorText),
                "- %s",
                quotes[index].author
            );

            drawCenteredText(
                authorText,
                AUTHOR_Y
            );
        }

        // -----------------------------
        // Quote counter
        // -----------------------------

        char counter[10];

        snprintf(
            counter,
            sizeof(counter),
            "%d / %d",
            index + 1,
            NUM_QUOTES
        );

        display.setFont(&FreeMonoBold12pt7b);

        int16_t x1;
        int16_t y1;
        uint16_t w;
        uint16_t h;

        display.getTextBounds(
            counter,
            0,
            0,
            &x1,
            &y1,
            &w,
            &h
        );

        display.setCursor(
            400 - w - 15,
            285
        );

        display.println(counter);

    } while (display.nextPage());

    // -----------------------------
    // Serial information
    // -----------------------------

    Serial.print("Quote ");
    Serial.print(index + 1);
    Serial.print("/");
    Serial.print(NUM_QUOTES);
    Serial.print(": ");

    Serial.println(quotes[index].line1);
}


// =================================================
// NEXT QUOTE
// =================================================

void nextQuote()
{
    currentQuote++;

    // Circular navigation
    if (currentQuote >= NUM_QUOTES)
    {
        currentQuote = 0;
    }

    showQuote(currentQuote);
}


// =================================================
// PREVIOUS QUOTE
// =================================================

void previousQuote()
{
    currentQuote--;

    // Circular navigation
    if (currentQuote < 0)
    {
        currentQuote = NUM_QUOTES - 1;
    }

    showQuote(currentQuote);
}


// =================================================
// WAIT FOR RELEASE
// =================================================

void waitForRelease(int pin)
{
    while (digitalRead(pin) == LOW)
    {
        delay(10);
    }

    delay(DEBOUNCE_TIME);
}


// =================================================
// SETUP
// =================================================

void setup()
{
    Serial.begin(115200);

    delay(2000);

    Serial.println();
    Serial.println("==============================");
    Serial.println(" CrowPanel Quote Display");
    Serial.println("==============================");

    // =================================================
    // E-PAPER POWER
    // =================================================

    pinMode(EPD_POWER, OUTPUT);

    digitalWrite(EPD_POWER, HIGH);

    delay(1000);

    // =================================================
    // ROTARY INPUTS
    // =================================================

    pinMode(ROTARY_UP, INPUT_PULLUP);
    pinMode(ROTARY_DOWN, INPUT_PULLUP);
    pinMode(ROTARY_PRESS, INPUT_PULLUP);

    // =================================================
    // SPI
    // =================================================

    SPI.begin(
        EPD_SCK,
        -1,
        EPD_MOSI,
        EPD_CS
    );

    delay(500);

    // =================================================
    // DISPLAY INITIALIZATION
    // =================================================

    display.init(115200);

    // =================================================
    // FIRST QUOTE
    // =================================================

    showQuote(currentQuote);

    Serial.println("DISPLAY READY");
    Serial.println();
    Serial.println("Rotary UP    -> Next quote");
    Serial.println("Rotary DOWN  -> Previous quote");
    Serial.println("Rotary PRESS -> Refresh quote");
}


// =================================================
// LOOP
// =================================================

void loop()
{
    // =================================================
    // ROTARY UP
    // =================================================

    if (digitalRead(ROTARY_UP) == LOW)
    {
        delay(DEBOUNCE_TIME);

        if (digitalRead(ROTARY_UP) == LOW)
        {
            Serial.println("ROTARY: NEXT");

            previousQuote();

            waitForRelease(ROTARY_UP);
        }
    }


    // =================================================
    // ROTARY DOWN
    // =================================================

    if (digitalRead(ROTARY_DOWN) == LOW)
    {
        delay(DEBOUNCE_TIME);

        if (digitalRead(ROTARY_DOWN) == LOW)
        {
            Serial.println("ROTARY: PREVIOUS");
            
            nextQuote();

            waitForRelease(ROTARY_DOWN);
        }
    }


    // =================================================
    // ROTARY PRESS
    // =================================================

    if (digitalRead(ROTARY_PRESS) == LOW)
    {
        delay(DEBOUNCE_TIME);

        if (digitalRead(ROTARY_PRESS) == LOW)
        {
            Serial.println("ROTARY: PRESS");

            // Refresh current quote
            showQuote(currentQuote);

            waitForRelease(ROTARY_PRESS);
        }
    }

    delay(10);
}