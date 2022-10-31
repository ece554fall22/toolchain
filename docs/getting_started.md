# getting started working on the `morpheus toolchain` ✨
i (erin) am going to open this with a preface that it is currently 2 am and i have been writing for a history of science class since i woke up and i decided to take a break by doing more writing and this was probably a bad idea.

## building the code lol
you need
- modern gcc or clang (supports C++20).
- [ninja](https://ninja-build.org/).
- [meson](https://mesonbuild.com).
- python3.9

i wouldn't recommend trying to build directly on windows because you would need to use mingw or fiddle with making msvc compile this. instead probably just use WSL. on CAE you can just install ninja and meson locally and use the system C++ compiler and python, like so:
```sh
python3.9 -m pip install --user --upgrade meson ninja
```

once you have everything set up just do the following in the root of the repository:
```sh
$ meson setup builddir
```
this sets up a build directory called `builddir/`. you can `cd` there and do stuff or just work from the root. i'll do the latter.

to build all targets,
```sh
$ ninja -C builddir/
```
(or just `ninja` if you're inside `builddir/`).

the assembler is then in ``./builddir/asm/asm`. most of the time when im working i do something like (hits up arrow to get `ninja -C builddir && ./builddir/asm/asm asm/tests/sketch.s`) (hits enter)
