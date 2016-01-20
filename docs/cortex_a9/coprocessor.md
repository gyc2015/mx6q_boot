# 协处理器 Coprocessor

为拓展处理器的功能, ARM架构提供了对协处理器的支持, 并提供了协处理器指令集(Coprocessor Instructions)来访问
16个协处理器(CP0~CP15).

* CP15提供系统控制功能. 包括体系结构和特征标识(architecture and feature indentification), 以及控制、状态信息
和配置. CP15也提供Performance monitor register.

* CP14支持调试、Thumb执行环境、Java bytecode的直接执行.

* CP10和CP11支持浮点和向量运算, 控制和配置浮点以及高级SIMD.

* CP8, CP9, CP12, CP13保留做后序扩展, 任何企图访问这些协处理器的指令都是UNDEFINED.

协处理器实际上是一系列寄存器. 可以通过MCR指令写数据到指定的协处理器寄存器中, 也可以通过MRC从协处理器中读取指定寄存器
的值. 指令语法如下:

	MCR{<cond>} <p>, <opcode_1>, <Rd>, <CRn>, <CRm> {, <opcode_2>}
	MRC{<cond>} <p>, <opcode_1>, <Rd>, <CRn>, <CRm> {, <opcode_2>}

其中, cond为指令执行条件码, 缺省为无条件执行; p为协处理器名称; Rd为处理器的寄存器; CRn和CRm为协处理器的寄存器; 
opcode_1和opcode_2为操作码. CRn, CRm, opcode_1和opcode_2一起决定了协处理器的操作.


## 系统控制协处理器 System Control

系统控制协处理器CP15的主要功能如下:

* 整个系统的控制和配置
* MMU的配置和管理
* cache的配置和管理
* 系统任务监视(system performance monitoring)

* [Cortex-A9 Technical Reference Manual](/docs/manuals/Cortex_A9_Technical_Reference_Manual.4.1.pdf)的4.2节描述了如何
访问该协处理器的寄存器.
* [ARMv7-A Architecture Reference Manual](/docs/manuals/ARMv7_A_architecture_reference_manual.pdf)的B4节描述了各个寄存
器的位定义和功能.
