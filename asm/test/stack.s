; we pretend by convention that our stack pointer is r0
; and stack grows down (from high to low addresses)

; our link register is r31

; probably will implement aliases
; - sp => r30
; - lr => r31

; low <------------ high
;            v      initial sp
;         v         after prologue
;         |         r0 + 0x0
;          |        r0 + 0x4
;            v      after epilogue

someFn:
    ;;;; prologue
    addi sp, sp, -0x10 ; enough for 2 x u36

    ; save link register (return address); u36
    st36 [sp+0x8], lr

    ; save r1 to stack
    ; pretending it's a callee-saved register
    st36 [sp+0x0], r1

    ;;;; ... function body that uses r1 and calls other functions ...

    ;;;; epilogue
    ; restore r1
    ld36 r1, [sp+0x0]

    ; restore lr
    ld36 lr, [sp+0x8]

    ; free the stack frame
    addi sp, sp, +0x10

    ; return to caller
    jmpr lr

main:
    ; imagine we set sp to 0x100000

    ;; call someFn, assuming that it's within range of PC
    jal someFn
