#!/usr/bin/env python3
"""Check the German DFPlayer system folders, catalog and firmware track IDs."""
from collections import defaultdict
from pathlib import Path
import re
import sys

def validate(root):
    errors = []
    tracks = {}
    for folder in ('mp3', 'advert'):
        by_number = defaultdict(list)
        for file in (root / 'sd-card' / folder).glob('*.mp3'):
            match = re.match(r'^(\d{4})(?:[_.]|$)', file.name)
            if not match:
                errors.append(f'Invalid filename: {file}')
                continue
            by_number[int(match[1])].append(file.name)
            if file.stat().st_size == 0:
                errors.append(f'Empty audio: {file}')
        for number, files in by_number.items():
            if len(files) > 1:
                errors.append(f'{folder}/{number:04d}: duplicate files {files}')
        tracks[folder] = by_number

    catalog = {}
    for line in (root / 'audio_messages_de.txt').read_text(encoding='utf-8').splitlines():
        if '|' not in line:
            continue
        name, _ = line.split('|', 1)
        key = (name.split('/')[0], int(name.split('/')[1][:4]))
        # Official Stopptanz sources are three speech fragments for one final track.
        freeze_parts = {f'advert/0301_freeze_freeze{i}.mp3' for i in (1, 2, 3)}
        if key in catalog and not (name in freeze_parts and catalog[key] in freeze_parts):
            errors.append(f'Duplicate catalog ID: {name} and {catalog[key]}')
        catalog[key] = name
        # DFPlayer addresses the four-digit prefix, independent of the suffix.
        if key[1] not in tracks.get(key[0], {}):
            errors.append(f'Catalog track missing: {name}')

    header = (root / 'src' / 'mp3.hpp').read_text(encoding='utf-8')
    for enum, folder in (('mp3Tracks', 'mp3'), ('advertTracks', 'advert')):
        block = re.search(r'enum class ' + enum + r'.*?\{(.*?)\};', header, re.S)[1]
        assigned = {}
        for name, value in re.findall(r'^\s*(t_\w+)\s*=\s*(\d+)', block, re.M):
            number = int(value)
            if number in assigned:
                errors.append(f'{enum}: {name} collides with {assigned[number]}')
            assigned[number] = name
            if number and number not in tracks[folder]:
                errors.append(f'{enum}: no {folder}/{number:04d} audio for {name}')
    return errors, sum(len(v) for v in tracks.values()), len(catalog)

if __name__ == '__main__':
    root = Path(__file__).resolve().parents[1]
    errors, files, messages = validate(root)
    if errors:
        print('\n'.join(errors), file=sys.stderr)
        sys.exit(1)
    print(f'Audio OK: {files} unique track IDs, {messages} German catalog messages.')
