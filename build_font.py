#!/usr/bin/env python3
"""
Build UTF-8 compatible fonts for buddy app.
Includes Basic Latin + Latin-1 Supplement (for Portuguese/Spanish accents).
"""

import subprocess
from pathlib import Path

SCRIPT_DIR = Path(__file__).parent
VLWCONV = SCRIPT_DIR / "companion-cube-tools" / "vlwconv.py"
COMPANION_CUBE = Path("/home/moro/source/WIP/companion-cube")
NERD_FONT = "/usr/share/fonts/TTF/JetBrainsMonoNLNerdFont-SemiBold.ttf"

def build_font(font_size):
    """Build VLW font with Latin-1 support"""

    if not COMPANION_CUBE.exists():
        print(f"ERROR: companion-cube not found at {COMPANION_CUBE}")
        return False

    # Copy vlwconv tools if not present
    tools_dir = SCRIPT_DIR / "companion-cube-tools"
    if not tools_dir.exists():
        print(f"Setting up vlwconv tools...")
        subprocess.run(["cp", "-r", str(COMPANION_CUBE / "vlwconv"), str(tools_dir)])

    VLWCONV = tools_dir / "vlwconv.py"

    output_file = SCRIPT_DIR / "data" / f"UTF8-Latin1-{font_size}.vlw"
    output_file.parent.mkdir(parents=True, exist_ok=True)

    # Remove existing
    if output_file.exists():
        output_file.unlink()

    cmd = [
        "python3",
        str(VLWCONV),
        "-s", str(font_size),
        # All printable ASCII + Latin-1 with accents (ç, á, é, etc)
        "-r", "U+0020-U+007E",  # ASCII printable
        "-r", "U+00A0-U+00FF",  # Latin-1 Supplement
        str(NERD_FONT),
        str(output_file)
    ]

    print(f"Building {font_size}pt font with Latin-1 support...")
    print(f"Command: {' '.join(cmd)}\n")

    result = subprocess.run(cmd, capture_output=True, text=True)

    if result.returncode != 0:
        print(f"ERROR: vlwconv failed:")
        print(result.stderr)
        return False

    print(result.stdout)
    file_size_kb = output_file.stat().st_size / 1024
    print(f"✓ Font created: {output_file}")
    print(f"  File size: {file_size_kb:.1f} KB\n")

    return True

if __name__ == "__main__":
    print("Building UTF-8 fonts for buddy app...\n")

    all_ok = True
    for size in [16, 24, 32]:
        if not build_font(size):
            all_ok = False

    if all_ok:
        print("✓ All fonts built successfully!")
        print("\nNext: pio run -e esp32 -t uploadfs  # Upload to device")
    else:
        print("✗ Some fonts failed to build")
        exit(1)
