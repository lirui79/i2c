此目录编出的是操作 i2c 的一些测试程序，用于调试，或者脚本中调用。
详见 Makefile

对 I2C 的操作，主要是 open, write, read，位于 i2cdev.c。

testi2c.c 编出一个测试程序，可以在串口上执行，打印指定地址的数据内容

adjustSystemTimeByRTC.c 编出一个测试程序，可以在串口上执行，通过读取 RTC 中的时间来校准系统时间


