# i.MX6Q

本来是想在一个i.MX6Q的板子上移植一个u-boot和kernel的, 但是不知道自己的板子有没有问题.就想写一些裸版的程序先测试一下各种外设, 再移植.
翻看了u-boot的源码之后, 发现它想要支持各种板子, 源码目录里有很多不需要的东西, 所以决定抛开u-boot, 针对自己的板子搞一个测试项目. 当然,
很多代码都还是借鉴的u-boot, 有些是从u-boot中直接拷贝过来的.

这个项目的主要目的是学习Cortex-A9的结构和i.MX6Q的配置, 为后序操作系统的开发做准备工作.

关于板子的配置写在include/config.h中，前缀 CONFIG_

约定一个word为32位, 一个doubleword为64位, 一个halfword为16位, 一个byte为8位

## i.MX6Q外设和配置

i.MX6Quad是Freescale公司提供的一款Cortex-A9内核的4核ARM芯片.

* [i.MX6Q系统的启动过程](/docs/system_boot.md)
* [i.MX6Q引脚功能的配置](/docs/iomux.md)
* [i.MX6Q的DDR3配置](/docs/mmdc.md)

## ARM Cortex-A9 处理器

* [ARM Cortex-A9寄存器](/docs/cortex_a9/cortex_a9.md)
* [指令集 Instruction Sets](/docs/cortex_a9/instruction_sets.md)
* [协处理器Coprocessor](/docs/cortex_a9/coprocessor.md)

## 计算机体系结构

* [Caches](/docs/arch/caches.md)
* [Memory Management Unit, MMU](/docs/arch/mmu.md)

