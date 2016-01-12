# i.MX6Q的启动

上电后(Power On Reset, POR)由硬件重置逻辑(hardware reset logic)控制core从片上的boot ROM开始运行。
boot ROM的工作如下：
1. 查询BOOT_MODE[1:0]以及eFUSEs或者GPIO配置以决定启动设备
2. 从启动设备中读取IVT和设备配置数据(Device Configuration Data, DCD)

在启动过程中,core可以访问所有的外设.

## 启动引脚配置

i.MX6Q有四种启动模式，由BOOT_MODE[1:0]决定。当POR_B输入的上升沿到来时，Core根据采样引脚BOOT_MODE0和
BOOT_MODE1到BOOT_MODE[1:0]中。

| Boot_MODE[1:0] |      Boot Type    |
|:--------------:|:-----------------:|
|       00       | Boot From Fuses   |
|       01       | Serial Downloader |
|       10       | Internal Boot     |
|       11       | Reserved          |

i.MX6Q支持的boot设备：
* NOR Flash
* NAND Flash
* OneNAND Flash
* SD/MMC
* Serial ATA (SATA) HDD
* Serial (I2C/SPI) NOR Flash and EEPROM

## Serial Downloader, Boot_MODE=10

Serial Downloader提供了一种通过USB接口下载镜像到芯片的功能.在这种模式下,若WDOG_ENABLE的eFuse被置1,
ROM设置将配置WDOG1的超时阈值为90秒,并不断地查询USB链接情况. 若在USB OTG1上没有检测到链接并且看门狗
溢出,芯片将重启. 需要注意的是,下载的镜像必须持续地喂狗.

* [串行下载协议(Serial Download Protocol, SDP)](./serial_download_protocol)

USB的VID/PID和Strings如下:

| Description                       | Value                          |
|-----------------------------------|--------------------------------|
| VID                               | 0x15A2(Freescale vendor ID)    |
| PID                               | 0x0054                         |
| String Descriptor1 (manufacturer) | Freescale Semiconductor, Inc.  |
| string Descriptor2 (product)      | S Blank<br>ARIK<br>SE Blank<br>ARIK<br>NS Blank<br>ARIK |
| string Descriptor4                | Freescale Flash                |
| string Descriptor5                | Freescale Flash                |

## Image Vector Table, IVT

Image Vector Table(IVT)是ROM从启动设备的程序镜像中读取的数据结构,其格式如下:

    int32 header:  [version = 0x40,31:24 | length = 0x0020, 23:8 | tag = 0xD1, 7:0], length大端存储
    int32 program_entry: 可执行程序的入口地址
    int32 reserved1
    int32 dcd_ptr: 设备配置数据(DCD)的绝对地址
    int32 boot_data_ptr: 指向boot data的绝对地址
    int32 self_ptr: 指向IVT自己的绝对地址
    int32 csf_ptr: HAB库用到的Command Sequence File(CSF)的绝对地址
    int32 reserved2

在启动过程中,ROM从启动设备的固定地址中装载IVT到芯片中.不同设备的IVT地址如下:

|   Boot Device Type   |   Image Vector Table Offset  |  Initial Load Region Size  |
|----------------------|------------------------------|----------------------------|
| NOR                  | 4 Kbyte = 0x1000 bytes       | Entire Image Size          |
| NAND                 | 1 Kbyte = 0x400  bytes       | 4 Kbyte                    |
| OneNAND              | 256 bytes = 0x100 bytes      | 1 Kbyte                    |
| SD/MMC/eSD/eMMC/SDXC | 1 Kbyte = 0x400 bytes        | 4 Kbyte                    |
| I2C/SPI EEPROM       | 1 Kbyte = 0x400 bytes        | 4 Kbyte                    |
| SATA                 | 1 Kbyte = 0x400 bytes        | 4 Kbyte                    |

## Boot Data Structure

在IVT中有boot_data_ptr指引了boot data的绝对地址. boot data描述了程序镜像的主要信息,其格式如下:

    int32 start: 镜像的绝对地址
	int32 length: 镜像的大小
    int32 plugin: plugin flag

## Device Configuration Data, DCD

系统上电后,芯片会对所有外设的寄存器有个默认的配置.但是,有些外设需要在使用之前重新配置,例如DDR.
DCD是镜像中对于外设的配置信息的描述数据,是一个大端存储的列表,由Header和一系列的CMD组成,最多有1768个字节.
其格式如下:

	|  Header  |
	|----------|
	|  [CMD]   |
	|----------|
	|  [CMD]   |
	|----------|
	|   ....   |
	
DCD只支持如下的一些设备:

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

DCD支持四种指令: Write Data Command, Check Data Command, NOP Command, Unlock Command.这里只介绍Write Data
指令,至于其余指令参考i.MX 6Dual/6Quad Applications Processor Reference Manual(IMX6DQRM)的8.6.2节.

### Header格式

  [version = 0x41,31:24 | length, 23:8 | tag = 0xD2, 7:0]

其中version指的是HAB的版本, i.MX6支持HAB 4.1, 而i.MX53则是HAB 4.0. V4.1与V4.0相比增加了Encrypted Boot的功能.
length以大端格式记录了整个DCD的字节数,包括Header在内.
tag就是一个标识.

### Write Data Command

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

