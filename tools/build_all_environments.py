#!/usr/bin/env python3
"""Build every configured environment, including the official firmware variants."""
import configparser
import subprocess

config = configparser.ConfigParser(interpolation=None)
config.read('platformio.ini')
environments = [section.removeprefix('env:') for section in config.sections() if section.startswith('env:')]
args = ['pio', 'run']
for environment in environments:
    args.extend(['-e', environment])
subprocess.run(args, check=True)
