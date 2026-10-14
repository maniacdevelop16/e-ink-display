#include "Quotes.h"

namespace
{
const Quote quotes[] =
{
    { "Know thyself .", "", "", "Socrates" },
    { "The unexamined life", "is not worth living.", "", "Socrates" },
    { "I think, therefore", "I am.", "", "Descartes" },
    { "He who has a why", "to live can bear", "almost any how.", "Nietzsche" },
    { "We are what we", "repeatedly do.", "", "Aristotle" },
    { "Happiness depends", "upon ourselves.", "", "Aristotle" },
    { "The obstacle is", "the way.", "", "Marcus Aurelius" },
    { "What you seek", "is seeking you.", "", "Rumi" },
    { "No man is free", "who is not master", "of himself.", "Epictetus" },
    { "Be here now.", "", "", "Ram Dass" }
};
}

const uint8_t NUM_QUOTES = sizeof(quotes) / sizeof(quotes[0]);

const Quote& getQuote(uint8_t index)
{
    return quotes[index];
}
