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

## Program Status Register, 程序状态寄存器

程序状态寄存器涉及到三个寄存器: CPSR(Current Program Status Register), SPSRs(Saved Program Status Registers)和
APSR(Application Program Status Register).

APSR是CPSR的一个子集, 通过APSR可以访问到CPSR的一些状态位. SPSRs则是用来保存先前的CPSR寄存器的值, 主要是为了异常处理
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