#include "display/Widgets.h"

#include <math.h>
#include "config/Config.h"
#include "display/Display.h"
#include "display/TinyFont.h"

namespace Widgets
{
    // ---- Fonts ---------------------------------------------------------------
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

    Font bestFont(const char *text)
    {
        if (textWidth(Font::Classic, text) <= Layout::kWidth)
            return Font::Classic;
        if (textWidth(Font::Tiny, text) <= Layout::kWidth)
            return Font::Tiny;
        return Font::Classic;
    }

    // ---- Icon / tag + value --------------------------------------------------
    namespace
    {
        // Value centered right of a `leftWidth` px element. Returns false
        // when the value only fits without the left element.
        bool placeValue(int16_t panelWidth, int16_t leftWidth, int16_t valueWidth, int16_t &x)
        {
            const int16_t areaX = leftWidth + 1; // one blank column after it
            const int16_t areaW = panelWidth - areaX;
            if (valueWidth <= areaW)
            {
                x = areaX + (areaW - valueWidth) / 2;
                return true;
            }
            if (leftWidth + valueWidth <= panelWidth)
            {
                x = panelWidth - valueWidth; // tight fit, no gap column
                return true;
            }
            x = (panelWidth - valueWidth) / 2;
            return false;
        }
    }

    void iconAndValue(Adafruit_GFX &g, const Icons::Icon &icon, const char *value, uint16_t color)
    {
        int16_t x;
        if (placeValue(g.width(), icon.width, TinyFont::textWidth(value), x))
            Icons::draw(g, icon, 0, 0);
        TinyFont::draw(g, x, 0, value, color);
    }

    void tagAndValue(Adafruit_GFX &g, const char *tag, uint16_t tagColor, const char *value,
                     uint16_t valueColor)
    {
        int16_t x;
        if (placeValue(g.width(), TinyFont::textWidth(tag), TinyFont::textWidth(value), x))
            TinyFont::draw(g, 0, 0, tag, tagColor);
        TinyFont::draw(g, x, 0, value, valueColor);
    }

    void formatTemperature(float celsius, char *out, size_t size)
    {
        const int t = static_cast<int>(lroundf(celsius));
        if (t <= -10)
            snprintf(out, size, "%d", t);
        else
            snprintf(out, size, "%d%c", t, TinyFont::kDegree);
    }

    // ---- Labels --------------------------------------------------------------
    void makeLabel(Label &label, const char *text, uint16_t color, Font font)
    {
        strlcpy(label.text, text, sizeof(label.text));
        label.font = font;
        label.color = color;
        label.width = textWidth(font, label.text);
    }

    uint32_t panDurationMs(const Label &label)
    {
        const int16_t overflow = label.width - Layout::kWidth;
        if (overflow <= 0)
            return 0;
        return 2 * Config::kPanHoldMs + static_cast<uint32_t>(overflow) * Config::kScrollSpeedMs;
    }

    int16_t panOffset(const Label &label, uint32_t elapsedMs)
    {
        const int16_t overflow = label.width - Layout::kWidth;
        if (overflow <= 0 || elapsedMs <= Config::kPanHoldMs)
            return 0;
        const uint32_t offset = (elapsedMs - Config::kPanHoldMs) / Config::kScrollSpeedMs;
        return offset >= static_cast<uint32_t>(overflow) ? overflow : static_cast<int16_t>(offset);
    }

    void drawLabel(Adafruit_GFX &g, const Label &label, uint32_t elapsedMs)
    {
        if (label.text[0] == '\0')
            return;
        const int16_t x = label.width <= g.width() ? (g.width() - label.width) / 2
                                                   : -panOffset(label, elapsedMs);
        drawText(g, label.font, x, label.text, label.color);
    }
}
