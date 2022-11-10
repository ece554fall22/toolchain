#section text

#for i in 0..31
    xor r%i, r%i, r%i
#endfor

#macros add2(reg a)
    add a, a, #0x02
#endmacro

    add2! r3
