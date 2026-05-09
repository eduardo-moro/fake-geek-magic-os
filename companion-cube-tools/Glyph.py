from io import BufferedIOBase, RawIOBase
from typing import Union

AnyIOBase = Union[BufferedIOBase, RawIOBase]

class Glyph:
    codepoint: int # codepoint of glyph
    bitmap_height: int # height of bitmap
    bitmap_width: int # width of bitmap
    advance: int # cursor advance amount
    bearing_y: int # y offset to draw glyph at (may be negative)
    bearing_x: int # x offset to draw glyph at (may be negative)
    bitmap_buf: bytearray # ensure stride == width on data written here
    
    def bitmap_string(self) -> str:
        out: str = ""
        
        for y in range(0, self.bitmap_height):
            for x in range(0, self.bitmap_width):
                v = self.bitmap_buf[y*self.bitmap_width + x]
                out += "#" if v > 191 else "+" if v > 127 else ":" if v > 63 else " "
            out += "\n"
        
        return out
    
    # write glyph header to stream
    def write_header(self, io: AnyIOBase):
        io.write(self.codepoint.to_bytes(4, byteorder="big", signed=True))     # codepoint
        io.write(self.bitmap_height.to_bytes(4, byteorder="big", signed=True)) # height
        io.write(self.bitmap_width.to_bytes(4, byteorder="big", signed=True))  # width
        io.write(self.advance.to_bytes(4, byteorder="big", signed=True))       # advance
        io.write(self.bearing_y.to_bytes(4, byteorder="big", signed=True))     # top bearing
        io.write(self.bearing_x.to_bytes(4, byteorder="big", signed=True))     # left bearing
        io.write((0).to_bytes(4, byteorder="big", signed=True))                # padding
    
    # write glyph bitmap to stream
    def write_bitmap(self, io: AnyIOBase):
        io.write(self.bitmap_buf)
