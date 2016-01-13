# 设备配置数据

系统上电后,芯片会对所有外设的寄存器有个默认的配置.但是,有些外设需要在使用之前重新配置,例如DDR.
i.MX6Q提供了一种设备配置数据(Device Configuration Data, DCD),可以批量的修改芯片各种外设的寄存器,
进而提高系统启动速度.	

一般设备启动时,需要在镜像文件中的特定位置写有DCD数据,参见[镜像文件构成](docs/system_boot.md);
通过[USB OTG提供的serial downloader启动时](docs/serial_download_protocol.md),可以用DCD_WRITE指令直接写入芯片.
DCD只支持对如下设备的配置:

|   Address range   |  Start address  |  Last Address  |
|:-----------------:|:---------------:|:--------------:|
| IOMUXC registers  |    0x020E0000   |   0x020E3FFF   |
| CCM register set  |    0x020C4000   |   0x020C7FFF   |
| ANADIG registers  |    0x020C8000   |   0x020C8FFF   |
| MMDC register set |    0x021B0000   |   0x021B7FFF   |
| IRAM fress space  |    0x00907000   |   0x00937FF0   |
| EIM memory        |    0x08000000   |   0x0FFEFFFF   |
| EIM registers     |    0x021B8000   |   0x021BBFFF   |
| DDR               |    0x10000000   |   0xFFFFFFFF   |

DCD是一个大端存储的列表,由Header和一系列的CMD组成,最多有1768个字节.
其格式如下:

	|  Header  |
	|----------|
	|  [CMD]   |
	|----------|
	|  [CMD]   |
	|----------|
	|   ....   |

## Header格式

  [version = 0x41,31:24 | length, 23:8 | tag = 0xD2, 7:0]

其中version指的是HAB的版本, i.MX6支持HAB 4.1, 而i.MX53则是HAB 4.0. V4.1与V4.0相比增加了Encrypted Boot的功能.
length以大端格式记录了整个DCD的字节数,包括Header在内.
tag就是一个标识.

DCD支持四种指令: Write Data Command, Check Data Command, NOP Command, Unlock Command.这里只介绍Write Data
指令,至于其余指令参考i.MX 6Dual/6Quad Applications Processor Reference Manual(IMX6DQRM)的8.6.2节.

## Write Data Command

Write Data Command用来向目标地址列表中写入1-, 2-, or 4-byte的值,或者是bitmasks.
其格式如下:

	|  Parameter  |  Length  |  Tag  |
	|--------------------------------|
	|           Address              |
	|--------------------------------|
	|         Value/Mask             |
	|--------------------------------|
	|             ...                |
	|--------------------------------|
	|           Address              |
	|--------------------------------|
	|         Value/Mask             |

	
其中, Tag=0xCC,是一个字节;
Length占两个字节,是整个写指令的字节数,包括header在内;
Address为目标地址;
Value/Mask为将要被写入的数据或者bitmask
Parameter描述了写指令的各个参数,具体结构如下:

	|  7  |  6  |  5  |  Mask  |  Set  |  2  |  1  |  0  |
	|----------------------------------|-----------------|
	|               flags              |      bytes      |

其中, bytes指示了目标地址的大小: 1, 2, 还是4个字节;flags控制了指令的具体功能:

|  Mask  |  Set  |        Action            |     说明    |
|:------:|:-----:|:------------------------:|-------------|
|   0    |   0   | *address = val_mak       |    写数据   |
|   0    |   1   | *address = val_msk       |    写数据   |
|   1    |   0   | *address &= ~val_msk     |    清除位   |
|   1    |   1   | *address &#124;= val_msk |    设置位   |

