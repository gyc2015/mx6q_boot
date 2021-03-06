# ARM Coretex-A9

Cortex-A9是一款ARMv7-A架构的处理器, 支持32位的ARM指令集、16位和32为的Thumb指令集, 在Jazelle状态下
支持8位的Java bytecodes.

## Core Registers, 内核寄存器

一般ARM处理器有13个32位的通用寄存器, 记为R0-R12; 另外有3个特殊寄存器, 分别被记为SP, LR和PC, 有时也记为R13-R15.

* SP the stack pointer, 栈指针寄存器(R13):
处理器用SP指向当前活动的栈. 在Thumb指令集中, 大部分指令不能够访问SP, 只有那些需要处理SP的指针才能够访问.
在ARM指令集中, SP可以被当做通用寄存器使用, 但并不建议这样做.

* LR the link register, 链接寄存器(R14):
LR是一个保存返回链接信息的特殊寄存器. 不需要时, 该寄存器也可以用作其它目的.

* PC the program counter, 指令计数寄存器(R15):
在ARM指令集中, PC中的值为当前指令地址加8; 在Thumb指令集中, PC中的值为当前指令地址加4; 直接向PC中写入一个地址将导致
程序跳转到相应的地址. 大部分的Thumb指令是不能访问PC的.

* Low registers, (R0-R7):
因为大部分16位的Thumb指令只能访问前8个内核寄存器, 故, 称之为Low Registers. 而R8-R15则被成为High Registers.

虽然软件可以在不同的工作模式下访问这些寄存器, 但有些工作模式下这些寄存器可能有不同的物理存储, 这样的现象被称为Banking,
下图中显示了不同模式下寄存器的物理意义. 在所有的模式下, Low registers和PC都有相同的物理存储. 而High registers在不同的
模式下有可能不同, 如在FIQ模式下的R8-R12.

![ARM core registers](/docs/images/ARM_Register_Set.png)

## Program Status Register, 程序状态寄存器

程序状态寄存器涉及到三个寄存器: CPSR(Current Program Status Register), SPSRs(Saved Program Status Registers)和
APSR(Application Program Status Register).

在User模式下, 只能访问到CPSR的部分状态位, 此时的状态寄存器也被称为APSR. SPSRs则是用来保存先前的CPSR寄存器的值, 主要是为了异常处理
提供必要的信息的. 通过指令MRS可以把状态寄存器中的值读到一个内核寄存器中, MSR可以把一个内核寄存器或者立即数写到状态寄存器中.
通过指令CPS指令可以修改CPSR.M字段和CPSR.{A,I,F}字段.CPSR记录了处理器的状态和控制信息, 其位定义如下:

1. 条件标识位 Condition flags, bits[31:28], 这些位可以在任何模式下通过APSR读写访问.

* N, bit[31] 符号标识位. 如果指令的运行结果是负数, 则置1, 否则置0.
* Z, bit[30] 零标识位. 如果指令的运行结果为零, 则置1, 否则置0.
* C, bit[29] 进位标识位. 如果指令的执行过程中发生了进位或者借位, 则置1, 否则置0.
* V, bit[28] 溢出标识位. 如果指令的执行过程中发生了数据溢出, 则置1, 否则置0.

2. 溢出或者饱和标识位 Overflow or saturation flag, bit[27], 该位可以在任何模式下通过APSR访问.

* Q, bit[27] Set to 1 to indicate overflow or saturation occurred in some instructions, normally related to digital
signal processing (DSP).

3. Thumb的If-Then的执行状态位 IT[7:0], bits[15:10, 26:25]

* IT[7:5] 存放了当前IT块的base condition.
* IT[4:0] 需要执行的分支语句数量 | The value of the least significant bit of the condition code for each instruction in the block.

4. Jazelle bit, bit[24]

* J, bit[24] Jazelle模式标识位. 与T一起标识系统的工作模式.

5. bits[23,20], Reserved. RAZ/SBZP.

