#!/usr/bin/env python3
"""Higher-quality wrapper around the stable PC compatibility visual generator.

The retail title frames are 1024x768. The previous 384px cap was visibly soft
when expanded to the Wii U's 640x480 4:3 viewport. Keep the proven encoder but
raise only title-background and title-glitch working resolution to 512x384.
The base generator still auto-reduces a texture if its uint16 RLE stream would
overflow, so this remains safe on real hardware.
"""
from pathlib import Path

base = Path(__file__).with_name("convert_pc_compat_visuals_base.py")
source = base.read_text(encoding="utf-8")
source = source.replace("max_dimension=384", "max_dimension=512", 1)
source = source.replace("max_size=(384, 288)", "max_size=(512, 384)", 1)
namespace = {
    "__name__": "__main__",
    "__file__": str(base),
}
exec(compile(source, str(base), "exec"), namespace, namespace)
