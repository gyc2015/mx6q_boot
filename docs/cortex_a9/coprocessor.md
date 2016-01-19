# 协处理器 Coprocessor

为拓展处理器的功能, ARM架构提供了对协处理器的支持, 并提供了协处理器指令集(Coprocessor Instructions)来访问
16个协处理器(CP0~CP15).

* CP15提供系统控制功能. 包括体系结构和特征标识(architecture and feature indentification), 以及控制、状态信息
和配置. CP15也提供Performance monitor register.

* CP14支持调试、Thumb执行环境、Java bytecode的直接执行.

* CP10和CP11支持浮点和向量运算, 控制和配置浮点以及高级SIMD.

* CP8, CP9, CP12, CP13保留做后序扩展, 任何企图访问这些协处理器的指令都是UNDEFINED.
