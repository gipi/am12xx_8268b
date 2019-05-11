1. 配置Card各项说明

//*****************************************************//
A.  CardType： 代表支持什么类型，如SD,MMC,MS,XD,CF
  0x00  :Disable SD,MS,XD ,代码强制Disable SD/XD detect pin
  0x01  :支持SD/MMC,MS 不支持XD 
  0x02  :支持SD/MMC, 不支持MS,XD 
  0x03  :支持SD/MMC,MS,XD：

//*****************************************************//
B. CFCardtype: 代表支持CF 相关类型
  0x00  :Disable CF 功能，强制Disable CF Detect pin
  0x01  : 支持16Bit Data Bus,
  0x02  : 只支持8Bit Data Bus, 
 
//*****************************************************//
C. SDMultipin: 代表系统SD卡multi pin 复用关系 
   0x00 : 代表SD data Bus 不与nand flash data Bus 复用
   0x01 : 代表SD data Bus 与nand flash data Bus 复用 
   0x02 : Reserved

//*****************************************************//
D. MSMultipin: 代表系统MS卡multi pin 复用关系 
   0x00 : 代表MS data Bus 不与nand flash data Bus 复用
   0x01 : 代表MS data Bus 与nand flash data Bus 复用 
   0x02 : Reserved

//*****************************************************//
E. XDMultipin: 代表系统XD卡multi pin 复用关系 
   0x00 : 代表XD data Bus 不与nand flash data Bus 复用
   0x01 : 代表xd data Bus 与nand flash data Bus 复用 
   0x02 : Reserved

//*****************************************************//
F.MSStandard: 代表系统MS Data1 是否复用关系
   0x00 : 默认情况MS Data1与SD/nand flash bus 复用
   0x01 : 表示MS Data1 单独

//*****************************************************//
G.CF_DataBus:此参数保留.
     

2. 对于Card GPIO 配置，具体见GPIO 那项


   
 
   