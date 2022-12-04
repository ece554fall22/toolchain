;; preseed values for registers
lil r0, -0x11b43
lih r0, 0xa533
lil r1, -0x1f2c0
lih r1, -0x118d0
lil r2, 0x1b39e
lih r2, 0x87b5
lil r3, 0xcaa1
lih r3, 0x3e18
lil r4, 0x1e9ad
lih r4, -0xfc79
lil r5, 0x1bed8
lih r5, -0x1e560
lil r6, 0x6428
lih r6, -0x8887
lil r7, -0x13559
lih r7, 0x1ef02
lil r8, -0x1cd2b
lih r8, -0x16188
lil r9, 0x1816c
lih r9, 0x185e8
lil r10, 0xd24e
lih r10, 0xca91
lil r11, -0x2a9b
lih r11, -0x157dc
lil r12, -0x1e30d
lih r12, -0x17fa1
lil r13, 0x36f2
lih r13, 0x1473d
lil r14, 0x14583
lih r14, 0x10a0b
lil r15, 0x6ba2
lih r15, 0x1a39a
lil r16, 0xa9f2
lih r16, 0x1f61
lil r17, 0x1e4cf
lih r17, -0x9f06
lil r18, 0xcf30
lih r18, 0x1aefe
lil r19, -0x325c
lih r19, -0x307c
lil r20, -0x12634
lih r20, -0x1d3a0
lil r21, 0x6a4
lih r21, -0x85a1
lil r22, -0x6378
lih r22, -0x1f1fa
lil r23, -0x499a
lih r23, -0x1a5ef
lil r24, 0x84fe
lih r24, -0x15baa
lil r25, -0x148a0
lih r25, -0x2936
lil r26, -0x1b467
lih r26, 0xbcb3
lil r27, 0x1129f
lih r27, -0x9fd
lil r28, -0x1c1a8
lih r28, 0x1d4a6
lil r29, -0xfc77
lih r29, -0x1a2b2
lil r30, -0xc8b9
lih r30, 0x6bd5
lil r31, -0xfa98
lih r31, -0x6d4d
bkpt 0
;; random scalar arithmetic
xor  r27, r12, r5
shl  r24, r0, r17
and  r31, r7, r7
add  r29, r16, r6
not  r23, r8
add  r9, r23, r23
sub  r27, r28, r4
mul  r9, r19, r31
and  r29, r18, r18
xor  r17, r3, r19
mul  r14, r4, r26
sub  r26, r21, r28
mul  r19, r30, r8
or   r10, r30, r11
xor  r18, r18, r19
shr  r12, r10, r7
shl  r26, r19, r21
mul  r17, r3, r4
add  r19, r18, r31
or   r1, r26, r15
shl  r28, r31, r4
xor  r23, r25, r16
not  r7, r26
mul  r14, r18, r17
shr  r2, r6, r2
add  r10, r13, r4
bkpt 1
or   r13, r10, r24
sub  r18, r26, r16
sub  r30, r24, r15
not  r23, r28
or   r2, r14, r10
and  r13, r12, r9
not  r22, r19
xor  r17, r22, r12
mul  r14, r1, r31
shl  r25, r21, r20
or   r12, r22, r24
not  r19, r22
and  r14, r9, r11
shr  r6, r11, r17
shl  r6, r2, r19
mul  r1, r24, r8
xor  r30, r24, r17
and  r26, r31, r0
xor  r28, r8, r20
and  r0, r3, r13
shr  r12, r0, r24
shr  r18, r28, r10
mul  r9, r12, r12
shr  r26, r29, r7
mul  r19, r10, r9
not  r31, r0
mul  r15, r3, r5
add  r0, r23, r11
add  r19, r5, r8
mul  r10, r24, r21
not  r12, r9
or   r3, r20, r23
mul  r7, r27, r17
sub  r6, r12, r17
xor  r24, r0, r6
sub  r8, r18, r31
mul  r27, r26, r24
shl  r22, r15, r29
not  r21, r22
xor  r21, r8, r18
shl  r10, r1, r14
not  r15, r30
and  r25, r29, r0
and  r29, r31, r24
not  r29, r27
mul  r16, r16, r0
and  r31, r10, r18
or   r30, r15, r28
shl  r21, r29, r28
sub  r18, r15, r11
shr  r22, r10, r8
and  r14, r23, r18
sub  r10, r29, r6
not  r11, r17
mul  r21, r5, r31
sub  r1, r7, r14
mul  r27, r14, r13
shl  r8, r10, r19
sub  r17, r11, r29
not  r23, r31
add  r15, r20, r1
shl  r4, r8, r5
xor  r23, r13, r31
xor  r0, r5, r30
shl  r12, r24, r24
mul  r11, r18, r30
shr  r18, r9, r30
and  r1, r7, r29
not  r21, r25
shl  r31, r2, r2
add  r14, r3, r27
mul  r24, r14, r7
shl  r18, r26, r23
sub  r19, r22, r26
sub  r1, r5, r30
and  r31, r12, r3
xor  r31, r25, r9
shl  r4, r22, r27
not  r7, r12
or   r30, r13, r0
xor  r21, r8, r8
xor  r5, r0, r23
shr  r29, r14, r27
add  r24, r26, r11
sub  r9, r24, r23
mul  r26, r0, r22
not  r7, r7
shl  r4, r25, r17
shr  r29, r5, r20
mul  r25, r14, r1
xor  r12, r18, r31
sub  r6, r22, r3
not  r29, r11
sub  r8, r11, r3
sub  r28, r16, r22
not  r21, r19
shl  r25, r30, r21
shr  r18, r12, r24
shr  r29, r29, r7
shr  r21, r7, r4
and  r20, r20, r24
shr  r17, r5, r4
