#!/usr/bin/env python3
"""Shape Hindi with HarfBuzz and pack rendered quote art for the e-paper.

Run after changing content/hi.json or adding a category. Requires hb-view and Pillow.
The generated C++ file is committed, so Arduino builds need neither tool.
"""

import argparse
import json
import subprocess
import tempfile
from pathlib import Path

from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parents[1]
FONT = ROOT / "assets" / "NotoSansDevanagari.ttf"
SOURCE = ROOT / "content" / "hi.json"
OUTPUT = ROOT / "HindiBitmaps.cpp"
CATEGORIES = (
    "default", "ramayana", "mahabharata", "kabir",
    "upanishads", "jain", "buddhist", "diwali",
)
CATEGORY_NAMES = (
    "सामान्य", "रामायण", "महाभारत", "कबीर",
    "उपनिषद", "जैन दर्शन", "बौद्ध दर्शन", "दीपावली",
)
BODY_SIZES = (19, 24, 29)
AUTHOR_SIZES = (16, 18, 20)
HEADER_SIZES = (16, 18, 20)
BODY_SPACING = (32, 36, 40)
MAX_BODY_WIDTH = 360
MAX_AUTHOR_WIDTH = 340


def shaped(text: str, size: int, temporary: Path) -> Image.Image:
    output = temporary / "line.png"
    subprocess.run(
        ["hb-view", f"--font-size={size}", "--margin=0", "--background=none",
         "--foreground=000000", "-o", str(output), str(FONT), text],
        check=True, capture_output=True,
    )
    picture = Image.open(output).convert("RGBA")
    alpha = picture.getchannel("A")
    bounds = alpha.getbbox()
    if bounds is None:
        raise ValueError(f"No rendered pixels: {text!r}")
    return alpha.crop(bounds)


