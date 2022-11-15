#!/usr/bin/env python3

import click
import math
from typing import TextIO, Literal, Optional

Radix = Literal['hex', 'bin']
BASE: dict[Radix, int] = {'hex': 16, 'bin': 2}

def dump(f: TextIO, memory, width, address_radix: Radix = 'hex', data_radix: Radix = 'bin'):
    f.write(f'WIDTH = {width};\n')
    f.write(f'DEPTH = {len(memory)};\n')
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
@click.argument('infile', type=click.File('rb'))
@click.option('-o', '--output', type=click.File('w'), default='-')
@click.option('-w', '--wordsize', default=32, help='word width. must be a multiple of 8')
@click.option('-e', '--endianness', type=click.Choice(['little', 'big']), default='little', help='endianness of source file')
@click.option('-a', '--address-radix', type=click.Choice(['hex', 'bin']), default='hex', help='radix of addresses in mif output')
@click.option('-d', '--data-radix', type=click.Choice(['hex', 'bin']), default='bin', help='radix of data in mif output')
def cli(infile, output, wordsize, endianness, address_radix, data_radix):
    if wordsize % 8 != 0:
        click.echo(f'word size {w} is not a multiple of 8 bits', err=True)
        exit(-1)

    nbytes = wordsize//8

    data = []
    while word := infile.read(nbytes):
        word = int.from_bytes(word, endianness)
        data.append(word)

    dump(output, data, wordsize, address_radix, data_radix)

if __name__ == '__main__':
    cli()
