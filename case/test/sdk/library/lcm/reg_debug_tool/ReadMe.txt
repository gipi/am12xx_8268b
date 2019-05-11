先将debug.app放在nand或U盘上。
读物理内存：
./debug.app rm 0xb0040000
写物理内存：
./debug.app mm 0xb00f0000 0x6