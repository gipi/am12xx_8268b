在命令行方式下，执行 swf target.swf 即打开并开始播放target.swf文件，直接运行swf.exe将默认执行LOCAL\main.swf。
可选的参数有：
	-d : debug mode，显示执行的脚本中遇到的函数调用
	-e : echo mode   显示脚本执行到的所有语句
	-t : tree mode   生成dump_displaylist.txt文件，用于trace每一帧的元素树
	-m ：memory monitor 显示执行过程中的内存占用情况
	-s : step mode   单帧显示
按键方式：
	由于数码相框不支持鼠标，按Flash Lite的方式通过键盘模拟鼠标，初始位置在左上角。
	上下左右分别对应小键盘的8246，enter键为5或enter，退出键为escape。
	小键盘/ * -分别对应SD，CF，USB介质（SD\,CF\,UDISK\）
	
引擎还处于开发完善阶段，可能存在bug。如果发现错误，请根据错误打印信息及时沟通。

vvd路径：
模板：SD\vividshare\themes
文件：SD\vividshare\my_vvd
