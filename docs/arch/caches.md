# Caches

## 1. 引言

cache这个词源自于法语的*cacher*, 意思是"to hide". 对于处理器来说, cache是处理器用来存放指令和数据的地方, 对于程序员和
系统而言是隐藏的. 在很多情况下, 我们即使不知道cache的存在也能写出很庞大的程序, 但是对于cache的理解还是很有必要的. 它能够
指引我们优化代码, 甚至一些潜在的bug也是因为cache导致的.

在计算机发展的初期, 处理器的计算速率和内存访问速率大致是相同的. 目前处理器的计算频率已经有了很大的提高, 而外部总线和存储
设备的速率提升却不能跟上处理器的计算频率, 在很多ARM系统中需要在几十甚至上百个内核周期以访问内存. 当然目前片上的SRAM具有
和内核相同的速率, 但是由于其昂贵的造价, 只能少量地使用. 为了平衡处理器和存储设备的频率, 以提高CPU的利用率, 进而提高系统的
处理速度, 人们提出了cache的概念.

本质上, cache就是在内核与内存之间的一块小而快的memory, 存储的是对内存的copies. 由于cache中存储的是内存的片段, 所以cache
必须同时记录片段在内存中的地址以及相应的数据. 当内核需要访问内存时, 它将先查看cache中是否已经有目标地址的拷贝, 若有则用之.
这就引入了cache hit和cache miss的概念, 提高cache命中率是提高系统速度的一个主要手段, 会在以后进行详细的介绍.

之所以可以通过提高cahe的命中率来提高系统的速度, 是由于程序运行过程中对代码和数据的访问往往是局部的. 也就是说, 人们倾向于
复用相同的代码, 以至于短时间内访问的内存地址不会有过大的变化. 例如, 代码中的循环意味着短时间内会重复的执行相同的代码很多遍.
而对于数据的访问, 也可以通过合理地安排数据的存储空间把数据访问地址限定在一个较小的范围. 

Cache在提高系统速度的同时, 也带来了不确定性. 因为Cache只是保存了内存的片段, 而且对其的访问速度远快于对内存的速度, 所以存在
cache中的内容与内存中不同步的情况. 在多核或者有DMA机制下就可能访问到未及时更新的数据, 这就是所谓的coherency问题. 如果程序
员没有意识到这个问题, 有时就会出现bug而且很难查找原因.

本文结构安排如下:

## 2. Cache结构

在计算机科学中, 存储设备的层级结构(memory hierarchy)是指靠近处理器内核的小而快的memory和相对远离内核的大而慢的memory所形成
的好似金字塔的层级结构. 例如在PC机中, 我们有靠近CPU的DDR内存用于运行程序, 由很大但很慢的硬盘用于存放各种资料和文件; 在嵌入式
系统中我们有运行快速的片上存储还有相对慢的片外扩展. 理论上在系统的任何两级具有不同速率的存储设备之间增加一个cache都可以提高
系统的速度.

在冯诺依曼架构中, 只有一个unified cache同时处理指令和数据. 而在基于ARM处理器的系统中, 通常都有两个cache, 一个用于取指令称为
instruction cache(I-cache), 另一个用于装载或者存储数据称为data cache(D-cache). 这种cache结构被称为Harvard caches. 一般,
把直接与内核逻辑链接的cache称为一级缓存(L1), 随着SRAM的大小和速度的发展, L1的大小也在不断增加, 目前一般都是16KB或者32KB.很
多ARM系统还有一个二级缓存(L2). 这是一个比L1大很多的cache, 一般为256KB, 512KB或者是1MB, 但是相对较慢而且是unified cache. 如
下图所示:

![Typical harvard caches](/docs/images/harvard_cache.png)

Cache需要记录地址, 数据和一些状态信息. 32位地址的top bits告诉cache这些信息来自于内存的何处, 称为tag. cache大小是指cache所能
记录的数据的大小. 虽然tags也需要在cache中有物理存储, 但它们并不参与cache大小的计算. 若对每个数据都记录一个tag地址, 将造成很大
的浪费, 一般会把多个地址作为一组标记在一个tag下面. 这种逻辑上的分块就是所谓的cache line, 用地址的middle bits(称为index)标记.
index是cache内部的索引方式不需要物理存储. 当一个cache line中有数据或者指令时, 称其是valid, 否则称invalid. 这样地址的bottom few
bits就可以不被记录在tag中了. 下图显示了一个cache的结构, 其中一个32位的地址被分为了3段: Tags在cache的tag RAM中有存储位置指示了
其在内存中的位置, index标记了cache内部line的索引方式, offset则是对line中的特定的存储空间的索引.

![cache architecture](/docs/images/cache_architecture.png)

* line: cache装载的最小单位, 对应着内存的一块连续的word空间.
* index: 是32位地址中的一部分用来标识line.
* way: 是对cache的一种划分方式, 每个way都有相同的大小.
* set: 每个way中具有相同index的line所构成的集合.
* tag: 是32位地址中的一部分用来告知cache相应的line中的数据对应的内存.

## 3. Cache的映射方式

Cache有很多种映射方式, 这里先介绍做简单的直接映射方式来说明前面提到的cache的一些概念和名词. 然后
介绍ARM经常使用的Set associative cache.

### 3.1 直接映射(Direct Mapped Cache)

在直接映射中, 内存中的每块地址都对应cache中唯一的一个存储空间. 但是由于内存空间远比cache大, 所以
存在多个内存地址映射到一块cache存储空间的情况. 如下图所示的一个cache, 其中只有4个line每个line可以
存放4个word的数据. 0xXXXX.XXY0被映射到相同的存储空间中, 其中X为任意16进制数字, Y为对4取模相等的16
进制数字.

![direct mapped cache](/docs/images/direct_mapped_cache.png)

对于这种直接映射的Cache, 其控制器将用32位地址中的[3:2]作为offset来指引一个word, 或者[3:1]指引一
个halfword, [3:0]指引一个byte. 用[5:4]作为index指引一个line, 用[31:6]表示tag. 访问一个地址时,
首先从中取出index位然后对比index的tag是否匹配, 若两者相同并且valid位标识着line中有正确的数据, 则称为
一次命中(hit), 可以根据offset访问数据. 如果tag不匹配, 则称为一次miss, 此时将从内存中一次读取4个
word替换line中原有的数据并更新tag.

在这种映射中, 内存中具有相同[5:4]的地址将被映射到相同的cache line中. 而同一时间只能有一个被加载
到cache中. 这将很容易产生一种被称为thrashing的问题. 比如下面的这个循环, 如果result, data1和data2所指
的内存分别为0x00, 0x40和0x80, 那么每次循环都会产生3次miss, 迫使cache不断的刷新起不到提升速度的作用.

	for (int i = 0; i < size; i++) {
		result[i] = data1[i] + data2[i];
	}


### 3.2 Set associative caches

ARM通常采用set accociative cache, 这种映射方式可以大大的降低直接映射中的thrashing发生的概率. 所付出的
代价就是更为复杂的硬件结构和较长的hit检测(需要对比多个tag). 其映射示意图如下:

![set associative cache](/docs/images/set_associative_cache.png)

在这种映射机制中, cache被分为一系列大小相等的way. 内存地址将被映射到一个way中而不是更细粒度的line中.
index被用来指引的line实际上是每个way中具有相同index的line的集合, 即set. 为了判定一次访问是否命中, 必须
依次对比set中每个line的tag.



---

通常用V表示valid bit, D表示dirty bit
