   对于Flash_DebugTool 主要方便验证Flash 相关功能，同时也测试boot loader 相关功能。

以破坏boot loader reparit 为例：
1. 选择FireWare固件存入在Debug_FW, 如AM7555_ACM_11_Debug.bin
2. 选择file要保存文件名，
3. 在device 进入ADFU 
4. 在CMD list ,
  A. 选择erase_phy_block cmd, 
  B. 填写block 相应编辑框，可以填写0至7,分别代表四份mbrec和Brec
  C. 对于sector 填1.
  D. 点击"DoCmd" ，block number Erase,破坏mbrec, Brec.
  E. 可以通过“read_root_block”，验证是否上一步erase 成功。
输出框，看到数据全部是0xFF，则表示擦除成功。
  F. 循环B至E频率，破坏其它block number .
  