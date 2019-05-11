作用：测试hantro_miso.so 中的jpeg_decode功能

编译：make -C ../case/test/sdk/drivers/usb/test_hantro_misc

目标文件：test_mjpeg_dec.app

使用示例：test_mjpeg_dec.app test file path  
	   test_mjpeg_dec.app /mnt/usb/1.jpg

note: 此用例非测试jpg文件解码，解码的输入为mjpeg的帧数据。
by liucan @2010.1.10
