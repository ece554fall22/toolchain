# randomized testcase/trace generation

## `testgen/rand_scalar.py`

`rand_scalar.py` outputs assembly code which exercises the scalar pipeline. it takes some options:
- `-s`, `--seed SEED`: produce a predictable random output given the `SEED`
- `-n`, `--instr-count N`: output N scalar arithmetic instructions
- `--constrain-regs CONSTRAINT`: you can pass a list of registers to use in the operations, either like `1,2,3` or as a range of registers like `1:5` (meaning `1,2,3,4`). this is useful for creating lots of hazards to exercise the forwarding/stalling logic.

it dumps its assembly output on stdout, so just run it like
```sh
./testgen/rand_scalar.py --seed foo -n 100 --constrain-regs 0:3 > scalar-foo.s
```

then you can assemble and simulate it to generate a trace `scalar-foo.trace`:
```sh
./builddir/asm/asm -o scalar-foo.bin scalar-foo.s
./builddir/sim/sim --trace scalar-foo.trace scalar-foo.bin
```
