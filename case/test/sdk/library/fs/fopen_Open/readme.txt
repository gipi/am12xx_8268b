作用：测试fopen/fread/fwrite & open/read/write函数；

宏定义：TEST_RAW_API=1测试open/read/write；否则测试的是fopen/fread/fwrite .

编译：make -C ../case/test/sdk/library/fs/Fopen &Open

目标文件：t_fopen.app

by yangli @2010.7.26
