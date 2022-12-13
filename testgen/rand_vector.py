#!/usr/bin/env python3

import click
from random import Random

ALLREGS = range(0, 32)

def rands(rng, bits):
    val = rng.randrange(1<<bits)
    if val > (1<<(bits-1)):
        val -= 1 << bits

    return val


@click.command()
@click.option('-n', '--instr-count', type=int, default=128)
@click.option('--constrain-regs')
@click.option('-s', '--seed')
def cli(instr_count, constrain_regs, seed):
    if not constrain_regs:
        regset = ALLREGS
    else:
        if ':' in constrain_regs:
            b, t = constrain_regs.strip().split(':')
            regset = range(int(b), int(t))
        else:
            regset = [int(r) for r in constrain_regs.strip().split(',')]

# vldi, vsti, vldr, vstr, Vadd, Vsub, Vmult, Vdiv, Vdot, Vdota, Vindx, Vreduce,
# Vsplat, Vswizzle, Vsadd, Vsmult, Vssub, Vsdiv, vsma, Vmax, Vmin
    ops = ('vsadd', 'vssub', 'vsmul', 'vsdiv')
    rng = Random(seed)


    # seed in random values for all registers
    print(';; preseed values for registers')
    for r in ALLREGS:
        lo = rands(rng, 18)
        hi = rands(rng, 18)
        print(f'lil r{r}, {lo:#x}')
        print(f'lih r{r}, {hi:#x}')

    # seed in random values for all registers
    print(';; preseed values for registers')
    for r in ALLREGS:
        print(f'vsplat v{r}, r{r}, 0xF')

    # then generate `instr_count` random ops
    print(';; random scalar arithmetic')
    for _ in range(instr_count):
        op = rng.choice(ops)
            vD = rng.choice(regset)
            vA = rng.choice(regset)
            vB = rng.choice(regset)
            imm = rands(rng, 4)
            print(f'{op:4} v{vD}, v{vA}, {imm:#x}')

if __name__ == '__main__':
    cli()
