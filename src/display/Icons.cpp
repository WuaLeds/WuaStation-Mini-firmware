#include "display/Icons.h"

#include "config/Theme.h"

namespace Icons
{
    // clang-format off
    const Icon kSun{7, 7,
        "...Y..."
        ".Y...Y."
        "..YYY.."
        "Y.YYY.Y"
        "..YYY.."
        ".Y...Y."
        "...Y..."};

    const Icon kMoon{7, 7,
        "..MMM.."
        ".MM...."
        "MM....."
        "MM....."
        "MM....."
        ".MM...."
        "..MMM.."};

    const Icon kPartlyCloudy{7, 7,
        ".Y.Y..."
        "..YY..."
        "YYY.WW."
        "..WWWWW"
        ".WWWWWW"
        "WWWWWWW"
        "......."};

    const Icon kPartlyCloudyNight{7, 7,
        ".MM...."
        "MM....."
        "MM..WW."
        ".MWWWWW"
        ".WWWWWW"
        "WWWWWWW"
        "......."};

    const Icon kCloud{7, 7,
        "......."
        "..WW..."
        ".WWWWW."
        "WWWWWWW"
        "WWWWWWW"
        ".WWWWW."
        "......."};

    const Icon kFog{7, 7,
        "......."
        "GGGGGG."
        "......."
        ".GGGGGG"
        "......."
        "GGGGGG."
        "......."};

    const Icon kRain{7, 7,
        "..WW..."
        ".WWWWW."
        "WWWWWWW"
        ".WWWWW."
        ".B..B.."
        "B..B..B"
        "..B..B."};

    const Icon kSnow{7, 7,
        "..WW..."
        ".WWWWW."
        "WWWWWWW"
        ".WWWWW."
        ".S..S.."
        "...S..S"
        ".S..S.."};

    const Icon kThunder{7, 7,
        "..WW..."
        ".WWWWW."
        "WWWWWWW"
        ".WWYWW."
        "...YY.."
        "....Y.."
        "...Y..."};

    const Icon kUnknown{7, 7,
        "..GGG.."
        ".G...G."
        ".....G."
        "....G.."
        "...G..."
        "......."
        "...G..."};

    const Icon kHouse{7, 7,
        "...R..."
        "..RRR.."
        ".RRRRR."
        "RRRRRRR"
        ".WWWWW."
        ".WW.WW."
        ".WW.WW."};

    const Icon kDrop{7, 7,
        "...B..."
        "...B..."
        "..BBB.."
        ".BBBBB."
        ".BBBBB."
        ".BBBBB."
        "..BBB.."};

    const Icon kWifi{7, 7,
        ".CCCCC."
        "C.....C"
        "..CCC.."
        ".C...C."
        "......."
        "...C..."
        "......."};

    const Icon kArrowUp{7, 7,
        "...R..."
        "..RRR.."
        ".R.R.R."
        "R..R..R"
        "...R..."
        "...R..."
        "...R..."};

    const Icon kArrowDown{7, 7,
        "...B..."
        "...B..."
        "...B..."
        "B..B..B"
        ".B.B.B."
        "..BBB.."
        "...B..."};

    const Icon kUmbrella{7, 7,
        "..BBB.."
        ".BBBBB."
        "BBBBBBB"
        "...W..."
        "...W..."
        ".W.W..."
        "..W...."};

    const Icon kHourglass{7, 7,
        ".WWWWW."
        ".WYYYW."
        "..WYW.."
        "...W..."
        "..W.W.."
        ".WYYYW."
        ".WWWWW."};

    const Icon kTomato{7, 7,
        "..g.g.."
        "...g..."
        ".RRRRR."
        "RRRRRRR"
        "RRRRRRR"
        ".RRRRR."
        "..RRR.."};
    // clang-format on

    namespace
    {
        bool paletteColor(char c, uint16_t &out)
        {
            Rgb rgb{0, 0, 0};
            switch (c)
            {
            case 'Y': rgb = {255, 190, 0}; break;
            case 'O': rgb = {255, 110, 0}; break;
            case 'W': rgb = {200, 200, 215}; break;
            case 'G': rgb = {110, 110, 125}; break;
            case 'B': rgb = {30, 110, 255}; break;
            case 'C': rgb = {0, 200, 255}; break;
            case 'S': rgb = {180, 220, 255}; break;
            case 'R': rgb = {255, 40, 20}; break;
            case 'M': rgb = {230, 230, 160}; break;
            case 'g': rgb = {40, 200, 40}; break;
            default: return false;
            }
            out = rgb.to565();
            return true;
        }
    }

    void draw(Adafruit_GFX &g, const Icon &icon, int16_t x, int16_t y)
    {
        for (int16_t row = 0; row < icon.height; ++row)
        {
            for (int16_t col = 0; col < icon.width; ++col)
            {
                uint16_t color;
                if (paletteColor(icon.pixels[row * icon.width + col], color))
                    g.drawPixel(x + col, y + row, color);
            }
        }
    }

    void drawTinted(Adafruit_GFX &g, const Icon &icon, int16_t x, int16_t y, uint16_t color)
    {
        for (int16_t row = 0; row < icon.height; ++row)
        {
            for (int16_t col = 0; col < icon.width; ++col)
            {
                if (icon.pixels[row * icon.width + col] != '.')
                    g.drawPixel(x + col, y + row, color);
            }
        }
    }
}
