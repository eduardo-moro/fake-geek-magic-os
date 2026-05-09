from io import BufferedIOBase, RawIOBase
from typing import Union
import Glyph

AnyIOBase = Union[BufferedIOBase, RawIOBase]

class VlwFont:
    height: int # line height
    ascent: int # positive int, how high characters extend above baseline
    descent: int  # positive(?) int, how high characters extend below baseline
    glyphs: list[Glyph.Glyph] # inserter is responsible for maintaining sorted state
    name: str # name of font
    psname: str # postscript name of font (why?)
    aa: bool # glyphs are antialiased?
    
    # write VLW file to stream
    def write_stream(self, io: AnyIOBase):
        io.write(len(self.glyphs).to_bytes(4, byteorder="big", signed=True)) # glyphCount
        io.write((11).to_bytes(4, byteorder="big", signed=True))             # version
        io.write(self.height.to_bytes(4, byteorder="big", signed=True))      # size
        io.write((0).to_bytes(4, byteorder="big", signed=True))              # deprecated
        io.write(self.ascent.to_bytes(4, byteorder="big", signed=True))      # ascent
        io.write(self.descent.to_bytes(4, byteorder="big", signed=True))     # descent

        for g in self.glyphs:
            g.write_header(io)

        for g in self.glyphs:
            g.write_bitmap(io)
        
        # Bodmer got string encoding wrong.
        # Actual is 2-byte length, then non-terminated string)
        # ref: https://github.com/openjdk-mirror/jdk7u-jdk/blob/f4d80957e89a19a29bb9f9807d2a28351ed7f7df/src/share/classes/java/io/DataOutputStream.java#L346
        name_utf = self.name.encode("utf-8")
        io.write(len(name_utf).to_bytes(2, byteorder="big", signed=False))
        io.write(name_utf)
        
        psname_utf = self.psname.encode("utf-8")
        io.write(len(psname_utf).to_bytes(2, byteorder="big", signed=False))
        io.write(psname_utf)
        
        aa_int = 1 if self.aa else 0
        io.write(aa_int.to_bytes(1, byteorder="big", signed=False))
