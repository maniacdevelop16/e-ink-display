#include "Quotes.h"

namespace
{
constexpr uint8_t CATEGORY_QUOTE_COUNT = 10;

// These compact lines are display-friendly adaptations of teachings and
// passages, not claims of word-for-word translation.
const Quote ramayanaQuotes[CATEGORY_QUOTE_COUNT] =
{
    { "Let duty be", "your steady path.", "", "Rama (adapted)" },
    { "Purpose awakens", "the strength", "already within.", "Hanuman (adapted)" },
    { "Patience protects", "what anger", "would destroy.", "Sita (adapted)" },
    { "A crown has worth", "only when it", "serves the people.", "Bharata (adapted)" },
    { "Stand beside truth", "when the road", "becomes difficult.", "Lakshmana (adapted)" },
    { "Choose what is right", "even above", "kinship.", "Vibhishana (adapted)" },
    { "One brave act", "can outlive", "a lifetime.", "Jatayu (adapted)" },
    { "Strength needs", "wisdom to find", "its right direction.", "Tara (adapted)" },
    { "Hope can still see", "a path across", "the widest sea.", "Sampati (adapted)" },
    { "Compassion makes", "great power", "worthy.", "Valmiki (adapted)" }
};

const Quote mahabharataQuotes[CATEGORY_QUOTE_COUNT] =
{
    { "Give your effort.", "Release your claim", "upon the result.", "Krishna (adapted)" },
    { "Truth is the", "highest shelter", "of a ruler.", "Yudhishthira (adapted)" },
    { "Dignity can stand", "even when power", "refuses to listen.", "Draupadi (adapted)" },
    { "Duty is subtle.", "Meet it with", "patient wisdom.", "Bhishma (adapted)" },
    { "A wise person hears", "the unwelcome", "truth in time.", "Vidura (adapted)" },
    { "Hardship reveals", "the courage hidden", "in the heart.", "Kunti (adapted)" },
    { "A steady mind", "makes the arrow", "find its mark.", "Arjuna (adapted)" },
    { "Generosity shines", "when nothing is", "asked in return.", "Karna (adapted)" },
    { "Every choice plants", "a seed in", "the future.", "Vyasa (adapted)" },
    { "Grief becomes wisdom", "when it ends", "the cycle of harm.", "Gandhari (adapted)" }
};

const Quote kabirQuotes[CATEGORY_QUOTE_COUNT] =
{
    { "The river is within.", "Why search", "beyond yourself?", "Kabir (adapted)" },
    { "Where love is awake,", "fear finds", "no room.", "Kabir (adapted)" },
    { "The quiet heart", "hears what noise", "cannot teach.", "Kabir (adapted)" },
    { "Drop the veil.", "The light was", "always near.", "Kabir (adapted)" },
    { "Do not postpone", "the journey", "toward truth.", "Kabir (adapted)" },
    { "The same breath", "moves through", "every life.", "Kabir (adapted)" },
    { "A humble cup", "can hold the", "whole sky.", "Kabir (adapted)" },
    { "Look with love.", "The ordinary", "becomes sacred.", "Kabir (adapted)" },
    { "Let the false self", "fall away", "without regret.", "Kabir (adapted)" },
    { "Home is found", "when seeking", "becomes still.", "Kabir (adapted)" }
};

const Quote upanishadQuotes[CATEGORY_QUOTE_COUNT] =
{
    { "All this world", "is held within", "the sacred whole.", "Isha Upanishad" },
    { "The ear of the ear", "is nearer than", "the mind can reach.", "Kena Upanishad" },
    { "Choose the good", "over the merely", "pleasant.", "Katha Upanishad" },
    { "Truth alone", "opens the path", "beyond illusion.", "Mundaka Upanishad" },
    { "You are rooted", "in the vast", "reality of being.", "Chandogya Upanishad" },
    { "From fullness,", "fullness always", "remains.", "Brihadaranyaka" },
    { "Speak truth.", "Walk in", "right conduct.", "Taittiriya Upanishad" },
    { "The quiet fourth", "is peace beyond", "all division.", "Mandukya Upanishad" },
    { "Life rises through", "breath, attention,", "and awareness.", "Prashna Upanishad" },
    { "The one light", "dwells quietly", "in every being.", "Shvetashvatara" }
};

const Quote jainQuotes[CATEGORY_QUOTE_COUNT] =
{
    { "Nonviolence is", "the heart of", "clear conduct.", "Mahavira (adapted)" },
    { "Know the self", "apart from", "passing attachments.", "Kundakunda (adapted)" },
    { "Right faith, knowledge,", "and conduct form", "the path.", "Umasvati (adapted)" },
    { "Many viewpoints", "can reveal one", "complex truth.", "Siddhasena (adapted)" },
    { "Restraint turns", "energy toward", "freedom.", "Hemachandra (adapted)" },
    { "Compassion begins", "by seeing life", "everywhere.", "Haribhadra (adapted)" },
    { "Living beings", "support one", "another.", "Tattvartha Sutra" },
    { "Conquer anger", "with awareness", "and patience.", "Bhadrabahu (adapted)" },
    { "Release possession", "and the spirit", "grows lighter.", "Pujyapada (adapted)" },
    { "The pure self", "is luminous", "and free.", "Amritachandra (adapted)" }
};

const Quote buddhistQuotes[CATEGORY_QUOTE_COUNT] =
{
    { "Peace grows", "from a mind", "that understands.", "Buddha (adapted)" },
    { "We become shaped", "by the thoughts", "we cultivate.", "Dhammapada (adapted)" },
    { "Nothing stands", "alone or exists", "by itself.", "Nagarjuna (adapted)" },
    { "All joy begins", "in wishing well", "for others.", "Shantideva (adapted)" },
    { "Awareness makes", "even a small act", "complete.", "Buddhaghosa (adapted)" },
    { "See the mind", "and confusion", "loses its hold.", "Milarepa (adapted)" },
    { "Let compassion", "and wisdom", "travel together.", "Atisha (adapted)" },
    { "Practice is found", "in this very", "moment.", "Dogen (adapted)" },
    { "Experience flows", "without a fixed", "self at its center.", "Vasubandhu (adapted)" },
    { "A quiet mind", "sees change", "without fear.", "Ashvaghosha (adapted)" }
};

// Original, short reflections for the festival; no historical attribution implied.
const Quote diwaliQuotes[CATEGORY_QUOTE_COUNT] =
{
    { "One small lamp", "can soften", "a long night.", "Diwali Reflection" },
    { "Make room for light", "in the places", "you call home.", "Diwali Reflection" },
    { "Share your joy.", "It grows brighter", "in another hand.", "Diwali Reflection" },
    { "A new beginning", "can start with", "one kind act.", "Diwali Reflection" },
    { "Let gratitude", "be the first lamp", "you light today.", "Diwali Reflection" },
    { "Even a quiet flame", "can guide someone", "toward hope.", "Diwali Reflection" },
    { "Celebrate the light", "you find", "in each other.", "Diwali Reflection" },
    { "Leave yesterday's", "shadows behind.", "Welcome today.", "Diwali Reflection" },
    { "May your home", "hold warmth, peace,", "and open hearts.", "Diwali Reflection" },
    { "Keep a little light", "for the road", "ahead.", "Diwali Reflection" }
};
}

