#!/usr/bin/env python3

from pathlib import Path
import subprocess
import sys

SCRIPTDIR = Path(__file__).resolve().parent
testdir = SCRIPTDIR / 'test'
outdir  = SCRIPTDIR.parent / 'builddir' / 'smoketest'
outdir.mkdir(exist_ok=True)
asmexec = Path(sys.argv[1])

for p in (testdir / 'smoketest').glob('*.s'):
    print(f'-> {p.name}')
    subprocess.run([str(asmexec), '-o', str(outdir / f'{p.stem}.bin'), str(p)])
