from typing import Optional

class UnicodeRange:
    _begin: int
    _end: int
    _name: str
    
    def __init__(self, begin: int, end: int, name: str):
        if (begin < 0):
            raise ValueError("begin must not be negative", begin)
        if (end < begin):
            raise ValueError("end must not be smaller than begin", begin, end)
        
        self._begin = begin
        self._end = end
        self._name = name
    
    def __repr__(self) -> str:
        return "UnicodeRange(0x{:04x}, 0x{:04x}, \"{}\")".format(self._begin, self._end, self._name)
    
    # takes format like "0000-0010", "0x0000-0x0010", "U+0000-U+0010", "0000 0010", "0000..0010", etc.
    @staticmethod
    def from_hex_string(s: str):
        split_chars = ["-", " ", ".", ","]
        
        # find split_point_1 -- first separator char
        split_point_1: Optional[int] = None
        for i, c in enumerate(s):
            if c in split_chars:
                split_point_1 = i
                break
        if split_point_1 is None:
            raise ValueError("No range separator found", s, split_chars)
        
        # find split_point_2 -- first non-separator char after split_point_1
        split_point_2: Optional[int] = None
        for i, c in enumerate(s[split_point_1:]):
            if c not in split_chars:
                split_point_2 = split_point_1 + i
                break
        if split_point_2 is None:
            raise ValueError("Incomplete range (no end value)", s)
        
        left = s[:split_point_1].strip()
        right = s[split_point_2:].strip()
        for c in right:
            if c in split_chars:
                raise ValueError("Range contains excess separators", s)
        
        if left.startswith("U+"):
            left = left[2:]
        if right.startswith("U+"):
            right = right[2:]
        
        begin = int(left, 16)
        end = int(right, 16)
        
        return UnicodeRange(begin, end, s)
    
    @property
    def begin(self) -> int:
        return self._begin
    
    @property
    def end(self) -> int:
        return self._end
    
    @property
    def name(self) -> str:
        return self._name