6. 大于等于标识位 GE[3,0], bits[19:16], 这些位可以在任何模式下通过APSR读写访问.

7. 大小端标识位 E, bit[9]

* 0 小端存储
* 1 大端存储

8. 中断掩码位, Mask bits[8:6]

* A, bit[8] Asynchronous abort掩码位
* I, bit[7] IRQ掩码位
* F, bit[6] FIQ掩码位

9. Thumb execution state bit, bit[5]

* T, bit[5] Thumb执行状态位. 和J一起决定了指令集状态.

10. M[4:0], bits[4:0]. 该位决定了当前处理器的状态.

## ARM处理器模式

在ARMv6之前处理器只有7种模式, 其中6种为Privileged, 只有User模式是Unprivileged. 引入TurstZone安全扩展(Security Extensions)后, 带来了Security的状态,
同时引入了一个Monitor的处理器模式, 该模式是Secure和Non-secure状态之间切换的gateway. ARMv7-A架构的Virtualization Extensions增加了一个Hypervisor模式.
而Virtualization允许在同一个平台上运行多个操作系统.

![Secure and Non-secure Worlds](/docs/images/secure_and_non_secure_worlds.png)
![Hypervisor](/docs/images/hypervisor_mode.png)

| Processor mode | CPSR.M[4:0] | 优先级 | Implemented                   | Security state  | 描述 |
|:--------------:|:-----------:|:-----:|:------------------------------:|:---------------:|:-----|
| User(USR)      | 10000       | PL0   | Always                         | Both            | 一般, 应用程序是工作在User mode下的. 在User mode下, CPU工作在PL0级, 不能访问受保护的系统资源. 在该模式下, 也不能够修改系统的工作模式. 若想要跳出User模式需要通过异常(exception)机制来实现. |
| FIQ            | 10001       | PL1   | Always                         | Both            | 是FIQ中断的默认模式. |
| IRQ            | 10010       | PL1   | Always                         | Both            | 是IRQ中断的默认模式. |
| Supervisor(SVC)| 10011       | PL1   | Always                         | Both            | Supervisor模式是SVC(Supervisor Call)异常的默认工作模式. 运行一个SVC指令, 将产生一个Supervisor Call异常. 系统复位后的就是工作在这种模式下的. |
| Monitor(MON)   | 10110       | PL1   | With Security Extensions       | Secure only     | 是Secure Monitor Call异常的工作模式 |
| Abort(ABT)     | 10111       | PL1   | Always                         | Both            | 当Data Abort异常和Prefetch Abort异常出现时, 系统的默认工作模式. |
| Hyp            | 11010       | PL2   | With Virtualization Extensions | Non-secure only | 是Non-Secure的PL2模式. 是Virtualization异常的一部分. 具体不知道. |
| Undefined(UND) | 11011       | PL1   | Always                         | Both            | 是任何指令相关异常的默认模式, 也是UNDEFINED指令的默认模式. |
| System(SYS)    | 11111       | PL1   | Always                         | Both            | 在System模式下, 软件工作在PL1级, System模式具有和User模式相同的对寄存器的访问权限, 但是不能够通过任何异常进入. |

![Modes, privilege levels, and security states](/docs/images/processor_modes.png)

## 优先级 Privilege level

在安全状态(secure state)下, 有两个优先级:

* PL0 在用户模式(User mode)下运行的软件具有PL0优先级
* PL1 除了用户模式之外的处理器模式具有PL1优先级

在非安全状态(non-secure state)下, 有三个优先级:

* PL0 User mode下运行的软件
* PL1 除了User和Hyp模式下的软件
* PL2 在由Virtualization Extensions的处理器上, 工作下Hyp模式下的软件具有PL2优先级.

在PL0下, 软件不能访问处理器的很多资源, 特别是一些用于配置系统的资源, 它们只能访问一些unprivileged的内存. 在PL1下, 软件可以访问除Virtualization Extension添加的功能外的所有资源.
PL2下的软件可以访问所有的系统资源.
