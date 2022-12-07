vadd 0b1111, v3, v2, v1
vsub 0b1111, v4, v3, v1

vsadd 0b1111, v5, v4, r0

vidx r1, v4, 3
vsplat 0b1111, v10, r1

; v5 <- v6.yxwz
vswizzle 0b1111, v5, v6, 1, 0, 3, 2

vreduce r4, v5, 0b1111

vdot r4, v5, v6
vdota r1, r2, v3, v4