def compose_body(lines: list[str], size: int, spacing: int, temporary: Path) -> Image.Image:
    rendered = [shaped(line, size, temporary) if line else None for line in lines]
    width = max((image.width for image in rendered if image is not None), default=0)
    height = max(i * spacing + image.height for i, image in enumerate(rendered)
                 if image is not None)
    canvas = Image.new("L", (width, height), 0)
    for index, image in enumerate(rendered):
        if image is not None:
            canvas.paste(image, ((width - image.width) // 2, index * spacing))
    return canvas


def packed(image: Image.Image) -> bytes:
    width, height = image.size
    stride = (width + 7) // 8
    result = bytearray(stride * height)
    for y in range(height):
        for x in range(width):
            if image.getpixel((x, y)) >= 100:
                result[y * stride + x // 8] |= 0x80 >> (x % 8)
    return bytes(result)


def compress(raw: bytes) -> bytes:
    """Tokens 0..127 carry 1..128 literal bytes; 128..255 carry zero runs."""
    result = bytearray()
    index = 0
    while index < len(raw):
        if raw[index] == 0:
            end = index
            while end < len(raw) and raw[end] == 0 and end - index < 128:
                end += 1
            result.append(127 + end - index)
            index = end
        else:
            end = index
            while end < len(raw) and raw[end] != 0 and end - index < 128:
                end += 1
            result.append(end - index - 1)
            result.extend(raw[index:end])
            index = end
    return bytes(result)


def preview(content: dict, temporary: Path, directory: Path) -> None:
    directory.mkdir(parents=True, exist_ok=True)
    for category in ("default", "diwali"):
        category_index = CATEGORIES.index(category)
        quote = content[category][0]
        canvas = Image.new("RGB", (400, 300), "white")
        draw = ImageDraw.Draw(canvas)
        draw.line((82, 56, 318, 56), fill="black")
        draw.line((58, 84, 58, 186), fill="black")
        draw.line((104, 250, 296, 250), fill="black")

        def paste(image: Image.Image, top: int) -> None:
            canvas.paste(Image.new("RGB", image.size, "black"),
                         ((400 - image.width) // 2, top), image)

        paste(shaped(CATEGORY_NAMES[category_index], HEADER_SIZES[1], temporary), 17)
        paste(compose_body(quote[:3], BODY_SIZES[1], BODY_SPACING[1], temporary), 97)
        paste(shaped("— " + quote[3], AUTHOR_SIZES[1], temporary), 217)
        draw.text((185, 262), "1 / 10", fill="black")
        canvas.save(directory / f"hindi-{category}.png")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--preview-dir", type=Path)
    args = parser.parse_args()
    content = json.loads(SOURCE.read_text(encoding="utf-8"))
    if list(key for key in content if key != "note") != list(CATEGORIES):
        raise ValueError("Category order must match QuoteCategory in Quotes.h")
    for name in CATEGORIES:
        if not 0 < len(content[name]) <= 255:
            raise ValueError(f"{name}: expected 1–255 quotes")
        for quote in content[name]:
            if len(quote) != 4 or not quote[0] or not quote[3]:
                raise ValueError(f"{name}: expected three quote lines and an author")

    declarations = []
    body_rows = []
    quote_counts = []
    category_rows = []
    output_bytes = 0

    with tempfile.TemporaryDirectory() as temporary_name:
        temporary = Path(temporary_name)

        def add_bitmap(name: str, image: Image.Image, max_width: int) -> str:
            nonlocal output_bytes
            if image.width > max_width:
                raise ValueError(f"{name} is {image.width}px wide; limit is {max_width}px")
            if name.endswith("_body") and image.height > 120:
                raise ValueError(f"{name} is too tall for the author area")
            if name.endswith("_author") and image.height > 30:
                raise ValueError(f"{name} is too tall for the footer area")
            raw = packed(image)
            if len(raw) > 8192:
                raise ValueError(f"{name} exceeds the 8 KB rendering buffer")
            data = compress(raw)
            output_bytes += len(data)
            values = ", ".join(f"0x{byte:02x}" for byte in data)
            declarations.append(f"static const uint8_t {name}[] PROGMEM = {{{values}}};")
            return f"{{{name}, {image.width}, {image.height}, {len(data)}}}"

        for category_index, category in enumerate(CATEGORIES):
            quotes = []
            labels = []
            for size_index in range(3):
                name = f"category_{category_index}_{size_index}"
                labels.append(add_bitmap(
                    name, shaped(CATEGORY_NAMES[category_index], HEADER_SIZES[size_index], temporary),
                    MAX_AUTHOR_WIDTH,
                ))
            category_rows.append("{" + ", ".join(labels) + "}")

            for quote_index, quote in enumerate(content[category]):
                variants = []
                for size_index in range(3):
                    prefix = f"quote_{category_index}_{quote_index}_{size_index}"
                    body = add_bitmap(
                        prefix + "_body",
                        compose_body(quote[:3], BODY_SIZES[size_index],
                                     BODY_SPACING[size_index], temporary),
                        MAX_BODY_WIDTH,
                    )
                    author = add_bitmap(
                        prefix + "_author",
                        shaped("— " + quote[3], AUTHOR_SIZES[size_index], temporary),
                        MAX_AUTHOR_WIDTH,
                    )
                    variants.append("{" + body + ", " + author + "}")
                quotes.append("{" + ", ".join(variants) + "}")
            body_rows.append(
                f"const HindiQuoteArt quotes_{category_index}[][3] = {{\n"
                + ",\n".join(quotes) + "\n};"
            )
            quote_counts.append(len(quotes))

        hint = add_bitmap(
            "quote_hint", shaped("ऊपर/नीचे: कथन   दबाएँ: विकल्प", 12, temporary),
            MAX_BODY_WIDTH,
        )
        if args.preview_dir:
            preview(content, temporary, args.preview_dir)

    generated = (
        "// Generated by tools/generate_hindi_bitmaps.py from content/hi.json.\n"
        "// Devanagari shaping happens at generation time; the device draws packed pixels.\n"
        '#include "HindiBitmaps.h"\n\n'
        "namespace {\n" + "\n".join(declarations) + "\n"
        + "\n".join(body_rows) + "\n"
        + "const HindiQuoteArt (*quotes[QUOTE_CATEGORY_COUNT])[3] = {"
        + ", ".join(f"quotes_{index}" for index in range(len(CATEGORIES))) + "};\n"
        + "const uint8_t counts[QUOTE_CATEGORY_COUNT] = {"
        + ", ".join(str(count) for count in quote_counts) + "};\n"
        "const HindiBitmap categories[QUOTE_CATEGORY_COUNT][3] = {\n"
        + ",\n".join(category_rows) + "\n};\n"
        f"const HindiBitmap hint = {hint};\n"
        "}\n\n"
        "const HindiQuoteArt& getHindiQuoteArt(QuoteCategory category, uint8_t index, FontSize size)\n"
        "{\n    const uint8_t safeCategory = category < QUOTE_CATEGORY_COUNT ? category : QUOTE_CATEGORY_DEFAULT;\n"
        "    return quotes[safeCategory][index % counts[safeCategory]]"
        "[size < 3 ? size : FONT_SIZE_MEDIUM];\n}\n\n"
        "const HindiBitmap& getHindiCategoryArt(QuoteCategory category, FontSize size)\n"
        "{\n    return categories[category < QUOTE_CATEGORY_COUNT ? category : QUOTE_CATEGORY_DEFAULT]"
        "[size < 3 ? size : FONT_SIZE_MEDIUM];\n}\n\n"
        "const HindiBitmap& getHindiQuoteHintArt() { return hint; }\n"
    )
    OUTPUT.write_text(generated, encoding="utf-8")
    print(f"Generated {OUTPUT.name}: {output_bytes:,} compressed bitmap bytes")


if __name__ == "__main__":
    main()
