lil r1, 0x30

vldi 0b1111, v0, [r1+=0x10]
vsti 0b1111, [r1+=0x10], v0

vldr 0b1111, v0, [r1+=r2]
vstr 0b1111, [r1+=r2], v0
