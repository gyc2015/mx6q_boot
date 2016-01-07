# i.MX6Q的引脚功能配置

芯片上有很多引脚,每个引脚都有特定的功能.例如,串口UART需要两个引脚来接收和发送数据.
为了节省引脚,通常会让一个引脚对应很多功能,用户在需要的时候进行配置.

在i.MX6Q中每个引脚最多可以有8种复用的功能(called ALT modes).Freescale提供了IOMUX以及
IOMUX Controller管理各个引脚的功能.