all:
	rm -rf builddir
	CC=clang-15 CXX=clang++-15 meson setup builddir
	ninja -C builddir/
	./testgen/rand_scalar.py --seed foo -n 1000 --constrain-regs 0:3 > scalar.s
	./builddir/asm/asm -o scalar.bin scalar.s
	./builddir/sim/sim --trace scalar.trace scalar.bin
