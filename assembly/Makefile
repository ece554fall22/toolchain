stride_one:
		mkdir -p temp
		nasm -e depthwise_conv_s1.txt -o temp/depth_s1.s
		../builddir/asm/asm -o temp/depth_s1.bin temp/depth_s1.s
		../builddir/sim/sim temp/depth_s1.bin --trace temp/depth_s1.trace
		
stride_two:
		mkdir -p temp
		nasm -e depthwise_conv_s2.txt -o temp/depth_s2.s
		../builddir/asm/asm -o temp/depth_s2.bin temp/depth_s2.s
		../builddir/sim/sim temp/depth_s2.bin --trace temp/depth_s2.trace