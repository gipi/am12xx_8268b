9d:2c:64:78:35:2e
2e:35:78:64:2c:9d
d2:11:22:33:44:55
d3:11:22:33:44:55
d4:11:22:33:44:55

说明：mac地址的书写规范如下：
      1.每一个mac地址均由这种格式表示: XX:XX:XX:XX:XX:XX 。“X”为合法的16进制字符。每个mac地址由5个“:”分隔的6个数据组成。第一个
        “:”之前的16进制数据表示mac 地址的最高位字节的值，最后一个“:”之后的16进制数据表示mac 地址的最低位字节的值。每两个“:”之
        间的16进制数据则表示mac 地址中相应位置上的字节的值。每个字节的值均由两个16进制字符组成的数据表示。如需要将小机的mac地址配置成
        0x9d2c6478352e，则需要在保存mac地址的.txt文档中输入以下字符串： 
        9d:2c:64:78:35:2e
      2.每个保存mac地址的.txt文档均可以保存多个mac地址，每个mac地址均独占一行。每烧录一台小机会依次使用其中的一个mac地址直到所有的地
        址被使用完。已经被烧录过mac地址将不会被再再次使用。

在量产过程中烧写Mac addrs，需使用MassProduct界面的ADFU升级 
        
MassProduct ADFU升级加入烧录MAC地址说明:
    1.进入LSDK\sdk\bootloader\eeprom\scripts\目录 执行 make clean; sh am_sdk_config.sh AM_8251; make 三个命令 
    2.打开LSDK\case\images\fwimage_1213_8251.cfg,在里面寻找"FWSC=fwscf648.bin,0xA0018000,0,'F648'"这一行,找到后在下面新加一行:
      "FWSC=eeprom.bin,0xA0080000,0,'EROM'".注意没有双引号.
    3..将mac地址按照上面规范存到一个txt文档里面.文档的名字是mac_data.txt,把它和升级工具放在同一个目录.
    4.打开升级工具,对固件进行打包,然后切换到MassProduct标签,勾选WRITE_MAC按钮,如果要升级的板子是IIC的EEPROM,则勾选IIC.如果是SPI的EEPROM则勾选SPI.
    5.点击Download按钮,升级的时候就会把MAC地址写入到小机的EEPROM里面
    

MassProduct ADFU升级工具烧录MAC地址出错说明:
   1.升级工具的界面上会显示剩余的MAC地址和总共的MAC地址的个数,格式如下:Remain MAC amount/Total MAC amount: 10/11  
     如果显示0/11,则表示mac地址已经用完.
   2.升级的过程中会在工具的目录下生成一个以MassProductReport为前缀后面加上当前日期的文本文档.里面会有一些升级出错的信息,
     如果升级出错里面会有一些打印信息可以查看.    
       

在 System Running状态下, 处于USB Mass storage mode 去写 Mac Address 需使用EthernetTool界面


    