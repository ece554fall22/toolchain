# simulator trace format
```
rd  ra  rb
vd  va  vb
imm
vecmask
branchtype

rD: index of reg
*rD: value of reg
```

vector values are formatted as `({x},{y},{z},{w})`; ex: `(1.2,3.4,5.6,7.8)`

the trace is formed as a sequence of instructions. each instruction consists of an instruction header
with its PC and IR, followed by space-indented records, one per line. note that records may be omitted
if they are not relevant to an instruction.
each record is formed from a kind, then a colon, and then the contents of that record.
the order of records should always follow the given trace format, even if some are omitted.

in `inputs` records, only the relevant registers to an instruction's type are included
the order of the inputs record is always
```
    rD rA rB vD vA vB  imm
```

trace format:
```
*** {PC}: {instruction} ***
    inputs: rD={rD}={*rD} rA={rA}={*rA} rB={rB}={*rB} vD={vD}={*vD} vA={vA}={*vA} vB={vB}={*vB} imm={imm}
    vector_mask: {vector_mask:b}
    branch_condition_code: {conditioncode}
    control_flow: taken={is_taken} taken_addr={taken_addr} not_taken_addr={not_taken_addr}
    flag_writeback: {flags}
    scalar_writeback: r{writeback_reg} = {writeback_val}
    vector_writeback: v{writeback_reg} = {writeback_val}
    scalar_load: {read_addr} = {read_val}
    scalar_store: {write_addr} = {write_val}
    vector_load: {read_addr} = {read_val}
    vector_store: {write_addr} = {write_val}
    asm: // assembly here
```

if not specified, all integers are formatted as lowercase hexadecimal.

example for `add r1, r2, r3` at `pc=0x0`, with r1=0, r2=4, r3=7 before instruction execution:

```
*** 0: <instruction here> ***
    inputs: rD=1=0 rA=2=4 rB=3=7
    scalar_writeback: r1 = 11
    asm: add r1, r2, r3

```
