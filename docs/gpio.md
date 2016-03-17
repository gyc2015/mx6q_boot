# i.MX6Q的通用IO端口, GPIO

GPIO用8个寄存器, 一个边缘检测电路, 和一个中断逻辑提供通用端口的服务. 8个寄存器分别是:

* GPIO_DR, data register, 数据寄存器
* GPIO_GDIR, direction register, 方向寄存器
* GPIO_PSR, pad sample register, 引脚采样寄存器
* GPIO_ICR1, GPIO_ICR2, interrupt control register, 中断控制寄存器
* GPOIO_EDGE_SEL, edge select register, 边缘选择寄存器
* GPIO_IMR, interrupt mask register
* GPIO_ISR, interrupt status register

每个GPIO端口都有一个边缘检测电路, 可以通过软件配置以上升沿, 下降沿, 高电平还是低电平作为输入信号.

GPIO的时钟源如下表, 关于系统时钟配置参见[i.MX6Q系统时钟](/docs/ccm.md).

| clock name | clock root   | description             |
|------------|--------------|-------------------------|
| ipg_clk_s  | ipg_clk_root | Peripheral access clock | 


## 功能

当IOMUX设置为GPIO模式时, 相应的GPIO信号可以被用作general-purpose input/output.
每个GPIO信号都可以通过方向寄存器GPIO_GDIR配置为输入或者输出信号.
当配置为输出信号(GPIO_GDIR[n] = 1), 数据寄存器(GPIO_DR)的值将被放到输出引脚上;
当为输入信号(GPIO_GDIR[n] = 0), 输入状态将被存到采样寄存器(GPIO_PSR)的相应bit位上.

## 写模式

* 通过IOMUXC配置相应的引脚为GPIO模式, 如果需要通过PSR读取引脚输出反馈需要打开SION.
* 配置GPIO方向位输出GPIO_GDIR[n] = 1b
* 向数据寄存器GPIO_DR中写入数据.

> write sw_mux_ctl_pad_&lt;output[0-3]&gt;.mux_mode, &lt;GPIO_MUX_MODE&gt;<br>
> write sw_mux_ctl_pad_&lt;output[0-3]&gt;.sion, 1<br>
> write GDIR[31:4, output3_bit, output2_bit, output1_bit, output0_bit], 32'hxxxxxxxF<br>
> write DR, 32'hxxxxxxx5<br>
> read_cmp PSR, 32'hxxxxxxx5<br>




