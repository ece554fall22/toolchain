# getting started working on the `morpheus toolchain` ✨
i (erin) am going to open this with a preface that it is currently 2 am and i have been writing for a history of science class since i woke up and i decided to take a break by doing more writing and this was probably a bad idea.

## building the code lol
you need
- modern gcc or clang (supports C++20).
- [ninja](https://ninja-build.org/).
- [meson](https://mesonbuild.com).
- python3.9 or later

i wouldn't recommend trying to build directly on windows because you would need to use mingw or fiddle with making msvc compile this. instead probably just use WSL. on CAE you can just install ninja and meson locally and use the system C++ compiler and python, like so:
```sh
python3 -m pip install --user --upgrade meson ninja
```
if you're using bash and the CAE default bashrc, you need to add `~/.local/bin` to your path to use things installed via pip. you can do that by adding the line
```bash
export PATH="$PATH:$HOME/.local/bin"
```
to the end of your `~/.bashrc`.

on MacOS, if you have [homebrew](https://brew.sh) you can do
```sh
brew install meson ninja python@3.10
```

on WSL it's ubuntu so you have an extremely old `gcc` and no clang. just grab it from llvm's repos:
```sh
wget https://apt.llvm.org/llvm.sh; chmod +x llvm.sh
sudo ./llvm.sh 15
```

once you have everything set up just do the following in the root of the repository:
```sh
meson setup builddir
# or specify specific compilers if you had to install clang or something
CC=clang-15 CXX=clang++-15 meson setup builddir
```
this sets up a build directory called `builddir/`. you can `cd` there and do stuff or just work from the root. i'll do the latter.

to build all targets,
```sh
ninja -C builddir/
```
(or just `ninja` if you're inside `builddir/`).

the assembler is then in `./builddir/asm/asm`. most of the time when im working i do something like (hits up arrow to get `ninja -C builddir && ./builddir/asm/asm asm/tests/sketch.s`) (hits enter)

## how is stuff structured
the assembler lives in `asm/`. the simulator is in `sim/`. common C++ code is in `libmorph`; note that this is where instruction decoding/encoding lives.

basically every component lives in its own directory if you look at how meson is set up, each of these is just a `subdir()`, and then the `meson.build` living in the toolchain component's directory chains everything into one build.

tests are in a subdirectory of each component, e.g. `sim/tests/`. we use [doctest](https://github.com/doctest/doctest) for unit testing; they have [a decent tutorial](https://github.com/doctest/doctest/blob/master/doc/markdown/tutorial.md) on how to write doctest tests. note that if you add a test you need to add it to that component's `meson.build`.

# oh yeah the assembler
it completely unnecessarily has a lex-parse frontend → codegen backend structure like a compiler. macros are implemented via AST grafting. inclusion (will probably) be done by AST grafting. etc.

i want to move the serdes of instructions to a `libisa` minilibrary so we can consume the encoder in the assembler/linker and the decoder in the simulator
