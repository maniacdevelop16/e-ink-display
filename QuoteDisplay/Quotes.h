#pragma once

#include <Arduino.h>

struct Quote
{
    const char* line1;
    const char* line2;
    const char* line3;
    const char* author;
};

enum QuoteCategory
{
    QUOTE_CATEGORY_DEFAULT,
    QUOTE_CATEGORY_RAMAYANA,
    QUOTE_CATEGORY_MAHABHARATA,
    QUOTE_CATEGORY_KABIR,
    QUOTE_CATEGORY_UPANISHADS,
    QUOTE_CATEGORY_JAIN,
    QUOTE_CATEGORY_BUDDHIST,
    QUOTE_CATEGORY_COUNT
};

extern const uint8_t NUM_QUOTES;

const Quote& getQuote(uint8_t index);
const Quote& getQuote(QuoteCategory category, uint8_t index);
uint8_t quoteCount(QuoteCategory category);
const char* quoteCategoryLabel(QuoteCategory category);
