# vlwconv

vlwconv converts FreeType-compatible fonts to antialiased VLW format, suitable
for use with [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI).

I've used it a few times, it seems to work, but issues may exist.

vlwconv is written in Python 3. Install Python and the requirements
(freetype-py) before using.
Todo (eventually, maybe): package, release as binary and on PyPI.


## Usage

`vlwconv <options> -s SIZE INPUT_PATH OUTPUT_PATH`

SIZE is the font size in pixels.
INPUT_PATH should be the path to a font file (e.g. ttf).
OUTPUT_PATH is the desired VLW file path/name.

### Other Options

- `-b`/`--block`: Specify named Unicode block (can combine multiple, use `-h` for list)
- `-r`/`--range`: Specify custom unicode (can combine multiple, see examples)
- `-c`/`--chars`: Include characters found in string
- `-t`/`--ttc-index`: TTC font files contain multiple styles. Use this to select one.

### Examples

- `vlwconv -b basic_latin -s 16 font.ttf font.vlw`: Create a font containing Basic Latin Unicode block (ASCII)
- `vlwconv -b basic_latin -b latin_1_supplement -s 16 font.ttf font.vlw`: Create a font containing Basic Latin and Latin-1 Supplement Unicode blocks
- `vlwconv -c "0123456789" -r U+0041-U+005A -r U+0061-007A -s 16 font.ttf font.vlw`: Create a mixed-case alphanumeric font (numbers specified by string, letters covered by ranges)


## License

MIT License.
Copyright 2023 Justin De La Rue
