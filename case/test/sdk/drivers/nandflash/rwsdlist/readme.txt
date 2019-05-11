测试内容：
从卡里读4个文件写入到flash中，测试写入到flash是否正确
4个文件分写都是写100次，读100次
卡里面总共有4个test.bin文件，放在bin文件夹中 
详细信息如下：
filename        filesize(byte)    checksum
test1.bin       0x9b06a           0x4ddff0e
test2.bin       0x111d5f          0x891c8c9
test3.bin       0x1dea24          0xf07a243
test4.bin       0xde0ff6          0x70149686