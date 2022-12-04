#!/usr/bin/env python3

from pathlib import Path
import subprocess
import sys

testdir = Path(__file__).resolve().parent / 'test'
asmexec = Path(sys.argv[1])

for p in (testdir / 'smoketest').glob('*.s'):
    print(f'-> {p.name}')
    subprocess.run([str(asmexec), '-o', 'tmp.bin', str(p)])
