lil r0, 0x10
jalr r0, 0x4  ; <- 0x4
nop           ; <- 0x8
nop           ; <- 0x12
nop           ; <- 0x16
tgt: nop      ; <- 0x20
bkpt 0xa ; r31 should be 0x4
