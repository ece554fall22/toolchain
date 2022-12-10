# using NASM for preprocessing

make sure you have NASM installed. it is an x86_64 assembler; we are only using it for its preprocessor. to install it, you can do `apt install nasm` on debianish systems, or `brew install nasm` on macos.

you probably want to also check out [NASM's macro docs](https://nasm.us/doc/nasmdoc4.html).

for this example, imagine we have several files in the same directory. `a.s` contains
```
%rep 24
    systolicstep
%endrep

%include b.s
```

`b.s` contains
```
%define FOO 5
addi r3, r4, 5
```

we will use `a.s` as the top-level file to be assembled. run
```
nasm -E a.s -o preprocessed.s
```

`-E` asks nasm to *only preprocess* `a.s`; `-o` tells it where to put the result. you can now simply assemble `preprocessed.s` as normal.

as always, i would recommend writing a makefile to automate this.
