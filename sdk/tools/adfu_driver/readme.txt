目前驱动分为两份： x86 32bit 和 amd 64bit。
分别在 xp 32bit, vista 32bit, win7 32bit, win7 64bit 测试安装ok。

Q1: 如何解决驱动安装后提示服务名错误？
A1: 解决方法：
   1. 删除C:\windows\inf\oem*.inf 相应的驱动信息文件。可采用查找文件内容("vid_1de1&pid_1205")方式找到该文件。
   2. 删除注册表信息。
      a. 硬件子键。将 HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\USB 下的子项 Vid_1de1&Pid_1205 删除。（注意需要先修改其权限）
      b. 类子键。 它位于HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class 的目录下，{36FC9E60-C465-11CF-8056-444553540000} 这个是针对usb的GUID. 
         所以需要在该子项下在找到相应的含有"MathchingDeviedId = usb\vid_1de1&pid_1205"等信息的子项（如命名为：0000，0001, ....)，然后删除整个子项即可。
      c. 服务子键.   它位于HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services目录下。注意：老驱动命名为：adfuud， 最新驱动命名为：AdfuSrv。 删除此子项即可。
   3. 重启动电脑，重新安装驱动即可。

Q2: win7 64bit os 安装驱动后，提示需要数字认证，否则不能正常加载驱动。
A2: WIN7 X64系统中对驱动程序要求有数字签名，否则无法正常使用。但有时需要用到没有数字签名的驱动程序，可正常安装后驱动是无法使用的，这时我们只有在开机时按F8用“禁用驱动程序签名强制”模式进入系统后驱动使用才正常。
    目前该问题还没有完美的解决方案，只有一个临时性的解决方法：
    临时解决方法：运行，输入：bcdedit/set testsigning on   回车
    然后重启
    就可以关闭强制数字签名
    但是启动到桌面后会有提示水印，不过在使用上已经没有实质上的不便了。