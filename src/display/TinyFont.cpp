#include "display/TinyFont.h"

namespace TinyFont
{
    namespace
    {
        // Each glyph is `width` columns x 7 rows, written row by row as a
        // string ('#' = lit). Easy to read and to tweak by hand.
        struct Glyph
        {
            char ch;
            uint8_t width;
            const char *rows;
        };

        // clang-format off
        const Glyph kGlyphs[] = {
            {'0', 3, "###" "#.#" "#.#" "#.#" "#.#" "#.#" "###"},
            {'1', 3, ".#." "##." ".#." ".#." ".#." ".#." "###"},
            {'2', 3, "###" "..#" "..#" "###" "#.." "#.." "###"},
            {'3', 3, "###" "..#" "..#" ".##" "..#" "..#" "###"},
            {'4', 3, "#.#" "#.#" "#.#" "###" "..#" "..#" "..#"},
            {'5', 3, "###" "#.." "#.." "###" "..#" "..#" "###"},
            {'6', 3, "###" "#.." "#.." "###" "#.#" "#.#" "###"},
            {'7', 3, "###" "..#" "..#" ".#." ".#." ".#." ".#."},
            {'8', 3, "###" "#.#" "#.#" "###" "#.#" "#.#" "###"},
            {'9', 3, "###" "#.#" "#.#" "###" "..#" "..#" "###"},
            {':', 1, "."   "."   "#"   "."   "#"   "."   "."  },
            {'.', 1, "."   "."   "."   "."   "."   "."   "#"  },
            {'-', 2, ".."  ".."  ".."  "##"  ".."  ".."  ".." },
            {'+', 3, "..." "..." ".#." "###" ".#." "..." "..."},
            {'%', 3, "#.." "#.#" "..#" ".#." "#.." "#.#" "..#"},
            {'/', 3, "..#" "..#" ".#." ".#." ".#." "#.." "#.."},
            {' ', 2, ".."  ".."  ".."  ".."  ".."  ".."  ".." },
            {kDegree, 2, "##" "##" ".." ".." ".." ".." ".."},
        };
        // clang-format on

        const Glyph *find(char c)
        {
            for (const Glyph &g : kGlyphs)
            {
                if (g.ch == c)
                    return &g;
            }
            return nullptr;
        }
    }

    int16_t charWidth(char c)
    {
        const Glyph *g = find(c);
        return g ? g->width : 0;
    }

    int16_t textWidth(const char *text)
    {
        int16_t width = 0;
        bool first = true;
        for (const char *p = text; *p; ++p)
        {
            const Glyph *g = find(*p);
            if (!g)
                continue;
            if (!first)
                width += kSpacing;
            width += g->width;
            first = false;
        }
        return width;
    }

    int16_t draw(Adafruit_GFX &gfx, int16_t x, int16_t y, const char *text, uint16_t color)
    {
        for (const char *p = text; *p; ++p)
        {
            const Glyph *g = find(*p);
            if (!g)
                continue;
            for (int16_t row = 0; row < kHeight; ++row)
            {
                for (int16_t col = 0; col < g->width; ++col)
                {
                    if (g->rows[row * g->width + col] == '#')
                        gfx.drawPixel(x + col, y + row, color);
                }
            }
            x += g->width + kSpacing;
        }
        return x;
    }

    void drawCentered(Adafruit_GFX &gfx, const char *text, uint16_t color,
                      int16_t areaX, int16_t areaW, int16_t y)
    {
        if (areaW < 0)
            areaW = gfx.width() - areaX;
        int16_t x = areaX + (areaW - textWidth(text)) / 2;
        if (x < areaX)
            x = areaX;
        draw(gfx, x, y, text, color);
    }
}
