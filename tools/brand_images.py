#!/usr/bin/env python3
"""Render the Wualeds brand images Home Assistant asks for.

    python tools/brand_images.py [out=docs/brand/home-assistant]

Draws the symbol of docs/images/wualeds-logo.svg (arch and LED bars with the
pink-orange gradient) and writes the files of the home-assistant/brands
repository (core_integrations/wualeds/):

    icon.png      256 x 256   symbol, square, transparent
    icon@2x.png   512 x 512
    logo.png      128 px high symbol + "Wualeds" (dark text, light themes)
    logo@2x.png   256 px high
    dark_logo.png / dark_logo@2x.png   same with light text (dark themes)

The text uses the website's header style (system sans, weight 800: Segoe UI
Black on Windows). Needs Pillow and NumPy (pip install pillow numpy).
"""

import math
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont

VIEW_W, VIEW_H = 102, 158  # viewBox of wualeds-logo.svg
STOPS = [(0.0, (0xE8, 0x34, 0x8C)), (0.5, (0xF6, 0x60, 0x4A)), (1.0, (0xFB, 0x8B, 0x24))]
TEXT_LIGHT_THEME = (0x1C, 0x12, 0x1B)  # --text of wualeds.com
TEXT_DARK_THEME = (0xF6, 0xEE, 0xF4)
FONTS = ["C:/Windows/Fonts/seguibl.ttf", "C:/Windows/Fonts/segoeuib.ttf", "DejaVuSans-Bold.ttf"]
SS = 4  # supersampling


def arc(cx, cy, r, start, end, steps=90):
    return [
        (cx + r * math.cos(math.radians(start + (end - start) * i / steps)),
         cy + r * math.sin(math.radians(start + (end - start) * i / steps)))
        for i in range(steps + 1)
    ]


def symbol(height):
    """The symbol trimmed to its shapes, exactly `height` px high, transparent."""
    drawn = draw_symbol(round(height * VIEW_H / (155 - 10)) + 2)  # shapes span y 10..155
    drawn = drawn.crop(drawn.getbbox())
    return drawn.resize((round(drawn.width * height / drawn.height), height), Image.LANCZOS)


def draw_symbol(height):
    """The whole viewBox, `height` px high."""
    scale = height * SS / VIEW_H
    w, h = round(VIEW_W * scale), round(VIEW_H * scale)
    mask = Image.new("L", (w, h), 0)
    d = ImageDraw.Draw(mask)

    def pt(points):
        return [(x * scale, y * scale) for x, y in points]

    # M10 92V51a41 41 0 0 1 82 0v41H76V51a25 25 0 0 0-50 0v41z
    arch = [(10, 92)] + arc(51, 51, 41, 180, 360) + [(92, 92), (76, 92)] + arc(51, 51, 25, 360, 180) + [(26, 92)]
    d.polygon(pt(arch), fill=255)
    for x, y, rw, rh in [(2, 90, 98, 15), (26, 104, 18, 51), (60, 104, 16, 43)]:
        d.rounded_rectangle([x * scale, y * scale, (x + rw) * scale, (y + rh) * scale], radius=1.5 * scale, fill=255)

    # Linear gradient from (0,0) to (102,158), userSpaceOnUse.
    gx, gy = VIEW_W * scale, VIEW_H * scale
    ys, xs = np.mgrid[0:h, 0:w]
    t = np.clip((xs * gx + ys * gy) / (gx * gx + gy * gy), 0.0, 1.0)
    offsets = [s[0] for s in STOPS]
    rgb = np.stack([np.interp(t, offsets, [s[1][c] for s in STOPS]) for c in range(3)], axis=-1)
    out = Image.fromarray(np.round(rgb).astype(np.uint8), "RGB").convert("RGBA")
    out.putalpha(mask)
    return out.resize((round(w / SS), round(h / SS)), Image.LANCZOS)


def font(size):
    for name in FONTS:
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            continue
    sys.exit("no bold font found")


def icon(size):
    sym = symbol(size)
    canvas = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    canvas.alpha_composite(sym, ((size - sym.width) // 2, 0))
    return canvas


def logo(height, color):
    sym = symbol(height)
    f = font(round(height * 0.62))
    text = "Wualeds"
    left, top, right, bottom = f.getbbox(text)
    gap = round(height * 0.16)
    width = sym.width + gap + (right - left)
    canvas = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    canvas.alpha_composite(sym, (0, 0))
    # Text centred on the symbol's height, tracking -0.02em like the site header.
    d = ImageDraw.Draw(canvas)
    x = sym.width + gap - left
    y = (height - (bottom - top)) // 2 - top
    tracking = -0.02 * f.size
    for ch in text:
        d.text((x, y), ch, font=f, fill=color + (255,))
        x += f.getlength(ch) + tracking
    left, _, right, _ = canvas.getbbox()
    return canvas.crop((left, 0, right, height))  # keep the full height


def save(img, path):
    img.save(path, optimize=True)
    print(f"{path.name:22} {img.width} x {img.height}")


def main():
    out = Path(sys.argv[1] if len(sys.argv) > 1 else "docs/brand/home-assistant")
    out.mkdir(parents=True, exist_ok=True)
    save(icon(256), out / "icon.png")
    save(icon(512), out / "icon@2x.png")
    save(logo(128, TEXT_LIGHT_THEME), out / "logo.png")
    save(logo(256, TEXT_LIGHT_THEME), out / "logo@2x.png")
    save(logo(128, TEXT_DARK_THEME), out / "dark_logo.png")
    save(logo(256, TEXT_DARK_THEME), out / "dark_logo@2x.png")


if __name__ == "__main__":
    main()
