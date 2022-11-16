# simulator trace format

flow
```
{PC}: FLOW {opcode:b} |
```

scalar instructions
```
{PC}: SCALAR {opcode:b} | ({regno}={val})+ imm={imm}? | {} <- {} | M[{}] <- {} | {asm}
```

vector
```
{PC}: VECTOR {opcode:b} | {mask} ({regno}={val})+ imm={imm}? | {} <- ({}, {}, {}, {}) | M[{}] <- {} | {asm}
```

as an example:
code is
```
0x0000000:
addi r1, r2, 0xab
```


all integers implicitly hex
```
0000: SCALAR 13 | r1=0 r2=1 imm=ab | r1 <- ac | addi r1, r2, 0xab
```
