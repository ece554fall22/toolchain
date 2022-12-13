host:
		mkdir -p temp
		nasm -e mobilenet.txt -o temp/mobilenet.s
		../builddir/asm/asm -o temp/mobilenet.bin temp/mobilenet.s
		ninja -C builddir && ../builddir/host/host ./temp/mobilenet.bin 0x1000:a.bin 0x2000:c.bin

# ../builddir/host/host <code program> <progAddr1>:<prog1> <progAddrN>:<progN> <out_start_addr>:<out_length>:<out_file>

c2:
		mkdir -p temp
		nasm -e mobilenet.txt -o temp/conv_2d.s
		../builddir/asm/asm -o temp/conv_2d.bin temp/conv_2d.s
		../builddir/sim/sim temp/conv_2d.bin --trace temp/conv_2d.trace

s2:
		mkdir -p temp
		nasm -e depthwise_conv_s2.txt -o temp/depth_s2.s
		../builddir/asm/asm -o temp/depth_s2.bin temp/depth_s2.s
		../builddir/sim/sim temp/depth_s2.bin --trace temp/depth_s2.trace

s1:
		mkdir -p temp
		nasm -e depthwise_conv_s1.txt -o temp/depth_s1.s
		../builddir/asm/asm -o temp/depth_s1.bin temp/depth_s1.s
		../builddir/sim/sim temp/depth_s1.bin --trace temp/depth_s1.trace
		

		