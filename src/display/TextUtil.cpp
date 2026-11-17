#include "display/TextUtil.h"

namespace TextUtil
{
    namespace
    {
        struct Mapping
        {
            uint16_t codepoint; // Unicode (Latin-1 range)
            uint8_t cp437;      // CP437 index, or a plain ASCII fallback
        };

        // Latin-1 -> CP437. Letters missing from CP437 fall back to ASCII.
        // clang-format off
        const Mapping kMap[] = {
            {0x00A0, ' '},  {0x00A1, 0xAD}, {0x00A2, 0x9B}, {0x00A3, 0x9C},
            {0x00A5, 0x9D}, {0x00AA, 0xA6}, {0x00AB, 0xAE}, {0x00B0, 0xF8},
            {0x00B1, 0xF1}, {0x00B2, 0xFD}, {0x00B5, 0xE6}, {0x00B7, 0xFA},
            {0x00BA, 0xA7}, {0x00BB, 0xAF}, {0x00BC, 0xAC}, {0x00BD, 0xAB},
            {0x00BF, 0xA8},
            {0x00C0, 'A'},  {0x00C1, 'A'},  {0x00C2, 'A'},  {0x00C3, 'A'},
            {0x00C4, 0x8E}, {0x00C5, 0x8F}, {0x00C6, 0x92}, {0x00C7, 0x80},
            {0x00C8, 'E'},  {0x00C9, 0x90}, {0x00CA, 'E'},  {0x00CB, 'E'},
            {0x00CC, 'I'},  {0x00CD, 'I'},  {0x00CE, 'I'},  {0x00CF, 'I'},
            {0x00D1, 0xA5}, {0x00D2, 'O'},  {0x00D3, 'O'},  {0x00D4, 'O'},
            {0x00D5, 'O'},  {0x00D6, 0x99}, {0x00D9, 'U'},  {0x00DA, 'U'},
            {0x00DB, 'U'},  {0x00DC, 0x9A}, {0x00DF, 0xE1},
            {0x00E0, 0x85}, {0x00E1, 0xA0}, {0x00E2, 0x83}, {0x00E3, 'a'},
            {0x00E4, 0x84}, {0x00E5, 0x86}, {0x00E6, 0x91}, {0x00E7, 0x87},
            {0x00E8, 0x8A}, {0x00E9, 0x82}, {0x00EA, 0x88}, {0x00EB, 0x89},
            {0x00EC, 0x8D}, {0x00ED, 0xA1}, {0x00EE, 0x8C}, {0x00EF, 0x8B},
            {0x00F1, 0xA4}, {0x00F2, 0x95}, {0x00F3, 0xA2}, {0x00F4, 0x93},
            {0x00F5, 'o'},  {0x00F6, 0x94}, {0x00F7, 0xF6}, {0x00F9, 0x97},
            {0x00FA, 0xA3}, {0x00FB, 0x96}, {0x00FC, 0x81}, {0x00FF, 0x98},
            {0x20AC, 'E'}, // euro sign
        };
        // clang-format on

        uint8_t mapCodepoint(uint32_t cp)
        {
            for (const Mapping &m : kMap)
            {
                if (m.codepoint == cp)
                    return m.cp437;
            }
            return '?';
        }
    }

    String utf8ToCp437(const char *text)
    {
        String out;
        out.reserve(strlen(text));
        const uint8_t *p = reinterpret_cast<const uint8_t *>(text);
        while (*p)
        {
            uint32_t cp;
            uint8_t extra;
            if (*p < 0x80)
            {
                out += static_cast<char>(*p++);
                continue;
            }
            if ((*p & 0xE0) == 0xC0)
            {
                cp = *p & 0x1F;
                extra = 1;
            }
            else if ((*p & 0xF0) == 0xE0)
            {
                cp = *p & 0x0F;
                extra = 2;
            }
            else if ((*p & 0xF8) == 0xF0)
            {
                cp = *p & 0x07;
                extra = 3;
            }
            else
            {
                // Stray byte (not UTF-8): keep it as-is, it may already be CP437.
                out += static_cast<char>(*p++);
                continue;
            }
            ++p;
            bool valid = true;
            for (uint8_t i = 0; i < extra; ++i)
            {
                if ((*p & 0xC0) != 0x80)
                {
                    valid = false;
                    break;
                }
                cp = (cp << 6) | (*p++ & 0x3F);
            }
            out += static_cast<char>(valid ? mapCodepoint(cp) : '?');
        }
        return out;
    }

    String urlEncode(const char *text)
    {
        static const char kHex[] = "0123456789ABCDEF";
        String out;
        out.reserve(strlen(text) * 3);
        for (const uint8_t *p = reinterpret_cast<const uint8_t *>(text); *p; ++p)
        {
            const uint8_t c = *p;
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
            {
                out += static_cast<char>(c);
            }
            else
            {
                out += '%';
                out += kHex[c >> 4];
                out += kHex[c & 0x0F];
            }
        }
        return out;
    }
}
