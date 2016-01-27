# Memory Management Unit, MMU

内存管理单元(Memory Mangament Unit, MMU)的主要功能就是提供了从虚拟内存空间(virtual memory space)到物理内存空间
(physical memory space)的映射关系. 一般这种映射关系对于应用程序而言是透明的. 也就是说, 应用程序并不清楚这种映射
机制, 通常都是由专门硬件实现. 这使得应用程序可以完全不用考虑系统中同时运行的其它程序, 而在自己的虚拟内存空间中独立
地运行, 好像占用了整个内存空间一样.

MMU使得我们能够在一个系统中创建多个虚拟地址映射, 每个task都可以有一个自己的虚拟内存映射. 操作系统负责把每个程序的
代码和数据加载到合适的物理地址中, 而应用程序本身是不需要知道物理地址的. MMU通过树形结构的转换表(translation tables)
来建立两个地址空间之间的映射关系. 

转换表的entries是根据虚拟地址组织的, 在描述地址映射关系外, 还记录了访问权限和地址页的内存属性. 下图是Core, MMU, Cache和
内存之间的关系. 由Core产生的地址是虚拟地址, 当开启MMU时所有由Core产生的地址都需要经过MMU处理. 本质上MMU替换了虚拟地址中
的高位数据, 进而生成物理地址(physical address). 在MMU中专门设计了硬件Table Walk Unit读取内存中的转换表, 读取的过程
称为 translation table walking.

![MMU](/docs/images/memory_management_unit.png)

## Translation Lookaside Buffer, TLB

TLB是记录了MMU中当前正在执行页转换的一个cache. 系统的一次内存访问, 将首先查询TLB中是否已经缓存了相应的转换. 如果存在,
则称为一次TLB命中, 并由TLB直接得到物理地址. 否则称为一次TLB miss, 需要经过translation table walking加载新的转换表.
下图是一个TLB结构的示意图, 以虚拟地址作为tag, 记录了物理地址, 内存属性, ASID(Address Space ID).

![Illustration of TLB structure](/docs/images/tlb_structure.png)

## 开启MMU

如下段代码所示, 通过修改系统控制寄存器SCTLR的M位就可以开启MMU. 在开启MMU之前, 转换表(translation tables)必须已经
写在内存中了, 而且寄存器TTBR必须指向这段内存.

	MRC p15, 0, R1, c1, c0, 0      ; Read control register
	ORR R1, #0x1                   ; Set M bit
	MCR p15, 0, R1, c1, c0, 0      ; Write control register and enable MMU
	

