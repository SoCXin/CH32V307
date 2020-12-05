# [CH558](https://github.com/SoCXin/CH558)

[![sites](http://182.61.61.133/link/resources/SoC.png)](http://www.SoC.Xin)

#### [Vendor](https://github.com/SoCXin/Vendor)：[WCH](http://www.wch.cn/)
#### [Core](https://github.com/SoCXin/8051)：[E8051](https://github.com/SoCXin/8051)
#### [Level](https://github.com/SoCXin/Level)：12/56MHz

## [简介](https://github.com/SoCXin/CH558/wiki)

[CH558](https://github.com/SoCXin/CH558)兼容MCS51的增强型E8051内核，79%指令是单字节单周期指令，3K BootLoader + 32K CodeFlash，4K xRAM + 256B iRAM，5K DataFlash。

CH558内嵌USB全速收发器，支持USB-Device 设备模式，支持 USB 2.0全速 12Mbps。内置 FIFO支持最大64字节数据包,支持 DMA。

CH558提供丰富的接口资源，包括内置类485数据收发器、LED控制卡接口和快速电平捕捉功能，其他包括1组SPI、2路串口、8位并口等，UART1兼容16C550；内置4组定时器；提供1路16位PWM；支持8通道10/11位ADC。


[![sites](docs/CH558.png)](http://www.wch.cn/products/CH558.html)

#### 关键特性

* 5K DataFlash
* USB全速设备模式
* SPI + UART x 2 (类485数据收发器/16C550)
* 封装(LQFP48/SSOP20)

### [资源收录](https://github.com/SoCXin)

* [参考文档](docs/)
* [参考资源](src/)
* [参考工程](project/)

### [选型建议](https://github.com/SoCXin)

[CH558](https://github.com/SoCXin/CH558)配置接近[CH559](https://github.com/SoCXin/CH559)，主要特点在于数据存储和串口收发通信

###  [SoC芯平台](http://www.SoC.Xin)
