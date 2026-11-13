#include "display/Widgets.h"

#include "display/TinyFont.h"

namespace Widgets
{
    void iconAndValue(Adafruit_GFX &g, const Icons::Icon &icon, const char *value, uint16_t color)
    {
        const int16_t width = g.width();
        const int16_t textW = TinyFont::textWidth(value);
        const int16_t areaX = icon.width + 1; // one blank column after the icon
        const int16_t areaW = width - areaX;

        if (textW <= areaW)
        {
            Icons::draw(g, icon, 0, 0);
            TinyFont::draw(g, areaX + (areaW - textW) / 2, 0, value, color);
        }
        else if (icon.width + textW <= width)
        {
            // Tight fit: no gap column (icons have their own side padding).
            Icons::draw(g, icon, 0, 0);
            TinyFont::draw(g, width - textW, 0, value, color);
        }
        else
        {
            TinyFont::drawCentered(g, value, color);
        }
    }

    int16_t textWidth(Font font, const char *text)
    {
        if (font == Font::Tiny)
            return TinyFont::textWidth(text);
        // Classic GFX font at size 1: 5 px glyphs + 1 px spacing, monospaced.
        const int16_t n = static_cast<int16_t>(strlen(text));
        return n > 0 ? n * 6 - 1 : 0;
    }

    void drawText(Adafruit_GFX &g, Font font, int16_t x, const char *text, uint16_t color)
    {
        if (font == Font::Tiny)
        {
            TinyFont::draw(g, x, 0, text, color);
            return;
        }
        g.setFont(nullptr);
        g.setTextSize(1);
        g.setTextColor(color);
        g.setCursor(x, 0);
        g.print(text);
    }
}
