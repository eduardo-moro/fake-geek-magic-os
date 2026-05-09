# vlwconv - converts fonts to Processing's VLW format

# Original implementation in Processing is here:
# https://github.com/processing/processing/blob/master/core/src/processing/core/PFont.java

# Bodmer has better format documentation here:
# https://github.com/Bodmer/TFT_eSPI/blob/master/Extensions/Smooth_font.cpp

# Bodmer's Processing script for converting fonts with useful options:
# https://github.com/Bodmer/TFT_eSPI/blob/master/Tools/Create_Smooth_Font/Create_font/Create_font.pde

import freetype
from copy import deepcopy
from os import path

from UnicodeRange import UnicodeRange, UnicodeBlocksDict
from Glyph import Glyph
from VlwFont import VlwFont

# Processing ignores the font face's ascener and descender metrics,
# measuring "d" and "p" characters instead.
# Use this to choose whether to trust the font face's metrics for them.
USE_FACE_ASCENDER_DESCENDER = True

# convert int to 26.6 fixed point fraction
def to_26_6(v: int) -> int:
    return v << 6

# convert 26.6 fixed point fraction to integer (rounded)
def from_26_6(v: int) -> int:
    return round(v / 64)

if __name__ == "__main__":
    def get_args():
        def unicode_blocks_list() -> str:
            out: str = "Available Unicode Blocks\n"
            out +=     "========================\n"
            for k, v in UnicodeBlocksDict.items():
                out += "  \"{}\": {} (U+{:04X}..U+{:04X})\n".format(k, v.name, v.begin, v.end)
            
            return out
        
        import argparse
        parser = argparse.ArgumentParser(
            prog = "vlwconv",
            description = "Converts fonts to Processing's VLW format without installing Processing.",
            epilog = unicode_blocks_list(),
            formatter_class = argparse.RawDescriptionHelpFormatter
        )
        
        parser.add_argument("-b", "--block", dest="BLOCKS", action="append",
            choices=UnicodeBlocksDict.keys(), metavar="BLOCK", default=[],
            help="Specify Unicode block to include in output (can use multiple times)"
        )
        parser.add_argument("-r", "--range", dest="RANGES", action="append",
            metavar="BEGIN-END", default=[],
            help="Specify custom hexadecimal character range to include in output (can use multiple times)"
        )
        parser.add_argument("-c", "--chars", dest="CHARS",
            default="",
            help="Include all chars from CHARS string in output"
        )
        parser.add_argument("-t", "--ttc-index", dest="TTC_INDEX",
            type=int, default=0,
            help="Index of desired face in TTC font."
        )
        parser.add_argument("-s", "--size", dest="SIZE",
            type=int, required=True,
            help="Font size."
        )
        parser.add_argument("INPUT_FILE",
            help="Outline font to use as source"
        )
        parser.add_argument("OUTPUT_FILE",
            help="VLW file to write"
        )
        
        return parser.parse_args()
    
    # return sorted list of all characters to generate from args
    # that is, all characters in specified blocks, ranges, chars
    # return value is a sorted list of codepoint integers with no duplicates
    def gen_charlist(args) -> list[int]:
        charset = set() # set can't have duplicates!
        
        for b in args.BLOCKS:
            ub = UnicodeBlocksDict[b]
            charset.update({i for i in range(ub.begin, ub.end + 1)})
        for r in args.RANGES:
            ur = UnicodeRange.from_hex_string(r)
            charset.update({i for i in range(ur.begin, ur.end + 1)})
        charset.update({ord(c) for c in args.CHARS})
        
        return sorted(list(charset))
    
    args = get_args()
    # print (args)
    
    input_file = args.INPUT_FILE
    if (not path.isfile(input_file)):
        raise Exception("Input file (\"{}\") does not exist.".format(input_file))
    
    output_file = args.OUTPUT_FILE
    if (path.exists(output_file)):
        raise Exception("Output file (\"{}\") already exists.".format(output_file))
    
    size = args.SIZE
    if (size <= 0):
        raise Exception("Font size must be greater than 0.")
    
    charlist = gen_charlist(args)
    if (len(charlist) == 0):
        raise Exception("No characters to generate. Make sure to specify blocks, ranges, or chars.")
    # print (charlist)
    
    ttc_index = args.TTC_INDEX
    
    vlw = VlwFont()
    
    
    # load freetype face
    # ==================
    
    face = freetype.Face(input_file, ttc_index)
    try:
        face.select_charmap(freetype.FT_ENCODING_UNICODE)
    except freetype.FT_Exception as e:
        if e.errcode == 0x06:
            raise Exception("Font doesn't support Unicode and cannot be used.")
        else:
            raise e
    
    # size (and many other metrics) are given as 26.6 fixed point fractions,
    # so get used to seeing `to_26_6` and `from_26_6`
    face.set_char_size(to_26_6(size)) # note: 72 dpi, so 1px == 1pt
    
    
    # get font metrics
    # ================
    
    vlw.height = from_26_6(face.size.height)
    print ("Height: {}".format(vlw.height))
    
    if USE_FACE_ASCENDER_DESCENDER:
        # trust font metrics - use size.ascender and size.descender
        vlw.ascent = from_26_6(face.size.ascender)
        vlw.descent = -from_26_6(face.size.descender) # VLW seems to expect positive val?
    else:
        # don't trust metrics - measure d and p chars for ascent and descent
        # (for strict compatibility with Processing)
        # on the rare occasion a font doesn't have 'd' or 'p' glyphs, this
        # should measure missing char glyph instead (which is probably fine)
        face.load_char('d')
        vlw.ascent = from_26_6(face.glyph.metrics.horiBearingY)
        face.load_char('p')
        vlw.descent = -(from_26_6(face.glyph.metrics.horiBearingY) - face.glyph.bitmap.rows) # VLW seems to expect positive val?
    
    print ("Ascent: {}".format(vlw.ascent))
    print ("Descent: {}".format(vlw.descent))
    
    
    # set other font info
    # ===================
    
    vlw.name = face.family_name.decode("ascii") # names are indeed just ascii
    if face.style_name: vlw.name += " " + face.style_name.decode("ascii")
    vlw.psname = face.postscript_name.decode("ascii")
    vlw.aa = True
    
    
    # get information for all glyphs in-memory
    # ========================================
    
    vlw.glyphs = []
    
    for c in charlist:
        idx = face.get_char_index(c)
        if idx == 0:
            print ("Font has no glyph for codepoint U+{:04X}.".format(c))
            continue # no point including missing chars
        
        # load_glyph with FT_LOAD_RENDER
        # bitmap is rendered in FT_RENDER_MODE_NORMAL mode (8-bit antialiased)
        face.load_glyph(idx, freetype.FT_LOAD_RENDER)
        
        # FT_Glyph_Metrics struct:
        # https://github.com/rougier/freetype-py/blob/51ee6e15e6d7b3a9ca0f5e96b11bfa8c07575c36/freetype/ft_structs.py#L353
        
        g = Glyph()
        g.codepoint = c
        g.bitmap_height = face.glyph.bitmap.rows
        g.bitmap_width = face.glyph.bitmap.width
        g.advance = from_26_6(face.glyph.metrics.horiAdvance)
        g.bearing_y = from_26_6(face.glyph.metrics.horiBearingY)
        g.bearing_x = from_26_6(face.glyph.metrics.horiBearingX)
        g.bitmap_buf = bytearray(face.glyph.bitmap.width * face.glyph.bitmap.rows)
        
        # pitch < 0 means bottom row comes first
        up_flow = face.glyph.bitmap.pitch < 0
        
        # copy bitmap data
        # just implement as a loop because then we get free stride change if necessary
        for y in range(0, face.glyph.bitmap.rows):
            if up_flow: src_row_off = (face.glyph.bitmap.rows-1 - y) * -face.glyph.bitmap.pitch
            else: src_row_off = y * face.glyph.bitmap.pitch
            dest_row_off = y * face.glyph.bitmap.width
            
            # for x in range(0, face.glyph.bitmap.width):
            #     src_idx = src_row_off + x
            #     dest_idx = dest_row_off + x
            #     g.bitmap_buf[dest_idx] = face.glyph.bitmap.buffer[src_idx]
            
            # replace entire slice at once for more speed
            src_row_end = src_row_off+face.glyph.bitmap.width
            dest_row_end = dest_row_off+face.glyph.bitmap.width
            g.bitmap_buf[dest_row_off:dest_row_end] = face.glyph.bitmap.buffer[src_row_off:src_row_end]
        
        vlw.glyphs.append(g)
        print ("Processed character U+{:04X}.".format(c))
        # print ("  adv: {}, bY: {}, bX: {}, w: {}, h: {}.".format(g.advance, g.bearing_y, g.bearing_x, g.bitmap_width, g.bitmap_height))
        # print (g.bitmap_string())
    
    with open(output_file, "wb") as f:
        vlw.write_stream(f)
