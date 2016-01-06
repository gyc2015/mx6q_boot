# i.MX6Q的uboot移植

* 关于板子的配置写在include/config.h中，前缀 CONFIG_

# i.MX6Q的启动过程

上电后(Power On Reset, POR)由硬件重置逻辑(hardware reset logic)控制ARM内核从片上的boot ROM开始运行。
boot ROM的工作如下：
1. 查询BOOT_MODE[1:0]决定启动方式:
| BOOT_MODE[1:0] | 启动模式      |
|:--------------:|:-------------:|
| 00             | 从Fuses启动   |
| 01             | 从USB OTG启动 |
| 10             | Internal Boot |
| 11             | 保留          |

i.MX6Q支持的boot设备：
* NOR Flash
* NAND Flash
* OneNAND Flash
* SD/MMC
* Serial ATA (SATA) HDD
* Serial (I2C/SPI) NOR Flash and EEPROM