;; special
halt
thisIsALabel:

nop
;; jumps

add r0, r1, r2

bgzi .branchTo

lih r0, 0x3ffff
lil r0, 0x3ffff

ld32 r1, [r2+0x10]

blah [foo]