# Unicode blocks from https://en.wikipedia.org/wiki/Unicode_block#List_of_blocks
# only BMP included because it's already a long list
UnicodeBlocks = [
    UnicodeRange(0x0000, 0x007F, "Basic Latin"),
    UnicodeRange(0x0080, 0x00FF, "Latin-1 Supplement"),
    UnicodeRange(0x0100, 0x017F, "Latin Extended-A"),
    UnicodeRange(0x0180, 0x024F, "Latin Extended-B"),
    UnicodeRange(0x0250, 0x02AF, "IPA Extensions"),
    UnicodeRange(0x02B0, 0x02FF, "Spacing Modifier Letters"),
    UnicodeRange(0x0300, 0x036F, "Combining Diacritical Marks"),
    UnicodeRange(0x0370, 0x03FF, "Greek and Coptic"),
    UnicodeRange(0x0400, 0x04FF, "Cyrillic"),
    UnicodeRange(0x0500, 0x052F, "Cyrillic Supplement"),
    UnicodeRange(0x0530, 0x058F, "Armenian"),
    UnicodeRange(0x0590, 0x05FF, "Hebrew"),
    UnicodeRange(0x0600, 0x06FF, "Arabic"),
    UnicodeRange(0x0700, 0x074F, "Syriac"),
    UnicodeRange(0x0750, 0x077F, "Arabic Supplement"),
    UnicodeRange(0x0780, 0x07BF, "Thaana"),
    UnicodeRange(0x07C0, 0x07FF, "NKo"),
    UnicodeRange(0x0800, 0x083F, "Samaritan"),
    UnicodeRange(0x0840, 0x085F, "Mandaic"),
    UnicodeRange(0x0860, 0x086F, "Syriac Supplement"),
    UnicodeRange(0x0870, 0x089F, "Arabic Extended-B"),
    UnicodeRange(0x08A0, 0x08FF, "Arabic Extended-A"),
    UnicodeRange(0x0900, 0x097F, "Devanagari"),
    UnicodeRange(0x0980, 0x09FF, "Bengali"),
    UnicodeRange(0x0A00, 0x0A7F, "Gurmukhi"),
    UnicodeRange(0x0A80, 0x0AFF, "Gujarati"),
    UnicodeRange(0x0B00, 0x0B7F, "Oriya"),
    UnicodeRange(0x0B80, 0x0BFF, "Tamil"),
    UnicodeRange(0x0C00, 0x0C7F, "Telugu"),
    UnicodeRange(0x0C80, 0x0CFF, "Kannada"),
    UnicodeRange(0x0D00, 0x0D7F, "Malayalam"),
    UnicodeRange(0x0D80, 0x0DFF, "Sinhala"),
    UnicodeRange(0x0E00, 0x0E7F, "Thai"),
    UnicodeRange(0x0E80, 0x0EFF, "Lao"),
    UnicodeRange(0x0F00, 0x0FFF, "Tibetan"),
    UnicodeRange(0x1000, 0x109F, "Myanmar"),
    UnicodeRange(0x10A0, 0x10FF, "Georgian"),
    UnicodeRange(0x1100, 0x11FF, "Hangul Jamo"),
    UnicodeRange(0x1200, 0x137F, "Ethiopic"),
    UnicodeRange(0x1380, 0x139F, "Ethiopic Supplement"),
    UnicodeRange(0x13A0, 0x13FF, "Cherokee"),
    UnicodeRange(0x1400, 0x167F, "Unified Canadian Aboriginal Syllabics"),
    UnicodeRange(0x1680, 0x169F, "Ogham"),
    UnicodeRange(0x16A0, 0x16FF, "Runic"),
    UnicodeRange(0x1700, 0x171F, "Tagalog"),
    UnicodeRange(0x1720, 0x173F, "Hanunoo"),
    UnicodeRange(0x1740, 0x175F, "Buhid"),
    UnicodeRange(0x1760, 0x177F, "Tagbanwa"),
    UnicodeRange(0x1780, 0x17FF, "Khmer"),
    UnicodeRange(0x1800, 0x18AF, "Mongolian"),
    UnicodeRange(0x18B0, 0x18FF, "Unified Canadian Aboriginal Syllabics Extended"),
    UnicodeRange(0x1900, 0x194F, "Limbu"),
    UnicodeRange(0x1950, 0x197F, "Tai Le"),
    UnicodeRange(0x1980, 0x19DF, "New Tai Lue"),
    UnicodeRange(0x19E0, 0x19FF, "Khmer Symbols"),
    UnicodeRange(0x1A00, 0x1A1F, "Buginese"),
    UnicodeRange(0x1A20, 0x1AAF, "Tai Tham"),
    UnicodeRange(0x1AB0, 0x1AFF, "Combining Diacritical Marks Extended"),
    UnicodeRange(0x1B00, 0x1B7F, "Balinese"),
    UnicodeRange(0x1B80, 0x1BBF, "Sundanese"),
    UnicodeRange(0x1BC0, 0x1BFF, "Batak"),
    UnicodeRange(0x1C00, 0x1C4F, "Lepcha"),
    UnicodeRange(0x1C50, 0x1C7F, "Ol Chiki"),
    UnicodeRange(0x1C80, 0x1C8F, "Cyrillic Extended-C"),
    UnicodeRange(0x1C90, 0x1CBF, "Georgian Extended"),
    UnicodeRange(0x1CC0, 0x1CCF, "Sundanese Supplement"),
    UnicodeRange(0x1CD0, 0x1CFF, "Vedic Extensions"),
    UnicodeRange(0x1D00, 0x1D7F, "Phonetic Extensions"),
    UnicodeRange(0x1D80, 0x1DBF, "Phonetic Extensions Supplement"),
    UnicodeRange(0x1DC0, 0x1DFF, "Combining Diacritical Marks Supplement"),
    UnicodeRange(0x1E00, 0x1EFF, "Latin Extended Additional"),
    UnicodeRange(0x1F00, 0x1FFF, "Greek Extended"),
    UnicodeRange(0x2000, 0x206F, "General Punctuation"),
    UnicodeRange(0x2070, 0x209F, "Superscripts and Subscripts"),
    UnicodeRange(0x20A0, 0x20CF, "Currency Symbols"),
    UnicodeRange(0x20D0, 0x20FF, "Combining Diacritical Marks for Symbols"),
    UnicodeRange(0x2100, 0x214F, "Letterlike Symbols"),
    UnicodeRange(0x2150, 0x218F, "Number Forms"),
    UnicodeRange(0x2190, 0x21FF, "Arrows"),
    UnicodeRange(0x2200, 0x22FF, "Mathematical Operators"),
    UnicodeRange(0x2300, 0x23FF, "Miscellaneous Technical"),
    UnicodeRange(0x2400, 0x243F, "Control Pictures"),
    UnicodeRange(0x2440, 0x245F, "Optical Character Recognition"),
    UnicodeRange(0x2460, 0x24FF, "Enclosed Alphanumerics"),
    UnicodeRange(0x2500, 0x257F, "Box Drawing"),
    UnicodeRange(0x2580, 0x259F, "Block Elements"),
    UnicodeRange(0x25A0, 0x25FF, "Geometric Shapes"),
    UnicodeRange(0x2600, 0x26FF, "Miscellaneous Symbols"),
    UnicodeRange(0x2700, 0x27BF, "Dingbats"),
    UnicodeRange(0x27C0, 0x27EF, "Miscellaneous Mathematical Symbols-A"),
    UnicodeRange(0x27F0, 0x27FF, "Supplemental Arrows-A"),
    UnicodeRange(0x2800, 0x28FF, "Braille Patterns"),
    UnicodeRange(0x2900, 0x297F, "Supplemental Arrows-B"),
    UnicodeRange(0x2980, 0x29FF, "Miscellaneous Mathematical Symbols-B"),
    UnicodeRange(0x2A00, 0x2AFF, "Supplemental Mathematical Operators"),
    UnicodeRange(0x2B00, 0x2BFF, "Miscellaneous Symbols and Arrows"),
    UnicodeRange(0x2C00, 0x2C5F, "Glagolitic"),
    UnicodeRange(0x2C60, 0x2C7F, "Latin Extended-C"),
    UnicodeRange(0x2C80, 0x2CFF, "Coptic"),
    UnicodeRange(0x2D00, 0x2D2F, "Georgian Supplement"),
    UnicodeRange(0x2D30, 0x2D7F, "Tifinagh"),
    UnicodeRange(0x2D80, 0x2DDF, "Ethiopic Extended"),
    UnicodeRange(0x2DE0, 0x2DFF, "Cyrillic Extended-A"),
    UnicodeRange(0x2E00, 0x2E7F, "Supplemental Punctuation"),
    UnicodeRange(0x2E80, 0x2EFF, "CJK Radicals Supplement"),
    UnicodeRange(0x2F00, 0x2FDF, "Kangxi Radicals"),
    UnicodeRange(0x2FF0, 0x2FFF, "Ideographic Description Characters"),
    UnicodeRange(0x3000, 0x303F, "CJK Symbols and Punctuation"),
    UnicodeRange(0x3040, 0x309F, "Hiragana"),
    UnicodeRange(0x30A0, 0x30FF, "Katakana"),
    UnicodeRange(0x3100, 0x312F, "Bopomofo"),
    UnicodeRange(0x3130, 0x318F, "Hangul Compatibility Jamo"),
    UnicodeRange(0x3190, 0x319F, "Kanbun"),
    UnicodeRange(0x31A0, 0x31BF, "Bopomofo Extended"),
    UnicodeRange(0x31C0, 0x31EF, "CJK Strokes"),
    UnicodeRange(0x31F0, 0x31FF, "Katakana Phonetic Extensions"),
    UnicodeRange(0x3200, 0x32FF, "Enclosed CJK Letters and Months"),
    UnicodeRange(0x3300, 0x33FF, "CJK Compatibility"),
    UnicodeRange(0x3400, 0x4DBF, "CJK Unified Ideographs Extension A"),
    UnicodeRange(0x4DC0, 0x4DFF, "Yijing Hexagram Symbols"),
    UnicodeRange(0x4E00, 0x9FFF, "CJK Unified Ideographs"),
    UnicodeRange(0xA000, 0xA48F, "Yi Syllables"),
    UnicodeRange(0xA490, 0xA4CF, "Yi Radicals"),
    UnicodeRange(0xA4D0, 0xA4FF, "Lisu"),
    UnicodeRange(0xA500, 0xA63F, "Vai"),
    UnicodeRange(0xA640, 0xA69F, "Cyrillic Extended-B"),
    UnicodeRange(0xA6A0, 0xA6FF, "Bamum"),
    UnicodeRange(0xA700, 0xA71F, "Modifier Tone Letters"),
    UnicodeRange(0xA720, 0xA7FF, "Latin Extended-D"),
    UnicodeRange(0xA800, 0xA82F, "Syloti Nagri"),
    UnicodeRange(0xA830, 0xA83F, "Common Indic Number Forms"),
    UnicodeRange(0xA840, 0xA87F, "Phags-pa"),
    UnicodeRange(0xA880, 0xA8DF, "Saurashtra"),
    UnicodeRange(0xA8E0, 0xA8FF, "Devanagari Extended"),
    UnicodeRange(0xA900, 0xA92F, "Kayah Li"),
    UnicodeRange(0xA930, 0xA95F, "Rejang"),
    UnicodeRange(0xA960, 0xA97F, "Hangul Jamo Extended-A"),
    UnicodeRange(0xA980, 0xA9DF, "Javanese"),
    UnicodeRange(0xA9E0, 0xA9FF, "Myanmar Extended-B"),
    UnicodeRange(0xAA00, 0xAA5F, "Cham"),
    UnicodeRange(0xAA60, 0xAA7F, "Myanmar Extended-A"),
    UnicodeRange(0xAA80, 0xAADF, "Tai Viet"),
    UnicodeRange(0xAAE0, 0xAAFF, "Meetei Mayek Extensions"),
    UnicodeRange(0xAB00, 0xAB2F, "Ethiopic Extended-A"),
    UnicodeRange(0xAB30, 0xAB6F, "Latin Extended-E"),
    UnicodeRange(0xAB70, 0xABBF, "Cherokee Supplement"),
    UnicodeRange(0xABC0, 0xABFF, "Meetei Mayek"),
    UnicodeRange(0xAC00, 0xD7AF, "Hangul Syllables"),
    UnicodeRange(0xD7B0, 0xD7FF, "Hangul Jamo Extended-B"),
    UnicodeRange(0xD800, 0xDB7F, "High Surrogates"),
    UnicodeRange(0xDB80, 0xDBFF, "High Private Use Surrogates"),
    UnicodeRange(0xDC00, 0xDFFF, "Low Surrogates"),
    UnicodeRange(0xE000, 0xF8FF, "Private Use Area"),
    UnicodeRange(0xF900, 0xFAFF, "CJK Compatibility Ideographs"),
    UnicodeRange(0xFB00, 0xFB4F, "Alphabetic Presentation Forms"),
    UnicodeRange(0xFB50, 0xFDFF, "Arabic Presentation Forms-A"),
    UnicodeRange(0xFE00, 0xFE0F, "Variation Selectors"),
    UnicodeRange(0xFE10, 0xFE1F, "Vertical Forms"),
    UnicodeRange(0xFE20, 0xFE2F, "Combining Half Marks"),
    UnicodeRange(0xFE30, 0xFE4F, "CJK Compatibility Forms"),
    UnicodeRange(0xFE50, 0xFE6F, "Small Form Variants"),
    UnicodeRange(0xFE70, 0xFEFF, "Arabic Presentation Forms-B"),
    UnicodeRange(0xFF00, 0xFFEF, "Halfwidth and Fullwidth Forms"),
    UnicodeRange(0xFFF0, 0xFFFF, "Specials")
]

# dictionary with lowercase, non-spaced names as keys
UnicodeBlocksDict = {r.name.lower().replace(" ", "_").replace("-", "_"): r for r in UnicodeBlocks}
