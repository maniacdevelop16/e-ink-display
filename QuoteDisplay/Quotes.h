#pragma once

#include <Arduino.h>

struct Quote
{
    const char* line1;
    const char* line2;
    const char* line3;
    const char* author;
};

extern const uint8_t NUM_QUOTES;

const Quote& getQuote(uint8_t index);