const Quote& getQuote(QuoteCategory category, uint8_t index)
{
    const uint8_t safeIndex = index % quoteCount(category);
    switch (category)
    {
        case QUOTE_CATEGORY_RAMAYANA: return ramayanaQuotes[safeIndex];
        case QUOTE_CATEGORY_MAHABHARATA: return mahabharataQuotes[safeIndex];
        case QUOTE_CATEGORY_KABIR: return kabirQuotes[safeIndex];
        case QUOTE_CATEGORY_UPANISHADS: return upanishadQuotes[safeIndex];
        case QUOTE_CATEGORY_JAIN: return jainQuotes[safeIndex];
        case QUOTE_CATEGORY_BUDDHIST: return buddhistQuotes[safeIndex];
        case QUOTE_CATEGORY_DIWALI: return diwaliQuotes[safeIndex];
        case QUOTE_CATEGORY_DEFAULT:
        case QUOTE_CATEGORY_COUNT:
        default: return getQuote(safeIndex);
    }
}

uint8_t quoteCount(QuoteCategory category)
{
    return category == QUOTE_CATEGORY_DEFAULT ? NUM_QUOTES : CATEGORY_QUOTE_COUNT;
}

const char* quoteCategoryLabel(QuoteCategory category)
{
    const char* labels[] = {
        "DEFAULT", "RAMAYANA", "MAHABHARATA", "KABIR",
        "UPANISHADS", "JAIN", "BUDDHIST", "DIWALI"
    };
    return labels[category < QUOTE_CATEGORY_COUNT ? category : QUOTE_CATEGORY_DEFAULT];
}
