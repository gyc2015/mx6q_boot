# i.MX6Q的芯片特性

i.MX6Q是一款基于ARM Coretx-A9 MPCore的芯片, 有4个CPU核. 每个核都有独立的32KByte的L1指令和数据Cache, 各自
的计时器(timer)和看门狗(watchdog), 以及Cortex-A9 NEON MPE(Media Processing Engine)协处理器(Co-processor).

## L1指令和数据Cache

* A9的指令和数据Cache是分开的, 在i.MX6Q中I-cache和D-cache的大小都是32KBytes. Each cache can be disabled independently.
* 两个Cache都是4通道(4-Ways)的[set associative cache](/docs/arch/caches.md).
* 线长度(line length)为32bytes.
* I-Cache是虚拟索引(virtually indexed)物理标记(physically tagged)的.
* D-Cache是物理索引(physically indexed)物理标记的.

## L2 Cache

i.MX6Q提供了1M的[基于PL310控制器的L2 Cache](/docs/manuals/PrimeCell_Level_2_Cache_Controller_PL310.pdf).

* 4个核共用一个unified cache.
* 有16个way.
* Two Master AXI (64-bit) bus interfaces output of L2 cache.
