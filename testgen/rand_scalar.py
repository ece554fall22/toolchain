#/usr/bin/env python3

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

    ops = ('add', 'sub', 'mul', 'and', 'or', 'xor', 'shr', 'shl', 'not')
    rng = Random(seed)

    # seed in random values for all registers
    print(';; preseed values for registers')
    for r in ALLREGS:
        lo = rands(rng, 18)
        hi = rands(rng, 18)
        print(f'lil r{r}, {lo:#x}')
        print(f'lih r{r}, {hi:#x}')

    # then generate `instr_count` random ops
    print(';; random scalar arithmetic')
    for _ in range(instr_count):
        op = rng.choice(ops)
        if op == 'not':
            rD = rng.choice(regset)
            rA = rng.choice(regset)
            print(f'{op:4} r{rD}, r{rA}')
        else:
            rD = rng.choice(regset)
            rA = rng.choice(regset)
            rB = rng.choice(regset)
            print(f'{op:4} r{rD}, r{rA}, r{rB}')

if __name__ == '__main__':
    cli()
