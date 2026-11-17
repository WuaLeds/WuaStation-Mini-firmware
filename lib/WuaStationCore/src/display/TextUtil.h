#pragma once

#include <Arduino.h>

namespace TextUtil
{
    // Convert UTF-8 text to the CP437 charset of the classic GFX font
    // (requires cp437(true) on the target). Spanish/European accents, n-tilde,
    // inverted marks and the degree sign are mapped; unknown symbols become '?'.
    String utf8ToCp437(const char *text);

    // Percent-encode a string for use in a URL query value.
    String urlEncode(const char *text);
}
