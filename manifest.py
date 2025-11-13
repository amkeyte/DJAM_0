#!/usr/bin/env python3
"""
generate_manifest.py

Scans a folder and writes a simple manifest.txt containing
one absolute URL per line.

Usage:
    python generate_manifest.py [folder] [base_url] [output_file]

Example:
    python generate_manifest.py ./source https://raw.githubusercontent.com/amkeyte/DJAM_0/master/source ./manifest.txt
"""

import os
import sys

def generate_manifest(target_folder, base_url, output_file):
    entries = []

    for root, _, files in os.walk(target_folder):
        for f in files:
            rel_path = os.path.relpath(os.path.join(root, f), target_folder).replace("\\", "/")
            abs_url = f"{base_url.rstrip('/')}/{rel_path}"
            entries.append(abs_url)

    entries.sort(key=str.lower)

    out_dir = os.path.dirname(os.path.abspath(output_file))
    if out_dir and not os.path.exists(out_dir):
        os.makedirs(out_dir, exist_ok=True)

    with open(output_file, "w", encoding="utf-8") as mf:
        for url in entries:
            mf.write(url + "\n")

    print(f"✅ Manifest written to: {os.path.abspath(output_file)}")
    print(f"   {len(entries)} entries")

if __name__ == "__main__":
    folder = sys.argv[1] if len(sys.argv) > 1 else "."
    base_url = sys.argv[2] if len(sys.argv) > 2 else "https://example.com"
    output_file = sys.argv[3] if len(sys.argv) > 3 else "manifest.txt"

    generate_manifest(folder, base_url, output_file)
