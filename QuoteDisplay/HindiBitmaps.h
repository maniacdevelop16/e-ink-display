#pragma once

#include <Arduino.h>

#include "Quotes.h"
#include "Settings.h"

struct HindiBitmap
{
    const uint8_t* compressed;
    uint16_t width;
    uint16_t height;
    uint16_t length;
};

struct HindiQuoteArt
{
    HindiBitmap body;
    HindiBitmap author;
};

const HindiQuoteArt& getHindiQuoteArt(QuoteCategory category, uint8_t index, FontSize size);
const HindiBitmap& getHindiCategoryArt(QuoteCategory category, FontSize size);
const HindiBitmap& getHindiQuoteHintArt();
