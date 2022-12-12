
c2:
		mkdir -p temp
		nasm -e conv_2d.txt -o temp/conv_2d.s
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
		
host:
		mkdir -p temp
		nasm -e bootloader.txt -o temp/bootloader.s
		../builddir/asm/asm -o temp/bootloader.bin temp/bootloader.s
		