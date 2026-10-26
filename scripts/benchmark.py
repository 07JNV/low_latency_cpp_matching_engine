#!/usr/bin/env python3
import subprocess

counts = [10_000, 100_000, 500_000]
for count in counts:
    print(f"\n=== {count:,} commands ===")
    subprocess.run(["./build/matching_engine", "--orders", str(count)], check=True)
