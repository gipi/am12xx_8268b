1.确认make menconfig 已经选择MMC SD 
   Device drivers-->MMC/SD/Card support 选择<M>
   进入此选项
   确认以下选项OK
    <M> MMC block device drivers
	<M> Action AM7x Card Reader MMC/SD function support
2.输入命令make
3.输出文件:
   drivers/mmc/core/mmc_core.ko
   drivers/mmc/card/mmc_block.ko
   drivers//mmc/host/am7x_sd.ko
4.运行linux-tmp/Copy-mods.sh ，此脚本会把上述三个文件copy 
   initrd/lib/modules
   rootfs/lib/modules
5.生成相应镜像文件。
6.在终端顺序insmod .ko 
 insmod /lib/modules/mmc_core.ko
 insmod /lib/modules/mmc_block.ko
 insmod /lib/modules/am7x_sd.ko
7.插入SD/MMC ，SD drivers会自动初始化
 可以mount /dev/mmc /mnt/
 cd /mnt 
 ls -l
 进可以看到SD卡的内容