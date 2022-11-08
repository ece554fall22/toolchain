;; special
halt
nop
;; jumps
jmp lbl0
jal lbl0
jmpr r0, #+0x10
jalr r0, #+0x10
;; cond branches
bnzi .branchTo
bezi .branchTo
blzi .branchTo
bgzi .branchTo
blei .branchTo
bgei .branchTo
;; cond branches (to register)
bnzr r0, #+0x10
bezr r0, #+0x10
blzr r0, #+0x10
bgzr r0, #+0x10
bler r0, #+0x10
bger r0, #+0x10
; `bnz` will pick bnzr/bnzi based on operands
;; immediate loads
lih r0, #0x3ffff
lil r0, #0x3ffff
;; memory movement
ld32 r1, [r2+0x10]
ld36 r1, [r2+0x10]
st32 r1, [r2+0x10]
st36 r1, [r2+0x10]
;; vector movement
; explicit mask
vldi 0b1011, v0, [r0+=0x30]
; better syntax
vldi {v0.0, v0.2, v0.3}, [r0+=0x30]
; register stride
vldi {v0.0, v0.2, v0.3}, [r0+=r1]
; assembler can pick the actual instruction 
vld {v0.0, v0.2, v0.3}, [r0+=0x30]
vld {v0.0, v0.2, v0.3}, [r0+=r1]

;; scalar integer arithmetic, immediate
addi r0, r1, #0x10
subi r0, r1, #0x10
andi r0, r1, #0x10
ori r0, r1, #0x10
xori r0, r1, #0x10
shli r0, r1, #0x10
shri r0, r1, #0x10
;; scalar integer arithmetic, register2register
add r0, r1, r2
sub r0, r1, r2
mul r0, r1, r2
and r0, r1, r2
or r0, r1, r2
xor r0, r1, r2
shr r0, r1, r2
shl r0, r1, r2
not r0, r1
;; scalar floating point arithmetic
fadd r0, r1, r2
fsub r0, r1, r2
fmul r0, r1, r2
fdiv r0, r1, r2
;; comparisons: immediate, register
cmpi r1, #0x10
cmp r1, r2

thisIsALabel:
this_is_also_a_label:
.soisthis_butitslocal:
