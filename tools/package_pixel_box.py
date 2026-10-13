#!/usr/bin/env python3
"""Package the validated German system prompts for the SD-card root."""
from pathlib import Path
import sys
import zipfile

from validate_pixel_box_audio import validate

root = Path(__file__).resolve().parents[1]
errors, _, _ = validate(root)
if errors:
    print('\n'.join(errors), file=sys.stderr)
    sys.exit(1)
output = root / 'build' / 'pixelbox_sdcard.zip'
output.parent.mkdir(exist_ok=True)
with zipfile.ZipFile(output, 'w', zipfile.ZIP_DEFLATED) as archive:
    for file in sorted((root / 'sd-card').rglob('*.mp3')):
        archive.write(file, file.relative_to(root / 'sd-card').as_posix())
print(output)
