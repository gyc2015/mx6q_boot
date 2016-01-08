# i.MX6Q的DDR3配置

i.MX6Q支持DDR3/DDR3L x16/x32/x64以及LPDDR2两通道的x16/x32内存,
通过Multi Mode DDR Controller(MMDC)配置.

为了保护MMDC寄存器的数据不被随意修改,在配置MMDC时,必须通过置位
MDSCR[CON_REQ]并检测MDSCR[CON_ACK]进入配置模式下.在配置完成后,
需要清除MDSCR[CON_REQ].


## 初始化

因为, 在芯片启动后MMDC并没有工作,没有时钟信号驱动DDR设备,整个DDR设备
接口都还没有启用.需要通过如下的步骤启动MMDR:

1. 置位MDSCR[CON_REQ],请求配置.因为此时MMDC还没有启动,所以没有必要查询
   MDSCR[CON_ACK]位.
2. 配置MDCFG0, MDCFG1, MDCFG2, MDOTC寄存器,设置期望的时间参数.
3. 配置MDMISC寄存器,设置DDR类型以及其它各种参数.
4. 配置MDOR寄存器,设置MMDC启动后的延时.
5. 配置MDCTL寄存器,设置DDR的物理参数(density and burst length).
6. 进行ZQ校验,以调整DDR的驱动能力.
7. 通过片选MDCTL[SDE_0]或者MDCTL[SDE_1]使能MMDC.此时MMDC进入启动和初始化状态.
8. 通过寄存器MDSCR向DDR设备发送特定的指令,以验证和完成初始化.
9. 通过寄存器MDSCR向DDR设备发送特定的指令,以配置DDR的模式寄存器.
10. 配置MDPDC和MAPSR寄存器,设置power down和self-refresh的进入和退出参数.
11. 通过MPZQHWCTRL和MPZQLP2CTRL寄存器设置ZQ方案.
12. 配置MDREF寄存器,设置和激活刷新方案.
13. 清除MDSCR[CON_REQ],释放配置请求.

其中步骤1到5可以以任何顺序执行.

## Write/Read Data Flow

1. 每次读/写请求都被一个长度为16/8的FIFO接收.只有当至少有两个可用的entry时,FIFO才
   接收读/写请求. 当burst length大于8时,拆分为两个包:一个burst length等于8,剩余的构成另外一个包.
2. 在各个读写访问之间有一个轮询仲裁机制,只有该仲裁的胜利者才被发送到re-ordering buffer.
3. 激活re-ordering机制根据dynamic score确定能够最有效地利用DDR总线的访问.ref: Dynamic scoring mode
4. 通过上个阶段的读/写请求将被发送到DDR控制逻辑中.
5. 当DDR指令控制单元(command control uint)准备好接受读/写访问时,如果必要的话它会根据bank model的状态
   以及计时器的参数向DDR设备发送一个precharge/active的指令.
6. 对于读请求,MMDC PHY采样数据,DDR逻辑把数据转移到读缓存中;对于写请求,DDR逻辑通过
   DDR PHY发送相应的数据到DDR设备中.
7. 对于写请求,MMDC包数据传回master.

