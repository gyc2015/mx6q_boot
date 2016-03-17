# i.MX6Q的引脚功能配置 IOMUX Controller, IOMUXC

芯片上有很多引脚,每个引脚都有特定的功能.例如,串口UART需要两个引脚来接收和发送数据.
为了节省引脚,通常会让一个引脚对应很多功能,用户在需要的时候进行配置.

在i.MX6Q中每个引脚最多可以有8种复用的功能(called ALT modes).Freescale提供了IOMUX以及
IOMUX Controller管理各个引脚的功能.

## 寄存器

* 32位的功能复用选择寄存器 (IOMUXC_SW_MUX_CTL_PAD_&lt;PAD NAME&gt;
or IOMUXC_SW_MUX_CTL_GRP_&lt;GROUP NAME&gt;)配置一个引脚或者一个预定义的引脚组工作在最多8个功能中的一个,
也可以配置强制打开引脚的输入通道(SION bit).
* 32位的引脚控制寄存器 (IOMUXC_SW_PAD_CTL_PAD_&lt;PAD_NAME&gt;
or IOMUXC_SW_PAD_CTL_GPR_&lg;GROUP NAME&gt;)配置一个引脚或者一个预定义的引脚组的特性.
* 14个32位通用寄存器(GPR0 to GPR13).
* 32位输入选择控制寄存器(IOMUXC_&lt;SOURCE&gt;_SELECT_INPUT)用于当有多个引脚作为一个模块的输入时选择信号源.

## 时钟

IOMUXC的时钟源如下表, 关于系统时钟配置参见[i.MX6Q系统时钟](/docs/ccm.md).

| clock name | clock root   | description             |
|------------|--------------|-------------------------|
| ipt_clk_io | enfc_clk_root| io clock                | 
| ipg_clk_s  | ipg_clk_root | Peripheral access clock | 

## 系统结构

![iomux_cell结构框图](/docs/images/iomux_cell_block_diagram.png)



