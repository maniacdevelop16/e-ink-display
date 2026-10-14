# Quote languages and categories

The Quote Display app stores its English text in `Quotes.cpp` and
`QuoteCategories.cpp`. Hindi text lives in `hi.json` in exactly the same quote
order. The Diwali category contains ten original short reflections; teachings
in the other named categories remain display-friendly adaptations.

To add a quote, add the English entry and its Hindi counterpart in the same
position. For a new category, add its enum and English entries in `Quotes.h`
and `QuoteCategories.cpp`, then add the category name and Hindi entries in
`hi.json` and `tools/generate_hindi_bitmaps.py`. Update the English category
count or selection logic as needed. The generator validates text width and
height so it cannot silently clip on the 400 × 300 display.

Generate the Hindi asset before compiling firmware:

```sh
python3 -m pip install Pillow
python3 QuoteDisplay/tools/generate_hindi_bitmaps.py
```

The generator also needs `hb-view` from HarfBuzz on the command path. It uses
`assets/NotoSansDevanagari.ttf`, which is distributed under the included Open
Font License. The generated `HindiBitmaps.cpp` is committed so ordinary
Arduino builds need neither Python nor HarfBuzz. For a visual check, pass
`--preview-dir /tmp/quote-preview` to the generator.

English continues to use the five global font choices. Hindi uses the
Devanagari font for accurate character shaping and follows the global
small/medium/large font-size setting. The selected quote language is saved
in the ESP32's settings and applies to every quote category, including future
ones.
