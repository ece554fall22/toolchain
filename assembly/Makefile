stride_one:
		mkdir -p temp
		nasm -e depthwise_conv_s1.txt -o temp/depth_s1.s
		../builddir/asm/asm -o temp/depth.bin temp/depth_s1.s
		../builddir/sim/sim temp/depth.bin --trace temp/depth_s1.trace