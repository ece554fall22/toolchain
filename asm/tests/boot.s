%define CSR_COREID 0x2

%section text
%org 0x000000000
    ; wipe out previous code if we're restarting
    ; note: never remove this; if we do, icache could get stale.
    flushicache
    ; can zero registers, etc here if necessary.

    ; determine where we should start
    rcsr r1, CSR_COREID
    ; compute offset into jump table
    lda r2, entrypointTable
    add r2, r2, r1
    ; find entrypoint and go
    ld36 r1, [r2]
    jmpr r1

    ; jump table in the text section? yeah, it doesn't matter, we jump out before it
entrypointTable: ; maps processor id to entrypoint address
    dw36 0x555555555 ; cpu0: entrypoint A
    dw36 0x555555555 ; cpu1: entrypoint A
    dw36 0xaaaaaaaaa ; cpu2: entrypoint B
    dw36 0xaaaaaaaaa ; cpu3: entrypoint B
