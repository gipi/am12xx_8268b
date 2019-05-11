本工具具有ReadFirmware和DestroyFirmware两种功能

ReadFirmware可读取Flash中的固件数据，与ReadFlash工具区别是不需要固件支持,但小机必须能启动U盘

DestroyFirmware可破坏Flash中的固件数据（主要为测试sdk recover功能设计）



1.在U盘模式下使用

2.按钮会在小机启动U盘后自动生成

2.可多次读取或破坏不同分区，需手动重启

3.StartSector和EndSector为所选项目所在的扇区数。

4.工具默认为ReadFirmware功能，每次“Read”后，会在工具目录下生成readflash_temp.bin，为所选扇区的flash内数据

6.使用DestroyFirmware功能时，需选中“DestroyFirmware”选项。每次“Destroy”后，会在工具目录下生成readflash_temp.bin和writeflash_temp.bin文件，分别为从Flash读出的数据和经过破坏后重新写入Flash的数据



