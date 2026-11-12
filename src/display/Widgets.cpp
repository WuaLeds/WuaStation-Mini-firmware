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
}
