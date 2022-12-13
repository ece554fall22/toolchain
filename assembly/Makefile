host:
		mkdir -p temp
		nasm -e mobilenet.txt -o temp/mobilenet.s
		../builddir/asm/asm -o temp/mobilenet.bin temp/mobilenet.s
		ninja -C ../builddir && ../builddir/host/host --mem-size 20971520 ./temp/mobilenet.bin 0x40000:./binary_files/image.bin 0xc00000:./binary_files/weights_conv2d.bin -o 0x40000:0x62000:./temp/output.bin

# ../builddir/host/host --mem-size 20971520 <code program> <progAddr1>:<prog1> <progAddrN>:<progN> -o <out_start_addr>:<out_length>:<out_file>

test:
		mkdir -p temp
		nasm -e bootloader.txt -o temp/bootloader.s
		../builddir/asm/asm -o temp/bootloader.bin temp/bootloader.s
		ninja -C ../builddir && ../builddir/host/host ./temp/bootloader.bin 0x2000:./binary_files/weights_conv2d.bin -o 0x2000:0x02:./temp/test.out

# For Allison :)
boot:
		mkdir -p temp
		nasm -e bootloader.txt -o temp/bootloader.s
		../builddir/asm/asm -o temp/bootloader.bin temp/bootloader.s

c2:
		mkdir -p temp
		nasm -e mobilenet_test.txt -o temp/conv_2d.s
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
		

		