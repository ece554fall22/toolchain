#!/usr/bin/env python3

import click
import math
from typing import TextIO, Literal, Optional

Radix = Literal['hex', 'bin']
BASE: dict[Radix, int] = {'hex': 16, 'bin': 2}

def dump(f: TextIO, memory, width, address_radix: Radix = 'hex', data_radix: Radix = 'bin'):
    f.write(f'WIDTH = {width};\n')
    f.write(f'DEPTH = {width};\n')
    f.write(f'ADDRESS_RADIX = {address_radix.upper()};\n')
    f.write(f'DATA_RADIX = {data_radix.upper()};\n')
    f.write('CONTENT\nBEGIN\n\n')

    for addr, data in enumerate(memory):
        f.write(_format_int(addr, address_radix, maxval=len(memory)))
        f.write(' : ')
        f.write(_format_int(data, data_radix, maxval=(1<<width) - 1))
        f.write(';\n')

    f.write('\nEND;\n')

def _format_int(val: int, radix: Radix, maxval: Optional[int] = None) -> str:
    if radix == 'bin':
        s = f'{val:b}'
    elif radix == 'hex':
        s = f'{val:x}'

    if maxval is not None:
        maxwidth = math.ceil(math.log(maxval)/math.log(BASE[radix]))
        s = s.zfill(maxwidth)

    return s

@click.command()
@click.argument('input', type=click.File('rb'))
@click.option('-o', '--output', type=click.File('w'), default='-')
def cli(input, output):
    dump(output, [0xa]*128, 32)

if __name__ == '__main__':
    cli()
